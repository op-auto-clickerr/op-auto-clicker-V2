#include "../headers/functions.h"
#include "../headers/widgets.h"
#include "playerbar.h"

void c_playerbar::render()
{
    if (!gui || !draw || !font)
        return;
    
    // Spawn Animation// discord.gg/dopamina

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
    float spawn_alpha = var->gui.playerbar_spawn_alpha;
    if (spawn_alpha <= 0.01f)
        return;
    
    // discord.gg/dopamina
    float spawn_scale = 0.85f + (spawn_alpha * 0.15f);
    
    float anim_alpha = spawn_alpha;
    float scale = spawn_scale;
    
    try
    {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        
        // Font
        ImFont* text_font = font->get(main_font_data, 13.0f);
        if (!text_font)
            return;
        
        // Size
        float avatar_size = SCALE(60.0f);
        float padding = SCALE(12.0f);
        float width = SCALE(350.0f);
        float height = avatar_size + padding * 2;
        
        ImVec2 pos = ImVec2(playerbar_pos.x, playerbar_pos.y);
        ImVec2 size = ImVec2(width, height);
        
        // 3D Scale-Transformation
        ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        ImVec2 scaled_pos = ImVec2(
            center.x + (pos.x - center.x) * spawn_scale,
            center.y + (pos.y - center.y) * spawn_scale
        );
        ImVec2 scaled_size = ImVec2(size.x * spawn_scale, size.y * spawn_scale);
        
        ImRect playerbar_rect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y));
        
        // Dragging Logic
        bool hovered = playerbar_rect.Contains(ImGui::GetMousePos());
        bool menu_open = var->gui.menu_open;
        
        if (menu_open && hovered)
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                is_dragging = true;
                drag_offset = ImVec2(
                    ImGui::GetMousePos().x - pos.x,
                    ImGui::GetMousePos().y - pos.y
                );
            }
        }
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            is_dragging = false;
        }
        
        if (is_dragging)
        {
            playerbar_pos.x = ImGui::GetMousePos().x - drag_offset.x;
            playerbar_pos.y = ImGui::GetMousePos().y - drag_offset.y;
            
            // Clamp to screen bounds
            playerbar_pos.x = ImMax(0.0f, ImMin(playerbar_pos.x, io.DisplaySize.x - width));
            playerbar_pos.y = ImMax(0.0f, ImMin(playerbar_pos.y, io.DisplaySize.y - height));
            
            pos = ImVec2(playerbar_pos.x, playerbar_pos.y);
            
            // Update scaled values
            center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
            scaled_pos = ImVec2(
                center.x + (pos.x - center.x) * spawn_scale,
                center.y + (pos.y - center.y) * spawn_scale
            );
            scaled_size = ImVec2(size.x * spawn_scale, size.y * spawn_scale);
            playerbar_rect = ImRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y));
        }
        
        // discord.gg/dopamina
        float shadow_blur = SCALE(12.0f);
        for (int s = 0; s < 6; s++)
        {
            float progress = (float)s / 6.0f;
            float alpha = (1.0f - progress) * 0.25f * anim_alpha;
            float blur_amount = shadow_blur * progress;
            
            draw_list->AddRectFilled(
                ImVec2(scaled_pos.x - blur_amount, scaled_pos.y - blur_amount),
                ImVec2(scaled_pos.x + scaled_size.x + blur_amount, scaled_pos.y + scaled_size.y + blur_amount),
                IM_COL32(0, 0, 0, (int)(alpha * 255)),
                SCALE(8.0f) + blur_amount
            );
        }
        
        // discord.gg/dopamina
        draw_list->PushClipRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y), true);
        
        // ── Bankroll playerbar background ────────────────────
        ImVec4 bg_top    = ImVec4(20.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 0.95f * anim_alpha);
        ImVec4 bg_bottom = ImVec4( 9.0f / 255.0f,  9.0f / 255.0f,  9.0f / 255.0f, 0.95f * anim_alpha);
        
        draw_list->AddRectFilledMultiColor(
            scaled_pos,
            ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y),
            draw->get_clr(bg_top),
            draw->get_clr(bg_top),
            draw->get_clr(bg_bottom),
            draw->get_clr(bg_bottom)
        );
        
        // discord.gg/dopamina
        for (int b = 0; b < 16; b++)
        {
            float blur_inset = SCALE(b * 0.8f);
            float blur_alpha = 0.06f * anim_alpha;
            
            float brightness = (b % 2 == 0) ? 45.0f : 12.0f;
            ImVec4 blur_color = ImVec4(brightness / 255.0f, brightness / 255.0f, (brightness + 5.0f) / 255.0f, blur_alpha);
            
            draw_list->AddRectFilled(
                ImVec2(scaled_pos.x + blur_inset, scaled_pos.y + blur_inset),
                ImVec2(scaled_pos.x + scaled_size.x - blur_inset, scaled_pos.y + scaled_size.y - blur_inset),
                draw->get_clr(blur_color),
                ImMax(SCALE(8.0f) - blur_inset, 0.0f)
            );
        }
        
        // discord.gg/dopamina
        static float shimmer_time = 0.0f;
        shimmer_time += io.DeltaTime * 0.25f;
        
        float shimmer_progress = fmodf(shimmer_time, 1.0f);
        float shimmer_width = SCALE(60.0f);
        
        float start_offset = -(shimmer_width * 1.5f);
        float end_offset = scaled_size.x + scaled_size.y + shimmer_width * 1.5f;
        float current_offset = start_offset + (end_offset - start_offset) * shimmer_progress;
        
        for (int i = 0; i < 15; i++)
        {
            float strip_t = (float)i / 15.0f;
            float next_strip_t = (float)(i + 1) / 15.0f;
            float alpha_mult = (1.0f - fabs(strip_t - 0.5f) * 2.0f) * anim_alpha;
            
            float strip_offset = (strip_t - 0.5f) * shimmer_width;
            float next_strip_offset = (next_strip_t - 0.5f) * shimmer_width;
            
            ImVec2 top_left = ImVec2(scaled_pos.x + current_offset + strip_offset, scaled_pos.y - current_offset - strip_offset);
            ImVec2 top_right = ImVec2(scaled_pos.x + current_offset + next_strip_offset, scaled_pos.y - current_offset - next_strip_offset);
            ImVec2 bottom_right = ImVec2(scaled_pos.x + current_offset + next_strip_offset + scaled_size.x + scaled_size.y, scaled_pos.y - current_offset - next_strip_offset + scaled_size.x + scaled_size.y);
            ImVec2 bottom_left = ImVec2(scaled_pos.x + current_offset + strip_offset + scaled_size.x + scaled_size.y, scaled_pos.y - current_offset - strip_offset + scaled_size.x + scaled_size.y);
            
        ImU32 shimmer_color = IM_COL32(168, 130, 210, (int)(50 * alpha_mult));  // bankroll purple
            draw_list->AddQuadFilled(top_left, top_right, bottom_right, bottom_left, shimmer_color);
        }
        
        draw_list->PopClipRect();
        
        // Bankroll purple border glow
        ImVec4 glow_color = ImVec4(168.0f / 255.0f, 130.0f / 255.0f, 210.0f / 255.0f, 0.15f * anim_alpha);
        draw_list->AddRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y), draw->get_clr(glow_color), SCALE(8.0f), 0, SCALE(1.5f));
        
        // --- CONTENT ---
        
        // discord.gg/dopamina
        ImVec2 avatar_center = ImVec2(scaled_pos.x + padding * scale + avatar_size * 0.5f * scale, scaled_pos.y + padding * scale + avatar_size * 0.5f * scale);
        float avatar_radius = avatar_size * 0.5f * scale;
        
        // discord.gg/dopamina
        draw_list->AddCircleFilled(avatar_center, avatar_radius, IM_COL32(40, 40, 50, (int)(255 * anim_alpha)), 32);
        
        // Bankroll purple avatar ring
        draw_list->AddCircle(avatar_center, avatar_radius, IM_COL32(168, 130, 210, (int)(200 * anim_alpha)), 32, SCALE(2.0f));
        
        // discord.gg/dopamina
        const char* user_icon = "\xEF\x81\xBF";
        ImVec2 avatar_icon_size = text_font->CalcTextSizeA(28.0f, FLT_MAX, 0.0f, user_icon);
        ImVec2 avatar_icon_pos = ImVec2(avatar_center.x - avatar_icon_size.x * 0.5f, avatar_center.y - avatar_icon_size.y * 0.5f);
        draw_list->AddText(text_font, 28.0f, avatar_icon_pos, IM_COL32(180, 180, 200, (int)(255 * anim_alpha)), user_icon);
        
        // discord.gg/dopamina
        float content_x = scaled_pos.x + padding * 2 * scale + avatar_size * scale;
        float content_y = scaled_pos.y + padding * scale;
        
        // discord.gg/dopamina
        ImVec2 username_pos = ImVec2(content_x, content_y + SCALE(2.0f) * scale);
        ImVec4 text_white = ImVec4(0.9f, 0.9f, 0.9f, anim_alpha);
        draw_list->AddText(text_font, 13.0f, username_pos, draw->get_clr(text_white), username.c_str());
        
        // discord.gg/dopamina
        float bar_y = content_y + SCALE(18.0f) * scale;
        float bar_width = SCALE(240.0f) * scale;
        float bar_height = SCALE(8.0f) * scale;
        
        // discord.gg/dopamina
        ImVec2 bar_bg_min = ImVec2(content_x, bar_y);
        ImVec2 bar_bg_max = ImVec2(content_x + bar_width, bar_y + bar_height);
        draw_list->AddRectFilled(bar_bg_min, bar_bg_max, IM_COL32(20, 20, 25, (int)(200 * anim_alpha)), SCALE(2.0f));
        
        // discord.gg/dopamina
        float health_percent = (float)health / (float)max_health;
        ImVec2 bar_fill_max = ImVec2(content_x + bar_width * health_percent, bar_y + bar_height);
        draw_list->AddRectFilled(bar_bg_min, bar_fill_max, IM_COL32(50, 200, 50, (int)(255 * anim_alpha)), SCALE(2.0f));
        
        // discord.gg/dopamina
        float info_y = bar_y + bar_height + SCALE(4.0f) * scale;
        
        // Health Text
        char health_text[32];
        sprintf_s(health_text, "%d / %d ", health, max_health);
        ImVec2 health_text_pos = ImVec2(content_x, info_y);
        ImVec4 text_gray = ImVec4(0.7f, 0.7f, 0.75f, anim_alpha);
        draw_list->AddText(text_font, 13.0f, health_text_pos, draw->get_clr(text_gray), health_text);
        
        // discord.gg/dopamina
        ImVec2 health_text_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, health_text);
        float heart_x = content_x + health_text_size.x + SCALE(2.0f) * scale;
        float heart_y = info_y + SCALE(2.0f) * scale;
        float heart_size = SCALE(9.0f) * scale;
        
        ImVec2 heart_center = ImVec2(heart_x + heart_size * 0.5f, heart_y + heart_size * 0.5f);
        
        float circle_radius = heart_size * 0.3f;
        draw_list->AddCircleFilled(ImVec2(heart_center.x - circle_radius * 0.7f, heart_center.y - circle_radius * 0.3f), circle_radius, IM_COL32(255, 70, 70, (int)(255 * anim_alpha)), 12);
        draw_list->AddCircleFilled(ImVec2(heart_center.x + circle_radius * 0.7f, heart_center.y - circle_radius * 0.3f), circle_radius, IM_COL32(255, 70, 70, (int)(255 * anim_alpha)), 12);
        
        ImVec2 tri_p1 = ImVec2(heart_center.x - heart_size * 0.45f, heart_center.y);
        ImVec2 tri_p2 = ImVec2(heart_center.x + heart_size * 0.45f, heart_center.y);
        ImVec2 tri_p3 = ImVec2(heart_center.x, heart_center.y + heart_size * 0.6f);
        draw_list->AddTriangleFilled(tri_p1, tri_p2, tri_p3, IM_COL32(255, 70, 70, (int)(255 * anim_alpha)));
        
        // Separator
        float sep_x = content_x + health_text_size.x + heart_size + SCALE(8.0f) * scale;
        const char* separator = " • ";
        ImVec2 sep_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, separator);
        draw_list->AddText(text_font, 13.0f, ImVec2(sep_x, info_y), draw->get_clr(text_gray), separator);
        
        // Weapon
        float weapon_x = sep_x + sep_size.x;
        draw_list->AddText(text_font, 13.0f, ImVec2(weapon_x, info_y), draw->get_clr(text_white), weapon.c_str());
        
        // discord.gg/dopamina
        char distance_text[32];
        sprintf_s(distance_text, "%.1fm", distance);
        ImVec2 distance_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, distance_text);
        ImVec2 distance_pos = ImVec2(scaled_pos.x + scaled_size.x - padding * scale - distance_size.x, info_y);
        
        // Distance Icon
        const char* distance_icon = "\xE2\x8A\x95 ";
        ImVec2 icon_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, distance_icon);
        ImVec2 icon_pos = ImVec2(distance_pos.x - icon_size.x, info_y);
        draw_list->AddText(text_font, 13.0f, icon_pos, draw->get_clr(text_gray), distance_icon);
        
        draw_list->AddText(text_font, 13.0f, distance_pos, draw->get_clr(text_white), distance_text);
    }
    catch (...)
    {
        // Catch errors
    }
}
