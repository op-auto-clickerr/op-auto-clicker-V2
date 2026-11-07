#include "../headers/functions.h"
#include "../headers/widgets.h"
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
void c_widgets::slider_int(std::string_view name, int* value, int min, int max)
{
    // Safety checks
    if (!value || !gui || !draw || !font)
        return;
    
    try
    {
        struct anim_t
        {
            float fill_width{ 0 };
            float hover_alpha{ 0 };
        };

        ImGuiWindow* window = gui->get_window();
        if (!window)
            return;
            
        ImGuiID id = window->GetID(name.data());
    
        // discord.gg/dopamina
        float track_height = SCALE(16.0f);
        float total_height = SCALE(34.0f);
        float top_offset = SCALE(6.0f);
        
        ImVec2 pos = window->DC.CursorPos;
        pos.y += top_offset;
        
        float width = gui->content_avail().x - SCALE(10.0f);
        
        // Rects
        ImRect full_rect(pos, ImVec2(pos.x + width, pos.y + total_height));
        ImRect track_rect(ImVec2(full_rect.Min.x, full_rect.Min.y + SCALE(16.0f)), 
                         ImVec2(full_rect.Max.x, full_rect.Min.y + SCALE(16.0f) + track_height));
        
        // Get animation state
        anim_t* anim = gui->anim_container<anim_t>(id);
        if (!anim)
            return;
        
        // Slider calculations
        float progress = static_cast<float>(*value - min) / static_cast<float>(max - min);
        float target_fill_width = track_rect.GetWidth() * progress;
        
        // Interaction
        bool hovered = track_rect.Contains(gui->mouse_pos());
        bool held;
        gui->button_behavior(track_rect, id, &hovered, &held);
        
        // Handle dragging
        if (held)
        {
            const float t = ImClamp((gui->mouse_pos().x - track_rect.Min.x) / track_rect.GetWidth(), 0.0f, 1.0f);
            *value = min + static_cast<int>(t * (max - min));
            *value = ImClamp(*value, min, max);
        }
        
        // Animations
        gui->easing(anim->fill_width, target_fill_width, 12.f, dynamic_easing);
        gui->easing(anim->hover_alpha, hovered ? 1.0f : 0.0f, 15.f, dynamic_easing);
        
        // discord.gg/dopamina
        ImVec4 track_bg = ImVec4(18.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 0.95f);
        ImVec4 track_bg_detail = ImVec4(12.0f / 255.0f, 12.0f / 255.0f, 16.0f / 255.0f, 0.95f);
        ImVec4 track_fill = ImVec4(179.0f / 255.0f, 143.0f / 255.0f, 228.0f / 255.0f, 1.0f);
        ImVec4 text_color = ImVec4(0.88f, 0.88f, 0.88f, 1.0f);
        
        // discord.gg/dopamina
        float shadow_strength = 0.5f + (0.2f * anim->hover_alpha);
        float shadow_blur = SCALE(15.0f);
        float shadow_offset = SCALE(2.5f * anim->hover_alpha);
        
        for (int s = 0; s < 4; s++)
        {
            float shadow_progress = (float)s / 4.0f;
            float shadow_alpha = (1.0f - shadow_progress) * 0.15f * shadow_strength;
            float blur_amount = shadow_blur * shadow_progress;
            
            gui->window_drawlist()->AddRectFilled(
                ImVec2(track_rect.Min.x - blur_amount + shadow_offset, track_rect.Min.y - blur_amount + shadow_offset),
                ImVec2(track_rect.Max.x + blur_amount + shadow_offset, track_rect.Max.y + blur_amount + shadow_offset),
                IM_COL32(0, 0, 0, (int)(shadow_alpha * 255)),
                SCALE(4.0f)
            );
        }
        
        // discord.gg/dopamina
        draw->rect_filled(gui->window_drawlist(), 
            ImVec2(track_rect.Min.x + SCALE(1.0f), track_rect.Min.y + SCALE(1.0f)), 
            track_rect.Max, 
            draw->get_clr(track_bg_detail), SCALE(4.0f));
        
        // discord.gg/dopamina
        draw->rect_filled(gui->window_drawlist(), track_rect.Min, track_rect.Max, 
            draw->get_clr(track_bg), SCALE(4.0f));
        
        // discord.gg/dopamina
        if (anim->fill_width > SCALE(5.0f))
        {
            ImVec2 fill_end = ImVec2(track_rect.Min.x + anim->fill_width, track_rect.Max.y);
            
            draw->rect_filled(gui->window_drawlist(), 
                ImVec2(track_rect.Min.x + SCALE(1.0f), track_rect.Min.y + SCALE(1.0f)), 
                fill_end, 
                draw->get_clr(track_fill), SCALE(4.0f));
        }
        
        // discord.gg/dopamina
        ImFont* text_font = font->get(main_font_data, 13.0f);
        if (!text_font)
            return;
        
        // Label
        draw->text(gui->window_drawlist(), text_font, 13.0f,
            ImVec2(full_rect.Min.x, full_rect.Min.y), 
            draw->get_clr(text_color), name.data());
        
        // discord.gg/dopamina
        if (anim->hover_alpha > 0.01f)
        {
            char value_buf[64];
            sprintf_s(value_buf, sizeof(value_buf), "%d", *value);
            
            ImVec2 value_text_size = gui->text_size(text_font, value_buf);
            ImVec2 box_padding = ImVec2(SCALE(8.0f), SCALE(4.0f));
            ImVec2 box_size = ImVec2(value_text_size.x + box_padding.x * 2.0f, value_text_size.y + box_padding.y * 2.0f);
            ImVec2 box_pos = ImVec2(
                track_rect.Min.x + anim->fill_width - (box_size.x * 0.5f),
                track_rect.Min.y - box_size.y - SCALE(6.0f)
            );
            
            // discord.gg/dopamina
            float shadow_blur = SCALE(12.0f);
            for (int s = 0; s < 6; s++)
            {
                float shadow_progress = (float)s / 6.0f;
                float shadow_alpha = (1.0f - shadow_progress) * 0.25f * anim->hover_alpha;
                float blur_amount = shadow_blur * shadow_progress;
                
                gui->window_drawlist()->AddRectFilled(
                    ImVec2(box_pos.x - blur_amount, box_pos.y - blur_amount),
                    ImVec2(box_pos.x + box_size.x + blur_amount, box_pos.y + box_size.y + blur_amount),
                    IM_COL32(0, 0, 0, (int)(shadow_alpha * 255)),
                    SCALE(4.0f)
                );
            }
            
            // discord.gg/dopamina
            gui->window_drawlist()->PushClipRect(box_pos, ImVec2(box_pos.x + box_size.x, box_pos.y + box_size.y), true);
            
            // discord.gg/dopamina
            ImVec4 box_bg_top = ImVec4(18.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 0.95f * anim->hover_alpha);
            ImVec4 box_bg_bottom = ImVec4(10.0f / 255.0f, 10.0f / 255.0f, 14.0f / 255.0f, 0.95f * anim->hover_alpha);
            
            gui->window_drawlist()->AddRectFilledMultiColor(
                box_pos,
                ImVec2(box_pos.x + box_size.x, box_pos.y + box_size.y),
                draw->get_clr(box_bg_top),
                draw->get_clr(box_bg_top),
                draw->get_clr(box_bg_bottom),
                draw->get_clr(box_bg_bottom)
            );
            
            // discord.gg/dopamina
            for (int b = 0; b < 8; b++)
            {
                float blur_inset = SCALE(b * 0.5f);
                float blur_alpha = 0.06f * anim->hover_alpha;
                
                float brightness = (b % 2 == 0) ? 30.0f : 8.0f;
                ImVec4 blur_color = ImVec4(brightness / 255.0f, brightness / 255.0f, (brightness + 3.0f) / 255.0f, blur_alpha);
                
                gui->window_drawlist()->AddRectFilled(
                    ImVec2(box_pos.x + blur_inset, box_pos.y + blur_inset),
                    ImVec2(box_pos.x + box_size.x - blur_inset, box_pos.y + box_size.y - blur_inset),
                    draw->get_clr(blur_color),
                    ImMax(SCALE(4.0f) - blur_inset, 0.0f)
                );
            }
            
            gui->window_drawlist()->PopClipRect();
            
            // Value text
            ImVec2 text_pos = ImVec2(box_pos.x + box_padding.x, box_pos.y + box_padding.y);
            draw->text(gui->window_drawlist(), text_font, 13.0f, text_pos, 
                draw->get_clr(text_color, anim->hover_alpha), value_buf);
        }
        
        // Tooltip
        render_tooltip(name, hovered);
        
        // Advance cursor
        gui->item_size(full_rect);
        gui->item_add(full_rect, id);
    }
    catch (...)
    {
        // Catch any crash in slider
    }
}
