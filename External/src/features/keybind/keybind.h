#pragma once
#include <windows.h>
#include <string>
#include <imgui.h>

namespace keybind
{
	enum class activation_mode
	{
		toggle,
		hold,
		always
	};

	struct keybind_t
	{
		int key = 0;
		activation_mode mode = activation_mode::hold;
		bool state = false;
		bool was_pressed = false;
	};

	struct key_data_t
	{
		const char* name;
		int value;
	};

	extern key_data_t key_names[];
	extern int key_count;

	const char* get_key_name(int key);
	bool is_active(keybind_t& kb);
	ImGuiKey vk_to_imgui_key(int vk);
	int imgui_key_to_vk(ImGuiKey key);
}
