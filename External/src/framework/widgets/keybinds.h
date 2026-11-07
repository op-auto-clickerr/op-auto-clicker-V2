#pragma once
#include "../headers/includes.h"
// discord.gg/dopamina

 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina

 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina


 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
 // discord.gg/dopamina
enum class keybind_mode
{
    toggle = 0,
    hold   = 1,
    always = 2,
};

struct keybind_item
{
    std::string   name;
    int           key      = 0;      // VK code, 0 = unbound
    bool          active   = false;
    keybind_mode  mode     = keybind_mode::toggle;
    bool          state    = false;  // aktueller Zustand (toggle/hold)
    bool          listening = false; // discord.gg/dopamina
};

class c_keybinds
{
public:
    void render();
    void update();  // discord.gg/dopamina
    void add_keybind(const std::string& name, int key = 0,
                     keybind_mode mode = keybind_mode::toggle, bool active = true);
    
    bool is_active(const std::string& name); // discord.gg/dopamina
    
private:
    ImVec2 keybind_pos  = ImVec2(15.0f, 200.0f);
    bool   is_dragging  = false;
    ImVec2 drag_offset  = ImVec2(0.0f, 0.0f);

    int    context_item = -1;  // discord.gg/dopamina
    float  context_alpha = 0.0f;

    std::vector<keybind_item> keybinds;

    std::string vk_to_string(int vk);
};

inline std::unique_ptr<c_keybinds> keybinds = std::make_unique<c_keybinds>();
