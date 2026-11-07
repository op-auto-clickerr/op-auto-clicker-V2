#include "headers/includes.h"
#include "data/IconsFontAwesome6.h"
#include "data/IconsV2_Defines.h"
#include "widgets/watermark.h"
#include "widgets/keybinds.h"
#include "widgets/playerbar.h"
#include "widgets/musicplayer.h"
#include "widgets/playerlist.h"

void c_gui::render()
{
	// discord.gg/dopamina
	gui->easing(var->gui.watermark_spawn_alpha,      var->gui.show_watermark       ? 1.0f : 0.0f, 8.0f, dynamic_easing);
	gui->easing(var->gui.active_hotkeys_spawn_alpha, var->gui.show_active_hotkeys  ? 1.0f : 0.0f, 4.0f, dynamic_easing); // discord.gg/dopamina
	gui->easing(var->gui.playerbar_spawn_alpha,      var->gui.show_playerbar       ? 1.0f : 0.0f, 8.0f, dynamic_easing);
	gui->easing(var->gui.spotify_spawn_alpha,        var->gui.show_spotify         ? 1.0f : 0.0f, 8.0f, dynamic_easing);
	gui->easing(var->gui.playerlist_spawn_alpha,     var->gui.show_playerlist      ? 1.0f : 0.0f, 8.0f, dynamic_easing);

	// discord.gg/dopamina
	if (!var->gui.menu_open)
	{
		var->gui.menu_was_closed = true;
		// discord.gg/dopamina
		if (var->gui.watermark_spawn_alpha > 0.001f)  watermark->render();
		if (var->gui.active_hotkeys_spawn_alpha > 0.001f) { keybinds->update(); keybinds->render(); }
		if (var->gui.playerbar_spawn_alpha > 0.001f)  playerbar->render();
		if (var->gui.spotify_spawn_alpha > 0.001f)    { musicplayer->update(); musicplayer->render(); }
		if (var->gui.playerlist_spawn_alpha > 0.001f) playerlist->render();
		return;
	}

	// discord.gg/dopamina
	if (var->gui.menu_was_closed)
	{
		var->gui.menu_open_alpha = 0.0f;
		var->gui.menu_was_closed = false;
	}

	try
	{
		// discord.gg/dopamina
		gui->easing(var->gui.menu_open_alpha, 1.0f, 6.0f, dynamic_easing);
		
		// discord.gg/dopamina
		float scale_progress = var->gui.menu_open_alpha;
		scale_progress = scale_progress * scale_progress;  // discord.gg/dopamina
		float menu_scale = 0.85f + (scale_progress * 0.15f);  // discord.gg/dopamina
		
		// Load pattern texture if not loaded yet
		static bool pattern_loaded = false;
		static bool pattern_load_attempted = false;
		if (!pattern_load_attempted)
		{
			try {
				LoadPatternTexture();
				pattern_loaded = true;
			}
			catch (...) {
				// Pattern loading failed, continue without pattern
				pattern_loaded = false;
			}
			pattern_load_attempted = true;
		}

		gui->initialize();
		gui->set_next_window_size(SCALE(elements->window.size));
		gui->begin(elements->window.name, nullptr, window_flags_no_scrollbar | window_flags_no_scroll_with_mouse | window_flags_no_bring_to_front_on_focus | window_flags_no_focus_on_appearing | window_flags_no_background | window_flags_no_decoration);
		{
			c_window* window = gui->get_window();
			c_vec2 pos = window->Pos;
			c_vec2 size = window->Size;
			
			// discord.gg/dopamina
			ImVec2 menu_center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
			ImVec2 scaled_pos = ImVec2(
				menu_center.x + (pos.x - menu_center.x) * menu_scale,
				menu_center.y + (pos.y - menu_center.y) * menu_scale
			);
			ImVec2 scaled_size = ImVec2(size.x * menu_scale, size.y * menu_scale);
			
			c_rect rect = c_rect(scaled_pos, scaled_pos + scaled_size);
			c_draw_list* draw_list = gui->background_drawlist();

			// discord.gg/dopamina
			float menu_shadow_blur = SCALE(12.0f);
			for (int s = 0; s < 6; s++)
			{
				float progress = (float)s / 6.0f;
				float alpha = (1.0f - progress) * 0.35f * var->gui.menu_open_alpha;  // discord.gg/dopamina
				float blur_amount = menu_shadow_blur * progress;
				ImVec2 shadow_min = ImVec2(scaled_pos.x - blur_amount, scaled_pos.y - blur_amount);
				ImVec2 shadow_max = ImVec2(scaled_pos.x + scaled_size.x + blur_amount, scaled_pos.y + scaled_size.y + blur_amount);
				
				draw_list->AddRectFilled(
					shadow_min,
					shadow_max,
					IM_COL32(0, 0, 0, (int)(alpha * 255)),
					SCALE(elements->window.rounding) + blur_amount
				);
			}

			// discord.gg/dopamina
			draw_list->PushClipRect(scaled_pos, ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y), true);

			// ── Bankroll menu background ────────────────────────────
			// Very dark near-black with subtle top-to-bottom gradient
			ImVec4 bg_top    = ImVec4(18.0f / 255.0f, 16.0f / 255.0f, 20.0f / 255.0f, 0.95f * var->gui.menu_open_alpha);
			ImVec4 bg_bottom = ImVec4(9.0f  / 255.0f,  9.0f / 255.0f,  9.0f / 255.0f, 0.95f * var->gui.menu_open_alpha);

			// discord.gg/dopamina
			draw_list->AddRectFilled(
				scaled_pos,
				ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y),
				draw->get_clr(bg_bottom),
				SCALE(elements->window.rounding)
			);

			// discord.gg/dopamina
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
				float blur_alpha = 0.06f;
				float brightness = (b % 2 == 0) ? 45.0f : 12.0f;
				ImVec4 blur_color = ImVec4(brightness / 255.0f, brightness / 255.0f, (brightness + 5.0f) / 255.0f, blur_alpha);
				
				draw_list->AddRectFilled(
					ImVec2(scaled_pos.x + blur_inset, scaled_pos.y + blur_inset),
					ImVec2(scaled_pos.x + scaled_size.x - blur_inset, scaled_pos.y + scaled_size.y - blur_inset),
					draw->get_clr(blur_color),
					ImMax(SCALE(elements->window.rounding) - blur_inset, 0.0f)
				);
			}

			draw_list->PopClipRect();

			// Bankroll purple border glow
			ImVec4 glow_color = ImVec4(168.0f / 255.0f, 130.0f / 255.0f, 210.0f / 255.0f, 0.15f * var->gui.menu_open_alpha);
			draw_list->AddRect(
				scaled_pos, 
				ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + scaled_size.y), 
				draw->get_clr(glow_color), 
				SCALE(elements->window.rounding), 
				0, 
				SCALE(1.5f)
			);

			// discord.gg/dopamina
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, var->gui.menu_open_alpha);

			// ── Bankroll topbar ─────────────────────────────────────
			float tabbar_height = SCALE(45.0f);
			ImVec2 tabbar_start = scaled_pos;
			ImVec2 tabbar_end = ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + tabbar_height);

			draw_list->PushClipRect(tabbar_start, tabbar_end, true);

			// Bankroll: very dark gradient topbar
			ImVec4 tabbar_top    = ImVec4(20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 1.0f);
			ImVec4 tabbar_bottom = ImVec4(7.0f / 255.0f,  7.0f / 255.0f,  7.0f / 255.0f,  1.0f);

			draw_list->AddRectFilled(
				tabbar_start, tabbar_end,
				draw->get_clr(tabbar_bottom),
				SCALE(elements->window.rounding), ImDrawFlags_RoundCornersTop
			);
			draw_list->AddRectFilledMultiColor(
				tabbar_start, tabbar_end,
				draw->get_clr(tabbar_top), draw->get_clr(tabbar_top),
				draw->get_clr(tabbar_bottom), draw->get_clr(tabbar_bottom)
			);

			// Bankroll gradient separator line across bottom of topbar
			{
				float lx = tabbar_start.x;
				float ly = tabbar_end.y;
				float lw = scaled_size.x;
				// centre glow fades out to edges
				draw_list->AddRectFilledMultiColor(
					ImVec2(lx,           ly - SCALE(1.0f)),
					ImVec2(lx + lw * 0.5f, ly),
					IM_COL32(0, 0, 0, 0),
					IM_COL32(168, 130, 210, 120),
					IM_COL32(168, 130, 210, 120),
					IM_COL32(0, 0, 0, 0)
				);
				draw_list->AddRectFilledMultiColor(
					ImVec2(lx + lw * 0.5f, ly - SCALE(1.0f)),
					ImVec2(lx + lw, ly),
					IM_COL32(168, 130, 210, 120),
					IM_COL32(0, 0, 0, 0),
					IM_COL32(0, 0, 0, 0),
					IM_COL32(168, 130, 210, 120)
				);
			}

			// "bankroll mafia" title – left side of topbar
			{
				ImFont* title_font = font->get(main_font_data, 13.0f);
				if (title_font)
				{
					const char* title = "bankroll mafia";
					ImVec2 ts = ImGui::CalcTextSize(title);
					float tx = tabbar_start.x + SCALE(15.0f);
					float ty = tabbar_start.y + (tabbar_height - ts.y) * 0.5f;
					// subtle drop-shadow
					draw_list->AddText(title_font, 13.0f,
						ImVec2(tx + SCALE(1.0f), ty + SCALE(1.0f)),
						IM_COL32(0, 0, 0, 80), title);
					draw_list->AddText(title_font, 13.0f,
						ImVec2(tx, ty),
						IM_COL32(185, 182, 196, 255), title);
				}
			}

			draw_list->PopClipRect();

			// Bankroll purple accent colours for tab icons
			ImVec4 accent_color   = ImVec4(168.0f / 255.0f, 130.0f / 255.0f, 210.0f / 255.0f, 1.0f);
			ImVec4 inactive_color = ImVec4(82.0f  / 255.0f,  79.0f / 255.0f,  95.0f / 255.0f, 1.0f);
			ImVec4 hover_color    = ImVec4(120.0f / 255.0f, 116.0f / 255.0f, 135.0f / 255.0f, 1.0f);
			
			// discord.gg/dopamina
			const char* tab_icons[] = { 
				"\xEF\x80\x98",    // U+F018 - Aimbot
				"\xEF\x81\x8B",    // U+F04B - Visuals
				"\xEF\x80\x8C",    // U+F00C - Misc
				"\xEF\x80\xAF",    // U+F02F - Configs
				"\xEF\x81\x94",    // U+F054 - Colors
				"\xEF\x80\x8D"     // U+F00D - Settings
			};
			int tab_count = 6;
			
			// discord.gg/dopamina
			float box_size = SCALE(32.0f);
			float tab_spacing = SCALE(8.0f); // discord.gg/dopamina
			float total_tabs_width = (tab_count * box_size) + ((tab_count - 1) * tab_spacing);
			float tabs_start_x = scaled_pos.x + scaled_size.x - total_tabs_width - SCALE(15.0f); // discord.gg/dopamina
			
			ImFont* tab_icon_font = font->get(main_font_data, 16.0f);
			
			if (tab_icon_font)
			{
				for (int i = 0; i < tab_count; i++)
				{
					float tab_x = tabs_start_x + (i * (box_size + tab_spacing));
					
					// discord.gg/dopamina
					ImVec2 box_pos = ImVec2(tab_x, scaled_pos.y + (tabbar_height - box_size) / 2.0f);
					ImVec2 box_end = ImVec2(box_pos.x + box_size, box_pos.y + box_size);
					
					// Hit detection
					c_rect tab_rect = c_rect(box_pos, box_end);
					
					bool hovered = IsMouseHoveringRect(tab_rect.Min, tab_rect.Max, false);
					bool clicked = hovered && gui->mouse_clicked(0, false);
					
					if (clicked)
					{
						if (var->gui.active_tab != i)  // discord.gg/dopamina
						{
							var->gui.active_tab = i;  // discord.gg/dopamina
							var->gui.tab_transition_alpha = 0.0f;  // discord.gg/dopamina
						}
					}
					
					bool is_active = (i == var->gui.active_tab);  // discord.gg/dopamina
					
					// discord.gg/dopamina
					float shadow_blur = SCALE(6.0f);
					
					// discord.gg/dopamina
					for (int s = 0; s < 5; s++)
					{
						float progress = (float)s / 5.0f;
						float alpha = (1.0f - progress) * 0.25f;
						float blur_amount = shadow_blur * progress;
						
						// discord.gg/dopamina
						ImVec2 shadow_min = ImVec2(box_pos.x - blur_amount, box_pos.y - blur_amount);
						ImVec2 shadow_max = ImVec2(box_end.x + blur_amount, box_end.y + blur_amount);
						
						draw_list->AddRectFilled(
							shadow_min,
							shadow_max,
							IM_COL32(0, 0, 0, (int)(alpha * 255)),
							SCALE(6.0f + blur_amount)
						);
					}
					
					// discord.gg/dopamina
					ImVec4 box_bg_top = ImVec4(18.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 1.0f);
					ImVec4 box_bg_bottom = ImVec4(12.0f / 255.0f, 12.0f / 255.0f, 16.0f / 255.0f, 1.0f);
					
					// discord.gg/dopamina
					draw_list->PushClipRect(box_pos, box_end, true);
					
					// discord.gg/dopamina
					draw_list->AddRectFilled(
						box_pos,
						box_end,
						draw->get_clr(box_bg_bottom),
						SCALE(6.0f)
					);
					
					// discord.gg/dopamina
					draw_list->AddRectFilledMultiColor(
						box_pos,
						box_end,
						draw->get_clr(box_bg_top),
						draw->get_clr(box_bg_top),
						draw->get_clr(box_bg_bottom),
						draw->get_clr(box_bg_bottom)
					);
					
					draw_list->PopClipRect();
					
					// discord.gg/dopamina
					ImVec4 current_color = is_active ? accent_color : (hovered ? hover_color : inactive_color);
					
					// discord.gg/dopamina
					gui->push_font(tab_icon_font);
					
					// discord.gg/dopamina
					ImVec2 icon_size = ImGui::CalcTextSize(tab_icons[i]);
					
					// discord.gg/dopamina
					ImVec2 centered_icon_pos = ImVec2(
						box_pos.x + (box_size - icon_size.x) * 0.5f,
						box_pos.y + (box_size - icon_size.y) * 0.5f + SCALE(1.0f) // discord.gg/dopamina
					);
					
					// discord.gg/dopamina
					if (is_active)
					{
						// discord.gg/dopamina
						for (int g = 2; g > 0; g--)
						{
							float glow_progress = (float)g / 2.0f;
							float glow_alpha = (1.0f - glow_progress) * 0.25f; // discord.gg/dopamina
							float glow_offset = SCALE(glow_progress * 1.5f); // discord.gg/dopamina
							
							// discord.gg/dopamina
							ImVec2 offsets[] = {
								ImVec2(-glow_offset, 0),
								ImVec2(glow_offset, 0),
								ImVec2(0, -glow_offset),
								ImVec2(0, glow_offset)
							};
							
							for (int d = 0; d < 4; d++)
							{
								ImVec2 glow_pos = ImVec2(
									centered_icon_pos.x + offsets[d].x,
									centered_icon_pos.y + offsets[d].y
								);
								
								draw_list->AddText(
									tab_icon_font, 
									16.0f, 
									glow_pos, 
									IM_COL32(179, 143, 228, (int)(glow_alpha * 255)),  // discord.gg/dopamina
									tab_icons[i]
								);
							}
						}
					}
					
					// discord.gg/dopamina
					draw_list->AddText(tab_icon_font, 16.0f, centered_icon_pos, draw->get_clr(current_color), tab_icons[i]);
					
					gui->pop_font();
				}
			}
			
			// discord.gg/dopamina
			float shadow_total_height = SCALE(20.0f); // discord.gg/dopamina
			int shadow_layers = 12; // discord.gg/dopamina
			
			for (int i = 0; i < shadow_layers; i++)
			{
				float layer_progress = (float)i / (float)shadow_layers;
				float next_progress = (float)(i + 1) / (float)shadow_layers;
				
				// discord.gg/dopamina
				float alpha_start = (1.0f - layer_progress) * (1.0f - layer_progress) * 0.6f * var->gui.menu_open_alpha; // discord.gg/dopamina
				float alpha_end = (1.0f - next_progress) * (1.0f - next_progress) * 0.6f * var->gui.menu_open_alpha;
				
				ImVec2 layer_start = ImVec2(scaled_pos.x, scaled_pos.y + tabbar_height + (shadow_total_height * layer_progress));
				ImVec2 layer_end = ImVec2(scaled_pos.x + scaled_size.x, scaled_pos.y + tabbar_height + (shadow_total_height * next_progress));
				
				draw_list->AddRectFilledMultiColor(
					layer_start,
					layer_end,
					draw->get_clr(ImVec4(0.0f, 0.0f, 0.0f, alpha_start)), // discord.gg/dopamina
					draw->get_clr(ImVec4(0.0f, 0.0f, 0.0f, alpha_start)), // discord.gg/dopamina
					draw->get_clr(ImVec4(0.0f, 0.0f, 0.0f, alpha_end)),   // discord.gg/dopamina
					draw->get_clr(ImVec4(0.0f, 0.0f, 0.0f, alpha_end))    // discord.gg/dopamina
				);
			}
			
			// discord.gg/dopamina
			float content_padding = SCALE(20.0f);
			float content_y = scaled_pos.y + tabbar_height + SCALE(25.0f); // discord.gg/dopamina
			ImVec2 content_pos = ImVec2(scaled_pos.x + content_padding, content_y);
			float content_width = scaled_size.x - (content_padding * 2);
			float content_height = scaled_size.y - tabbar_height - SCALE(25.0f) - content_padding;
			
			// discord.gg/dopamina
			// discord.gg/dopamina
			// discord.gg/dopamina
			gui->easing(var->gui.tab_transition_alpha, 1.0f, 6.0f, dynamic_easing);  // Smooth fade
			
			float child_spacing = SCALE(15.0f);
			float child_width = (content_width - child_spacing) / 2.0f; // discord.gg/dopamina
			float child_height = (content_height - child_spacing) / 2.0f; // discord.gg/dopamina
			
			// discord.gg/dopamina
			ImVec4 child_bg_top = ImVec4(18.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 1.0f);
			ImVec4 child_bg_bottom = ImVec4(10.0f / 255.0f, 10.0f / 255.0f, 14.0f / 255.0f, 1.0f);
			
			// discord.gg/dopamina
			ImVec4 child_header_top = ImVec4(18.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 1.0f);
			ImVec4 child_header_bottom = ImVec4(14.0f / 255.0f, 14.0f / 255.0f, 18.0f / 255.0f, 1.0f);
			
			// discord.gg/dopamina
			const char* child_titles[6][4] = {
				// Tab 0: Aimbot
				{ "targeting", "smoothing", "recoil", "triggerbot" },
				// Tab 1: Visuals
				{ "esp", "chams", "glow", "radar" },
				// Tab 2: Misc
				{ "movement", "automation", "exploits", "utilities" },
				// Tab 3: Config
				{ "profiles", "keybinds", "themes", "about" },
				// Tab 4: Colors
				{ "colors", "", "", "" },
				// Tab 5: Settings
				{ "settings", "", "", "" }
			};
			
			float child_header_height = SCALE(32.0f);
			ImFont* child_title_font = font->get(main_font_data, 15.0f);
			
			// discord.gg/dopamina
			static bool test_checks[6][16] = { false };  // [tab][checkbox]
			
			// discord.gg/dopamina
			static int test_sliders[6][8] = { 0 };
			
			// discord.gg/dopamina
			static float test_colors[6][20][4] = {
				{
					{1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f},
					{1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.0f, 1.0f}, {0.5f, 0.0f, 1.0f, 1.0f}
				},
				{
					{0.8f, 0.2f, 0.2f, 1.0f}, {0.2f, 0.8f, 0.2f, 1.0f}, {0.2f, 0.2f, 0.8f, 1.0f}, {0.8f, 0.8f, 0.2f, 1.0f},
					{0.8f, 0.2f, 0.8f, 1.0f}, {0.2f, 0.8f, 0.8f, 1.0f}, {0.8f, 0.5f, 0.2f, 1.0f}, {0.5f, 0.2f, 0.8f, 1.0f}
				},
				{
					{0.6f, 0.1f, 0.1f, 1.0f}, {0.1f, 0.6f, 0.1f, 1.0f}, {0.1f, 0.1f, 0.6f, 1.0f}, {0.6f, 0.6f, 0.1f, 1.0f},
					{0.6f, 0.1f, 0.6f, 1.0f}, {0.1f, 0.6f, 0.6f, 1.0f}, {0.6f, 0.3f, 0.1f, 1.0f}, {0.3f, 0.1f, 0.6f, 1.0f}
				},
				{
					{0.9f, 0.3f, 0.3f, 1.0f}, {0.3f, 0.9f, 0.3f, 1.0f}, {0.3f, 0.3f, 0.9f, 1.0f}, {0.9f, 0.9f, 0.3f, 1.0f},
					{0.9f, 0.3f, 0.9f, 1.0f}, {0.3f, 0.9f, 0.9f, 1.0f}, {0.9f, 0.6f, 0.3f, 1.0f}, {0.6f, 0.3f, 0.9f, 1.0f}
				},
				// Tab 4: Colors
				{
					{179.0f/255.0f, 143.0f/255.0f, 228.0f/255.0f, 1.0f},
					{1.0f, 0.2f, 0.2f, 1.0f}, {0.2f, 1.0f, 0.2f, 1.0f}, {1.0f, 1.0f, 0.2f, 1.0f},
					{0.2f, 0.8f, 1.0f, 1.0f}, {1.0f, 0.2f, 0.8f, 1.0f}, {0.8f, 1.0f, 0.2f, 1.0f}, {1.0f, 0.5f, 0.0f, 1.0f},
					{0.6f, 0.2f, 1.0f, 1.0f}, {1.0f, 0.2f, 0.2f, 0.5f}, {0.2f, 1.0f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f},
					{1.0f, 0.8f, 0.2f, 1.0f}, {0.2f, 0.5f, 1.0f, 1.0f}, {1.0f, 0.3f, 0.8f, 1.0f},
					{179.0f/255.0f, 143.0f/255.0f, 228.0f/255.0f, 1.0f},
					{0.3f, 0.9f, 0.9f, 1.0f}, {1.0f, 0.5f, 0.2f, 1.0f}, {1.0f, 0.2f, 0.2f, 1.0f}, {0.5f, 0.5f, 0.8f, 1.0f}
				},
				// discord.gg/dopamina
				{}
			};
			
			// discord.gg/dopamina
			static int test_dropdowns[6][8] = { 0 };
			
			// discord.gg/dopamina
			if (var->gui.active_tab == 5)
			{
				// discord.gg/dopamina
				float large_child_width = content_width;
				float large_child_height = content_height;
				
				ImVec2 child_pos = content_pos;
				ImVec2 child_end = ImVec2(child_pos.x + large_child_width, child_pos.y + large_child_height);
				
				// discord.gg/dopamina
				float scale_progress = var->gui.tab_transition_alpha;
				scale_progress = scale_progress * scale_progress;
				float scale_effect = 0.85f + (scale_progress * 0.15f);
				
				ImVec2 center = ImVec2((child_pos.x + child_end.x) * 0.5f, (child_pos.y + child_end.y) * 0.5f);
				ImVec2 scaled_min = ImVec2(
					center.x + (child_pos.x - center.x) * scale_effect,
					center.y + (child_pos.y - center.y) * scale_effect
				);
				ImVec2 scaled_max = ImVec2(
					center.x + (child_end.x - center.x) * scale_effect,
					center.y + (child_end.y - center.y) * scale_effect
				);
				
				// discord.gg/dopamina
				float shadow_blur = SCALE(8.0f);
				for (int s = 0; s < 5; s++)
				{
					float progress = (float)s / 5.0f;
					float alpha = (1.0f - progress) * 0.25f * var->gui.tab_transition_alpha;
					float blur_amount = shadow_blur * progress;
					
					ImVec2 shadow_min = ImVec2(scaled_min.x - blur_amount, scaled_min.y - blur_amount);
					ImVec2 shadow_max = ImVec2(scaled_max.x + blur_amount, scaled_max.y + blur_amount);
					
					draw_list->AddRectFilled(shadow_min, shadow_max, IM_COL32(0, 0, 0, (int)(alpha * 255)), SCALE(8.0f + blur_amount));
				}
				
				// discord.gg/dopamina
				ImVec4 animated_child_bg_top = child_bg_top;
				ImVec4 animated_child_bg_bottom = child_bg_bottom;
				animated_child_bg_top.w *= var->gui.tab_transition_alpha;
				animated_child_bg_bottom.w *= var->gui.tab_transition_alpha;
				
				// discord.gg/dopamina
				draw_list->PushClipRect(scaled_min, scaled_max, true);
				
				// discord.gg/dopamina
				draw_list->AddRectFilled(
					scaled_min,
					scaled_max,
					draw->get_clr(animated_child_bg_bottom),
					SCALE(8.0f)
				);
				
				// discord.gg/dopamina
				draw_list->AddRectFilledMultiColor(
					scaled_min,
					scaled_max,
					draw->get_clr(animated_child_bg_top),
					draw->get_clr(animated_child_bg_top),
					draw->get_clr(animated_child_bg_bottom),
					draw->get_clr(animated_child_bg_bottom)
				);
				
				draw_list->PopClipRect();
				
				// Header
				ImVec2 header_start = scaled_min;
				ImVec2 header_end = ImVec2(scaled_max.x, scaled_min.y + child_header_height);
				
				ImVec4 animated_header_top = child_header_top;
				ImVec4 animated_header_bottom = child_header_bottom;
				animated_header_top.w *= var->gui.tab_transition_alpha;
				animated_header_bottom.w *= var->gui.tab_transition_alpha;
				
				// discord.gg/dopamina
				draw_list->PushClipRect(header_start, header_end, true);
				
				// discord.gg/dopamina
				draw_list->AddRectFilled(
					header_start,
					header_end,
					draw->get_clr(animated_header_bottom),
					SCALE(8.0f),
					ImDrawFlags_RoundCornersTop
				);
				
				// discord.gg/dopamina
				draw_list->AddRectFilledMultiColor(
					header_start,
					header_end,
					draw->get_clr(animated_header_top),
					draw->get_clr(animated_header_top),
					draw->get_clr(animated_header_bottom),
					draw->get_clr(animated_header_bottom)
				);
				
				draw_list->PopClipRect();
				
				// discord.gg/dopamina
				if (child_title_font)
				{
					gui->push_font(child_title_font);
					const char* title = "settings";
					ImVec2 title_size = ImGui::CalcTextSize(title);
					ImVec2 title_pos = ImVec2(scaled_min.x + SCALE(15.0f), scaled_min.y + (child_header_height - title_size.y) * 0.5f);
					
					ImVec4 animated_accent = accent_color;
					animated_accent.w *= var->gui.tab_transition_alpha;
					draw_list->AddText(child_title_font, 15.0f, title_pos, draw->get_clr(animated_accent), title);
					gui->pop_font();
				}
				
				// discord.gg/dopamina
				float bottom_shadow_height = SCALE(10.0f);
				for (int bs = 0; bs < 5; bs++)
				{
					float bs_progress = (float)bs / 5.0f;
					float bs_alpha = (1.0f - bs_progress) * 0.35f * var->gui.tab_transition_alpha;
					
					ImVec2 bs_start = ImVec2(scaled_min.x, header_end.y + (bottom_shadow_height * bs_progress));
					ImVec2 bs_end = ImVec2(scaled_max.x, header_end.y + (bottom_shadow_height * (bs_progress + 0.2f)));
					
					draw_list->AddRectFilledMultiColor(bs_start, bs_end,
						IM_COL32(0, 0, 0, (int)(bs_alpha * 255)), IM_COL32(0, 0, 0, (int)(bs_alpha * 255)),
						IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0));
				}
				
				// Clipping
				draw_list->PushClipRect(scaled_min, scaled_max, true);
				
				// Content area
				float content_start_y = child_pos.y + child_header_height + SCALE(15.0f);
				float content_area_height = large_child_height - child_header_height - SCALE(20.0f);
				ImVec2 child_content_pos = ImVec2(child_pos.x, content_start_y);
				ImVec2 child_content_size = ImVec2(large_child_width, content_area_height);
				
				float scale_offset_x = scaled_min.x - child_pos.x;
				float scale_offset_y = scaled_min.y - child_pos.y;
				
				ImGui::SetCursorPos(ImVec2(child_content_pos.x - pos.x + scale_offset_x, child_content_pos.y - pos.y + scale_offset_y));
				
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, var->gui.tab_transition_alpha);
				
				ImGui::BeginChild("##child_settings", child_content_size, false, ImGuiWindowFlags_NoBackground);
				
				// Padding
				ImGui::SetCursorPosX(SCALE(12.0f));
				
				// UI Element Toggles
				widgets->checkbox("show playerlist", &var->gui.show_playerlist);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(4.0f));
				
				ImGui::SetCursorPosX(SCALE(12.0f));
				widgets->checkbox("show playerbar", &var->gui.show_playerbar);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(4.0f));
				
				ImGui::SetCursorPosX(SCALE(12.0f));
				widgets->checkbox("show spotify player", &var->gui.show_spotify);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(4.0f));
				
				ImGui::SetCursorPosX(SCALE(12.0f));
				widgets->checkbox("show active hotkeys", &var->gui.show_active_hotkeys);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(4.0f));
				
				ImGui::SetCursorPosX(SCALE(12.0f));
				widgets->checkbox("show watermark", &var->gui.show_watermark);
				
				ImGui::EndChild();
				
				ImGui::PopStyleVar();  // Alpha
				
				draw_list->PopClipRect();
			}
			// discord.gg/dopamina
			else if (var->gui.active_tab == 4)
			{
				// discord.gg/dopamina
				float large_child_width = content_width;
				float large_child_height = content_height;
				
				ImVec2 child_pos = content_pos;
				ImVec2 child_end = ImVec2(child_pos.x + large_child_width, child_pos.y + large_child_height);
				
				// discord.gg/dopamina
				float scale_progress = var->gui.tab_transition_alpha;
				scale_progress = scale_progress * scale_progress;
				float scale_effect = 0.85f + (scale_progress * 0.15f);
				
				ImVec2 center = ImVec2((child_pos.x + child_end.x) * 0.5f, (child_pos.y + child_end.y) * 0.5f);
				ImVec2 scaled_min = ImVec2(
					center.x + (child_pos.x - center.x) * scale_effect,
					center.y + (child_pos.y - center.y) * scale_effect
				);
				ImVec2 scaled_max = ImVec2(
					center.x + (child_end.x - center.x) * scale_effect,
					center.y + (child_end.y - center.y) * scale_effect
				);
				
				// discord.gg/dopamina
				float shadow_blur = SCALE(8.0f);
				for (int s = 0; s < 5; s++)
				{
					float progress = (float)s / 5.0f;
					float alpha = (1.0f - progress) * 0.25f * var->gui.tab_transition_alpha;
					float blur_amount = shadow_blur * progress;
					
					ImVec2 shadow_min = ImVec2(scaled_min.x - blur_amount, scaled_min.y - blur_amount);
					ImVec2 shadow_max = ImVec2(scaled_max.x + blur_amount, scaled_max.y + blur_amount);
					
					draw_list->AddRectFilled(shadow_min, shadow_max, IM_COL32(0, 0, 0, (int)(alpha * 255)), SCALE(8.0f + blur_amount));
				}
				
				// discord.gg/dopamina
				ImVec4 animated_child_bg_top = child_bg_top;
				ImVec4 animated_child_bg_bottom = child_bg_bottom;
				animated_child_bg_top.w *= var->gui.tab_transition_alpha;
				animated_child_bg_bottom.w *= var->gui.tab_transition_alpha;
				
				// discord.gg/dopamina
				draw_list->PushClipRect(scaled_min, scaled_max, true);
				
				// discord.gg/dopamina
				draw_list->AddRectFilled(
					scaled_min,
					scaled_max,
					draw->get_clr(animated_child_bg_bottom),
					SCALE(8.0f)
				);
				
				// discord.gg/dopamina
				draw_list->AddRectFilledMultiColor(
					scaled_min,
					scaled_max,
					draw->get_clr(animated_child_bg_top),
					draw->get_clr(animated_child_bg_top),
					draw->get_clr(animated_child_bg_bottom),
					draw->get_clr(animated_child_bg_bottom)
				);
				
				draw_list->PopClipRect();
				
				// Header
				ImVec2 header_start = scaled_min;
				ImVec2 header_end = ImVec2(scaled_max.x, scaled_min.y + child_header_height);
				
				ImVec4 animated_header_top = child_header_top;
				ImVec4 animated_header_bottom = child_header_bottom;
				animated_header_top.w *= var->gui.tab_transition_alpha;
				animated_header_bottom.w *= var->gui.tab_transition_alpha;
				
				// discord.gg/dopamina
				draw_list->PushClipRect(header_start, header_end, true);
				
				// discord.gg/dopamina
				draw_list->AddRectFilled(
					header_start,
					header_end,
					draw->get_clr(animated_header_bottom),
					SCALE(8.0f),
					ImDrawFlags_RoundCornersTop
				);
				
				// discord.gg/dopamina
				draw_list->AddRectFilledMultiColor(
					header_start,
					header_end,
					draw->get_clr(animated_header_top),
					draw->get_clr(animated_header_top),
					draw->get_clr(animated_header_bottom),
					draw->get_clr(animated_header_bottom)
				);
				
				draw_list->PopClipRect();
				
				// discord.gg/dopamina
				if (child_title_font)
				{
					gui->push_font(child_title_font);
					const char* title = "colors";
					ImVec2 title_size = ImGui::CalcTextSize(title);
					ImVec2 title_pos = ImVec2(scaled_min.x + SCALE(15.0f), scaled_min.y + (child_header_height - title_size.y) * 0.5f);
					
					ImVec4 animated_accent = accent_color;
					animated_accent.w *= var->gui.tab_transition_alpha;
					draw_list->AddText(child_title_font, 15.0f, title_pos, draw->get_clr(animated_accent), title);
					gui->pop_font();
				}
				
				// discord.gg/dopamina
				float bottom_shadow_height = SCALE(10.0f);
				for (int bs = 0; bs < 5; bs++)
				{
					float bs_progress = (float)bs / 5.0f;
					float bs_alpha = (1.0f - bs_progress) * 0.35f * var->gui.tab_transition_alpha;
					
					ImVec2 bs_start = ImVec2(scaled_min.x, header_end.y + (bottom_shadow_height * bs_progress));
					ImVec2 bs_end = ImVec2(scaled_max.x, header_end.y + (bottom_shadow_height * (bs_progress + 0.2f)));
					
					draw_list->AddRectFilledMultiColor(bs_start, bs_end,
						IM_COL32(0, 0, 0, (int)(bs_alpha * 255)), IM_COL32(0, 0, 0, (int)(bs_alpha * 255)),
						IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0));
				}
				
				// Clipping
				draw_list->PushClipRect(scaled_min, scaled_max, true);
				
				// Content area
				float content_start_y = child_pos.y + child_header_height + SCALE(15.0f);
				float content_area_height = large_child_height - child_header_height - SCALE(20.0f);
				ImVec2 child_content_pos = ImVec2(child_pos.x, content_start_y);
				ImVec2 child_content_size = ImVec2(large_child_width, content_area_height);
				
				float scale_offset_x = scaled_min.x - child_pos.x;
				float scale_offset_y = scaled_min.y - child_pos.y;
				
				ImGui::SetCursorPos(ImVec2(child_content_pos.x - pos.x + scale_offset_x, child_content_pos.y - pos.y + scale_offset_y));
				
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, var->gui.tab_transition_alpha);
				
				// discord.gg/dopamina
				ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, SCALE(6.0f));  // discord.gg/dopamina
				ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, IM_COL32(45, 45, 50, 255));  // discord.gg/dopamina
				ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, IM_COL32(179, 143, 228, 255));  // discord.gg/dopamina
				ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, IM_COL32(189, 153, 238, 255));  // discord.gg/dopamina
				ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, IM_COL32(169, 133, 218, 255));  // discord.gg/dopamina
				ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, SCALE(4.0f));  // discord.gg/dopamina
				
				ImGui::BeginChild("##child_colors", child_content_size, false, ImGuiWindowFlags_NoBackground);
				
				// Padding
				ImGui::SetCursorPosX(SCALE(12.0f));
				
				// discord.gg/dopamina
				const char* color_labels[] = {
					"crosshair color", "fov circle", "target lock", "hit marker",
					"box esp", "name esp", "health bar", "distance",
					"chams visible", "chams hidden", "chams team", "chams local",
					"enemy glow", "team glow", "item glow",
					"accent color", "success color", "warning color", "error color", "info color"
				};
				
				for (int i = 0; i < 20; i++)
				{
					ImGui::SetCursorPosX(SCALE(12.0f));
					widgets->color_picker(color_labels[i], test_colors[4][i]);
					
					if (i < 19)
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(4.0f));
				}
				
				ImGui::EndChild();
				
				ImGui::PopStyleVar(3);  // ScrollbarSize, ScrollbarRounding, Alpha
				ImGui::PopStyleColor(4);  // Scrollbar Colors
				
				draw_list->PopClipRect();
			}
			else if (var->gui.active_tab == 5)
			{
				// discord.gg/dopamina
			}
			else  // discord.gg/dopamina
			{
				for (int row = 0; row < 2; row++)
				{
				for (int col = 0; col < 2; col++)
				{
					int child_index = row * 2 + col;
					
					float child_x = content_pos.x + (col * (child_width + child_spacing));
					float child_y = content_pos.y + (row * (child_height + child_spacing));
					
					ImVec2 child_pos = ImVec2(child_x, child_y);
					ImVec2 child_end = ImVec2(child_x + child_width, child_y + child_height);
					
					// discord.gg/dopamina
					// discord.gg/dopamina
					float scale_progress = var->gui.tab_transition_alpha;
					scale_progress = scale_progress * scale_progress;  // discord.gg/dopamina
					
					// discord.gg/dopamina
					float scale_effect = 0.85f + (scale_progress * 0.15f);
					
					// discord.gg/dopamina
					ImVec2 center = ImVec2((child_pos.x + child_end.x) * 0.5f, (child_pos.y + child_end.y) * 0.5f);
					ImVec2 scaled_min = ImVec2(
						center.x + (child_pos.x - center.x) * scale_effect,
						center.y + (child_pos.y - center.y) * scale_effect
					);
					ImVec2 scaled_max = ImVec2(
						center.x + (child_end.x - center.x) * scale_effect,
						center.y + (child_end.y - center.y) * scale_effect
					);
					
					// discord.gg/dopamina
					float shadow_blur = SCALE(8.0f);
					
					for (int s = 0; s < 5; s++)
					{
						float progress = (float)s / 5.0f;
						float alpha = (1.0f - progress) * 0.25f * var->gui.tab_transition_alpha;  // discord.gg/dopamina
						float blur_amount = shadow_blur * progress;
						
						ImVec2 shadow_min = ImVec2(scaled_min.x - blur_amount, scaled_min.y - blur_amount);
						ImVec2 shadow_max = ImVec2(scaled_max.x + blur_amount, scaled_max.y + blur_amount);
						
						draw_list->AddRectFilled(
							shadow_min,
							shadow_max,
							IM_COL32(0, 0, 0, (int)(alpha * 255)),
							SCALE(8.0f + blur_amount)
						);
					}
					
					// discord.gg/dopamina
					ImVec4 animated_child_bg_top = child_bg_top;
					ImVec4 animated_child_bg_bottom = child_bg_bottom;
					animated_child_bg_top.w *= var->gui.tab_transition_alpha;
					animated_child_bg_bottom.w *= var->gui.tab_transition_alpha;
					
					// discord.gg/dopamina
					draw_list->PushClipRect(scaled_min, scaled_max, true);
					
					// discord.gg/dopamina
					draw_list->AddRectFilled(
						scaled_min,
						scaled_max,
						draw->get_clr(animated_child_bg_bottom),
						SCALE(8.0f)
					);
					
					// discord.gg/dopamina
					draw_list->AddRectFilledMultiColor(
						scaled_min,
						scaled_max,
						draw->get_clr(animated_child_bg_top),
						draw->get_clr(animated_child_bg_top),
						draw->get_clr(animated_child_bg_bottom),
						draw->get_clr(animated_child_bg_bottom)
					);
					
					draw_list->PopClipRect();
					
					// discord.gg/dopamina
					ImVec2 header_start = scaled_min;
					ImVec2 header_end = ImVec2(scaled_max.x, scaled_min.y + child_header_height);
					
					// discord.gg/dopamina
					float header_shadow_blur = SCALE(6.0f);
					for (int hs = 0; hs < 4; hs++)
					{
						float hs_progress = (float)hs / 4.0f;
						float hs_alpha = (1.0f - hs_progress) * 0.2f * var->gui.tab_transition_alpha;  // discord.gg/dopamina
						float hs_blur = header_shadow_blur * hs_progress;
						
						ImVec2 hs_min = ImVec2(header_start.x - hs_blur, header_start.y - hs_blur);
						ImVec2 hs_max = ImVec2(header_end.x + hs_blur, header_end.y + hs_blur);
						
						draw_list->AddRectFilled(
							hs_min,
							hs_max,
							IM_COL32(0, 0, 0, (int)(hs_alpha * 255)),
							SCALE(8.0f + hs_blur),
							draw_flags_round_corners_top
						);
					}
					
					// discord.gg/dopamina
					ImVec4 animated_header_top = child_header_top;
					ImVec4 animated_header_bottom = child_header_bottom;
					animated_header_top.w *= var->gui.tab_transition_alpha;
					animated_header_bottom.w *= var->gui.tab_transition_alpha;
					
					// discord.gg/dopamina
					draw_list->PushClipRect(header_start, header_end, true);
					
					// discord.gg/dopamina
					draw_list->AddRectFilled(
						header_start,
						header_end,
						draw->get_clr(animated_header_bottom),
						SCALE(8.0f),
						ImDrawFlags_RoundCornersTop
					);
					
					// discord.gg/dopamina
					draw_list->AddRectFilledMultiColor(
						header_start,
						header_end,
						draw->get_clr(animated_header_top),
						draw->get_clr(animated_header_top),
						draw->get_clr(animated_header_bottom),
						draw->get_clr(animated_header_bottom)
					);
					
					draw_list->PopClipRect();
					
					// discord.gg/dopamina
					if (child_title_font)
					{
						gui->push_font(child_title_font);
						
						const char* title = child_titles[var->gui.active_tab][child_index];  // discord.gg/dopamina
						ImVec2 title_size = ImGui::CalcTextSize(title);
						ImVec2 title_pos = ImVec2(
							scaled_min.x + SCALE(15.0f), // discord.gg/dopamina
							scaled_min.y + (child_header_height - title_size.y) * 0.5f // discord.gg/dopamina
						);
						
						// discord.gg/dopamina
						ImVec4 animated_accent = accent_color;
						animated_accent.w *= var->gui.tab_transition_alpha;
						draw_list->AddText(child_title_font, 15.0f, title_pos, draw->get_clr(animated_accent), title);
						
						gui->pop_font();
					}
					
					// discord.gg/dopamina
					float bottom_shadow_height = SCALE(10.0f);
					for (int bs = 0; bs < 5; bs++)
					{
						float bs_progress = (float)bs / 5.0f;
						float bs_alpha = (1.0f - bs_progress) * 0.35f * var->gui.tab_transition_alpha;  // discord.gg/dopamina
						
						ImVec2 bs_start = ImVec2(scaled_min.x, header_end.y + (bottom_shadow_height * bs_progress));
						ImVec2 bs_end = ImVec2(scaled_max.x, header_end.y + (bottom_shadow_height * (bs_progress + 0.2f)));
						
						draw_list->AddRectFilledMultiColor(
							bs_start,
							bs_end,
							IM_COL32(0, 0, 0, (int)(bs_alpha * 255)),
							IM_COL32(0, 0, 0, (int)(bs_alpha * 255)),
							IM_COL32(0, 0, 0, 0),
							IM_COL32(0, 0, 0, 0)
						);
					}
					
					// discord.gg/dopamina
					draw_list->PushClipRect(scaled_min, scaled_max, true);
					
					// discord.gg/dopamina
					float content_start_y = child_y + child_header_height + SCALE(15.0f);
					float content_area_height = child_height - child_header_height - SCALE(20.0f);
					
					ImVec2 child_content_pos = ImVec2(child_x, content_start_y);
					ImVec2 child_content_size = ImVec2(child_width, content_area_height);
					
					// discord.gg/dopamina
					float scale_offset_x = scaled_min.x - child_pos.x;
					float scale_offset_y = scaled_min.y - child_pos.y;
					
					// discord.gg/dopamina
					ImGui::SetCursorPos(ImVec2(
						child_content_pos.x - pos.x + scale_offset_x,
						child_content_pos.y - pos.y + scale_offset_y
					));
					
					// discord.gg/dopamina
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, var->gui.tab_transition_alpha);
					
					char child_id[32];
					sprintf_s(child_id, "##child_tab%d_%d", var->gui.active_tab, child_index);
					
					ImGui::BeginChild(child_id, child_content_size, false, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
					
					// discord.gg/dopamina
					ImGui::SetCursorPosX(SCALE(12.0f));
					
					// discord.gg/dopamina
					if (var->gui.active_tab == 3)
					{
						// Config System Layout
						if (child_index == 0 || child_index == 2)  // discord.gg/dopamina
						{
							// discord.gg/dopamina
							ImGui::SetCursorPosX(SCALE(12.0f));
							
							if (child_index == 0)  // Top-left: Config slots
							{
								// Config Slot Buttons
								for (int i = 0; i < 5; i++)
								{
									ImGui::SetCursorPosX(SCALE(12.0f));
									
									char slot_label[64];
									sprintf_s(slot_label, "config slot %d", i + 1);
									
									if (widgets->button(slot_label))
									{
										// Config slot clicked - Load config from slot
										char notify_text[128];
										sprintf_s(notify_text, "Config Slot %d selected", i + 1);
										notify->add_notify(notify_text, info);
									}
									
									if (i < 4)
										ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(6.0f));
								}
							}
							else  // Bottom-left: Config management
							{
								// Config Management Buttons
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("save config"))
								{
									// Save current config
									notify->add_notify("Configuration saved successfully!", success);
								}
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(6.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("load config"))
								{
									// Load selected config
									notify->add_notify("Configuration loaded successfully!", success);
								}
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(6.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("delete config"))
								{
									// Delete selected config
									notify->add_notify("Configuration deleted", warning);
								}
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(6.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("reset to default"))
								{
									// Reset all settings
									notify->add_notify("Settings reset to default", info);
								}
							}
						}
						else  // discord.gg/dopamina
						{
							if (child_index == 1)  // Top-right: Auto-load settings
							{
								ImGui::SetCursorPosX(SCALE(12.0f));
								
								static bool auto_load_config = false;
								widgets->checkbox("auto-load on startup", &auto_load_config);
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								
								static bool save_on_exit = true;
								widgets->checkbox("save on exit", &save_on_exit);
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								
								static int default_slot = 0;
								static const char* slot_items[] = { "Slot 1", "Slot 2", "Slot 3", "Slot 4", "Slot 5" };
								widgets->dropdown("default slot", &default_slot, slot_items, 5);
							}
							else  // Bottom-right: Export/Import
							{
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("export config"))
								{
									// TODO: Export config to file
								}
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(6.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("import config"))
								{
									// TODO: Import config from file
								}
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(6.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("share config"))
								{
									// TODO: Generate share link
								}
								
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(6.0f));
								ImGui::SetCursorPosX(SCALE(12.0f));
								if (widgets->button("backup all"))
								{
									// TODO: Backup all configs
								}
							}
						}
					}
					// discord.gg/dopamina
					else if (var->gui.active_tab == 4)
					{
						// discord.gg/dopamina
						int color_start = child_index * 2;
						for (int i = 0; i < 4; i++)
						{
							ImGui::SetCursorPosX(SCALE(12.0f));
							
							char color_label[64];
							sprintf_s(color_label, "color %d", color_start + i + 1);
							
							widgets->color_picker(color_label, test_colors[var->gui.active_tab][color_start + i]);
							
							if (i < 3)
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(4.0f));
						}
					}
					else if (var->gui.active_tab == 5)
					{
						// Settings Tab - UI Element Toggles
						
						// discord.gg/dopamina
						ImGui::SetCursorPosX(SCALE(12.0f));
						widgets->checkbox("show playerlist", &var->gui.show_playerlist);
						
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
						ImGui::SetCursorPosX(SCALE(12.0f));
						widgets->checkbox("show playerbar", &var->gui.show_playerbar);
						
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
						ImGui::SetCursorPosX(SCALE(12.0f));
						widgets->checkbox("show spotify player", &var->gui.show_spotify);
						
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
						ImGui::SetCursorPosX(SCALE(12.0f));
						widgets->checkbox("show active hotkeys", &var->gui.show_active_hotkeys);
						
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
						ImGui::SetCursorPosX(SCALE(12.0f));
						widgets->checkbox("show watermark", &var->gui.show_watermark);
					}
					else
					{
						// discord.gg/dopamina
						
						// discord.gg/dopamina
						int checkbox_start = child_index * 4;
						
						ImVec2 kb_option1_pos = ImGui::GetCursorScreenPos(); // discord.gg/dopamina
						
						for (int i = 0; i < 2; i++)  // discord.gg/dopamina
						{
							ImGui::SetCursorPosX(SCALE(12.0f));
							
							char checkbox_label[64];
							sprintf_s(checkbox_label, "option %d", i + 1);
							
							widgets->checkbox(checkbox_label, &test_checks[var->gui.active_tab][checkbox_start + i]);
						}
						
						// discord.gg/dopamina
						{
							static int   kb_key     = -1;
							static int   kb_mode    = 0;
							static bool  kb_act     = false;
							static bool  kb_ctx     = false;
							static ImVec2 kb_cp     = {};
							static float  kb_anim   = 0.f;
							static float  kb_hov_a  = 0.f;
							static bool  kb_was_act = false; // discord.gg/dopamina
							
							ImFont* kf = font->get(main_font_data, 12.0f);
							if (kf)
							{
								ImDrawList* dl = ImGui::GetForegroundDrawList();
								ImGuiIO&    io = ImGui::GetIO();
								float dt = ImMax(io.DeltaTime, 0.001f);
								
								// discord.gg/dopamina
								if (kb_act) kb_anim += dt * 3.f;
								else        kb_anim  = 0.f;
								
								// Key-Scan
								if (kb_act)
								{
									if (!kb_was_act)
									{
										// discord.gg/dopamina
										if (!GetAsyncKeyState(VK_LBUTTON) && !GetAsyncKeyState(VK_RBUTTON))
											kb_was_act = true;
									}
									else
									{
										// discord.gg/dopamina
										// discord.gg/dopamina
										static bool prev_mouse[5] = {};
										bool cur_mouse[5] = {
											(GetAsyncKeyState(VK_LBUTTON)  & 0x8000) != 0,
											(GetAsyncKeyState(VK_RBUTTON)  & 0x8000) != 0,
											(GetAsyncKeyState(VK_MBUTTON)  & 0x8000) != 0,
											(GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0,
											(GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0,
										};
										int mouse_vks[5] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
										
										for (int m = 0; m < 5; m++)
										{
											// Edge: just pressed (false→true)
											if (cur_mouse[m] && !prev_mouse[m])
											{
												kb_key = mouse_vks[m];
												kb_act = false;
												break;
											}
										}
										for (int m=0;m<5;m++) prev_mouse[m] = cur_mouse[m];
										
										// Keyboard via ImGui
										if (kb_act)
										{
											static const struct { ImGuiKey ig; int vk; } km[] = {
												{ImGuiKey_Escape,VK_ESCAPE},{ImGuiKey_Tab,VK_TAB},
												{ImGuiKey_Backspace,VK_BACK},{ImGuiKey_Space,VK_SPACE},
												{ImGuiKey_Enter,VK_RETURN},{ImGuiKey_Insert,VK_INSERT},
												{ImGuiKey_Delete,VK_DELETE},{ImGuiKey_Home,VK_HOME},
												{ImGuiKey_End,VK_END},{ImGuiKey_PageUp,VK_PRIOR},
												{ImGuiKey_PageDown,VK_NEXT},{ImGuiKey_LeftArrow,VK_LEFT},
												{ImGuiKey_RightArrow,VK_RIGHT},{ImGuiKey_UpArrow,VK_UP},
												{ImGuiKey_DownArrow,VK_DOWN},
												{ImGuiKey_LeftShift,VK_LSHIFT},{ImGuiKey_RightShift,VK_RSHIFT},
												{ImGuiKey_LeftCtrl,VK_LCONTROL},{ImGuiKey_RightCtrl,VK_RCONTROL},
												{ImGuiKey_LeftAlt,VK_LMENU},{ImGuiKey_RightAlt,VK_RMENU},
												{ImGuiKey_F1,VK_F1},{ImGuiKey_F2,VK_F2},{ImGuiKey_F3,VK_F3},
												{ImGuiKey_F4,VK_F4},{ImGuiKey_F5,VK_F5},{ImGuiKey_F6,VK_F6},
												{ImGuiKey_F7,VK_F7},{ImGuiKey_F8,VK_F8},{ImGuiKey_F9,VK_F9},
												{ImGuiKey_F10,VK_F10},{ImGuiKey_F11,VK_F11},{ImGuiKey_F12,VK_F12},
												{ImGuiKey_CapsLock,VK_CAPITAL},{ImGuiKey_NumLock,VK_NUMLOCK},
												{ImGuiKey_0,'0'},{ImGuiKey_1,'1'},{ImGuiKey_2,'2'},
												{ImGuiKey_3,'3'},{ImGuiKey_4,'4'},{ImGuiKey_5,'5'},
												{ImGuiKey_6,'6'},{ImGuiKey_7,'7'},{ImGuiKey_8,'8'},{ImGuiKey_9,'9'},
												{ImGuiKey_A,'A'},{ImGuiKey_B,'B'},{ImGuiKey_C,'C'},{ImGuiKey_D,'D'},
												{ImGuiKey_E,'E'},{ImGuiKey_F,'F'},{ImGuiKey_G,'G'},{ImGuiKey_H,'H'},
												{ImGuiKey_I,'I'},{ImGuiKey_J,'J'},{ImGuiKey_K,'K'},{ImGuiKey_L,'L'},
												{ImGuiKey_M,'M'},{ImGuiKey_N,'N'},{ImGuiKey_O,'O'},{ImGuiKey_P,'P'},
												{ImGuiKey_Q,'Q'},{ImGuiKey_R,'R'},{ImGuiKey_S,'S'},{ImGuiKey_T,'T'},
												{ImGuiKey_U,'U'},{ImGuiKey_V,'V'},{ImGuiKey_W,'W'},{ImGuiKey_X,'X'},
												{ImGuiKey_Y,'Y'},{ImGuiKey_Z,'Z'},
												{ImGuiKey_Keypad0,VK_NUMPAD0},{ImGuiKey_Keypad1,VK_NUMPAD1},
												{ImGuiKey_Keypad2,VK_NUMPAD2},{ImGuiKey_Keypad3,VK_NUMPAD3},
												{ImGuiKey_Keypad4,VK_NUMPAD4},{ImGuiKey_Keypad5,VK_NUMPAD5},
												{ImGuiKey_Keypad6,VK_NUMPAD6},{ImGuiKey_Keypad7,VK_NUMPAD7},
												{ImGuiKey_Keypad8,VK_NUMPAD8},{ImGuiKey_Keypad9,VK_NUMPAD9},
											};
											for (auto& e : km)
											{
												if (ImGui::IsKeyPressed(e.ig, false))
												{
													kb_key = (e.ig == ImGuiKey_Escape) ? -1 : e.vk;
													kb_act = false;
													break;
												}
											}
										}
									}
								}
								else
								{
									kb_was_act = false;
								}
								
								// Key Name
								char buf[128] = {};
								if (kb_key == -1) {
									strcpy_s(buf, "None");
								} else {
									switch (kb_key) {
									case VK_LBUTTON:  strcpy_s(buf,"M1");  break;
									case VK_RBUTTON:  strcpy_s(buf,"M2");  break;
									case VK_MBUTTON:  strcpy_s(buf,"M3");  break;
									case VK_XBUTTON1: strcpy_s(buf,"M4");  break;
									case VK_XBUTTON2: strcpy_s(buf,"M5");  break;
									case VK_INSERT:   strcpy_s(buf,"Ins"); break;
									case VK_DELETE:   strcpy_s(buf,"Del"); break;
									default: {
										UINT sc = MapVirtualKeyA(kb_key, MAPVK_VK_TO_VSC);
										switch(kb_key){
											case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
											case VK_RCONTROL: case VK_RMENU: case VK_PRIOR: case VK_NEXT:
											case VK_END: case VK_HOME: sc |= KF_EXTENDED; break;
										}
										if (!GetKeyNameTextA(sc << 16, buf, 128) || !buf[0])
											sprintf_s(buf, "K%d", kb_key);
										if (buf[0]) buf[0] = toupper((unsigned char)buf[0]);
										for (int c=1;buf[c];c++) buf[c]=tolower((unsigned char)buf[c]);
									} break;
									}
								}
								
								// discord.gg/dopamina
								ImVec2 tsz = kf->CalcTextSizeA(12.f, FLT_MAX, 0, buf);
								float  bw  = ImMax(SCALE(38.f), tsz.x + SCALE(16.f));
								float  bh  = SCALE(18.f);
								float  chk_h = SCALE(31.f);
								
								ImVec2 bp = ImVec2(
									kb_option1_pos.x + (ImGui::GetWindowSize().x - SCALE(24.f)) - bw,
									kb_option1_pos.y + (chk_h - bh) * 0.5f + SCALE(6.f)
								);
								ImRect br(bp, ImVec2(bp.x + bw, bp.y + bh));
								bool   hov = br.Contains(ImGui::GetMousePos());
								
								// Hover anim
								kb_hov_a += ((hov ? 1.f : 0.f) - kb_hov_a) * ImMin(dt * 15.f, 1.f);
								
								// Shadow
								for (int s=0;s<4;s++) {
									float p=(float)s/4.f, bl=SCALE(8.f)*p;
									dl->AddRectFilled(
										ImVec2(bp.x-bl,bp.y-bl), ImVec2(bp.x+bw+bl,bp.y+bh+bl),
										IM_COL32(0,0,0,(int)((1-p)*45*(1.f+kb_hov_a*0.6f))), SCALE(4.f));
								}
								
								// discord.gg/dopamina
								dl->AddRectFilled(ImVec2(bp.x+1,bp.y+1),ImVec2(bp.x+bw,bp.y+bh),IM_COL32(10,10,14,240),SCALE(4.f));
								ImU32 bgc = IM_COL32((int)(18+kb_hov_a*8),(int)(18+kb_hov_a*6),(int)(22+kb_hov_a*10),240);
								dl->AddRectFilled(br.Min,br.Max,bgc,SCALE(4.f));
								
								// discord.gg/dopamina
								float ba = kb_act ? 0.25f : (0.12f + kb_hov_a * 0.28f);
								dl->AddRect(br.Min,br.Max,IM_COL32(179,143,228,(int)(ba*255)),SCALE(4.f),0,SCALE(1.f));
								
								// Content
								if (kb_act)
								{
									// discord.gg/dopamina
									float cx = bp.x + bw * 0.5f;
									float cy = bp.y + bh * 0.5f;
									for (int d=0;d<3;d++) {
										float phase  = kb_anim * 0.6f + d * 0.7f; // discord.gg/dopamina
										float palpha = 0.3f + 0.7f * (0.5f + 0.5f * sinf(phase * 0.8f)); // discord.gg/dopamina
										float psize  = SCALE(2.0f); // discord.gg/dopamina
										dl->AddCircleFilled(
											ImVec2(cx + (d-1) * SCALE(7.f), cy),
											psize, IM_COL32(179,143,228,(int)(palpha*255)));
									}
								}
								else
								{
									ImVec2 tp = ImVec2(bp.x+(bw-tsz.x)*0.5f, bp.y+(bh-SCALE(12.f))*0.5f);
									dl->AddText(kf,12.f,tp,IM_COL32(130,130,138,255),buf);
									if (kb_hov_a > 0.01f)
										dl->AddText(kf,12.f,tp,IM_COL32(210,200,235,(int)(kb_hov_a*200)),buf);
								}
								
								// discord.gg/dopamina
								if (hov && ImGui::IsMouseClicked(0) && !kb_act)
									kb_act = true;
								// discord.gg/dopamina
								if (hov && ImGui::IsMouseClicked(1) && !kb_act)
								{
									kb_ctx = true;
									kb_cp  = ImGui::GetMousePos();
								}
								
								// Context Menu
								if (kb_ctx)
								{
									ImDrawList* fg = ImGui::GetForegroundDrawList();
									const char* ml[3] = {"Hold","Toggle","Always On"};
									float mih=SCALE(22.f), mw=SCALE(100.f), mh=mih*3+SCALE(4.f);
									kb_cp.x=ImMin(kb_cp.x,io.DisplaySize.x-mw-4.f);
									kb_cp.y=ImMin(kb_cp.y,io.DisplaySize.y-mh-4.f);
									ImRect mr(kb_cp,ImVec2(kb_cp.x+mw,kb_cp.y+mh));
									
									for(int s=0;s<4;s++){float p=(float)s/4.f,bl=SCALE(8.f)*p;fg->AddRectFilled(ImVec2(kb_cp.x-bl,kb_cp.y-bl),ImVec2(kb_cp.x+mw+bl,kb_cp.y+mh+bl),IM_COL32(0,0,0,(int)((1-p)*90)),SCALE(5.f));}
									fg->AddRectFilled(mr.Min,mr.Max,IM_COL32(18,18,22,252),SCALE(5.f));
									fg->AddRect(mr.Min,mr.Max,IM_COL32(179,143,228,40),SCALE(5.f),0,SCALE(1.f));
									
									for (int m=0;m<3;m++) {
										ImVec2 mp2=ImVec2(kb_cp.x, kb_cp.y+SCALE(2.f)+mih*m);
										ImRect mri(mp2,ImVec2(mp2.x+mw,mp2.y+mih));
										bool mhv=mri.Contains(ImGui::GetMousePos());
										if(mhv) fg->AddRectFilled(mri.Min,mri.Max,IM_COL32(255,255,255,10),SCALE(4.f));
										bool sel=(kb_mode==m);
										ImVec2 msz=kf->CalcTextSizeA(12.f,FLT_MAX,0,ml[m]);
										fg->AddText(kf,12.f,
											ImVec2(mp2.x+(mw-msz.x)*0.5f,mp2.y+(mih-SCALE(12.f))*0.5f),
											sel?IM_COL32(179,143,228,255):(mhv?IM_COL32(220,218,230,255):IM_COL32(160,160,168,200)),
											ml[m]);
										if(mhv&&ImGui::IsMouseClicked(0)){kb_mode=m;kb_ctx=false;}
									}
									if((ImGui::IsMouseClicked(0)||ImGui::IsMouseClicked(1))&&!mr.Contains(ImGui::GetMousePos()))
										kb_ctx=false;
								}
							}
						}
						
						// Spacing
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
						
						// discord.gg/dopamina
						int slider_start = child_index * 2;
						for (int i = 0; i < 2; i++)  // 2 Slider
						{
							ImGui::SetCursorPosX(SCALE(12.0f));
							
							char slider_label[64];
							sprintf_s(slider_label, "slider %d", i + 1);
							
							widgets->slider_int(slider_label, &test_sliders[var->gui.active_tab][slider_start + i], 0, 100);
						}
						
						// Spacing
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SCALE(8.0f));
						
						// discord.gg/dopamina
						int dropdown_start = child_index * 2;
						for (int i = 0; i < 2; i++)  // 2 Dropdowns
						{
							ImGui::SetCursorPosX(SCALE(12.0f));
							
							char dropdown_label[64];
							sprintf_s(dropdown_label, "dropdown %d", i + 1);
							
							static const char* dropdown_items[] = { "Option A", "Option B", "Option C", "Option D" };
							widgets->dropdown(dropdown_label, &test_dropdowns[var->gui.active_tab][dropdown_start + i], dropdown_items, 4);
						}
					}
					
					ImGui::EndChild();
					
					ImGui::PopStyleVar();  // Alpha
					
					draw_list->PopClipRect();  // discord.gg/dopamina
				}
			}
			}  // discord.gg/dopamina
			
			// discord.gg/dopamina
			if (var->gui.active_tab == 1)
			{
				static bool esp_preview_open = true; // discord.gg/dopamina
				
				// discord.gg/dopamina
				float toggle_button_width = SCALE(18.0f);  // discord.gg/dopamina
				float toggle_button_height = SCALE(40.0f); // discord.gg/dopamina
				
				// discord.gg/dopamina
				ImVec2 toggle_pos = ImVec2(
					scaled_pos.x + scaled_size.x, 
					scaled_pos.y + tabbar_height + (content_height - toggle_button_height) * 0.5f
				);
				ImRect toggle_rect(toggle_pos, ImVec2(toggle_pos.x + toggle_button_width, toggle_pos.y + toggle_button_height));
				
				bool toggle_hovered = IsMouseHoveringRect(toggle_rect.Min, toggle_rect.Max, false);
				bool toggle_clicked = toggle_hovered && gui->mouse_clicked(0, false);
				
				if (toggle_clicked)
					esp_preview_open = !esp_preview_open;
				
				// Toggle Button Animation
				static float toggle_hover_alpha = 0.0f;
				gui->easing(toggle_hover_alpha, toggle_hovered ? 1.0f : 0.0f, 20.0f, static_easing);
				
				// discord.gg/dopamina
				float btn_shadow_blur = SCALE(6.0f);
				for (int s = 0; s < 5; s++)
				{
					float progress = (float)s / 5.0f;
					float alpha = (1.0f - progress) * 0.25f * var->gui.menu_open_alpha;
					float blur_amount = btn_shadow_blur * progress;
					
					draw_list->AddRectFilled(
						ImVec2(toggle_rect.Min.x - blur_amount, toggle_rect.Min.y - blur_amount),
						ImVec2(toggle_rect.Max.x + blur_amount, toggle_rect.Max.y + blur_amount),
						IM_COL32(0, 0, 0, (int)(alpha * 255)),
						SCALE(6.0f) + blur_amount
					);
				}
				
				// discord.gg/dopamina
				ImVec4 btn_bg = ImVec4(40.0f / 255.0f, 40.0f / 255.0f, 46.0f / 255.0f, var->gui.menu_open_alpha);
				draw_list->AddRectFilled(toggle_rect.Min, toggle_rect.Max, draw->get_clr(btn_bg), SCALE(6.0f));
				
				// discord.gg/dopamina
				if (toggle_hover_alpha > 0.01f || esp_preview_open)
				{
					float effect_alpha = esp_preview_open ? 1.0f : toggle_hover_alpha;
					ImVec4 btn_accent = ImVec4(179.0f / 255.0f, 143.0f / 255.0f, 228.0f / 255.0f, 0.25f * effect_alpha * var->gui.menu_open_alpha);
					draw_list->AddRectFilled(toggle_rect.Min, toggle_rect.Max, draw->get_clr(btn_accent), SCALE(6.0f));
				}
				
				// Icon: U+F05F (FontAwesome Crosshairs/Target)
				ImFont* btn_font = font->get(main_font_data, 14.0f);
				if (btn_font)
				{
					const char* icon = "\xEF\x81\x9F"; // U+F05F
					ImVec2 icon_size = btn_font->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, icon);
					ImVec2 icon_pos = ImVec2(
						toggle_rect.Min.x + (toggle_button_width - icon_size.x) * 0.5f,
						toggle_rect.Min.y + (toggle_button_height - icon_size.y) * 0.5f
					);
					
					// discord.gg/dopamina
					ImVec4 icon_color;
					if (esp_preview_open)
						icon_color = ImVec4(179.0f / 255.0f, 143.0f / 255.0f, 228.0f / 255.0f, var->gui.menu_open_alpha);
					else if (toggle_hovered)
						icon_color = ImVec4(0.85f, 0.85f, 0.9f, var->gui.menu_open_alpha);
					else
						icon_color = ImVec4(0.7f, 0.7f, 0.75f, var->gui.menu_open_alpha);
					
					draw_list->AddText(btn_font, 14.0f, icon_pos, draw->get_clr(icon_color), icon);
				}
				
				// ESP Preview Box Animation
				static float esp_preview_alpha = 0.0f;
				gui->easing(esp_preview_alpha, esp_preview_open ? 1.0f : 0.0f, 8.0f, dynamic_easing);
				
				if (esp_preview_alpha > 0.01f)
				{
					// discord.gg/dopamina
					float preview_width = SCALE(300.0f);
					float preview_height = content_height;
					float spacing = SCALE(15.0f); // discord.gg/dopamina
					ImVec2 preview_pos = ImVec2(toggle_rect.Max.x + spacing, scaled_pos.y + tabbar_height);
					
					// 3D Scale Animation
					float preview_scale = 0.85f + (esp_preview_alpha * 0.15f);
					ImVec2 preview_center = ImVec2(preview_pos.x + preview_width * 0.5f, preview_pos.y + preview_height * 0.5f);
					ImVec2 preview_scaled_min = ImVec2(
						preview_center.x + (preview_pos.x - preview_center.x) * preview_scale,
						preview_center.y + (preview_pos.y - preview_center.y) * preview_scale
					);
					ImVec2 preview_scaled_max = ImVec2(
						preview_center.x + ((preview_pos.x + preview_width) - preview_center.x) * preview_scale,
						preview_center.y + ((preview_pos.y + preview_height) - preview_center.y) * preview_scale
					);
					
					// discord.gg/dopamina
					float preview_shadow_blur = SCALE(12.0f);
					for (int s = 0; s < 6; s++)
					{
						float progress = (float)s / 6.0f;
						float alpha = (1.0f - progress) * 0.25f * esp_preview_alpha;
						float blur_amount = preview_shadow_blur * progress;
						
						draw_list->AddRectFilled(
							ImVec2(preview_scaled_min.x - blur_amount, preview_scaled_min.y - blur_amount),
							ImVec2(preview_scaled_max.x + blur_amount, preview_scaled_max.y + blur_amount),
							IM_COL32(0, 0, 0, (int)(alpha * 255)),
							SCALE(8.0f) + blur_amount
						);
					}
					
					// discord.gg/dopamina
					draw_list->PushClipRect(preview_scaled_min, preview_scaled_max, true);
					
					// discord.gg/dopamina
					ImVec4 preview_bg_top = ImVec4(18.0f / 255.0f, 18.0f / 255.0f, 22.0f / 255.0f, 0.95f * esp_preview_alpha);
					ImVec4 preview_bg_bottom = ImVec4(10.0f / 255.0f, 10.0f / 255.0f, 14.0f / 255.0f, 0.95f * esp_preview_alpha);
					
					draw_list->AddRectFilledMultiColor(
						preview_scaled_min, preview_scaled_max,
						draw->get_clr(preview_bg_top), draw->get_clr(preview_bg_top),
						draw->get_clr(preview_bg_bottom), draw->get_clr(preview_bg_bottom)
					);
					
					// discord.gg/dopamina
					for (int b = 0; b < 16; b++)
					{
						float blur_inset = SCALE(b * 0.8f);
						float blur_alpha = 0.06f * esp_preview_alpha;
						
						float brightness = (b % 2 == 0) ? 45.0f : 12.0f;
						ImVec4 blur_color = ImVec4(brightness / 255.0f, brightness / 255.0f, (brightness + 5.0f) / 255.0f, blur_alpha);
						
						ImVec2 blur_min = ImVec2(preview_scaled_min.x + blur_inset, preview_scaled_min.y + blur_inset);
						ImVec2 blur_max = ImVec2(preview_scaled_max.x - blur_inset, preview_scaled_max.y - blur_inset);
						
						draw_list->AddRectFilled(blur_min, blur_max, draw->get_clr(blur_color), 
							ImMax(SCALE(8.0f) - blur_inset, 0.0f));
					}
					
					draw_list->PopClipRect();
					
					// discord.gg/dopamina
					ImVec4 preview_border = ImVec4(179.0f / 255.0f, 143.0f / 255.0f, 228.0f / 255.0f, 0.15f * esp_preview_alpha);
					draw_list->AddRect(preview_scaled_min, preview_scaled_max, draw->get_clr(preview_border), SCALE(8.0f), 0, SCALE(1.5f));
					
					// discord.gg/dopamina
					if (btn_font)
					{
						const char* header_text = "3d esp preview";
						ImVec2 header_size = btn_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, header_text);
						ImVec2 header_pos = ImVec2(preview_scaled_min.x + SCALE(15.0f), preview_scaled_min.y + SCALE(15.0f));
						
						ImVec4 header_color = ImVec4(179.0f / 255.0f, 143.0f / 255.0f, 228.0f / 255.0f, esp_preview_alpha);
						draw_list->AddText(btn_font, 13.0f, header_pos, draw->get_clr(header_color), header_text);
					}
					
					// discord.gg/dopamina
				}
			}
			
			ImGui::PopStyleVar();  // Menu Open Alpha
		}
		gui->end();
	}
	catch (...)
	{
		// Handle any exceptions silently
	}

	// discord.gg/dopamina
	// discord.gg/dopamina
	
	// discord.gg/dopamina
	if (var->gui.playerlist_spawn_alpha > 0.001f)
		playerlist->render();
	
	if (var->gui.watermark_spawn_alpha > 0.001f)
		watermark->render();

	if (var->gui.active_hotkeys_spawn_alpha > 0.001f)
	{
		keybinds->update();
		keybinds->render();
	}

	if (var->gui.playerbar_spawn_alpha > 0.001f)
		playerbar->render();

	// discord.gg/dopamina
	if (var->gui.spotify_spawn_alpha > 0.001f)
	{
		musicplayer->update();
		musicplayer->render();
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
