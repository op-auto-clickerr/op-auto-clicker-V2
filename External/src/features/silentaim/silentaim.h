#pragma once
#include <cache/cache.h>
#include <sdk/math/math.h>

namespace silentaim
{
	struct silent_state_t
	{
		bool data_ready{ false };
		
		cache::entity_t target{};
		math::vector2 target_screen_pos{};
		math::vector3 target_world_pos{};
		math::vector2 cursor_client_pos{};
		math::vector2 screen_dims{};
		std::uint64_t spoof_pos_x{ 0 };
		std::uint64_t spoof_pos_y{ 0 };
		
		rbx::instance_t aim_indicator{};
		math::vector2 original_size{};
		std::vector<std::pair<std::uint64_t, math::vector2>> original_children_sizes{};
		bool has_original_sizes{ false };
	};

	inline silent_state_t state{};

	bool is_active();
	void run();
}
