// Executor internal RVAs — Valkz Dumper
// Roblox: version-5a21b87bbc2641cf
// Resolved: 64  Missing: 0

#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cstdint>

#define REBASE_RBX(x) (reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + (x))

struct lua_State;

namespace RBX
{
    // [Core]
    inline constexpr uintptr_t CastArgs = REBASE_RBX(0x3F93510);
    inline constexpr uintptr_t FireTouchInterest = REBASE_RBX(0xA566E0);
    inline constexpr uintptr_t GetCapabilities = REBASE_RBX(0x1CA46D0);
    inline constexpr uintptr_t GetCurrentThreadIdentity = REBASE_RBX(0x40D2A40);
    inline constexpr uintptr_t GetIdentityStruct = REBASE_RBX(0x4170);
    inline constexpr uintptr_t GetLuaState = REBASE_RBX(0x787CF0);
    inline constexpr uintptr_t GetProperty = REBASE_RBX(0x1CA2B00);
    inline constexpr uintptr_t Impersonator = REBASE_RBX(0x2C608B0);
    inline constexpr uintptr_t KTable = REBASE_RBX(0x7FAF260);
    inline constexpr uintptr_t LuaVM_Load = REBASE_RBX(0x4021B10);
    inline constexpr uintptr_t Luau_Execute = REBASE_RBX(0x26BDA30);
    inline constexpr uintptr_t NewInstance = REBASE_RBX(0x45CFB70);
    inline constexpr uintptr_t OpCodeLookupTable = REBASE_RBX(0x6D45820);
    inline constexpr uintptr_t Print = REBASE_RBX(0x1C68FE0);
    inline constexpr uintptr_t PushInstance = REBASE_RBX(0x4013BA0);
    inline constexpr uintptr_t ScriptContextResume = REBASE_RBX(0x40CBD40);
    inline constexpr uintptr_t epochTime = REBASE_RBX(0x4781D40);
    inline constexpr uintptr_t get_tss_data = REBASE_RBX(0x4170);
    inline constexpr uintptr_t loadsafe = REBASE_RBX(0x40D1DA0);
    inline constexpr uintptr_t luavm__load = REBASE_RBX(0x40D2498);

    // [TaskScheduler]
    inline constexpr uintptr_t RawScheduler = REBASE_RBX(0x8C9D698);
    inline constexpr uintptr_t TaskCancel = REBASE_RBX(0x41876F0);
    inline constexpr uintptr_t TaskDefer = REBASE_RBX(0x4186980);
    inline constexpr uintptr_t TaskDelay = REBASE_RBX(0x4187180);
    inline constexpr uintptr_t TaskDesynchronize = REBASE_RBX(0x4185DB0);
    inline constexpr uintptr_t TaskSchedulerTargetFps = REBASE_RBX(0x80993C8);
    inline constexpr uintptr_t TaskSpawn = REBASE_RBX(0x4186E10);
    inline constexpr uintptr_t TaskSynchronize = REBASE_RBX(0x41859A0);
    inline constexpr uintptr_t TaskWait = REBASE_RBX(0x4187480);

    // [Input]
    inline constexpr uintptr_t FireMouseClick = REBASE_RBX(0x3AC4300);
    inline constexpr uintptr_t FireMouseHoverEnter = REBASE_RBX(0x3AC58F0);
    inline constexpr uintptr_t FireMouseHoverLeave = REBASE_RBX(0x3AC5A90);
    inline constexpr uintptr_t FireProximityPrompt = REBASE_RBX(0x309BD00);
    inline constexpr uintptr_t FireRightMouseClick = REBASE_RBX(0x3AC44A0);

    // [Luau]
    inline constexpr uintptr_t LuaH_dummynode = REBASE_RBX(0x62F6EC8);
    inline constexpr uintptr_t LuaO_nilobject = REBASE_RBX(0x62F7418);
    inline constexpr uintptr_t luaB_error = REBASE_RBX(0x26E6810);
    inline constexpr uintptr_t luaB_gcinfo = REBASE_RBX(0x26EA470);
    inline constexpr uintptr_t luaB_getfenv = REBASE_RBX(0x26E7E70);
    inline constexpr uintptr_t luaB_getmetatable = REBASE_RBX(0x26E7250);
    inline constexpr uintptr_t luaB_print = REBASE_RBX(0x26E63C0);
    inline constexpr uintptr_t luaB_rawequal = REBASE_RBX(0x26E8160);
    inline constexpr uintptr_t luaB_rawget = REBASE_RBX(0x26E8270);
    inline constexpr uintptr_t luaB_rawlen = REBASE_RBX(0x26EA3D0);
    inline constexpr uintptr_t luaB_rawset = REBASE_RBX(0x26E87C0);
    inline constexpr uintptr_t luaB_select = REBASE_RBX(0x26EBBF0);
    inline constexpr uintptr_t luaB_setfenv = REBASE_RBX(0x26E7F30);
    inline constexpr uintptr_t luaB_setmetatable = REBASE_RBX(0x26E73D0);
    inline constexpr uintptr_t luaB_tonumber = REBASE_RBX(0x26E6490);
    inline constexpr uintptr_t luaB_tostring = REBASE_RBX(0x26ECA00);
    inline constexpr uintptr_t luaB_type = REBASE_RBX(0x26EA490);
    inline constexpr uintptr_t luaB_typeof = REBASE_RBX(0x26EAA90);
    inline constexpr uintptr_t luaC_step = REBASE_RBX(0x26BC760);
    inline constexpr uintptr_t luaD_throw = REBASE_RBX(0x26ADAD0);
    inline constexpr uintptr_t lua_pcall = REBASE_RBX(0x26B8F80);

    // [Connection]
    inline constexpr uintptr_t DisconnectConnection = REBASE_RBX(0x337D430);

    // [Globals]
    inline constexpr uintptr_t BytecodeSignSeedPtr = REBASE_RBX(0x7981C58);
    inline constexpr uintptr_t FFlagListPointer = REBASE_RBX(0x876CCC8);
    inline constexpr uintptr_t FakeDataModelPointer = REBASE_RBX(0x8EB6960);
    inline constexpr uintptr_t IdentityPtr = REBASE_RBX(0x8051178);
    inline constexpr uintptr_t VisualEnginePointer = REBASE_RBX(0x8942E38);
    inline constexpr uintptr_t VmLoadTimerTls = REBASE_RBX(0x8386220);

    // [Crash]
    inline constexpr uintptr_t FLogHook = REBASE_RBX(0x876CDA0);
    inline constexpr uintptr_t RBXCrash = REBASE_RBX(0x876CD90);

} // namespace RBX
