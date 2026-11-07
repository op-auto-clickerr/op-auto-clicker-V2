#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include "../../../ext/json/json.hpp"
#include "Offsets.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace OffsetLoader {

    inline std::string g_ExpectedVersion;
    inline bool g_LoadedFromJson = false;

    inline void EnsureOffsetsFolder(const std::string& path = "offsets.json") {
        namespace fs = std::filesystem;
        fs::path jsonPath(path);
        fs::path dir = jsonPath.parent_path();

        if (!dir.empty() && !fs::exists(dir)) {
            fs::create_directories(dir);
            printf("Created %s folder\n", dir.string().c_str());
        }

        if (!fs::exists(jsonPath)) {
            std::ofstream f(jsonPath);
            f.close();
            printf("Created empty %s (place your offsets here)\n", path.c_str());
        }
    }

    // ---- loader macros (undefined at the bottom of the function) ----
    // SEC(Namespace) opens a section, O(Namespace, Name) loads one offset, END closes it.
#define SEC(NS) if (root.contains(#NS) && root[#NS].is_object()) { auto& ns = root[#NS];
#define O(NS, X) if (ns.contains(#X)) { Offsets::NS::X = ns[#X].get<uintptr_t>(); ++loaded; }
#define END }

    // Returns: 0 = no JSON (using defaults), 1 = loaded from JSON, -1 = JSON exists but parse error
    inline int LoadOffsets(const std::string& path = "offsets.json") {
        EnsureOffsetsFolder(path);

        std::ifstream file(path);
        if (!file.is_open()) {
            return 0;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        if (content.empty() || content.find_first_not_of(" \t\n\r") == std::string::npos) {
            return 0;
        }

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(content);
        }
        catch (const std::exception& e) {
            printf("OffsetLoader: JSON parse error in %s: %s\n", path.c_str(), e.what());
            return -1;
        }

        if (j.empty() || !j.is_object()) {
            return 0;
        }

        int loaded = 0;

        try {
            if (j.contains("ClientVersion") && j["ClientVersion"].is_string()) {
                g_ExpectedVersion = j["ClientVersion"].get<std::string>();
                Offsets::ClientVersion = g_ExpectedVersion;
            }
            else if (j.contains("Roblox Version") && j["Roblox Version"].is_string()) {
                g_ExpectedVersion = j["Roblox Version"].get<std::string>();
                Offsets::ClientVersion = g_ExpectedVersion;
            }

            nlohmann::json& root = (j.contains("Offsets") && j["Offsets"].is_object()) ? j["Offsets"] : j;

            SEC(AirProperties) O(AirProperties, AirDensity) O(AirProperties, GlobalWind) END
                SEC(AnimationTrack)
                O(AnimationTrack, Animation) O(AnimationTrack, Animator) O(AnimationTrack, IsPlaying)
                O(AnimationTrack, Looped) O(AnimationTrack, Speed) O(AnimationTrack, TimePosition)
                END
                SEC(Animator) O(Animator, ActiveAnimations) END
                SEC(Atmosphere)
                O(Atmosphere, Color) O(Atmosphere, Decay) O(Atmosphere, Density)
                O(Atmosphere, Glare) O(Atmosphere, Haze) O(Atmosphere, Offset)
                END
                SEC(Attachment) O(Attachment, Position) END
                SEC(BasePart)
                O(BasePart, CastShadow) O(BasePart, Color3) O(BasePart, Locked) O(BasePart, Massless)
                O(BasePart, Primitive) O(BasePart, Reflectance) O(BasePart, Shape) O(BasePart, Transparency)
                END
                SEC(Beam)
                O(Beam, Attachment0) O(Beam, Attachment1) O(Beam, Brightness) O(Beam, CurveSize0)
                O(Beam, CurveSize1) O(Beam, LightEmission) O(Beam, LightInfluence) O(Beam, Texture)
                O(Beam, TextureLength) O(Beam, TextureSpeed) O(Beam, Width0) O(Beam, Width1) O(Beam, ZOffset)
                END
                SEC(BloomEffect) O(BloomEffect, Enabled) O(BloomEffect, Intensity) O(BloomEffect, Size) O(BloomEffect, Threshold) END
                SEC(BlurEffect) O(BlurEffect, Enabled) O(BlurEffect, Size) END
                SEC(ByteCode) O(ByteCode, Pointer) O(ByteCode, Size) END
                SEC(CachedItem) O(CachedItem, FileMeshData) END
                SEC(Camera)
                O(Camera, CameraSubject) O(Camera, CameraType) O(Camera, FieldOfView) O(Camera, ImagePlaneDepth)
                O(Camera, Position) O(Camera, Rotation) O(Camera, Viewport) O(Camera, ViewportSize)
                END
                SEC(CharacterMesh)
                O(CharacterMesh, BaseTextureId) O(CharacterMesh, BodyPart)
                O(CharacterMesh, MeshId) O(CharacterMesh, OverlayTextureId)
                END
                SEC(ClickDetector) O(ClickDetector, MaxActivationDistance) O(ClickDetector, MouseIcon) END
                SEC(Clothing) O(Clothing, Color3) O(Clothing, Template) END
                SEC(ColorCorrectionEffect)
                O(ColorCorrectionEffect, Brightness) O(ColorCorrectionEffect, Contrast)
                O(ColorCorrectionEffect, Enabled) O(ColorCorrectionEffect, TintColor)
                END
                SEC(ColorGradingEffect) O(ColorGradingEffect, Enabled) O(ColorGradingEffect, TonemapperPreset) END
                SEC(DataModel)
                O(DataModel, CreatorId) O(DataModel, GameId) O(DataModel, GameLoaded) O(DataModel, JobId)
                O(DataModel, PlaceId) O(DataModel, PlaceVersion) O(DataModel, PrimitiveCount)
                O(DataModel, ScriptContext) O(DataModel, ServerIP) O(DataModel, ToRenderView1)
                O(DataModel, ToRenderView2) O(DataModel, ToRenderView3) O(DataModel, Workspace)
                END
                SEC(DepthOfFieldEffect)
                O(DepthOfFieldEffect, Enabled) O(DepthOfFieldEffect, FarIntensity) O(DepthOfFieldEffect, FocusDistance)
                O(DepthOfFieldEffect, InFocusRadius) O(DepthOfFieldEffect, NearIntensity)
                END
                SEC(DragDetector)
                O(DragDetector, ActivatedCursorIcon) O(DragDetector, CursorIcon) O(DragDetector, MaxActivationDistance)
                O(DragDetector, MaxDragAngle) O(DragDetector, MaxDragTranslation) O(DragDetector, MaxForce)
                O(DragDetector, MaxTorque) O(DragDetector, MinDragAngle) O(DragDetector, MinDragTranslation)
                O(DragDetector, ReferenceInstance) O(DragDetector, Responsiveness)
                END
                SEC(FakeDataModel) O(FakeDataModel, Pointer) O(FakeDataModel, RealDataModel) END
                SEC(FileMeshData)
                O(FileMeshData, AABBMax) O(FileMeshData, AABBMin) O(FileMeshData, Faces)
                O(FileMeshData, FacesEnd) O(FileMeshData, Vertices) O(FileMeshData, VerticesEnd)
                END
                SEC(GuiBase2D) O(GuiBase2D, AbsolutePosition) O(GuiBase2D, AbsoluteRotation) O(GuiBase2D, AbsoluteSize) END
                SEC(GuiObject)
                O(GuiObject, BackgroundColor3) O(GuiObject, BackgroundTransparency) O(GuiObject, BorderColor3)
                O(GuiObject, Image) O(GuiObject, LayoutOrder) O(GuiObject, Position) O(GuiObject, RichText)
                O(GuiObject, Rotation) O(GuiObject, ScreenGui_Enabled) O(GuiObject, Size) O(GuiObject, Text)
                O(GuiObject, TextColor3) O(GuiObject, Visible) O(GuiObject, ZIndex)
                END
                SEC(Humanoid)
                O(Humanoid, AutoJumpEnabled) O(Humanoid, AutoRotate) O(Humanoid, AutomaticScalingEnabled)
                O(Humanoid, BreakJointsOnDeath) O(Humanoid, CameraOffset) O(Humanoid, DisplayDistanceType)
                O(Humanoid, DisplayName) O(Humanoid, EvaluateStateMachine) O(Humanoid, FloorMaterial)
                O(Humanoid, Health) O(Humanoid, HealthDisplayDistance) O(Humanoid, HealthDisplayType)
                O(Humanoid, HipHeight) O(Humanoid, HumanoidRootPart) O(Humanoid, HumanoidState)
                O(Humanoid, HumanoidStateID) O(Humanoid, IsWalking) O(Humanoid, Jump) O(Humanoid, JumpHeight)
                O(Humanoid, JumpPower) O(Humanoid, MaxHealth) O(Humanoid, MaxSlopeAngle) O(Humanoid, MoveDirection)
                O(Humanoid, MoveToPart) O(Humanoid, MoveToPoint) O(Humanoid, NameDisplayDistance)
                O(Humanoid, NameOcclusion) O(Humanoid, PlatformStand) O(Humanoid, PlatformStatePointer)
                O(Humanoid, RequiresNeck) O(Humanoid, RigType) O(Humanoid, SeatPart) O(Humanoid, Sit)
                O(Humanoid, TargetPoint) O(Humanoid, UseJumpPower) O(Humanoid, WalkTimer)
                O(Humanoid, Walkspeed) O(Humanoid, WalkspeedCheck)
                END
                SEC(Instance)
                O(Instance, ChildrenEnd) O(Instance, ChildrenStart) O(Instance, ClassBase)
                O(Instance, ClassDescriptor) O(Instance, ClassName) O(Instance, Name)
                O(Instance, NameContainer) O(Instance, Parent) O(Instance, This)
                END
                SEC(LRUHolder) O(LRUHolder, MemEnforcedLRUCache) END
                SEC(LRUNode) O(LRUNode, AssetID) O(LRUNode, CachedItem) O(LRUNode, Next) END
                SEC(Lighting)
                O(Lighting, Ambient) O(Lighting, Brightness) O(Lighting, ClockTime) O(Lighting, ColorShift_Bottom)
                O(Lighting, ColorShift_Top) O(Lighting, EnvironmentDiffuseScale) O(Lighting, EnvironmentSpecularScale)
                O(Lighting, ExposureCompensation) O(Lighting, FogColor) O(Lighting, FogEnd) O(Lighting, FogStart)
                O(Lighting, GeographicLatitude) O(Lighting, GlobalShadows) O(Lighting, GradientBottom)
                O(Lighting, GradientTop) O(Lighting, LightColor) O(Lighting, LightDirection) O(Lighting, MoonPosition)
                O(Lighting, OutdoorAmbient) O(Lighting, Sky) O(Lighting, Source) O(Lighting, SunPosition)
                END
                SEC(LocalScript) O(LocalScript, ByteCode) O(LocalScript, GUID) O(LocalScript, Hash) END
                SEC(MaterialColors)
                O(MaterialColors, Asphalt) O(MaterialColors, Basalt) O(MaterialColors, Brick) O(MaterialColors, Cobblestone)
                O(MaterialColors, Concrete) O(MaterialColors, CrackedLava) O(MaterialColors, Glacier) O(MaterialColors, Grass)
                O(MaterialColors, Ground) O(MaterialColors, Ice) O(MaterialColors, LeafyGrass) O(MaterialColors, Limestone)
                O(MaterialColors, Mud) O(MaterialColors, Pavement) O(MaterialColors, Rock) O(MaterialColors, Salt)
                O(MaterialColors, Sand) O(MaterialColors, Sandstone) O(MaterialColors, Slate) O(MaterialColors, Snow)
                O(MaterialColors, WoodPlanks)
                END
                SEC(MemEnforcedLRUCache) O(MemEnforcedLRUCache, Head) END
                SEC(MeshContentProvider) O(MeshContentProvider, LRUHolder) END
                SEC(MeshPart) O(MeshPart, MeshId) O(MeshPart, Texture) END
                SEC(Misc) O(Misc, Adornee) O(Misc, AnimationId) O(Misc, StringLength) O(Misc, Value) END
                SEC(Model) O(Model, PrimaryPart) O(Model, Scale) END
                SEC(ModuleScript) O(ModuleScript, ByteCode) O(ModuleScript, GUID) O(ModuleScript, Hash) O(ModuleScript, IsCoreScript) END
                SEC(MouseService)
                O(MouseService, InputObject) O(MouseService, InputObject2)
                O(MouseService, MousePosition) O(MouseService, SensitivityPointer)
                END
                SEC(ParticleEmitter)
                O(ParticleEmitter, Acceleration) O(ParticleEmitter, Brightness) O(ParticleEmitter, Drag)
                O(ParticleEmitter, Lifetime) O(ParticleEmitter, LightEmission) O(ParticleEmitter, LightInfluence)
                O(ParticleEmitter, Rate) O(ParticleEmitter, RotSpeed) O(ParticleEmitter, Rotation)
                O(ParticleEmitter, Speed) O(ParticleEmitter, SpreadAngle) O(ParticleEmitter, Texture)
                O(ParticleEmitter, TimeScale) O(ParticleEmitter, VelocityInheritance) O(ParticleEmitter, ZOffset)
                END
                SEC(Player)
                O(Player, AccountAge) O(Player, CameraMode) O(Player, DisplayName) O(Player, HealthDisplayDistance)
                O(Player, LocalPlayer) O(Player, LocaleId) O(Player, MaxZoomDistance) O(Player, MinZoomDistance)
                O(Player, ModelInstance) O(Player, Mouse) O(Player, NameDisplayDistance) O(Player, Team)
                O(Player, TeamColor) O(Player, UserId)
                END
                SEC(PlayerConfigurer) O(PlayerConfigurer, Pointer) END
                SEC(PlayerMouse) O(PlayerMouse, Icon) O(PlayerMouse, Workspace) END
                SEC(Primitive)
                O(Primitive, AssemblyAngularVelocity) O(Primitive, AssemblyLinearVelocity) O(Primitive, Flags)
                O(Primitive, Material) O(Primitive, Owner) O(Primitive, Position) O(Primitive, Rotation)
                O(Primitive, Size) O(Primitive, Validate)
                END
                SEC(PrimitiveFlags)
                O(PrimitiveFlags, Anchored) O(PrimitiveFlags, CanCollide)
                O(PrimitiveFlags, CanQuery) O(PrimitiveFlags, CanTouch)
                END
                SEC(ProximityPrompt)
                O(ProximityPrompt, ActionText) O(ProximityPrompt, Enabled) O(ProximityPrompt, GamepadKeyCode)
                O(ProximityPrompt, HoldDuration) O(ProximityPrompt, KeyCode) O(ProximityPrompt, MaxActivationDistance)
                O(ProximityPrompt, ObjectText) O(ProximityPrompt, RequiresLineOfSight)
                END
                SEC(RenderJob) O(RenderJob, FakeDataModel) O(RenderJob, RealDataModel) O(RenderJob, RenderView) END
                SEC(RenderView)
                O(RenderView, DeviceD3D11) O(RenderView, LightingValid)
                O(RenderView, SkyValid) O(RenderView, VisualEngine)
                END
                SEC(RunService) O(RunService, HeartbeatFPS) O(RunService, HeartbeatTask) END
                SEC(Script) O(Script, ByteCode) O(Script, GUID) O(Script, Hash) END
                SEC(ScriptContext) O(ScriptContext, RequireBypass) END
                SEC(Seat) O(Seat, Occupant) END
                SEC(Sky)
                O(Sky, MoonAngularSize) O(Sky, MoonTextureId) O(Sky, SkyboxBk) O(Sky, SkyboxDn) O(Sky, SkyboxFt)
                O(Sky, SkyboxLf) O(Sky, SkyboxOrientation) O(Sky, SkyboxRt) O(Sky, SkyboxUp) O(Sky, StarCount)
                O(Sky, SunAngularSize) O(Sky, SunTextureId)
                END
                SEC(Sound)
                O(Sound, IsPlaying) O(Sound, Looped) O(Sound, PlaybackSpeed) O(Sound, RollOffMaxDistance)
                O(Sound, RollOffMinDistance) O(Sound, SoundGroup) O(Sound, SoundId) O(Sound, Volume)
                END
                SEC(SpawnLocation)
                O(SpawnLocation, AllowTeamChangeOnTouch) O(SpawnLocation, Enabled) O(SpawnLocation, ForcefieldDuration)
                O(SpawnLocation, Neutral) O(SpawnLocation, TeamColor)
                END
                SEC(SpecialMesh) O(SpecialMesh, MeshId) O(SpecialMesh, Scale) END
                SEC(StatsItem) O(StatsItem, Value) END
                SEC(SunRaysEffect) O(SunRaysEffect, Enabled) O(SunRaysEffect, Intensity) O(SunRaysEffect, Spread) END
                SEC(SurfaceAppearance)
                O(SurfaceAppearance, AlphaMode) O(SurfaceAppearance, Color) O(SurfaceAppearance, ColorMap)
                O(SurfaceAppearance, EmissiveMaskContent) O(SurfaceAppearance, EmissiveStrength)
                O(SurfaceAppearance, EmissiveTint) O(SurfaceAppearance, MetalnessMap)
                O(SurfaceAppearance, NormalMap) O(SurfaceAppearance, RoughnessMap)
                END
                SEC(TaskScheduler)
                O(TaskScheduler, JobEnd) O(TaskScheduler, JobName) O(TaskScheduler, JobStart)
                O(TaskScheduler, MaxFPS) O(TaskScheduler, Pointer)
                END
                SEC(Team) O(Team, BrickColor) END
                SEC(Terrain)
                O(Terrain, GrassLength) O(Terrain, MaterialColors) O(Terrain, WaterColor) O(Terrain, WaterReflectance)
                O(Terrain, WaterTransparency) O(Terrain, WaterWaveSize) O(Terrain, WaterWaveSpeed)
                END
                SEC(Textures) O(Textures, Decal_Texture) O(Textures, Texture_Texture) END
                SEC(Tool)
                O(Tool, CanBeDropped) O(Tool, Enabled) O(Tool, Grip) O(Tool, ManualActivationOnly)
                O(Tool, RequiresHandle) O(Tool, TextureId) O(Tool, Tooltip)
                END
                SEC(UnionOperation) O(UnionOperation, AssetId) END
                SEC(UserInputService) O(UserInputService, WindowInputState) END
                SEC(VehicleSeat)
                O(VehicleSeat, MaxSpeed) O(VehicleSeat, SteerFloat) O(VehicleSeat, ThrottleFloat)
                O(VehicleSeat, Torque) O(VehicleSeat, TurnSpeed)
                END
                SEC(VisualEngine)
                O(VisualEngine, Dimensions) O(VisualEngine, FakeDataModel) O(VisualEngine, Pointer)
                O(VisualEngine, RenderView) O(VisualEngine, ViewMatrix)
                END
                SEC(Weld) O(Weld, Part0) O(Weld, Part1) END
                SEC(WeldConstraint) O(WeldConstraint, Part0) O(WeldConstraint, Part1) END
                SEC(WindowInputState) O(WindowInputState, CapsLock) O(WindowInputState, CurrentTextBox) END
                SEC(Workspace)
                O(Workspace, CurrentCamera) O(Workspace, DistributedGameTime)
                O(Workspace, ReadOnlyGravity) O(Workspace, World)
                END
                SEC(World)
                O(World, AirProperties) O(World, FallenPartsDestroyHeight) O(World, Gravity)
                O(World, Primitives) O(World, worldStepsPerSec)
                END
        }
        catch (const std::exception& e) {
            printf("OffsetLoader: bad value type in %s: %s\n", path.c_str(), e.what());
            return -1;
        }

        g_LoadedFromJson = true;
        printf("Offsets loaded from %s (%d offsets applied)\n", path.c_str(), loaded);
        return 1;
    }

#undef SEC
#undef O
#undef END

    inline bool WasLoadedFromJson() {
        return g_LoadedFromJson;
    }

    inline std::string GetExpectedVersion() {
        return g_ExpectedVersion;
    }

#ifdef _WIN32
    inline std::string GetRobloxVersionFromProcess(HANDLE processHandle, uint32_t processId) {
        wchar_t exePath[MAX_PATH] = { 0 };
        DWORD size = MAX_PATH;

        if (!QueryFullProcessImageNameW(processHandle, 0, exePath, &size)) {
            return "";
        }

        std::wstring pathW(exePath);
        std::string path;
        path.reserve(pathW.size());
        for (wchar_t wc : pathW) path += static_cast<char>(wc);

        std::string marker = "version-";
        size_t pos = path.find(marker);
        if (pos == std::string::npos) {
            return "";
        }

        size_t end = path.find('\\', pos);
        if (end == std::string::npos) {
            end = path.find('/', pos);
        }
        if (end == std::string::npos) {
            return "";
        }

        return path.substr(pos, end - pos);
    }
#endif

}