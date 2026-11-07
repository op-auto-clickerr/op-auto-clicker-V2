#pragma once
#include <string>
#include <vector>
#include "../../thirdparty/imgui/imgui.h"
#include "../headers/flags.h"


class c_variables
{
public:

	struct
	{
		float dpi = 1.f;
		int stored_dpi = 100;
		bool dpi_changed = true;
		bool menu_open = true;  // discord.gg/dopamina
		int active_tab = 0;  // discord.gg/dopamina
		float tab_transition_alpha = 1.0f;  // discord.gg/dopamina
		float menu_open_alpha = 1.0f;  // discord.gg/dopamina
		bool menu_was_closed = false;  // discord.gg/dopamina
		
		// UI Element Toggles
		bool show_playerlist = true;
		bool show_playerbar = true;
		bool show_spotify = true;
		bool show_active_hotkeys = true;
		bool show_watermark = true;
		
		// discord.gg/dopamina
		bool playerlist_spawned = false;
		bool playerbar_spawned = false;
		bool spotify_spawned = false;
		bool active_hotkeys_spawned = false;
		bool watermark_spawned = false;
		
		float playerlist_spawn_alpha = 0.0f;
		float playerbar_spawn_alpha = 0.0f;
		float spotify_spawn_alpha = 0.0f;
		float active_hotkeys_spawn_alpha = 0.0f;
		float watermark_spawn_alpha = 0.0f;
	} gui;

	gui_style style;

};

inline std::unique_ptr<c_variables> var = std::make_unique<c_variables>();
