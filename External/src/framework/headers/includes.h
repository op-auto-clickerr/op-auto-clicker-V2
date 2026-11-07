#pragma once

// Define this to use local ImGui math operators
#define IMGUI_DEFINE_MATH_OPERATORS

// Include local ImGui FIRST - direct path from framework/headers to thirdparty
#include "../../thirdparty/imgui/imgui.h"
#include "../../thirdparty/imgui/imgui_internal.h"

// Now include other headers
#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>
#include <algorithm>
#include "flags.h"
#include "../settings/colors.h"
#include "../settings/elements.h"
#include "../settings/variables.h"
#include "fonts.h"
#include "draw.h"
#include "functions.h"
#include "../data/fonts.h"
#include "../data/images.h"