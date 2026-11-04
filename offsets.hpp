/*
 * Dumped With: Valkz Dumper 1.0
 * Created by: Myst (2kr5x)
 * Github: https://github.com/2kr5x/Myst
 * Roblox Version: embedded-at-build (remote may override)
 * Time Taken: 13007 ms (13.007000 seconds)
 * Total Offsets: 16042
 */

#pragma once
/* Embedded at build - remote fetch may update at runtime */
#include <cstdint>

// clang-format off
namespace offsets {
    inline char roblox_version[64] = "version-5a21b87bbc2641cf";

    namespace Actor {
        inline uintptr_t ParallelFlag = 0x100;
    }

    namespace AirProperties {
        inline uintptr_t AirDensity = 0x18;
        inline uintptr_t GlobalWind = 0x3C;
    }

    namespace AnimationTrack {
        inline uintptr_t Animation = 0xA0;
        inline uintptr_t Animator = 0xF8;
        inline uintptr_t IsPlaying = 0xA10;
        inline uintptr_t Looped = 0xCD;
        inline uintptr_t Speed = 0xBC;
        inline uintptr_t TimePosition = 0xC0;
    }

    namespace Animator {
        inline uintptr_t ActiveAnimations = 0x900;
    }

    namespace Atmosphere {
        inline uintptr_t Color = 0xA0;
        inline uintptr_t Decay = 0xAC;
        inline uintptr_t Density = 0xB8;
        inline uintptr_t Glare = 0xBC;
        inline uintptr_t Haze = 0xC0;
        inline uintptr_t Offset = 0xC4;
    }

    namespace Attachment {
        inline uintptr_t Position = 0xAC;
    }

    namespace Attribute {
        inline uintptr_t Key = 0x0;
        inline uintptr_t Size = 0x58;
        inline uintptr_t Value = 0x18;
    }

    namespace BasePart {
        inline uintptr_t CastShadow = 0x120;
        inline uintptr_t Color3 = 0x11C;
        inline uintptr_t Locked = 0x121;
        inline uintptr_t Massless = 0x122;
        inline uintptr_t Overlap = 0x1F8;
        inline uintptr_t Primitive = 0x178;
        inline uintptr_t Reflectance = 0xF4;
        inline uintptr_t Shape = 0x1A0;
        inline uintptr_t Transparency = 0x118;
    }

    namespace Beam {
        inline uintptr_t Attachment0 = 0x148;
        inline uintptr_t Attachment1 = 0x158;
        inline uintptr_t Brightness = 0x168;
        inline uintptr_t CurveSize0 = 0x16C;
        inline uintptr_t CurveSize1 = 0x170;
        inline uintptr_t LightEmission = 0x174;
        inline uintptr_t LightInfluence = 0x178;
        inline uintptr_t Texture = 0x128;
        inline uintptr_t TextureLength = 0x184;
        inline uintptr_t TextureSpeed = 0x18C;
        inline uintptr_t Width0 = 0x190;
        inline uintptr_t Width1 = 0x194;
        inline uintptr_t ZOffset = 0x198;
    }

    namespace BloomEffect {
        inline uintptr_t Enabled = 0x98;
        inline uintptr_t Intensity = 0xA0;
        inline uintptr_t Size = 0xA4;
        inline uintptr_t Threshold = 0xA8;
    }

    namespace BlurEffect {
        inline uintptr_t Enabled = 0x98;
        inline uintptr_t Size = 0xA0;
    }

    namespace ByteCode {
        inline uintptr_t Pointer = 0x10;
        inline uintptr_t Size = 0x28;
    }

    namespace CachedItem {
        inline uintptr_t FileMeshData = 0x40;
    }

    namespace CallInfo {
        inline uintptr_t base = 0x10;
        inline uintptr_t func = 0x18;
        inline uintptr_t nresults = 0x28;
        inline uintptr_t top = 0x0;
    }

    namespace Callback {
        inline uintptr_t DataModelAdjustment = 0x1C8;
        inline uintptr_t Reference = 0x34;
        inline uintptr_t WeakChain = 0x38;
        inline uintptr_t WeakObject = 0x18;
        inline uintptr_t Wrapper = 0x18;
    }

    namespace Camera {
        inline uintptr_t CFrame = 0xC0;
        inline uintptr_t CameraSubject = 0xB0;
        inline uintptr_t CameraType = 0x120;
        inline uintptr_t FieldOfView = 0x128;
        inline uintptr_t ImagePlaneDepth = 0x2D4;
        inline uintptr_t Position = 0xE4;
        inline uintptr_t Rotation = 0xC0;
        inline uintptr_t Viewport = 0x284;
        inline uintptr_t ViewportInt16 = 0x284;
        inline uintptr_t ViewportSize = 0x2C4;
    }

    namespace Capabilities {
        inline uintptr_t Capabilities = 0x28;
    }

    namespace CharacterMesh {
        inline uintptr_t BaseTextureId = 0xB0;
        inline uintptr_t BodyPart = 0x130;
        inline uintptr_t MeshId = 0xE0;
        inline uintptr_t OverlayTextureId = 0x110;
    }

    namespace ClassDescriptor {
        inline uintptr_t CallbackEntryKindValue = 0x4;
        inline uintptr_t PropertyEntryDescriptor = 0x0;
        inline uintptr_t PropertyEntryKind = 0x8;
        inline uintptr_t PropertyMap = 0x250;
        inline uintptr_t PropertyMapBucketIndices = 0x2C8;
        inline uintptr_t PropertyMapBucketMask = 0x2D8;
        inline uintptr_t PropertyMapEntries = 0x2D0;
        inline uintptr_t PropertyMapEntrySize = 0x10;
    }

    namespace ClickDetector {
        inline uintptr_t MaxActivationDistance = 0xD0;
        inline uintptr_t MouseIcon = 0xB0;
    }

    namespace Closure {
        inline uintptr_t f = 0x28;
        inline uintptr_t isC = 0x3;
        inline uintptr_t nupvalues = 0x4;
        inline uintptr_t stacksize = 0x5;
    }

    namespace Clothing {
        inline uintptr_t Color3 = 0x108;
        inline uintptr_t Template = 0xE8;
    }

    namespace ColorCorrectionEffect {
        inline uintptr_t Brightness = 0xAC;
        inline uintptr_t Contrast = 0xB0;
        inline uintptr_t Enabled = 0x98;
        inline uintptr_t TintColor = 0xA0;
    }

    namespace ColorGradingEffect {
        inline uintptr_t Enabled = 0x98;
        inline uintptr_t TonemapperPreset = 0xA0;
    }

    namespace DataModel {
        inline uintptr_t CreatorId = 0x170;
        inline uintptr_t GameId = 0x178;
        inline uintptr_t GameLoaded = 0x588;
        inline uintptr_t JobId = 0x108;
        inline uintptr_t PlaceId = 0x180;
        inline uintptr_t PlaceVersion = 0x19C;
        inline uintptr_t PrimitiveCount = 0x410;
        inline uintptr_t ScriptContext = 0x440;
        inline uintptr_t ServerIP = 0x570;
        inline uintptr_t ToRenderView1 = 0x1B8;
        inline uintptr_t ToRenderView2 = 0x8;
        inline uintptr_t ToRenderView3 = 0x28;
        inline uintptr_t Workspace = 0x148;
    }

    namespace DepthOfFieldEffect {
        inline uintptr_t Enabled = 0x98;
        inline uintptr_t FarIntensity = 0xA0;
        inline uintptr_t FocusDistance = 0xA4;
        inline uintptr_t InFocusRadius = 0xA8;
        inline uintptr_t NearIntensity = 0xAC;
    }

    namespace DragDetector {
        inline uintptr_t ActivatedCursorIcon = 0x188;
        inline uintptr_t CursorIcon = 0xB0;
        inline uintptr_t MaxActivationDistance = 0xD0;
        inline uintptr_t MaxDragAngle = 0x270;
        inline uintptr_t MaxDragTranslation = 0x234;
        inline uintptr_t MaxForce = 0x274;
        inline uintptr_t MaxTorque = 0x278;
        inline uintptr_t MinDragAngle = 0x27C;
        inline uintptr_t MinDragTranslation = 0x240;
        inline uintptr_t ReferenceInstance = 0x1B8;
        inline uintptr_t Responsiveness = 0x288;
    }

    namespace EventDescriptor {
        inline uintptr_t Signature = 0x48;
    }

    namespace EventInstance {
        inline uintptr_t Descriptor = 0x0;
        inline uintptr_t OwningInstance = 0x18;
    }

    namespace EventSignature {
        inline uintptr_t FirstParameter = 0x8;
        inline uintptr_t MaximumParameters = 0x80;
        inline uintptr_t ParameterStride = 0x70;
    }

    namespace ExtraSpace {
        inline uintptr_t IsCoreScript = 0x168;
        inline uintptr_t RequireBypass = 0xA00;
        inline uintptr_t ScriptContextToResume = 0x8E0;
        inline uintptr_t ScriptContextToState = 0x300;
        inline uintptr_t WaitingHybridScriptsJobScriptContext = 0x1C0;
    }

    namespace FakeDataModel {
        inline uintptr_t Pointer = 0x8D06600;
        inline uintptr_t RealDataModel = 0x1F8;
    }

    namespace FastClusterEntity {
        inline uintptr_t AlphaByte = 0x14;
        inline uintptr_t BBoxMaxX = 0xA4;
        inline uintptr_t BBoxMaxY = 0xA8;
        inline uintptr_t BBoxMaxZ = 0xAC;
        inline uintptr_t BBoxMinX = 0x98;
        inline uintptr_t BBoxMinY = 0x9C;
        inline uintptr_t BBoxMinZ = 0xA0;
        inline uintptr_t ContextPtr = 0x8;
        inline uintptr_t DecalMaterialPtr = 0x48;
        inline uintptr_t MaterialPtr = 0x20;
        inline uintptr_t PrimitiveIndexArrayPtr = 0x80;
        inline uintptr_t PrimitivePoolPtr = 0x1A0;
        inline uintptr_t RenderQueueId = 0x10;
        inline uintptr_t TechniqueArrayPtr = 0x70;
        inline uintptr_t VTableRva = 0x0;
    }

    namespace FileMeshData {
        inline uintptr_t AABBMax = 0x18C;
        inline uintptr_t AABBMin = 0x180;
        inline uintptr_t Faces = 0x30;
        inline uintptr_t FacesEnd = 0x38;
        inline uintptr_t Vertices = 0x0;
        inline uintptr_t VerticesEnd = 0x8;
    }

    namespace GarbageCollector {
        inline uintptr_t L_global = 0x28;
        inline uintptr_t g_freepages = 0x68;
        inline uintptr_t g_gcstate = 0x39;
        inline uintptr_t g_gcstepmul = 0x44;
        inline uintptr_t g_gcstepsize = 0x40;
        inline uintptr_t g_gray = 0x20;
        inline uintptr_t g_grayagain = 0x18;
        inline uintptr_t g_weak = 0x10;
    }

    namespace GuiBase2D {
        inline uintptr_t AbsolutePosition = 0xF4;
        inline uintptr_t AbsoluteRotation = 0xD0;
        inline uintptr_t AbsoluteSize = 0xFC;
    }

    namespace GuiObject {
        inline uintptr_t Active = 0x570;
        inline uintptr_t AnchorPoint = 0x520;
        inline uintptr_t AutomaticSize = 0x528;
        inline uintptr_t BackgroundColor3 = 0x508;
        inline uintptr_t BackgroundTransparency = 0x514;
        inline uintptr_t BorderColor3 = 0x514;
        inline uintptr_t BorderMode = 0x530;
        inline uintptr_t BorderSizePixel = 0x534;
        inline uintptr_t ClipsDescendants = 0x571;
        inline uintptr_t GuiState = 0x540;
        inline uintptr_t Image = 0x9A8;
        inline uintptr_t Interactable = 0x573;
        inline uintptr_t LayoutOrder = 0x544;
        inline uintptr_t Position = 0x4D8;
        inline uintptr_t RichText = 0xBA0;
        inline uintptr_t Rotation = 0xD0;
        inline uintptr_t ScreenGui_Enabled = 0x48C;
        inline uintptr_t Selectable = 0x574;
        inline uintptr_t SelectionOrder = 0x560;
        inline uintptr_t Size = 0x4F8;
        inline uintptr_t SizeConstraint = 0x568;
        inline uintptr_t Text = 0xE08;
        inline uintptr_t TextColor3 = 0xEB8;
        inline uintptr_t Visible = 0x575;
        inline uintptr_t ZIndex = 0x56C;
    }

    namespace Humanoid {
        inline uintptr_t AutoJumpEnabled = 0x1C8;
        inline uintptr_t AutoRotate = 0x1C9;
        inline uintptr_t AutomaticScalingEnabled = 0x1CA;
        inline uintptr_t BreakJointsOnDeath = 0x1CC;
        inline uintptr_t CameraOffset = 0x110;
        inline uintptr_t DisplayDistanceType = 0x174;
        inline uintptr_t DisplayName = 0xA0;
        inline uintptr_t EvaluateStateMachine = 0x1CD;
        inline uintptr_t FloorMaterial = 0x178;
        inline uintptr_t Health = 0x184;
        inline uintptr_t HealthDisplayDistance = 0x17C;
        inline uintptr_t HealthDisplayType = 0x180;
        inline uintptr_t HipHeight = 0x188;
        inline uintptr_t HumanoidRootPart = 0x460;
        inline uintptr_t HumanoidState = 0x8A8;
        inline uintptr_t HumanoidStateID = 0x20;
        inline uintptr_t IsWalking = 0xA2E;
        inline uintptr_t Jump = 0x1D0;
        inline uintptr_t JumpHeight = 0x194;
        inline uintptr_t JumpPower = 0x198;
        inline uintptr_t MaxHealth = 0x19C;
        inline uintptr_t MaxSlopeAngle = 0x1A0;
        inline uintptr_t MoveDirection = 0x134;
        inline uintptr_t MoveToPart = 0x100;
        inline uintptr_t MoveToPoint = 0x158;
        inline uintptr_t NameDisplayDistance = 0x1A4;
        inline uintptr_t NameOcclusion = 0x1A8;
        inline uintptr_t PlatformStand = 0x1D2;
        inline uintptr_t PlatformStatePointer = 0x4746A4F8;
        inline uintptr_t RequiresNeck = 0x1D3;
        inline uintptr_t RigType = 0x1B4;
        inline uintptr_t SeatPart = 0xF0;
        inline uintptr_t Sit = 0x1D3;
        inline uintptr_t TargetPoint = 0x140;
        inline uintptr_t UseJumpPower = 0x1D6;
        inline uintptr_t WalkSpeed = 0x1C4;
        inline uintptr_t WalkSpeedCheck = 0x39C;
        inline uintptr_t WalkTimer = 0x0;
        inline uintptr_t WalkToPoint = 0x158;
        inline uintptr_t Walkspeed = 0x1C4;
        inline uintptr_t WalkspeedCheck = 0x39C;
    }

    namespace Hyperion {
        inline uintptr_t Bitmap = 0x1473148;
        inline uintptr_t InstrumentationCallback = 0x800400;
    }

    namespace InputObject {
        inline uintptr_t MousePosition = 0xD4;
    }

    namespace Instance {
        inline uintptr_t Children = 0x78;
        inline uintptr_t ChildrenEnd = 0x8;
        inline uintptr_t ChildrenStart = 0x70;
        inline uintptr_t ClassBase = 0x1B0;
        inline uintptr_t ClassDescriptor = 0x18;
        inline uintptr_t ClassName = 0x8;
        inline uintptr_t ComponentMap = 0x38;
        inline uintptr_t Name = 0x0;
        inline uintptr_t NameContainer = 0x68;
        inline uintptr_t Parent = 0x60;
        inline uintptr_t This = 0x8;
    }

    namespace LRUHolder {
        inline uintptr_t MemEnforcedLRUCache = 0x20;
    }

    namespace LRUNode {
        inline uintptr_t AssetID = 0x10;
        inline uintptr_t CachedItem = 0x40;
        inline uintptr_t Next = 0x0;
    }

    namespace Lighting {
        inline uintptr_t Ambient = 0xB8;
        inline uintptr_t Atmosphere = 0x1C0;
        inline uintptr_t Brightness = 0x100;
        inline uintptr_t ClockTime = 0xB0;
        inline uintptr_t ColorShift_Bottom = 0xD0;
        inline uintptr_t ColorShift_Top = 0xC4;
        inline uintptr_t EnvironmentDiffuseScale = 0x104;
        inline uintptr_t EnvironmentSpecularScale = 0x108;
        inline uintptr_t ExposureCompensation = 0x10C;
        inline uintptr_t FogColor = 0xDC;
        inline uintptr_t FogEnd = 0x114;
        inline uintptr_t FogStart = 0x118;
        inline uintptr_t GeographicLatitude = 0x11C;
        inline uintptr_t GlobalShadows = 0x12C;
        inline uintptr_t GradientBottom = 0x178;
        inline uintptr_t GradientTop = 0x138;
        inline uintptr_t LightColor = 0x144;
        inline uintptr_t LightDirection = 0x150;
        inline uintptr_t MoonPosition = 0x16C;
        inline uintptr_t OutdoorAmbient = 0xE8;
        inline uintptr_t ShadowSoftness = 0x124;
        inline uintptr_t Sky = 0x1B0;
        inline uintptr_t Source = 0x15C;
        inline uintptr_t SunPosition = 0x160;
    }

    namespace LightingParameters {
        inline uintptr_t GeographicLatitude = 0x11C;
        inline uintptr_t LightColor = 0x144;
        inline uintptr_t LightDirection = 0x150;
        inline uintptr_t SkyAmbient = 0x138;
        inline uintptr_t SkyAmbient2 = 0x120;
        inline uintptr_t Source = 0x15C;
        inline uintptr_t TrueMoonPosition = 0x16C;
        inline uintptr_t TrueSunPosition = 0x160;
    }

    namespace LocalScript {
        inline uintptr_t ByteCode = 0x0;
        inline uintptr_t Bytecode = 0x190;
        inline uintptr_t GUID = 0xB8;
        inline uintptr_t Hash = 0x188;
    }

    namespace MaterialColors {
        inline uintptr_t Asphalt = 0x30;
        inline uintptr_t Basalt = 0x27;
        inline uintptr_t Brick = 0xF;
        inline uintptr_t Cobblestone = 0x33;
        inline uintptr_t Concrete = 0xC;
        inline uintptr_t CrackedLava = 0x2D;
        inline uintptr_t Glacier = 0x1B;
        inline uintptr_t Grass = 0x6;
        inline uintptr_t Ground = 0x2A;
        inline uintptr_t Ice = 0x36;
        inline uintptr_t LeafyGrass = 0x39;
        inline uintptr_t Limestone = 0x3F;
        inline uintptr_t Mud = 0x24;
        inline uintptr_t Pavement = 0x42;
        inline uintptr_t Rock = 0x18;
        inline uintptr_t Salt = 0x3C;
        inline uintptr_t Sand = 0x12;
        inline uintptr_t Sandstone = 0x21;
        inline uintptr_t Slate = 0x9;
        inline uintptr_t Snow = 0x1E;
        inline uintptr_t WoodPlanks = 0x15;
    }

    namespace MaterialLayer {
        inline uintptr_t ColorData = 0x24;
        inline uintptr_t FillModeByte = 0x11;
        inline uintptr_t Flags2 = 0x20;
        inline uintptr_t MatFlags = 0x18;
        inline uintptr_t Param = 0x1C;
        inline uintptr_t Stride = 0x88;
    }

    namespace MemEnforcedLRUCache {
        inline uintptr_t Head = 0x8;
    }

    namespace MeshContentProvider {
        inline uintptr_t LRUHolder = 0xC0;
    }

    namespace MeshPart {
        inline uintptr_t MeshId = 0x2F8;
        inline uintptr_t Texture = 0x328;
        inline uintptr_t TextureId = 0x328;
    }

    namespace Misc {
        inline uintptr_t Adornee = 0xD8;
        inline uintptr_t AnimationId = 0xA8;
        inline uintptr_t StringLength = 0x10;
        inline uintptr_t Value = 0xA0;
    }

    namespace Model {
        inline uintptr_t PrimaryPart = 0x240;
        inline uintptr_t Scale = 0x12C;
    }

    namespace ModuleScript {
        inline uintptr_t ByteCode = 0x0;
        inline uintptr_t Bytecode = 0x138;
        inline uintptr_t GUID = 0xB8;
        inline uintptr_t Hash = 0x348;
        inline uintptr_t IsCoreScript = 0x0;
    }

    namespace MouseService {
        inline uintptr_t InputObject = 0xE8;
        inline uintptr_t InputObject2 = 0xF8;
        inline uintptr_t MousePosition = 0xBC;
        inline uintptr_t SensitivityPointer = 0x0;
    }

    namespace ParticleEmitter {
        inline uintptr_t Acceleration = 0x1C8;
        inline uintptr_t Brightness = 0x204;
        inline uintptr_t Drag = 0x208;
        inline uintptr_t Lifetime = 0x1DC;
        inline uintptr_t LightEmission = 0x220;
        inline uintptr_t LightInfluence = 0x224;
        inline uintptr_t Rate = 0x230;
        inline uintptr_t RotSpeed = 0x1E4;
        inline uintptr_t Rotation = 0x1EC;
        inline uintptr_t Speed = 0x1F4;
        inline uintptr_t SpreadAngle = 0x1FC;
        inline uintptr_t Texture = 0x1A8;
        inline uintptr_t TimeScale = 0x244;
        inline uintptr_t VelocityInheritance = 0x248;
        inline uintptr_t ZOffset = 0x24C;
    }

    namespace Player {
        inline uintptr_t AccountAge = 0x364;
        inline uintptr_t CameraMode = 0x378;
        inline uintptr_t Character = 0x2A0;
        inline uintptr_t DisplayName = 0x120;
        inline uintptr_t HealthDisplayDistance = 0x3A0;
        inline uintptr_t LocalPlayer = 0x118;
        inline uintptr_t LocaleId = 0x100;
        inline uintptr_t MaxZoomDistance = 0x370;
        inline uintptr_t MinZoomDistance = 0x374;
        inline uintptr_t ModelInstance = 0x2A0;
        inline uintptr_t Mouse = 0x11F0;
        inline uintptr_t NameDisplayDistance = 0x3B0;
        inline uintptr_t Team = 0x2E0;
        inline uintptr_t TeamColor = 0x3B8;
        inline uintptr_t UserId = 0xB8;
    }

    namespace PlayerConfigurer {
        inline uintptr_t Pointer = 0x0;
    }

    namespace PlayerMouse {
        inline uintptr_t Icon = 0xB0;
        inline uintptr_t Workspace = 0x138;
    }

    namespace Players {
        inline uintptr_t LocalPlayer = 0x118;
    }

    namespace Primitive {
        inline uintptr_t AssemblyAngularVelocity = 0xEC;
        inline uintptr_t AssemblyLinearVelocity = 0xE0;
        inline uintptr_t CFrame = 0xB0;
        inline uintptr_t Flags = 0x1BE;
        inline uintptr_t Material = 0x24E;
        inline uintptr_t Orientation = 0xB0;
        inline uintptr_t Owner = 0x218;
        inline uintptr_t Position = 0xD4;
        inline uintptr_t PrimitiveFlags = 0x1BE;
        inline uintptr_t Rotation = 0xB0;
        inline uintptr_t Size = 0x1C4;
        inline uintptr_t Validate = 0x6;
    }

    namespace PrimitiveFlags {
        inline uintptr_t Anchored = 0x2;
        inline uintptr_t CanCollide = 0x8;
        inline uintptr_t CanQuery = 0x20;
        inline uintptr_t CanTouch = 0x10;
    }

    namespace PrimitivePool {
        inline uintptr_t ArrayBase = 0x20;
    }

    namespace PrimitiveRecord {
        inline uintptr_t Stride = 0x30;
        inline uintptr_t Translation = 0x24;
    }

    namespace Properties {
        inline uintptr_t ClassDescriptor = 0x18;
        inline uintptr_t ClassName = 0x8;
        inline uintptr_t Properties = 0x70;
        inline uintptr_t PropertyDescriptorBitFlags = 0x8C;
        inline uintptr_t TType = 0x68;
        inline uintptr_t TTypeNumber = 0x30;
    }

    namespace PropertyDescriptor {
        inline uintptr_t CallbackStorage = 0x80;
        inline uintptr_t ScriptableMask = 0x10;
        inline uintptr_t TypeDescriptor = 0x68;
    }

    namespace Proto {
        inline uintptr_t code = 0x30;
        inline uintptr_t k = 0x28;
        inline uintptr_t sizecode = 0x98;
        inline uintptr_t sizek = 0xA8;
    }

    namespace ProximityPrompt {
        inline uintptr_t ActionText = 0x98;
        inline uintptr_t Enabled = 0x11E;
        inline uintptr_t GamepadKeyCode = 0x104;
        inline uintptr_t HoldDuration = 0x108;
        inline uintptr_t KeyCode = 0x10C;
        inline uintptr_t KeyboardKeyCode = 0x10C;
        inline uintptr_t MaxActivationDistance = 0x110;
        inline uintptr_t ObjectText = 0xB8;
        inline uintptr_t RequiresLineOfSight = 0x11F;
    }

    namespace Render {
        inline uintptr_t RenderJobToView = 0x1D0;
        inline uintptr_t RenderViewToDevice = 0x8;
    }

    namespace RenderJob {
        inline uintptr_t FakeDataModel = 0x38;
        inline uintptr_t RealDataModel = 0x1F0;
        inline uintptr_t RenderView = 0x1E0;
    }

    namespace RenderQueue {
        inline uintptr_t AlwaysOnTop = 0xD;
        inline uintptr_t AlwaysOnTopAdorns = 0xE;
        inline uintptr_t Decals = 0x2;
        inline uintptr_t Glass = 0x8;
        inline uintptr_t GlassTint = 0x7;
        inline uintptr_t OnTopReadOnlyDepth = 0xC;
        inline uintptr_t OnTopWithDepth = 0xB;
        inline uintptr_t Opaque = 0x0;
        inline uintptr_t OpaqueAdorns = 0x4;
        inline uintptr_t OpaqueCasters = 0x3;
        inline uintptr_t OpaqueWithAlpha = 0x5;
        inline uintptr_t Screen = 0xF;
        inline uintptr_t ScreenOnTopOfBlur = 0x10;
        inline uintptr_t Terrain = 0x1;
        inline uintptr_t Transparent = 0x9;
        inline uintptr_t TransparentCasters = 0xA;
        inline uintptr_t Water = 0x6;
    }

    namespace RenderView {
        inline uintptr_t DeviceD3D11 = 0x0;
        inline uintptr_t LightingValid = 0x278;
        inline uintptr_t SkyValid = 0x0;
        inline uintptr_t SkyboxValid = 0x28D;
        inline uintptr_t VisualEngine = 0x0;
    }

    namespace RobloxExtraSpace {
        inline uintptr_t Capabilities = 0x70;
        inline uintptr_t Continuations = 0x98;
        inline uintptr_t Identity = 0x48;
        inline uintptr_t Script = 0x60;
        inline uintptr_t Shared = 0x18;
        inline uintptr_t capabilities = 0x68;
        inline uintptr_t identity = 0x48;
        inline uintptr_t script = 0x58;
    }

    namespace RunService {
        inline uintptr_t HeartbeatFPS = 0xF4;
        inline uintptr_t HeartbeatTask = 0x9668;
    }

    namespace Script {
        inline uintptr_t ByteCode = 0x0;
        inline uintptr_t GUID = 0xB8;
        inline uintptr_t Hash = 0x188;
    }

    namespace ScriptContext {
        inline uintptr_t LuaState = 0x300;
        inline uintptr_t RequireBypass = 0xA00;
        inline uintptr_t Resume = 0x8E0;
    }

    namespace Seat {
        inline uintptr_t Occupant = 0x200;
    }

    namespace SignalConnection {
        inline uintptr_t Enabled = 0x20;
        inline uintptr_t FunctionScriptSlotSelf = 0x18;
        inline uintptr_t Next = 0x10;
        inline uintptr_t SelfWeakFallback = 0x48;
        inline uintptr_t SlotWrapper = 0x30;
        inline uintptr_t SlotWrapperFallbackSelf = 0x40;
        inline uintptr_t SlotWrapperSelf = 0x38;
        inline uintptr_t WeakChain = 0x68;
        inline uintptr_t WeakChainNext = 0x8;
    }

    namespace SignalWeakObject {
        inline uintptr_t FunctionReference = 0x34;
        inline uintptr_t Pointer = 0x60;
        inline uintptr_t Thread = 0x28;
        inline uintptr_t WaitContainer = 0x50;
    }

    namespace Sky {
        inline uintptr_t MoonAngularSize = 0x22C;
        inline uintptr_t MoonTextureId = 0xB0;
        inline uintptr_t SkyboxBk = 0xE0;
        inline uintptr_t SkyboxDn = 0x110;
        inline uintptr_t SkyboxFt = 0x140;
        inline uintptr_t SkyboxLf = 0x170;
        inline uintptr_t SkyboxOrientation = 0x220;
        inline uintptr_t SkyboxRt = 0x1A0;
        inline uintptr_t SkyboxUp = 0x1D0;
        inline uintptr_t StarCount = 0x230;
        inline uintptr_t SunAngularSize = 0x224;
        inline uintptr_t SunTextureId = 0x200;
    }

    namespace Sound {
        inline uintptr_t IsPlaying = 0x128;
        inline uintptr_t Looped = 0x125;
        inline uintptr_t PlaybackSpeed = 0x104;
        inline uintptr_t RollOffMaxDistance = 0x108;
        inline uintptr_t RollOffMinDistance = 0x10C;
        inline uintptr_t SoundGroup = 0xD0;
        inline uintptr_t SoundId = 0xB0;
        inline uintptr_t Volume = 0x118;
    }

    namespace SpawnLocation {
        inline uintptr_t AllowTeamChangeOnTouch = 0x1D8;
        inline uintptr_t Enabled = 0x1D9;
        inline uintptr_t ForcefieldDuration = 0x1D0;
        inline uintptr_t Neutral = 0x1DA;
        inline uintptr_t TeamColor = 0x1D4;
    }

    namespace SpecialMesh {
        inline uintptr_t MeshId = 0xE0;
        inline uintptr_t Offset = 0xA0;
        inline uintptr_t Scale = 0xAC;
        inline uintptr_t TextureId = 0x110;
    }

    namespace StatsItem {
        inline uintptr_t Value = 0xB8;
    }

    namespace SunRaysEffect {
        inline uintptr_t Enabled = 0x98;
        inline uintptr_t Intensity = 0xA0;
        inline uintptr_t Spread = 0xA4;
    }

    namespace SurfaceAppearance {
        inline uintptr_t AlphaMode = 0x1D8;
        inline uintptr_t Color = 0x1C0;
        inline uintptr_t ColorMap = 0xB0;
        inline uintptr_t EmissiveMaskContent = 0xE0;
        inline uintptr_t EmissiveStrength = 0x1DC;
        inline uintptr_t EmissiveTint = 0x1CC;
        inline uintptr_t MetalnessMap = 0x110;
        inline uintptr_t NormalMap = 0x140;
        inline uintptr_t RoughnessMap = 0x170;
    }

    namespace TString {
        inline uintptr_t hash = 0x10;
        inline uintptr_t len = 0x14;
        inline uintptr_t TypeTag = 0x01;
        inline uintptr_t Data = 0x18;
    }

    namespace luau {
        inline uintptr_t TStringTypeTag = 0x01;
        inline uintptr_t TStringHash = 0x10;
        inline uintptr_t TStringLength = 0x14;
        inline uintptr_t TStringData = 0x18;
        inline uintptr_t TValueValue = 0x00;
        inline uintptr_t TValueTag = 0x0c;
        inline uintptr_t NodeKey = 0x10;
        inline std::uint32_t TagNumber = 3;
        inline std::uint32_t TagString = 6;
    }

    namespace TaskScheduler {
        inline uintptr_t JobEnd = 0xD0;
        inline uintptr_t JobName = 0x18;
        inline uintptr_t JobStart = 0xC8;
        inline uintptr_t JobTypeName = 0xF8;
        inline uintptr_t MaxFPS = 0xB0;
        inline uintptr_t Pointer = 0x8C9D698;
        inline uintptr_t ScriptContext = 0x1B8;
    }

    namespace Team {
        inline uintptr_t BrickColor = 0xA0;
        inline uintptr_t TeamColor = 0xA0;
    }

    namespace TechniqueArray {
        inline uintptr_t BeginOffset = 0x0;
        inline uintptr_t EndOffset = 0x8;
        inline uintptr_t EntryStride = 0x88;
    }

    namespace Terrain {
        inline uintptr_t GrassLength = 0x1D8;
        inline uintptr_t MaterialColors = 0x4A0;
        inline uintptr_t WaterColor = 0x1C8;
        inline uintptr_t WaterReflectance = 0x1E0;
        inline uintptr_t WaterTransparency = 0x1E4;
        inline uintptr_t WaterWaveSize = 0x1E8;
        inline uintptr_t WaterWaveSpeed = 0x1EC;
    }

    namespace TextButton {
        inline uintptr_t AutoButtonColor = 0x9E4;
        inline uintptr_t ContentText = 0xE20;
        inline uintptr_t Font = 0x1140;
        inline uintptr_t LineHeight = 0xF38;
        inline uintptr_t LocalizedText = 0xE20;
        inline uintptr_t MaxVisibleGraphemes = 0x1154;
        inline uintptr_t Modal = 0x9E5;
        inline uintptr_t RichText = 0xE1D;
        inline uintptr_t Selected = 0x9E6;
        inline uintptr_t Text = 0xE20;
        inline uintptr_t TextColor3 = 0x1138;
        inline uintptr_t TextDirection = 0xFD8;
        inline uintptr_t TextScaled = 0xE09;
        inline uintptr_t TextSize = 0x115C;
        inline uintptr_t TextStrokeColor3 = 0x1144;
        inline uintptr_t TextStrokeTransparency = 0x1160;
        inline uintptr_t TextTransparency = 0x1164;
        inline uintptr_t TextTruncate = 0x1168;
        inline uintptr_t TextWrapped = 0x1030;
        inline uintptr_t TextXAlignment = 0x116C;
        inline uintptr_t TextYAlignment = 0xF80;
    }

    namespace TextLabel {
        inline uintptr_t ContentText = 0xBA0;
        inline uintptr_t Font = 0xEC0;
        inline uintptr_t LineHeight = 0xCB8;
        inline uintptr_t LocalizedText = 0xBA0;
        inline uintptr_t MaxVisibleGraphemes = 0xED4;
        inline uintptr_t RichText = 0xDB6;
        inline uintptr_t Text = 0xBA0;
        inline uintptr_t TextColor3 = 0xEB8;
        inline uintptr_t TextDirection = 0xD58;
        inline uintptr_t TextScaled = 0xDAE;
        inline uintptr_t TextSize = 0xEDC;
        inline uintptr_t TextStrokeColor3 = 0xEC4;
        inline uintptr_t TextStrokeTransparency = 0xEE0;
        inline uintptr_t TextTransparency = 0xEE4;
        inline uintptr_t TextTruncate = 0xEE8;
        inline uintptr_t TextWrapped = 0xDB0;
        inline uintptr_t TextXAlignment = 0xEEC;
        inline uintptr_t TextYAlignment = 0xD00;
    }

    namespace Textures {
        inline uintptr_t Decal_Texture = 0x1C8;
        inline uintptr_t Texture_Texture = 0x1C8;
    }

    namespace Tool {
        inline uintptr_t CanBeDropped = 0x4A0;
        inline uintptr_t Enabled = 0x4A1;
        inline uintptr_t Grip = 0x494;
        inline uintptr_t GripForward = 0x488;
        inline uintptr_t GripPos = 0x494;
        inline uintptr_t GripRight = 0x470;
        inline uintptr_t GripUp = 0x47C;
        inline uintptr_t ManualActivationOnly = 0x4A2;
        inline uintptr_t RequiresHandle = 0x4A3;
        inline uintptr_t TextureId = 0x348;
        inline uintptr_t Tooltip = 0x450;
    }

    namespace UnionOperation {
        inline uintptr_t AssetId = 0x2F8;
    }

    namespace UpVal {
        inline uintptr_t v = 0x18;
    }

    namespace UserInputService {
        inline uintptr_t WindowInputState = 0x288;
    }

    namespace Value {
        inline uintptr_t Value = 0xA0;
    }

    namespace VehicleSeat {
        inline uintptr_t MaxSpeed = 0x210;
        inline uintptr_t Occupant = 0x1F0;
        inline uintptr_t SteerFloat = 0x214;
        inline uintptr_t ThrottleFloat = 0x218;
        inline uintptr_t Torque = 0x21C;
        inline uintptr_t TurnSpeed = 0x220;
    }

    namespace VirtualTable {
        inline uintptr_t RaiseEventInvocation = 0x18;
    }

    namespace VisualEngine {
        inline uintptr_t Dimensions = 0xB10;
        inline uintptr_t FakeDataModel = 0xAF0;
        inline uintptr_t Pointer = 0x879A218;
        inline uintptr_t RenderView = 0xC40;
        inline uintptr_t ViewMatrix = 0x1B0;
    }

    namespace Weld {
        inline uintptr_t Part0 = 0x100;
        inline uintptr_t Part1 = 0x110;
    }

    namespace WeldConstraint {
        inline uintptr_t Part0 = 0xA0;
        inline uintptr_t Part1 = 0xB0;
    }

    namespace WindowInputState {
        inline uintptr_t CapsLock = 0x40;
        inline uintptr_t CurrentTextBox = 0x48;
    }

    namespace Functions {
        inline uintptr_t Raycast = 0x0;
    }

    namespace Workspace {
        inline uintptr_t CurrentCamera = 0x4A0;
        inline uintptr_t DistributedGameTime = 0x4C0;
        inline uintptr_t ReadOnlyGravity = 0x9C0;
        inline uintptr_t World = 0x3F8;
        inline uintptr_t Raycast = 0x0;
    }

    namespace World {
        inline uintptr_t AirProperties = 0x230;
        inline uintptr_t FallenPartsDestroyHeight = 0x20C;
        inline uintptr_t Gravity = 0x218;
        inline uintptr_t Primitives = 0x2A0;
        inline uintptr_t WorldSteps = 0x858;
        inline uintptr_t worldStepsPerSec = 0x858;
    }

    namespace WorldRoot {
        inline uintptr_t RaycastBoundDesc = 0x6DCCB80;
        inline uintptr_t RaycastBoundFn = 0x80;
        inline uintptr_t FindPartOnRayDescriptorRva = 0x0;
        inline uintptr_t FindPartOnRayWithIgnoreListDescriptorRva = 0x0;
        inline uintptr_t FindPartOnRayWithWhitelistDescriptorRva = 0x0;
        inline uintptr_t RaycastBoundFunctionOffset = 0x80;
        inline uintptr_t RaycastCompleteObjectLocatorRva = 0x0;
        inline uintptr_t RaycastDescriptorRva = 0x0;
        inline uintptr_t RaycastDescriptorVtableRva = 0x0;
        inline uintptr_t RaycastTypeDescriptorRva = 0x0;
    }

    namespace lua_State {
        inline uintptr_t base = 0x30;
        inline uintptr_t ci = 0x40;
        inline uintptr_t global = 0x28;
        inline uintptr_t gt = 0x78;
        inline uintptr_t stack = 0x20;
        inline uintptr_t status = 0x3;
        inline uintptr_t top = 0x18;
        inline uintptr_t userdata = 0x70;
    }

} // namespace offsets
