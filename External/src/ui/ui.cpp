#include "lazyui/lazyui.hpp"

#include "ui.hpp"
#include "pages.hpp"
#include "assets.hpp"
#include "text.hpp"
#include "theme.hpp"
#include "lazyui/runtime.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <cctype>
#include <iterator>

#include <settings.h>
#include <features/keybind/keybind.h>

namespace ui
{
    namespace controls
    {
        using namespace lazyui;

        switch_state silent_aim_enabled{ false, 0.f, 0.f };
        switch_state use_aimbot_target{ false, 0.f, 0.f };
        switch_state team_check{ false, 0.f, 0.f };
        switch_state gun_check{ false, 0.f, 0.f };
        switch_state knock_check{ true, 1.f, 0.f };
        switch_state health_check{ false, 0.f, 0.f };
        switch_state use_fov{ false, 0.f, 0.f };
        switch_state draw_fov{ false, 0.f, 0.f };
        switch_state draw_target_line{ false, 0.f, 0.f };
        switch_state enable_prediction{ false, 0.f, 0.f };
        selection_state< 11 > nav_main{ 2, 0.f, 188.71f };
        const ImVec4 nav_main_slots[11] = {
            ImVec4{ 21.29f, 172.4509f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 217.3609f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 278.f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 322.4839f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 382.4139f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 427.5009f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 487.5009f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 533.3109f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 591.84f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 638.4269f, 188.71f, 30.56f },
            ImVec4{ 21.29f, 698.4739f, 188.71f, 30.56f }
        };
        keybind_state aim_keybind{ };
        slider_state fov_slider{ };
        switch_state sticky_aim{ false, 0.f, 0.f };
        switch_state raycast_enabled{ false, 0.f, 0.f };
        button_state target_part_cycle{ };
    }

    namespace
    {
        using namespace lazyui;
        using namespace controls;

        bool target_part_open = false;
        bool aim_method_open = false;

        lazyui::slider_state prediction_x_slider{ };
        lazyui::slider_state prediction_y_slider{ };
        lazyui::color_state fov_circle_color{ };
        lazyui::color_state fov_outline_color{ };
        lazyui::color_state snap_line_color{ };

        const char* const k_part_names[6] = { "Head", "HumanoidRootPart", "LeftArm", "RightArm", "LeftLeg", "RightLeg" };

        constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;

        const char* target_part_name( int part )
        {
            switch ( part )
            {
            case 2: return "HumanoidRootPart";
            case 3: return "LeftArm";
            case 4: return "RightArm";
            case 5: return "LeftLeg";
            case 6: return "RightLeg";
            default: return "Head";
            }
        }

        int target_part_index( const char* text, int fallback )
        {
            char lower[64]{ };
            int length = 0;
            for ( ; text[length] != '\0' && length < 63; ++length )
                lower[length] = static_cast< char >( std::tolower( static_cast< unsigned char >( text[length] ) ) );
            if ( std::strcmp( lower, "head" ) == 0 )
                return 1;
            if ( std::strcmp( lower, "humanoidrootpart" ) == 0 || std::strcmp( lower, "root" ) == 0 || std::strcmp( lower, "torso" ) == 0 )
                return 2;
            if ( std::strcmp( lower, "leftarm" ) == 0 )
                return 3;
            if ( std::strcmp( lower, "rightarm" ) == 0 )
                return 4;
            if ( std::strcmp( lower, "leftleg" ) == 0 )
                return 5;
            if ( std::strcmp( lower, "rightleg" ) == 0 )
                return 6;
            return fallback;
        }

        void sync_settings( )
        {
            static bool initialised = false;
            if ( !initialised )
            {
                use_aimbot_target.value = settings::silentaim::use_aimbot_target;
                team_check.value = settings::silentaim::teamcheck;
                gun_check.value = settings::silentaim::guncheck;
                knock_check.value = settings::silentaim::knock_check;
                health_check.value = settings::silentaim::health_check_enabled;
                use_fov.value = settings::silentaim::use_fov;
                draw_fov.value = settings::silentaim::draw_fov;
                draw_target_line.value = settings::silentaim::draw_snap_line;
                enable_prediction.value = settings::silentaim::enable_prediction;
                silent_aim_enabled.value = settings::silentaim::enabled;
                sticky_aim.value = settings::silentaim::sticky_aim;
                sticky_aim.anim = sticky_aim.value ? 1.f : 0.f;
                raycast_enabled.value = settings::silentaim::raycast;
                raycast_enabled.anim = raycast_enabled.value ? 1.f : 0.f;
                for ( lazyui::switch_state* state : { &silent_aim_enabled, &use_aimbot_target, &team_check, &gun_check, &knock_check, &health_check, &use_fov, &draw_fov, &draw_target_line, &enable_prediction, &sticky_aim, &raycast_enabled } )
                    state->anim = state->value ? 1.f : 0.f;
                fov_slider.value = ImClamp( ( settings::silentaim::fov - 10.f ) / 490.f, 0.f, 1.f );
                fov_slider.shown = fov_slider.value;
                prediction_x_slider.value = ImClamp( settings::silentaim::prediction_x / 20.f, 0.f, 1.f );
                prediction_x_slider.shown = prediction_x_slider.value;
                prediction_y_slider.value = ImClamp( settings::silentaim::prediction_y / 20.f, 0.f, 1.f );
                prediction_y_slider.shown = prediction_y_slider.value;
                fov_circle_color.value = ImVec4{ settings::silentaim::fov_circle_colour[0], settings::silentaim::fov_circle_colour[1], settings::silentaim::fov_circle_colour[2], settings::silentaim::fov_circle_colour[3] };
                fov_outline_color.value = ImVec4{ settings::silentaim::fov_outline_colour[0], settings::silentaim::fov_outline_colour[1], settings::silentaim::fov_outline_colour[2], settings::silentaim::fov_outline_colour[3] };
                snap_line_color.value = ImVec4{ settings::silentaim::snap_line_color[0], settings::silentaim::snap_line_color[1], settings::silentaim::snap_line_color[2], settings::silentaim::snap_line_color[3] };
                aim_keybind.context_mode = settings::silentaim::activation_mode;
                aim_keybind.has_mode = true;
                aim_keybind.key = keybind::vk_to_imgui_key( settings::silentaim::keybind );
                if ( aim_keybind.key == 0 )
                {
                    aim_keybind.key = ImGuiKey_MouseLeft;
                    settings::silentaim::keybind = VK_LBUTTON;
                }
                initialised = true;
            }

            settings::silentaim::use_aimbot_target = use_aimbot_target.value;
            settings::silentaim::teamcheck = team_check.value;
            settings::silentaim::guncheck = gun_check.value;
            settings::silentaim::knock_check = knock_check.value;
            settings::silentaim::health_check_enabled = health_check.value;
            settings::silentaim::use_fov = use_fov.value;
            settings::silentaim::draw_fov = draw_fov.value;
            settings::silentaim::draw_snap_line = draw_target_line.value;
            settings::silentaim::enable_prediction = enable_prediction.value;
            settings::silentaim::enabled = silent_aim_enabled.value;


            settings::silentaim::sticky_aim = sticky_aim.value;
            settings::silentaim::raycast = raycast_enabled.value;
            settings::silentaim::fov = 10.f + fov_slider.value * 490.f;
            settings::silentaim::keybind = keybind::imgui_key_to_vk( static_cast< ImGuiKey >( aim_keybind.key ) );
            settings::silentaim::activation_mode = aim_keybind.context_mode;
            settings::silentaim::prediction_x = prediction_x_slider.value * 20.f;
            settings::silentaim::prediction_y = prediction_y_slider.value * 20.f;
            settings::silentaim::fov_circle_colour[0] = fov_circle_color.value.x;
            settings::silentaim::fov_circle_colour[1] = fov_circle_color.value.y;
            settings::silentaim::fov_circle_colour[2] = fov_circle_color.value.z;
            settings::silentaim::fov_circle_colour[3] = fov_circle_color.value.w;
            settings::silentaim::fov_outline_colour[0] = fov_outline_color.value.x;
            settings::silentaim::fov_outline_colour[1] = fov_outline_color.value.y;
            settings::silentaim::fov_outline_colour[2] = fov_outline_color.value.z;
            settings::silentaim::fov_outline_colour[3] = fov_outline_color.value.w;
            settings::silentaim::snap_line_color[0] = snap_line_color.value.x;
            settings::silentaim::snap_line_color[1] = snap_line_color.value.y;
            settings::silentaim::snap_line_color[2] = snap_line_color.value.z;
            settings::silentaim::snap_line_color[3] = snap_line_color.value.w;
        }

        struct embedded_image
        {
            const unsigned char* data;
            std::size_t size;
        };

        const embedded_image image_sources[] = {
            { assets::picture_detail_bytes, assets::picture_detail_bytes_size },
            { assets::red_emblem_bytes, assets::red_emblem_bytes_size },
            { assets::picture_detail_2_bytes, assets::picture_detail_2_bytes_size },
            { assets::picture_detail_3_bytes, assets::picture_detail_3_bytes_size },
            { assets::enable_silent_aim_checkbox_bytes, assets::enable_silent_aim_checkbox_bytes_size },
            { assets::picture_detail_4_bytes, assets::picture_detail_4_bytes_size },
            { assets::picture_detail_5_bytes, assets::picture_detail_5_bytes_size },
            { assets::picture_detail_6_bytes, assets::picture_detail_6_bytes_size },
            { assets::target_selection_section_heading_bytes, assets::target_selection_section_heading_bytes_size },
            { assets::picture_detail_7_bytes, assets::picture_detail_7_bytes_size },
            { assets::picture_detail_8_bytes, assets::picture_detail_8_bytes_size },
            { assets::picture_detail_9_bytes, assets::picture_detail_9_bytes_size },
            { assets::picture_detail_10_bytes, assets::picture_detail_10_bytes_size },
            { assets::picture_detail_11_bytes, assets::picture_detail_11_bytes_size },
            { assets::target_part_dropdown_gradient_bytes, assets::target_part_dropdown_gradient_bytes_size },
            { assets::target_part_dropdown_arrow_bytes, assets::target_part_dropdown_arrow_bytes_size },
            { assets::picture_detail_12_bytes, assets::picture_detail_12_bytes_size },
            { assets::fov_slider_filled_part_bytes, assets::fov_slider_filled_part_bytes_size },
            { assets::picture_detail_13_bytes, assets::picture_detail_13_bytes_size },
            { assets::sticky_aim_checkbox_bytes, assets::sticky_aim_checkbox_bytes_size },
            { assets::picture_detail_14_bytes, assets::picture_detail_14_bytes_size },
            { assets::picture_detail_15_bytes, assets::picture_detail_15_bytes_size },
            { assets::picture_detail_16_bytes, assets::picture_detail_16_bytes_size }
        };

        std::array< c_image, std::size( image_sources ) > images;

        c_blureffect framework_blur_effect;

        void load_images( )
        {
            for ( std::size_t index = 0; index < images.size( ); ++index )
                if ( !images[index].ready( ) )
                    ( void )images[index].load( lazyui::app.device, image_sources[index].data, image_sources[index].size );
        }

        bool ensure_framework_blur( )
        {
            if ( framework_blur_effect.ready( ) && framework_blur_effect.matches_display( ImGui::GetIO( ).DisplaySize ) )
                return true;
            return framework_blur_effect.setup( lazyui::app.device, lazyui::app.device_context, lazyui::app.swap_chain );
        }

        const motion_settings design_motion = {
            {
                { 0.389f, curve_spring },
                { 0.23f, curve_smooth },
                { 0.154f, curve_smooth },
                { 0.346f, curve_spring },
                { 0.444f, curve_spring },
                { 0.518f, curve_spring },
                { 0.444f, curve_spring }
            },
            transition_scale, 14.f, 18.f, intro_scale, 0.52f
        };

        const char shader_source_0[] = R"lazyui_hlsl(
        cbuffer lazyui_shader_parameters : register( b0 )
        {
            float2 uResolution; float uTime; float uSpeed;
            float3 uColor1; float uScale;
            float3 uColor2; float uIntensity;
            float3 uColor3; float uAmount;
            float2 uMouse; float uDetail; float uAlpha;
        };
        #define vec2 float2
        #define vec3 float3
        #define vec4 float4
        #define mix lerp
        #define fract frac
        float mod( float x, float y ) { return x - y * floor( x / y ); }
        float2 mod( float2 x, float2 y ) { return x - y * floor( x / y ); }
        float2 mod( float2 x, float y ) { return x - y * floor( x / y ); }
        float3 mod( float3 x, float3 y ) { return x - y * floor( x / y ); }
        float3 mod( float3 x, float y ) { return x - y * floor( x / y ); }

        vec2 splat2( float v ) { return vec2( v, v ); }
        vec3 splat3( float v ) { return vec3( v, v, v ); }
        vec4 splat4( float v ) { return vec4( v, v, v, v ); }
        vec2 rot2( vec2 p, float a ) { float c = cos( a ); float s = sin( a ); return vec2( c * p.x - s * p.y, s * p.x + c * p.y ); }
        float hash21( vec2 p ) { p = fract( p * vec2( 123.34, 456.21 ) ); p += dot( p, p + 45.32 ); return fract( p.x * p.y ); }
        vec2 hash22( vec2 p ) { float n = hash21( p ); return vec2( n, hash21( p + n ) ); }
        float noise2( vec2 p ) {
          vec2 i = floor( p );
          vec2 f = fract( p );
          vec2 u = f * f * ( 3.0 - 2.0 * f );
          float a = hash21( i );
          float b = hash21( i + vec2( 1.0, 0.0 ) );
          float c = hash21( i + vec2( 0.0, 1.0 ) );
          float d = hash21( i + vec2( 1.0, 1.0 ) );
          return mix( mix( a, b, u.x ), mix( c, d, u.x ), u.y );
        }
        float fbm( vec2 p ) {
          float value = 0.0;
          float amplitude = 0.5;
          for ( int i = 0; i < 5; i++ ) {
            value += amplitude * noise2( p );
            p = rot2( p, 0.5 ) * 2.03 + vec2( 17.0, 9.0 );
            amplitude *= 0.5;
          }
          return value;
        }
        vec3 palette3( float t ) {
          t = fract( t );
          vec3 a = mix( uColor1, uColor2, smoothstep( 0.0, 0.5, t ) );
          return mix( a, uColor3, smoothstep( 0.5, 1.0, t ) );
        }
        float luma( vec3 c ) { return dot( c, vec3( 0.299, 0.587, 0.114 ) ); }
        vec4 lightOver( vec3 c ) {
          c = max( c, splat3( 0.0 ) );
          float a = clamp( max( max( c.r, c.g ), c.b ), 0.0, 1.0 );
          return vec4( c / max( a, 0.0001 ), a );
        }
        float grain( vec2 fragCoord ) { return hash21( floor( fragCoord ) + fract( uTime * 7.13 ) * 91.7 ) - 0.5; }

        vec4 shade( vec2 fragCoord ) {
          vec2 p = ( fragCoord - 0.5 * uResolution ) / uResolution.y * 2.4 * uScale;
          if ( uMouse.x >= 0.0 )
            p += ( ( uMouse - 0.5 ) * 0.6 ) * exp( -length( fragCoord / uResolution - uMouse ) * 3.0 );
          float t = uTime * 0.3;
          for ( int i = 1; i < 6; i++ ) {
            float fi = float( i );
            p.x += 0.55 * uAmount / fi * sin( fi * p.y * 1.3 + t + fi * 0.7 );
            p.y += 0.45 * uAmount / fi * cos( fi * p.x * 1.1 - t * 0.8 + fi * 1.9 );
          }
          float bands = 0.5 + 0.5 * sin( p.x * 2.2 + p.y * 1.3 );
          float highlight = pow( bands, 14.0 );
          float body = smoothstep( 0.0, 1.0, bands );
          vec3 color = mix( uColor2, uColor1, body ) * uIntensity + splat3( highlight * 0.85 * uIntensity );
          return vec4( color, 1.0 );
        }
        struct PS_INPUT
        {
            float4 pos : SV_POSITION;
            float4 col : COLOR0;
            float2 uv : TEXCOORD0;
        };
        float4 main( PS_INPUT input ) : SV_Target
        {
            float4 color = shade( float2( input.uv.x * uResolution.x, ( 1.0 - input.uv.y ) * uResolution.y ) );
            return float4( saturate( color.rgb ), saturate( color.a ) * uAlpha * input.col.a );
        }
        )lazyui_hlsl";

        std::array< shader_background, 1 > shader_backgrounds{ {
            shader_background{ shader_source_0, { { 0.f, 0.f }, 0.f, 0.f, { 0.9765f, 0.2706f, 0.1961f }, 0.25f, { 0.0824f, 0.0824f, 0.0824f }, 0.f, { 0.6039f, 0.6431f, 0.7216f }, 0.2f, { -1.f, -1.f }, 1.f, 0.05f }, true }
        } };

        void update_controls( ImVec2 origin )
        {
            begin_pointer_frame( );

            const ImVec2 part_box_min = origin + ImVec2{ 254.f, 192.f };
            const ImVec2 part_box_max = origin + ImVec2{ 511.f, 240.f };
            const ImVec2 part_popup_min{ part_box_min.x, part_box_max.y };
            const ImVec2 part_popup_max{ part_box_max.x, part_box_max.y + 6.f * 22.f };
            const ImVec2 method_box_min = origin + ImVec2{ 541.f, 271.f };
            const ImVec2 method_box_max = origin + ImVec2{ 740.f, 295.f };
            const ImVec2 method_popup_min{ method_box_min.x, method_box_max.y };
            const ImVec2 method_popup_max{ method_box_max.x, method_box_max.y + 2.f * 22.f };

            if ( nav_main.selected == 2 && aim_method_open )
            {
                pointer.modal = true;
                pointer.modal_min = method_box_min;
                pointer.modal_max = method_popup_max;
            }

            if ( nav_main.selected == 2 && target_part_open )
            {
                pointer.modal = true;
                pointer.modal_min = part_box_min;
                pointer.modal_max = part_popup_max;
            }

            update_selection( nav_main, nav_main_slots, origin, true, 278.f, 188.71f );

            if ( nav_main.selected == 2 )
            {
                update_switch( enable_prediction, origin + ImVec2{ 833.f, 57.725f }, origin + ImVec2{ 978.4125f, 81.725f } );
                update_switch( silent_aim_enabled, origin + ImVec2{ 249.f, 56.725f }, origin + ImVec2{ 275.f, 82.f } );
                update_switch( use_aimbot_target, origin + ImVec2{ 248.f, 245.855f }, origin + ImVec2{ 405.2559f, 269.855f } );
                update_switch( team_check, origin + ImVec2{ 249.f, 315.725f }, origin + ImVec2{ 360.8488f, 339.725f } );
                update_switch( gun_check, origin + ImVec2{ 249.f, 345.055f }, origin + ImVec2{ 352.108f, 369.055f } );
                update_switch( knock_check, origin + ImVec2{ 249.f, 375.738f }, origin + ImVec2{ 365.2666f, 399.738f } );
                update_switch( health_check, origin + ImVec2{ 249.f, 405.725f }, origin + ImVec2{ 367.4978f, 429.725f } );
                update_switch( use_fov, origin + ImVec2{ 541.f, 103.7309f }, origin + ImVec2{ 631.5326f, 127.7309f } );
                update_switch( draw_fov, origin + ImVec2{ 540.25f, 133.7309f }, origin + ImVec2{ 638.4219f, 157.7309f } );
                update_switch( draw_target_line, origin + ImVec2{ 541.f, 233.618f }, origin + ImVec2{ 684.9698f, 257.618f } );
                update_keybind( aim_keybind, origin + ImVec2{ 400.f, 59.f }, origin + ImVec2{ 466.f, 81.f } );
                update_switch( sticky_aim, origin + ImVec2{ 541.f, 203.f }, origin + ImVec2{ 570.f, 230.f } );
                update_slider( fov_slider, origin + ImVec2{ 576.39f, 88.65f }, origin + ImVec2{ 803.f, 88.65f }, origin + ImVec2{ 540.f, 78.f }, origin + ImVec2{ 812.f, 100.f } );
                if ( draw_fov.value )
                {
                    update_color( fov_circle_color, origin + ImVec2{ 700.f, 134.f }, origin + ImVec2{ 718.f, 152.f } );
                    update_color( fov_outline_color, origin + ImVec2{ 724.f, 134.f }, origin + ImVec2{ 742.f, 152.f } );
                }
                if ( draw_target_line.value )
                    update_color( snap_line_color, origin + ImVec2{ 700.f, 239.f }, origin + ImVec2{ 718.f, 257.f } );
                if ( enable_prediction.value )
                {
                    update_slider( prediction_x_slider, origin + ImVec2{ 835.f, 121.f }, origin + ImVec2{ 1010.f, 125.f }, origin + ImVec2{ 830.f, 115.f }, origin + ImVec2{ 1015.f, 131.f } );
                    update_slider( prediction_y_slider, origin + ImVec2{ 835.f, 159.f }, origin + ImVec2{ 1010.f, 163.f }, origin + ImVec2{ 830.f, 153.f }, origin + ImVec2{ 1015.f, 169.f } );
                }

                if ( pointer_over( part_box_min, part_box_max ) )
                {
                    ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
                    if ( pointer.clicked )
                    {
                        target_part_open = !target_part_open;
                        aim_method_open = false;
                    }
                }

                if ( target_part_open )
                {
                    for ( int index = 0; index < 6; ++index )
                    {
                        const ImVec2 row_min{ part_popup_min.x, part_popup_min.y + index * 22.f };
                        const ImVec2 row_max{ part_popup_max.x, row_min.y + 22.f };
                        if ( pointer_over( row_min, row_max ) )
                        {
                            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
                            if ( pointer.clicked )
                            {
                                settings::silentaim::target_part = index + 1;
                                target_part_open = false;
                            }
                        }
                    }

                    if ( pointer.clicked && !pointer_inside( part_box_min, part_popup_max ) )
                        target_part_open = false;
                }

                if ( pointer_over( method_box_min, method_box_max ) )
                {
                    ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
                    if ( pointer.clicked )
                    {
                        aim_method_open = !aim_method_open;
                        target_part_open = false;
                    }
                }

                if ( aim_method_open )
                {
                    for ( int index = 0; index < 2; ++index )
                    {
                        const ImVec2 row_min{ method_popup_min.x, method_popup_min.y + index * 22.f };
                        const ImVec2 row_max{ method_popup_max.x, row_min.y + 22.f };
                        if ( pointer_over( row_min, row_max ) )
                        {
                            ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
                            if ( pointer.clicked )
                            {
                                raycast_enabled.value = ( index == 1 );
                                aim_method_open = false;
                            }
                        }
                    }

                    if ( pointer.clicked && !pointer_inside( method_box_min, method_popup_max ) )
                        aim_method_open = false;
                }
            }
        }

        void draw_nav_main_item( ImDrawList* draw, ImVec2 at, int index, glyph_text label )
        {
            add_rounded_rect_filled( draw, at, at + ImVec2{ 188.71f, 30.56f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[index], nav_main.hover[index] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_17_5, at + ImVec2{ 65.66f, 5.0261f }, selection_tint( nav_main, index, theme::text_faint, theme::text_muted, theme::text_5 ), label );
        }

        void draw_checkbox( ImDrawList* draw, ImVec2 at, const switch_state& state, float page_alpha )
        {
            add_rounded_rect_filled( draw, at + ImVec2{ 4.f, 3.275f }, at + ImVec2{ 22.f, 21.275f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, state.anim ), theme::fill_red_2, state.hover * ( 1.f - state.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, at + ImVec2{ 4.5f, 3.775f }, at + ImVec2{ 21.5f, 20.775f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, state.anim ), page_alpha ), 1.f );
        }

        void draw_picture_detail( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_0 = draw->VtxBuffer.Size;
            draw_image( draw, 0, origin + ImVec2{ 1139.f, 0.f }, origin + ImVec2{ 1149.f, 19.f }, ImVec2{ 0.f, 0.f }, ImVec2{ 1.f, 1.f }, corner_radii{ 0.f, 7.f, 0.f, 0.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_0, origin + ImVec2{ 1144.f, 9.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_sidebar( ImDrawList* draw, ImVec2 origin )
        {
            add_rounded_rect_filled( draw, origin, origin + ImVec2{ 225.f, 748.f }, corner_radii{ 7.f, 0.f, 0.f, 7.f }, theme::background );
            add_rounded_rect_filled( draw, origin + ImVec2{ 224.f, 0.f }, origin + ImVec2{ 225.f, 748.f }, corner_radii{ }, theme::surface_2 );
            draw_image( draw, 1, origin + ImVec2{ 53.f, 26.f }, origin + ImVec2{ 172.f, 137.f } );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 278.f }, origin + ImVec2{ 210.f, 308.56f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[2], nav_main.hover[2] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_700_16, origin + ImVec2{ 83.32f, 283.67f }, mix_color( theme::text_faint, theme::text_muted, nav_main.amount[2] ), text::silent_aim );
            draw_image( draw, 2, origin + ImVec2{ 12.f, 274.f }, origin + ImVec2{ 23.f, 311.f } );
            add_rounded_rect_filled( draw, origin + ImVec2{ 16.f, 271.f }, origin + ImVec2{ 210.f, 272.f }, corner_radii{ }, theme::surface );
            add_rounded_rect_filled( draw, origin + ImVec2{ 16.f, 376.f }, origin + ImVec2{ 209.f, 377.f }, corner_radii{ }, theme::surface );
            add_rounded_rect_filled( draw, origin + ImVec2{ 16.5f, 482.f }, origin + ImVec2{ 210.5f, 483.f }, corner_radii{ }, theme::surface_3 );
            add_rounded_rect_filled( draw, origin + ImVec2{ 16.87f, 587.f }, origin + ImVec2{ 209.87f, 588.f }, corner_radii{ }, theme::surface_4 );
            add_rounded_rect_filled( draw, origin + ImVec2{ 17.f, 692.f }, origin + ImVec2{ 210.f, 693.f }, corner_radii{ }, theme::surface );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 172.4509f }, origin + ImVec2{ 210.f, 203.0109f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[0], nav_main.hover[0] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 86.95f, 177.77f }, selection_tint( nav_main, 0, theme::text_faint, theme::text_muted, theme::text_5 ), text::aimbot );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 217.3609f }, origin + ImVec2{ 210.f, 247.9209f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[1], nav_main.hover[1] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 87.55f, 222.68f }, selection_tint( nav_main, 1, theme::text_faint, theme::text_muted, theme::text_5 ), text::trigger );
            draw_nav_main_item( draw, origin + ImVec2{ 21.29f, 322.4839f }, 3, text::visuals );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 382.4139f }, origin + ImVec2{ 210.f, 412.9739f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[4], nav_main.hover[4] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_17_5, origin + ImVec2{ 93.81f, 387.44f }, selection_tint( nav_main, 4, theme::text_faint, theme::text_muted, theme::text_5 ), text::rage );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 427.5009f }, origin + ImVec2{ 210.f, 458.0609f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[5], nav_main.hover[5] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 89.03f, 432.82f }, selection_tint( nav_main, 5, theme::text_faint, theme::text_muted, theme::text_5 ), text::player );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 487.5009f }, origin + ImVec2{ 210.f, 518.0609f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[6], nav_main.hover[6] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 67.6f, 492.82f }, selection_tint( nav_main, 6, theme::text_faint, theme::text_muted, theme::text_5 ), text::local_players );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 533.3109f }, origin + ImVec2{ 210.f, 563.8709f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[7], nav_main.hover[7] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 84.03f, 538.63f }, selection_tint( nav_main, 7, theme::text_faint, theme::text_muted, theme::text_5 ), text::settings );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 591.84f }, origin + ImVec2{ 210.f, 622.4f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[8], nav_main.hover[8] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::outfit_thin_600_17, origin + ImVec2{ 89.08f, 596.41f }, selection_tint( nav_main, 8, theme::text_faint, theme::text_muted, theme::text_5 ), text::config );
            add_rounded_rect_filled( draw, origin + ImVec2{ 21.29f, 638.4269f }, origin + ImVec2{ 210.f, 668.9869f }, corner_radii{ 3.52f }, fade( theme::fill_red, ImMax( nav_main.amount[9], nav_main.hover[9] * 0.4f ) ) );
            draw_glyph_run( draw, text_fonts::roboto_600_18, origin + ImVec2{ 98.75f, 643.16f }, selection_tint( nav_main, 9, theme::text_faint, theme::text_muted, theme::text_5 ), text::dex );
            draw_nav_main_item( draw, origin + ImVec2{ 21.29f, 698.4739f }, 10, text::colors );
        }

        void draw_picture_detail_2( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_1 = draw->VtxBuffer.Size;
            draw_image( draw, 3, origin + ImVec2{ 223.f, 736.f }, origin + ImVec2{ 1149.f, 748.f }, ImVec2{ 0.f, 0.f }, ImVec2{ 1.f, 1.f }, corner_radii{ 0.f, 0.f, 7.f, 0.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_1, origin + ImVec2{ 686.f, 742.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_activation_section_heading( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_2 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_700_18, origin + ImVec2{ 251.57f, 23.8f }, fade( theme::text_red, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::activation );
            scale_vertices( draw, scale_start_2, origin + ImVec2{ 296.7788f, 34.3469f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_activation_heading_divider( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_3 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 252.4f, 51.f }, origin + ImVec2{ 510.4f, 52.f }, corner_radii{ }, fade( theme::surface_2, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_3, origin + ImVec2{ 381.4f, 51.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_enable_silent_aim_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            draw_checkbox( draw, origin + ImVec2{ 249.f, 56.725f }, silent_aim_enabled, page_alpha );
        }

        void draw_picture_detail_3( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_5 = draw->VtxBuffer.Size;
            draw_image( draw, 5, origin + ImVec2{ 255.f, 57.f }, origin + ImVec2{ 271.f, 62.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_5, origin + ImVec2{ 263.f, 59.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_4( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_6 = draw->VtxBuffer.Size;
            draw_image( draw, 6, origin + ImVec2{ 250.f, 60.f }, origin + ImVec2{ 255.f, 77.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_6, origin + ImVec2{ 252.5f, 68.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_5( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_7 = draw->VtxBuffer.Size;
            draw_image( draw, 7, origin + ImVec2{ 253.f, 60.f }, origin + ImVec2{ 276.f, 81.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_7, origin + ImVec2{ 264.5f, 70.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_enable_silent_aim_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_8 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::inter_600_15_5, origin + ImVec2{ 276.42f, 60.29f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::enable_silent_aim );
            scale_vertices( draw, scale_start_8, origin + ImVec2{ 333.1287f, 69.6672f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_silent_aim_keybind( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_9 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 400.f, 59.f }, origin + ImVec2{ 466.f, 81.f }, corner_radii{ 3.52f }, fade( mix_color( mix_color( theme::fill_blue, theme::surface_5, aim_keybind.hover ), theme::fill_red, aim_keybind.waiting ? 0.85f : 0.f ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 400.5f, 59.5f }, origin + ImVec2{ 465.5f, 80.5f }, corner_radii{ 3.02f }, fade( mix_color( theme::border_2, theme::accent, aim_keybind.waiting ? 1.f : 0.f ), page_alpha ), 1.f );
            const char* key_text = aim_keybind.waiting ? "..." : short_key_name( aim_keybind.key );
            draw_text_line( draw, text_fonts::roboto_600_15_5, origin + ImVec2{ 433.f, 74.5f }, 1, fade( theme::text, page_alpha ), key_text );
            scale_vertices( draw, scale_start_9, origin + ImVec2{ 433.f, 70.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_prediction_sliders( ImDrawList* draw, ImVec2 origin )
        {
            if ( !enable_prediction.value )
                return;

            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            for ( int axis = 0; axis < 2; ++axis )
            {
                const lazyui::slider_state& state = axis == 0 ? prediction_x_slider : prediction_y_slider;
                const float y = 104.f + axis * 38.f;
                const float value = axis == 0 ? settings::silentaim::prediction_x : settings::silentaim::prediction_y;
                draw_text_line( draw, text_fonts::roboto_600_16, origin + ImVec2{ 835.f, y }, 0, fade( theme::text_muted, page_alpha ), axis == 0 ? "Prediction X" : "Prediction Y" );
                draw_text_line( draw, text_fonts::roboto_600_16, origin + ImVec2{ 1010.f, y }, 2, fade( theme::text, page_alpha ), format_value( "%.1f", value ) );
                add_rounded_rect_filled( draw, origin + ImVec2{ 835.f, y + 17.f }, origin + ImVec2{ 1010.f, y + 21.f }, corner_radii{ 2.f }, fade( theme::surface_5, page_alpha ) );
                add_rounded_rect_filled( draw, origin + ImVec2{ 835.f, y + 17.f }, origin + ImVec2{ 835.f + 175.f * state.value, y + 21.f }, corner_radii{ 2.f }, fade( theme::accent, page_alpha ) );
                add_rounded_rect_filled( draw, origin + ImVec2{ 832.f + 175.f * state.value, y + 14.f }, origin + ImVec2{ 838.f + 175.f * state.value, y + 24.f }, corner_radii{ 3.f }, fade( theme::text, page_alpha ) );
            }
        }

        void draw_color_swatch_box( ImDrawList* draw, ImVec2 min, ImVec2 max, const lazyui::color_state& state, float alpha )
        {
            add_rounded_rect_filled( draw, min, max, corner_radii{ 3.f }, fade( ImGui::ColorConvertFloat4ToU32( state.value ), alpha ) );
            add_rounded_rect_stroke( draw, min + ImVec2{ 0.5f, 0.5f }, max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 2.5f }, fade( mix_color( theme::border, theme::accent, state.hover ), alpha ), 1.f );
        }

        void draw_aim_color_swatches( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            if ( draw_fov.value )
            {
                draw_color_swatch_box( draw, origin + ImVec2{ 700.f, 134.f }, origin + ImVec2{ 718.f, 152.f }, fov_circle_color, page_alpha );
                draw_color_swatch_box( draw, origin + ImVec2{ 724.f, 134.f }, origin + ImVec2{ 742.f, 152.f }, fov_outline_color, page_alpha );
            }
            if ( draw_target_line.value )
                draw_color_swatch_box( draw, origin + ImVec2{ 700.f, 239.f }, origin + ImVec2{ 718.f, 257.f }, snap_line_color, page_alpha );
        }

        void draw_aim_color_popups( ImDrawList* draw, ImVec2 origin )
        {
            if ( nav_main.amount[2] <= 0.001f )
                return;
            draw_color_popup( draw, fov_circle_color, origin + ImVec2{ 700.f, 134.f }, origin + ImVec2{ 718.f, 152.f }, theme::surface_5, theme::border_2 );
            draw_color_popup( draw, fov_outline_color, origin + ImVec2{ 724.f, 134.f }, origin + ImVec2{ 742.f, 152.f }, theme::surface_5, theme::border_2 );
            draw_color_popup( draw, snap_line_color, origin + ImVec2{ 700.f, 239.f }, origin + ImVec2{ 718.f, 257.f }, theme::surface_5, theme::border_2 );
        }

        void draw_target_selection_section_heading( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_12 = draw->VtxBuffer.Size;
            draw_image( draw, 8, origin + ImVec2{ 253.f, 136.f }, origin + ImVec2{ 366.f, 152.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_12, origin + ImVec2{ 309.5f, 144.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_6( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_13 = draw->VtxBuffer.Size;
            draw_image( draw, 9, origin + ImVec2{ 341.f, 132.f }, origin + ImVec2{ 400.f, 151.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_13, origin + ImVec2{ 370.5f, 141.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_7( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_14 = draw->VtxBuffer.Size;
            draw_image( draw, 10, origin + ImVec2{ 316.f, 132.f }, origin + ImVec2{ 339.f, 138.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_14, origin + ImVec2{ 327.5f, 135.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_8( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_15 = draw->VtxBuffer.Size;
            draw_image( draw, 11, origin + ImVec2{ 271.f, 132.f }, origin + ImVec2{ 284.f, 138.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_15, origin + ImVec2{ 277.5f, 135.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_9( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_16 = draw->VtxBuffer.Size;
            draw_image( draw, 12, origin + ImVec2{ 283.f, 132.f }, origin + ImVec2{ 314.f, 138.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_16, origin + ImVec2{ 298.5f, 135.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_10( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_17 = draw->VtxBuffer.Size;
            draw_image( draw, 13, origin + ImVec2{ 253.f, 132.f }, origin + ImVec2{ 266.f, 138.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_17, origin + ImVec2{ 259.5f, 135.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_target_selection_heading_divider( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_18 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 252.37f, 159.f }, origin + ImVec2{ 510.37f, 160.f }, corner_radii{ }, fade( theme::surface_2, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_18, origin + ImVec2{ 381.37f, 159.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_target_part_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_19 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::inter_600_15_5, origin + ImVec2{ 251.57f, 164.99f }, fade( theme::text_subtle, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::target_part );
            scale_vertices( draw, scale_start_19, origin + ImVec2{ 288.5883f, 174.3672f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_target_part_dropdown( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_20 = draw->VtxBuffer.Size;
            const bool hovered = pointer_inside( origin + ImVec2{ 254.f, 192.f }, origin + ImVec2{ 511.f, 240.f } );
            draw_image( draw, 14, origin + ImVec2{ 254.f, 192.f }, origin + ImVec2{ 511.f, 240.f }, fade( IM_COL32_WHITE, page_alpha ) );
            draw_text_line( draw, text_fonts::roboto_600_15_5, origin + ImVec2{ 268.33f, 220.34f }, 0, fade( target_part_open ? theme::accent : theme::text, page_alpha ), target_part_name( settings::silentaim::target_part ) );
            add_rounded_rect_filled( draw, origin + ImVec2{ 487.f, 193.28f }, origin + ImVec2{ 488.f, 237.28f }, corner_radii{ }, fade( theme::surface_7, page_alpha ) );
            draw_image( draw, 15, origin + ImVec2{ 481.f, 210.28f }, origin + ImVec2{ 490.f, 218.28f }, fade( hovered ? theme::accent : IM_COL32_WHITE, page_alpha ) );
            if ( hovered )
                add_rounded_rect_stroke( draw, origin + ImVec2{ 254.5f, 192.5f }, origin + ImVec2{ 510.5f, 239.5f }, corner_radii{ 3.f }, fade( theme::accent, page_alpha ), 1.f );
            scale_vertices( draw, scale_start_20, origin + ImVec2{ 382.36f, 215.64f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_target_part_popup( ImDrawList* draw, ImVec2 origin )
        {
            if ( !target_part_open || nav_main.selected != 2 )
                return;

            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const ImVec2 box_min = origin + ImVec2{ 254.f, 240.f };
            const ImVec2 box_max = box_min + ImVec2{ 257.f, 6.f * 22.f };

            add_rounded_rect_filled( draw, box_min, box_max, corner_radii{ 4.f }, fade( theme::surface_5, page_alpha ) );
            add_rounded_rect_stroke( draw, box_min + ImVec2{ 0.5f, 0.5f }, box_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, page_alpha ), 1.f );

            for ( int index = 0; index < 6; ++index )
            {
                const ImVec2 row_min{ box_min.x, box_min.y + index * 22.f };
                const ImVec2 row_max{ box_max.x, row_min.y + 22.f };
                const bool hovered = pointer_inside( row_min, row_max ) && ImGui::IsMouseHoveringRect( row_min, row_max, false );
                const bool selected = settings::silentaim::target_part == index + 1;
                if ( hovered || selected )
                    add_rounded_rect_filled( draw, row_min + ImVec2{ 4.f, 2.f }, row_max - ImVec2{ 4.f, 2.f }, corner_radii{ 3.f }, fade( mix_color( theme::surface_6, theme::accent, selected ? 0.65f : 0.3f ), page_alpha ) );
                draw_text_line( draw, text_fonts::roboto_600_15_5, ImVec2{ row_min.x + 14.f, row_min.y + 15.5f }, 0, fade( selected ? theme::text : theme::text_faint, page_alpha ), k_part_names[ index ] );
            }
        }

        void draw_use_aimbot_target_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_21 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 252.f, 249.f }, origin + ImVec2{ 270.f, 267.f }, corner_radii{ 3.52f }, fade( mix_color( mix_color( theme::surface, theme::accent, use_aimbot_target.anim ), theme::fill_red_2, use_aimbot_target.hover * ( 1.f - use_aimbot_target.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 252.5f, 249.5f }, origin + ImVec2{ 269.5f, 266.5f }, corner_radii{ 3.02f }, fade( mix_color( theme::border, theme::accent, use_aimbot_target.anim ), page_alpha ), 1.f );
            scale_vertices( draw, scale_start_21, origin + ImVec2{ 261.f, 258.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_use_aimbot_target_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_22 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_16, origin + ImVec2{ 276.16f, 248.48f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::use_aimbot_target );
            scale_vertices( draw, scale_start_22, origin + ImVec2{ 338.708f, 257.855f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_checks_safety_section_heading( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_23 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::inter_800_18, origin + ImVec2{ 249.5f, 280.96f }, fade( theme::text_red, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::checks_safety );
            scale_vertices( draw, scale_start_23, origin + ImVec2{ 330.5161f, 291.8496f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_checks_safety_heading_divider( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_24 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 252.72f, 309.f }, origin + ImVec2{ 510.72f, 310.f }, corner_radii{ }, fade( theme::surface_2, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_24, origin + ImVec2{ 381.72f, 309.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_team_check_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_25 = draw->VtxBuffer.Size;
            draw_checkbox( draw, origin + ImVec2{ 249.f, 315.725f }, team_check, page_alpha );
            scale_vertices( draw, scale_start_25, origin + ImVec2{ 262.f, 328.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_team_check_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_26 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_16, origin + ImVec2{ 276.18f, 318.35f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::team_check );
            scale_vertices( draw, scale_start_26, origin + ImVec2{ 316.5144f, 327.725f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_gun_check_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_27 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 253.f, 348.44f }, origin + ImVec2{ 271.f, 366.44f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, gun_check.anim ), theme::fill_red_2, gun_check.hover * ( 1.f - gun_check.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 253.5f, 348.94f }, origin + ImVec2{ 270.5f, 365.94f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, gun_check.anim ), page_alpha ), 1.f );
            scale_vertices( draw, scale_start_27, origin + ImVec2{ 262.f, 357.44f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_gun_check_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_28 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::outfit_thin_600_16_5, origin + ImVec2{ 276.18f, 346.66f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::gun_check );
            scale_vertices( draw, scale_start_28, origin + ImVec2{ 312.144f, 357.055f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_knock_check_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_29 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 253.f, 379.f }, origin + ImVec2{ 271.f, 396.f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, knock_check.anim ), theme::fill_red_2, knock_check.hover * ( 1.f - knock_check.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 253.5f, 379.5f }, origin + ImVec2{ 270.5f, 395.5f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, knock_check.anim ), page_alpha ), 1.f );
            scale_vertices( draw, scale_start_29, origin + ImVec2{ 262.f, 387.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_11( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_30 = draw->VtxBuffer.Size;
            draw_image( draw, 16, origin + ImVec2{ 250.f, 375.f }, origin + ImVec2{ 274.f, 399.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_30, origin + ImVec2{ 262.f, 387.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_knock_check_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_31 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_16_5, origin + ImVec2{ 276.18f, 378.07f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::knock_check );
            scale_vertices( draw, scale_start_31, origin + ImVec2{ 318.7233f, 387.738f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_health_check_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_32 = draw->VtxBuffer.Size;
            draw_checkbox( draw, origin + ImVec2{ 249.f, 405.725f }, health_check, page_alpha );
            scale_vertices( draw, scale_start_32, origin + ImVec2{ 262.f, 418.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_health_check_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_33 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_16, origin + ImVec2{ 276.26f, 408.35f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::health_check );
            scale_vertices( draw, scale_start_33, origin + ImVec2{ 319.8789f, 417.725f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_fov_accuracy_section_heading( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_34 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_18, origin + ImVec2{ 543.27f, 23.8f }, fade( theme::text_red_2, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::fov_accuracy );
            scale_vertices( draw, scale_start_34, origin + ImVec2{ 609.4072f, 34.3469f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_fov_accuracy_heading_divider( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_35 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 545.f, 51.f }, origin + ImVec2{ 803.f, 52.f }, corner_radii{ }, fade( theme::surface_2, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_35, origin + ImVec2{ 674.f, 51.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_fov_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_36 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 543.06f, 56.77f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::fov );
            scale_vertices( draw, scale_start_36, origin + ImVec2{ 558.3624f, 66.7309f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_fov_value( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_37 = draw->VtxBuffer.Size;
            draw_text_line( draw, text_fonts::roboto_600_17_5, origin + ImVec2{ 803.f, 66.7339f }, 2, fade( theme::text_faint, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), format_value( "%.0f", settings::silentaim::fov ) );
            scale_vertices( draw, scale_start_37, origin + ImVec2{ 790.3263f, 66.7339f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_fov_slider_track( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_38 = draw->VtxBuffer.Size;
            const float knob_x = 576.39f + ImClamp( fov_slider.shown, 0.f, 1.f ) * 226.61f;
            add_rounded_rect_filled( draw, origin + ImVec2{ 576.39f, 86.f }, origin + ImVec2{ 803.f, 91.3f }, corner_radii{ 2.65f }, fade( theme::surface_8, page_alpha ) );
            add_rounded_rect_filled( draw, origin + ImVec2{ 576.39f, 86.f }, origin + ImVec2{ knob_x, 91.3f }, corner_radii{ 2.65f }, fade( mix_color( theme::fill_red, theme::accent, fov_slider.active ? 1.f : 0.f ), page_alpha ) );
            draw_image( draw, 17, origin + ImVec2{ 545.39f, 85.f }, origin + ImVec2{ 570.39f, 92.f }, ImVec2{ 0.f, 0.f }, ImVec2{ 1.f, 1.f }, corner_radii{ 3.26f }, fade( IM_COL32_WHITE, page_alpha ) );
            scale_vertices( draw, scale_start_38, origin + ImVec2{ 689.695f, 88.65f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_fov_slider_knob( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const float knob_x = 576.39f + ImClamp( fov_slider.shown, 0.f, 1.f ) * 226.61f;
            draw_image( draw, 18, origin + ImVec2{ knob_x - 15.5f, 82.f }, origin + ImVec2{ knob_x + 15.5f, 96.f }, ImVec2{ 0.f, 0.f }, ImVec2{ 1.f, 1.f }, corner_radii{ }, fade( IM_COL32_WHITE, page_alpha ) );
            add_ellipse_filled( draw, origin + ImVec2{ knob_x, 88.5f }, ImVec2{ 4.5f, 4.5f }, fade( mix_color( theme::surface_9, theme::accent, fov_slider.hover ), page_alpha ) );
        }

        void draw_picture_detail_12( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_39 = draw->VtxBuffer.Size;
            draw_image( draw, 18, origin + ImVec2{ 542.f, 82.f }, origin + ImVec2{ 573.f, 96.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_39, origin + ImVec2{ 557.5f, 89.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_use_fov_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_40 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 545.f, 107.f }, origin + ImVec2{ 563.f, 125.f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, use_fov.anim ), theme::fill_red_2, use_fov.hover * ( 1.f - use_fov.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 545.5f, 107.5f }, origin + ImVec2{ 562.5f, 124.5f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, use_fov.anim ), page_alpha ), 1.f );
            scale_vertices( draw, scale_start_40, origin + ImVec2{ 554.f, 116.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_use_fov_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_41 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 567.37f, 105.77f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::use_fov );
            scale_vertices( draw, scale_start_41, origin + ImVec2{ 597.4513f, 115.7309f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_draw_fov_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_42 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 544.25f, 137.f }, origin + ImVec2{ 563.25f, 155.f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, draw_fov.anim ), theme::fill_red_2, draw_fov.hover * ( 1.f - draw_fov.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 544.75f, 137.5f }, origin + ImVec2{ 562.75f, 154.5f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, draw_fov.anim ), page_alpha ), 1.f );
            scale_vertices( draw, scale_start_42, origin + ImVec2{ 553.75f, 146.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_draw_fov_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_43 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_17, origin + ImVec2{ 567.37f, 135.77f }, fade( theme::text_muted, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::draw_fov );
            scale_vertices( draw, scale_start_43, origin + ImVec2{ 600.896f, 145.7309f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_advanced_section_heading( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_44 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_18, origin + ImVec2{ 543.06f, 169.8f }, fade( theme::text_red, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::advanced );
            scale_vertices( draw, scale_start_44, origin + ImVec2{ 583.7854f, 180.3469f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_advanced_heading_divider( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_45 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 545.f, 197.f }, origin + ImVec2{ 803.f, 198.f }, corner_radii{ }, fade( theme::surface_2, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_45, origin + ImVec2{ 674.f, 197.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_sticky_aim_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_46 = draw->VtxBuffer.Size;
            draw_checkbox( draw, origin + ImVec2{ 541.f, 203.f }, sticky_aim, page_alpha );
            scale_vertices( draw, scale_start_46, origin + ImVec2{ 554.f, 215.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_13( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_47 = draw->VtxBuffer.Size;
            draw_image( draw, 20, origin + ImVec2{ 545.24f, 203.f }, origin + ImVec2{ 561.24f, 208.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_47, origin + ImVec2{ 553.24f, 205.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_14( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_48 = draw->VtxBuffer.Size;
            draw_image( draw, 21, origin + ImVec2{ 542.f, 206.f }, origin + ImVec2{ 547.f, 223.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_48, origin + ImVec2{ 544.5f, 214.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_picture_detail_15( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_49 = draw->VtxBuffer.Size;
            draw_image( draw, 22, origin + ImVec2{ 545.f, 206.f }, origin + ImVec2{ 568.f, 227.f }, fade( IM_COL32_WHITE, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_49, origin + ImVec2{ 556.5f, 216.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_sticky_aim_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_50 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_16, origin + ImVec2{ 567.37f, 206.45f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::sticky_aim );
            scale_vertices( draw, scale_start_50, origin + ImVec2{ 600.5911f, 215.825f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_draw_target_line_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_51 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 545.f, 237.f }, origin + ImVec2{ 563.f, 255.f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, draw_target_line.anim ), theme::fill_red_2, draw_target_line.hover * ( 1.f - draw_target_line.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 545.5f, 237.5f }, origin + ImVec2{ 562.5f, 254.5f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, draw_target_line.anim ), page_alpha ), 1.f );
            scale_vertices( draw, scale_start_51, origin + ImVec2{ 554.f, 246.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_draw_target_line_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_52 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_16_5, origin + ImVec2{ 567.37f, 235.95f }, fade( theme::text_muted, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::draw_target_line );
            scale_vertices( draw, scale_start_52, origin + ImVec2{ 624.1699f, 245.618f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_raycast_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_rc = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 545.f, 271.f }, origin + ImVec2{ 563.f, 289.f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, raycast_enabled.anim ), theme::fill_red_2, raycast_enabled.hover * ( 1.f - raycast_enabled.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 545.5f, 271.5f }, origin + ImVec2{ 562.5f, 288.5f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, raycast_enabled.anim ), page_alpha ), 1.f );
            scale_vertices( draw, scale_start_rc, origin + ImVec2{ 554.f, 280.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_raycast_label( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_rc = draw->VtxBuffer.Size;
            draw_text_line( draw, text_fonts::roboto_600_16_5, origin + ImVec2{ 567.37f, 269.95f }, 0, fade( theme::text_muted, page_alpha ), "Raycast" );
            scale_vertices( draw, scale_start_rc, origin + ImVec2{ 597.f, 280.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_silentaim_method_dropdown( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const int scale_start_md = draw->VtxBuffer.Size;
            const ImVec2 box_min = origin + ImVec2{ 541.f, 271.f };
            const ImVec2 box_max = origin + ImVec2{ 740.f, 295.f };
            const bool hovered = pointer_inside( box_min, box_max );

            add_rounded_rect_filled( draw, box_min, box_max, corner_radii{ 3.f }, fade( theme::surface_5, page_alpha ) );
            add_rounded_rect_stroke( draw, box_min + ImVec2{ 0.5f, 0.5f }, box_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.f }, fade( aim_method_open || hovered ? theme::accent : theme::border_2, page_alpha ), 1.f );

            draw_text_line( draw, text_fonts::roboto_600_16_5, ImVec2{ box_min.x + 12.f, box_min.y + 16.5f }, 0, fade( theme::text_muted, page_alpha ), "Method" );
            draw_text_line( draw, text_fonts::roboto_600_16_5, ImVec2{ box_min.x + 80.f, box_min.y + 16.5f }, 0, fade( aim_method_open ? theme::accent : theme::text, page_alpha ), raycast_enabled.value ? "Raycast" : "Viewport" );

            const ImVec2 chevron{ box_max.x - 16.f, box_min.y + 11.f };
            draw->AddLine( ImVec2{ chevron.x - 5.f, chevron.y }, ImVec2{ chevron.x, chevron.y + 5.f }, fade( theme::text_muted, page_alpha ), 1.f );
            draw->AddLine( ImVec2{ chevron.x, chevron.y + 5.f }, ImVec2{ chevron.x + 5.f, chevron.y }, fade( theme::text_muted, page_alpha ), 1.f );

            scale_vertices( draw, scale_start_md, origin + ImVec2{ 640.f, 283.f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_silentaim_method_popup( ImDrawList* draw, ImVec2 origin )
        {
            if ( !aim_method_open || nav_main.selected != 2 )
                return;

            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            const ImVec2 box_min = origin + ImVec2{ 541.f, 295.f };
            const ImVec2 box_max = box_min + ImVec2{ 199.f, 2.f * 22.f };

            add_rounded_rect_filled( draw, box_min, box_max, corner_radii{ 4.f }, fade( theme::surface_5, page_alpha ) );
            add_rounded_rect_stroke( draw, box_min + ImVec2{ 0.5f, 0.5f }, box_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, page_alpha ), 1.f );

            static const char* const method_names[2] = { "Viewport", "Raycast" };
            for ( int index = 0; index < 2; ++index )
            {
                const ImVec2 row_min{ box_min.x, box_min.y + index * 22.f };
                const ImVec2 row_max{ box_max.x, row_min.y + 22.f };
                const bool hovered = pointer_inside( row_min, row_max ) && ImGui::IsMouseHoveringRect( row_min, row_max, false );
                const bool selected = ( raycast_enabled.value == ( index == 1 ) );
                if ( hovered || selected )
                    add_rounded_rect_filled( draw, row_min + ImVec2{ 4.f, 2.f }, row_max - ImVec2{ 4.f, 2.f }, corner_radii{ 3.f }, fade( mix_color( theme::surface_6, theme::accent, selected ? 0.65f : 0.3f ), page_alpha ) );
                draw_text_line( draw, text_fonts::roboto_600_15_5, ImVec2{ row_min.x + 14.f, row_min.y + 15.5f }, 0, fade( selected ? theme::text : theme::text_faint, page_alpha ), method_names[ index ] );
            }
        }

        void draw_prediction_section_heading( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_53 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_18, origin + ImVec2{ 834.91f, 23.8f }, fade( theme::text_red, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::prediction );
            scale_vertices( draw, scale_start_53, origin + ImVec2{ 879.5787f, 34.3469f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_prediction_heading_divider( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_54 = draw->VtxBuffer.Size;
            add_rounded_rect_filled( draw, origin + ImVec2{ 837.f, 51.f }, origin + ImVec2{ 1095.f, 52.f }, corner_radii{ }, fade( theme::surface_2, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ) );
            scale_vertices( draw, scale_start_54, origin + ImVec2{ 966.f, 51.5f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_enable_prediction_checkbox( ImDrawList* draw, ImVec2 origin )
        {
            const float page_alpha = page_fade( nav_main.amount[2], nav_main.selected == 2 );
            add_rounded_rect_filled( draw, origin + ImVec2{ 837.f, 61.f }, origin + ImVec2{ 855.f, 79.f }, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, enable_prediction.anim ), theme::fill_red_2, enable_prediction.hover * ( 1.f - enable_prediction.anim ) ), page_alpha ) );
            add_rounded_rect_stroke( draw, origin + ImVec2{ 837.5f, 61.5f }, origin + ImVec2{ 854.5f, 78.5f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, enable_prediction.anim ), page_alpha ), 1.f );
        }

        void draw_enable_prediction_label( ImDrawList* draw, ImVec2 origin )
        {
            const int scale_start_55 = draw->VtxBuffer.Size;
            draw_glyph_run( draw, text_fonts::roboto_600_16, origin + ImVec2{ 860.5f, 60.35f }, fade( theme::text, page_fade( nav_main.amount[2], nav_main.selected == 2 ) ), text::enable_prediction );
            scale_vertices( draw, scale_start_55, origin + ImVec2{ 917.4563f, 69.725f }, page_scale( nav_main.amount[2], nav_main.selected == 2 ) );
        }

        void draw_frame( ImDrawList* draw, ImVec2 origin )
        {
            add_rounded_rect_filled( draw, origin, origin + ImVec2{ 1149.f, 748.f }, corner_radii{ 7.f }, theme::background );
            draw_shader( draw, shader_backgrounds[0], origin, origin + ImVec2{ 1149.f, 748.f }, corner_radii{ 7.f } );
            draw->PushClipRect( origin, origin + ImVec2{ 1149.f, 748.f }, true );
                if ( nav_main.amount[2] > 0.001f )
                    draw_picture_detail( draw, origin );
                draw_sidebar( draw, origin );
                if ( nav_main.amount[2] > 0.001f )
                {
                    draw_picture_detail_2( draw, origin );
                    draw_activation_section_heading( draw, origin );
                    draw_activation_heading_divider( draw, origin );
                    draw_enable_silent_aim_checkbox( draw, origin );
                    draw_enable_silent_aim_label( draw, origin );
                    draw_silent_aim_keybind( draw, origin );
                    draw_target_selection_section_heading( draw, origin );
                    draw_picture_detail_6( draw, origin );
                    draw_picture_detail_7( draw, origin );
                    draw_picture_detail_8( draw, origin );
                    draw_picture_detail_9( draw, origin );
                    draw_picture_detail_10( draw, origin );
                    draw_target_selection_heading_divider( draw, origin );
                    draw_target_part_label( draw, origin );
                    draw_target_part_dropdown( draw, origin );
                    draw_use_aimbot_target_checkbox( draw, origin );
                    draw_use_aimbot_target_label( draw, origin );
                    draw_checks_safety_section_heading( draw, origin );
                    draw_checks_safety_heading_divider( draw, origin );
                    draw_team_check_checkbox( draw, origin );
                    draw_team_check_label( draw, origin );
                    draw_gun_check_checkbox( draw, origin );
                    draw_gun_check_label( draw, origin );
                    draw_knock_check_checkbox( draw, origin );
                    draw_knock_check_label( draw, origin );
                    draw_health_check_checkbox( draw, origin );
                    draw_health_check_label( draw, origin );
                    draw_fov_accuracy_section_heading( draw, origin );
                    draw_fov_accuracy_heading_divider( draw, origin );
                    draw_fov_label( draw, origin );
                    draw_fov_value( draw, origin );
                    draw_fov_slider_track( draw, origin );
                    draw_fov_slider_knob( draw, origin );
                }
                if ( nav_main.amount[2] > 0.001f )
                {
                    draw_use_fov_checkbox( draw, origin );
                    draw_use_fov_label( draw, origin );
                    draw_draw_fov_checkbox( draw, origin );
                    draw_draw_fov_label( draw, origin );
                    draw_advanced_section_heading( draw, origin );
                    draw_advanced_heading_divider( draw, origin );
                    draw_sticky_aim_checkbox( draw, origin );
                    draw_sticky_aim_label( draw, origin );
                    draw_draw_target_line_checkbox( draw, origin );
                    draw_draw_target_line_label( draw, origin );
                    draw_silentaim_method_dropdown( draw, origin );
                    draw_prediction_section_heading( draw, origin );
                    draw_prediction_heading_divider( draw, origin );
                    draw_enable_prediction_checkbox( draw, origin );
                }
                if ( nav_main.amount[2] > 0.001f )
                {
                    draw_enable_prediction_label( draw, origin );
                    draw_prediction_sliders( draw, origin );
                    draw_aim_color_swatches( draw, origin );
                }
                pages::draw( draw, origin, nav_main.selected );
                draw_target_part_popup( draw, origin );
                draw_silentaim_method_popup( draw, origin );
                draw_aim_color_popups( draw, origin );
                if ( nav_main.amount[2] > 0.001f )
                    draw_activation_menu( draw, aim_keybind, origin + ImVec2{ 400.f, 59.f }, origin + ImVec2{ 466.f, 81.f }, page_fade( nav_main.amount[2], nav_main.selected == 2 ) );
                erase_outside_corners( draw, origin, origin + ImVec2{ 1149.f, 748.f }, corner_radii{ 7.f } );
            draw->PopClipRect( );
        }
    }

    void draw_activation_menu( ImDrawList* draw, lazyui::keybind_state& state, ImVec2 box_min, ImVec2 box_max, float alpha )
    {
        if ( !state.context_open )
        {
            state.over_context = false;
            return;
        }

        const ImVec2 min{ box_min.x, box_max.y + 2.f };
        const ImVec2 max{ min.x + 120.f, min.y + 66.f };
        state.over_context = ImGui::IsMouseHoveringRect( min, max, false );
        add_rounded_rect_filled( draw, min, max, corner_radii{ 4.f }, fade( theme::surface_6, alpha ) );
        add_rounded_rect_stroke( draw, min + ImVec2{ 0.5f, 0.5f }, max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, alpha ), 1.f );

        static const char* const names[3] = { "Toggle", "Hold", "Always" };
        for ( int index = 0; index < 3; ++index )
        {
            const ImVec2 row_min{ min.x, min.y + 22.f * index };
            const ImVec2 row_max{ max.x, row_min.y + 22.f };
            const bool hovered = ImGui::IsMouseHoveringRect( row_min, row_max, false );
            if ( hovered )
                ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
            if ( hovered || state.context_mode == index )
                add_rounded_rect_filled( draw, row_min + ImVec2{ 3.f, 3.f }, row_max - ImVec2{ 3.f, 3.f }, corner_radii{ 3.f }, fade( hovered ? theme::fill_red_2 : theme::surface_5, alpha ) );
            draw_text_line( draw, text_fonts::roboto_600_15_5, ImVec2{ row_min.x + 10.f, row_min.y + 15.5f }, 0, fade( state.context_mode == index ? theme::text : theme::text_faint, alpha ), names[index] );
            if ( hovered && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
            {
                state.context_mode = index;
                state.context_open = false;
            }
        }
    }

    ImVec2 frame_size( )
    {
        return ImVec2{ theme::frame_width, theme::frame_height };
    }

    void render( )
    {
        motion = design_motion;
        load_images( );
        set_graphics( lazyui::app.device, lazyui::app.swap_chain );

        const ImVec2 display = ImGui::GetIO( ).DisplaySize;
        const ImVec2 design = frame_size( );

        // Fit the fixed 1149x748 layout to the display instead of letting it spill
        // off small screens: shrink uniformly when it does not fit, never enlarge.
        float layout_scale = 1.f;
        if ( display.x > 0.f && display.y > 0.f )
        {
            const float fit_x = ( display.x - 48.f ) / design.x;
            const float fit_y = ( display.y - 48.f ) / design.y;
            layout_scale = ImClamp( ImMin( fit_x, fit_y ), 0.4f, 1.f );
        }
        const ImVec2 size = ImFloor( design * layout_scale );
        const ImVec2 centered = ImFloor( ( display - size ) * 0.5f );
        const ImVec2 position = ImFloor( ImClamp( centered + window_drag.offset, ImMin( centered, ImVec2{ 0.f, 0.f } ), ImMax( ImMax( display - size, ImVec2{ 0.f, 0.f } ), centered ) ) );
        window_drag.offset = position - centered;
        ImGui::SetNextWindowPos( position );
        ImGui::SetNextWindowSize( size );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2{ 0.f, 0.f } );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.f );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.f );
        ImGui::Begin( "Silent Aim improved", nullptr, window_flags );
        ImDrawList* draw = ImGui::GetWindowDrawList( );
        const ImVec2 origin = ImGui::GetWindowPos( );
        const int intro_start = draw->VtxBuffer.Size;
        // Hit tests run in design space, so feed them the mouse mapped back through
        // layout_scale for the duration of the input pass, then restore it.
        ImGuiIO& io_ref = ImGui::GetIO( );
        const ImVec2 real_mouse = io_ref.MousePos;
        if ( layout_scale != 1.f )
            io_ref.MousePos = origin + ( real_mouse - origin ) / layout_scale;
        update_controls( origin );
        sync_settings( );
        draw_frame( draw, origin );
        ( void )play_intro( draw, intro_start, origin + size * 0.5f );

        // The menu draws in absolute design coordinates, so scale the vertices this
        // frame produced around the window origin to match the shrunken window.
        if ( layout_scale != 1.f )
        {
            for ( int vertex = intro_start; vertex < draw->VtxBuffer.Size; ++vertex )
            {
                ImVec2& pos = draw->VtxBuffer[ vertex ].pos;
                pos = origin + ( pos - origin ) * layout_scale;
            }
        }
        // Restore the real cursor before drag handling, which works in screen space.
        io_ref.MousePos = real_mouse;
        update_window_drag( pointer.claimed || pointer.popup_open, false );
        ImGui::End( );
        ImGui::PopStyleVar( 3 );
    }
}

bool lazyui::image_texture( std::size_t index, texture_handle& texture )
{
    if ( index >= ui::images.size( ) || !ui::images[index].ready( ) )
        return false;
    texture = ui::images[index];
    return true;
}

void lazyui::framework_blur( ImDrawList* draw, ImVec2 min, ImVec2 max, const corner_radii& radii )
{
    if ( ui::ensure_framework_blur( ) )
        ui::framework_blur_effect.draw( draw, min, max, radii.top_left, radii.top_right, radii.bottom_right, radii.bottom_left );
}

void lazyui::close_window( )
{
    ::PostMessageW( lazyui::app.window, WM_CLOSE, 0, 0 );
}

void lazyui::minimize_window( )
{
    ::ShowWindow( lazyui::app.window, SW_MINIMIZE );
}

void lazyui::resize_window( ImVec2 )
{
}
