#include "../headers/functions.h"
#include "../headers/widgets.h"
#include "watermark.h"
#include <d3dcompiler.h>
#include <sstream>
#include <iomanip>
// discord.gg/dopamina

 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina

 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina


 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
#pragma comment(lib, "d3dcompiler.lib")

void c_watermark::initialize(ID3D11Device* device)
{
    if (initialized)
        return;
    
    try
    {
        create_blur_shader(device);
        initialized = true;
    }
    catch (...)
    {
        initialized = false;
    }
}

void c_watermark::start_timer()
{
    if (!timer_started)
    {
        injection_time = std::chrono::steady_clock::now();
        timer_started = true;
    }
}

void c_watermark::create_blur_shader(ID3D11Device* device)
{
    if (!device)
        return;
    
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
    ID3DBlob* error_blob = nullptr;
    
    // Compile Vertex Shader
    HRESULT hr = D3DCompile(
        blur_shader_hlsl,
        strlen(blur_shader_hlsl),
        nullptr,
        nullptr,
        nullptr,
        "VS",
        "vs_5_0",
        0,
        0,
        &vs_blob,
        &error_blob
    );
    
    if (SUCCEEDED(hr))
    {
        device->CreateVertexShader(
            vs_blob->GetBufferPointer(),
            vs_blob->GetBufferSize(),
            nullptr,
            &vertex_shader
        );
    }
    
    if (error_blob) error_blob->Release();
    error_blob = nullptr;
    
    // Compile Pixel Shader
    hr = D3DCompile(
        blur_shader_hlsl,
        strlen(blur_shader_hlsl),
        nullptr,
        nullptr,
        nullptr,
        "PS",
        "ps_5_0",
        0,
        0,
        &ps_blob,
        &error_blob
    );
    
    if (SUCCEEDED(hr))
    {
        device->CreatePixelShader(
            ps_blob->GetBufferPointer(),
            ps_blob->GetBufferSize(),
            nullptr,
            &pixel_shader
        );
    }
    
    if (vs_blob) vs_blob->Release();
    if (ps_blob) ps_blob->Release();
    if (error_blob) error_blob->Release();
    
    // Create Constant Buffer
    D3D11_BUFFER_DESC cb_desc = {};
    cb_desc.ByteWidth = sizeof(float) * 4; // resolution (2) + blurAmount (1) + padding (1)
    cb_desc.Usage = D3D11_USAGE_DYNAMIC;
    cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    device->CreateBuffer(&cb_desc, nullptr, &constant_buffer);
    
    // Create Sampler State
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    
    device->CreateSamplerState(&sampler_desc, &sampler_state);
    
    // Create Blend State for transparency
    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    device->CreateBlendState(&blend_desc, &blend_state);
}

void c_watermark::render()
{
    if (!gui || !draw || !font)
        return;
    
    // Spawn Animation
    float spawn_alpha = var->gui.watermark_spawn_alpha;
    if (spawn_alpha <= 0.01f)
        return;
    
    // discord.gg/dopamina
    float spawn_scale = 0.85f + (spawn_alpha * 0.15f);
    float scale = spawn_scale;
    
    // Start timer on first render
    if (!timer_started)
        start_timer();
    
    try
    {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        
        // Watermark Info
        static const char* discord_text = "discord.gg/dopamina";
        static const char* username_text = "Admin"; // Username = Admin
        
        // FPS
        int fps = (int)io.Framerate;
        char fps_text[32];
        sprintf_s(fps_text, "FPS: %d", fps);
        
        // Ping (Mock - you'd get this from your game)
        static int ping = 45;
        char ping_text[32];
        sprintf_s(ping_text, "Ping: %dms", ping);
        
        // discord.gg/dopamina
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - injection_time);
        int hours = (int)(elapsed.count() / 3600);
        int minutes = (int)((elapsed.count() % 3600) / 60);
        int seconds = (int)(elapsed.count() % 60);
        
        char timer_text[64];
        sprintf_s(timer_text, "Time: %02d:%02d:%02d", hours, minutes, seconds);
        
        // Font
        ImFont* text_font = font->get(main_font_data, 13.0f);
        if (!text_font)
            return;
        
        // Calculate sizes
        ImVec2 discord_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, discord_text);
        ImVec2 username_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, username_text);
        ImVec2 fps_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, fps_text);
        ImVec2 ping_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, ping_text);
        ImVec2 timer_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, timer_text);
        
        // Layout: Discord | Username | FPS | Ping | Timer
        float padding = SCALE(15.0f);
        float separator_width = SCALE(8.0f);
        float total_width = discord_size.x + separator_width + username_size.x + separator_width +
                           fps_size.x + separator_width + ping_size.x + separator_width + timer_size.x + padding * 2;
        float box_height = SCALE(32.0f);
        
        // Position: Use saved position (dragable)
        ImVec2 pos = ImVec2(watermark_pos.x, watermark_pos.y);
        ImVec2 size = ImVec2(total_width, box_height);
        
        // 3D Scale-Transformation
        ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        ImVec2 scaled_pos = ImVec2(
            center.x + (pos.x - center.x) * spawn_scale,
            center.y + (pos.y - center.y) * spawn_scale
        );
        ImVec2 scaled_size = ImVec2(size.x * spawn_scale, size.y * spawn_scale);
        
        ImRect watermark_rect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y));
        
        // discord.gg/dopamina
        bool menu_open = var->gui.menu_open;
        
        if (menu_open)
        {
            bool hovered = watermark_rect.Contains(ImGui::GetMousePos());
            
            // Start dragging
            if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                is_dragging = true;
                drag_offset = ImVec2(
                    ImGui::GetMousePos().x - pos.x,
                    ImGui::GetMousePos().y - pos.y
                );
            }
            
            // Stop dragging
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                is_dragging = false;
            }
            
            // Update position while dragging
            if (is_dragging)
            {
                watermark_pos.x = (ImGui::GetMousePos().x - drag_offset.x);
                watermark_pos.y = (ImGui::GetMousePos().y - drag_offset.y);
                
                // Clamp to screen bounds
                watermark_pos.x = ImMax(0.0f, ImMin(watermark_pos.x, io.DisplaySize.x - total_width));
                watermark_pos.y = ImMax(0.0f, ImMin(watermark_pos.y, io.DisplaySize.y - box_height));
                
                // Update pos
                pos = ImVec2(watermark_pos.x, watermark_pos.y);
                
                // Update scaled values
                center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
                scaled_pos = ImVec2(
                    center.x + (pos.x - center.x) * spawn_scale,
                    center.y + (pos.y - center.y) * spawn_scale
                );
                scaled_size = ImVec2(size.x * spawn_scale, size.y * spawn_scale);
                watermark_rect = ImRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y));
            }
        }
        
        // discord.gg/dopamina
        // discord.gg/dopamina
        float shadow_blur = SCALE(8.0f);
        for (int s = 0; s < 4; s++)
        {
            float progress = (float)s / 4.0f;
            float alpha = (1.0f - progress) * 0.15f * spawn_alpha; // discord.gg/dopamina
            float blur_amount = shadow_blur * progress;
            
            draw_list->AddRectFilled(
                ImVec2(scaled_pos.x - blur_amount, scaled_pos.y - blur_amount),
                ImVec2(scaled_pos.x + scaled_size.x + blur_amount, scaled_pos.y + scaled_size.y + blur_amount),
                IM_COL32(0, 0, 0, (int)(alpha * 255)),
                SCALE(6.0f) + blur_amount
            );
        }
        
        // discord.gg/dopamina
        draw_list->PushClipRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y), true);
        
        // ── Bankroll watermark background ───────────────────────
        ImVec4 bg_top    = ImVec4(20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 0.92f * spawn_alpha);
        ImVec4 bg_bottom = ImVec4( 7.0f / 255.0f,  7.0f / 255.0f,  7.0f / 255.0f, 0.92f * spawn_alpha);
        
        draw_list->AddRectFilledMultiColor(
            scaled_pos,
            ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y),
            draw->get_clr(bg_top),
            draw->get_clr(bg_top),
            draw->get_clr(bg_bottom),
            draw->get_clr(bg_bottom)
        );
        
        // discord.gg/dopamina
        for (int b = 0; b < 12; b++)
        {
            float blur_inset = SCALE(b * 1.0f);
            float blur_alpha = 0.05f * spawn_alpha; // discord.gg/dopamina
            
            // discord.gg/dopamina
            float brightness = (b % 2 == 0) ? 40.0f : 15.0f;
            ImVec4 blur_color = ImVec4(brightness / 255.0f, brightness / 255.0f, (brightness + 5.0f) / 255.0f, blur_alpha);
            
            draw_list->AddRectFilled(
                ImVec2(scaled_pos.x + blur_inset, scaled_pos.y + blur_inset),
                ImVec2(scaled_pos.x + scaled_size.x - blur_inset, scaled_pos.y + scaled_size.y - blur_inset),
                draw->get_clr(blur_color),
                ImMax(SCALE(6.0f) - blur_inset, 0.0f)
            );
        }
        
        draw_list->PopClipRect();
        
        // discord.gg/dopamina
        static float shimmer_time = 0.0f;
        shimmer_time += io.DeltaTime * 0.8f; // discord.gg/dopamina
        
        // discord.gg/dopamina
        if (shimmer_time > 6.28318f) shimmer_time = 0.0f; // 2*PI
        
        // discord.gg/dopamina
        float shimmer_progress = fmodf(shimmer_time / 3.0f, 1.0f);
        
        // discord.gg/dopamina
        draw_list->PushClipRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y), true);
        
        float shimmer_width = SCALE(80.0f);
        
        // discord.gg/dopamina
        float start_offset = -(shimmer_width * 1.5f);
        float end_offset = scaled_size.x + scaled_size.y + shimmer_width * 1.5f;
        float current_offset = start_offset + (end_offset - start_offset) * shimmer_progress;
        
        // discord.gg/dopamina
        // discord.gg/dopamina
        for (int i = 0; i < 20; i++)
        {
            float strip_t = (float)i / 20.0f;
            float next_strip_t = (float)(i + 1) / 20.0f;
            
            // discord.gg/dopamina
            float alpha_mult = (1.0f - fabs(strip_t - 0.5f) * 2.0f) * spawn_alpha;
            
            float strip_offset = (strip_t - 0.5f) * shimmer_width;
            float next_strip_offset = (next_strip_t - 0.5f) * shimmer_width;
            
            // discord.gg/dopamina
            // discord.gg/dopamina
            ImVec2 top_left = ImVec2(
                scaled_pos.x + current_offset + strip_offset,
                scaled_pos.y - current_offset - strip_offset
            );
            ImVec2 top_right = ImVec2(
                scaled_pos.x + current_offset + next_strip_offset,
                scaled_pos.y - current_offset - next_strip_offset
            );
            ImVec2 bottom_right = ImVec2(
                scaled_pos.x + current_offset + next_strip_offset + scaled_size.x + scaled_size.y,
                scaled_pos.y - current_offset - next_strip_offset + scaled_size.x + scaled_size.y
            );
            ImVec2 bottom_left = ImVec2(
                scaled_pos.x + current_offset + strip_offset + scaled_size.x + scaled_size.y,
                scaled_pos.y - current_offset - strip_offset + scaled_size.x + scaled_size.y
            );
            
            ImU32 shimmer_color = IM_COL32(168, 130, 210, (int)(60 * alpha_mult));  // bankroll purple shimmer
            draw_list->AddQuadFilled(top_left, top_right, bottom_right, bottom_left, shimmer_color);
        }
        
        draw_list->PopClipRect();
        
        // Bankroll purple border
        ImVec4 border_color = ImVec4(60.0f / 255.0f, 45.0f / 255.0f, 75.0f / 255.0f, 0.6f * spawn_alpha);
        draw_list->AddRect(
            scaled_pos,
            ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y),
            draw->get_clr(border_color),
            SCALE(6.0f),
            0,
            SCALE(1.0f)
        );
        
        // Bankroll text colours
        ImVec4 text_white  = ImVec4(0.95f, 0.95f, 0.95f, spawn_alpha);
        ImVec4 text_gray   = ImVec4(0.66f, 0.64f, 0.70f, spawn_alpha);
        ImVec4 text_green  = ImVec4(0.3f,  0.95f, 0.3f,  spawn_alpha);
        ImVec4 text_purple = ImVec4(168.0f / 255.0f, 130.0f / 255.0f, 210.0f / 255.0f, spawn_alpha); // bankroll purple
        ImVec4 separator_color = ImVec4(0.4f, 0.3f, 0.5f, 0.5f * spawn_alpha);
        
        // discord.gg/dopamina
        float text_y = scaled_pos.y + (scaled_size.y - discord_size.y * scale) * 0.5f;
        float cursor_x = scaled_pos.x + padding * scale;
        
        // discord.gg/dopamina
        draw_list->AddText(text_font, 13.0f, ImVec2(cursor_x, text_y),
            draw->get_clr(text_purple), discord_text);
        cursor_x += discord_size.x * scale + separator_width * 0.5f * scale;
        
        // Separator 1
        draw_list->AddLine(
            ImVec2(cursor_x, scaled_pos.y + SCALE(8.0f) * scale),
            ImVec2(cursor_x, scaled_pos.y + scaled_size.y - SCALE(8.0f) * scale),
            draw->get_clr(separator_color),
            SCALE(1.0f)
        );
        cursor_x += separator_width * 0.5f * scale;
        
        // discord.gg/dopamina
        draw_list->AddText(text_font, 13.0f, ImVec2(cursor_x, text_y),
            draw->get_clr(text_gray), username_text);
        cursor_x += username_size.x * scale + separator_width * 0.5f * scale;
        
        // Separator 2
        draw_list->AddLine(
            ImVec2(cursor_x, scaled_pos.y + SCALE(8.0f) * scale),
            ImVec2(cursor_x, scaled_pos.y + scaled_size.y - SCALE(8.0f) * scale),
            draw->get_clr(separator_color),
            SCALE(1.0f)
        );
        cursor_x += separator_width * 0.5f * scale;
        
        // discord.gg/dopamina
        // discord.gg/dopamina
        for (int g = 3; g > 0; g--)
        {
            float glow_alpha = (g / 3.0f) * 0.3f * spawn_alpha;
            float glow_offset = SCALE(g * 0.5f);
            
            draw_list->AddText(text_font, 13.0f, ImVec2(cursor_x, text_y),
                IM_COL32(76, 242, 76, (int)(glow_alpha * 255)), fps_text);
        }
        draw_list->AddText(text_font, 13.0f, ImVec2(cursor_x, text_y),
            draw->get_clr(text_green), fps_text);
        cursor_x += fps_size.x * scale + separator_width * 0.5f * scale;
        
        // Separator 3
        draw_list->AddLine(
            ImVec2(cursor_x, scaled_pos.y + SCALE(8.0f) * scale),
            ImVec2(cursor_x, scaled_pos.y + scaled_size.y - SCALE(8.0f) * scale),
            draw->get_clr(separator_color),
            SCALE(1.0f)
        );
        cursor_x += separator_width * 0.5f * scale;
        
        // discord.gg/dopamina
        draw_list->AddText(text_font, 13.0f, ImVec2(cursor_x, text_y),
            draw->get_clr(text_white), ping_text);
        cursor_x += ping_size.x * scale + separator_width * 0.5f * scale;
        
        // Separator 4
        draw_list->AddLine(
            ImVec2(cursor_x, scaled_pos.y + SCALE(8.0f) * scale),
            ImVec2(cursor_x, scaled_pos.y + scaled_size.y - SCALE(8.0f) * scale),
            draw->get_clr(separator_color),
            SCALE(1.0f)
        );
        cursor_x += separator_width * 0.5f * scale;
        
        // discord.gg/dopamina
        draw_list->AddText(text_font, 13.0f, ImVec2(cursor_x, text_y),
            draw->get_clr(text_white), timer_text);
    }
    catch (...)
    {
        // Catch any errors
    }
}

void c_watermark::cleanup()
{
    if (vertex_shader) { vertex_shader->Release(); vertex_shader = nullptr; }
    if (pixel_shader) { pixel_shader->Release(); pixel_shader = nullptr; }
    if (constant_buffer) { constant_buffer->Release(); constant_buffer = nullptr; }
    if (sampler_state) { sampler_state->Release(); sampler_state = nullptr; }
    if (blend_state) { blend_state->Release(); blend_state = nullptr; }
    
    initialized = false;
}

