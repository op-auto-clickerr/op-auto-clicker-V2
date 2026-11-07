#include "../headers/functions.h"
#include "../headers/widgets.h"
#include "keybinds.h"

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
std::string c_keybinds::vk_to_string(int vk)
{
    if (vk == 0)             return "NONE";
    if (vk == VK_LBUTTON)   return "M1";
    if (vk == VK_RBUTTON)   return "M2";
    if (vk == VK_MBUTTON)   return "M3";
    if (vk == VK_XBUTTON1)  return "M4";
    if (vk == VK_XBUTTON2)  return "M5";
    if (vk == VK_SPACE)     return "SPACE";
    if (vk == VK_RETURN)    return "ENTER";
    if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) return "SHIFT";
    if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL) return "CTRL";
    if (vk == VK_MENU)      return "ALT";
    if (vk == VK_DELETE)    return "DEL";
    if (vk == VK_INSERT)    return "INS";
    if (vk == VK_HOME)      return "HOME";
    if (vk == VK_END)       return "END";
    if (vk == VK_PRIOR)     return "PGUP";
    if (vk == VK_NEXT)      return "PGDN";
    if (vk == VK_ESCAPE)    return "ESC";
    if (vk == VK_TAB)       return "TAB";
    if (vk == VK_CAPITAL)   return "CAPS";

    // F1-F12
    if (vk >= VK_F1 && vk <= VK_F12)
    {
        char buf[8];
        sprintf_s(buf, "F%d", vk - VK_F1 + 1);
        return buf;
    }

    // Numpad
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
    {
        char buf[8];
        sprintf_s(buf, "NUM%d", vk - VK_NUMPAD0);
        return buf;
    }

    // A-Z / 0-9
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
    {
        char buf[4] = { (char)vk, 0 };
        return buf;
    }

    char buf[32];
    sprintf_s(buf, "KEY%d", vk);
    return buf;
}


void c_keybinds::add_keybind(const std::string& name, int key,
                              keybind_mode mode, bool active)
{
    // Doppelte verhindern
    for (auto& kb : keybinds)
        if (kb.name == name) return;

    keybind_item item;
    item.name   = name;
    item.key    = key;
    item.mode   = mode;
    item.active = active;
    item.state  = (mode == keybind_mode::always);
    keybinds.push_back(item);
}


bool c_keybinds::is_active(const std::string& name)
{
    for (auto& kb : keybinds)
        if (kb.name == name) return kb.state;
    return false;
}

void c_keybinds::update()
{
    for (auto& kb : keybinds)
    {
        // discord.gg/dopamina
        if (kb.listening)
        {
            // ESC = abbrechen
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                kb.listening = false;
                continue;
            }

            // discord.gg/dopamina
            static const int scan_keys[] = {
                VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2,
                VK_SPACE, VK_RETURN, VK_SHIFT, VK_CONTROL, VK_MENU,
                VK_DELETE, VK_INSERT, VK_HOME, VK_END, VK_PRIOR, VK_NEXT,
                VK_TAB, VK_CAPITAL,
                VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,
                VK_F7,VK_F8,VK_F9,VK_F10,VK_F11,VK_F12,
                VK_NUMPAD0,VK_NUMPAD1,VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD4,
                VK_NUMPAD5,VK_NUMPAD6,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9,
            };
            for (int vk : scan_keys)
            {
                if (GetAsyncKeyState(vk) & 0x8000)
                {
                    kb.key       = vk;
                    kb.listening = false;
                    break;
                }
            }
            // A-Z, 0-9
            if (kb.listening)
            {
                for (int vk = '0'; vk <= 'Z'; vk++)
                {
                    if (GetAsyncKeyState(vk) & 0x8000)
                    {
                        kb.key       = vk;
                        kb.listening = false;
                        break;
                    }
                }
            }
            continue; // discord.gg/dopamina
        }

        // discord.gg/dopamina
        switch (kb.mode)
        {
        case keybind_mode::always:
            kb.state = true;
            break;

        case keybind_mode::hold:
            if (kb.key != 0)
                kb.state = (GetAsyncKeyState(kb.key) & 0x8000) != 0;
            else
                kb.state = false;
            break;

        case keybind_mode::toggle:
        {
            if (kb.key != 0)
            {
                static std::unordered_map<std::string, bool> prev_down;
                bool down = (GetAsyncKeyState(kb.key) & 0x8000) != 0;
                if (down && !prev_down[kb.name])
                    kb.state = !kb.state;
                prev_down[kb.name] = down;
            }
            break;
        }
        }
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
void c_keybinds::render()
{
    if (!gui || !draw || !font)
        return;

    // Check if should render at bottom center (new style)
    if (settings::menu::keybind_indicator)
    {
        // Use new bottom-center style
        float spawn_alpha = 1.0f; // Always visible when enabled
        
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        ImFont* text_font = font->get(main_font_data, 14.0f);
        if (!text_font) return;

        // Collect active keybinds
        std::vector<keybind_item*> active_kb;
        for (auto& kb : keybinds)
            if (kb.active && kb.state)
                active_kb.push_back(&kb);

        if (active_kb.empty()) return;

        // Calculate position (bottom center)
        float item_height = 25.0f;
        float spacing = 8.0f;
        float total_width = 0.0f;

        for (auto* kb : active_kb)
        {
            ImVec2 text_size = text_font->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, kb->name.c_str());
            total_width += text_size.x + 20.0f + spacing;
        }

        float start_x = (io.DisplaySize.x - total_width) * 0.5f;
        float start_y = io.DisplaySize.y - item_height - 20.0f;

        // Draw each keybind
        float current_x = start_x;
        for (auto* kb : active_kb)
        {
            ImVec2 text_size = text_font->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, kb->name.c_str());
            float item_width = text_size.x + 20.0f;

            ImVec2 bg_min(current_x, start_y);
            ImVec2 bg_max(current_x + item_width, start_y + item_height);

            // Determine colors based on mode
            ImU32 bg_color, text_color, border_color;
            
            if (kb->mode == keybind_mode::hold || kb->mode == keybind_mode::always)
            {
                // Active: Red
                bg_color = IM_COL32(
                    static_cast<int>(settings::menu::keybind_indicator_active_color[0] * 255.f),
                    static_cast<int>(settings::menu::keybind_indicator_active_color[1] * 255.f),
                    static_cast<int>(settings::menu::keybind_indicator_active_color[2] * 255.f),
                    220
                );
                text_color = IM_COL32(255, 255, 255, 255);
                border_color = IM_COL32(255, 100, 100, 200);
            }
            else
            {
                // Toggle: White
                bg_color = IM_COL32(
                    static_cast<int>(settings::menu::keybind_indicator_inactive_color[0] * 255.f),
                    static_cast<int>(settings::menu::keybind_indicator_inactive_color[1] * 255.f),
                    static_cast<int>(settings::menu::keybind_indicator_inactive_color[2] * 255.f),
                    200
                );
                text_color = IM_COL32(20, 20, 20, 255);
                border_color = IM_COL32(200, 200, 200, 150);
            }

            // Shadow
            draw_list->AddRectFilled(
                ImVec2(bg_min.x + 2, bg_min.y + 2),
                ImVec2(bg_max.x + 2, bg_max.y + 2),
                IM_COL32(0, 0, 0, 120),
                4.0f
            );
            
            // Background
            draw_list->AddRectFilled(bg_min, bg_max, bg_color, 4.0f);
            
            // Border
            draw_list->AddRect(bg_min, bg_max, border_color, 4.0f, 0, 1.5f);

            // Text
            ImVec2 text_pos(current_x + 10.0f, start_y + (item_height - text_size.y) * 0.5f);
            draw_list->AddText(text_font, 14.0f, text_pos, text_color, kb->name.c_str());

            current_x += item_width + spacing;
        }
        
        return;
    }

    // Old style (top-left draggable window)
    float spawn_alpha = var->gui.active_hotkeys_spawn_alpha;
    if (spawn_alpha <= 0.01f)
        return;

    // discord.gg/dopamina
    float anim_alpha  = spawn_alpha;
    float scale       = 1.0f; // discord.gg/dopamina

    // Defaults
    if (keybinds.empty())
    {
        add_keybind("Aimbot",     VK_XBUTTON2, keybind_mode::hold,   true);
        add_keybind("ESP",        VK_INSERT,   keybind_mode::toggle, true);
        add_keybind("Triggerbot", 'F',         keybind_mode::hold,   false);
        add_keybind("Bhop",       VK_SPACE,    keybind_mode::toggle, true);
    }

    try
    {
        ImGuiIO&    io        = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();

        ImFont* text_font = font->get(main_font_data, 13.0f);
        ImFont* icon_font = font->get(main_font_data, 14.0f);
        if (!text_font || !icon_font) return;

        float header_height = SCALE(28.0f);
        float item_height   = SCALE(26.0f);
        float padding       = SCALE(12.0f);
        float width         = SCALE(200.0f);

        // discord.gg/dopamina
        int active_count = 0;
        for (auto& kb : keybinds)
            if (kb.active) active_count++;
        if (active_count == 0) return;

        float total_height = header_height + active_count * item_height + SCALE(8.0f);

        ImVec2 pos  = keybind_pos;
        ImVec2 size = ImVec2(width, total_height);
        
        // 3D Scale-Transformation
        ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        ImVec2 scaled_pos = ImVec2(
            center.x + (pos.x - center.x) * scale,
            center.y + (pos.y - center.y) * scale
        );
        ImVec2 scaled_size = ImVec2(size.x * scale, size.y * scale);

        // ── Dragging ────────────────────────────────────────────────
        ImRect header_rect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + header_height * scale));
        bool   header_hovered = header_rect.Contains(ImGui::GetMousePos());
        bool   menu_open      = var->gui.menu_open;

        if (menu_open && header_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            is_dragging = true;
            drag_offset = ImVec2(ImGui::GetMousePos().x - pos.x,
                                 ImGui::GetMousePos().y - pos.y);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            is_dragging = false;

        if (is_dragging)
        {
            keybind_pos.x = ImClamp(ImGui::GetMousePos().x - drag_offset.x,
                                    0.0f, io.DisplaySize.x - width);
            keybind_pos.y = ImClamp(ImGui::GetMousePos().y - drag_offset.y,
                                    0.0f, io.DisplaySize.y - total_height);
            pos = keybind_pos;
            
            // Update scaled values
            center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
            scaled_pos = ImVec2(
                center.x + (pos.x - center.x) * scale,
                center.y + (pos.y - center.y) * scale
            );
            scaled_size = ImVec2(size.x * scale, size.y * scale);
        }

        // discord.gg/dopamina
        float shadow_blur = SCALE(12.0f);
        for (int s = 0; s < 6; s++)
        {
            float p  = (float)s / 6.0f;
            float a  = (1.0f - p) * 0.25f * anim_alpha;
            float bl = shadow_blur * p;
            draw_list->AddRectFilled(
                ImVec2(scaled_pos.x - bl, scaled_pos.y - bl),
                ImVec2(scaled_pos.x + scaled_size.x + bl, scaled_pos.y + scaled_size.y + bl),
                IM_COL32(0, 0, 0, (int)(a * 255)),
                SCALE(6.0f) + bl);
        }

        // discord.gg/dopamina
        draw_list->PushClipRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y), true);

        ImVec4 bg_top    = ImVec4(18.f/255, 18.f/255, 22.f/255, 0.95f * anim_alpha);
        ImVec4 bg_bottom = ImVec4(10.f/255, 10.f/255, 14.f/255, 0.95f * anim_alpha);
        draw_list->AddRectFilledMultiColor(
            scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y),
            draw->get_clr(bg_top),    draw->get_clr(bg_top),
            draw->get_clr(bg_bottom), draw->get_clr(bg_bottom));

        // Blur layers
        for (int b = 0; b < 16; b++)
        {
            float bi   = SCALE(b * 0.8f);
            float bri  = (b % 2 == 0) ? 45.f : 12.f;
            ImVec4 bc  = ImVec4(bri/255, bri/255, (bri+5)/255, 0.06f * anim_alpha);
            draw_list->AddRectFilled(
                ImVec2(scaled_pos.x + bi, scaled_pos.y + bi),
                ImVec2(scaled_pos.x + scaled_size.x - bi, scaled_pos.y + scaled_size.y - bi),
                draw->get_clr(bc),
                ImMax(SCALE(6.f) - bi, 0.f));
        }

        // Shimmer// discord.gg/dopamina

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
        static float shimmer_time = 0.f;
        shimmer_time += io.DeltaTime * 0.25f;
        float sp  = fmodf(shimmer_time, 1.0f);
        float sw  = SCALE(60.f);
        float so  = -(sw * 1.5f);
        float eo  = scaled_size.x + scaled_size.y + sw * 1.5f;
        float co  = so + (eo - so) * sp;
        for (int i = 0; i < 15; i++)
        {
            float t   = (float)i / 15.f;
            float nt  = (float)(i+1) / 15.f;
            float am  = (1.f - fabsf(t - 0.5f) * 2.f) * anim_alpha;
            float sof = (t  - 0.5f) * sw;
            float nsof= (nt - 0.5f) * sw;
            float d   = scaled_size.x + scaled_size.y;
            draw_list->AddQuadFilled(
                ImVec2(scaled_pos.x + co + sof,      scaled_pos.y - co - sof),
                ImVec2(scaled_pos.x + co + nsof,     scaled_pos.y - co - nsof),
                ImVec2(scaled_pos.x + co + nsof + d, scaled_pos.y - co - nsof + d),
                ImVec2(scaled_pos.x + co + sof  + d, scaled_pos.y - co - sof  + d),
                IM_COL32(179, 143, 228, (int)(50 * am)));
        }

        draw_list->PopClipRect();

        // Border glow
        draw_list->AddRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y),
            IM_COL32(179, 143, 228, (int)(38 * anim_alpha)), SCALE(6.f), 0, SCALE(1.5f));

        // Header bg
        draw_list->AddRectFilled(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + header_height * scale),
            IM_COL32(12, 12, 16, (int)(230 * anim_alpha)), SCALE(6.f), ImDrawFlags_RoundCornersTop);

        // Header icon + text
        const char* hdr_icon = "\xEF\x81\xB4"; // U+F074
        ImVec2      hdr_icon_sz = icon_font->CalcTextSizeA(14.f, FLT_MAX, 0, hdr_icon);
        
        // discord.gg/dopamina
        if (spawn_alpha < 0.99f)
        {
            // discord.gg/dopamina
            static float loading_time = 0.0f;
            loading_time += io.DeltaTime * 0.8f; // discord.gg/dopamina
            
            float loading_radius = SCALE(7.0f) * scale;
            ImVec2 loading_center = ImVec2(
                scaled_pos.x + SCALE(10) * scale + loading_radius,
                scaled_pos.y + header_height * scale * 0.5f
            );
            
            // discord.gg/dopamina
            for (int i = 0; i < 3; i++)
            {
                float angle = loading_time + (i * 2.0f * IM_PI / 3.0f);
                float dot_x = loading_center.x + cosf(angle) * loading_radius * 0.5f;
                float dot_y = loading_center.y + sinf(angle) * loading_radius * 0.5f;
                
                // discord.gg/dopamina
                float wave = (sinf(loading_time * 1.5f + i * 0.8f) + 1.0f) * 0.5f;
                int dot_alpha = (int)(120 * anim_alpha * (0.5f + wave * 0.5f)); // discord.gg/dopamina
                
                draw_list->AddCircleFilled(
                    ImVec2(dot_x, dot_y),
                    SCALE(2.0f) * scale, // discord.gg/dopamina
                    IM_COL32(179, 143, 228, dot_alpha)
                );
            }
        }
        else
        {
            // discord.gg/dopamina
            draw_list->AddText(icon_font, 14.f,
                ImVec2(scaled_pos.x + SCALE(10) * scale, scaled_pos.y + (header_height * scale - hdr_icon_sz.y) * 0.5f),
                IM_COL32(179, 143, 228, (int)(255 * anim_alpha)), hdr_icon);
        }

        const char* hdr_text = "active keybinds";
        ImVec2      hdr_text_sz = text_font->CalcTextSizeA(13.f, FLT_MAX, 0, hdr_text);
        draw_list->AddText(text_font, 13.f,
            ImVec2(scaled_pos.x + (SCALE(10) + hdr_icon_sz.x + SCALE(6)) * scale,
                   scaled_pos.y + (header_height * scale - hdr_text_sz.y) * 0.5f),
            IM_COL32(230, 230, 230, (int)(255 * anim_alpha)), hdr_text);

        // discord.gg/dopamina
        float item_y = scaled_pos.y + header_height * scale + SCALE(4.f) * scale;
        int   drawn  = 0;

        float item_scaled_h = item_height * scale;
        float pad_scaled    = padding * scale;

        for (int idx = 0; idx < (int)keybinds.size(); idx++)
        {
            auto& kb = keybinds[idx];
            
            // discord.gg/dopamina
            if (!kb.active || !kb.state) continue;

            ImRect item_rect(
                ImVec2(scaled_pos.x, item_y),
                ImVec2(scaled_pos.x + scaled_size.x, item_y + item_scaled_h));

            // ── Feature Name ─────────────────────────────────────
            draw_list->AddText(text_font, 13.f,
                ImVec2(scaled_pos.x + pad_scaled, item_y + (item_scaled_h - SCALE(13.f)) * 0.5f),
                IM_COL32(220, 220, 225, (int)(255 * anim_alpha)), kb.name.c_str());

            // discord.gg/dopamina
            std::string key_str = vk_to_string(kb.key);
            ImVec2 key_sz = text_font->CalcTextSizeA(11.f, FLT_MAX, 0, key_str.c_str());
            
            draw_list->AddText(text_font, 11.f,
                ImVec2(scaled_pos.x + scaled_size.x - pad_scaled - key_sz.x,
                       item_y + (item_scaled_h - SCALE(11.f)) * 0.5f),
                IM_COL32(150, 150, 160, (int)(255 * anim_alpha)), 
                key_str.c_str());

            // ── Status Dot (zeigt Mode) ──────────────────────────
            ImVec2 dot_pos = ImVec2(scaled_pos.x + scaled_size.x - pad_scaled - key_sz.x - SCALE(12.f) * scale,
                                    item_y + item_scaled_h * 0.5f);
            ImU32 dot_clr;
            switch (kb.mode)
            {
            case keybind_mode::always: dot_clr = IM_COL32(100, 220, 130, (int)(200 * anim_alpha)); break;
            case keybind_mode::hold:   dot_clr = IM_COL32(100, 170, 255, (int)(200 * anim_alpha)); break;
            default:                   dot_clr = IM_COL32(179, 143, 228, (int)(200 * anim_alpha)); break;
            }
            draw_list->AddCircleFilled(dot_pos, SCALE(2.5f) * scale, dot_clr);

            item_y += item_scaled_h;
            drawn++;
        }
    }
    catch (...) {}
}
