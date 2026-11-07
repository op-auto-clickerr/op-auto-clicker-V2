#include "../headers/functions.h"
#include "../headers/widgets.h"
#include "playerlist.h"

void c_playerlist::init_dummy_data()
{
    static bool initialized = false;
    if (initialized) return;
    
    // Dummy players for testing
    players.clear();
    player_hover_alpha.clear();
    
    for (int i = 0; i < 25; i++)
    {
        player_info p;
        p.id = i;
        p.name = "Player " + std::to_string(i + 1);
        p.health = 50.0f + (float)(rand() % 50);
        p.is_alive = (rand() % 10) != 0;  // 90% alive
        p.avatar = nullptr;
        
        players.push_back(p);
        player_hover_alpha.push_back(0.0f);
    }
    
    initialized = true;
}

void c_playerlist::render()
{
    if (!gui || !draw || !font)
        return;
    
    // discord.gg/dopamina
    float anim_alpha = var->gui.playerlist_spawn_alpha;
    if (anim_alpha <= 0.01f)
        return;
    
    float scale = 0.85f + (anim_alpha * 0.15f);
    
    init_dummy_data();
    
    try
    {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        
        // Font
        ImFont* text_font = font->get(main_font_data, 13.0f);
        ImFont* title_font = font->get(main_font_data, 15.0f);
        if (!text_font || !title_font)
            return;
        
        // discord.gg/dopamina
        float width = SCALE(350.0f);
        float header_height = SCALE(50.0f);
        float player_item_height = SCALE(48.0f);
        float max_visible_players = 8;
        float list_height = player_item_height * max_visible_players;
        float buttons_height = SCALE(60.0f);
        float total_height = header_height + list_height + (selected_player_id >= 0 ? buttons_height : 0.0f);
        float padding = SCALE(12.0f);
        
        ImVec2 pos = window_pos;
        ImVec2 size = ImVec2(width, total_height);
        
        // 3D Scale-Transformation
        ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        ImVec2 scaled_pos = ImVec2(
            center.x + (pos.x - center.x) * scale,
            center.y + (pos.y - center.y) * scale
        );
        ImVec2 scaled_size = ImVec2(size.x * scale, size.y * scale);
        
        ImRect window_rect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y));
        
        // Dragging
        bool hovered = window_rect.Contains(ImGui::GetMousePos());
        bool menu_open = var->gui.menu_open;
        
        if (menu_open && hovered && ImGui::GetMousePos().y < pos.y + header_height)
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
            is_dragging = false;
        
        if (is_dragging)
        {
            window_pos.x = ImGui::GetMousePos().x - drag_offset.x;
            window_pos.y = ImGui::GetMousePos().y - drag_offset.y;
            
            window_pos.x = ImMax(0.0f, ImMin(window_pos.x, io.DisplaySize.x - width));
            window_pos.y = ImMax(0.0f, ImMin(window_pos.y, io.DisplaySize.y - total_height));
            
            pos = window_pos;
            window_rect = ImRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        }
        
        gui->easing(hover_alpha, hovered ? 1.0f : 0.0f, 10.0f, dynamic_easing);
        
        // discord.gg/dopamina
        
        // Soft Shadow (6 layers)
        float shadow_blur = SCALE(12.0f);
        for (int s = 0; s < 6; s++)
        {
            float progress = (float)s / 6.0f;
            float alpha = (1.0f - progress) * 0.25f;
            float blur_amount = shadow_blur * progress;
            
            draw_list->AddRectFilled(
                ImVec2(pos.x - blur_amount, pos.y - blur_amount),
                ImVec2(pos.x + size.x + blur_amount, pos.y + size.y + blur_amount),
                IM_COL32(0, 0, 0, (int)(alpha * 255)),
                SCALE(8.0f) + blur_amount
            );
        }
        
        // ── Bankroll playerlist background ───────────────────
        ImVec4 bg_top    = ImVec4(20.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 0.95f);
        ImVec4 bg_bottom = ImVec4( 9.0f / 255.0f,  9.0f / 255.0f,  9.0f / 255.0f, 0.95f);
        
        draw_list->AddRectFilledMultiColor(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            draw->get_clr(bg_top),
            draw->get_clr(bg_top),
            draw->get_clr(bg_bottom),
            draw->get_clr(bg_bottom)
        );
        
        // Blur Effect (16 layers)
        for (int b = 0; b < 16; b++)
        {
            float blur_inset = SCALE(b * 0.8f);
            float blur_alpha = 0.06f;
            
            float brightness = (b % 2 == 0) ? 45.0f : 12.0f;
            ImVec4 blur_color = ImVec4(brightness / 255.0f, brightness / 255.0f, (brightness + 5.0f) / 255.0f, blur_alpha);
            
            draw_list->AddRectFilled(
                ImVec2(pos.x + blur_inset, pos.y + blur_inset),
                ImVec2(pos.x + size.x - blur_inset, pos.y + size.y - blur_inset),
                draw->get_clr(blur_color),
                ImMax(SCALE(8.0f) - blur_inset, 0.0f)
            );
        }
        
        // discord.gg/dopamina
        static float shimmer_time = 0.0f;
        shimmer_time += io.DeltaTime * 0.12f;
        
        float shimmer_progress = fmodf(shimmer_time, 1.0f);
        float shimmer_width = SCALE(60.0f);
        
        float start_offset = -(shimmer_width * 1.5f);
        float end_offset = size.x + size.y + shimmer_width * 1.5f;
        float current_offset = start_offset + (end_offset - start_offset) * shimmer_progress;
        
        // discord.gg/dopamina
        draw_list->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);
        
        for (int i = 0; i < 15; i++)
        {
            float strip_t = (float)i / 15.0f;
            float next_strip_t = (float)(i + 1) / 15.0f;
            float alpha_mult = 1.0f - fabs(strip_t - 0.5f) * 2.0f;
            
            float strip_offset = (strip_t - 0.5f) * shimmer_width;
            float next_strip_offset = (next_strip_t - 0.5f) * shimmer_width;
            
            ImVec2 top_left = ImVec2(pos.x + current_offset + strip_offset, pos.y - current_offset - strip_offset);
            ImVec2 top_right = ImVec2(pos.x + current_offset + next_strip_offset, pos.y - current_offset - next_strip_offset);
            ImVec2 bottom_right = ImVec2(pos.x + current_offset + next_strip_offset + size.x + size.y, pos.y - current_offset - next_strip_offset + size.x + size.y);
            ImVec2 bottom_left = ImVec2(pos.x + current_offset + strip_offset + size.x + size.y, pos.y - current_offset - strip_offset + size.x + size.y);
            
            ImU32 shimmer_color = IM_COL32(168, 130, 210, (int)(50 * alpha_mult));  // bankroll purple
            draw_list->AddQuadFilled(top_left, top_right, bottom_right, bottom_left, shimmer_color);
        }
        
        draw_list->PopClipRect();
        
        // === HEADER ===
        ImVec2 header_pos = pos;
        
        // Title
        const char* title = "Player List";
        ImVec2 title_size = title_font->CalcTextSizeA(15.0f, FLT_MAX, 0.0f, title);
        ImVec2 title_pos = ImVec2(header_pos.x + padding, header_pos.y + (header_height - title_size.y) * 0.5f);
        ImVec4 title_color = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
        draw_list->AddText(title_font, 15.0f, title_pos, draw->get_clr(title_color), title);
        
        // Player count
        char count_text[32];
        int alive_count = 0;
        for (auto& p : players)
            if (p.is_alive) alive_count++;
        
        sprintf_s(count_text, "%d / %d", alive_count, (int)players.size());
        ImVec2 count_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, count_text);
        ImVec2 count_pos = ImVec2(header_pos.x + width - padding - count_size.x, header_pos.y + (header_height - count_size.y) * 0.5f);
        ImVec4 count_color = ImVec4(0.7f, 0.7f, 0.75f, 1.0f);
        draw_list->AddText(text_font, 13.0f, count_pos, draw->get_clr(count_color), count_text);
        
        // Separator line
        draw_list->AddLine(
            ImVec2(pos.x + padding, pos.y + header_height),
            ImVec2(pos.x + width - padding, pos.y + header_height),
            IM_COL32(255, 255, 255, 15),
            1.0f
        );
        
        // === PLAYER LIST (scrollable) ===
        ImVec2 list_pos = ImVec2(pos.x, pos.y + header_height);
        ImRect list_rect(list_pos, ImVec2(list_pos.x + width, list_pos.y + list_height));
        
        // Scroll handling
        if (list_rect.Contains(ImGui::GetMousePos()))
        {
            float scroll_delta = io.MouseWheel * player_item_height * 2.0f;
            target_scroll -= scroll_delta;
            
            float max_scroll = ImMax(0.0f, (float)players.size() * player_item_height - list_height);
            target_scroll = ImClamp(target_scroll, 0.0f, max_scroll);
        }
        
        gui->easing(scroll_offset, target_scroll, 12.0f, dynamic_easing);
        
        // Clip for scrolling
        draw_list->PushClipRect(list_pos, ImVec2(list_pos.x + width, list_pos.y + list_height), true);
        
        // Draw players
        int start_index = (int)(scroll_offset / player_item_height);
        int end_index = ImMin((int)players.size(), start_index + (int)max_visible_players + 2);
        
        for (int i = start_index; i < end_index; i++)
        {
            player_info& player = players[i];
            float y_offset = list_pos.y + (i * player_item_height) - scroll_offset;
            
            ImVec2 item_pos = ImVec2(list_pos.x, y_offset);
            ImRect item_rect(item_pos, ImVec2(item_pos.x + width, item_pos.y + player_item_height));
            
            bool item_hovered = item_rect.Contains(ImGui::GetMousePos()) && list_rect.Contains(ImGui::GetMousePos());
            gui->easing(player_hover_alpha[i], item_hovered ? 1.0f : 0.0f, 15.0f, dynamic_easing);
            
            // Click to select
            if (item_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                selected_player_id = player.id;
            }
            
            // Highlight if selected
            bool is_selected = (selected_player_id == player.id);
            
            if (is_selected || player_hover_alpha[i] > 0.01f)
            {
                if (is_selected)
                {
                    // discord.gg/dopamina
                    // discord.gg/dopamina
                    for (int s = 0; s < 4; s++)
                    {
                        float sp = (float)s / 4.0f;
                        float sa = (1.0f - sp) * 0.12f;
                        float sb = SCALE(4.0f) * sp;
                        draw_list->AddRectFilled(
                            ImVec2(item_pos.x - sb, item_pos.y - sb),
                            ImVec2(item_pos.x + width + sb, item_pos.y + player_item_height + sb),
                            IM_COL32(168, 130, 210, (int)(sa * 255)), SCALE(4.0f)  // bankroll purple glow
                        );
                    }
                    
                    // discord.gg/dopamina
                    draw_list->AddRectFilledMultiColor(
                        item_pos,
                        ImVec2(item_pos.x + width, item_pos.y + player_item_height),
                        IM_COL32(168, 130, 210, 45),
                        IM_COL32(168, 130, 210, 20),
                        IM_COL32(168, 130, 210, 10),
                        IM_COL32(168, 130, 210, 25)
                    );
                    
                    // selected left accent bar
                    draw_list->AddRectFilled(
                        item_pos,
                        ImVec2(item_pos.x + SCALE(2.5f), item_pos.y + player_item_height),
                        IM_COL32(168, 130, 210, 255), SCALE(2.0f)
                    );
                    
                    // discord.gg/dopamina
                    for (int g = 0; g < 3; g++)
                    {
                        float ga = (1.0f - (float)g / 3.0f) * 0.3f;
                        float go = SCALE(g * 2.0f);
                        draw_list->AddRectFilled(
                            ImVec2(item_pos.x - go, item_pos.y),
                            ImVec2(item_pos.x + SCALE(2.5f) + go, item_pos.y + player_item_height),
                            IM_COL32(168, 130, 210, (int)(ga * 255)), SCALE(2.0f)  // bankroll purple
                        );
                    }
                }
                else
                {
                    // discord.gg/dopamina
                    draw_list->AddRectFilled(
                        item_pos,
                        ImVec2(item_pos.x + width, item_pos.y + player_item_height),
                        IM_COL32(255, 255, 255, (int)(player_hover_alpha[i] * 18))
                    );
                }
            }
            
            // Avatar (circle placeholder)
            float avatar_size = SCALE(32.0f);
            ImVec2 avatar_center = ImVec2(item_pos.x + padding + avatar_size * 0.5f, item_pos.y + player_item_height * 0.5f);
            
            ImVec4 avatar_bg = player.is_alive ? ImVec4(0.15f, 0.15f, 0.2f, 1.0f) : ImVec4(0.3f, 0.1f, 0.1f, 1.0f);
            draw_list->AddCircleFilled(avatar_center, avatar_size * 0.5f, draw->get_clr(avatar_bg), 32);
            
            // Avatar initial (first letter of name)
            if (!player.name.empty())
            {
                char initial[2] = { player.name[0], '\0' };
                ImVec2 initial_size = text_font->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, initial);
                ImVec2 initial_pos = ImVec2(avatar_center.x - initial_size.x * 0.5f, avatar_center.y - initial_size.y * 0.5f);
                ImVec4 initial_color = ImVec4(0.8f, 0.8f, 0.85f, 1.0f);
                draw_list->AddText(text_font, 14.0f, initial_pos, draw->get_clr(initial_color), initial);
            }
            
            // Player name
            float text_x = item_pos.x + padding * 2 + avatar_size;
            float text_y = item_pos.y + SCALE(10.0f);
            
            ImVec4 name_color = player.is_alive ? ImVec4(0.95f, 0.95f, 0.95f, 1.0f) : ImVec4(0.5f, 0.5f, 0.55f, 1.0f);
            draw_list->AddText(text_font, 13.0f, ImVec2(text_x, text_y), draw->get_clr(name_color), player.name.c_str());
            
            // Health bar
            float bar_y = text_y + SCALE(18.0f);
            float bar_width = SCALE(180.0f);
            float bar_height = SCALE(4.0f);
            
            // Health bar background
            ImVec4 bar_bg = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
            draw_list->AddRectFilled(
                ImVec2(text_x, bar_y),
                ImVec2(text_x + bar_width, bar_y + bar_height),
                draw->get_clr(bar_bg),
                SCALE(2.0f)
            );
            
            // Health bar fill
            if (player.is_alive && player.health > 0.0f)
            {
                float health_pct = player.health / 100.0f;
                float fill_width = bar_width * health_pct;
                
                ImVec4 health_color;
                if (health_pct > 0.6f)
                    health_color = ImVec4(0.2f, 1.0f, 0.4f, 1.0f);  // Green
                else if (health_pct > 0.3f)
                    health_color = ImVec4(1.0f, 0.7f, 0.2f, 1.0f);  // Orange
                else
                    health_color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);  // Red
                
                draw_list->AddRectFilled(
                    ImVec2(text_x, bar_y),
                    ImVec2(text_x + fill_width, bar_y + bar_height),
                    draw->get_clr(health_color),
                    SCALE(2.0f)
                );
            }
            
            // Health text
            char health_text[16];
            sprintf_s(health_text, "%.0f HP", player.health);
            ImVec2 health_size = text_font->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, health_text);
            ImVec2 health_pos = ImVec2(text_x + bar_width + SCALE(8.0f), bar_y - SCALE(2.0f));
            ImVec4 health_text_color = ImVec4(0.7f, 0.7f, 0.75f, 1.0f);
            draw_list->AddText(text_font, 11.0f, health_pos, draw->get_clr(health_text_color), health_text);
        }
        
        draw_list->PopClipRect();
        
        // === ACTION BUTTONS (if player selected) ===
        if (selected_player_id >= 0)
        {
            ImVec2 buttons_pos = ImVec2(pos.x, pos.y + header_height + list_height);
            
            // Separator line
            draw_list->AddLine(
                ImVec2(buttons_pos.x + padding, buttons_pos.y),
                ImVec2(buttons_pos.x + width - padding, buttons_pos.y),
                IM_COL32(255, 255, 255, 15),
                1.0f
            );
            
            float button_width = (width - padding * 3) * 0.5f;
            float button_height = SCALE(28.0f);  // discord.gg/dopamina
            float button_y = buttons_pos.y + (buttons_height - button_height) * 0.5f;
            
            // Config button style colors
            ImVec4 button_bg = ImVec4(18.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 0.95f);
            ImVec4 button_bg_detail = ImVec4(12.0f / 255.0f, 12.0f / 255.0f, 16.0f / 255.0f, 0.95f);
            ImVec4 text_color = ImVec4(0.88f, 0.88f, 0.88f, 1.0f);
            ImVec4 text_color_active = ImVec4(168.0f / 255.0f, 130.0f / 255.0f, 210.0f / 255.0f, 1.0f); // bankroll purple
            
            // === TELEPORT BUTTON ===
            ImVec2 teleport_base_pos = ImVec2(buttons_pos.x + padding, button_y);
            
            // Hover/Active detection
            ImRect teleport_rect(teleport_base_pos, ImVec2(teleport_base_pos.x + button_width, teleport_base_pos.y + button_height));
            bool teleport_hovered = teleport_rect.Contains(ImGui::GetMousePos());
            bool teleport_held = teleport_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
            
            gui->easing(teleport_hover, teleport_hovered ? 1.0f : 0.0f, 20.0f, static_easing);
            gui->easing(teleport_active, teleport_held ? 1.0f : 0.0f, 20.0f, dynamic_easing);
            
            if (teleport_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                notify->add_notify("Teleported to player", success);
            }
            
            // Scale effect when pressed
            float teleport_scale_offset_x = (button_width * 0.1f) * teleport_active;
            ImVec2 teleport_pos = ImVec2(teleport_base_pos.x + teleport_scale_offset_x * 0.5f, teleport_base_pos.y);
            ImVec2 teleport_size = ImVec2(button_width - teleport_scale_offset_x, button_height);
            ImRect teleport_button_rect(teleport_pos, ImVec2(teleport_pos.x + teleport_size.x, teleport_pos.y + teleport_size.y));
            
            // Shadow layers
            float shadow_strength = 0.5f + (0.2f * teleport_hover);
            float shadow_blur = SCALE(15.0f);
            float shadow_offset = SCALE(2.5f * teleport_hover);
            
            for (int s = 0; s < 4; s++)
            {
                float progress = (float)s / 4.0f;
                float alpha = (1.0f - progress) * 0.15f * shadow_strength;
                float blur_amount = shadow_blur * progress;
                
                draw_list->AddRectFilled(
                    ImVec2(teleport_button_rect.Min.x - blur_amount + shadow_offset, teleport_button_rect.Min.y - blur_amount + shadow_offset),
                    ImVec2(teleport_button_rect.Max.x + blur_amount + shadow_offset, teleport_button_rect.Max.y + blur_amount + shadow_offset),
                    IM_COL32(0, 0, 0, (int)(alpha * 255)),
                    SCALE(4.0f)
                );
            }
            
            // Subtle glow effect (hover responsive)
            float glow_strength = 0.3f + (teleport_hover * 0.4f);
            for (int g = 0; g < 3; g++)
            {
                float glow_offset = SCALE(1.0f + g * 1.5f);
                float glow_alpha = (1.0f - (float)g / 3.0f) * 0.12f * glow_strength;
                ImVec4 glow_color = ImVec4(168.0f / 255.0f, 130.0f / 255.0f, 210.0f / 255.0f, glow_alpha); // bankroll purple
                
                draw_list->AddRect(
                    ImVec2(teleport_button_rect.Min.x - glow_offset, teleport_button_rect.Min.y - glow_offset),
                    ImVec2(teleport_button_rect.Max.x + glow_offset, teleport_button_rect.Max.y + glow_offset),
                    draw->get_clr(glow_color),
                    SCALE(4.0f), 0, SCALE(1.0f + g * 0.5f)
                );
            }
            
            // Background Detail Layer
            draw_list->AddRectFilled(
                ImVec2(teleport_button_rect.Min.x + SCALE(1.0f), teleport_button_rect.Min.y + SCALE(1.0f)),
                teleport_button_rect.Max,
                draw->get_clr(button_bg_detail),
                SCALE(4.0f)
            );
            
            // Background Main
            draw_list->AddRectFilled(
                teleport_button_rect.Min,
                teleport_button_rect.Max,
                draw->get_clr(button_bg),
                SCALE(4.0f)
            );
            
            // Teleport text - centered, dual layer
            const char* teleport_text = "Teleport";
            ImVec2 teleport_text_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, teleport_text);
            ImVec2 teleport_text_pos = ImVec2(
                teleport_button_rect.Min.x + (teleport_size.x - teleport_text_size.x) * 0.5f,
                teleport_button_rect.Min.y + (button_height - teleport_text_size.y) * 0.5f
            );
            
            // Disabled text layer
            draw_list->AddText(text_font, 13.0f, teleport_text_pos, draw->get_clr(text_color), teleport_text);
            
            // discord.gg/dopamina
            draw_list->AddText(text_font, 13.0f, teleport_text_pos, draw->get_clr(text_color_active, teleport_active), teleport_text);
            
            // === SPECTATE BUTTON ===
            ImVec2 spectate_base_pos = ImVec2(buttons_pos.x + padding * 2 + button_width, button_y);
            
            // Hover/Active detection
            ImRect spectate_rect(spectate_base_pos, ImVec2(spectate_base_pos.x + button_width, spectate_base_pos.y + button_height));
            bool spectate_hovered = spectate_rect.Contains(ImGui::GetMousePos());
            bool spectate_held = spectate_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
            
            gui->easing(spectate_hover, spectate_hovered ? 1.0f : 0.0f, 20.0f, static_easing);
            gui->easing(spectate_active, spectate_held ? 1.0f : 0.0f, 20.0f, dynamic_easing);
            
            if (spectate_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                notify->add_notify("Spectating player", info);
            }
            
            // Scale effect when pressed
            float spectate_scale_offset_x = (button_width * 0.1f) * spectate_active;
            ImVec2 spectate_pos = ImVec2(spectate_base_pos.x + spectate_scale_offset_x * 0.5f, spectate_base_pos.y);
            ImVec2 spectate_size = ImVec2(button_width - spectate_scale_offset_x, button_height);
            ImRect spectate_button_rect(spectate_pos, ImVec2(spectate_pos.x + spectate_size.x, spectate_pos.y + spectate_size.y));
            
            // Shadow layers
            shadow_strength = 0.5f + (0.2f * spectate_hover);
            
            for (int s = 0; s < 4; s++)
            {
                float progress = (float)s / 4.0f;
                float alpha = (1.0f - progress) * 0.15f * shadow_strength;
                float blur_amount = shadow_blur * progress;
                
                draw_list->AddRectFilled(
                    ImVec2(spectate_button_rect.Min.x - blur_amount + shadow_offset, spectate_button_rect.Min.y - blur_amount + shadow_offset),
                    ImVec2(spectate_button_rect.Max.x + blur_amount + shadow_offset, spectate_button_rect.Max.y + blur_amount + shadow_offset),
                    IM_COL32(0, 0, 0, (int)(alpha * 255)),
                    SCALE(4.0f)
                );
            }
            
            // Subtle glow effect (hover responsive)
            glow_strength = 0.3f + (spectate_hover * 0.4f);
            for (int g = 0; g < 3; g++)
            {
                float glow_offset = SCALE(1.0f + g * 1.5f);
                float glow_alpha = (1.0f - (float)g / 3.0f) * 0.12f * glow_strength;
                ImVec4 glow_color = ImVec4(168.0f / 255.0f, 130.0f / 255.0f, 210.0f / 255.0f, glow_alpha); // bankroll purple
                
                draw_list->AddRect(
                    ImVec2(spectate_button_rect.Min.x - glow_offset, spectate_button_rect.Min.y - glow_offset),
                    ImVec2(spectate_button_rect.Max.x + glow_offset, spectate_button_rect.Max.y + glow_offset),
                    draw->get_clr(glow_color),
                    SCALE(4.0f), 0, SCALE(1.0f + g * 0.5f)
                );
            }
            
            // Background Detail Layer
            draw_list->AddRectFilled(
                ImVec2(spectate_button_rect.Min.x + SCALE(1.0f), spectate_button_rect.Min.y + SCALE(1.0f)),
                spectate_button_rect.Max,
                draw->get_clr(button_bg_detail),
                SCALE(4.0f)
            );
            
            // Background Main
            draw_list->AddRectFilled(
                spectate_button_rect.Min,
                spectate_button_rect.Max,
                draw->get_clr(button_bg),
                SCALE(4.0f)
            );
            
            // Spectate text - centered, dual layer
            const char* spectate_text = "Spectate";
            ImVec2 spectate_text_size = text_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, spectate_text);
            ImVec2 spectate_text_pos = ImVec2(
                spectate_button_rect.Min.x + (spectate_size.x - spectate_text_size.x) * 0.5f,
                spectate_button_rect.Min.y + (button_height - spectate_text_size.y) * 0.5f
            );
            
            // Disabled text layer
            draw_list->AddText(text_font, 13.0f, spectate_text_pos, draw->get_clr(text_color), spectate_text);
            
            // discord.gg/dopamina
            draw_list->AddText(text_font, 13.0f, spectate_text_pos, draw->get_clr(text_color_active, spectate_active), spectate_text);
        }
    }
    catch (...)
    {
        // Handle errors
    }
}
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