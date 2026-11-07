#include "silentaim.h"

#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <random>
#include <windows.h>
#include <game/game.h>
#include <memory/memory.h>
#include <Offsets/Offsets.hpp>
#include <settings.h>
#include <features/keybind/keybind.h>
#include <cache/bodyparts/bodyparts.h>
#include <cache/cache.h>
#include <cache/custom_entities/custom_entities.h>
#include <features/aimbot/aimbot.h>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <features/silentaim/raycast/raycast_silent.h>
#include <cstring>

static void silentaim_log(const char* message)
{
	char path[MAX_PATH]{};
	if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
		return;

	char* slash = strrchr(path, '\\');
	if (!slash)
		return;

	strcpy_s(slash + 1, MAX_PATH - static_cast<size_t>((slash + 1) - path), "silentaim.log");

	FILE* file = nullptr;
	if (fopen_s(&file, path, "a") != 0 || file == nullptr)
		return;

	fprintf(file, "%s\n", message);
	fclose(file);
}

static std::vector<const char*> PART_NAMES = { "Head", "HumanoidRootPart", "LeftArm", "RightArm", "LeftLeg", "RightLeg" };
static constexpr int PART_COUNT = 6;

static rbx::vector2int16 make_viewport_from_target(const math::vector2& screenSize, const math::vector2& targetScreen)
{
	const auto clamp_i16 = [](int v) -> int16_t
		{
			v = std::clamp(v, static_cast<int>(INT16_MIN), static_cast<int>(INT16_MAX));
			return static_cast<int16_t>(v);
		};

	const int vx = static_cast<int>(std::lround(2.0f * (screenSize.x - targetScreen.x)));
	const int vy = static_cast<int>(std::lround(2.0f * (screenSize.y - targetScreen.y)));

	return rbx::vector2int16{
		clamp_i16(vx),
		clamp_i16(vy)
	};
}

static float get_distance_from_center(const math::vector2& point, const math::vector2& position)
{
	float dx{ position.x - point.x };
	float dy{ position.y - point.y };
	return std::sqrtf(dx * dx + dy * dy);
}

static math::vector3 get_velocity(cache::entity_t& entity)
{
	auto root_it = entity.parts.find("HumanoidRootPart");
	if (root_it == entity.parts.end() || !root_it->second.address)
		return math::vector3{};

	rbx::primitive_t prim = root_it->second.get_primitive();
	if (!prim.address)
		return math::vector3{};

	return memory->read<math::vector3>(prim.address + Offsets::Primitive::AssemblyLinearVelocity);
}

static math::vector3 apply_prediction(cache::entity_t& entity, const math::vector3& position)
{
	math::vector3 velocity = get_velocity(entity);
	float factor_x = (20.0f - settings::silentaim::prediction_x) * 0.02f;
	float factor_y = (20.0f - settings::silentaim::prediction_y) * 0.02f;

	return {
		position.x + velocity.x * factor_x,
		position.y + velocity.y * factor_y,
		position.z
	};
}
static bool is_silent_aim_active()
{
	if (!game::datamodel.address || !game::visengine.address || !settings::silentaim::enabled)
		return false;

	if (settings::silentaim::guncheck)
	{
		if (!silentaim::state.aim_indicator.address)
			return false;

		bool is_visible = memory->read<bool>(silentaim::state.aim_indicator.address + Offsets::GuiObject::Visible);
		if (!is_visible)
			return false;
	}

	if (settings::silentaim::use_aimbot_target)
	{
		if (aimbot::sticky_target.instance.address != 0 || aimbot::player.instance.address != 0)
			return true;
	}

	static keybind::keybind_t silentaim_kb{};
	silentaim_kb.key = settings::silentaim::keybind;
	silentaim_kb.mode = static_cast<keybind::activation_mode>(settings::silentaim::activation_mode);
	return keybind::is_active(silentaim_kb);
}

static bool is_player_knocked(const cache::entity_t& player)
{
	if (player.instance.address == 0)
		return false;

	rbx::model_instance_t model_instance = rbx::player_t(player.instance.address).get_model_instance();

	if (model_instance.address == 0)
		return false;

	rbx::instance_t body_effects = model_instance.find_first_child("BodyEffects");
	if (body_effects.address == 0)
	{
		std::vector<rbx::instance_t> children = model_instance.get_children();
		for (const auto& child : children)
		{
			if (child.get_name() == "BodyEffects")
			{
				body_effects = child;
				break;
			}
		}
		if (body_effects.address == 0)
			return false;
	}

	rbx::instance_t ko = body_effects.find_first_child("K.O");
	if (ko.address == 0)
		return false;

	std::string ko_class = ko.get_class_name();
	if (ko_class != "BoolValue")
		return false;

	bool value = false;
	try {
		value = memory->read<bool>(ko.address + Offsets::Misc::Value);
	}
	catch (...) {
		value = false;
	}
	return value;
}

static bool is_custom_entity_knocked(const settings::custom_entities::custom_entity_t& custom_entity)
{
	if (custom_entity.instance.address == 0)
		return false;

	rbx::instance_t instance(custom_entity.instance.address);
	std::string instance_class = instance.get_class_name();

	rbx::model_instance_t model_instance{ 0 };
	if (instance_class == "Model")
	{
		model_instance = rbx::model_instance_t(custom_entity.instance.address);
	}
	else
	{
		rbx::instance_t model_child = instance.find_first_child_by_class("Model");
		if (model_child.address == 0)
			return false;
		model_instance = rbx::model_instance_t(model_child.address);
	}

	if (model_instance.address == 0)
		return false;

	rbx::instance_t body_effects = model_instance.find_first_child("BodyEffects");
	if (body_effects.address == 0)
	{
		std::vector<rbx::instance_t> children = model_instance.get_children();
		for (const auto& child : children)
		{
			if (child.get_name() == "BodyEffects")
			{
				body_effects = child;
				break;
			}
		}
		if (body_effects.address == 0)
			return false;
	}

	rbx::instance_t ko = body_effects.find_first_child("K.O");
	if (ko.address == 0)
		return false;

	std::string ko_class = ko.get_class_name();
	if (ko_class != "BoolValue")
		return false;

	bool value = false;
	try {
		value = memory->read<bool>(ko.address + Offsets::Misc::Value);
	}
	catch (...) {
		value = false;
	}
	return value;
}

static bool is_valid_target(const cache::entity_t& entity, const cache::entity_t& local_player, bool ignore_friendly)
{
	if (entity.instance.address == local_player.instance.address)
		return false;

	if (settings::silentaim::teamcheck && entity.team == local_player.team)
		return false;

	if (settings::rage::is_whitelisted(entity.name))
		return false;

	// Target filter: if a player is targeted, skip everyone else
	if (settings::rage::playerlist::enabled && !settings::rage::playerlist::target_name.empty() && entity.name != settings::rage::playerlist::target_name)
		return false;

	if (settings::silentaim::knock_check && is_player_knocked(entity))
		return false;

	if (settings::silentaim::health_check_enabled && entity.health < settings::silentaim::min_health)
		return false;

	if (settings::visuals::max_distance_enabled)
	{
		auto get_pos = [](const cache::entity_t& e, math::vector3& out) -> bool {
			auto it = e.parts.find("HumanoidRootPart");
			if (it == e.parts.end() || !it->second.address) it = e.parts.find("Head");
			if (it == e.parts.end() || !it->second.address) return false;
			rbx::primitive_t prim = it->second.get_primitive();
			if (!prim.address) return false;
			out = prim.get_position();
			return true;
		};
		math::vector3 target_pos{}, local_pos{};
		if (get_pos(entity, target_pos) && get_pos(local_player, local_pos))
		{
			math::vector3 diff = { target_pos.x - local_pos.x, target_pos.y - local_pos.y, target_pos.z - local_pos.z };
			float dist = std::sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
			if (dist > settings::visuals::max_distance)
				return false;
		}
	}

	if (ignore_friendly)
		return false;

	return true;
}

static bool get_target_position(const cache::entity_t& entity, const math::matrix4& view, const math::vector2& dims, const math::vector2& mouse_pos, math::vector3& out_pos)
{
	if (settings::silentaim::target_part == 0)
	{
		// Sample a grid of points across every face of each part and aim at whichever sample
		// sits closest to the cursor.
		constexpr int samples_per_edge = 10;
		float best_distance_squared = FLT_MAX;
		math::vector3 best_pos;
		bool found = false;

		for (const auto& part_pair : entity.parts)
		{
			rbx::part_t part = part_pair.second;
			if (!part.address)
				continue;

			rbx::primitive_t prim = part.get_primitive();
			if (!prim.address)
				continue;

			math::vector3 part_pos = prim.get_position();
			if (isnan(part_pos.x) || isnan(part_pos.y) || isnan(part_pos.z))
				continue;
			if (part_pos.x == 0.f && part_pos.y == 0.f && part_pos.z == 0.f)
				continue;

			math::vector3 part_size = prim.get_size();
			if (part_size.x <= 0.f || part_size.y <= 0.f || part_size.z <= 0.f)
				continue;

			math::vector2 part_screen;
			if (!game::visengine.world_to_screen(part_pos, part_screen, dims, view))
				continue;

			math::matrix3 rotation = prim.get_rotation();
			const math::vector3 axes[3] = {
				{ rotation.m[0][0], rotation.m[0][1], rotation.m[0][2] },
				{ rotation.m[1][0], rotation.m[1][1], rotation.m[1][2] },
				{ rotation.m[2][0], rotation.m[2][1], rotation.m[2][2] }
			};
			const float sizes[3] = { part_size.x, part_size.y, part_size.z };

			for (int face = 0; face < 6; ++face)
			{
				const int axis = face / 2;
				const float sign = (face % 2 == 0) ? 1.f : -1.f;
				const int u = (axis + 1) % 3;
				const int v = (axis + 2) % 3;
				const math::vector3 face_centre = part_pos + axes[axis] * (sizes[axis] * 0.5f * sign);

				for (int i = 0; i < samples_per_edge; ++i)
				{
					const float fu = (i / (samples_per_edge - 1.f)) - 0.5f;
					for (int j = 0; j < samples_per_edge; ++j)
					{
						const float fv = (j / (samples_per_edge - 1.f)) - 0.5f;
						math::vector3 point = face_centre + axes[u] * (sizes[u] * fu) + axes[v] * (sizes[v] * fv);

						math::vector2 screen_point;
						if (!game::visengine.world_to_screen(point, screen_point, dims, view))
							continue;

						math::vector2 client_screen_point = screen_point;
						client_screen_point.x -= game::window_offset_x;
						client_screen_point.y -= game::window_offset_y;

						float dx = mouse_pos.x - client_screen_point.x;
						float dy = mouse_pos.y - client_screen_point.y;
						float distance_squared = dx * dx + dy * dy;

						if (distance_squared < best_distance_squared)
						{
							best_distance_squared = distance_squared;
							best_pos = point;
							found = true;
						}
					}
				}
			}
		}

		if (!found)
			return false;

		out_pos = best_pos;
	}
	else
	{
		const char* part_name = "Head";
		switch (settings::silentaim::target_part)
		{
		case 2: part_name = "HumanoidRootPart"; break;
		case 3: part_name = "LeftArm"; break;
		case 4: part_name = "RightArm"; break;
		case 5: part_name = "LeftLeg"; break;
		case 6: part_name = "RightLeg"; break;
		default: break;
		}

		if (!bodyparts::get_part_position(entity, part_name, out_pos))
		{
			if (settings::silentaim::target_part == 1)
			{
				if (!bodyparts::get_part_position(entity, "head", out_pos))
					return false;
			}
			else if (settings::silentaim::target_part == 2)
			{
				if (!bodyparts::get_part_position(entity, "Torso", out_pos) &&
					!bodyparts::get_part_position(entity, "UpperTorso", out_pos) &&
					!bodyparts::get_part_position(entity, "LowerTorso", out_pos))
					return false;
			}
			else
			{
				return false;
			}
		}
	}

	if (settings::silentaim::enable_prediction)
		out_pos = apply_prediction(const_cast<cache::entity_t&>(entity), out_pos);

	return true;
}

static cache::entity_t get_closest_to_mouse(const cache::entity_t& preferred)
{
	if (!game::visengine.address || !game::datamodel.address)
		return cache::entity_t{};

	const bool ignore_friendly = (settings::silentaim::priorities & (1 << 0)) != 0;
	const bool prioritise_hostile = (settings::silentaim::priorities & (1 << 1)) != 0;

	const math::matrix4 view = game::visengine.get_viewmatrix();

	HWND roblox_window = game::get_roblox_window();
	math::vector2 dims = game::visengine.get_dimensions();
	if (roblox_window)
	{
		RECT client_rect{};
		if (GetClientRect(roblox_window, &client_rect))
		{
			dims.x = static_cast<float>(client_rect.right - client_rect.left);
			dims.y = static_cast<float>(client_rect.bottom - client_rect.top);
		}
	}

	POINT cursor{};
	GetCursorPos(&cursor);

	POINT client_cursor = cursor;
	if (roblox_window)
	{
		ScreenToClient(roblox_window, &client_cursor);
	}
	const math::vector2 mouse_pos{ static_cast<float>(client_cursor.x), static_cast<float>(client_cursor.y) };

	std::vector<cache::entity_t> entities_snapshot;
	cache::entity_t local_player_snapshot;
	{
		std::lock_guard<std::mutex> lock(cache::mtx);
		entities_snapshot = cache::cached_players;
		local_player_snapshot = cache::cached_local_player;
	}

	cache::entity_t best_target{};
	float best_distance = FLT_MAX;
	cache::entity_t best_hostile_target{};
	float best_hostile_distance = FLT_MAX;

	cache::entity_t preferred_entity{};
	float preferred_distance = FLT_MAX;
	bool preferred_seen = false;

	for (const auto& entity : entities_snapshot)
	{
		if (settings::silentaim::knock_check && is_player_knocked(entity))
			continue;

		if (!is_valid_target(entity, local_player_snapshot, ignore_friendly))
			continue;

		math::vector3 target_pos;
		if (!get_target_position(entity, view, dims, mouse_pos, target_pos))
			continue;

		math::vector2 screen_pos{};
		if (!game::visengine.world_to_screen(target_pos, screen_pos, dims, view))
			continue;

		math::vector2 client_screen_pos = screen_pos;
		client_screen_pos.x -= game::window_offset_x;
		client_screen_pos.y -= game::window_offset_y;

		const float distance = get_distance_from_center(mouse_pos, client_screen_pos);
		if (settings::silentaim::use_fov && distance > settings::silentaim::fov)
			continue;

		if (preferred.instance.address && entity.instance.address == preferred.instance.address)
		{
			preferred_entity = entity;
			preferred_distance = distance;
			preferred_seen = true;
		}

		if (prioritise_hostile)
		{
			if (distance < best_hostile_distance)
			{
				best_hostile_distance = distance;
				best_hostile_target = entity;
			}
		}
		else if (distance < best_distance)
		{
			best_distance = distance;
			best_target = entity;
		}
	}

	std::vector<settings::custom_entities::custom_container_t> containers_snapshot;
	{
		std::lock_guard<std::mutex> lock(custom_entities::containers_mtx);
		containers_snapshot = settings::custom_entities::containers;
	}

	for (const auto& container : containers_snapshot)
	{
		if (!container.enabled) continue;

		for (const auto& custom_entity : container.entities)
		{
			if (!custom_entity.enabled) continue;
			if (!custom_entity.instance.address) continue;

			if (settings::silentaim::knock_check && is_custom_entity_knocked(custom_entity))
				continue;

			if (custom_entity.root_part.position.x == 0 && custom_entity.root_part.position.y == 0 && custom_entity.root_part.position.z == 0)
				continue;

			std::vector<math::vector3> target_parts;

			if (custom_entity.head.part.address)
			{
				math::vector3 pos = custom_entity.head.position;
				if (pos.x != 0 || pos.y != 0 || pos.z != 0)
					target_parts.push_back(pos);
			}

			if (custom_entity.root_part.part.address)
			{
				math::vector3 pos = custom_entity.root_part.position;
				if (pos.x != 0 || pos.y != 0 || pos.z != 0)
					target_parts.push_back(pos);
			}

			if (target_parts.empty())
				continue;

			int hitbox_index = 0;
			if (settings::silentaim::target_part >= 1 && settings::silentaim::target_part <= 6)
			{
				hitbox_index = settings::silentaim::target_part - 1;
				if (hitbox_index == 0 && target_parts.size() > 0) hitbox_index = 0;
				else if (hitbox_index == 1 && target_parts.size() > 1) hitbox_index = 1;
				else hitbox_index = 0;
			}
			else
			{
				float best_part_distance = FLT_MAX;
				int best_part_index = 0;
				for (size_t i = 0; i < target_parts.size(); ++i)
				{
					math::vector2 screen_pos;
					if (game::visengine.world_to_screen(target_parts[i], screen_pos, dims, view))
					{
						math::vector2 client_screen_pos = screen_pos;
						client_screen_pos.x -= game::window_offset_x;
						client_screen_pos.y -= game::window_offset_y;
						float distance = get_distance_from_center(mouse_pos, client_screen_pos);
						if (distance < best_part_distance)
						{
							best_part_distance = distance;
							best_part_index = static_cast<int>(i);
						}
					}
				}
				hitbox_index = best_part_index;
			}

			if (hitbox_index >= static_cast<int>(target_parts.size()))
				hitbox_index = 0;

			math::vector3 target_pos = target_parts[hitbox_index];

			if (isnan(target_pos.x) || isnan(target_pos.y) || isnan(target_pos.z) ||
				(target_pos.x == 0 && target_pos.y == 0 && target_pos.z == 0))
				continue;

			if (settings::silentaim::enable_prediction)
			{
				cache::entity_t temp_entity;
				temp_entity.instance = custom_entity.instance;
				temp_entity.parts.clear();
				for (const auto& part_pair : custom_entity.parts)
				{
					if (part_pair.second.part.address != 0)
					{
						try
						{
							rbx::part_t part_copy = part_pair.second.part;
							rbx::primitive_t test_prim = part_copy.get_primitive();
							if (test_prim.address != 0)
							{
								temp_entity.parts[part_pair.first] = part_pair.second.part;
							}
						}
						catch (...)
						{
							continue;
						}
					}
				}
				target_pos = apply_prediction(temp_entity, target_pos);
			}

			math::vector2 screen_pos;
			if (!game::visengine.world_to_screen(target_pos, screen_pos, dims, view))
				continue;

			math::vector2 client_screen_pos = screen_pos;
			client_screen_pos.x -= game::window_offset_x;
			client_screen_pos.y -= game::window_offset_y;

			const float distance = get_distance_from_center(mouse_pos, client_screen_pos);
			if (settings::silentaim::use_fov && distance > settings::silentaim::fov)
				continue;

			cache::entity_t custom_entity_wrapper;
			custom_entity_wrapper.instance = custom_entity.instance;
			custom_entity_wrapper.name = custom_entity.name;
			custom_entity_wrapper.display_name = custom_entity.name;
			custom_entity_wrapper.humanoid = rbx::humanoid_t{ 0 };
			custom_entity_wrapper.rig_type = 0;
			custom_entity_wrapper.team = 0;
			custom_entity_wrapper.health = 0.0f;
			custom_entity_wrapper.max_health = 0.0f;
			custom_entity_wrapper.knocked = false;

			custom_entity_wrapper.parts.clear();
			for (const auto& part_pair : custom_entity.parts)
			{
				if (part_pair.second.part.address != 0)
				{
					try
					{
						rbx::part_t part_copy = part_pair.second.part;
						rbx::primitive_t test_prim = part_copy.get_primitive();
						if (test_prim.address != 0)
						{
							custom_entity_wrapper.parts[part_pair.first] = part_pair.second.part;
						}
					}
					catch (...)
					{
						continue;
					}
				}
			}

			if (preferred.instance.address && custom_entity_wrapper.instance.address == preferred.instance.address)
			{
				preferred_entity = custom_entity_wrapper;
				preferred_distance = distance;
				preferred_seen = true;
			}

			if (prioritise_hostile)
			{
				if (distance < best_hostile_distance)
				{
					best_hostile_distance = distance;
					best_hostile_target = custom_entity_wrapper;
				}
			}
			else if (distance < best_distance)
			{
				best_distance = distance;
				best_target = custom_entity_wrapper;
			}
		}
	}

	cache::entity_t chosen = (prioritise_hostile && best_hostile_target.instance.address) ? best_hostile_target : best_target;
	const float chosen_distance = (prioritise_hostile && best_hostile_target.instance.address) ? best_hostile_distance : best_distance;

	if (preferred_seen && preferred_entity.instance.address &&
		(!chosen.instance.address || (chosen.instance.address != preferred.instance.address && chosen_distance >= preferred_distance - 18.0f)))
	{
		return preferred_entity;
	}

	return chosen;
}

static void targeting_thread()
{
	using namespace std::chrono_literals;

	static std::int32_t aim_indicator_check_counter = 0;

	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		if (!game::datamodel.address || !game::visengine.address)
		{
			std::this_thread::sleep_for(100ms);
			continue;
		}

		if (aim_indicator_check_counter++ % 10 == 0)
		{
			try
			{
				rbx::instance_t player_gui{};
				{
					std::lock_guard<std::mutex> lock(cache::mtx);
					if (cache::cached_local_player.instance.address)
					{
						player_gui = cache::cached_local_player.instance.find_first_child("PlayerGui");
					}
				}

				if (!player_gui.address)
				{
					silentaim::state.aim_indicator = rbx::instance_t{};
					continue;
				}

				rbx::instance_t aim_frame{};
				std::vector<rbx::instance_t> children = player_gui.get_children();

				for (auto& child : children)
				{
					if (!child.address)
						continue;

					std::string child_name = child.get_name();
					if (child_name == "Aim")
					{
						aim_frame = child;
						break;
					}

					std::string child_class = child.get_class_name();
					if (child_class == "Frame" || child_class == "ScreenGui" || child_class == "GuiObject")
					{
						std::string child_lower = child_name;
						std::transform(child_lower.begin(), child_lower.end(), child_lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

						if (child_lower.find("main") != std::string::npos)
						{
							std::vector<rbx::instance_t> grandchildren = child.get_children();
							for (auto& grandchild : grandchildren)
							{
								if (grandchild.address)
								{
									std::string grandchild_name = grandchild.get_name();
									if (grandchild_name == "Aim")
									{
										aim_frame = grandchild;
										break;
									}
								}
							}

							if (aim_frame.address)
								break;
						}
					}
				}

				if (aim_frame.address != silentaim::state.aim_indicator.address)
				{
					if (silentaim::state.aim_indicator.address && silentaim::state.has_original_sizes)
					{
						memory->write<math::vector2>(silentaim::state.aim_indicator.address + Offsets::GuiObject::Size, silentaim::state.original_size);
						for (const auto& [child_addr, original_size] : silentaim::state.original_children_sizes)
						{
							memory->write<math::vector2>(child_addr + Offsets::GuiObject::Size, original_size);
						}
					}

					silentaim::state.aim_indicator = aim_frame;
					silentaim::state.has_original_sizes = false;
					silentaim::state.original_children_sizes.clear();

					if (silentaim::state.aim_indicator.address)
					{
						silentaim::state.original_size = memory->read<math::vector2>(silentaim::state.aim_indicator.address + Offsets::GuiObject::Size);
						std::vector<rbx::instance_t> children = silentaim::state.aim_indicator.get_children();
						for (auto& child : children)
						{
							math::vector2 child_size = memory->read<math::vector2>(child.address + Offsets::GuiObject::Size);
							silentaim::state.original_children_sizes.push_back({ child.address, child_size });
						}
						silentaim::state.has_original_sizes = true;
					}
				}

				if (settings::silentaim::enabled && silentaim::state.aim_indicator.address && silentaim::state.has_original_sizes)
				{
					memory->write<math::vector2>(silentaim::state.aim_indicator.address + Offsets::GuiObject::Size, { 0, 0 });

					std::vector<rbx::instance_t> children = silentaim::state.aim_indicator.get_children();
					for (auto& child : children)
					{
						memory->write<math::vector2>(child.address + Offsets::GuiObject::Size, { 0, 0 });
					}
				}
				else if (silentaim::state.aim_indicator.address && silentaim::state.has_original_sizes)
				{
					memory->write<math::vector2>(silentaim::state.aim_indicator.address + Offsets::GuiObject::Size, silentaim::state.original_size);
					for (const auto& [child_addr, original_size] : silentaim::state.original_children_sizes)
					{
						memory->write<math::vector2>(child_addr + Offsets::GuiObject::Size, original_size);
					}
				}
			}
			catch (...)
			{
				silentaim::state.aim_indicator = rbx::instance_t{};
			}
		}

		if (!settings::silentaim::enabled && silentaim::state.aim_indicator.address && silentaim::state.has_original_sizes)
		{
			memory->write<math::vector2>(silentaim::state.aim_indicator.address + Offsets::GuiObject::Size, silentaim::state.original_size);
			for (const auto& [child_addr, original_size] : silentaim::state.original_children_sizes)
			{
				memory->write<math::vector2>(child_addr + Offsets::GuiObject::Size, original_size);
			}
		}

		if (!settings::silentaim::enabled)
		{
			silentaim::state.data_ready = false;
			silentaim::state.target = cache::entity_t{};
			silentaim::state.target_world_pos = math::vector3{};

			continue;
		}

		const bool ignore_friendly = (settings::silentaim::priorities & (1 << 0)) != 0;

		if (settings::silentaim::sticky_aim && silentaim::state.target.instance.address)
		{
			cache::entity_t refreshed_target{};
			cache::entity_t local_player_snapshot{};
			bool target_found = false;
			{
				std::lock_guard<std::mutex> lock(cache::mtx);
				local_player_snapshot = cache::cached_local_player;
				for (const auto& entity : cache::cached_players)
				{
					if (entity.instance.address == silentaim::state.target.instance.address)
					{
						refreshed_target = entity;
						target_found = true;
						break;
					}
				}
			}

			if (target_found)
			{
				if (is_valid_target(refreshed_target, local_player_snapshot, ignore_friendly))
				{
					silentaim::state.target = refreshed_target;
				}
				else
				{
					silentaim::state.target = refreshed_target;
					silentaim::state.data_ready = false;
					continue;
				}
			}
			else
			{
				silentaim::state.target = cache::entity_t{};
				silentaim::state.target_world_pos = math::vector3{};
				silentaim::state.data_ready = false;
				continue;
			}
		}
		if (!settings::silentaim::sticky_aim || !silentaim::state.target.instance.address)
		{
			cache::entity_t new_target{};

			if (settings::silentaim::use_aimbot_target)
			{
				cache::entity_t local_player_snapshot{};
				{
					std::lock_guard<std::mutex> lock(cache::mtx);
					local_player_snapshot = cache::cached_local_player;

					if (aimbot::sticky_target.instance.address != 0)
					{
						for (const auto& entity : cache::cached_players)
						{
							if (entity.instance.address == aimbot::sticky_target.instance.address)
							{
								new_target = entity;
								break;
							}
						}
					}
					else if (aimbot::player.instance.address != 0)
					{
						for (const auto& entity : cache::cached_players)
						{
							if (entity.instance.address == aimbot::player.instance.address)
							{
								new_target = entity;
								break;
							}
						}
					}
				}

				if (new_target.instance.address)
				{
					const bool ignore_friendly = (settings::silentaim::priorities & (1 << 0)) != 0;
					if (!is_valid_target(new_target, local_player_snapshot, ignore_friendly))
					{
						new_target = cache::entity_t{};
					}
				}
			}

			if (!new_target.instance.address)
			{
				new_target = get_closest_to_mouse(silentaim::state.target);
			}

			if (new_target.instance.address)
				silentaim::state.target = new_target;
			else if (!silentaim::state.target.instance.address)
			{
				silentaim::state.data_ready = false;
				continue;
			}
		}

		if (!silentaim::state.target.instance.address || !game::visengine.address)
		{
			silentaim::state.data_ready = false;
			continue;
		}

		game::update_window_offset();

		const math::matrix4 view = game::visengine.get_viewmatrix();

		HWND roblox_window = game::get_roblox_window();
		math::vector2 dims = game::visengine.get_dimensions();
		if (roblox_window)
		{
			RECT client_rect{};
			if (GetClientRect(roblox_window, &client_rect))
			{
				dims.x = static_cast<float>(client_rect.right - client_rect.left);
				dims.y = static_cast<float>(client_rect.bottom - client_rect.top);
			}
		}

		POINT cursor{};
		GetCursorPos(&cursor);

		POINT client_cursor = cursor;
		if (roblox_window)
		{
			ScreenToClient(roblox_window, &client_cursor);
		}
		const math::vector2 mouse_pos{ static_cast<float>(client_cursor.x), static_cast<float>(client_cursor.y) };

		math::vector3 target_pos;
		if (!get_target_position(silentaim::state.target, view, dims, mouse_pos, target_pos))
		{
			silentaim::state.target = cache::entity_t{};
			silentaim::state.target_world_pos = math::vector3{};
			silentaim::state.data_ready = false;
			continue;
		}

		silentaim::state.target_world_pos = target_pos;

		if (!game::visengine.world_to_screen(target_pos, silentaim::state.target_screen_pos, dims, view))
		{
			silentaim::state.target_world_pos = math::vector3{};
			silentaim::state.data_ready = false;
			continue;
		}

		math::vector2 client_target_pos = silentaim::state.target_screen_pos;
		client_target_pos.x -= game::window_offset_x;
		client_target_pos.y -= game::window_offset_y;

		if (settings::silentaim::use_fov)
		{
			const float distance = get_distance_from_center(mouse_pos, client_target_pos);
			if (distance > settings::silentaim::fov)
			{
				silentaim::state.target = cache::entity_t{};
				silentaim::state.target_world_pos = math::vector3{};
				silentaim::state.data_ready = false;
				continue;
			}
		}

		silentaim::state.spoof_pos_x = static_cast<std::uint64_t>(client_cursor.x);
		silentaim::state.spoof_pos_y = static_cast<std::uint64_t>(client_cursor.y);
		silentaim::state.cursor_client_pos = mouse_pos;
		silentaim::state.screen_dims = dims;
		silentaim::state.target_screen_pos = client_target_pos;
		silentaim::state.data_ready = true;
	}
}

static void aim_application_thread()
{
	using namespace std::chrono_literals;

	bool viewport_applied = false;
	unsigned long long write_count = 0;
	math::vector2 last_target_pos{};
	math::vector2 last_cursor_pos{};
	math::vector2 last_dims{};
	auto last_valid_time = std::chrono::steady_clock::now();
	bool had_valid_target = false;
	rbx::vector2int16 last_viewport{};
	std::uint64_t mouse_service_addr = 0;
	std::uint64_t resolved_camera = 0;
	auto last_camera_resolve = std::chrono::steady_clock::now() - 1s;
	bool prev_lmb = false;

	for (;;)
	{
		const bool active = is_silent_aim_active();
		const bool has_target = silentaim::state.data_ready && silentaim::state.target.instance.address != 0;
		const auto current_time = std::chrono::steady_clock::now();

		// Raycast method (ported from the semun build): drive the engine raycast hook,
		// feed it the current target, and run the hitchance roll. Wallbang is on when the
		// dedicated flag is set or when the wallbang method is set to "Raycast".
		if ( settings::silentaim::raycast )
		{
			raycast_silent::ensure( true );
			raycast_silent::set_scale( settings::silentaim::raycast_wallbang_strength );

			static bool s_roll_pass = true;
			static auto s_last_roll = std::chrono::steady_clock::time_point{};
			if ( std::chrono::duration_cast< std::chrono::milliseconds >( current_time - s_last_roll ).count() >= 80 )
			{
				s_last_roll = current_time;
				const int hitchance = settings::silentaim::raycast_hitchance;
				if ( hitchance >= 100 )
					s_roll_pass = true;
				else if ( hitchance <= 0 )
					s_roll_pass = false;
				else
					s_roll_pass = ( ( std::rand() % 100 ) < hitchance );
			}

			const math::vector3 raycast_target = silentaim::state.target_world_pos;
			const bool target_ok = has_target &&
				!isnan( raycast_target.x ) && !isnan( raycast_target.y ) && !isnan( raycast_target.z ) &&
				!( raycast_target.x == 0.f && raycast_target.y == 0.f && raycast_target.z == 0.f );

			const bool raycast_wallbang =
				settings::silentaim::raycast_wallbang ||
				( settings::silentaim::wallbang && settings::silentaim::wallbang_method == 2 );

			if ( active && target_ok && s_roll_pass )
				raycast_silent::set_active( true, raycast_target, raycast_wallbang );
			else
				raycast_silent::set_active( false, math::vector3{}, false );
		}
		else
		{
			raycast_silent::ensure( false );
		}

		if (game::datamodel.address && (resolved_camera == 0 || current_time - last_camera_resolve > 500ms))
		{
			last_camera_resolve = current_time;
			resolved_camera = game::camera;

			if (game::workspace.address)
			{
				try
				{
					rbx::instance_t camera_instance = game::workspace.find_first_child_by_class("Camera");
					if (camera_instance.address)
						resolved_camera = camera_instance.address;
				}
				catch (...) {}
			}
		}

		const std::uint64_t camera_address = resolved_camera ? resolved_camera : game::camera;

		math::vector2 target_pos{};
		math::vector2 cursor_pos = silentaim::state.cursor_client_pos;
		math::vector2 dims = silentaim::state.screen_dims;
		bool should_write = false;

		if (has_target)
		{
			target_pos = silentaim::state.target_screen_pos;
			last_target_pos = target_pos;
			last_cursor_pos = cursor_pos;
			last_dims = dims;
			last_valid_time = current_time;
			had_valid_target = true;
			should_write = true;
		}
		else if (had_valid_target)
		{
			const auto since = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_valid_time);
			const bool lmb_hold = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			const int grace_ms = lmb_hold ? 50 : 1;
			if (since.count() <= grace_ms)
			{
				target_pos = last_target_pos;
				cursor_pos = last_cursor_pos;
				dims = last_dims;
				should_write = true;
			}
			else
			{
				had_valid_target = false;
			}
		}

		const bool apply_target = active && should_write && !settings::silentaim::raycast;

		const bool lmb_down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		if (lmb_down && !prev_lmb)
		{
			char shot_line[384];
			snprintf(shot_line, sizeof(shot_line),
				"SHOT active=%d has_target=%d target=%llu tx=%.1f ty=%.1f cx=%.1f cy=%.1f vp=%d,%d applied=%d writes=%llu",
				active ? 1 : 0, has_target ? 1 : 0,
				static_cast<unsigned long long>(silentaim::state.target.instance.address),
				silentaim::state.target_screen_pos.x, silentaim::state.target_screen_pos.y,
				cursor_pos.x, cursor_pos.y, last_viewport.x, last_viewport.y,
				viewport_applied ? 1 : 0, write_count);
			silentaim_log(shot_line);
		}
		prev_lmb = lmb_down;

		static auto last_debug_time = std::chrono::steady_clock::now();
		static std::uint64_t last_logged_target = 0;
		const std::uint64_t target_address = silentaim::state.target.instance.address;
		const bool target_changed = target_address != last_logged_target;
		if (target_changed || current_time - last_debug_time > 1s)
		{
			const std::string target_name = target_address ? silentaim::state.target.instance.get_name() : std::string("-");
			char debug_line[512];
			snprintf(debug_line, sizeof(debug_line),
				"active=%d has_target=%d data_ready=%d target=%llu name=%s tx=%.1f ty=%.1f cx=%.1f cy=%.1f dx=%.1f dy=%.1f vp=%d,%d applied=%d writes=%llu pf=%d camera=%llu mouse=%llu",
				active ? 1 : 0, has_target ? 1 : 0, silentaim::state.data_ready ? 1 : 0,
				static_cast<unsigned long long>(target_address), target_name.c_str(),
				silentaim::state.target_screen_pos.x, silentaim::state.target_screen_pos.y,
				cursor_pos.x, cursor_pos.y, dims.x, dims.y,
				last_viewport.x, last_viewport.y, viewport_applied ? 1 : 0,
				write_count, game::is_phantom_forces ? 1 : 0,
				static_cast<unsigned long long>(camera_address),
				static_cast<unsigned long long>(mouse_service_addr));
			silentaim_log(debug_line);
			last_debug_time = current_time;
			last_logged_target = target_address;
		}

		if (!game::datamodel.address || camera_address == 0)
		{
			std::this_thread::sleep_for(50ms);
			continue;
		}

		if (!apply_target)
		{
			if (viewport_applied)
			{
				rbx::camera_t camera{ camera_address };
				math::vector2 restore_dims = game::visengine.get_dimensions();
				rbx::vector2int16 restore_vp{};
				restore_vp.x = static_cast<int16_t>(std::lround(restore_dims.x));
				restore_vp.y = static_cast<int16_t>(std::lround(restore_dims.y));
				camera.set_viewport(restore_vp);
				last_viewport = restore_vp;
				viewport_applied = false;
			}

			std::this_thread::sleep_for(10ms);
			continue;
		}

		if (target_pos.x < -2.0f || target_pos.y < -2.0f || target_pos.x > 10000.0f || target_pos.y > 10000.0f)
		{
			std::this_thread::sleep_for(10ms);
			continue;
		}

		if (settings::silentaim::hitchance_enabled && !game::is_phantom_forces)
		{
			static std::mt19937 sa_hc_rng(std::random_device{}());
			std::uniform_real_distribution<float> dist(0.0f, 100.0f);
			if (dist(sa_hc_rng) > settings::silentaim::hitchance)
			{
				std::this_thread::sleep_for(5ms);
				continue;
			}
		}

		try
		{
			if (game::is_phantom_forces)
			{
				std::uint64_t workspace_addr = memory->read<std::uint64_t>(game::datamodel.address + Offsets::DataModel::Workspace);
				if (workspace_addr == 0) { continue; }

				std::uint64_t cam = memory->read<std::uint64_t>(workspace_addr + Offsets::Workspace::CurrentCamera);
				if (cam == 0) { continue; }

				rbx::instance_t camera_instance(cam);
				rbx::instance_t camera_part_inst = camera_instance.find_first_child("Part");
				if (camera_part_inst.address == 0) { continue; }

				rbx::part_t camera_part(camera_part_inst.address);
				rbx::primitive_t cam_prim = camera_part.get_primitive();
				if (cam_prim.address == 0) { continue; }

				math::vector3 camera_pos = memory->read<math::vector3>(cam + Offsets::Camera::Position);
				math::vector3 target_world = silentaim::state.target_world_pos;

				if (std::isnan(target_world.x) || std::isnan(target_world.y) || std::isnan(target_world.z) ||
					(target_world.x == 0.0f && target_world.y == 0.0f && target_world.z == 0.0f))
				{
					continue;
				}

				auto pf_look_at = [](const math::vector3& from, const math::vector3& to) -> math::matrix3
				{
					math::vector3 dir = to - from;
					float len = dir.length();
					if (len < 0.0001f) return math::matrix3::identity();
					math::vector3 forward = dir * (1.0f / len);
					math::vector3 world_up(0.0f, 1.0f, 0.0f);
					math::vector3 right = world_up.cross(forward);
					len = right.length();
					if (len < 0.0001f) return math::matrix3::identity();
					right = right * (1.0f / len);
					math::vector3 up = forward.cross(right);
					math::matrix3 r{};
					r.m[0][0] = -right.x;  r.m[0][1] = up.x;  r.m[0][2] = -forward.x;
					r.m[1][0] =  right.y;  r.m[1][1] = up.y;  r.m[1][2] = -forward.y;
					r.m[2][0] = -right.z;  r.m[2][1] = up.z;  r.m[2][2] = -forward.z;
					return r;
				};

				math::matrix3 target_matrix = pf_look_at(camera_pos, target_world);
				target_matrix.m[1][1] = 0.01f;

				memory->write<math::matrix3>(cam_prim.address + Offsets::Primitive::Rotation, target_matrix);
				++write_count;
				std::this_thread::sleep_for(2ms);
				continue;
			}

			rbx::camera_t camera{ camera_address };

			math::vector2 write_dims = dims;
			if (write_dims.x <= 0.0f || write_dims.y <= 0.0f)
				write_dims = game::visengine.get_dimensions();

			const rbx::vector2int16 vp = camera.calculate_viewport(target_pos, write_dims, cursor_pos);
			camera.set_viewport(vp);
			last_viewport = vp;
			viewport_applied = true;
			++write_count;

			std::this_thread::yield();
		}
		catch (...)
		{
			std::this_thread::sleep_for(10ms);
		}
	}
}

bool silentaim::is_active()
{
	return is_silent_aim_active();
}

void silentaim::run()
{
	silentaim_log("silentaim::run() called - starting threads");

	std::thread([] {
		for (;;)
		{
			try
			{
				targeting_thread();
			}
			catch (...)
			{
				silentaim_log("targeting_thread crashed, restarting");
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}).detach();

	std::thread([] {
		for (;;)
		{
			try
			{
				aim_application_thread();
			}
			catch (...)
			{
				silentaim_log("aim_application_thread crashed, restarting");
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}).detach();
}
