#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>
#include "fonts.hpp"
#include "lazyui/runtime.hpp"

namespace ui
{
    ImVec2 frame_size( );
    void render( );
    void draw_activation_menu( ImDrawList* draw, lazyui::keybind_state& state, ImVec2 box_min, ImVec2 box_max, float alpha );

    namespace controls
    {
        extern lazyui::switch_state silent_aim_enabled;
        extern lazyui::switch_state use_aimbot_target;
        extern lazyui::switch_state team_check;
        extern lazyui::switch_state gun_check;
        extern lazyui::switch_state knock_check;
        extern lazyui::switch_state health_check;
        extern lazyui::switch_state use_fov;
        extern lazyui::switch_state draw_fov;
        extern lazyui::switch_state draw_target_line;
        extern lazyui::switch_state enable_prediction;
        extern lazyui::selection_state< 11 > nav_main;
        extern lazyui::keybind_state aim_keybind;
        extern lazyui::slider_state fov_slider;
        extern lazyui::switch_state sticky_aim;
    }
}
