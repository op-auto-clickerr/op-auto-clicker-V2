#define IMGUI_DEFINE_MATH_OPERATORS
#include <render/render.h>

#include <dwmapi.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <cfloat>

#include <settings.h>
#include <features/esp/esp.h>
#include <features/rage/desync/desync.h>
#include <features/silentaim/silentaim.h>
#include <features/triggerbot/triggerbot.h>
#include <cache/cache.h>
#include <cache/custom_entities/custom_entities.h>
#include <cache/bodyparts/bodyparts.h>
#include <features/esp/parser/meshparser.h>
#include <features/explorer/explorer.h>
#include <memory/memory.h>
#include <game/game.h>
#include <Offsets/Offsets.hpp>
#include <features/keybind/keybind.h>
#include <features/config/config.h>
#include <ui/ui.hpp>
#include <cstring>
#include <cstdlib>

namespace helper
{
	void draw_text_outlined(ImDrawList* draw, ImFont* font, float font_size, ImVec2 pos, ImU32 col, const char* text_begin, const char* text_end = nullptr);
	void box(ImVec2& c1, ImVec2& c2, ImU32 color);
	void corner_box(ImDrawList* draw, ImVec2 min, ImVec2 max, ImU32 col, float thickness = 1.f);
}
#include "../../ext/font/tahoma.h"
#include "../../ext/font/sp7.h"
#include "../../ext/font/arial.h"
#include "../../ext/font/config/font_config.h"
#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_internal.h"
#include "../../ext/imgui/misc/imgui_freetype.h"
#include "textures/texture.h"
#include <features/aimbot/aimbot.h>
#include "../../ext/dicons/warn.h"
#include "3d/main_api.hpp"
#include "3d/parsers/obj_parser.hpp"
#include "3d/texture/texture_cache.hpp"
#include <features/avatarmanager/avatarmanager.h>
#include <wallcheck/wallcheck.h>
#include <features/radar/radar.h>
#include <features/rage/hittracers/hittracers.h>
#include <gamesupport/gamesupport.h>
#include <gamesupport/LumberTycoon2/lt2.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	void log_stall(const char* stage, double ms)
	{
		if (ms < 200.0)
			return;

		char path[MAX_PATH]{};
		if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
			return;

		char* slash = strrchr(path, '\\');
		if (!slash)
			return;

		strcpy_s(slash + 1, MAX_PATH - static_cast<size_t>((slash + 1) - path), "stall.log");

		FILE* file = nullptr;
		if (fopen_s(&file, path, "a") != 0 || file == nullptr)
			return;

		SYSTEMTIME time{};
		GetLocalTime(&time);
		fprintf(file, "%02u:%02u:%02u.%03u  %-20s %9.1f ms\n",
			time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, stage, ms);
		fclose(file);
	}

	double ms_since(const std::chrono::steady_clock::time_point& start)
	{
		return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	}
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
	{
		return true;
	}

	switch (msg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
		{
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		if (wParam == VK_F4) {
			DestroyWindow(hwnd);
			return 0;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		return 0;
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

render_t::render_t()
{
	detail = std::make_unique<detail_t>();
}

render_t::~render_t()
{
	destroy_imgui();
	destroy_window();
	destroy_device();
}

bool render_t::create_window()
{
	detail->window_class.cbSize = sizeof(detail->window_class);
	detail->window_class.style = CS_CLASSDC;
	detail->window_class.lpszClassName = "T4";
	detail->window_class.hInstance = GetModuleHandleA(0);
	detail->window_class.lpfnWndProc = wnd_proc;

	RegisterClassExA(&detail->window_class);

	detail->window = CreateWindowExA(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		detail->window_class.lpszClassName,
		"T4",
		WS_POPUP,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		0,
		0,
		detail->window_class.hInstance,
		0
	);

	if (!detail->window)
	{
		return false;
	}

	SetLayeredWindowAttributes(detail->window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	RECT client_area{};
	RECT window_area{};

	GetClientRect(detail->window, &client_area);
	GetWindowRect(detail->window, &window_area);

	POINT diff{};
	ClientToScreen(detail->window, &diff);

	MARGINS margins
	{
		window_area.left + (diff.x - window_area.left),
		window_area.top + (diff.y - window_area.top),
		window_area.right,
		window_area.bottom,
	};

	DwmExtendFrameIntoClientArea(detail->window, &margins);

	ShowWindow(detail->window, SW_SHOW);
	UpdateWindow(detail->window);

	return true;
}

bool render_t::create_device()
{
	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};

	swap_chain_desc.BufferCount = 1;

	swap_chain_desc.BufferDesc.Width = 0;
	swap_chain_desc.BufferDesc.Height = 0;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swap_chain_desc.OutputWindow = detail->window;

	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Windowed = 1;

	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	swap_chain_desc.SampleDesc.Count = 2;
	swap_chain_desc.SampleDesc.Quality = 0;

	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	D3D_FEATURE_LEVEL feature_level;
	D3D_FEATURE_LEVEL feature_level_list[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		feature_level_list,
		2,
		D3D11_SDK_VERSION,
		&swap_chain_desc,
		&detail->swap_chain,
		&detail->device,
		&feature_level,
		&detail->device_context
	);

	if (result == DXGI_ERROR_UNSUPPORTED)
	{
		result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0,
			feature_level_list,
			2,
			D3D11_SDK_VERSION,
			&swap_chain_desc,
			&detail->swap_chain,
			&detail->device,
			&feature_level,
			&detail->device_context
		);
	}

	if (result != S_OK)
	{
		MessageBoxA(nullptr, "This software can not run on your computer.", "Critical Problem", MB_ICONERROR | MB_OK);
	}

	ID3D11Texture2D* back_buffer{ nullptr };
	detail->swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

	if (back_buffer)
	{
		detail->device->CreateRenderTargetView(back_buffer, nullptr, &detail->render_target_view);
		back_buffer->Release();

		return true;
	}

	return false;
}

bool imgui_initialized = false;

bool render_t::create_imgui()
{
	using namespace ImGui;
	CreateContext();

	float main_scale = 1.0f;

	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	style.WindowRounding = 0;
	style.ChildRounding = 0;
	style.FrameRounding = 0;
	style.ScrollbarRounding = 0;
	style.GrabRounding = 0;
	style.PopupRounding = 0;
	style.TabRounding = 0;

	style.WindowBorderSize = 1;
	style.FrameBorderSize = 1;
	style.PopupBorderSize = 1;

	style.WindowPadding = ImVec2(8, 8);
	style.FramePadding = ImVec2(2, 2);
	style.ItemSpacing = ImVec2(8, 6);
	style.ItemInnerSpacing = ImVec2(3, 3);
	style.CellPadding = ImVec2(2, 1);

	style.WindowMinSize = ImVec2(0, 0);
	style.GrabMinSize = 10;
	style.ScrollbarSize = 1;

	style.Colors[ImGuiCol_WindowBg] = ImColor(16, 16, 16);
	style.Colors[ImGuiCol_ChildBg] = ImColor(17, 17, 17);
	style.Colors[ImGuiCol_PopupBg] = ImColor(16, 16, 16);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(20, 20, 20);

	style.Colors[ImGuiCol_SliderGrab] = ImColor(118, 125, 216);
	style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_SliderGrab];
	style.Colors[ImGuiCol_SliderGrabActive].w = 0.5f;
	style.Colors[ImGuiCol_TextSelectedBg] = style.Colors[ImGuiCol_SliderGrabActive];

	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4();
	style.Colors[ImGuiCol_ScrollbarGrab] = style.Colors[ImGuiCol_SliderGrabActive];
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = style.Colors[ImGuiCol_ScrollbarGrab];
	style.Colors[ImGuiCol_ScrollbarGrabActive] = style.Colors[ImGuiCol_ScrollbarGrab];

	style.Colors[ImGuiCol_Border] = ImColor(82, 82, 82, 150);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0);
	style.Colors[ImGuiCol_Separator] = ImColor(58, 58, 58);

	style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(146, 146, 146);

	style.Colors[ImGuiCol_FrameBg] = ImColor(13, 13, 13);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(16, 16, 16);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(11, 11, 11);

	style.Colors[ImGuiCol_Button] = ImColor(23, 23, 23);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(27, 27, 27);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(20, 20, 20);

	style.ScaleAllSizes(main_scale);
	style.FontScaleDpi = main_scale;

	io.Fonts->AddFontDefault();

	float dpi_scale = main_scale > 0.0f ? main_scale : 1.0f;

	ImFontConfig font_cfg_tahoma;
	font_cfg_tahoma.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_MonoHinting | ImGuiFreeTypeLoaderFlags_Monochrome;
	font_cfg_tahoma.PixelSnapH = true;
	font_cfg_tahoma.OversampleH = 2;
	font_cfg_tahoma.OversampleV = 1;
	font_cfg_tahoma.RasterizerMultiply = 1.05f;
	font_cfg_tahoma.GlyphOffset = ImVec2(0, 1);
	esp_font_tahoma = io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(rawData)), sizeof(rawData), font_config::tahoma::font_size / dpi_scale, &font_cfg_tahoma, nullptr);

	ImFontConfig font_cfg_sp7;
	font_cfg_sp7.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_ForceAutoHint;
	font_cfg_sp7.GlyphOffset = ImVec2(0, 1);
	esp_font_sp7 = io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(font_sp7)), font_sp7_size, font_config::sp7::font_size / dpi_scale, &font_cfg_sp7, nullptr);

	ImFontConfig font_cfg_arial;
	font_cfg_arial.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_ForceAutoHint;
	font_cfg_arial.GlyphOffset = ImVec2(0, 1);
	esp_font_arial = io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(font_arial)), font_arial_size, font_config::arial::font_size / dpi_scale, &font_cfg_arial, nullptr);

	// Add GothamBold font (using Arial as fallback since we don't have the actual font data)
	ImFontConfig font_cfg_gotham_bold;
	font_cfg_gotham_bold.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_Bold | ImGuiFreeTypeLoaderFlags_ForceAutoHint;
	font_cfg_gotham_bold.GlyphOffset = ImVec2(0, 1);
	esp_font_gotham_bold = io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(font_arial)), font_arial_size, font_config::gotham_bold::font_size / dpi_scale, &font_cfg_gotham_bold, nullptr);

	if (settings::visuals::esp_font == 0)
		esp_font = esp_font_tahoma;
	else if (settings::visuals::esp_font == 1)
		esp_font = esp_font_sp7;
	else if (settings::visuals::esp_font == 2)
		esp_font = esp_font_arial;
	else
		esp_font = esp_font_gotham_bold;

	if (!ImGui_ImplWin32_Init(detail->window))
	{
		return false;
	}

	if (!detail->device || !detail->device_context)
	{
		return false;
	}

	if (!ImGui_ImplDX11_Init(detail->device, detail->device_context))
	{
		return false;
	}

	imgui_initialized = true;

	if (detail->device && detail->device_context)
	{
		g_avatar_manager = std::make_unique<AvatarManager>(detail->device, detail->device_context);
	}

	return true;
}

void render_t::destroy_device()
{
	if (detail->render_target_view) detail->render_target_view->Release();
	if (detail->swap_chain) detail->swap_chain->Release();
	if (detail->device_context) detail->device_context->Release();
	if (detail->device) detail->device->Release();
}

void render_t::destroy_window()
{
	DestroyWindow(detail->window);
	UnregisterClassA(detail->window_class.lpszClassName, detail->window_class.hInstance);
}

void render_t::destroy_imgui()
{
	if (!imgui_initialized) return;
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	imgui_initialized = false;
}

void render_t::start_render()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (settings::menu::streamproof)
	{
		SetWindowDisplayAffinity(detail->window, WDA_EXCLUDEFROMCAPTURE);
	}
	else
	{
		SetWindowDisplayAffinity(detail->window, WDA_NONE);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (settings::visuals::esp_font == 0)
		esp_font = esp_font_tahoma;
	else if (settings::visuals::esp_font == 1)
		esp_font = esp_font_sp7;
	else if (settings::visuals::esp_font == 2)
		esp_font = esp_font_arial;
	else
		esp_font = esp_font_gotham_bold;

	if (GetAsyncKeyState(settings::menu::menu_keybind) & 1)
	{
		running = !running;

		if (running)
		{
			SetWindowLong(detail->window, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED);
		}
		else
		{
			SetWindowLong(detail->window, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED);
		}
	}
}

void render_t::end_render()
{
	ImGui::Render();

	float clear_color[4]{ 0, 0, 0, 0 };
	detail->device_context->OMSetRenderTargets(1, &detail->render_target_view, nullptr);
	detail->device_context->ClearRenderTargetView(detail->render_target_view, clear_color);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	detail->swap_chain->Present(settings::menu::vsync ? 1 : 0, 0);
}

void render_t::render_menu()
{
	ui::render();
}

static void draw_fov_circle(float fov_radius, float circle_colour[4], float outline_colour[4], bool attach_to_target = false)
{
	if (game::visengine.address == 0)
	{
		return;
	}

	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	ImVec2 target_center{};
	float current_radius = fov_radius;

	if (attach_to_target && settings::silentaim::enabled && silentaim::state.data_ready && silentaim::state.target.instance.address != 0)
	{
		const math::matrix4 view = game::visengine.get_viewmatrix();
		const math::vector2 dims = game::visengine.get_dimensions();

		math::vector2 target_screen_pos;
		if (game::visengine.world_to_screen(silentaim::state.target_world_pos, target_screen_pos, dims, view))
		{
			target_center = ImVec2(target_screen_pos.x, target_screen_pos.y);
		}
		else
		{
			POINT cursor{};
			if (!GetCursorPos(&cursor))
				return;
			target_center = ImVec2(static_cast<float>(cursor.x), static_cast<float>(cursor.y));
		}
	}
	else
	{
		POINT cursor{};
		if (!GetCursorPos(&cursor))
		{
			return;
		}
		target_center = ImVec2(static_cast<float>(cursor.x), static_cast<float>(cursor.y));
	}

	ImVec2 center = target_center;
	current_radius = fov_radius;

	ImU32 outline_col = IM_COL32(
		static_cast<int>(outline_colour[0] * 255.f),
		static_cast<int>(outline_colour[1] * 255.f),
		static_cast<int>(outline_colour[2] * 255.f),
		static_cast<int>(outline_colour[3] * 255.f)
	);

	ImU32 circle_col = IM_COL32(
		static_cast<int>(circle_colour[0] * 255.f),
		static_cast<int>(circle_colour[1] * 255.f),
		static_cast<int>(circle_colour[2] * 255.f),
		static_cast<int>(circle_colour[3] * 255.f)
	);

	draw->AddCircle(center, current_radius, outline_col, 0, 3.f);
	draw->AddCircle(center, current_radius, circle_col, 0, 1.f);
}

static void draw_custom_cursor()
{
	if (!settings::silentaim::enabled || !silentaim::state.aim_indicator.address)
	{
		return;
	}

	bool is_visible = false;
	is_visible = memory->read<bool>(silentaim::state.aim_indicator.address + Offsets::GuiObject::Visible);

	if (!is_visible)
	{
		return;
	}

	POINT pt;
	if (!GetCursorPos(&pt))
	{
		return;
	}

	bool right_click_held = GetAsyncKeyState(VK_RBUTTON);
	float gap = right_click_held ? 4.0f : 10.0f;

	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	ImU32 col = IM_COL32(255, 255, 255, 255);

	float dot_size = 4.0f;
	float line_width = 2.0f;
	float line_length = 10.0f;

	ImVec2 center = { (float)pt.x, (float)pt.y };

	ImVec2 dot_min(center.x - dot_size * 0.5f, center.y - dot_size * 0.5f);
	ImVec2 dot_max(center.x + dot_size * 0.5f, center.y + dot_size * 0.5f);
	draw->AddRectFilled(dot_min, dot_max, col, 0.0f);

	ImVec2 top_min(center.x - line_width * 0.5f, center.y - gap - line_length);
	ImVec2 top_max(center.x + line_width * 0.5f, center.y - gap);
	draw->AddRectFilled(top_min, top_max, col, 0.0f);

	ImVec2 bottom_min(center.x - line_width * 0.5f, center.y + gap);
	ImVec2 bottom_max(center.x + line_width * 0.5f, center.y + gap + line_length);
	draw->AddRectFilled(bottom_min, bottom_max, col, 0.0f);

	ImVec2 left_min(center.x - gap - line_length, center.y - line_width * 0.5f);
	ImVec2 left_max(center.x - gap, center.y + line_width * 0.5f);
	draw->AddRectFilled(left_min, left_max, col, 0.0f);

	ImVec2 right_min(center.x + gap, center.y - line_width * 0.5f);
	ImVec2 right_max(center.x + gap + line_length, center.y + line_width * 0.5f);
	draw->AddRectFilled(right_min, right_max, col, 0.0f);
}

static void draw_silent_aim_target_dot()
{
	if (!settings::silentaim::draw_target_dot || !settings::silentaim::enabled)
		return;

	if (!silentaim::state.data_ready || !silentaim::state.target.instance.address)
		return;

	if (!game::visengine.address)
		return;

	const math::matrix4 view = game::visengine.get_viewmatrix();
	const math::vector2 dims = game::visengine.get_dimensions();

	math::vector2 screen_pos;
	if (!game::visengine.world_to_screen(silentaim::state.target_world_pos, screen_pos, dims, view))
		return;

	if (screen_pos.x < 0.0f || screen_pos.y < 0.0f || screen_pos.x > dims.x || screen_pos.y > dims.y)
		return;

	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	ImU32 dot_color = IM_COL32(
		static_cast<int>(settings::silentaim::target_dot_color[0] * 255.f),
		static_cast<int>(settings::silentaim::target_dot_color[1] * 255.f),
		static_cast<int>(settings::silentaim::target_dot_color[2] * 255.f),
		static_cast<int>(settings::silentaim::target_dot_color[3] * 255.f)
	);

	ImVec2 center(screen_pos.x, screen_pos.y);
	float dot_size = settings::silentaim::target_dot_size;

	draw->AddCircleFilled(center, dot_size, dot_color, 0);
}

static void draw_silent_aim_snap_line()
{
	if (!settings::silentaim::draw_snap_line)
		return;

	// Check if either aimbot or silent aim is enabled and has a target
	bool has_silentaim_target = settings::silentaim::enabled && silentaim::state.data_ready && silentaim::state.target.instance.address != 0;
	bool has_aimbot_target = settings::aimbot::enabled && aimbot::player.instance.address != 0;

	if (!has_silentaim_target && !has_aimbot_target)
		return;

	if (!game::visengine.address)
		return;

	POINT cursor{};
	if (!GetCursorPos(&cursor))
		return;

	const math::matrix4 view = game::visengine.get_viewmatrix();
	const math::vector2 dims = game::visengine.get_dimensions();

	// Determine which target to use (prioritize silent aim if both are active)
	cache::entity_t target_entity = has_silentaim_target ? silentaim::state.target : aimbot::player;

	// Get torso position instead of head
	math::vector3 torso_world_pos;
	if (!bodyparts::get_part_position(target_entity, "Torso", torso_world_pos))
	{
		// Fallback to UpperTorso or HumanoidRootPart if Torso doesn't exist
		if (!bodyparts::get_part_position(target_entity, "UpperTorso", torso_world_pos))
		{
			if (!bodyparts::get_part_position(target_entity, "HumanoidRootPart", torso_world_pos))
				return;
		}
	}

	math::vector2 torso_screen_pos;
	if (!game::visengine.world_to_screen(torso_world_pos, torso_screen_pos, dims, view))
		return;

	if (torso_screen_pos.x < 0.0f || torso_screen_pos.y < 0.0f || torso_screen_pos.x > 10000.0f || torso_screen_pos.y > 10000.0f)
		return;

	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	ImVec2 cursor_pos(static_cast<float>(cursor.x), static_cast<float>(cursor.y));
	ImVec2 torso_pos(torso_screen_pos.x, torso_screen_pos.y);

	float dx = torso_pos.x - cursor_pos.x;
	float dy = torso_pos.y - cursor_pos.y;
	float distance = std::sqrtf(dx * dx + dy * dy);

	if (distance > 0.0f)
	{
		ImDrawListFlags old_flags = draw->Flags;
		draw->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

		// Get base color from settings
		ImU32 base_color = IM_COL32(
			static_cast<int>(settings::silentaim::snap_line_color[0] * 255.f),
			static_cast<int>(settings::silentaim::snap_line_color[1] * 255.f),
			static_cast<int>(settings::silentaim::snap_line_color[2] * 255.f),
			255
		);

		// Draw very thin line with minimal glow
		// Subtle outer glow
		draw->AddLine(cursor_pos, torso_pos, IM_COL32(
			static_cast<int>(settings::silentaim::snap_line_color[0] * 255.f),
			static_cast<int>(settings::silentaim::snap_line_color[1] * 255.f),
			static_cast<int>(settings::silentaim::snap_line_color[2] * 255.f),
			20
		), 3.0f);

		// Main line
		draw->AddLine(cursor_pos, torso_pos, base_color, 1.5f);

		draw->Flags = old_flags;
	}
}

static ID3D11ShaderResourceView* g_warn_icon_texture = nullptr;
static bool g_warn_icon_loaded = false;
std::unique_ptr<AvatarManager> g_avatar_manager = nullptr;

static void load_warn_icon()
{
	if (g_warn_icon_loaded || !render || !render->detail || !render->detail->device)
		return;

	const unsigned char* icon_data = &warn_icon_data[0];
	constexpr size_t icon_data_size = 9621;

	g_warn_icon_texture = D3D11CreateTextureFromBytes(
		render->detail->device,
		icon_data,
		icon_data_size
	);

	if (g_warn_icon_texture)
		g_warn_icon_loaded = true;
}

static ImU32 get_blink_color(float speed = 2.0f)
{
	float time = static_cast<float>(ImGui::GetTime()) * speed;
	float cycle = fmodf(time, 2.0f);

	if (cycle < 1.0f)
	{
		return IM_COL32(255, 0, 0, 255);
	}
	else
	{
		return IM_COL32(0, 0, 0, 255);
	}
}

static void draw_target_warning_icon()
{
	if (!settings::visuals::target_warning_icon)
		return;

	if (!game::visengine.address)
		return;

	load_warn_icon();

	if (!g_warn_icon_texture)
		return;

	cache::entity_t target_entity{};
	bool has_target = false;

	if (settings::aimbot::enabled && aimbot::player.instance.address != 0)
	{
		target_entity = aimbot::player;
		has_target = true;
	}
	else if (settings::silentaim::enabled && silentaim::state.data_ready && silentaim::state.target.instance.address != 0)
	{
		target_entity = silentaim::state.target;
		has_target = true;
	}

	if (!has_target)
		return;

	const math::matrix4 view = game::visengine.get_viewmatrix();
	const math::vector2 dims = game::visengine.get_dimensions();

	float left = FLT_MAX, top = FLT_MAX;
	float right = -FLT_MAX, bottom = -FLT_MAX;
	bool valid = false;

	static const math::vector3 corners[] = {
		{ -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
		{ -1, -1, 1 }, { 1, -1, 1 }, { 1, 1, 1 }, { -1, 1, 1 }
	};

	for (auto& parts : target_entity.parts)
	{
		rbx::part_t part = parts.second;
		rbx::primitive_t prim = part.get_primitive();
		auto size = prim.get_size();
		auto pos = prim.get_position();
		auto rot = prim.get_rotation();

		if (size.x == 0 || size.y == 0 || size.z == 0)
			continue;

		for (auto& corner : corners)
		{
			math::vector3 world = pos + rot * math::vector3
			{
				corner.x * size.x * 0.5f,
				corner.y * size.y * 0.5f,
				corner.z * size.z * 0.5f
			};

			math::vector2 out{};
			if (game::visengine.world_to_screen(world, out, dims, view))
			{
				valid = true;
				left = min(left, out.x);
				top = min(top, out.y);
				right = max(right, out.x);
				bottom = max(bottom, out.y);
			}
		}
	}

	if (!valid || left >= right || top >= bottom)
		return;

	float box_center_x = (left + right) * 0.5f;
	float box_top = top;

	ImFont* font;
	float font_size;
	if (settings::visuals::esp_font == 0)
	{
		font = esp_font_tahoma;
		font_size = font_config::tahoma::font_size;
	}
	else if (settings::visuals::esp_font == 1)
	{
		font = esp_font_sp7;
		font_size = font_config::sp7::font_size;
	}
	else if (settings::visuals::esp_font == 2)
	{
		font = esp_font_arial;
		font_size = font_config::arial::font_size;
	}
	else
	{
		font = esp_font_gotham_bold;
		font_size = font_config::gotham_bold::font_size;
	}
	if (!font)
		font = ImGui::GetFont();

	std::string name_to_display = target_entity.display_name.empty() ? target_entity.name : target_entity.display_name;
	float text_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, name_to_display.c_str()).x;
	float name_y = box_top - font_size - 5.f;

	float icon_size = 24.0f;
	float icon_y = name_y - icon_size - 3.0f;

	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	ImVec2 icon_min(box_center_x - icon_size * 0.5f, icon_y);
	ImVec2 icon_max(box_center_x + icon_size * 0.5f, icon_y + icon_size);

	ImU32 blink_color = get_blink_color(2.0f);
	draw->AddImage(reinterpret_cast<ImTextureID>(g_warn_icon_texture), icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), blink_color);
}

static std::string get_server_job_id()
{
	std::string job_id;
	try
	{
		if (game::datamodel.address != 0)
		{
			const std::uint64_t str_ptr = memory->read<std::uint64_t>(game::datamodel.address + Offsets::DataModel::JobId);
			if (str_ptr != 0)
				job_id = memory->read_string(str_ptr);
		}
	}
	catch (...)
	{
	}
	return job_id;
}

static void draw_watermark()
{
	if (!settings::ui::watermark)
		return;

	ImFont* font = esp_font_tahoma != nullptr ? esp_font_tahoma : ImGui::GetFont();
	if (font == nullptr)
		return;

	const float font_size = font_config::tahoma::font_size;
	static const char* separators[] = { " | ", " - ", " / ", " :: ", "  " };
	const char* sep = (settings::watermark::separator_type >= 0 && settings::watermark::separator_type < 5)
		? separators[settings::watermark::separator_type]
		: separators[0];

	std::vector<std::string> parts;

	if (settings::watermark::show_cheat_name)
		parts.push_back(external_config::cheat_name);

	if (settings::watermark::show_display_name && !cache::cached_local_player.display_name.empty())
		parts.push_back(cache::cached_local_player.display_name);

	if (settings::watermark::show_username && !cache::cached_local_player.name.empty())
		parts.push_back(cache::cached_local_player.name);

	if (settings::watermark::show_fps)
	{
		char fps[32];
		snprintf(fps, sizeof(fps), "%.0f FPS", ImGui::GetIO().Framerate);
		parts.push_back(fps);
	}

	if (settings::watermark::show_server_ip)
	{
		const std::string job_id = get_server_job_id();
		if (!job_id.empty())
			parts.push_back(job_id);
	}

	if (parts.empty())
		return;

	std::string text;
	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (i != 0)
			text += sep;
		text += parts[i];
	}

	ImU32 color = IM_COL32(
		static_cast<int>(settings::watermark::text_color[0] * 255.f),
		static_cast<int>(settings::watermark::text_color[1] * 255.f),
		static_cast<int>(settings::watermark::text_color[2] * 255.f),
		static_cast<int>(settings::watermark::text_color[3] * 255.f));

	if (settings::watermark::rainbow)
	{
		const float hue = fmodf(static_cast<float>(ImGui::GetTime()) * settings::watermark::rainbow_speed * 0.1f, 1.0f);
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(hue, 1.f, 1.f, r, g, b);
		color = IM_COL32(static_cast<int>(r * 255.f), static_cast<int>(g * 255.f), static_cast<int>(b * 255.f), 255);
	}

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, text.c_str());
	const ImVec2 pos(settings::watermark::pos_x, settings::watermark::pos_y);
	draw->AddRectFilled(ImVec2(pos.x - 6.f, pos.y - 4.f), ImVec2(pos.x + text_size.x + 6.f, pos.y + text_size.y + 4.f), IM_COL32(0, 0, 0, 180), 3.f);
	draw->AddText(font, font_size, pos, color, text.c_str());
}

struct keybind_list_entry
{
	const char* label;
	const int* key;
	const int* mode;
	const bool* enabled;
};

static void draw_keybind_list()
{
	if (!settings::ui::keybinds)
		return;

	static const keybind_list_entry entries[] = {
		{ "Aimbot", &settings::aimbot::keybind, &settings::aimbot::activation_mode, &settings::aimbot::enabled },
		{ "Silent Aim", &settings::silentaim::keybind, &settings::silentaim::activation_mode, &settings::silentaim::enabled },
		{ "Silent Trigger", &settings::silentaim::triggerbot::keybind, &settings::silentaim::triggerbot::activation_mode, &settings::silentaim::triggerbot::enabled },
		{ "Spin 360", &settings::rage::spin360::keybind, &settings::rage::spin360::activation_mode, &settings::rage::spin360::enabled },
		{ "Desync", &settings::desync::keybind, &settings::desync::keybind_mode, &settings::desync::enabled },
		{ "Magic Bullet", &settings::magicbullet::keybind, &settings::magicbullet::activation_mode, &settings::magicbullet::enabled },
		{ "Speed Hack", &settings::movement::speedhack::keybind, &settings::movement::speedhack::activation_mode, &settings::movement::speedhack::enabled },
		{ "Jump Hack", &settings::movement::jumphack::keybind, &settings::movement::jumphack::activation_mode, &settings::movement::jumphack::enabled },
		{ "Fly Hack", &settings::movement::flyhack::keybind, &settings::movement::flyhack::activation_mode, &settings::movement::flyhack::enabled },
		{ "Freeze Player", &settings::exploits::freezeplayer::keybind, &settings::exploits::freezeplayer::activation_mode, &settings::exploits::freezeplayer::enabled },
	};

	ImFont* font = esp_font_tahoma != nullptr ? esp_font_tahoma : ImGui::GetFont();
	if (font == nullptr)
		return;

	const float font_size = font_config::tahoma::font_size;
	constexpr float padding = 6.f;
	const float line_height = font_size + 6.f;

	float width = 0.f;
	int visible = 0;
	for (const auto& entry : entries)
	{
		if (*entry.key == 0)
			continue;

		char line[128];
		snprintf(line, sizeof(line), "%s  [%s]", entry.label, keybind::get_key_name(*entry.key));
		const float w = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, line).x;
		if (w > width)
			width = w;
		++visible;
	}

	if (visible == 0)
		return;

	const ImVec2 origin(settings::watermark::pos_x, settings::watermark::pos_y + font_size + 18.f);
	ImDrawList* draw = ImGui::GetForegroundDrawList();
	draw->AddRectFilled(ImVec2(origin.x - padding, origin.y - 4.f), ImVec2(origin.x + width + padding, origin.y + line_height * visible + 2.f), IM_COL32(0, 0, 0, 160), 3.f);

	float y = origin.y;
	for (const auto& entry : entries)
	{
		if (*entry.key == 0)
			continue;

		const bool held = (*entry.mode == 2) || ((GetAsyncKeyState(*entry.key) & 0x8000) != 0);
		const bool on = *entry.enabled;
		const ImU32 color = held ? IM_COL32(255, 90, 90, 255) : (on ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 160, 160, 255));

		char line[128];
		snprintf(line, sizeof(line), "%s  [%s]", entry.label, keybind::get_key_name(*entry.key));
		draw->AddText(font, font_size, ImVec2(origin.x, y), color, line);
		y += line_height;
	}
}

void render_t::render_visuals()
{
	const auto frame_start = std::chrono::steady_clock::now();
	auto stage_start = frame_start;

	custom_entities::update_containers();
	log_stall("custom_entities", ms_since(stage_start));

	stage_start = std::chrono::steady_clock::now();
	if (g_avatar_manager)
	{
		g_avatar_manager->update();
	}
	log_stall("avatar_manager", ms_since(stage_start));

	stage_start = std::chrono::steady_clock::now();
	radar::run();
	log_stall("radar", ms_since(stage_start));

	stage_start = std::chrono::steady_clock::now();
	esp::run();
	log_stall("esp", ms_since(stage_start));

	stage_start = std::chrono::steady_clock::now();
	rage::desync::render_visualizer();

	if (settings::aimbot::draw_fov && settings::aimbot::enabled)
	{
		draw_fov_circle(settings::aimbot::fov, settings::aimbot::fov_circle_colour, settings::aimbot::fov_outline_colour);
	}

	if (settings::silentaim::draw_fov && settings::silentaim::enabled)
	{
		draw_fov_circle(settings::silentaim::fov, settings::silentaim::fov_circle_colour, settings::silentaim::fov_outline_colour, settings::silentaim::attach_fov_to_target);
	}

	draw_custom_cursor();
	draw_silent_aim_target_dot();
	draw_silent_aim_snap_line();

	draw_target_warning_icon();

	if (settings::visuals::debug_wallcheck)
	{
		wallcheck->draw_debug();
	}

	static bool radar_wallcheck_cached = false;
	static auto last_cache_time = std::chrono::steady_clock::now();
	auto now = std::chrono::steady_clock::now();
	if (settings::visuals::radar_enabled)
	{
		bool should_cache = !radar_wallcheck_cached;
		if (!should_cache && std::chrono::duration_cast<std::chrono::seconds>(now - last_cache_time).count() > 5)
			should_cache = true;
		if (should_cache)
		{
			const auto cache_start = std::chrono::steady_clock::now();
			radar_wallcheck_cached = wallcheck->cache_workspace();
			last_cache_time = now;
			log_stall("wallcheck_cache", ms_since(cache_start));
		}
	}
	else
	{
		radar_wallcheck_cached = false;
	}

	stage_start = std::chrono::steady_clock::now();
	rage::draw_hit_tracers();
	log_stall("hit_tracers", ms_since(stage_start));

	stage_start = std::chrono::steady_clock::now();
	draw_watermark();
	draw_keybind_list();
	log_stall("overlays", ms_since(stage_start));

	log_stall("visuals_total", ms_since(frame_start));
}
