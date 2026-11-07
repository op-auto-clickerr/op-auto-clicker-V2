#include "pages.hpp"

#include "lazyui/lazyui.hpp"
#include "ui/ui.hpp"
#include "theme.hpp"
#include "ui/fonts.hpp"

#include <settings.h>

#include <cache/cache.h>
#include <features/keybind/keybind.h>
#include <features/config/config.h>
#include <features/explorer/explorer.h>

#include <imgui_internal.h>

#include <cstring>

namespace ui::pages
{
    namespace
    {
        using namespace lazyui;

        constexpr float column_x = 249.f;
        constexpr float column_width = 275.f;
        constexpr float column_gap = 10.f;
        constexpr float start_y = 20.f;
        constexpr float bottom = 726.f;
        constexpr int column_count = 3;
        constexpr int body_slot = text_fonts::inter_600_15_5;
        constexpr int heading_slot = text_fonts::roboto_700_16;

        enum kind : int
        {
            k_heading,
            k_toggle,
            k_slider,
            k_keybind,
            k_combo,
            k_color,
            k_label,
            k_button,
            k_field
        };

        struct item
        {
            kind type;
            const char* label = "";
            void* state = nullptr;
            void* bind = nullptr;
            void* bind_mode = nullptr;
            float min = 0.f;
            float max = 100.f;
            const char* const* options = nullptr;
            int option_count = 0;
            int offset = 0;
            int ( *action )( ) = nullptr;
            const char* format = "%.0f";
        };

        int action_config_save( );
        int action_config_load( );
        int action_config_delete( );
        int action_config_reset( );
        int action_config_folder( );
        int action_refresh_explorer( );

        field_state config_name_field{ };

        switch_state aim_enabled, aim_air_part, aim_use_fov, aim_draw_fov, aim_smoothing, aim_prediction,
            aim_team, aim_knock, aim_health, aim_offset;
        slider_state aim_fov, aim_smooth_x, aim_smooth_y, aim_pred_x, aim_pred_y, aim_min_health, aim_offset_x, aim_offset_y;
        keybind_state aim_key;
        combo_state aim_mode, aim_part, aim_air_part_combo, aim_smooth_style;

        switch_state trg_enabled, trg_max_distance, trg_wallcheck;
        slider_state trg_reaction, trg_hold, trg_cps, trg_distance;
        keybind_state trg_key;
        combo_state trg_activation, trg_fire_mode;

        switch_state vis_enemies, vis_local, vis_team, vis_knock, vis_team_color, vis_ignore_whitelisted,
            vis_box, vis_box_fill, vis_name, vis_avatar, vis_blend, vis_healthbar, vis_health_based,
            vis_gradient_health, vis_health_percent, vis_armorbar, vis_distance, vis_tool, vis_flags,
            vis_chams, vis_hitbox, vis_max_distance, vis_radar,
            vis_client_box, vis_client_box_fill, vis_client_name, vis_client_avatar, vis_client_health, vis_client_health_percent,
            vis_client_armor, vis_client_distance, vis_client_tool, vis_client_chams, vis_client_flags,
            vis_client_headless, vis_client_korblox;
        slider_state vis_max_distance_value, vis_fade_in, vis_fade_out, vis_radar_size;

        switch_state rage_hitsounds, rage_rapidfire, rage_noclip, rage_tracers, rage_hip,
            rage_expander, rage_expander_knock, rage_spin, rage_desync, rage_desync_vis,
            rage_magic, rage_no_spread, rage_wallbang, rage_inf_range;
        slider_state rage_tracer_duration, rage_hip_height, rage_exp_size_x, rage_exp_size_y, rage_exp_size_z,
            rage_spin_speed, rage_desync_thickness, rage_magic_offset, rage_no_spread_amount;
        combo_state rage_exp_part, rage_magic_source, rage_wallbang_method;
        keybind_state rage_spin_key, rage_desync_key, rage_magic_key;

        switch_state move_speed, move_jump, move_no_cooldown, move_fly, move_tick, move_orbit,
            move_orbit_spectate, move_orbit_random, move_gravity, move_antiafk, move_freeze;
        slider_state move_speed_value, move_jump_value, move_fly_value, move_tick_value, move_orbit_speed,
            move_orbit_radius, move_orbit_height, move_gravity_value;
        keybind_state move_speed_key, move_jump_key, move_fly_key, move_freeze_key;

        switch_state set_streamproof, set_vsync, set_performance, set_hide_console, set_watermark, set_keybinds,
            set_wm_name, set_wm_display, set_wm_username, set_wm_fps, set_wm_server, set_wm_rainbow,
            light_fog, light_shadows, light_clock, light_skybox, light_exposure,
            set_keybind_indicator;
        slider_state set_rainbow_speed, light_fog_start, light_fog_end, light_clock_value, light_exposure_value;
        combo_state set_separator;
        keybind_state set_menu_key;

        switch_state player_list_enabled;

        button_state config_save_button, config_load_button, config_delete_button, config_reset_button, config_folder_button;
        button_state dex_refresh_button;

        color_state color_box, color_box_fill, color_name, color_name_start, color_name_end, color_health,
            color_health_start, color_health_end, color_health_percent, color_armor, color_distance, color_tool,
            color_chams_fill, color_chams_outline, color_flags, color_hitbox, color_tracers,
            color_client_box, color_client_box_fill, color_client_name, color_client_health,
            color_client_health_percent, color_client_armor, color_client_distance, color_client_tool,
            color_client_chams_fill, color_client_chams_outline, color_client_flags,
            color_keybind_active, color_keybind_inactive;

        const char* const mode_options[] = { "Mouse", "Camera" };
        const char* const part_options[] = { "Head", "HumanoidRootPart", "Left Arm", "Right Arm", "Left Leg", "Right Leg" };
        const char* const activation_options[] = { "Toggle", "Hold", "Always" };
        const char* const fire_options[] = { "Click", "Hold" };
        const char* const smooth_options[] = { "Instant", "Linear", "In Quad", "Out Quad", "In Out Quad", "In Cubic", "Out Cubic", "In Out Cubic", "In Sine", "Out Sine", "In Out Sine" };
        const char* const magic_source_options[] = { "Silent Aim", "Aimbot", "Auto" };
        const char* const wallbang_method_options[] = { "Engine Hook", "Position Spoof", "Raycast" };
        const char* const separator_options[] = { " | ", " - ", " / ", " :: ", "  " };

        const item* items_for( int page, int& count )
        {
            static const item aimbot_items[] = {
                { .type = k_heading, .label = "Activation" },
                { .type = k_toggle, .label = "Enabled", .state = &aim_enabled, .bind = &settings::aimbot::enabled },
                { .type = k_keybind, .label = "Keybind", .state = &aim_key, .bind = &settings::aimbot::keybind, .bind_mode = &settings::aimbot::activation_mode },
                { .type = k_combo, .label = "Mode", .state = &aim_mode, .bind = &settings::aimbot::mode, .options = mode_options, .option_count = 2 },
                { .type = k_heading, .label = "Target" },
                { .type = k_combo, .label = "Target Part", .state = &aim_part, .bind = &settings::aimbot::target_part, .options = part_options, .option_count = 6, .offset = 1 },
                { .type = k_toggle, .label = "Air Part", .state = &aim_air_part, .bind = &settings::aimbot::air_part_enabled },
                { .type = k_combo, .label = "Air Target Part", .state = &aim_air_part_combo, .bind = &settings::aimbot::air_part, .options = part_options, .option_count = 6, .offset = 1 },
                { .type = k_heading, .label = "Field of View" },
                { .type = k_slider, .label = "FOV", .state = &aim_fov, .bind = &settings::aimbot::fov, .min = 10.f, .max = 500.f },
                { .type = k_toggle, .label = "Use FOV", .state = &aim_use_fov, .bind = &settings::aimbot::use_fov },
                { .type = k_toggle, .label = "Draw FOV", .state = &aim_draw_fov, .bind = &settings::aimbot::draw_fov },
                { .type = k_heading, .label = "Smoothing" },
                { .type = k_toggle, .label = "Smoothing", .state = &aim_smoothing, .bind = &settings::aimbot::smoothing },
                { .type = k_slider, .label = "Speed X", .state = &aim_smooth_x, .bind = &settings::aimbot::smoothingx, .min = 1.f, .max = 100.f, .format = "%.1f" },
                { .type = k_slider, .label = "Speed Y", .state = &aim_smooth_y, .bind = &settings::aimbot::smoothingy, .min = 1.f, .max = 100.f, .format = "%.1f" },
                { .type = k_combo, .label = "Style", .state = &aim_smooth_style, .bind = &settings::aimbot::smoothing_style, .options = smooth_options, .option_count = 11 },
                { .type = k_heading, .label = "Prediction" },
                { .type = k_toggle, .label = "Prediction", .state = &aim_prediction, .bind = &settings::aimbot::enable_prediction },
                { .type = k_slider, .label = "Prediction X", .state = &aim_pred_x, .bind = &settings::aimbot::prediction_x, .min = 0.f, .max = 50.f, .format = "%.1f" },
                { .type = k_slider, .label = "Prediction Y", .state = &aim_pred_y, .bind = &settings::aimbot::prediction_y, .min = 0.f, .max = 50.f, .format = "%.1f" },
                { .type = k_heading, .label = "Checks" },
                { .type = k_toggle, .label = "Team Check", .state = &aim_team, .bind = &settings::aimbot::teamcheck },
                { .type = k_toggle, .label = "Knock Check", .state = &aim_knock, .bind = &settings::aimbot::knock_check },
                { .type = k_toggle, .label = "Health Check", .state = &aim_health, .bind = &settings::aimbot::health_check_enabled },
                { .type = k_slider, .label = "Min Health", .state = &aim_min_health, .bind = &settings::aimbot::min_health, .min = 0.f, .max = 100.f },
                { .type = k_heading, .label = "Offset" },
                { .type = k_toggle, .label = "Offset", .state = &aim_offset, .bind = &settings::aimbot::offset_enabled },
                { .type = k_slider, .label = "Offset X", .state = &aim_offset_x, .bind = &settings::aimbot::offset_x, .min = -100.f, .max = 100.f },
                { .type = k_slider, .label = "Offset Y", .state = &aim_offset_y, .bind = &settings::aimbot::offset_y, .min = -100.f, .max = 100.f },
            };

            static const item trigger_items[] = {
                { .type = k_heading, .label = "Triggerbot" },
                { .type = k_toggle, .label = "Enabled", .state = &trg_enabled, .bind = &settings::aimbot::triggerbot::enabled },
                { .type = k_keybind, .label = "Keybind", .state = &trg_key, .bind = &settings::aimbot::triggerbot::keybind, .bind_mode = &settings::aimbot::triggerbot::activation_mode },
                { .type = k_combo, .label = "Activation", .state = &trg_activation, .bind = &settings::aimbot::triggerbot::activation_mode, .options = activation_options, .option_count = 3 },
                { .type = k_combo, .label = "Fire Mode", .state = &trg_fire_mode, .bind = &settings::aimbot::triggerbot::fire_mode, .options = fire_options, .option_count = 2 },
                { .type = k_heading, .label = "Timing" },
                { .type = k_slider, .label = "Reaction (ms)", .state = &trg_reaction, .bind = &settings::aimbot::triggerbot::reaction_ms, .min = 0.f, .max = 500.f },
                { .type = k_slider, .label = "Hold Duration", .state = &trg_hold, .bind = &settings::aimbot::triggerbot::hold_duration, .min = 0.f, .max = 1.f, .format = "%.2f" },
                { .type = k_slider, .label = "Clicks / Sec", .state = &trg_cps, .bind = &settings::aimbot::triggerbot::clicks_per_second, .min = 1.f, .max = 30.f, .format = "%.1f" },
                { .type = k_heading, .label = "Limits" },
                { .type = k_toggle, .label = "Max Distance", .state = &trg_max_distance, .bind = &settings::aimbot::triggerbot::max_distance_enabled },
                { .type = k_slider, .label = "Max Distance", .state = &trg_distance, .bind = &settings::aimbot::triggerbot::max_distance, .min = 0.f, .max = 1000.f },
                { .type = k_toggle, .label = "Wallcheck", .state = &trg_wallcheck, .bind = &settings::aimbot::triggerbot::wallcheck },
            };

            static const item visuals_items[] = {
                { .type = k_heading, .label = "Players" },
                { .type = k_toggle, .label = "Enemies", .state = &vis_enemies, .bind = &settings::visuals::enable_enemies },
                { .type = k_toggle, .label = "Local Player", .state = &vis_local, .bind = &settings::visuals::local_player },
                { .type = k_toggle, .label = "Team Check", .state = &vis_team, .bind = &settings::visuals::teamcheck },
                { .type = k_toggle, .label = "Knock Check", .state = &vis_knock, .bind = &settings::visuals::knock_check },
                { .type = k_toggle, .label = "Use Team Color", .state = &vis_team_color, .bind = &settings::visuals::use_team_color },
                { .type = k_toggle, .label = "Ignore Whitelisted", .state = &vis_ignore_whitelisted, .bind = &settings::visuals::ignore_whitelisted },
                { .type = k_heading, .label = "Box" },
                { .type = k_toggle, .label = "Box", .state = &vis_box, .bind = &settings::visuals::box },
                { .type = k_toggle, .label = "Box Fill", .state = &vis_box_fill, .bind = &settings::visuals::box_fill },
                { .type = k_heading, .label = "Name" },
                { .type = k_toggle, .label = "Name", .state = &vis_name, .bind = &settings::visuals::name },
                { .type = k_toggle, .label = "Avatar", .state = &vis_avatar, .bind = &settings::visuals::avatar },
                { .type = k_toggle, .label = "Blend", .state = &vis_blend, .bind = &settings::visuals::blend },
                { .type = k_heading, .label = "Bars" },
                { .type = k_toggle, .label = "Health Bar", .state = &vis_healthbar, .bind = &settings::visuals::healthbar },
                { .type = k_toggle, .label = "Health Based", .state = &vis_health_based, .bind = &settings::visuals::health_based_healthbar },
                { .type = k_toggle, .label = "Gradient Health", .state = &vis_gradient_health, .bind = &settings::visuals::gradient_healthbar },
                { .type = k_toggle, .label = "Health Percent", .state = &vis_health_percent, .bind = &settings::visuals::health_percent },
                { .type = k_toggle, .label = "Armor Bar", .state = &vis_armorbar, .bind = &settings::visuals::armorbar },
                { .type = k_heading, .label = "Info" },
                { .type = k_toggle, .label = "Distance", .state = &vis_distance, .bind = &settings::visuals::distance },
                { .type = k_toggle, .label = "Tool", .state = &vis_tool, .bind = &settings::visuals::tool },
                { .type = k_toggle, .label = "Flags", .state = &vis_flags, .bind = &settings::visuals::flags },
                { .type = k_toggle, .label = "Chams", .state = &vis_chams, .bind = &settings::visuals::chams },
                { .type = k_toggle, .label = "View Hitbox", .state = &vis_hitbox, .bind = &settings::visuals::view_hitbox },
                { .type = k_heading, .label = "Limits" },
                { .type = k_toggle, .label = "Max Distance", .state = &vis_max_distance, .bind = &settings::visuals::max_distance_enabled },
                { .type = k_slider, .label = "Max Distance", .state = &vis_max_distance_value, .bind = &settings::visuals::max_distance, .min = 0.f, .max = 2000.f },
                { .type = k_slider, .label = "Fade In", .state = &vis_fade_in, .bind = &settings::visuals::fade_in_speed, .min = 0.f, .max = 20.f, .format = "%.1f" },
                { .type = k_slider, .label = "Fade Out", .state = &vis_fade_out, .bind = &settings::visuals::fade_out_speed, .min = 0.f, .max = 20.f, .format = "%.1f" },
                { .type = k_heading, .label = "Radar" },
                { .type = k_toggle, .label = "Radar", .state = &vis_radar, .bind = &settings::visuals::radar_enabled },
                { .type = k_slider, .label = "Radar Size", .state = &vis_radar_size, .bind = &settings::visuals::radar_size, .min = 0.f, .max = 500.f },
                { .type = k_heading, .label = "Local Player" },
                { .type = k_toggle, .label = "Box", .state = &vis_client_box, .bind = &settings::visuals::client_box },
                { .type = k_toggle, .label = "Box Fill", .state = &vis_client_box_fill, .bind = &settings::visuals::client_box_fill },
                { .type = k_toggle, .label = "Name", .state = &vis_client_name, .bind = &settings::visuals::client_name },
                { .type = k_toggle, .label = "Avatar", .state = &vis_client_avatar, .bind = &settings::visuals::client_avatar },
                { .type = k_toggle, .label = "Health Bar", .state = &vis_client_health, .bind = &settings::visuals::client_healthbar },
                { .type = k_toggle, .label = "Health Percent", .state = &vis_client_health_percent, .bind = &settings::visuals::client_health_percent },
                { .type = k_toggle, .label = "Armor Bar", .state = &vis_client_armor, .bind = &settings::visuals::client_armorbar },
                { .type = k_toggle, .label = "Distance", .state = &vis_client_distance, .bind = &settings::visuals::client_distance },
                { .type = k_toggle, .label = "Tool", .state = &vis_client_tool, .bind = &settings::visuals::client_tool },
                { .type = k_toggle, .label = "Chams", .state = &vis_client_chams, .bind = &settings::visuals::client_chams },
                { .type = k_toggle, .label = "Flags", .state = &vis_client_flags, .bind = &settings::visuals::client_flags },
                { .type = k_toggle, .label = "Headless", .state = &vis_client_headless, .bind = &settings::visuals::client_headless },
                { .type = k_toggle, .label = "Korblox", .state = &vis_client_korblox, .bind = &settings::visuals::client_korblox },
            };

            static const item rage_items[] = {
                { .type = k_heading, .label = "Combat" },
                { .type = k_toggle, .label = "Hitsounds", .state = &rage_hitsounds, .bind = &settings::rage::hitsounds },
                { .type = k_toggle, .label = "Rapid Fire", .state = &rage_rapidfire, .bind = &settings::rage::rapidfire },
                { .type = k_toggle, .label = "Inf Range", .state = &rage_inf_range, .bind = &settings::rage::infrange::enabled },
                { .type = k_toggle, .label = "No Clip", .state = &rage_noclip, .bind = &settings::rage::noclip },
                { .type = k_toggle, .label = "Hit Tracers", .state = &rage_tracers, .bind = &settings::rage::hit_tracers },
                { .type = k_slider, .label = "Tracer Duration", .state = &rage_tracer_duration, .bind = &settings::rage::hit_tracers_duration, .min = 0.f, .max = 5.f, .format = "%.2f" },
                { .type = k_heading, .label = "Hip Height" },
                { .type = k_toggle, .label = "Enabled", .state = &rage_hip, .bind = &settings::rage::hipheight::enabled },
                { .type = k_slider, .label = "Height", .state = &rage_hip_height, .bind = &settings::rage::hipheight::height, .min = 0.f, .max = 10.f, .format = "%.1f" },
                { .type = k_heading, .label = "Hitbox Expander" },
                { .type = k_toggle, .label = "Enabled", .state = &rage_expander, .bind = &settings::rage::hitbox_expander::enabled },
                { .type = k_combo, .label = "Target Part", .state = &rage_exp_part, .bind = &settings::rage::hitbox_expander::target_part, .options = part_options, .option_count = 6, .offset = 1 },
                { .type = k_slider, .label = "Size X", .state = &rage_exp_size_x, .bind = &settings::rage::hitbox_expander::size_x, .min = 0.f, .max = 10.f, .format = "%.1f" },
                { .type = k_slider, .label = "Size Y", .state = &rage_exp_size_y, .bind = &settings::rage::hitbox_expander::size_y, .min = 0.f, .max = 10.f, .format = "%.1f" },
                { .type = k_slider, .label = "Size Z", .state = &rage_exp_size_z, .bind = &settings::rage::hitbox_expander::size_z, .min = 0.f, .max = 10.f, .format = "%.1f" },
                { .type = k_toggle, .label = "Knock Check", .state = &rage_expander_knock, .bind = &settings::rage::hitbox_expander::knock_check },
                { .type = k_heading, .label = "Spin 360" },
                { .type = k_toggle, .label = "Enabled", .state = &rage_spin, .bind = &settings::rage::spin360::enabled },
                { .type = k_slider, .label = "Speed", .state = &rage_spin_speed, .bind = &settings::rage::spin360::speed, .min = 0.f, .max = 30.f, .format = "%.1f" },
                { .type = k_keybind, .label = "Keybind", .state = &rage_spin_key, .bind = &settings::rage::spin360::keybind, .bind_mode = &settings::rage::spin360::activation_mode },
                { .type = k_heading, .label = "Desync" },
                { .type = k_toggle, .label = "Enabled", .state = &rage_desync, .bind = &settings::desync::enabled },
                { .type = k_keybind, .label = "Keybind", .state = &rage_desync_key, .bind = &settings::desync::keybind, .bind_mode = &settings::desync::keybind_mode },
                { .type = k_toggle, .label = "Visualizer", .state = &rage_desync_vis, .bind = &settings::desync::visualizer::enabled },
                { .type = k_slider, .label = "Thickness", .state = &rage_desync_thickness, .bind = &settings::desync::visualizer::thickness, .min = 0.f, .max = 10.f, .format = "%.1f" },
                { .type = k_heading, .label = "Magic Bullet" },
                { .type = k_toggle, .label = "Enabled", .state = &rage_magic, .bind = &settings::magicbullet::enabled },
                { .type = k_keybind, .label = "Keybind", .state = &rage_magic_key, .bind = &settings::magicbullet::keybind, .bind_mode = &settings::magicbullet::activation_mode },
                { .type = k_combo, .label = "Target Source", .state = &rage_magic_source, .bind = &settings::magicbullet::target_source, .options = magic_source_options, .option_count = 3 },
                { .type = k_slider, .label = "Offset Distance", .state = &rage_magic_offset, .bind = &settings::magicbullet::offset_distance, .min = 0.f, .max = 20.f, .format = "%.1f" },
                { .type = k_heading, .label = "No Spread" },
                { .type = k_toggle, .label = "Enabled", .state = &rage_no_spread, .bind = &settings::exploits::no_spread::enabled },
                { .type = k_slider, .label = "Amount", .state = &rage_no_spread_amount, .bind = &settings::exploits::no_spread::amount, .min = 0.f, .max = 100.f },
                { .type = k_heading, .label = "Wallbang" },
                { .type = k_toggle, .label = "Enabled", .state = &rage_wallbang, .bind = &settings::silentaim::wallbang },
                { .type = k_combo, .label = "Method", .state = &rage_wallbang_method, .bind = &settings::silentaim::wallbang_method, .options = wallbang_method_options, .option_count = 3 },
            };

            static const item player_items[] = {
                { .type = k_heading, .label = "Player List" },
                { .type = k_toggle, .label = "Enabled", .state = &player_list_enabled, .bind = &settings::rage::playerlist::enabled },
                { .type = k_label, .label = "Whitelist and spectate are managed" },
                { .type = k_label, .label = "through the player list itself." },
            };

            static const item local_items[] = {
                { .type = k_heading, .label = "Speed" },
                { .type = k_toggle, .label = "Enabled", .state = &move_speed, .bind = &settings::movement::speedhack::enabled },
                { .type = k_slider, .label = "Speed", .state = &move_speed_value, .bind = &settings::movement::speedhack::speed, .min = 0.f, .max = 500.f },
                { .type = k_keybind, .label = "Keybind", .state = &move_speed_key, .bind = &settings::movement::speedhack::keybind, .bind_mode = &settings::movement::speedhack::activation_mode },
                { .type = k_heading, .label = "Jump" },
                { .type = k_toggle, .label = "Enabled", .state = &move_jump, .bind = &settings::movement::jumphack::enabled },
                { .type = k_slider, .label = "Power", .state = &move_jump_value, .bind = &settings::movement::jumphack::value, .min = 0.f, .max = 500.f },
                { .type = k_keybind, .label = "Keybind", .state = &move_jump_key, .bind = &settings::movement::jumphack::keybind, .bind_mode = &settings::movement::jumphack::activation_mode },
                { .type = k_toggle, .label = "No Jump Cooldown", .state = &move_no_cooldown, .bind = &settings::movement::nojumpcooldown::enabled },
                { .type = k_heading, .label = "Fly" },
                { .type = k_toggle, .label = "Enabled", .state = &move_fly, .bind = &settings::movement::flyhack::enabled },
                { .type = k_slider, .label = "Speed", .state = &move_fly_value, .bind = &settings::movement::flyhack::speed, .min = 0.f, .max = 500.f },
                { .type = k_keybind, .label = "Keybind", .state = &move_fly_key, .bind = &settings::movement::flyhack::keybind, .bind_mode = &settings::movement::flyhack::activation_mode },
                { .type = k_heading, .label = "Tick Rate" },
                { .type = k_toggle, .label = "Enabled", .state = &move_tick, .bind = &settings::movement::tickrate::enabled },
                { .type = k_slider, .label = "Rate", .state = &move_tick_value, .bind = &settings::movement::tickrate::value, .min = 60.f, .max = 1000.f },
                { .type = k_heading, .label = "Orbit" },
                { .type = k_toggle, .label = "Enabled", .state = &move_orbit, .bind = &settings::movement::orbit::enabled },
                { .type = k_slider, .label = "Speed", .state = &move_orbit_speed, .bind = &settings::movement::orbit::speed, .min = 0.f, .max = 300.f },
                { .type = k_slider, .label = "Radius", .state = &move_orbit_radius, .bind = &settings::movement::orbit::radius, .min = 0.f, .max = 100.f },
                { .type = k_slider, .label = "Height", .state = &move_orbit_height, .bind = &settings::movement::orbit::height_offset, .min = -100.f, .max = 100.f },
                { .type = k_toggle, .label = "Spectate Target", .state = &move_orbit_spectate, .bind = &settings::movement::orbit::spectate_target },
                { .type = k_toggle, .label = "Randomize", .state = &move_orbit_random, .bind = &settings::movement::orbit::randomize },
                { .type = k_heading, .label = "Gravity" },
                { .type = k_toggle, .label = "Enabled", .state = &move_gravity, .bind = &settings::movement::gravity::enabled },
                { .type = k_slider, .label = "Value", .state = &move_gravity_value, .bind = &settings::movement::gravity::value, .min = 0.f, .max = 500.f },
                { .type = k_heading, .label = "Misc" },
                { .type = k_toggle, .label = "Anti AFK", .state = &move_antiafk, .bind = &settings::exploits::antiafk::enabled },
                { .type = k_toggle, .label = "Freeze Player", .state = &move_freeze, .bind = &settings::exploits::freezeplayer::enabled },
                { .type = k_keybind, .label = "Freeze Keybind", .state = &move_freeze_key, .bind = &settings::exploits::freezeplayer::keybind, .bind_mode = &settings::exploits::freezeplayer::activation_mode },
            };

            static const item settings_items[] = {
                { .type = k_heading, .label = "Menu" },
                { .type = k_keybind, .label = "Menu Keybind", .state = &set_menu_key, .bind = &settings::menu::menu_keybind },
                { .type = k_toggle, .label = "Stream Proof", .state = &set_streamproof, .bind = &settings::menu::streamproof },
                { .type = k_toggle, .label = "VSync", .state = &set_vsync, .bind = &settings::menu::vsync },
                { .type = k_toggle, .label = "Performance Mode", .state = &set_performance, .bind = &settings::menu::performance_mode },
                { .type = k_toggle, .label = "Hide Console", .state = &set_hide_console, .bind = &settings::menu::hide_console },
                { .type = k_heading, .label = "Interface" },
                { .type = k_toggle, .label = "Watermark", .state = &set_watermark, .bind = &settings::ui::watermark },
                { .type = k_toggle, .label = "Keybind List", .state = &set_keybinds, .bind = &settings::ui::keybinds },
                { .type = k_toggle, .label = "Keybind Indicator", .state = &set_keybind_indicator, .bind = &settings::menu::keybind_indicator },
                { .type = k_heading, .label = "Watermark" },
                { .type = k_toggle, .label = "Cheat Name", .state = &set_wm_name, .bind = &settings::watermark::show_cheat_name },
                { .type = k_toggle, .label = "Display Name", .state = &set_wm_display, .bind = &settings::watermark::show_display_name },
                { .type = k_toggle, .label = "Username", .state = &set_wm_username, .bind = &settings::watermark::show_username },
                { .type = k_toggle, .label = "FPS", .state = &set_wm_fps, .bind = &settings::watermark::show_fps },
                { .type = k_toggle, .label = "Server IP", .state = &set_wm_server, .bind = &settings::watermark::show_server_ip },
                { .type = k_combo, .label = "Separator", .state = &set_separator, .bind = &settings::watermark::separator_type, .options = separator_options, .option_count = 5 },
                { .type = k_toggle, .label = "Rainbow", .state = &set_wm_rainbow, .bind = &settings::watermark::rainbow },
                { .type = k_slider, .label = "Rainbow Speed", .state = &set_rainbow_speed, .bind = &settings::watermark::rainbow_speed, .min = 0.f, .max = 10.f, .format = "%.1f" },
                { .type = k_heading, .label = "Lighting" },
                { .type = k_toggle, .label = "Fog", .state = &light_fog, .bind = &settings::lighting::fog::enabled },
                { .type = k_slider, .label = "Fog Start", .state = &light_fog_start, .bind = &settings::lighting::fog::fog_start, .min = 0.f, .max = 2000.f },
                { .type = k_slider, .label = "Fog End", .state = &light_fog_end, .bind = &settings::lighting::fog::fog_end, .min = 0.f, .max = 5000.f },
                { .type = k_toggle, .label = "Disable Shadows", .state = &light_shadows, .bind = &settings::lighting::shadows::disable },
                { .type = k_toggle, .label = "Clock Time", .state = &light_clock, .bind = &settings::lighting::clocktime::enabled },
                { .type = k_slider, .label = "Time", .state = &light_clock_value, .bind = &settings::lighting::clocktime::clock_time, .min = 0.f, .max = 24.f, .format = "%.1f" },
                { .type = k_toggle, .label = "Skybox", .state = &light_skybox, .bind = &settings::lighting::skybox::enabled },
                { .type = k_toggle, .label = "Exposure", .state = &light_exposure, .bind = &settings::lighting::exposure::enabled },
                { .type = k_slider, .label = "Exposure", .state = &light_exposure_value, .bind = &settings::lighting::exposure::exposure, .min = -5.f, .max = 5.f, .format = "%.2f" },
            };

            static const item config_items[] = {
                { .type = k_heading, .label = "Config" },
                { .type = k_field, .label = "Name", .state = &config_name_field },
                { .type = k_button, .label = "Save Config", .state = &config_save_button, .action = &action_config_save },
                { .type = k_button, .label = "Load Config", .state = &config_load_button, .action = &action_config_load },
                { .type = k_button, .label = "Delete Config", .state = &config_delete_button, .action = &action_config_delete },
                { .type = k_heading, .label = "Actions" },
                { .type = k_button, .label = "Reset To Defaults", .state = &config_reset_button, .action = &action_config_reset },
                { .type = k_button, .label = "Open Folder", .state = &config_folder_button, .action = &action_config_folder },
            };

            static const item dex_items[] = {
                { .type = k_heading, .label = "Explorer" },
                { .type = k_button, .label = "Refresh Tree", .state = &dex_refresh_button, .action = &action_refresh_explorer },
                { .type = k_label, .label = "Re-caches the DataModel tree." },
                { .type = k_label, .label = "Use the console for path queries." },
            };

            static const item color_items[] = {
                { .type = k_heading, .label = "Box" },
                { .type = k_color, .label = "Box", .state = &color_box, .bind = &settings::visuals::box_color },
                { .type = k_color, .label = "Box Fill", .state = &color_box_fill, .bind = &settings::visuals::box_fill_color },
                { .type = k_heading, .label = "Name" },
                { .type = k_color, .label = "Name", .state = &color_name, .bind = &settings::visuals::name_color },
                { .type = k_color, .label = "Blend Start", .state = &color_name_start, .bind = &settings::visuals::name_color_blend_start },
                { .type = k_color, .label = "Blend End", .state = &color_name_end, .bind = &settings::visuals::name_color_blend_end },
                { .type = k_heading, .label = "Bars" },
                { .type = k_color, .label = "Health Bar", .state = &color_health, .bind = &settings::visuals::healthbar_color },
                { .type = k_color, .label = "Gradient Start", .state = &color_health_start, .bind = &settings::visuals::gradient_healthbar_color_start },
                { .type = k_color, .label = "Gradient End", .state = &color_health_end, .bind = &settings::visuals::gradient_healthbar_color_end },
                { .type = k_color, .label = "Health Percent", .state = &color_health_percent, .bind = &settings::visuals::health_percent_color },
                { .type = k_color, .label = "Armor Bar", .state = &color_armor, .bind = &settings::visuals::armorbar_color },
                { .type = k_heading, .label = "Info" },
                { .type = k_color, .label = "Distance", .state = &color_distance, .bind = &settings::visuals::distance_color },
                { .type = k_color, .label = "Tool", .state = &color_tool, .bind = &settings::visuals::tool_color },
                { .type = k_color, .label = "Flags", .state = &color_flags, .bind = &settings::visuals::flags_state_colour },
                { .type = k_color, .label = "View Hitbox", .state = &color_hitbox, .bind = &settings::visuals::view_hitbox_color },
                { .type = k_color, .label = "Hit Tracers", .state = &color_tracers, .bind = &settings::rage::hit_tracers_color },
                { .type = k_heading, .label = "Chams" },
                { .type = k_color, .label = "Chams Fill", .state = &color_chams_fill, .bind = &settings::visuals::chams_fill_color },
                { .type = k_color, .label = "Chams Outline", .state = &color_chams_outline, .bind = &settings::visuals::chams_outline_color },
                { .type = k_heading, .label = "Local Player" },
                { .type = k_color, .label = "Box", .state = &color_client_box, .bind = &settings::visuals::client_box_color },
                { .type = k_color, .label = "Box Fill", .state = &color_client_box_fill, .bind = &settings::visuals::client_box_fill_color },
                { .type = k_color, .label = "Name", .state = &color_client_name, .bind = &settings::visuals::client_name_color },
                { .type = k_color, .label = "Health Bar", .state = &color_client_health, .bind = &settings::visuals::client_healthbar_color },
                { .type = k_color, .label = "Health Percent", .state = &color_client_health_percent, .bind = &settings::visuals::client_health_percent_color },
                { .type = k_color, .label = "Armor Bar", .state = &color_client_armor, .bind = &settings::visuals::client_armorbar_color },
                { .type = k_color, .label = "Distance", .state = &color_client_distance, .bind = &settings::visuals::client_distance_color },
                { .type = k_color, .label = "Tool", .state = &color_client_tool, .bind = &settings::visuals::client_tool_color },
                { .type = k_color, .label = "Chams Fill", .state = &color_client_chams_fill, .bind = &settings::visuals::client_chams_fill_color },
                { .type = k_color, .label = "Chams Outline", .state = &color_client_chams_outline, .bind = &settings::visuals::client_chams_outline_color },
                { .type = k_color, .label = "Flags", .state = &color_client_flags, .bind = &settings::visuals::client_flags_state_colour },
                { .type = k_heading, .label = "Keybind Indicator" },
                { .type = k_color, .label = "Keybind Active", .state = &color_keybind_active, .bind = &settings::menu::keybind_indicator_active_color },
                { .type = k_color, .label = "Keybind Inactive", .state = &color_keybind_inactive, .bind = &settings::menu::keybind_indicator_inactive_color },
            };

            switch ( page )
            {
            case 0: count = static_cast< int >( sizeof( aimbot_items ) / sizeof( aimbot_items[0] ) ); return aimbot_items;
            case 1: count = static_cast< int >( sizeof( trigger_items ) / sizeof( trigger_items[0] ) ); return trigger_items;
            case 3: count = static_cast< int >( sizeof( visuals_items ) / sizeof( visuals_items[0] ) ); return visuals_items;
            case 4: count = static_cast< int >( sizeof( rage_items ) / sizeof( rage_items[0] ) ); return rage_items;
            case 5: count = static_cast< int >( sizeof( player_items ) / sizeof( player_items[0] ) ); return player_items;
            case 6: count = static_cast< int >( sizeof( local_items ) / sizeof( local_items[0] ) ); return local_items;
            case 7: count = static_cast< int >( sizeof( settings_items ) / sizeof( settings_items[0] ) ); return settings_items;
            case 8: count = static_cast< int >( sizeof( config_items ) / sizeof( config_items[0] ) ); return config_items;
            case 9: count = static_cast< int >( sizeof( dex_items ) / sizeof( dex_items[0] ) ); return dex_items;
            case 10: count = static_cast< int >( sizeof( color_items ) / sizeof( color_items[0] ) ); return color_items;
            default: count = 0; return nullptr;
            }
        }

        int action_config_save( )
        {
            const char* name = config_name_field.buffer.data( );
            config::save_config( name[0] != '\0' ? name : "default" );
            return 1;
        }

        int action_config_load( )
        {
            const char* name = config_name_field.buffer.data( );
            config::load_config( name[0] != '\0' ? name : "default" );
            return 1;
        }

        int action_config_delete( )
        {
            const char* name = config_name_field.buffer.data( );
            if ( name[0] != '\0' )
                config::delete_config( name );
            return 1;
        }

        int action_config_reset( )
        {
            config::reset_to_defaults( );
            return 1;
        }

        int action_config_folder( )
        {
            config::open_file_location( );
            return 1;
        }

        int action_refresh_explorer( )
        {
            explorer::run( );
            return 1;
        }

        void init_item( const item& it )
        {
            switch ( it.type )
            {
            case k_toggle:
            {
                switch_state& state = *static_cast< switch_state* >( it.state );
                state.value = *static_cast< bool* >( it.bind );
                state.anim = state.value ? 1.f : 0.f;
                break;
            }
            case k_slider:
            {
                slider_state& state = *static_cast< slider_state* >( it.state );
                const float value = *static_cast< float* >( it.bind );
                state.value = ImSaturate( ( value - it.min ) / ImMax( 1e-5f, it.max - it.min ) );
                state.shown = state.value;
                state.touched = true;
                break;
            }
            case k_keybind:
            {
                keybind_state& state = *static_cast< keybind_state* >( it.state );
                state.key = keybind::vk_to_imgui_key( *static_cast< int* >( it.bind ) );
                state.has_mode = it.bind_mode != nullptr;
                if ( it.bind_mode )
                    state.context_mode = *static_cast< int* >( it.bind_mode );
                break;
            }
            case k_combo:
            {
                combo_state& state = *static_cast< combo_state* >( it.state );
                state.selected = ImClamp( *static_cast< int* >( it.bind ) - it.offset, 0, it.option_count - 1 );
                break;
            }
            case k_color:
            {
                color_state& state = *static_cast< color_state* >( it.state );
                const float* value = static_cast< float* >( it.bind );
                state.value = ImVec4{ value[0], value[1], value[2], value[3] };
                break;
            }
            default:
                break;
            }
        }

        void apply_item( const item& it )
        {
            switch ( it.type )
            {
            case k_toggle:
                *static_cast< bool* >( it.bind ) = static_cast< switch_state* >( it.state )->value;
                break;
            case k_slider:
            {
                const slider_state& state = *static_cast< slider_state* >( it.state );
                *static_cast< float* >( it.bind ) = it.min + state.value * ( it.max - it.min );
                break;
            }
            case k_keybind:
            {
                const keybind_state& state = *static_cast< keybind_state* >( it.state );
                *static_cast< int* >( it.bind ) = keybind::imgui_key_to_vk( static_cast< ImGuiKey >( state.key ) );
                if ( it.bind_mode )
                    *static_cast< int* >( it.bind_mode ) = state.context_mode;
                break;
            }
            case k_combo:
                *static_cast< int* >( it.bind ) = static_cast< combo_state* >( it.state )->selected + it.offset;
                break;
            case k_color:
            {
                const ImVec4& value = static_cast< color_state* >( it.state )->value;
                float* target = static_cast< float* >( it.bind );
                target[0] = value.x;
                target[1] = value.y;
                target[2] = value.z;
                target[3] = value.w;
                break;
            }
            default:
                break;
            }
        }

        struct popup_ref
        {
            bool color = false;
            void* state = nullptr;
            const char* const* options = nullptr;
            int option_count = 0;
            ImVec2 min{ };
            ImVec2 max{ };
        };

        popup_ref popups[96];
        int popup_count = 0;

        template < typename Fn >
        void walk( const item* items, int count, ImVec2 origin, Fn&& fn )
        {
            int column = 0;
            float y = start_y;
            for ( int index = 0; index < count; ++index )
            {
                const item& it = items[index];
                
                // Skip wallbang method dropdown if raycast is not enabled
                if (it.bind == &settings::silentaim::wallbang_method && !settings::silentaim::raycast)
                {
                    continue;
                }
                
                const float height = it.type == k_heading ? 30.f : 26.f;
                if ( y + height > bottom )
                {
                    ++column;
                    y = start_y;
                    if ( column >= column_count )
                        return;
                }
                const float x = column_x + static_cast< float >( column ) * ( column_width + column_gap );
                fn( it, ImVec2{ origin.x + x, origin.y + y }, ImVec2{ origin.x + x + column_width, origin.y + y + height } );
                y += height;
            }
        }

        ImVec2 combo_box( ImVec2 min, ImVec2 max )
        {
            return ImVec2{ max.x - 118.f, min.y + 3.f };
        }

        ImVec2 keybind_box( ImVec2 min, ImVec2 max )
        {
            return ImVec2{ max.x - 96.f, min.y + 3.f };
        }

        ImVec2 swatch_box( ImVec2 min, ImVec2 max )
        {
            return ImVec2{ max.x - 62.f, min.y + 4.f };
        }

        ImVec2 button_box( ImVec2 min, ImVec2 max )
        {
            return ImVec2{ max.x - 150.f, min.y + 3.f };
        }

        void update_item( const item& it, ImVec2 min, ImVec2 max )
        {
            switch ( it.type )
            {
            case k_toggle:
                update_switch( *static_cast< switch_state* >( it.state ), ImVec2{ min.x, min.y + 2.f }, ImVec2{ max.x, min.y + 23.f } );
                break;
            case k_slider:
            {
                slider_state& state = *static_cast< slider_state* >( it.state );
                const float y = min.y + 19.f;
                update_slider( state, ImVec2{ min.x + 96.f, y }, ImVec2{ max.x, y }, ImVec2{ min.x, min.y + 11.f }, ImVec2{ max.x, min.y + 25.f } );
                break;
            }
            case k_keybind:
                update_keybind( *static_cast< keybind_state* >( it.state ), keybind_box( min, max ), ImVec2{ max.x, min.y + 22.f } );
                break;
            case k_combo:
            {
                combo_state& state = *static_cast< combo_state* >( it.state );
                const ImVec2 box_min = combo_box( min, max );
                update_combo( state, box_min, ImVec2{ max.x, box_min.y + 18.f }, it.option_count );
                break;
            }
            case k_color:
            {
                color_state& state = *static_cast< color_state* >( it.state );
                const ImVec2 swatch_min = swatch_box( min, max );
                update_color( state, swatch_min, ImVec2{ max.x, swatch_min.y + 16.f } );
                break;
            }
            case k_button:
            {
                button_state& state = *static_cast< button_state* >( it.state );
                const ImVec2 box_min = button_box( min, max );
                if ( update_button( state, box_min, ImVec2{ max.x, box_min.y + 18.f } ) && it.action )
                    it.action( );
                break;
            }
            case k_field:
            case k_label:
            case k_heading:
            default:
                break;
            }
        }

        void draw_toggle( ImDrawList* draw, ImVec2 min, const switch_state& state, float alpha )
        {
            const ImVec2 box_min{ min.x + 2.f, min.y + 4.f };
            const ImVec2 box_max{ min.x + 20.f, min.y + 22.f };
            add_rounded_rect_filled( draw, box_min, box_max, corner_radii{ 2.f }, fade( mix_color( mix_color( theme::surface, theme::accent, state.anim ), theme::fill_red_2, state.hover * ( 1.f - state.anim ) ), alpha ) );
            add_rounded_rect_stroke( draw, box_min + ImVec2{ 0.5f, 0.5f }, box_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 1.5f }, fade( mix_color( theme::border, theme::accent, state.anim ), alpha ), 1.f );
        }

        void draw_slider( ImDrawList* draw, ImVec2 min, ImVec2 max, const item& it, const slider_state& state, float alpha )
        {
            draw_text_line( draw, body_slot, ImVec2{ min.x, min.y + 13.f }, 0, fade( theme::text_faint, alpha ), it.label );
            const float value = it.min + state.value * ( it.max - it.min );
            draw_text_line( draw, body_slot, ImVec2{ max.x, min.y + 13.f }, 2, fade( theme::text, alpha ), format_value( it.format, value ) );

            const float left = min.x + 96.f;
            const float right = max.x;
            const float y = min.y + 19.f;
            add_rounded_rect_filled( draw, ImVec2{ left, y - 1.5f }, ImVec2{ right, y + 1.5f }, corner_radii{ 2.f }, fade( theme::surface_4, alpha ) );
            const float filled = ImLerp( left, right, state.shown );
            if ( filled > left + 0.5f )
                add_rounded_rect_filled( draw, ImVec2{ left, y - 1.5f }, ImVec2{ filled, y + 1.5f }, corner_radii{ 2.f }, fade( theme::accent, alpha ) );
            draw->AddCircleFilled( ImVec2{ filled, y }, 4.5f, fade( theme::text, alpha ), 14 );
        }

        void draw_keybind( ImDrawList* draw, ImVec2 min, ImVec2 max, const item& it, const keybind_state& state, float alpha )
        {
            draw_text_line( draw, body_slot, ImVec2{ min.x, min.y + 17.f }, 0, fade( theme::text_faint, alpha ), it.label );
            const ImVec2 box_min = keybind_box( min, max );
            const ImVec2 box_max{ max.x, box_min.y + 18.f };
            add_rounded_rect_filled( draw, box_min, box_max, corner_radii{ 4.f }, fade( mix_color( mix_color( theme::fill_blue, theme::surface_5, state.hover ), theme::surface_6, state.anim ), alpha ) );
            add_rounded_rect_stroke( draw, box_min + ImVec2{ 0.5f, 0.5f }, box_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, alpha ), 1.f );
            const char* text = state.waiting ? "..." : short_key_name( state.key );
            draw_text_line( draw, body_slot, ImVec2{ ( box_min.x + box_max.x ) * 0.5f, box_min.y + 14.f }, 1, fade( theme::text, alpha ), text );
        }

        void draw_combo_box( ImDrawList* draw, ImVec2 min, ImVec2 max, const item& it, const combo_state& state, float alpha )
        {
            draw_text_line( draw, body_slot, ImVec2{ min.x, min.y + 17.f }, 0, fade( theme::text_faint, alpha ), it.label );
            const ImVec2 box_min = combo_box( min, max );
            const ImVec2 box_max{ max.x, box_min.y + 18.f };
            add_rounded_rect_filled( draw, box_min, box_max, corner_radii{ 4.f }, fade( mix_color( theme::fill_blue, theme::surface_5, state.hover ), alpha ) );
            add_rounded_rect_stroke( draw, box_min + ImVec2{ 0.5f, 0.5f }, box_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, alpha ), 1.f );
            if ( it.options && state.selected >= 0 && state.selected < it.option_count )
                draw_text_line( draw, body_slot, ImVec2{ box_min.x + 8.f, box_min.y + 14.f }, 0, fade( theme::text, alpha ), it.options[state.selected] );
            draw->AddTriangleFilled( ImVec2{ box_max.x - 15.f, box_min.y + 7.5f }, ImVec2{ box_max.x - 7.f, box_min.y + 7.5f }, ImVec2{ box_max.x - 11.f, box_min.y + 12.5f }, fade( theme::text_faint, alpha ) );

            if ( state.anim <= 0.001f || popup_count >= 96 )
                return;
            popup_ref& ref = popups[popup_count++];
            ref.color = false;
            ref.state = const_cast< combo_state* >( &state );
            ref.options = it.options;
            ref.option_count = it.option_count;
            ref.min = box_min;
            ref.max = box_max;
        }

        void draw_color_swatch( ImDrawList* draw, ImVec2 min, ImVec2 max, const item& it, const color_state& state, float alpha )
        {
            draw_text_line( draw, body_slot, ImVec2{ min.x, min.y + 17.f }, 0, fade( theme::text_faint, alpha ), it.label );
            const ImVec2 swatch_min = swatch_box( min, max );
            const ImVec2 swatch_max{ max.x, swatch_min.y + 16.f };
            add_rounded_rect_filled( draw, swatch_min, swatch_max, corner_radii{ 4.f }, fade( IM_COL32( 255, 255, 255, 255 ), alpha ) );
            add_rounded_rect_filled( draw, swatch_min + ImVec2{ 2.f, 2.f }, swatch_max - ImVec2{ 2.f, 2.f }, corner_radii{ 3.f }, fade( ImGui::ColorConvertFloat4ToU32( state.value ), alpha ) );
            add_rounded_rect_stroke( draw, swatch_min + ImVec2{ 0.5f, 0.5f }, swatch_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, alpha ), 1.f );

            if ( state.anim <= 0.001f || popup_count >= 96 )
                return;
            popup_ref& ref = popups[popup_count++];
            ref.color = true;
            ref.state = const_cast< color_state* >( &state );
            ref.min = swatch_min;
            ref.max = swatch_max;
        }

        void draw_button( ImDrawList* draw, ImVec2 min, ImVec2 max, const item& it, const button_state& state, float alpha )
        {
            draw_text_line( draw, body_slot, ImVec2{ min.x, min.y + 17.f }, 0, fade( theme::text_faint, alpha ), it.label );
            const ImVec2 box_min = button_box( min, max );
            const ImVec2 box_max{ max.x, box_min.y + 18.f };
            add_rounded_rect_filled( draw, box_min, box_max, corner_radii{ 4.f }, fade( mix_color( mix_color( theme::fill_blue, theme::surface_5, state.hover ), theme::surface_6, state.press ), alpha ) );
            add_rounded_rect_stroke( draw, box_min + ImVec2{ 0.5f, 0.5f }, box_max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, alpha ), 1.f );
            draw_text_line( draw, body_slot, ImVec2{ ( box_min.x + box_max.x ) * 0.5f, box_min.y + 14.f }, 1, fade( theme::text, alpha ), it.label );
        }

        void draw_item( ImDrawList* draw, const item& it, ImVec2 min, ImVec2 max, float alpha )
        {
            switch ( it.type )
            {
            case k_heading:
                draw_text_line( draw, heading_slot, ImVec2{ min.x, min.y + 18.f }, 0, fade( theme::text_red, alpha ), it.label );
                add_rounded_rect_filled( draw, ImVec2{ min.x, min.y + 24.f }, ImVec2{ max.x, min.y + 25.f }, corner_radii{ }, fade( theme::surface_2, alpha ) );
                break;
            case k_toggle:
                draw_toggle( draw, min, *static_cast< switch_state* >( it.state ), alpha );
                draw_text_line( draw, body_slot, ImVec2{ min.x + 28.f, min.y + 17.f }, 0, fade( theme::text, alpha ), it.label );
                break;
            case k_slider:
                draw_slider( draw, min, max, it, *static_cast< slider_state* >( it.state ), alpha );
                break;
            case k_keybind:
                draw_keybind( draw, min, max, it, *static_cast< keybind_state* >( it.state ), alpha );
                break;
            case k_combo:
                draw_combo_box( draw, min, max, it, *static_cast< combo_state* >( it.state ), alpha );
                break;
            case k_color:
                draw_color_swatch( draw, min, max, it, *static_cast< color_state* >( it.state ), alpha );
                break;
            case k_button:
                draw_button( draw, min, max, it, *static_cast< button_state* >( it.state ), alpha );
                break;
            case k_field:
            {
                draw_text_line( draw, body_slot, ImVec2{ min.x, min.y + 17.f }, 0, fade( theme::text_faint, alpha ), it.label );
                field_state& state = *static_cast< field_state* >( it.state );
                const ImVec2 field_min{ max.x - 170.f, min.y + 3.f };
                const ImVec2 field_max{ max.x, field_min.y + 18.f };
                edit_field( state, "default", body_slot, ImVec2{ field_min.x + 6.f, field_min.y + 14.f }, 150.f, field_min, field_max, fade( theme::text, alpha ), false );
                break;
            }
            case k_label:
                draw_text_line( draw, body_slot, ImVec2{ min.x, min.y + 17.f }, 0, fade( theme::text_faint, alpha ), it.label );
                break;
            }
        }

        void draw_combo_popup( ImDrawList* draw, combo_state& state, ImVec2 box_min, ImVec2 box_max, const char* const* options, int count, float alpha )
        {
            if ( state.anim <= 0.001f )
                return;
            const float row = box_max.y - box_min.y;
            const ImVec2 min{ box_min.x, box_max.y + 6.f - ( 1.f - state.anim ) * 6.f };
            const ImVec2 max{ box_max.x, min.y + row * static_cast< float >( count ) };
            pointer.popup_open = state.open;
            pointer.popup_min = min;
            pointer.popup_max = max;
            add_rounded_rect_filled( draw, min, max, corner_radii{ 4.f }, fade( theme::surface_6, state.anim * alpha ) );
            state.hot = -1;
            for ( int index = 0; index < count; ++index )
            {
                const ImVec2 row_min{ min.x, min.y + row * static_cast< float >( index ) };
                const ImVec2 row_max{ max.x, row_min.y + row };
                const bool hovered = state.open && ImGui::IsWindowHovered( ImGuiHoveredFlags_ChildWindows ) && ImGui::IsMouseHoveringRect( row_min, row_max, false );
                if ( hovered )
                {
                    state.hot = index;
                    ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
                    add_rounded_rect_filled( draw, row_min + ImVec2{ 3.f, 3.f }, row_max - ImVec2{ 3.f, 3.f }, corner_radii{ 3.f }, fade( IM_COL32( 255, 255, 255, 12 ), state.anim * alpha ) );
                }
                draw_text_line( draw, body_slot, ImVec2{ row_min.x + 8.f, row_min.y + 14.f }, 0, fade( index == state.selected || hovered ? theme::text : theme::text_faint, state.anim * alpha ), options[index] );
            }
            add_rounded_rect_stroke( draw, min + ImVec2{ 0.5f, 0.5f }, max - ImVec2{ 0.5f, 0.5f }, corner_radii{ 3.5f }, fade( theme::border_2, state.anim * alpha ), 1.f );
            if ( state.open && state.hot >= 0 && pointer.clicked )
            {
                state.selected = state.hot;
                state.open = false;
            }
        }
        void draw_player_list( ImDrawList* draw, ImVec2 origin, float alpha )
        {
            const float x = column_x + ( column_width + column_gap );
            const float right = x + column_width;
            float y = start_y;

            draw_text_line( draw, heading_slot, ImVec2{ origin.x + x, origin.y + y + 18.f }, 0, fade( theme::text_red, alpha ), "Players" );
            add_rounded_rect_filled( draw, ImVec2{ origin.x + x, origin.y + y + 24.f }, ImVec2{ origin.x + right, origin.y + y + 25.f }, corner_radii{ }, fade( theme::surface_2, alpha ) );
            y += 30.f;

            int shown = 0;
            for ( const auto& entity : cache::cached_players )
            {
                if ( y + 24.f > bottom )
                    break;

                const ImVec2 row_min{ origin.x + x, origin.y + y };
                const ImVec2 row_max{ origin.x + right, origin.y + y + 24.f };
                const bool whitelisted = settings::rage::is_whitelisted( entity.name );
                const bool is_target = settings::rage::playerlist::target_name == entity.name;
                const bool hovered = pointer.hovered && pointer_inside( row_min, row_max );
                if ( hovered )
                    ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );

                add_rounded_rect_filled( draw, row_min, row_max, corner_radii{ 3.f }, fade( is_target ? theme::fill_red_2 : ( hovered ? theme::surface_5 : theme::surface ), alpha ) );

                std::string label = entity.name;
                if ( whitelisted )
                    label = "[WL] " + label;
                if ( is_target )
                    label = "[T] " + label;
                draw_text_line( draw, body_slot, ImVec2{ row_min.x + 8.f, row_min.y + 17.f }, 0, fade( whitelisted ? IM_COL32( 120, 255, 120, 255 ) : theme::text, alpha ), label.c_str( ) );

                const ImVec2 btn_min{ row_max.x - 52.f, row_min.y + 3.f };
                const ImVec2 btn_max{ row_max.x - 4.f, row_max.y - 3.f };
                add_rounded_rect_filled( draw, btn_min, btn_max, corner_radii{ 3.f }, fade( is_target ? theme::accent : theme::surface_7, alpha ) );
                draw_text_line( draw, body_slot, ImVec2{ ( btn_min.x + btn_max.x ) * 0.5f, btn_min.y + 14.f }, 1, fade( theme::text, alpha ), is_target ? "CLEAR" : "TARGET" );

                if ( hovered && pointer.clicked )
                {
                    if ( pointer_inside( btn_min, btn_max ) )
                    {
                        if ( is_target )
                        {
                            settings::rage::playerlist::target_name.clear( );
                            settings::rage::playerlist::target_address = 0;
                        }
                        else
                        {
                            settings::rage::playerlist::target_name = entity.name;
                            settings::rage::playerlist::target_address = entity.instance.address;
                        }
                    }
                    else if ( whitelisted )
                    {
                        settings::rage::playerlist::whitelist.erase( entity.name );
                    }
                    else
                    {
                        settings::rage::playerlist::whitelist.insert( entity.name );
                    }
                }

                y += 24.f;
                ++shown;
            }

            if ( shown == 0 )
            {
                draw_text_line( draw, body_slot, ImVec2{ origin.x + x, origin.y + y + 17.f }, 0, fade( theme::text_faint, alpha ), "No players cached." );
            }
        }
    }

    void draw( ImDrawList* draw, ImVec2 origin, int page )
    {
        if ( page == 2 )
            return;

        int count = 0;
        const item* items = items_for( page, count );
        if ( !items )
            return;

        static bool ready[11]{ };
        if ( !ready[page] )
        {
            for ( int index = 0; index < count; ++index )
                init_item( items[index] );
            ready[page] = true;
        }

        const bool interactive = controls::nav_main.selected == page;
        const float alpha = page_fade( controls::nav_main.amount[page], interactive );
        if ( alpha <= 0.001f )
            return;

        popup_count = 0;
        if ( interactive )
        {
            walk( items, count, origin, [ ]( const item& it, ImVec2 min, ImVec2 max ) { update_item( it, min, max ); } );
            walk( items, count, origin, [ ]( const item& it, ImVec2, ImVec2 ) { apply_item( it ); } );
        }
        keybind_state* open_keybind = nullptr;
        ImVec2 open_keybind_min{ };
        ImVec2 open_keybind_max{ };
        walk( items, count, origin, [ draw, alpha, &open_keybind, &open_keybind_min, &open_keybind_max ]( const item& it, ImVec2 min, ImVec2 max )
        {
            draw_item( draw, it, min, max, alpha );
            if ( it.type == k_keybind )
            {
                keybind_state& state = *static_cast< keybind_state* >( it.state );
                if ( state.context_open )
                {
                    open_keybind = &state;
                    open_keybind_min = keybind_box( min, max );
                    open_keybind_max = ImVec2{ max.x, min.y + 22.f };
                }
            }
        } );

        for ( int index = 0; index < popup_count; ++index )
        {
            const popup_ref& ref = popups[index];
            if ( ref.color )
                draw_color_popup( draw, *static_cast< color_state* >( ref.state ), ref.min, ref.max, theme::surface_6, theme::border_2 );
            else
                draw_combo_popup( draw, *static_cast< combo_state* >( ref.state ), ref.min, ref.max, ref.options, ref.option_count, alpha );
        }

        if ( open_keybind )
            ui::draw_activation_menu( draw, *open_keybind, open_keybind_min, open_keybind_max, alpha );

        if ( page == 5 )
            draw_player_list( draw, origin, alpha );
    }
}
