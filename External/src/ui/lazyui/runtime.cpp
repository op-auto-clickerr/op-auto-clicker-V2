#include "lazyui.hpp"

#include "runtime.hpp"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <unordered_map>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <shellapi.h>
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "shell32.lib" )

namespace lazyui
{
    pointer_frame pointer;
    window_drag_state window_drag;

    namespace
    {
        ID3D11Device* graphics_device = nullptr;
        IDXGISwapChain* graphics_swap_chain = nullptr;

        ImGui_ImplDX11_RenderState* render_state( )
        {
            return static_cast< ImGui_ImplDX11_RenderState* >( ImGui::GetPlatformIO( ).Renderer_RenderState );
        }

        template < typename T >
        void release( T*& object )
        {
            if ( object )
                object->Release( );
            object = nullptr;
        }

        void push_texture( ImDrawList* draw, texture_handle texture )
        {
#if IMGUI_VERSION_NUM >= 19200
            draw->PushTexture( texture );
#else
            draw->PushTextureID( texture );
#endif
        }

        void pop_texture( ImDrawList* draw )
        {
#if IMGUI_VERSION_NUM >= 19200
            draw->PopTexture( );
#else
            draw->PopTextureID( );
#endif
        }

        void fill_rounded_texture( ImDrawList* draw, texture_handle texture, ImVec2 min, ImVec2 max, ImVec2 uv_min, ImVec2 uv_max, const corner_radii& radii, ImU32 tint )
        {
            push_texture( draw, texture );
            const int vertex_start = draw->VtxBuffer.Size;
            rounded_path( draw, min, max, radii );
            draw->PathFillConvex( tint );
            ImGui::ShadeVertsLinearUV( draw, vertex_start, draw->VtxBuffer.Size, min, max, uv_min, uv_max, true );
            pop_texture( draw );
        }

        ID3D11BlendState* blend_states[3]{ };
        ID3D11Device* blend_device = nullptr;

        const char blend_shader[] = R"(
cbuffer blend_pass : register(b0)
{
    int mode;
    float3 padding;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

sampler sampler0 : register(s0);
Texture2D texture0 : register(t0);
Texture2D backdrop : register(t1);

float luminance(float3 c) { return dot(c, float3(0.3, 0.59, 0.11)); }

float3 clip_color(float3 c)
{
    const float l = luminance(c);
    const float low = min(c.r, min(c.g, c.b));
    const float high = max(c.r, max(c.g, c.b));
    if (low < 0.0)
        c = l + (c - l) * l / max(l - low, 1e-5);
    if (high > 1.0)
        c = l + (c - l) * (1.0 - l) / max(high - l, 1e-5);
    return c;
}

float3 set_luminance(float3 c, float l) { return clip_color(c + (l - luminance(c))); }

float saturation(float3 c) { return max(c.r, max(c.g, c.b)) - min(c.r, min(c.g, c.b)); }

float3 set_saturation(float3 c, float s)
{
    const float low = min(c.r, min(c.g, c.b));
    const float high = max(c.r, max(c.g, c.b));
    return high > low ? (c - low) * s / (high - low) : float3(0.0, 0.0, 0.0);
}

float3 screen(float3 b, float3 s) { return b + s - b * s; }
float3 hard_light(float3 b, float3 s) { return s <= 0.5 ? b * (2.0 * s) : screen(b, 2.0 * s - 1.0); }
float3 color_dodge(float3 b, float3 s) { return b <= 0.0 ? 0.0 : (s >= 1.0 ? 1.0 : min(1.0, b / max(1.0 - s, 1e-5))); }
float3 color_burn(float3 b, float3 s) { return b >= 1.0 ? 1.0 : (s <= 0.0 ? 0.0 : 1.0 - min(1.0, (1.0 - b) / max(s, 1e-5))); }

float3 soft_light(float3 b, float3 s)
{
    const float3 d = b <= 0.25 ? ((16.0 * b - 12.0) * b + 4.0) * b : sqrt(b);
    return s <= 0.5 ? b - (1.0 - 2.0 * s) * b * (1.0 - b) : b + (2.0 * s - 1.0) * (d - b);
}

float3 blend(float3 b, float3 s)
{
    switch (mode)
    {
    case 0: return b * s;
    case 1: return screen(b, s);
    case 2: return hard_light(s, b);
    case 3: return min(b, s);
    case 4: return max(b, s);
    case 5: return color_dodge(b, s);
    case 6: return color_burn(b, s);
    case 7: return hard_light(b, s);
    case 8: return soft_light(b, s);
    case 9: return abs(b - s);
    case 10: return b + s - 2.0 * b * s;
    case 11: return max(b + s - 1.0, 0.0);
    case 12: return set_luminance(set_saturation(s, saturation(b)), luminance(b));
    case 13: return set_luminance(set_saturation(b, saturation(s)), luminance(b));
    case 14: return set_luminance(s, luminance(b));
    default: return set_luminance(b, luminance(s));
    }
}

float4 main(PS_INPUT input) : SV_Target
{
    const float4 source = input.color * texture0.Sample(sampler0, input.uv);
    const float4 below = backdrop.Load(int3(input.position.xy, 0));
    const float3 base = below.a > 0.0 ? saturate(below.rgb / below.a) : float3(0.0, 0.0, 0.0);
    const float3 top = saturate(source.rgb);
    const float3 mixed = (1.0 - below.a) * top + below.a * blend(base, top);
    return float4(source.a * mixed + (1.0 - source.a) * below.rgb, source.a + below.a * (1.0 - source.a));
}
)";

        struct blend_constants
        {
            int mode;
            float padding[3];
        };

        struct backdrop_resources
        {
            ID3D11Device* device = nullptr;
            UINT width = 0;
            UINT height = 0;
            DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
            ID3D11Texture2D* texture = nullptr;
            ID3D11ShaderResourceView* view = nullptr;
            ID3D11PixelShader* shader = nullptr;
            ID3D11Buffer* constants = nullptr;
            ID3D11BlendState* overwrite = nullptr;
            bool failed = false;
        };

        backdrop_resources backdrop;

        void release_backdrop( )
        {
            release( backdrop.view );
            release( backdrop.texture );
            release( backdrop.shader );
            release( backdrop.constants );
            release( backdrop.overwrite );
            backdrop.width = backdrop.height = 0;
        }

        bool ensure_backdrop( ID3D11Device* device, const D3D11_TEXTURE2D_DESC& target )
        {
            if ( backdrop.failed && backdrop.device == device )
                return false;
            if ( backdrop.device == device && backdrop.width == target.Width && backdrop.height == target.Height && backdrop.format == target.Format && backdrop.shader )
                return true;
            release_backdrop( );
            backdrop.device = device;
            backdrop.failed = false;
            backdrop.width = target.Width;
            backdrop.height = target.Height;
            backdrop.format = target.Format;
            D3D11_TEXTURE2D_DESC description{ };
            description.Width = target.Width;
            description.Height = target.Height;
            description.MipLevels = 1;
            description.ArraySize = 1;
            description.Format = target.Format;
            description.SampleDesc = { 1, 0 };
            description.Usage = D3D11_USAGE_DEFAULT;
            description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            bool created = SUCCEEDED( device->CreateTexture2D( &description, nullptr, &backdrop.texture ) ) &&
                           SUCCEEDED( device->CreateShaderResourceView( backdrop.texture, nullptr, &backdrop.view ) );
            ID3DBlob* bytecode = nullptr;
            created = created && SUCCEEDED( D3DCompile( blend_shader, sizeof( blend_shader ) - 1, nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &bytecode, nullptr ) );
            created = created && SUCCEEDED( device->CreatePixelShader( bytecode->GetBufferPointer( ), bytecode->GetBufferSize( ), nullptr, &backdrop.shader ) );
            release( bytecode );
            D3D11_BUFFER_DESC buffer{ };
            buffer.ByteWidth = sizeof( blend_constants );
            buffer.Usage = D3D11_USAGE_DYNAMIC;
            buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            created = created && SUCCEEDED( device->CreateBuffer( &buffer, nullptr, &backdrop.constants ) );
            D3D11_BLEND_DESC overwrite{ };
            overwrite.RenderTarget[0].BlendEnable = FALSE;
            overwrite.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            created = created && SUCCEEDED( device->CreateBlendState( &overwrite, &backdrop.overwrite ) );
            if ( !created )
            {
                release_backdrop( );
                backdrop.failed = true;
            }
            return created;
        }

        void begin_backdrop_blend( ImGui_ImplDX11_RenderState* state, int formula )
        {
            ID3D11DeviceContext* context = state->DeviceContext;
            ID3D11RenderTargetView* target = nullptr;
            context->OMGetRenderTargets( 1, &target, nullptr );
            if ( !target )
                return;
            ID3D11Resource* resource = nullptr;
            target->GetResource( &resource );
            ID3D11Texture2D* texture = nullptr;
            bool copied = false;
            if ( resource && SUCCEEDED( resource->QueryInterface( IID_PPV_ARGS( &texture ) ) ) )
            {
                D3D11_TEXTURE2D_DESC description{ };
                texture->GetDesc( &description );
                if ( ensure_backdrop( state->Device, description ) )
                {
                    ID3D11ShaderResourceView* unbound = nullptr;
                    context->PSSetShaderResources( 1, 1, &unbound );
                    if ( description.SampleDesc.Count > 1 )
                        context->ResolveSubresource( backdrop.texture, 0, texture, 0, description.Format );
                    else
                        context->CopySubresourceRegion( backdrop.texture, 0, 0, 0, 0, texture, 0, nullptr );
                    copied = true;
                }
                texture->Release( );
            }
            if ( resource )
                resource->Release( );
            target->Release( );
            if ( !copied )
                return;
            D3D11_MAPPED_SUBRESOURCE mapped{ };
            if ( SUCCEEDED( context->Map( backdrop.constants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) )
            {
                const blend_constants constants{ formula, { } };
                std::memcpy( mapped.pData, &constants, sizeof( constants ) );
                context->Unmap( backdrop.constants, 0 );
            }
            context->PSSetConstantBuffers( 0, 1, &backdrop.constants );
            context->PSSetShaderResources( 1, 1, &backdrop.view );
            context->PSSetShader( backdrop.shader, nullptr, 0 );
            const float factor[4]{ };
            context->OMSetBlendState( backdrop.overwrite, factor, 0xFFFFFFFF );
        }

        const char blur_shader[] = R"(
cbuffer blur_pass : register(b0)
{
    float2 direction;
    float sigma;
    float radius;
    float step;
    float lod;
    float2 padding;
    float2 clamp_min;
    float2 clamp_max;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

sampler sampler0;
Texture2D texture0;

float4 main(PS_INPUT input) : SV_Target
{
    uint width;
    uint height;
    texture0.GetDimensions(width, height);
    const float2 texel = direction / float2(width, height);
    float4 sum = 0;
    float total = 0;
    [loop]
    for (float offset = -radius; offset <= radius; offset += step)
    {
        const float weight = exp(-0.5 * offset * offset / (sigma * sigma));
        float2 position = input.uv + texel * offset;
        position = clamp_min + abs(position - clamp_min);
        position = clamp_max - abs(clamp_max - position);
        sum += texture0.SampleLevel(sampler0, clamp(position, clamp_min, clamp_max), lod) * weight;
        total += weight;
    }
    return sum / max(total, 1e-5);
}
)";

        struct blur_constants
        {
            float direction_x;
            float direction_y;
            float sigma;
            float radius;
            float step;
            float lod;
            float padding[2];
            float clamp_min[2];
            float clamp_max[2];
        };

        struct blur_pass
        {
            blur_constants constants;
            int target;
        };

        struct blur_resources
        {
            ID3D11Device* device = nullptr;
            UINT width = 0;
            UINT height = 0;
            DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
            ID3D11Texture2D* textures[3]{ };
            ID3D11ShaderResourceView* views[3]{ };
            ID3D11RenderTargetView* targets[3]{ };
            ID3D11PixelShader* shader = nullptr;
            ID3D11Buffer* constants = nullptr;
            ID3D11BlendState* overwrite = nullptr;
            ID3D11RenderTargetView* saved_target = nullptr;
            bool failed = false;
        };

        blur_resources blur;

        void release_blur( )
        {
            for ( int index = 0; index < 3; ++index )
            {
                release( blur.targets[index] );
                release( blur.views[index] );
                release( blur.textures[index] );
            }
            release( blur.shader );
            release( blur.constants );
            release( blur.overwrite );
            blur.width = blur.height = 0;
        }

        bool ensure_blur( )
        {
            if ( blur.failed || !graphics_device || !graphics_swap_chain )
                return false;
            ID3D11Texture2D* back_buffer = nullptr;
            if ( FAILED( graphics_swap_chain->GetBuffer( 0, IID_PPV_ARGS( &back_buffer ) ) ) )
                return false;
            D3D11_TEXTURE2D_DESC description{ };
            back_buffer->GetDesc( &description );
            back_buffer->Release( );
            if ( blur.device == graphics_device && blur.width == description.Width && blur.height == description.Height && blur.shader )
                return true;
            release_blur( );
            blur.device = graphics_device;
            blur.width = description.Width;
            blur.height = description.Height;
            blur.format = description.Format;
            description.MipLevels = 0;
            description.ArraySize = 1;
            description.SampleDesc = { 1, 0 };
            description.Usage = D3D11_USAGE_DEFAULT;
            description.CPUAccessFlags = 0;
            description.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            bool created = true;
            for ( int index = 0; index < 3 && created; ++index )
            {
                created = SUCCEEDED( graphics_device->CreateTexture2D( &description, nullptr, &blur.textures[index] ) ) &&
                          SUCCEEDED( graphics_device->CreateShaderResourceView( blur.textures[index], nullptr, &blur.views[index] ) ) &&
                          SUCCEEDED( graphics_device->CreateRenderTargetView( blur.textures[index], nullptr, &blur.targets[index] ) );
            }
            ID3DBlob* bytecode = nullptr;
            created = created && SUCCEEDED( D3DCompile( blur_shader, sizeof( blur_shader ) - 1, nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &bytecode, nullptr ) );
            created = created && SUCCEEDED( graphics_device->CreatePixelShader( bytecode->GetBufferPointer( ), bytecode->GetBufferSize( ), nullptr, &blur.shader ) );
            release( bytecode );
            D3D11_BUFFER_DESC buffer{ };
            buffer.ByteWidth = sizeof( blur_constants );
            buffer.Usage = D3D11_USAGE_DYNAMIC;
            buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            created = created && SUCCEEDED( graphics_device->CreateBuffer( &buffer, nullptr, &blur.constants ) );
            D3D11_BLEND_DESC overwrite{ };
            overwrite.RenderTarget[0].BlendEnable = FALSE;
            overwrite.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            created = created && SUCCEEDED( graphics_device->CreateBlendState( &overwrite, &blur.overwrite ) );
            if ( !created )
            {
                release_blur( );
                blur.failed = true;
            }
            return created;
        }

        void blur_begin( const ImDrawList*, const ImDrawCmd* )
        {
            ImGui_ImplDX11_RenderState* state = render_state( );
            if ( !state || !blur.textures[0] )
                return;
            state->DeviceContext->OMGetRenderTargets( 1, &blur.saved_target, nullptr );
            if ( !blur.saved_target )
                return;
            ID3D11Resource* resource = nullptr;
            blur.saved_target->GetResource( &resource );
            ID3D11Texture2D* texture = nullptr;
            if ( resource && SUCCEEDED( resource->QueryInterface( IID_PPV_ARGS( &texture ) ) ) )
            {
                D3D11_TEXTURE2D_DESC description{ };
                texture->GetDesc( &description );
                if ( description.Width == blur.width && description.Height == blur.height && description.Format == blur.format )
                {
                    if ( description.SampleDesc.Count > 1 )
                        state->DeviceContext->ResolveSubresource( blur.textures[0], 0, texture, 0, description.Format );
                    else
                        state->DeviceContext->CopySubresourceRegion( blur.textures[0], 0, 0, 0, 0, texture, 0, nullptr );
                    state->DeviceContext->GenerateMips( blur.views[0] );
                }
                texture->Release( );
            }
            if ( resource )
                resource->Release( );
        }

        void blur_pass_callback( const ImDrawList*, const ImDrawCmd* command )
        {
            ImGui_ImplDX11_RenderState* state = render_state( );
            const auto* pass = static_cast< const blur_pass* >( command->UserCallbackData );
            if ( !state || !pass || !blur.targets[pass->target] )
                return;
            ID3D11DeviceContext* context = state->DeviceContext;
            context->OMSetRenderTargets( 1, &blur.targets[pass->target], nullptr );
            if ( pass->target == 2 )
                context->GenerateMips( blur.views[1] );
            D3D11_MAPPED_SUBRESOURCE mapped{ };
            if ( SUCCEEDED( context->Map( blur.constants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) )
            {
                std::memcpy( mapped.pData, &pass->constants, sizeof( pass->constants ) );
                context->Unmap( blur.constants, 0 );
            }
            context->PSSetConstantBuffers( 0, 1, &blur.constants );
            context->PSSetShader( blur.shader, nullptr, 0 );
            const float factor[4]{ };
            context->OMSetBlendState( blur.overwrite, factor, 0xFFFFFFFF );
        }

        void blur_end( const ImDrawList*, const ImDrawCmd* )
        {
            ImGui_ImplDX11_RenderState* state = render_state( );
            if ( state && blur.saved_target )
                state->DeviceContext->OMSetRenderTargets( 1, &blur.saved_target, nullptr );
            release( blur.saved_target );
        }

        texture_handle blur_texture( int index )
        {
#if IMGUI_VERSION_NUM >= 19200
            return ImTextureRef{ reinterpret_cast< ImTextureID >( blur.views[index] ) };
#else
            return reinterpret_cast< ImTextureID >( blur.views[index] );
#endif
        }

        bool draw_exact_blur( ImDrawList* draw, ImVec2 min, ImVec2 max, const corner_radii& radii, float sigma, float alpha )
        {
            if ( sigma <= 0.f || max.x <= min.x || max.y <= min.y || !ensure_blur( ) )
                return false;
            const float reach = ImCeil( sigma * 3.f );
            const ImVec2 size{ static_cast< float >( blur.width ), static_cast< float >( blur.height ) };
            const auto uv = [&]( ImVec2 point ) { return ImVec2{ point.x / size.x, point.y / size.y }; };
            const float step = ImMax( 1.f, reach / 48.f );
            const float lod = ImLog( step ) / ImLog( 2.f );
            const ImVec2 frame_min = uv( min + ImVec2{ 0.5f, 0.5f } );
            const ImVec2 frame_max = uv( max - ImVec2{ 0.5f, 0.5f } );
            draw->PushClipRectFullScreen( );
            draw->AddCallback( blur_begin, nullptr );
            const blur_pass horizontal{ { 1.f, 0.f, sigma, reach, step, lod, { }, { frame_min.x, frame_min.y }, { frame_max.x, frame_max.y } }, 1 };
            draw->AddCallback( blur_pass_callback, const_cast< blur_pass* >( &horizontal ), sizeof( horizontal ) );
            const float pad = ImPow( 2.f, ImCeil( lod ) ) + 1.f;
            const ImVec2 pass_min = ImMax( min - ImVec2{ pad, pad }, ImVec2{ 0.f, 0.f } );
            const ImVec2 pass_max = ImMin( max + ImVec2{ pad, pad }, size );
            draw->AddImage( blur_texture( 0 ), pass_min, pass_max, uv( pass_min ), uv( pass_max ) );
            const blur_pass vertical{ { 0.f, 1.f, sigma, reach, step, lod, { }, { frame_min.x, frame_min.y }, { frame_max.x, frame_max.y } }, 2 };
            draw->AddCallback( blur_pass_callback, const_cast< blur_pass* >( &vertical ), sizeof( vertical ) );
            draw->AddImage( blur_texture( 1 ), min, max, uv( min ), uv( max ) );
            draw->AddCallback( blur_end, nullptr );
            draw->AddCallback( ImDrawCallback_ResetRenderState, nullptr );
            draw->PopClipRect( );
            fill_rounded_texture( draw, blur_texture( 2 ), min, max, uv( min ), uv( max ), radii, IM_COL32( 255, 255, 255, static_cast< int >( ImSaturate( alpha ) * 255.f + 0.5f ) ) );
            return true;
        }
    }

    void set_graphics( ID3D11Device* device, IDXGISwapChain* swap_chain )
    {
        graphics_device = device;
        graphics_swap_chain = swap_chain;
    }

    corner_radii fit_corner_radii( ImVec2 min, ImVec2 max, const corner_radii& radii )
    {
        const float width = ImMax( 0.f, max.x - min.x );
        const float height = ImMax( 0.f, max.y - min.y );
        corner_radii fitted{ ImMax( 0.f, radii.top_left ), ImMax( 0.f, radii.top_right ), ImMax( 0.f, radii.bottom_right ), ImMax( 0.f, radii.bottom_left ) };
        float scale = 1.f;
        if ( fitted.top_left + fitted.top_right > width )
            scale = ImMin( scale, width / ( fitted.top_left + fitted.top_right ) );
        if ( fitted.bottom_left + fitted.bottom_right > width )
            scale = ImMin( scale, width / ( fitted.bottom_left + fitted.bottom_right ) );
        if ( fitted.top_left + fitted.bottom_left > height )
            scale = ImMin( scale, height / ( fitted.top_left + fitted.bottom_left ) );
        if ( fitted.top_right + fitted.bottom_right > height )
            scale = ImMin( scale, height / ( fitted.top_right + fitted.bottom_right ) );
        fitted.top_left *= scale;
        fitted.top_right *= scale;
        fitted.bottom_right *= scale;
        fitted.bottom_left *= scale;
        return fitted;
    }

    void rounded_path( ImDrawList* draw, ImVec2 min, ImVec2 max, const corner_radii& radii )
    {
        const corner_radii fitted = fit_corner_radii( min, max, radii );
        if ( fitted.top_left > 0.f )
            draw->PathArcTo( ImVec2{ min.x + fitted.top_left, min.y + fitted.top_left }, fitted.top_left, IM_PI, IM_PI * 1.5f );
        else
            draw->PathLineTo( min );
        if ( fitted.top_right > 0.f )
            draw->PathArcTo( ImVec2{ max.x - fitted.top_right, min.y + fitted.top_right }, fitted.top_right, IM_PI * 1.5f, IM_PI * 2.f );
        else
            draw->PathLineTo( ImVec2{ max.x, min.y } );
        if ( fitted.bottom_right > 0.f )
            draw->PathArcTo( ImVec2{ max.x - fitted.bottom_right, max.y - fitted.bottom_right }, fitted.bottom_right, 0.f, IM_PI * 0.5f );
        else
            draw->PathLineTo( max );
        if ( fitted.bottom_left > 0.f )
            draw->PathArcTo( ImVec2{ min.x + fitted.bottom_left, max.y - fitted.bottom_left }, fitted.bottom_left, IM_PI * 0.5f, IM_PI );
        else
            draw->PathLineTo( ImVec2{ min.x, max.y } );
        ImVector< ImVec2 >& path = draw->_Path;
        int count = 0;
        for ( int index = 0; index < path.Size; ++index )
        {
            if ( count > 0 && ImLengthSqr( path[index] - path[count - 1] ) < 0.0004f )
                continue;
            path[count++] = path[index];
        }
        while ( count > 2 && ImLengthSqr( path[count - 1] - path[0] ) < 0.0004f )
            --count;
        path.resize( count );
    }

    void add_rounded_rect_filled( ImDrawList* draw, ImVec2 min, ImVec2 max, const corner_radii& radii, ImU32 color )
    {
        if ( max.x <= min.x || max.y <= min.y || ( color & IM_COL32_A_MASK ) == 0 )
            return;
        rounded_path( draw, min, max, radii );
        draw->PathFillConvex( color );
    }

    void add_rounded_rect_stroke( ImDrawList* draw, ImVec2 min, ImVec2 max, const corner_radii& radii, ImU32 color, float thickness )
    {
        if ( max.x < min.x || max.y < min.y || ( color & IM_COL32_A_MASK ) == 0 )
            return;
        rounded_path( draw, min, max, radii );
        draw->PathStroke( color, ImDrawFlags_Closed, thickness );
    }

    void add_border_rects( ImDrawList* draw, ImVec2 min, ImVec2 max, float top, float right, float bottom, float left, int align, ImU32 color )
    {
        const float outside = align == 2 ? 1.f : align == 1 ? 0.5f : 0.f;
        const ImVec2 outer_min{ min.x - left * outside, min.y - top * outside };
        const ImVec2 outer_max{ max.x + right * outside, max.y + bottom * outside };
        if ( top > 0.f )
            draw->AddRectFilled( outer_min, ImVec2{ outer_max.x, outer_min.y + top }, color );
        if ( bottom > 0.f )
            draw->AddRectFilled( ImVec2{ outer_min.x, outer_max.y - bottom }, outer_max, color );
        if ( left > 0.f )
            draw->AddRectFilled( ImVec2{ outer_min.x, outer_min.y + top }, ImVec2{ outer_min.x + left, outer_max.y - bottom }, color );
        if ( right > 0.f )
            draw->AddRectFilled( ImVec2{ outer_max.x - right, outer_min.y + top }, ImVec2{ outer_max.x, outer_max.y - bottom }, color );
    }

    void add_ellipse_filled( ImDrawList* draw, ImVec2 center, ImVec2 radius, ImU32 color )
    {
        if ( ( color & IM_COL32_A_MASK ) != 0 )
            draw->AddEllipseFilled( center, radius, color );
    }

    void add_ellipse_stroke( ImDrawList* draw, ImVec2 center, ImVec2 radius, ImU32 color, float thickness )
    {
        if ( ( color & IM_COL32_A_MASK ) != 0 )
            draw->AddEllipse( center, radius, color, 0.f, 0, thickness );
    }

    void erase_outside_corners( ImDrawList* draw, ImVec2 min, ImVec2 max, const corner_radii& radii )
    {
        const corner_radii fitted = fit_corner_radii( min, max, radii );
        const float corners[4][4] = {
            { min.x, min.y, fitted.top_left, IM_PI * 1.5f },
            { max.x, min.y, fitted.top_right, IM_PI * 2.f },
            { max.x, max.y, fitted.bottom_right, IM_PI * 0.5f },
            { min.x, max.y, fitted.bottom_left, IM_PI }
        };
        draw->AddCallback( begin_blend_mode, reinterpret_cast< void* >( std::intptr_t{ blend_erase } ) );
        for ( const auto& corner : corners )
        {
            const float radius = corner[2];
            if ( radius <= 0.f )
                continue;
            const ImVec2 point{ corner[0], corner[1] };
            const ImVec2 inward{ point.x == min.x ? 1.f : -1.f, point.y == min.y ? 1.f : -1.f };
            draw->PathLineTo( point );
            draw->PathArcTo( ImVec2{ point.x + inward.x * radius, point.y + inward.y * radius }, radius, corner[3], corner[3] - IM_PI * 0.5f );
            draw->PathFillConvex( IM_COL32_WHITE );
        }
        draw->AddCallback( ImDrawCallback_ResetRenderState, nullptr );
    }

    void begin_blend_mode( const ImDrawList*, const ImDrawCmd* command )
    {
        ImGui_ImplDX11_RenderState* state = render_state( );
        const int mode = static_cast< int >( reinterpret_cast< std::intptr_t >( command->UserCallbackData ) );
        if ( !state || !state->Device || !state->DeviceContext )
            return;
        if ( mode >= blend_multiply && mode <= blend_luminosity )
        {
            begin_backdrop_blend( state, mode - blend_multiply );
            return;
        }
        if ( mode <= 0 || mode >= IM_ARRAYSIZE( blend_states ) )
            return;
        if ( blend_device != state->Device )
        {
            for ( ID3D11BlendState*& blend : blend_states )
                release( blend );
            blend_device = state->Device;
        }
        if ( !blend_states[mode] )
        {
            D3D11_BLEND_DESC description{ };
            D3D11_RENDER_TARGET_BLEND_DESC& target = description.RenderTarget[0];
            target.BlendEnable = TRUE;
            target.SrcBlend = mode == blend_erase ? D3D11_BLEND_ZERO : D3D11_BLEND_SRC_ALPHA;
            target.DestBlend = mode == blend_erase ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_ONE;
            target.BlendOp = D3D11_BLEND_OP_ADD;
            target.SrcBlendAlpha = D3D11_BLEND_ZERO;
            target.DestBlendAlpha = mode == blend_erase ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_ONE;
            target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            if ( FAILED( state->Device->CreateBlendState( &description, &blend_states[mode] ) ) )
                return;
        }
        const float factor[4]{ };
        state->DeviceContext->OMSetBlendState( blend_states[mode], factor, 0xFFFFFFFF );
    }

    void draw_image( ImDrawList* draw, std::size_t index, ImVec2 min, ImVec2 max, ImVec2 uv_min, ImVec2 uv_max, const corner_radii& radii, ImU32 tint )
    {
        texture_handle texture{ };
        if ( ( tint & IM_COL32_A_MASK ) == 0 || max.x <= min.x || max.y <= min.y || !image_texture( index, texture ) )
            return;
        const corner_radii fitted = fit_corner_radii( min, max, radii );
        if ( fitted.top_left <= 0.f && fitted.top_right <= 0.f && fitted.bottom_right <= 0.f && fitted.bottom_left <= 0.f )
            draw->AddImage( texture, min, max, uv_min, uv_max, tint );
        else
            fill_rounded_texture( draw, texture, min, max, uv_min, uv_max, fitted, tint );
    }

    void draw_image_sliced( ImDrawList* draw, std::size_t index, ImVec2 min, ImVec2 max, float source_width, float cap, ImU32 tint )
    {
        const float width = max.x - min.x;
        if ( width <= 0.f || source_width <= 0.f )
            return;
        const float edge = ImMin( cap, width * 0.5f );
        const float u = edge / source_width;
        draw_image( draw, index, min, ImVec2{ min.x + edge, max.y }, ImVec2{ 0.f, 0.f }, ImVec2{ u, 1.f }, corner_radii{ }, tint );
        draw_image( draw, index, ImVec2{ min.x + edge, min.y }, ImVec2{ max.x - edge, max.y }, ImVec2{ u, 0.f }, ImVec2{ 1.f - u, 1.f }, corner_radii{ }, tint );
        draw_image( draw, index, ImVec2{ max.x - edge, min.y }, max, ImVec2{ 1.f - u, 0.f }, ImVec2{ 1.f, 1.f }, corner_radii{ }, tint );
    }

    child_region::child_region( ImDrawList*& draw, const char* id, ImVec2 position, ImVec2 size ) : draw_( draw ), parent_( draw )
    {
        ImGui::SetCursorScreenPos( position );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2{ 0.f, 0.f } );
        ImGui::PushStyleVar( ImGuiStyleVar_ChildBorderSize, 0.f );
        ImGui::PushStyleColor( ImGuiCol_ChildBg, IM_COL32( 0, 0, 0, 0 ) );
        ( void )ImGui::BeginChild( id, size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground );
        draw = ImGui::GetWindowDrawList( );
    }

    child_region::~child_region( )
    {
        ImGui::EndChild( );
        draw_ = parent_;
        ImGui::PopStyleColor( );
        ImGui::PopStyleVar( 2 );
    }
    void draw_background_blur( ImDrawList* draw, ImVec2 min, ImVec2 max, const corner_radii& radii, float sigma, float alpha )
    {
        if ( alpha <= 0.001f || draw_exact_blur( draw, min, max, radii, sigma, alpha ) )
            return;
        if ( alpha >= 0.5f )
            framework_blur( draw, min, max, fit_corner_radii( min, max, radii ) );
    }

    namespace
    {
        struct shader_call
        {
            shader_background* background;
            shader_parameters parameters;
        };

        bool ensure_shader( shader_background& background, ID3D11Device* device )
        {
            if ( background.device == device && background.shader && background.constants )
                return true;
            if ( background.device == device && background.failed )
                return false;
            release( background.shader );
            release( background.constants );
            background.device = device;
            background.failed = true;
            if ( !device || !background.source )
                return false;
            ID3DBlob* bytecode = nullptr;
            ID3DBlob* errors = nullptr;
            const HRESULT compiled = D3DCompile( background.source, std::strlen( background.source ), "lazyui_shader_background", nullptr, nullptr, "main", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &bytecode, &errors );
            if ( errors )
            {
                ::OutputDebugStringA( static_cast< const char* >( errors->GetBufferPointer( ) ) );
                release( errors );
            }
            if ( FAILED( compiled ) || !bytecode )
            {
                release( bytecode );
                return false;
            }
            const bool created = SUCCEEDED( device->CreatePixelShader( bytecode->GetBufferPointer( ), bytecode->GetBufferSize( ), nullptr, &background.shader ) );
            release( bytecode );
            D3D11_BUFFER_DESC buffer{ };
            buffer.ByteWidth = static_cast< UINT >( ( sizeof( shader_parameters ) + 15 ) / 16 * 16 );
            buffer.Usage = D3D11_USAGE_DYNAMIC;
            buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if ( !created || FAILED( device->CreateBuffer( &buffer, nullptr, &background.constants ) ) )
            {
                release( background.shader );
                release( background.constants );
                return false;
            }
            background.failed = false;
            return true;
        }

        void shader_begin( const ImDrawList*, const ImDrawCmd* command )
        {
            const auto* call = static_cast< const shader_call* >( command->UserCallbackData );
            ImGui_ImplDX11_RenderState* state = render_state( );
            if ( !call || !state || !ensure_shader( *call->background, state->Device ) )
                return;
            ID3D11DeviceContext* context = state->DeviceContext;
            D3D11_MAPPED_SUBRESOURCE mapped{ };
            if ( FAILED( context->Map( call->background->constants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) )
                return;
            std::memcpy( mapped.pData, &call->parameters, sizeof( shader_parameters ) );
            context->Unmap( call->background->constants, 0 );
            context->PSSetConstantBuffers( 0, 1, &call->background->constants );
            context->PSSetShader( call->background->shader, nullptr, 0 );
        }
    }

    void draw_shader( ImDrawList* draw, shader_background& background, ImVec2 min, ImVec2 max, const corner_radii& radii, float alpha )
    {
        const float opacity = ImSaturate( alpha * background.parameters.alpha * ImGui::GetStyle( ).Alpha );
        if ( opacity <= 0.001f || max.x - min.x < 1.f || max.y - min.y < 1.f || ( background.failed && background.device ) )
            return;
        const int frame = ImGui::GetFrameCount( );
        if ( background.frame != frame )
        {
            background.clock += ImGui::GetIO( ).DeltaTime * background.parameters.speed;
            background.frame = frame;
        }
        shader_call call{ &background, background.parameters };
        call.parameters.resolution[0] = max.x - min.x;
        call.parameters.resolution[1] = max.y - min.y;
        call.parameters.time = background.clock;
        call.parameters.alpha = 1.f;
        if ( background.interactive && pointer_inside( min, max ) )
        {
            call.parameters.mouse[0] = ( pointer.position.x - min.x ) / ( max.x - min.x );
            call.parameters.mouse[1] = 1.f - ( pointer.position.y - min.y ) / ( max.y - min.y );
        }
        else
            call.parameters.mouse[0] = call.parameters.mouse[1] = -1.f;
        draw->AddCallback( shader_begin, &call, sizeof( call ) );
        const int start = draw->VtxBuffer.Size;
        rounded_path( draw, min, max, fit_corner_radii( min, max, radii ) );
        draw->PathFillConvex( IM_COL32( 255, 255, 255, static_cast< int >( opacity * 255.f + 0.5f ) ) );
        ImGui::ShadeVertsLinearUV( draw, start, draw->VtxBuffer.Size, min, max, ImVec2{ 0.f, 0.f }, ImVec2{ 1.f, 1.f }, true );
        draw->AddCallback( ImDrawCallback_ResetRenderState, nullptr );
    }

    float animate( float current, float target, float speed )
    {
        const float next = current + ( target - current ) * ( 1.f - std::exp( -speed * ImGui::GetIO( ).DeltaTime ) );
        return ImAbs( target - next ) < 0.0005f ? target : next;
    }

    motion_settings motion = {
        {
            { 4.60517f / 16.f, curve_smooth },
            { 4.60517f / 20.f, curve_smooth },
            { 4.60517f / 30.f, curve_smooth },
            { 4.60517f / 18.f, curve_smooth },
            { 4.60517f / 14.f, curve_smooth },
            { 4.60517f / 12.f, curve_smooth },
            { 4.60517f / 14.f, curve_smooth }
        },
        transition_rise, 10.f, 12.f, intro_none, 0.42f
    };

    namespace
    {
        struct motion_trace
        {
            float from = 0.f;
            float to = 0.f;
            float elapsed = 0.f;
            float last = 0.f;
        };

        std::unordered_map< const float*, motion_trace >& motion_traces( )
        {
            static std::unordered_map< const float*, motion_trace > traces;
            return traces;
        }

        double intro_started = -1.0;
    }

    float motion_curve_at( int curve, float t )
    {
        const float x = ImClamp( t, 0.f, 1.f );
        switch ( curve )
        {
        case curve_linear:
            return x;
        case curve_snappy:
            return 1.f - std::pow( 1.f - x, 5.f );
        case curve_ease:
            return x < 0.5f ? 4.f * x * x * x : 1.f - std::pow( -2.f * x + 2.f, 3.f ) * 0.5f;
        case curve_spring:
            return x >= 1.f ? 1.f : 1.f - std::exp( -6.f * x ) * std::cos( 9.4f * x );
        default:
            return 1.f - std::exp( -4.605f * x );
        }
    }

    float cubic_bezier_at( const float ( &bezier )[4], float t )
    {
        if ( t <= 0.f )
            return 0.f;
        if ( t >= 1.f )
            return 1.f;
        const auto along = []( float s, float a, float b ) { return 3.f * ( 1.f - s ) * ( 1.f - s ) * s * a + 3.f * ( 1.f - s ) * s * s * b + s * s * s; };
        const auto slope = []( float s, float a, float b ) { return 3.f * ( 1.f - s ) * ( 1.f - s ) * a + 6.f * ( 1.f - s ) * s * ( b - a ) + 3.f * s * s * ( 1.f - b ); };
        float s = t;
        for ( int step = 0; step < 8; ++step )
        {
            const float error = along( s, bezier[0], bezier[2] ) - t;
            const float derivative = slope( s, bezier[0], bezier[2] );
            if ( std::fabs( error ) < 1e-6f || std::fabs( derivative ) < 1e-6f )
                break;
            s -= error / derivative;
        }
        if ( !( s >= 0.f && s <= 1.f ) || std::fabs( along( s, bezier[0], bezier[2] ) - t ) > 1e-4f )
        {
            float low = 0.f;
            float high = 1.f;
            s = t;
            for ( int step = 0; step < 30; ++step )
            {
                if ( along( s, bezier[0], bezier[2] ) < t )
                    low = s;
                else
                    high = s;
                s = ( low + high ) * 0.5f;
            }
        }
        return along( s, bezier[1], bezier[3] );
    }

    float animate_to( float& value, float target, int channel )
    {
        const motion_track& track = motion.channels[ImClamp( channel, 0, static_cast< int >( motion_channel_count ) - 1 )];
        if ( track.duration <= 0.f )
            return value = target;
        if ( track.curve == curve_smooth )
            return value = animate( value, target, 4.60517f / track.duration );
        motion_trace& trace = motion_traces( )[&value];
        if ( trace.to != target || trace.last != value )
            trace = motion_trace{ value, target, 0.f, value };
        trace.elapsed += ImGui::GetIO( ).DeltaTime;
        const float t = trace.elapsed / track.duration;
        const float progress = track.curve == curve_custom ? cubic_bezier_at( track.bezier, t ) : motion_curve_at( track.curve, t );
        value = t >= 1.f ? target : trace.from + ( target - trace.from ) * progress;
        trace.last = value;
        return value;
    }

    void scale_vertices( ImDrawList* draw, int start, ImVec2 center, float scale )
    {
        if ( scale == 1.f )
            return;
        for ( int index = start; index < draw->VtxBuffer.Size; ++index )
            draw->VtxBuffer[index].pos = center + ( draw->VtxBuffer[index].pos - center ) * scale;
    }

    bool play_intro( ImDrawList* draw, int start, ImVec2 center )
    {
        if ( motion.intro == intro_none || motion.intro_duration <= 0.f )
            return false;
        const double now = ImGui::GetTime( );
        if ( intro_started < 0.0 )
            intro_started = now;
        const float t = static_cast< float >( ( now - intro_started ) / motion.intro_duration );
        if ( t >= 1.f )
            return false;
        const float amount = motion_curve_at( curve_snappy, t );
        if ( motion.intro == intro_rise )
            for ( int index = start; index < draw->VtxBuffer.Size; ++index )
                draw->VtxBuffer[index].pos.y += ( 1.f - amount ) * 18.f;
        else if ( motion.intro == intro_scale )
            scale_vertices( draw, start, center, 0.94f + 0.06f * amount );
        fade_vertices( draw, start, amount );
        return true;
    }

    ImU32 mix_color( ImU32 from, ImU32 to, float t )
    {
        return ImGui::ColorConvertFloat4ToU32( ImLerp( ImGui::ColorConvertU32ToFloat4( from ), ImGui::ColorConvertU32ToFloat4( to ), ImSaturate( t ) ) );
    }

    ImU32 fade( ImU32 color, float alpha )
    {
        const ImU32 value = static_cast< ImU32 >( static_cast< float >( ( color >> IM_COL32_A_SHIFT ) & 0xFF ) * ImSaturate( alpha ) + 0.5f );
        return ( color & ~IM_COL32_A_MASK ) | ( value << IM_COL32_A_SHIFT );
    }

    void fade_vertices( ImDrawList* draw, int start, float alpha )
    {
        if ( alpha >= 1.f )
            return;
        for ( int index = start; index < draw->VtxBuffer.Size; ++index )
        {
            ImU32& color = draw->VtxBuffer[index].col;
            const ImU32 faded = static_cast< ImU32 >( static_cast< float >( ( color >> IM_COL32_A_SHIFT ) & 0xFF ) * ImSaturate( alpha ) + 0.5f );
            color = ( color & ~IM_COL32_A_MASK ) | ( faded << IM_COL32_A_SHIFT );
        }
    }

    void rotate_vertices( ImDrawList* draw, int start, ImVec2 center, float angle )
    {
        const float sine = ImSin( angle );
        const float cosine = ImCos( angle );
        for ( int index = start; index < draw->VtxBuffer.Size; ++index )
        {
            const ImVec2 delta = draw->VtxBuffer[index].pos - center;
            draw->VtxBuffer[index].pos = ImVec2{ center.x + delta.x * cosine - delta.y * sine, center.y + delta.x * sine + delta.y * cosine };
        }
    }

    static keybind_state* open_keybind_context = nullptr;
    static ImVec2 open_keybind_box_min{ };
    static ImVec2 open_keybind_box_max{ };

    void begin_pointer_frame( )
    {
        const ImGuiIO& io = ImGui::GetIO( );
        const bool over_list = pointer.popup_open && ImGui::IsMouseHoveringRect( pointer.popup_min, pointer.popup_max, false );
        pointer.position = io.MousePos;
        pointer.clicked = ImGui::IsMouseClicked( ImGuiMouseButton_Left );
        pointer.hovered = ImGui::IsWindowHovered( ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem ) && !over_list && !( pointer.popup_open && pointer.clicked );
        pointer.down = ImGui::IsMouseDown( ImGuiMouseButton_Left );
        pointer.released = ImGui::IsMouseReleased( ImGuiMouseButton_Left );
        pointer.popup_open = false;
        pointer.claimed = false;
        pointer.modal = false;
        if ( open_keybind_context && open_keybind_context->context_open )
        {
            pointer.modal = true;
            pointer.modal_min = ImVec2{ open_keybind_box_min.x, open_keybind_box_max.y + 2.f };
            pointer.modal_max = ImVec2{ pointer.modal_min.x + 120.f, pointer.modal_min.y + 66.f };
        }
    }

    bool pointer_inside( ImVec2 min, ImVec2 max )
    {
        return pointer.position.x >= min.x && pointer.position.y >= min.y && pointer.position.x < max.x && pointer.position.y < max.y;
    }

    bool pointer_over( ImVec2 min, ImVec2 max )
    {
        const bool over = pointer.hovered && pointer_inside( min, max ) && ( !pointer.modal || pointer_inside( pointer.modal_min, pointer.modal_max ) );
        pointer.claimed |= over;
        return over;
    }

    void update_window_drag( bool blocked, bool move_host )
    {
        const ImGuiIO& io = ImGui::GetIO( );
        HWND host = static_cast< HWND >( ImGui::GetMainViewport( )->PlatformHandleRaw );
        if ( !window_drag.dragging && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) && ImGui::IsWindowHovered( ImGuiHoveredFlags_ChildWindows ) && !blocked && !ImGui::IsAnyItemHovered( ) && !ImGui::IsAnyItemActive( ) )
        {
            window_drag.dragging = true;
            POINT cursor{ };
            RECT bounds{ };
            if ( move_host && host && ::GetCursorPos( &cursor ) && ::GetWindowRect( host, &bounds ) )
            {
                window_drag.anchor_x = cursor.x;
                window_drag.anchor_y = cursor.y;
                window_drag.start_x = bounds.left;
                window_drag.start_y = bounds.top;
            }
        }
        if ( !io.MouseDown[ImGuiMouseButton_Left] )
            window_drag.dragging = false;
        if ( !window_drag.dragging )
            return;
        if ( !move_host || !host )
        {
            window_drag.offset += io.MouseDelta;
            return;
        }
        POINT cursor{ };
        if ( ::GetCursorPos( &cursor ) )
            ( void )::SetWindowPos( host, nullptr, static_cast< int >( window_drag.start_x + cursor.x - window_drag.anchor_x ), static_cast< int >( window_drag.start_y + cursor.y - window_drag.anchor_y ), 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
    }

    void popup_trigger( popup_state& state, ImVec2 min, ImVec2 max )
    {
        const bool hovered = pointer.hovered && pointer_inside( min, max );
        if ( hovered )
        {
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
            pointer.claimed = true;
        }
        state.trigger_hit |= hovered && pointer.clicked;
    }

    void update_popup( popup_state& state, ImVec2 min, ImVec2 max )
    {
        const bool inside = pointer.hovered && pointer_inside( min, max );
        if ( state.trigger_hit )
            state.open = !state.open;
        else if ( state.open && ( ( pointer.clicked && pointer.hovered && !inside ) || ImGui::IsKeyPressed( ImGuiKey_Escape, false ) ) )
            state.open = false;
        state.trigger_hit = false;
        animate_to( state.anim, state.open ? 1.f : 0.f, motion_lists );
        if ( !state.open )
            return;
        pointer.modal = true;
        pointer.modal_min = min;
        pointer.modal_max = max;
        pointer.claimed |= pointer.hovered;
    }

    void popup_close( popup_state& state, ImVec2 min, ImVec2 max )
    {
        if ( !state.open || !pointer_over( min, max ) )
            return;
        ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
        if ( pointer.clicked )
            state.open = false;
    }

    void copy_text( char* target, std::size_t size, const char* text )
    {
        if ( size == 0 )
            return;
        std::size_t length = 0;
        if ( text )
            while ( text[length] && length + 1 < size )
            {
                target[length] = text[length];
                ++length;
            }
        target[length] = '\0';
    }

    bool same_name( const char* left, const char* right )
    {
        if ( !left || !right )
            return false;
        const auto trim = []( const char*& begin, const char*& end )
        {
            while ( begin < end && ( *begin == ' ' || *begin == '\t' ) )
                ++begin;
            while ( end > begin && ( end[-1] == ' ' || end[-1] == '\t' ) )
                --end;
        };
        const char* left_end = left + std::strlen( left );
        const char* right_end = right + std::strlen( right );
        trim( left, left_end );
        trim( right, right_end );
        if ( left_end - left != right_end - right )
            return false;
        for ( ; left < left_end; ++left, ++right )
            if ( std::tolower( static_cast< unsigned char >( *left ) ) != std::tolower( static_cast< unsigned char >( *right ) ) )
                return false;
        return true;
    }

    void update_switch( switch_state& state, ImVec2 min, ImVec2 max )
    {
        const bool hovered = pointer_over( min, max );
        if ( hovered )
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
        if ( hovered && pointer.clicked )
            state.value = !state.value;
        animate_to( state.anim, state.value ? 1.f : 0.f, motion_controls );
        animate_to( state.hover, hovered ? 1.f : 0.f, motion_hover );
    }

    void update_slider( slider_state& state, ImVec2 track_min, ImVec2 track_max, ImVec2 hit_min, ImVec2 hit_max )
    {
        const bool hovered = pointer_over( hit_min, hit_max );
        if ( hovered || state.active )
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
        if ( hovered && pointer.clicked )
            state.active = state.touched = true;
        if ( state.active && pointer.down )
            state.value = ImSaturate( ( pointer.position.x - track_min.x ) / ImMax( 1.f, track_max.x - track_min.x ) );
        if ( !pointer.down )
            state.active = false;
        if ( state.active )
            state.shown = animate( state.shown, state.value, 40.f );
        else
            animate_to( state.shown, state.value, motion_controls );
        animate_to( state.hover, hovered || state.active ? 1.f : 0.f, motion_hover );
    }

    void update_combo( combo_state& state, ImVec2 min, ImVec2 max, int options )
    {
        const bool hovered = pointer_over( min, max );
        if ( hovered )
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
        if ( pointer.clicked && hovered )
            state.open = !state.open;
        else if ( pointer.clicked && state.open && state.hot < 0 )
            state.open = false;
        if ( options <= 0 )
            state.open = false;
        animate_to( state.anim, state.open ? 1.f : 0.f, motion_lists );
        animate_to( state.hover, hovered ? 1.f : 0.f, motion_hover );
    }

    void update_color( color_state& state, ImVec2 min, ImVec2 max )
    {
        const bool hovered = pointer_over( min, max );
        if ( hovered )
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
        if ( pointer.clicked && hovered )
        {
            state.open = !state.open;
            pointer.clicked = false;
        }
        else if ( state.open && ( ( pointer.clicked && !state.over_popup ) || ImGui::IsKeyPressed( ImGuiKey_Escape, false ) ) )
            state.open = false;
        animate_to( state.anim, state.open ? 1.f : 0.f, motion_lists );
        animate_to( state.hover, hovered ? 1.f : 0.f, motion_hover );
    }

    void draw_color_popup( ImDrawList* draw, color_state& state, ImVec2 swatch_min, ImVec2 swatch_max, ImU32 fill, ImU32 stroke )
    {
        state.over_popup = false;
        if ( state.anim <= 0.001f )
        {
            state.dragging = 0;
            return;
        }
        constexpr float pad = 10.f;
        constexpr float square = 150.f;
        constexpr float bar = 12.f;
        constexpr float gap = 8.f;
        const ImVec2 size{ pad * 2.f + square + gap + bar, pad * 2.f + square + gap + bar };
        const ImVec2 display = ImGui::GetIO( ).DisplaySize;
        ImVec2 min{ ImMax( 4.f, swatch_max.x - size.x ), swatch_max.y + 6.f - ( 1.f - state.anim ) * 6.f };
        if ( min.y + size.y > display.y - 4.f )
            min.y = swatch_min.y - 6.f - size.y + ( 1.f - state.anim ) * 6.f;
        const ImVec2 max = min + size;
        pointer.popup_open = state.open;
        pointer.popup_min = min;
        pointer.popup_max = max;
        state.over_popup = ImGui::IsMouseHoveringRect( min, max, false );

        float hue = 0.f;
        float saturation = 0.f;
        float brightness = 0.f;
        ImGui::ColorConvertRGBtoHSV( state.value.x, state.value.y, state.value.z, hue, saturation, brightness );
        if ( saturation <= 0.0001f || brightness <= 0.0001f )
            hue = state.hue;
        const ImVec2 square_min = min + ImVec2{ pad, pad };
        const ImVec2 square_max = square_min + ImVec2{ square, square };
        const ImVec2 hue_min{ square_max.x + gap, square_min.y };
        const ImVec2 hue_max{ hue_min.x + bar, square_max.y };
        const ImVec2 alpha_min{ square_min.x, square_max.y + gap };
        const ImVec2 alpha_max{ hue_max.x, alpha_min.y + bar };

        if ( state.open && pointer.clicked )
            state.dragging = ImGui::IsMouseHoveringRect( square_min, square_max ) ? 1 : ImGui::IsMouseHoveringRect( hue_min, hue_max ) ? 2 : ImGui::IsMouseHoveringRect( alpha_min, alpha_max ) ? 3 : 0;
        if ( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) || !state.open )
            state.dragging = 0;
        const ImVec2 mouse = ImGui::GetIO( ).MousePos;
        if ( state.dragging == 1 )
        {
            saturation = ImSaturate( ( mouse.x - square_min.x ) / square );
            brightness = 1.f - ImSaturate( ( mouse.y - square_min.y ) / square );
        }
        else if ( state.dragging == 2 )
            hue = ImMin( ImSaturate( ( mouse.y - hue_min.y ) / square ), 0.9999f );
        else if ( state.dragging == 3 )
            state.value.w = ImSaturate( ( mouse.x - alpha_min.x ) / ( alpha_max.x - alpha_min.x ) );
        if ( state.dragging == 1 || state.dragging == 2 )
        {
            ImGui::ColorConvertHSVtoRGB( hue, saturation, brightness, state.value.x, state.value.y, state.value.z );
            state.hue = hue;
        }
        if ( state.over_popup || state.dragging )
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );

        const float shown = state.anim;
        const auto faded = [ shown ]( ImU32 color ) { return fade( color, shown ); };
        add_rounded_rect_filled( draw, min, max, corner_radii{ 8.f }, faded( fill ) );
        add_rounded_rect_stroke( draw, min + ImVec2{ 0.5f, 0.5f }, max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 7.5f }, faded( stroke ), 1.f );

        float red = 0.f;
        float green = 0.f;
        float blue = 0.f;
        ImGui::ColorConvertHSVtoRGB( hue, 1.f, 1.f, red, green, blue );
        const ImU32 pure = faded( ImGui::ColorConvertFloat4ToU32( ImVec4{ red, green, blue, 1.f } ) );
        const ImU32 white = faded( IM_COL32_WHITE );
        const ImU32 black = faded( IM_COL32_BLACK );
        draw->AddRectFilledMultiColor( square_min, square_max, white, pure, pure, white );
        draw->AddRectFilledMultiColor( square_min, square_max, IM_COL32( 0, 0, 0, 0 ), IM_COL32( 0, 0, 0, 0 ), black, black );

        static const ImU32 hues[] = { IM_COL32( 255, 0, 0, 255 ), IM_COL32( 255, 255, 0, 255 ), IM_COL32( 0, 255, 0, 255 ), IM_COL32( 0, 255, 255, 255 ), IM_COL32( 0, 0, 255, 255 ), IM_COL32( 255, 0, 255, 255 ), IM_COL32( 255, 0, 0, 255 ) };
        for ( int index = 0; index < 6; ++index )
        {
            const float top = hue_min.y + square * static_cast< float >( index ) / 6.f;
            const float bottom = hue_min.y + square * static_cast< float >( index + 1 ) / 6.f;
            draw->AddRectFilledMultiColor( ImVec2{ hue_min.x, top }, ImVec2{ hue_max.x, bottom }, faded( hues[index] ), faded( hues[index] ), faded( hues[index + 1] ), faded( hues[index + 1] ) );
        }

        const ImU32 light = faded( IM_COL32( 204, 204, 204, 255 ) );
        const ImU32 dark = faded( IM_COL32( 128, 128, 128, 255 ) );
        int column = 0;
        for ( float x = alpha_min.x; x < alpha_max.x; x += bar * 0.5f, ++column )
            for ( int row = 0; row < 2; ++row )
                draw->AddRectFilled( ImVec2{ x, alpha_min.y + bar * 0.5f * static_cast< float >( row ) }, ImVec2{ ImMin( x + bar * 0.5f, alpha_max.x ), alpha_min.y + bar * 0.5f * static_cast< float >( row + 1 ) }, ( column + row ) % 2 ? dark : light );
        const ImU32 clear = faded( ImGui::ColorConvertFloat4ToU32( ImVec4{ state.value.x, state.value.y, state.value.z, 0.f } ) );
        const ImU32 solid = faded( ImGui::ColorConvertFloat4ToU32( ImVec4{ state.value.x, state.value.y, state.value.z, 1.f } ) );
        draw->AddRectFilledMultiColor( alpha_min, alpha_max, clear, solid, solid, clear );

        const ImVec2 spot{ square_min.x + saturation * square, square_min.y + ( 1.f - brightness ) * square };
        draw->AddCircle( spot, 5.f, faded( IM_COL32_BLACK ), 16, 3.f );
        draw->AddCircle( spot, 5.f, white, 16, 1.5f );
        const float hue_y = hue_min.y + hue * square;
        draw->AddRect( ImVec2{ hue_min.x - 2.f, hue_y - 2.f }, ImVec2{ hue_max.x + 2.f, hue_y + 2.f }, white, 2.f, 0, 1.5f );
        const float alpha_x = alpha_min.x + state.value.w * ( alpha_max.x - alpha_min.x );
        draw->AddRect( ImVec2{ alpha_x - 2.f, alpha_min.y - 2.f }, ImVec2{ alpha_x + 2.f, alpha_max.y + 2.f }, white, 2.f, 0, 1.5f );
    }

    void update_keybind( keybind_state& state, ImVec2 min, ImVec2 max )
    {
        if ( state.context_open )
        {
            open_keybind_context = &state;
            open_keybind_box_min = min;
            open_keybind_box_max = max;
            const ImVec2 popup_min{ min.x, max.y + 2.f };
            const ImVec2 popup_max{ popup_min.x + 120.f, popup_min.y + 66.f };
            state.over_context = ImGui::IsMouseHoveringRect( popup_min, popup_max, false );
            if ( ImGui::IsKeyPressed( ImGuiKey_Escape, false ) || ( pointer.clicked && !state.over_context && !pointer_inside( min, max ) ) )
            {
                state.context_open = false;
                open_keybind_context = nullptr;
            }
            animate_to( state.anim, state.waiting ? 1.f : 0.f, motion_controls );
            animate_to( state.hover, 0.f, motion_hover );
            return;
        }

        const bool hovered = pointer_over( min, max );
        if ( hovered )
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
        if ( state.has_mode && !state.waiting && hovered && ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
        {
            if ( open_keybind_context && open_keybind_context != &state )
                open_keybind_context->context_open = false;
            state.context_open = true;
            open_keybind_context = &state;
            open_keybind_box_min = min;
            open_keybind_box_max = max;
            state.over_context = true;
            state.waiting = false;
            return;
        }
        if ( state.waiting )
        {
            for ( int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_ReservedForModCtrl; ++key )
            {
                if ( key == ImGuiKey_MouseLeft || key == ImGuiKey_MouseWheelX || key == ImGuiKey_MouseWheelY || !ImGui::IsKeyPressed( static_cast< ImGuiKey >( key ), false ) )
                    continue;
                if ( key != ImGuiKey_Escape )
                    state.key = key;
                state.waiting = false;
                break;
            }
            if ( pointer.clicked && !hovered )
                state.waiting = false;
        }
        else if ( hovered && pointer.clicked )
            state.waiting = true;
        animate_to( state.anim, state.waiting ? 1.f : 0.f, motion_controls );
        animate_to( state.hover, hovered ? 1.f : 0.f, motion_hover );
    }

    bool update_button( button_state& state, ImVec2 min, ImVec2 max )
    {
        const bool hovered = pointer_over( min, max );
        if ( hovered )
            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
        if ( hovered && pointer.clicked )
            state.pressed = true;
        state.clicked = state.pressed && pointer.released && hovered;
        if ( !pointer.down )
            state.pressed = false;
        animate_to( state.hover, hovered ? 1.f : 0.f, motion_hover );
        animate_to( state.press, state.pressed && hovered ? 1.f : 0.f, motion_press );
        return state.clicked;
    }

    namespace
    {
        bool text_input( void* id, char* buffer, std::size_t size, int slot, ImVec2 baseline, float width, ImU32 text, ImU32 selection, bool focus, ImGuiInputTextFlags flags )
        {
            ImFont* font = ui::slot_font( slot );
            if ( !font )
                return false;
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f } );
            ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 0.f );
            ImGui::PushStyleColor( ImGuiCol_FrameBg, IM_COL32( 0, 0, 0, 0 ) );
            ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, IM_COL32( 0, 0, 0, 0 ) );
            ImGui::PushStyleColor( ImGuiCol_FrameBgActive, IM_COL32( 0, 0, 0, 0 ) );
            ImGui::PushStyleColor( ImGuiCol_Text, text );
            ImGui::PushStyleColor( ImGuiCol_TextSelectedBg, selection );
#if IMGUI_VERSION_NUM >= 19200
            ImGui::PushFont( font, ui::slot_font_size( slot ) );
#else
            ImGui::PushFont( font );
#endif
            ImGui::SetCursorScreenPos( ImVec2{ baseline.x, IM_ROUND( baseline.y ) - ui::slot_font_ascent( slot ) } );
            if ( focus && !ImGui::IsAnyItemActive( ) )
                ImGui::SetKeyboardFocusHere( );
            ImGui::SetNextItemWidth( width );
            ImGui::PushID( id );
            ( void )ImGui::InputText( "##text", buffer, size, flags );
            const bool active = ImGui::IsItemActive( );
            ImGui::PopID( );
            ImGui::PopFont( );
            ImGui::PopStyleColor( 5 );
            ImGui::PopStyleVar( 2 );
            return active;
        }
    }

    void edit_search( search_state& state, int slot, ImVec2 baseline, float width, ImVec2 field_min, ImVec2 field_max, ImU32 text, ImU32 placeholder )
    {
        const bool hovered = pointer_over( field_min, field_max );
        if ( hovered )
            ImGui::SetMouseCursor( ImGuiMouseCursor_TextInput );
        const bool active = text_input( &state, state.buffer.data( ), state.buffer.size( ), slot, baseline, width, text, fade( placeholder, 0.35f ), hovered && pointer.clicked, 0 );
        animate_to( state.focus, active ? 1.f : 0.f, motion_controls );
        state.placeholder = animate( state.placeholder, state.buffer[0] == '\0' ? 1.f : 0.f, 24.f );
    }

    void edit_field( field_state& state, const char* initial, int slot, ImVec2 baseline, float width, ImVec2 field_min, ImVec2 field_max, ImU32 text, bool password )
    {
        if ( !state.ready )
        {
            ImStrncpy( state.buffer.data( ), initial, state.buffer.size( ) );
            state.ready = true;
        }
        const bool hovered = pointer_over( field_min, field_max );
        if ( hovered )
            ImGui::SetMouseCursor( ImGuiMouseCursor_TextInput );
        const bool activate = hovered && pointer.clicked;
        if ( activate )
            state.touched = true;
        const bool active = state.touched && text_input( &state, state.buffer.data( ), state.buffer.size( ), slot, baseline, width, text, fade( text, 0.25f ), activate, password ? ImGuiInputTextFlags_Password : 0 );
        animate_to( state.focus, active ? 1.f : 0.f, motion_controls );
        animate_to( state.hover, hovered ? 1.f : 0.f, motion_hover );
        state.placeholder = animate( state.placeholder, state.buffer[0] == '\0' ? 1.f : 0.f, 24.f );
    }

    const char* format_value( const char* format, float value )
    {
        static char buffer[32];
        ImFormatString( buffer, IM_ARRAYSIZE( buffer ), format, value );
        return buffer;
    }

    const char* short_key_name( int key )
    {
        static char label[32];
        if ( key >= ImGuiKey_MouseLeft && key <= ImGuiKey_MouseX2 )
        {
            ImFormatString( label, IM_ARRAYSIZE( label ), "M%d", key - ImGuiKey_MouseLeft + 1 );
            return label;
        }
        const char* name = ImGui::GetKeyName( static_cast< ImGuiKey >( key ) );
        if ( !name )
            return "";
        const bool arrow = std::strcmp( name, "LeftArrow" ) == 0 || std::strcmp( name, "RightArrow" ) == 0;
        if ( !arrow && std::strncmp( name, "Left", 4 ) == 0 && name[4] != '\0' )
            name += 4;
        else if ( !arrow && std::strncmp( name, "Right", 5 ) == 0 && name[5] != '\0' )
            name += 5;
        else if ( std::strncmp( name, "Keypad", 6 ) == 0 )
        {
            ImFormatString( label, IM_ARRAYSIZE( label ), "NUM%s", name + 6 );
            name = label;
        }
        if ( std::strcmp( name, "Ctrl" ) == 0 )
            name = "CTRL";
        else if ( std::strcmp( name, "Delete" ) == 0 )
            name = "DEL";
        else if ( std::strcmp( name, "Escape" ) == 0 )
            name = "ESC";
        else if ( std::strcmp( name, "Insert" ) == 0 )
            name = "INS";
        char upper[32]{ };
        int length = 0;
        for ( ; name[length] && length + 1 < IM_ARRAYSIZE( upper ); ++length )
            upper[length] = static_cast< char >( name[length] >= 'a' && name[length] <= 'z' ? name[length] - 32 : name[length] );
        ImStrncpy( label, upper, IM_ARRAYSIZE( label ) );
        return label;
    }

    void open_link( const char* url )
    {
        if ( !url || ( std::strncmp( url, "https://", 8 ) != 0 && std::strncmp( url, "http://", 7 ) != 0 ) )
            return;
        wchar_t wide[2048]{ };
        if ( ::MultiByteToWideChar( CP_UTF8, 0, url, -1, wide, IM_ARRAYSIZE( wide ) ) > 0 )
            ::ShellExecuteW( nullptr, L"open", wide, nullptr, nullptr, SW_SHOWNORMAL );
    }
}
