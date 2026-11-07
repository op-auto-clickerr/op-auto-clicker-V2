#include "include.hxx"
#include <Offsets/OffsetLoader.hpp>
#include <lazyui/lazyui.hpp>
#include <ui/ui.hpp>
#include <features/config/config.h>
#include <features/exploits/infrange/infrange.h>

#include <cstdio>
#include <cstring>

static void log_loop_stall(const char* stage, double ms)
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

static double elapsed_ms(const std::chrono::steady_clock::time_point& start)
{
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

static void reset_stall_log()
{
    char path[MAX_PATH]{};
    if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
        return;

    char* slash = strrchr(path, '\\');
    if (!slash)
        return;

    strcpy_s(slash + 1, MAX_PATH - static_cast<size_t>((slash + 1) - path), "stall.log");
    DeleteFileA(path);
}

int main()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Load external config (rebranding) first
    external_config::load();

    printf("=== %s ===\n\n", external_config::cheat_name.c_str());

    static const char* BINARY_NAME = ("RobloxPlayerBeta.exe");

    printf("Waiting for Roblox...\n");
    while (!memory->find_process_id(BINARY_NAME))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    printf("Roblox found!\n");

    while (!memory->attach_to_process(BINARY_NAME))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    printf("Attached to Roblox!\n");

    while (!memory->find_module_address(BINARY_NAME))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    printf("Module found!\n");

    // Load offsets from JSON (creates offsets.json if missing)
    int offsetResult = OffsetLoader::LoadOffsets();
    if (offsetResult == -1) {
        printf("WARNING: offsets.json has errors, using built-in defaults\n");
    }

    // Check Roblox version
    std::string robloxVersion = OffsetLoader::GetRobloxVersionFromProcess(memory->get_process_handle(), memory->get_process_id());

    bool version_mismatch = false;

    if (OffsetLoader::WasLoadedFromJson()) {
        std::string expectedVersion = OffsetLoader::GetExpectedVersion();
        if (!robloxVersion.empty() && !expectedVersion.empty() && robloxVersion != expectedVersion) {
            version_mismatch = true;
            printf("\n");
            printf("========================================\n");
            printf("  WARNING: Roblox version mismatch!\n");
            printf("========================================\n");
            printf("  Offsets version: %s\n", expectedVersion.c_str());
            printf("  Roblox version:  %s\n", robloxVersion.c_str());
            printf("\n");
            printf("  Your offsets are OUTDATED.\n");
            printf("\n");
            printf("  HOW TO UPDATE:\n");
            printf("  1. Find a website that provides Roblox\n");
            printf("     offsets in JSON format\n");
            printf("  2. Save/replace the file as 'offsets.json'\n");
            printf("     in the same folder as this exe\n");
            printf("  3. Restart the cheat\n");
            printf("========================================\n");
            printf("\n");
            printf("Press ENTER to continue with outdated offsets (may crash)...\n");
            std::cin.get();
        }
    } else {
        if (!robloxVersion.empty() && robloxVersion != Offsets::ClientVersion) {
            version_mismatch = true;
            printf("\n");
            printf("========================================\n");
            printf("  WARNING: Built-in offsets are outdated!\n");
            printf("========================================\n");
            printf("  Built-in version: %s\n", Offsets::ClientVersion.c_str());
            printf("  Roblox version:   %s\n", robloxVersion.c_str());
            printf("\n");
            printf("  HOW TO FIX:\n");
            printf("  1. Find a website that provides Roblox\n");
            printf("     offsets in JSON format\n");
            printf("  2. Save the file as 'offsets.json'\n");
            printf("     in the same folder as this exe\n");
            printf("  3. Restart the cheat\n");
            printf("========================================\n");
            printf("\n");
            printf("Press ENTER to continue with outdated offsets (may crash)...\n");
            std::cin.get();
        }
    }

    // Wait for game to fully load (DataModel + Players must be valid)
    printf("Waiting for you to load in...\n");
    std::uint64_t fake_datamodel = 0;

    // Resolving DataModel + Players only means the loading screen started. Stay here
    // until the local character has a live Humanoid, otherwise every feature below
    // runs against an empty rig and the read/write threads churn on dead pointers.
    auto wait_in_game = [&]() -> bool
    {
        try
        {
            fake_datamodel = memory->read<std::uint64_t>(memory->get_module_address() + Offsets::FakeDataModel::Pointer);
            if (fake_datamodel == 0)
                return false;

            game::datamodel = rbx::instance_t(memory->read<std::uint64_t>(fake_datamodel + Offsets::FakeDataModel::RealDataModel));
            if (game::datamodel.address == 0)
                return false;

            game::players = game::datamodel.find_first_child_by_class("Players");
            if (game::players.address == 0)
                return false;

            const std::uint64_t lp = memory->read<std::uint64_t>(game::players.address + Offsets::Player::LocalPlayer);
            if (lp == 0)
                return false;

            const auto model = rbx::player_t(lp).get_model_instance();
            if (model.address == 0)
                return false;

            const auto humanoid = rbx::instance_t(model.address).find_first_child("Humanoid");
            if (humanoid.address == 0)
                return false;

            return memory->read<float>(humanoid.address + Offsets::Humanoid::Health) > 0.f;
        }
        catch (...)
        {
            return false;
        }
    };

    while (!wait_in_game())
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    game::visengine = {memory->read<std::uint64_t>(memory->get_module_address() + Offsets::VisualEngine::Pointer)};
    printf("In game!\n");

    // Validate all offsets by reading through them
    {
        int total = 0, valid = 0;
        std::string fails;
        #define VFAIL(name) do { fails += "  [FAIL] " name "\n"; } while(0)
        #define VCHECK(name, val) do { total++; if ((val) != 0) valid++; else VFAIL(name); } while(0)
        #define VREAD(name, base, off) [&]() -> std::uint64_t { total++; if ((base) == 0) { VFAIL(name); return 0; } try { auto v = memory->read<std::uint64_t>((base) + (off)); if (v != 0) { valid++; return v; } } catch (...) {} VFAIL(name); return 0; }()
        #define VFLOAT(name, base, off, lo, hi) do { total++; try { float v = memory->read<float>((base) + (off)); if (v >= (lo) && v <= (hi)) valid++; else VFAIL(name); } catch (...) { VFAIL(name); } } while(0)

        // Core
        VCHECK("FakeDataModel::Pointer", fake_datamodel);
        VCHECK("DataModel", game::datamodel.address);
        VCHECK("VisualEngine::Pointer", game::visengine.address);
        VCHECK("Players", game::players.address);

        // Services
        std::uint64_t workspace = game::datamodel.address ? game::datamodel.find_first_child_by_class("Workspace").address : 0;
        VCHECK("Workspace", workspace);
        std::uint64_t lighting = game::datamodel.address ? game::datamodel.find_first_child_by_class("Lighting").address : 0;
        VCHECK("Lighting", lighting);
        VCHECK("TextChatService", game::datamodel.address ? game::datamodel.find_first_child_by_class("TextChatService").address : 0);
        VCHECK("RunService", game::datamodel.address ? game::datamodel.find_first_child_by_class("RunService").address : 0);
        VCHECK("UserInputService", game::datamodel.address ? game::datamodel.find_first_child_by_class("UserInputService").address : 0);
        VCHECK("CoreGui", game::datamodel.address ? game::datamodel.find_first_child_by_class("CoreGui").address : 0);

        // Workspace
        std::uint64_t camera = VREAD("Workspace::CurrentCamera", workspace, Offsets::Workspace::CurrentCamera);
        std::uint64_t world = VREAD("Workspace::World", workspace, Offsets::Workspace::World);

        // Camera
        if (camera) { VREAD("Camera::CameraSubject", camera, Offsets::Camera::CameraSubject); VFLOAT("Camera::FieldOfView", camera, Offsets::Camera::FieldOfView, 1.0f, 180.0f); }

        // Player
        std::uint64_t local_player = 0;
        if (game::players.address) { local_player = VREAD("Player::LocalPlayer", game::players.address, Offsets::Player::LocalPlayer); if (local_player) { VREAD("Player::ModelInstance", local_player, Offsets::Player::ModelInstance); } }

        // VisualEngine
        std::uint64_t render_view = VREAD("VisualEngine::RenderView", game::visengine.address, Offsets::VisualEngine::RenderView);
        if (render_view) { VREAD("RenderView::VisualEngine", render_view, Offsets::RenderView::VisualEngine); VREAD("RenderView::DeviceD3D11", render_view, Offsets::RenderView::DeviceD3D11); }

        // World
        if (world) { VFLOAT("World::Gravity", world, Offsets::World::Gravity, 0.1f, 10000.0f); VREAD("World::Primitives", world, Offsets::World::Primitives); }

        // Lighting
        if (lighting) { VFLOAT("Lighting::Brightness", lighting, Offsets::Lighting::Brightness, 0.0f, 100.0f); VFLOAT("Lighting::ClockTime", lighting, Offsets::Lighting::ClockTime, -1.0f, 48.0f); }

        // DataModel props
        if (game::datamodel.address) { VREAD("DataModel::GameId", game::datamodel.address, Offsets::DataModel::GameId); VREAD("DataModel::PlaceId", game::datamodel.address, Offsets::DataModel::PlaceId); VREAD("DataModel::ScriptContext", game::datamodel.address, Offsets::DataModel::ScriptContext); }

        // Instance structure
        if (game::players.address) {
            try { auto cs = memory->read<std::uint64_t>(game::players.address + Offsets::Instance::ChildrenStart); auto s = memory->read<std::uint64_t>(cs); auto e = memory->read<std::uint64_t>(cs + Offsets::Instance::ChildrenEnd); total++; if (s && e && e > s) valid++; else VFAIL("Instance::Children"); } catch (...) { total++; VFAIL("Instance::Children"); }
            try { auto np = memory->read<std::uint64_t>(game::players.address + Offsets::Instance::Name); auto n = memory->read_string(np); total++; if (n == "Players") valid++; else VFAIL("Instance::Name"); } catch (...) { total++; VFAIL("Instance::Name"); }
            try { auto cd = memory->read<std::uint64_t>(game::players.address + Offsets::Instance::ClassDescriptor); auto cn = memory->read<std::uint64_t>(cd + Offsets::Instance::ClassName); auto c = memory->read_string(cn); total++; if (c == "Players") valid++; else VFAIL("Instance::ClassName"); } catch (...) { total++; VFAIL("Instance::ClassName"); }
        }

        // Humanoid
        if (local_player) { try { auto m = rbx::player_t(local_player).get_model_instance(); if (m.address) { auto h = rbx::instance_t(m.address).find_first_child("Humanoid"); if (h.address) { VFLOAT("Humanoid::Health", h.address, Offsets::Humanoid::Health, 0.0f, 1000000.0f); VFLOAT("Humanoid::MaxHealth", h.address, Offsets::Humanoid::MaxHealth, 0.1f, 1000000.0f); VFLOAT("Humanoid::Walkspeed", h.address, Offsets::Humanoid::Walkspeed, 0.0f, 10000.0f); VFLOAT("Humanoid::JumpPower", h.address, Offsets::Humanoid::JumpPower, 0.0f, 10000.0f); } } } catch (...) {} }

        // TaskScheduler
        try { auto ts = memory->read<std::uint64_t>(memory->get_module_address() + Offsets::TaskScheduler::Pointer); total++; if (ts) valid++; else VFAIL("TaskScheduler::Pointer"); } catch (...) { total++; VFAIL("TaskScheduler::Pointer"); }

        #undef VFAIL
        #undef VCHECK
        #undef VREAD
        #undef VFLOAT

        char summary[64];
        snprintf(summary, sizeof(summary), "=== %d/%d offsets valid ===\n", valid, total);
        settings::globals::offset_validation_result = "\n=== Offset Validation ===\n" + fails + summary;
        printf("%s", settings::globals::offset_validation_result.c_str());
    }

    std::thread(cache::run).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(25));

    if (!InitializeStorage())
    {
        printf("failed to initialize storage\n");
        printf("\nPress ENTER to exit...\n");
        std::cin.get();
        return 1;
    }

    std::thread(AutoRescanHandler).detach();
    std::thread(aimbot::run).detach();
    std::thread(silentaim::run).detach();
    std::thread(raycast_silentaim::run).detach();
    std::thread(exploits::no_spread::run).detach();
    std::thread(exploits::infrange::run).detach();
    std::thread(triggerbot::run_aimbot).detach();
    std::thread(triggerbot::run_silentaim).detach();
    std::thread(rage::hitsounds_detector_thread).detach();
    std::thread(rage::hittracers_detector_thread).detach();
    std::thread(rage::orbit::run).detach();
    std::thread(rage::rapidfire::run).detach();
    std::thread(rage::hitbox_expander::run).detach();
    std::thread(rage::spin360::run).detach();
    rage::desync::initialize();
    std::thread(rage::magicbullet::run).detach();
    std::thread(rage::noclip::run).detach();
    std::thread(rage::hipheight::run).detach();
    std::thread(movement::run).detach();
    std::thread(movement::gravity::run).detach();
    std::thread(lighting::fog::run).detach();
    std::thread(lighting::exposure::run).detach();
    std::thread(lighting::clocktime::run).detach();
    std::thread(lighting::shadows::run).detach();
    std::thread(lighting::skybox::run).detach();
    std::thread(exploits::headless::run).detach();
    std::thread(exploits::korblox::run).detach();
    std::thread(exploits::antiafk::run).detach();
    std::thread(exploits::fpscaps::run).detach();
    std::thread(exploits::freezeplayer::run).detach();
    std::thread(console::run).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(25));

    // Autoload config if set
    if (!external_config::autoload_config.empty())
    {
        if (config::config_exists(external_config::autoload_config))
        {
            config::load_config(external_config::autoload_config);
            printf("Autoloaded config: %s\n", external_config::autoload_config.c_str());
        }
        else
        {
            printf("Autoload config '%s' not found, using defaults\n", external_config::autoload_config.c_str());
        }
    }

    if (!render->create_window())
    {
        printf("failed to create window\n");
        printf("\nPress ENTER to exit...\n");
        std::cin.get();
        return 1;
    }

    if (!render->create_device())
    {
        printf("failed to create device\n");
        printf("\nPress ENTER to exit...\n");
        std::cin.get();
        return 1;
    }

    if (!render->create_imgui())
    {
        printf("failed to create imgui\n");
        printf("\nPress ENTER to exit...\n");
        std::cin.get();
        return 1;
    }

    lazyui::app = { render->detail->window, render->detail->device, render->detail->device_context, render->detail->swap_chain };
    if (!ui::load_fonts())
    {
        printf("failed to load lazyui fonts\n");
    }

    static auto last_pid_check = std::chrono::steady_clock::now();

    reset_stall_log();

    while (true)
    {
        // Check if Roblox is still running
        if (std::chrono::steady_clock::now() - last_pid_check > std::chrono::milliseconds(500))
        {
            DWORD exit_code = 0;
            HANDLE proc = memory->get_process_handle();
            if (proc == nullptr || proc == INVALID_HANDLE_VALUE ||
                (GetExitCodeProcess(proc, &exit_code) && exit_code != STILL_ACTIVE) ||
                memory->find_process_id(BINARY_NAME) == 0)
            {
                render->running = false;
                std::exit(0);
            }
            last_pid_check = std::chrono::steady_clock::now();
        }

        const auto loop_start = std::chrono::steady_clock::now();
        auto stage_start = loop_start;

        render->start_render();
        log_loop_stall("start_render", elapsed_ms(stage_start));

        if (!should_render_ui())
        {
            stage_start = std::chrono::steady_clock::now();
            render->end_render();
            log_loop_stall("end_render_skip", elapsed_ms(stage_start));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        stage_start = std::chrono::steady_clock::now();
        render->render_visuals();
        log_loop_stall("render_visuals", elapsed_ms(stage_start));

        if (render->running)
        {
            stage_start = std::chrono::steady_clock::now();
            render->render_menu();
            log_loop_stall("render_menu", elapsed_ms(stage_start));
        }

        stage_start = std::chrono::steady_clock::now();
        render->end_render();
        log_loop_stall("end_render", elapsed_ms(stage_start));

        log_loop_stall("frame_total", elapsed_ms(loop_start));

        if (settings::menu::performance_mode)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return 0;
}
