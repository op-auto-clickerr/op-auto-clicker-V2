#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "../../memory/memory.h"
#include "../../util/globals.h"
#include "../../util/offsets.hpp"
#include "../../util/classes/classes.h"

struct WeaponTexture {
    std::string weapon_name;
    std::string texture_id;
};

struct SkinData {
    std::string name;
    std::vector<WeaponTexture> weapons;
};

class SkinChanger {
private:
    std::vector<SkinData> skins;
    int selected_skin_index = -1;
    bool enabled = false;
    
    roblox::instance character;
    roblox::instance backpack;
    roblox::instance starter_gear;
    
    void InitializeSkins();
    bool GetCharacter();
    bool GetBackpack();
    bool GetStarterGear();
    roblox::instance FindWeaponInCharacter(const std::string& weapon_name);
    roblox::instance FindWeaponInBackpack(const std::string& weapon_name);
    roblox::instance FindWeaponInStarterGear(const std::string& weapon_name);
    void SetWeaponTexture(roblox::instance weapon, const std::string& texture_id);
    
public:
    SkinChanger();
    ~SkinChanger();
    
    void Initialize();
    void ApplySkin(int skin_index);
    void Update();
    
    std::vector<std::string> GetSkinList() const;
    void SetSelectedSkinIndex(int index);
    void SetEnabled(bool value) { enabled = value; }
    bool IsEnabled() const { return enabled; }
    int GetSelectedSkinIndex() const { return selected_skin_index; }
};
