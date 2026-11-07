#pragma once
#include <string>
#include "../../thirdparty/imgui/imgui.h"

class c_elements
{
public:

    struct 
    {
        c_text name{ "STARLIGHT" };
        c_vec2 size{ 1000, 700 };  // discord.gg/dopamina
        float rounding{ 10 };  // discord.gg/dopamina
    } window;
};

inline std::unique_ptr<c_elements> elements = std::make_unique<c_elements>();
