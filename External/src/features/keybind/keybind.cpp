#include "keybind.h"
#include <imgui.h>
#include <cstdio>
#include <game/game.h>

namespace keybind
{
	key_data_t key_names[] = {
		{ "none", 0 },
		{ "lm", VK_LBUTTON },
		{ "rm", VK_RBUTTON },
		{ "mm", VK_MBUTTON },
		{ "mb1", VK_XBUTTON1 },
		{ "mb2", VK_XBUTTON2 },
		{ "backspace", VK_BACK },
		{ "tab", VK_TAB },
		{ "enter", VK_RETURN },
		{ "shift", VK_SHIFT },
		{ "ctrl", VK_CONTROL },
		{ "alt", VK_MENU },
		{ "pause", VK_PAUSE },
		{ "caps lock", VK_CAPITAL },
		{ "escape", VK_ESCAPE },
		{ "space", VK_SPACE },
		{ "page up", VK_PRIOR },
		{ "page down", VK_NEXT },
		{ "end", VK_END },
		{ "home", VK_HOME },
		{ "left arrow", VK_LEFT },
		{ "up arrow", VK_UP },
		{ "right arrow", VK_RIGHT },
		{ "down arrow", VK_DOWN },
		{ "insert", VK_INSERT },
		{ "delete", VK_DELETE },
		{ "0", '0' },
		{ "1", '1' },
		{ "2", '2' },
		{ "3", '3' },
		{ "4", '4' },
		{ "5", '5' },
		{ "6", '6' },
		{ "7", '7' },
		{ "8", '8' },
		{ "9", '9' },
		{ "a", 'A' },
		{ "b", 'B' },
		{ "c", 'C' },
		{ "d", 'D' },
		{ "e", 'E' },
		{ "f", 'F' },
		{ "g", 'G' },
		{ "h", 'H' },
		{ "i", 'I' },
		{ "j", 'J' },
		{ "k", 'K' },
		{ "l", 'L' },
		{ "m", 'M' },
		{ "n", 'N' },
		{ "o", 'O' },
		{ "p", 'P' },
		{ "q", 'Q' },
		{ "r", 'R' },
		{ "s", 'S' },
		{ "t", 'T' },
		{ "u", 'U' },
		{ "v", 'V' },
		{ "w", 'W' },
		{ "x", 'X' },
		{ "y", 'Y' },
		{ "z", 'Z' },
		{ "left Win", VK_LWIN },
		{ "right Win", VK_RWIN },
		{ "apps", VK_APPS },
		{ "numpad 0", VK_NUMPAD0 },
		{ "numpad 1", VK_NUMPAD1 },
		{ "numpad 2", VK_NUMPAD2 },
		{ "numpad 3", VK_NUMPAD3 },
		{ "numpad 4", VK_NUMPAD4 },
		{ "numpad 5", VK_NUMPAD5 },
		{ "numpad 6", VK_NUMPAD6 },
		{ "numpad 7", VK_NUMPAD7 },
		{ "numpad 8", VK_NUMPAD8 },
		{ "numpad 9", VK_NUMPAD9 },
		{ "multiply", VK_MULTIPLY },
		{ "add", VK_ADD },
		{ "separator", VK_SEPARATOR },
		{ "subtract", VK_SUBTRACT },
		{ "decimal", VK_DECIMAL },
		{ "divide", VK_DIVIDE },
		{ "f1", VK_F1 },
		{ "f2", VK_F2 },
		{ "f3", VK_F3 },
		{ "f4", VK_F4 },
		{ "f5", VK_F5 },
		{ "f6", VK_F6 },
		{ "f7", VK_F7 },
		{ "f8", VK_F8 },
		{ "f9", VK_F9 },
		{ "f10", VK_F10 },
		{ "f11", VK_F11 },
		{ "f12", VK_F12 },
		{ "num Lock", VK_NUMLOCK },
		{ "scroll Lock", VK_SCROLL },
		{ "semicolon", VK_OEM_1 },
		{ "squals", VK_OEM_PLUS },
		{ "comma", VK_OEM_COMMA },
		{ "minus", VK_OEM_MINUS },
		{ "period", VK_OEM_PERIOD },
		{ "slash", VK_OEM_2 },
		{ "tilde", VK_OEM_3 },
		{ "left bracket", VK_OEM_4 },
		{ "backslash", VK_OEM_5 },
		{ "right bracket", VK_OEM_6 },
		{ "quote", VK_OEM_7 }
	};

	int key_count = static_cast<int>(sizeof(key_names) / sizeof(key_names[0]));

	const char* get_key_name(int key)
	{
		if (key == VK_LSHIFT || key == VK_RSHIFT)
			key = VK_SHIFT;

		if (key == VK_LCONTROL || key == VK_RCONTROL)
			key = VK_CONTROL;

		if (key == VK_LMENU || key == VK_RMENU)
			key = VK_MENU;

		if (key == 0)
			return "None";

		for (int i = 0; i < key_count; i++)
		{
			if (key_names[i].value == key)
			{
				return key_names[i].name;
			}
		}

		if (key >= 'A' && key <= 'Z')
		{
			static char key_buf[2] = { 0 };
			key_buf[0] = static_cast<char>(key + 32);
			return key_buf;
		}

		if (key >= '0' && key <= '9')
		{
			static char key_buf[2] = { 0 };
			key_buf[0] = static_cast<char>(key);
			return key_buf;
		}

		if (key >= VK_F1 && key <= VK_F12)
		{
			static char f_key[4] = { 0 };
			sprintf_s(f_key, "f%d", key - VK_F1 + 1);
			return f_key;
		}

		return "Unknown";
	}

	bool is_active(keybind_t& kb)
	{
		if (kb.mode == activation_mode::always)
		{
			return true;
		}

		if (kb.key == 0)
		{
			return false;
		}

		HWND foreground_window = GetForegroundWindow();
		HWND roblox_window = game::get_roblox_window();
		bool is_game_focused = (foreground_window == roblox_window);

		if (!is_game_focused)
		{
			if (kb.mode == activation_mode::toggle)
			{
				kb.was_pressed = false;
			}
			return false;
		}

		bool is_pressed = (GetAsyncKeyState(kb.key) & 0x8000) != 0;

		if (kb.mode == activation_mode::always)
		{
			return true;
		}
		else if (kb.mode == activation_mode::hold)
		{
			return is_pressed;
		}
		else if (kb.mode == activation_mode::toggle)
		{
			if (is_pressed && !kb.was_pressed)
			{
				kb.state = !kb.state;
				kb.was_pressed = true;
			}
			else if (!is_pressed)
			{
				kb.was_pressed = false;
			}
			return kb.state;
		}

		return false;
	}

	ImGuiKey vk_to_imgui_key(int vk)
	{
		if (vk >= 'A' && vk <= 'Z') return static_cast<ImGuiKey>(ImGuiKey_A + (vk - 'A'));
		if (vk >= '0' && vk <= '9') return static_cast<ImGuiKey>(ImGuiKey_0 + (vk - '0'));

		switch (vk)
		{
		case VK_LBUTTON: return ImGuiKey_MouseLeft;
		case VK_RBUTTON: return ImGuiKey_MouseRight;
		case VK_MBUTTON: return ImGuiKey_MouseMiddle;
		case VK_XBUTTON1: return ImGuiKey_MouseX1;
		case VK_XBUTTON2: return ImGuiKey_MouseX2;
		case VK_BACK: return ImGuiKey_Backspace;
		case VK_TAB: return ImGuiKey_Tab;
		case VK_RETURN: return ImGuiKey_Enter;
		case VK_LSHIFT:
		case VK_RSHIFT: return ImGuiKey_LeftShift;
		case VK_LCONTROL:
		case VK_RCONTROL: return ImGuiKey_LeftCtrl;
		case VK_LMENU:
		case VK_RMENU: return ImGuiKey_LeftAlt;
		case VK_LWIN: return ImGuiKey_LeftSuper;
		case VK_RWIN: return ImGuiKey_RightSuper;
		case VK_PAUSE: return ImGuiKey_Pause;
		case VK_CAPITAL: return ImGuiKey_CapsLock;
		case VK_ESCAPE: return ImGuiKey_Escape;
		case VK_SPACE: return ImGuiKey_Space;
		case VK_PRIOR: return ImGuiKey_PageUp;
		case VK_NEXT: return ImGuiKey_PageDown;
		case VK_END: return ImGuiKey_End;
		case VK_HOME: return ImGuiKey_Home;
		case VK_LEFT: return ImGuiKey_LeftArrow;
		case VK_UP: return ImGuiKey_UpArrow;
		case VK_RIGHT: return ImGuiKey_RightArrow;
		case VK_DOWN: return ImGuiKey_DownArrow;
		case VK_INSERT: return ImGuiKey_Insert;
		case VK_DELETE: return ImGuiKey_Delete;
		case VK_F1: return ImGuiKey_F1;
		case VK_F2: return ImGuiKey_F2;
		case VK_F3: return ImGuiKey_F3;
		case VK_F4: return ImGuiKey_F4;
		case VK_F5: return ImGuiKey_F5;
		case VK_F6: return ImGuiKey_F6;
		case VK_F7: return ImGuiKey_F7;
		case VK_F8: return ImGuiKey_F8;
		case VK_F9: return ImGuiKey_F9;
		case VK_F10: return ImGuiKey_F10;
		case VK_F11: return ImGuiKey_F11;
		case VK_F12: return ImGuiKey_F12;
		default: return ImGuiKey_None;
		}
	}

	int imgui_key_to_vk(ImGuiKey key)
	{
		if (key >= ImGuiKey_A && key <= ImGuiKey_Z) return 'A' + (key - ImGuiKey_A);
		if (key >= ImGuiKey_0 && key <= ImGuiKey_9) return '0' + (key - ImGuiKey_0);

		switch (key)
		{
		case ImGuiKey_MouseLeft: return VK_LBUTTON;
		case ImGuiKey_MouseRight: return VK_RBUTTON;
		case ImGuiKey_MouseMiddle: return VK_MBUTTON;
		case ImGuiKey_MouseX1: return VK_XBUTTON1;
		case ImGuiKey_MouseX2: return VK_XBUTTON2;
		case ImGuiKey_Backspace: return VK_BACK;
		case ImGuiKey_Tab: return VK_TAB;
		case ImGuiKey_Enter: return VK_RETURN;
		case ImGuiKey_LeftShift:
		case ImGuiKey_RightShift: return VK_LSHIFT;
		case ImGuiKey_LeftCtrl:
		case ImGuiKey_RightCtrl: return VK_LCONTROL;
		case ImGuiKey_LeftAlt:
		case ImGuiKey_RightAlt: return VK_LMENU;
		case ImGuiKey_LeftSuper: return VK_LWIN;
		case ImGuiKey_RightSuper: return VK_RWIN;
		case ImGuiKey_Pause: return VK_PAUSE;
		case ImGuiKey_CapsLock: return VK_CAPITAL;
		case ImGuiKey_Escape: return VK_ESCAPE;
		case ImGuiKey_Space: return VK_SPACE;
		case ImGuiKey_PageUp: return VK_PRIOR;
		case ImGuiKey_PageDown: return VK_NEXT;
		case ImGuiKey_End: return VK_END;
		case ImGuiKey_Home: return VK_HOME;
		case ImGuiKey_LeftArrow: return VK_LEFT;
		case ImGuiKey_UpArrow: return VK_UP;
		case ImGuiKey_RightArrow: return VK_RIGHT;
		case ImGuiKey_DownArrow: return VK_DOWN;
		case ImGuiKey_Insert: return VK_INSERT;
		case ImGuiKey_Delete: return VK_DELETE;
		case ImGuiKey_F1: return VK_F1;
		case ImGuiKey_F2: return VK_F2;
		case ImGuiKey_F3: return VK_F3;
		case ImGuiKey_F4: return VK_F4;
		case ImGuiKey_F5: return VK_F5;
		case ImGuiKey_F6: return VK_F6;
		case ImGuiKey_F7: return VK_F7;
		case ImGuiKey_F8: return VK_F8;
		case ImGuiKey_F9: return VK_F9;
		case ImGuiKey_F10: return VK_F10;
		case ImGuiKey_F11: return VK_F11;
		case ImGuiKey_F12: return VK_F12;
		default: return 0;
		}
	}
}
