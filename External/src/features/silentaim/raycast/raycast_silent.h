#pragma once
#include <sdk/math/math.h>

namespace raycast_silent
{

	void ensure(bool want);

	void set_active(bool on, const math::vector3& world_target, bool wallbang);

	void set_scale(float scale);

	bool installed();
	bool aiming();
}
