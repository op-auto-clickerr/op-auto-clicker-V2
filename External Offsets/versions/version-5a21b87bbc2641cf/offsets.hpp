/*
 * Dumped With: Valkz Dumper 1.0
 * Created by: Myst (2kr5x)
 * Github: https://github.com/2kr5x/Myst
 * Roblox Version: version-5a21b87bbc2641cf
 * Time Taken: 13007 ms (13.007000 seconds)
 * Total Offsets: 16042
 */

#pragma once
#include <cstdint>

// clang-format off
namespace offsets {
    inline constexpr const char* roblox_version = "version-5a21b87bbc2641cf";

    namespace Actor {
        inline constexpr uintptr_t ParallelFlag = 0x100;
    }

    namespace AirProperties {
        inline constexpr uintptr_t AirDensity = 0x18;
        inline constexpr uintptr_t GlobalWind = 0x3C;
    }

    namespace AnimationTrack {
        inline constexpr uintptr_t Animation = 0xA0;
        inline constexpr uintptr_t Animator = 0xF8;
        inline constexpr uintptr_t IsPlaying = 0xA10;
        inline constexpr uintptr_t Looped = 0xCD;
        inline constexpr uintptr_t Speed = 0xBC;
        inline constexpr uintptr_t TimePosition = 0xC0;
    }

    namespace Animator {
        inline constexpr uintptr_t ActiveAnimations = 0x900;
    }

    namespace Atmosphere {
        inline constexpr uintptr_t Color = 0xA0;
        inline constexpr uintptr_t Decay = 0xAC;
        inline constexpr uintptr_t Density = 0xB8;
        inline constexpr uintptr_t Glare = 0xBC;
        inline constexpr uintptr_t Haze = 0xC0;
        inline constexpr uintptr_t Offset = 0xC4;
    }

    namespace Attachment {
        inline constexpr uintptr_t Position = 0xAC;
    }

    namespace Attribute {
        inline constexpr uintptr_t Key = 0x0;
        inline constexpr uintptr_t Size = 0x58;
        inline constexpr uintptr_t Value = 0x18;
    }

    namespace BasePart {
        inline constexpr uintptr_t CastShadow = 0x120;
        inline constexpr uintptr_t Color3 = 0x11C;
        inline constexpr uintptr_t Locked = 0x121;
        inline constexpr uintptr_t Massless = 0x122;
        inline constexpr uintptr_t Overlap = 0x1F8;
        inline constexpr uintptr_t Primitive = 0x178;
        inline constexpr uintptr_t Reflectance = 0xF4;
        inline constexpr uintptr_t Shape = 0x1A0;
        inline constexpr uintptr_t Transparency = 0x118;
    }

    namespace Beam {
        inline constexpr uintptr_t Attachment0 = 0x148;
        inline constexpr uintptr_t Attachment1 = 0x158;
        inline constexpr uintptr_t Brightness = 0x168;
        inline constexpr uintptr_t CurveSize0 = 0x16C;
        inline constexpr uintptr_t CurveSize1 = 0x170;
        inline constexpr uintptr_t LightEmission = 0x174;
        inline constexpr uintptr_t LightInfluence = 0x178;
        inline constexpr uintptr_t Texture = 0x128;
        inline constexpr uintptr_t TextureLength = 0x184;
        inline constexpr uintptr_t TextureSpeed = 0x18C;
        inline constexpr uintptr_t Width0 = 0x190;
        inline constexpr uintptr_t Width1 = 0x194;
        inline constexpr uintptr_t ZOffset = 0x198;
    }

    namespace BloomEffect {
        inline constexpr uintptr_t Enabled = 0x98;
        inline constexpr uintptr_t Intensity = 0xA0;
        inline constexpr uintptr_t Size = 0xA4;
        inline constexpr uintptr_t Threshold = 0xA8;
    }

    namespace BlurEffect {
        inline constexpr uintptr_t Enabled = 0x98;
        inline constexpr uintptr_t Size = 0xA0;
    }

    namespace ByteCode {
        inline constexpr uintptr_t Pointer = 0x10;
        inline constexpr uintptr_t Size = 0x28;
    }

    namespace CachedItem {
        inline constexpr uintptr_t FileMeshData = 0x40;
    }

    namespace CallInfo {
        inline constexpr uintptr_t base = 0x10;
        inline constexpr uintptr_t func = 0x18;
        inline constexpr uintptr_t nresults = 0x28;
        inline constexpr uintptr_t top = 0x0;
    }

    namespace Callback {
        inline constexpr uintptr_t DataModelAdjustment = 0x1C8;
        inline constexpr uintptr_t Reference = 0x34;
        inline constexpr uintptr_t WeakChain = 0x38;
        inline constexpr uintptr_t WeakObject = 0x18;
        inline constexpr uintptr_t Wrapper = 0x18;
    }

    namespace Camera {
        inline constexpr uintptr_t CFrame = 0xC0;
        inline constexpr uintptr_t CameraSubject = 0xB0;
        inline constexpr uintptr_t CameraType = 0x120;
        inline constexpr uintptr_t FieldOfView = 0x128;
        inline constexpr uintptr_t ImagePlaneDepth = 0x2D4;
        inline constexpr uintptr_t Position = 0xE4;
        inline constexpr uintptr_t Rotation = 0xC0;
        inline constexpr uintptr_t Viewport = 0x284;
        inline constexpr uintptr_t ViewportInt16 = 0x284;
        inline constexpr uintptr_t ViewportSize = 0x2C4;
    }

    namespace Capabilities {
        inline constexpr uintptr_t Capabilities = 0x28;
    }

    namespace CharacterMesh {
        inline constexpr uintptr_t BaseTextureId = 0xB0;
        inline constexpr uintptr_t BodyPart = 0x130;
        inline constexpr uintptr_t MeshId = 0xE0;
        inline constexpr uintptr_t OverlayTextureId = 0x110;
    }

    namespace ClassDescriptor {
        inline constexpr uintptr_t CallbackEntryKindValue = 0x4;
        inline constexpr uintptr_t PropertyEntryDescriptor = 0x0;
        inline constexpr uintptr_t PropertyEntryKind = 0x8;
        inline constexpr uintptr_t PropertyMap = 0x250;
        inline constexpr uintptr_t PropertyMapBucketIndices = 0x2C8;
        inline constexpr uintptr_t PropertyMapBucketMask = 0x2D8;
        inline constexpr uintptr_t PropertyMapEntries = 0x2D0;
        inline constexpr uintptr_t PropertyMapEntrySize = 0x10;
    }

    namespace ClickDetector {
        inline constexpr uintptr_t MaxActivationDistance = 0xD0;
        inline constexpr uintptr_t MouseIcon = 0xB0;
    }

    namespace Closure {
        inline constexpr uintptr_t f = 0x28;
        inline constexpr uintptr_t isC = 0x3;
        inline constexpr uintptr_t nupvalues = 0x4;
        inline constexpr uintptr_t stacksize = 0x5;
    }

    namespace Clothing {
        inline constexpr uintptr_t Color3 = 0x108;
        inline constexpr uintptr_t Template = 0xE8;
    }

    namespace ColorCorrectionEffect {
        inline constexpr uintptr_t Brightness = 0xAC;
        inline constexpr uintptr_t Contrast = 0xB0;
        inline constexpr uintptr_t Enabled = 0x98;
        inline constexpr uintptr_t TintColor = 0xA0;
    }

    namespace ColorGradingEffect {
        inline constexpr uintptr_t Enabled = 0x98;
        inline constexpr uintptr_t TonemapperPreset = 0xA0;
    }

    namespace DataModel {
        inline constexpr uintptr_t CreatorId = 0x170;
        inline constexpr uintptr_t GameId = 0x178;
        inline constexpr uintptr_t GameLoaded = 0x588;
        inline constexpr uintptr_t GameLoaded = 0x588;
        inline constexpr uintptr_t JobId = 0x108;
        inline constexpr uintptr_t PlaceId = 0x180;
        inline constexpr uintptr_t PlaceVersion = 0x19C;
        inline constexpr uintptr_t PrimitiveCount = 0x410;
        inline constexpr uintptr_t ScriptContext = 0x440;
        inline constexpr uintptr_t ServerIP = 0x570;
        inline constexpr uintptr_t ToRenderView1 = 0x1B8;
        inline constexpr uintptr_t ToRenderView2 = 0x8;
        inline constexpr uintptr_t ToRenderView3 = 0x28;
        inline constexpr uintptr_t Workspace = 0x148;
    }

    namespace DepthOfFieldEffect {
        inline constexpr uintptr_t Enabled = 0x98;
        inline constexpr uintptr_t FarIntensity = 0xA0;
        inline constexpr uintptr_t FocusDistance = 0xA4;
        inline constexpr uintptr_t InFocusRadius = 0xA8;
        inline constexpr uintptr_t NearIntensity = 0xAC;
    }

    namespace DragDetector {
        inline constexpr uintptr_t ActivatedCursorIcon = 0x188;
        inline constexpr uintptr_t CursorIcon = 0xB0;
        inline constexpr uintptr_t MaxActivationDistance = 0xD0;
        inline constexpr uintptr_t MaxDragAngle = 0x270;
        inline constexpr uintptr_t MaxDragTranslation = 0x234;
        inline constexpr uintptr_t MaxForce = 0x274;
        inline constexpr uintptr_t MaxTorque = 0x278;
        inline constexpr uintptr_t MinDragAngle = 0x27C;
        inline constexpr uintptr_t MinDragTranslation = 0x240;
        inline constexpr uintptr_t ReferenceInstance = 0x1B8;
        inline constexpr uintptr_t Responsiveness = 0x288;
    }

    namespace EventDescriptor {
        inline constexpr uintptr_t Signature = 0x48;
    }

    namespace EventInstance {
        inline constexpr uintptr_t Descriptor = 0x0;
        inline constexpr uintptr_t OwningInstance = 0x18;
    }

    namespace EventSignature {
        inline constexpr uintptr_t FirstParameter = 0x8;
        inline constexpr uintptr_t MaximumParameters = 0x80;
        inline constexpr uintptr_t ParameterStride = 0x70;
    }

    namespace ExtraSpace {
        inline constexpr uintptr_t IsCoreScript = 0x168;
        inline constexpr uintptr_t RequireBypass = 0xA00;
        inline constexpr uintptr_t RequireBypass = 0xA00;
        inline constexpr uintptr_t ScriptContextToResume = 0x8E0;
        inline constexpr uintptr_t ScriptContextToState = 0x300;
        inline constexpr uintptr_t WaitingHybridScriptsJobScriptContext = 0x1C0;
    }

    namespace FakeDataModel {
        inline constexpr uintptr_t Pointer = 0x8D06600;
        inline constexpr uintptr_t RealDataModel = 0x1F8;
    }

    namespace FastClusterEntity {
        inline constexpr uintptr_t AlphaByte = 0x14;
        inline constexpr uintptr_t BBoxMaxX = 0xA4;
        inline constexpr uintptr_t BBoxMaxY = 0xA8;
        inline constexpr uintptr_t BBoxMaxZ = 0xAC;
        inline constexpr uintptr_t BBoxMinX = 0x98;
        inline constexpr uintptr_t BBoxMinY = 0x9C;
        inline constexpr uintptr_t BBoxMinZ = 0xA0;
        inline constexpr uintptr_t ContextPtr = 0x8;
        inline constexpr uintptr_t DecalMaterialPtr = 0x48;
        inline constexpr uintptr_t MaterialPtr = 0x20;
        inline constexpr uintptr_t PrimitiveIndexArrayPtr = 0x80;
        inline constexpr uintptr_t PrimitivePoolPtr = 0x1A0;
        inline constexpr uintptr_t RenderQueueId = 0x10;
        inline constexpr uintptr_t TechniqueArrayPtr = 0x70;
        inline constexpr uintptr_t VTableRva = 0x0;
    }

    namespace FileMeshData {
        inline constexpr uintptr_t AABBMax = 0x18C;
        inline constexpr uintptr_t AABBMin = 0x180;
        inline constexpr uintptr_t Faces = 0x30;
        inline constexpr uintptr_t FacesEnd = 0x38;
        inline constexpr uintptr_t Vertices = 0x0;
        inline constexpr uintptr_t VerticesEnd = 0x8;
    }

    namespace GarbageCollector {
        inline constexpr uintptr_t L_global = 0x28;
        inline constexpr uintptr_t g_freepages = 0x68;
        inline constexpr uintptr_t g_gcstate = 0x39;
        inline constexpr uintptr_t g_gcstepmul = 0x44;
        inline constexpr uintptr_t g_gcstepsize = 0x40;
        inline constexpr uintptr_t g_gray = 0x20;
        inline constexpr uintptr_t g_grayagain = 0x18;
        inline constexpr uintptr_t g_weak = 0x10;
    }

    namespace GuiBase2D {
        inline constexpr uintptr_t AbsolutePosition = 0xF4;
        inline constexpr uintptr_t AbsoluteRotation = 0xD0;
        inline constexpr uintptr_t AbsoluteSize = 0xFC;
    }

    namespace GuiObject {
        inline constexpr uintptr_t Active = 0x570;
        inline constexpr uintptr_t AnchorPoint = 0x520;
        inline constexpr uintptr_t AutomaticSize = 0x528;
        inline constexpr uintptr_t BackgroundColor3 = 0x508;
        inline constexpr uintptr_t BackgroundTransparency = 0x514;
        inline constexpr uintptr_t BorderColor3 = 0x514;
        inline constexpr uintptr_t BorderMode = 0x530;
        inline constexpr uintptr_t BorderSizePixel = 0x534;
        inline constexpr uintptr_t ClipsDescendants = 0x571;
        inline constexpr uintptr_t GuiState = 0x540;
        inline constexpr uintptr_t Image = 0x9A8;
        inline constexpr uintptr_t Interactable = 0x573;
        inline constexpr uintptr_t LayoutOrder = 0x544;
        inline constexpr uintptr_t Position = 0x4D8;
        inline constexpr uintptr_t RichText = 0xBA0;
        inline constexpr uintptr_t Rotation = 0xD0;
        inline constexpr uintptr_t ScreenGui_Enabled = 0x48C;
        inline constexpr uintptr_t Selectable = 0x574;
        inline constexpr uintptr_t SelectionOrder = 0x560;
        inline constexpr uintptr_t Size = 0x4F8;
        inline constexpr uintptr_t SizeConstraint = 0x568;
        inline constexpr uintptr_t Text = 0xE08;
        inline constexpr uintptr_t TextColor3 = 0xEB8;
        inline constexpr uintptr_t Visible = 0x575;
        inline constexpr uintptr_t ZIndex = 0x56C;
    }

    namespace Humanoid {
        inline constexpr uintptr_t AutoJumpEnabled = 0x1C8;
        inline constexpr uintptr_t AutoRotate = 0x1C9;
        inline constexpr uintptr_t AutomaticScalingEnabled = 0x1CA;
        inline constexpr uintptr_t BreakJointsOnDeath = 0x1CC;
        inline constexpr uintptr_t CameraOffset = 0x110;
        inline constexpr uintptr_t DisplayDistanceType = 0x174;
        inline constexpr uintptr_t DisplayName = 0xA0;
        inline constexpr uintptr_t EvaluateStateMachine = 0x1CD;
        inline constexpr uintptr_t FloorMaterial = 0x178;
        inline constexpr uintptr_t Health = 0x184;
        inline constexpr uintptr_t HealthDisplayDistance = 0x17C;
        inline constexpr uintptr_t HealthDisplayType = 0x180;
        inline constexpr uintptr_t HipHeight = 0x188;
        inline constexpr uintptr_t HumanoidRootPart = 0x460;
        inline constexpr uintptr_t HumanoidState = 0x8A8;
        inline constexpr uintptr_t HumanoidStateID = 0x20;
        inline constexpr uintptr_t IsWalking = 0xA2E;
        inline constexpr uintptr_t Jump = 0x1D0;
        inline constexpr uintptr_t JumpHeight = 0x194;
        inline constexpr uintptr_t JumpPower = 0x198;
        inline constexpr uintptr_t MaxHealth = 0x19C;
        inline constexpr uintptr_t MaxSlopeAngle = 0x1A0;
        inline constexpr uintptr_t MoveDirection = 0x134;
        inline constexpr uintptr_t MoveToPart = 0x100;
        inline constexpr uintptr_t MoveToPoint = 0x158;
        inline constexpr uintptr_t NameDisplayDistance = 0x1A4;
        inline constexpr uintptr_t NameOcclusion = 0x1A8;
        inline constexpr uintptr_t PlatformStand = 0x1D2;
        inline constexpr uintptr_t PlatformStatePointer = 0x4746A4F8;
        inline constexpr uintptr_t RequiresNeck = 0x1D3;
        inline constexpr uintptr_t RigType = 0x1B4;
        inline constexpr uintptr_t SeatPart = 0xF0;
        inline constexpr uintptr_t Sit = 0x1D3;
        inline constexpr uintptr_t TargetPoint = 0x140;
        inline constexpr uintptr_t UseJumpPower = 0x1D6;
        inline constexpr uintptr_t WalkSpeed = 0x1C4;
        inline constexpr uintptr_t WalkSpeedCheck = 0x39C;
        inline constexpr uintptr_t WalkTimer = 0x0;
        inline constexpr uintptr_t WalkToPoint = 0x158;
        inline constexpr uintptr_t Walkspeed = 0x1C4;
        inline constexpr uintptr_t WalkspeedCheck = 0x39C;
    }

    namespace Hyperion {
        inline constexpr uintptr_t Bitmap = 0x1473148;
        inline constexpr uintptr_t InstrumentationCallback = 0x800400;
    }

    namespace InputObject {
        inline constexpr uintptr_t MousePosition = 0xD4;
    }

    namespace Instance {
        inline constexpr uintptr_t Children = 0x78;
        inline constexpr uintptr_t ChildrenEnd = 0x8;
        inline constexpr uintptr_t ChildrenStart = 0x70;
        inline constexpr uintptr_t ClassBase = 0x1B0;
        inline constexpr uintptr_t ClassDescriptor = 0x18;
        inline constexpr uintptr_t ClassName = 0x8;
        inline constexpr uintptr_t ComponentMap = 0x38;
        inline constexpr uintptr_t Name = 0x0;
        inline constexpr uintptr_t NameContainer = 0x68;
        inline constexpr uintptr_t Parent = 0x60;
        inline constexpr uintptr_t This = 0x8;
    }

    namespace LRUHolder {
        inline constexpr uintptr_t MemEnforcedLRUCache = 0x20;
    }

    namespace LRUNode {
        inline constexpr uintptr_t AssetID = 0x10;
        inline constexpr uintptr_t CachedItem = 0x40;
        inline constexpr uintptr_t Next = 0x0;
    }

    namespace Lighting {
        inline constexpr uintptr_t Ambient = 0xB8;
        inline constexpr uintptr_t Atmosphere = 0x1C0;
        inline constexpr uintptr_t Brightness = 0x100;
        inline constexpr uintptr_t ClockTime = 0xB0;
        inline constexpr uintptr_t ColorShift_Bottom = 0xD0;
        inline constexpr uintptr_t ColorShift_Top = 0xC4;
        inline constexpr uintptr_t EnvironmentDiffuseScale = 0x104;
        inline constexpr uintptr_t EnvironmentSpecularScale = 0x108;
        inline constexpr uintptr_t ExposureCompensation = 0x10C;
        inline constexpr uintptr_t FogColor = 0xDC;
        inline constexpr uintptr_t FogEnd = 0x114;
        inline constexpr uintptr_t FogStart = 0x118;
        inline constexpr uintptr_t GeographicLatitude = 0x11C;
        inline constexpr uintptr_t GlobalShadows = 0x12C;
        inline constexpr uintptr_t GradientBottom = 0x178;
        inline constexpr uintptr_t GradientTop = 0x138;
        inline constexpr uintptr_t LightColor = 0x144;
        inline constexpr uintptr_t LightDirection = 0x150;
        inline constexpr uintptr_t MoonPosition = 0x16C;
        inline constexpr uintptr_t OutdoorAmbient = 0xE8;
        inline constexpr uintptr_t ShadowSoftness = 0x124;
        inline constexpr uintptr_t Sky = 0x1B0;
        inline constexpr uintptr_t Source = 0x15C;
        inline constexpr uintptr_t SunPosition = 0x160;
    }

    namespace LightingParameters {
        inline constexpr uintptr_t GeographicLatitude = 0x11C;
        inline constexpr uintptr_t LightColor = 0x144;
        inline constexpr uintptr_t LightDirection = 0x150;
        inline constexpr uintptr_t SkyAmbient = 0x138;
        inline constexpr uintptr_t SkyAmbient2 = 0x120;
        inline constexpr uintptr_t Source = 0x15C;
        inline constexpr uintptr_t TrueMoonPosition = 0x16C;
        inline constexpr uintptr_t TrueSunPosition = 0x160;
    }

    namespace LocalScript {
        inline constexpr uintptr_t ByteCode = 0x0;
        inline constexpr uintptr_t Bytecode = 0x190;
        inline constexpr uintptr_t GUID = 0xB8;
        inline constexpr uintptr_t Hash = 0x188;
    }

    namespace MaterialColors {
        inline constexpr uintptr_t Asphalt = 0x30;
        inline constexpr uintptr_t Basalt = 0x27;
        inline constexpr uintptr_t Brick = 0xF;
        inline constexpr uintptr_t Cobblestone = 0x33;
        inline constexpr uintptr_t Concrete = 0xC;
        inline constexpr uintptr_t CrackedLava = 0x2D;
        inline constexpr uintptr_t Glacier = 0x1B;
        inline constexpr uintptr_t Grass = 0x6;
        inline constexpr uintptr_t Ground = 0x2A;
        inline constexpr uintptr_t Ice = 0x36;
        inline constexpr uintptr_t LeafyGrass = 0x39;
        inline constexpr uintptr_t Limestone = 0x3F;
        inline constexpr uintptr_t Mud = 0x24;
        inline constexpr uintptr_t Pavement = 0x42;
        inline constexpr uintptr_t Rock = 0x18;
        inline constexpr uintptr_t Salt = 0x3C;
        inline constexpr uintptr_t Sand = 0x12;
        inline constexpr uintptr_t Sandstone = 0x21;
        inline constexpr uintptr_t Slate = 0x9;
        inline constexpr uintptr_t Snow = 0x1E;
        inline constexpr uintptr_t WoodPlanks = 0x15;
    }

    namespace MaterialLayer {
        inline constexpr uintptr_t ColorData = 0x24;
        inline constexpr uintptr_t FillModeByte = 0x11;
        inline constexpr uintptr_t Flags2 = 0x20;
        inline constexpr uintptr_t MatFlags = 0x18;
        inline constexpr uintptr_t Param = 0x1C;
        inline constexpr uintptr_t Stride = 0x88;
    }

    namespace MemEnforcedLRUCache {
        inline constexpr uintptr_t Head = 0x8;
    }

    namespace MeshContentProvider {
        inline constexpr uintptr_t LRUHolder = 0xC0;
    }

    namespace MeshPart {
        inline constexpr uintptr_t MeshId = 0x2F8;
        inline constexpr uintptr_t Texture = 0x328;
        inline constexpr uintptr_t TextureId = 0x328;
    }

    namespace Misc {
        inline constexpr uintptr_t Adornee = 0xD8;
        inline constexpr uintptr_t AnimationId = 0xA8;
        inline constexpr uintptr_t StringLength = 0x10;
        inline constexpr uintptr_t Value = 0xA0;
    }

    namespace Model {
        inline constexpr uintptr_t PrimaryPart = 0x240;
        inline constexpr uintptr_t Scale = 0x12C;
    }

    namespace ModuleScript {
        inline constexpr uintptr_t ByteCode = 0x0;
        inline constexpr uintptr_t Bytecode = 0x138;
        inline constexpr uintptr_t GUID = 0xB8;
        inline constexpr uintptr_t Hash = 0x348;
        inline constexpr uintptr_t IsCoreScript = 0x0;
    }

    namespace MouseService {
        inline constexpr uintptr_t InputObject = 0xE8;
        inline constexpr uintptr_t InputObject2 = 0xF8;
        inline constexpr uintptr_t MousePosition = 0xBC;
        inline constexpr uintptr_t SensitivityPointer = 0x0;
    }

    namespace ParticleEmitter {
        inline constexpr uintptr_t Acceleration = 0x1C8;
        inline constexpr uintptr_t Brightness = 0x204;
        inline constexpr uintptr_t Drag = 0x208;
        inline constexpr uintptr_t Lifetime = 0x1DC;
        inline constexpr uintptr_t LightEmission = 0x220;
        inline constexpr uintptr_t LightInfluence = 0x224;
        inline constexpr uintptr_t Rate = 0x230;
        inline constexpr uintptr_t RotSpeed = 0x1E4;
        inline constexpr uintptr_t Rotation = 0x1EC;
        inline constexpr uintptr_t Speed = 0x1F4;
        inline constexpr uintptr_t SpreadAngle = 0x1FC;
        inline constexpr uintptr_t Texture = 0x1A8;
        inline constexpr uintptr_t TimeScale = 0x244;
        inline constexpr uintptr_t VelocityInheritance = 0x248;
        inline constexpr uintptr_t ZOffset = 0x24C;
    }

    namespace Player {
        inline constexpr uintptr_t AccountAge = 0x364;
        inline constexpr uintptr_t CameraMode = 0x378;
        inline constexpr uintptr_t Character = 0x2A0;
        inline constexpr uintptr_t DisplayName = 0x120;
        inline constexpr uintptr_t HealthDisplayDistance = 0x3A0;
        inline constexpr uintptr_t LocalPlayer = 0x118;
        inline constexpr uintptr_t LocaleId = 0x100;
        inline constexpr uintptr_t MaxZoomDistance = 0x370;
        inline constexpr uintptr_t MinZoomDistance = 0x374;
        inline constexpr uintptr_t ModelInstance = 0x2A0;
        inline constexpr uintptr_t Mouse = 0x11F0;
        inline constexpr uintptr_t NameDisplayDistance = 0x3B0;
        inline constexpr uintptr_t Team = 0x2E0;
        inline constexpr uintptr_t TeamColor = 0x3B8;
        inline constexpr uintptr_t UserId = 0xB8;
    }

    namespace PlayerConfigurer {
        inline constexpr uintptr_t Pointer = 0x0;
    }

    namespace PlayerMouse {
        inline constexpr uintptr_t Icon = 0xB0;
        inline constexpr uintptr_t Workspace = 0x138;
    }

    namespace Players {
        inline constexpr uintptr_t LocalPlayer = 0x118;
    }

    namespace Primitive {
        inline constexpr uintptr_t AssemblyAngularVelocity = 0xEC;
        inline constexpr uintptr_t AssemblyLinearVelocity = 0xE0;
        inline constexpr uintptr_t CFrame = 0xB0;
        inline constexpr uintptr_t Flags = 0x1BE;
        inline constexpr uintptr_t Material = 0x24E;
        inline constexpr uintptr_t Orientation = 0xB0;
        inline constexpr uintptr_t Owner = 0x218;
        inline constexpr uintptr_t Position = 0xD4;
        inline constexpr uintptr_t PrimitiveFlags = 0x1BE;
        inline constexpr uintptr_t Rotation = 0xB0;
        inline constexpr uintptr_t Size = 0x1C4;
        inline constexpr uintptr_t Validate = 0x6;
    }

    namespace PrimitiveFlags {
        inline constexpr uintptr_t Anchored = 0x2;
        inline constexpr uintptr_t CanCollide = 0x8;
        inline constexpr uintptr_t CanQuery = 0x20;
        inline constexpr uintptr_t CanTouch = 0x10;
    }

    namespace PrimitivePool {
        inline constexpr uintptr_t ArrayBase = 0x20;
    }

    namespace PrimitiveRecord {
        inline constexpr uintptr_t Stride = 0x30;
        inline constexpr uintptr_t Translation = 0x24;
    }

    namespace Properties {
        inline constexpr uintptr_t ClassDescriptor = 0x18;
        inline constexpr uintptr_t ClassName = 0x8;
        inline constexpr uintptr_t Properties = 0x70;
        inline constexpr uintptr_t PropertyDescriptorBitFlags = 0x8C;
        inline constexpr uintptr_t TType = 0x68;
        inline constexpr uintptr_t TTypeNumber = 0x30;
    }

    namespace PropertyDescriptor {
        inline constexpr uintptr_t CallbackStorage = 0x80;
        inline constexpr uintptr_t ScriptableMask = 0x10;
        inline constexpr uintptr_t TypeDescriptor = 0x68;
    }

    namespace Proto {
        inline constexpr uintptr_t code = 0x30;
        inline constexpr uintptr_t k = 0x28;
        inline constexpr uintptr_t sizecode = 0x98;
        inline constexpr uintptr_t sizek = 0xA8;
    }

    namespace ProximityPrompt {
        inline constexpr uintptr_t ActionText = 0x98;
        inline constexpr uintptr_t Enabled = 0x11E;
        inline constexpr uintptr_t GamepadKeyCode = 0x104;
        inline constexpr uintptr_t HoldDuration = 0x108;
        inline constexpr uintptr_t KeyCode = 0x10C;
        inline constexpr uintptr_t KeyboardKeyCode = 0x10C;
        inline constexpr uintptr_t MaxActivationDistance = 0x110;
        inline constexpr uintptr_t ObjectText = 0xB8;
        inline constexpr uintptr_t RequiresLineOfSight = 0x11F;
    }

    namespace Render {
        inline constexpr uintptr_t RenderJobToView = 0x1D0;
        inline constexpr uintptr_t RenderViewToDevice = 0x8;
    }

    namespace RenderJob {
        inline constexpr uintptr_t FakeDataModel = 0x38;
        inline constexpr uintptr_t RealDataModel = 0x1F0;
        inline constexpr uintptr_t RenderView = 0x1E0;
    }

    namespace RenderQueue {
        inline constexpr uintptr_t AlwaysOnTop = 0xD;
        inline constexpr uintptr_t AlwaysOnTopAdorns = 0xE;
        inline constexpr uintptr_t Decals = 0x2;
        inline constexpr uintptr_t Glass = 0x8;
        inline constexpr uintptr_t GlassTint = 0x7;
        inline constexpr uintptr_t OnTopReadOnlyDepth = 0xC;
        inline constexpr uintptr_t OnTopWithDepth = 0xB;
        inline constexpr uintptr_t Opaque = 0x0;
        inline constexpr uintptr_t OpaqueAdorns = 0x4;
        inline constexpr uintptr_t OpaqueCasters = 0x3;
        inline constexpr uintptr_t OpaqueWithAlpha = 0x5;
        inline constexpr uintptr_t Screen = 0xF;
        inline constexpr uintptr_t ScreenOnTopOfBlur = 0x10;
        inline constexpr uintptr_t Terrain = 0x1;
        inline constexpr uintptr_t Transparent = 0x9;
        inline constexpr uintptr_t TransparentCasters = 0xA;
        inline constexpr uintptr_t Water = 0x6;
    }

    namespace RenderView {
        inline constexpr uintptr_t DeviceD3D11 = 0x0;
        inline constexpr uintptr_t LightingValid = 0x278;
        inline constexpr uintptr_t SkyValid = 0x0;
        inline constexpr uintptr_t SkyboxValid = 0x28D;
        inline constexpr uintptr_t VisualEngine = 0x0;
    }

    namespace RobloxExtraSpace {
        inline constexpr uintptr_t Capabilities = 0x70;
        inline constexpr uintptr_t Continuations = 0x98;
        inline constexpr uintptr_t Identity = 0x48;
        inline constexpr uintptr_t Script = 0x60;
        inline constexpr uintptr_t Shared = 0x18;
        inline constexpr uintptr_t capabilities = 0x68;
        inline constexpr uintptr_t identity = 0x48;
        inline constexpr uintptr_t script = 0x58;
    }

    namespace RunService {
        inline constexpr uintptr_t HeartbeatFPS = 0xF4;
        inline constexpr uintptr_t HeartbeatTask = 0x9668;
    }

    namespace Script {
        inline constexpr uintptr_t ByteCode = 0x0;
        inline constexpr uintptr_t GUID = 0xB8;
        inline constexpr uintptr_t Hash = 0x188;
    }

    namespace ScriptContext {
        inline constexpr uintptr_t LuaState = 0x300;
        inline constexpr uintptr_t RequireBypass = 0xA00;
        inline constexpr uintptr_t Resume = 0x8E0;
    }

    namespace Seat {
        inline constexpr uintptr_t Occupant = 0x200;
    }

    namespace SignalConnection {
        inline constexpr uintptr_t Enabled = 0x20;
        inline constexpr uintptr_t FunctionScriptSlotSelf = 0x18;
        inline constexpr uintptr_t Next = 0x10;
        inline constexpr uintptr_t SelfWeakFallback = 0x48;
        inline constexpr uintptr_t SlotWrapper = 0x30;
        inline constexpr uintptr_t SlotWrapperFallbackSelf = 0x40;
        inline constexpr uintptr_t SlotWrapperSelf = 0x38;
        inline constexpr uintptr_t WeakChain = 0x68;
        inline constexpr uintptr_t WeakChainNext = 0x8;
    }

    namespace SignalWeakObject {
        inline constexpr uintptr_t FunctionReference = 0x34;
        inline constexpr uintptr_t Pointer = 0x60;
        inline constexpr uintptr_t Thread = 0x28;
        inline constexpr uintptr_t WaitContainer = 0x50;
    }

    namespace Sky {
        inline constexpr uintptr_t MoonAngularSize = 0x22C;
        inline constexpr uintptr_t MoonTextureId = 0xB0;
        inline constexpr uintptr_t SkyboxBk = 0xE0;
        inline constexpr uintptr_t SkyboxDn = 0x110;
        inline constexpr uintptr_t SkyboxFt = 0x140;
        inline constexpr uintptr_t SkyboxLf = 0x170;
        inline constexpr uintptr_t SkyboxOrientation = 0x220;
        inline constexpr uintptr_t SkyboxRt = 0x1A0;
        inline constexpr uintptr_t SkyboxUp = 0x1D0;
        inline constexpr uintptr_t StarCount = 0x230;
        inline constexpr uintptr_t SunAngularSize = 0x224;
        inline constexpr uintptr_t SunTextureId = 0x200;
    }

    namespace Sound {
        inline constexpr uintptr_t IsPlaying = 0x128;
        inline constexpr uintptr_t Looped = 0x125;
        inline constexpr uintptr_t PlaybackSpeed = 0x104;
        inline constexpr uintptr_t RollOffMaxDistance = 0x108;
        inline constexpr uintptr_t RollOffMinDistance = 0x10C;
        inline constexpr uintptr_t SoundGroup = 0xD0;
        inline constexpr uintptr_t SoundId = 0xB0;
        inline constexpr uintptr_t Volume = 0x118;
    }

    namespace SpawnLocation {
        inline constexpr uintptr_t AllowTeamChangeOnTouch = 0x1D8;
        inline constexpr uintptr_t Enabled = 0x1D9;
        inline constexpr uintptr_t ForcefieldDuration = 0x1D0;
        inline constexpr uintptr_t Neutral = 0x1DA;
        inline constexpr uintptr_t TeamColor = 0x1D4;
    }

    namespace SpecialMesh {
        inline constexpr uintptr_t MeshId = 0xE0;
        inline constexpr uintptr_t Offset = 0xA0;
        inline constexpr uintptr_t Scale = 0xAC;
        inline constexpr uintptr_t TextureId = 0x110;
    }

    namespace StatsItem {
        inline constexpr uintptr_t Value = 0xB8;
    }

    namespace SunRaysEffect {
        inline constexpr uintptr_t Enabled = 0x98;
        inline constexpr uintptr_t Intensity = 0xA0;
        inline constexpr uintptr_t Spread = 0xA4;
    }

    namespace SurfaceAppearance {
        inline constexpr uintptr_t AlphaMode = 0x1D8;
        inline constexpr uintptr_t Color = 0x1C0;
        inline constexpr uintptr_t ColorMap = 0xB0;
        inline constexpr uintptr_t EmissiveMaskContent = 0xE0;
        inline constexpr uintptr_t EmissiveStrength = 0x1DC;
        inline constexpr uintptr_t EmissiveTint = 0x1CC;
        inline constexpr uintptr_t MetalnessMap = 0x110;
        inline constexpr uintptr_t NormalMap = 0x140;
        inline constexpr uintptr_t RoughnessMap = 0x170;
    }

    namespace TString {
        inline constexpr uintptr_t hash = 0x10;
        inline constexpr uintptr_t len = 0x14;
    }

    namespace TaskScheduler {
        inline constexpr uintptr_t JobEnd = 0xD0;
        inline constexpr uintptr_t JobName = 0x18;
        inline constexpr uintptr_t JobStart = 0xC8;
        inline constexpr uintptr_t JobTypeName = 0xF8;
        inline constexpr uintptr_t MaxFPS = 0xB0;
        inline constexpr uintptr_t Pointer = 0x8C9D698;
        inline constexpr uintptr_t ScriptContext = 0x1B8;
    }

    namespace Team {
        inline constexpr uintptr_t BrickColor = 0xA0;
        inline constexpr uintptr_t TeamColor = 0xA0;
    }

    namespace TechniqueArray {
        inline constexpr uintptr_t BeginOffset = 0x0;
        inline constexpr uintptr_t EndOffset = 0x8;
        inline constexpr uintptr_t EntryStride = 0x88;
    }

    namespace Terrain {
        inline constexpr uintptr_t GrassLength = 0x1D8;
        inline constexpr uintptr_t MaterialColors = 0x4A0;
        inline constexpr uintptr_t WaterColor = 0x1C8;
        inline constexpr uintptr_t WaterReflectance = 0x1E0;
        inline constexpr uintptr_t WaterTransparency = 0x1E4;
        inline constexpr uintptr_t WaterWaveSize = 0x1E8;
        inline constexpr uintptr_t WaterWaveSpeed = 0x1EC;
    }

    namespace TextButton {
        inline constexpr uintptr_t AutoButtonColor = 0x9E4;
        inline constexpr uintptr_t ContentText = 0xE20;
        inline constexpr uintptr_t Font = 0x1140;
        inline constexpr uintptr_t LineHeight = 0xF38;
        inline constexpr uintptr_t LocalizedText = 0xE20;
        inline constexpr uintptr_t MaxVisibleGraphemes = 0x1154;
        inline constexpr uintptr_t Modal = 0x9E5;
        inline constexpr uintptr_t RichText = 0xE1D;
        inline constexpr uintptr_t Selected = 0x9E6;
        inline constexpr uintptr_t Text = 0xE20;
        inline constexpr uintptr_t TextColor3 = 0x1138;
        inline constexpr uintptr_t TextDirection = 0xFD8;
        inline constexpr uintptr_t TextScaled = 0xE09;
        inline constexpr uintptr_t TextSize = 0x115C;
        inline constexpr uintptr_t TextStrokeColor3 = 0x1144;
        inline constexpr uintptr_t TextStrokeTransparency = 0x1160;
        inline constexpr uintptr_t TextTransparency = 0x1164;
        inline constexpr uintptr_t TextTruncate = 0x1168;
        inline constexpr uintptr_t TextWrapped = 0x1030;
        inline constexpr uintptr_t TextXAlignment = 0x116C;
        inline constexpr uintptr_t TextYAlignment = 0xF80;
    }

    namespace TextLabel {
        inline constexpr uintptr_t ContentText = 0xBA0;
        inline constexpr uintptr_t Font = 0xEC0;
        inline constexpr uintptr_t LineHeight = 0xCB8;
        inline constexpr uintptr_t LocalizedText = 0xBA0;
        inline constexpr uintptr_t MaxVisibleGraphemes = 0xED4;
        inline constexpr uintptr_t RichText = 0xDB6;
        inline constexpr uintptr_t Text = 0xBA0;
        inline constexpr uintptr_t TextColor3 = 0xEB8;
        inline constexpr uintptr_t TextDirection = 0xD58;
        inline constexpr uintptr_t TextScaled = 0xDAE;
        inline constexpr uintptr_t TextSize = 0xEDC;
        inline constexpr uintptr_t TextStrokeColor3 = 0xEC4;
        inline constexpr uintptr_t TextStrokeTransparency = 0xEE0;
        inline constexpr uintptr_t TextTransparency = 0xEE4;
        inline constexpr uintptr_t TextTruncate = 0xEE8;
        inline constexpr uintptr_t TextWrapped = 0xDB0;
        inline constexpr uintptr_t TextXAlignment = 0xEEC;
        inline constexpr uintptr_t TextYAlignment = 0xD00;
    }

    namespace Textures {
        inline constexpr uintptr_t Decal_Texture = 0x1C8;
        inline constexpr uintptr_t Texture_Texture = 0x1C8;
    }

    namespace Tool {
        inline constexpr uintptr_t CanBeDropped = 0x4A0;
        inline constexpr uintptr_t Enabled = 0x4A1;
        inline constexpr uintptr_t Grip = 0x494;
        inline constexpr uintptr_t GripForward = 0x488;
        inline constexpr uintptr_t GripPos = 0x494;
        inline constexpr uintptr_t GripRight = 0x470;
        inline constexpr uintptr_t GripUp = 0x47C;
        inline constexpr uintptr_t ManualActivationOnly = 0x4A2;
        inline constexpr uintptr_t RequiresHandle = 0x4A3;
        inline constexpr uintptr_t TextureId = 0x348;
        inline constexpr uintptr_t Tooltip = 0x450;
    }

    namespace UnionOperation {
        inline constexpr uintptr_t AssetId = 0x2F8;
    }

    namespace UpVal {
        inline constexpr uintptr_t v = 0x18;
    }

    namespace UserInputService {
        inline constexpr uintptr_t WindowInputState = 0x288;
    }

    namespace Value {
        inline constexpr uintptr_t Value = 0xA0;
    }

    namespace VehicleSeat {
        inline constexpr uintptr_t MaxSpeed = 0x210;
        inline constexpr uintptr_t Occupant = 0x1F0;
        inline constexpr uintptr_t SteerFloat = 0x214;
        inline constexpr uintptr_t ThrottleFloat = 0x218;
        inline constexpr uintptr_t Torque = 0x21C;
        inline constexpr uintptr_t TurnSpeed = 0x220;
    }

    namespace VirtualTable {
        inline constexpr uintptr_t RaiseEventInvocation = 0x18;
    }

    namespace VisualEngine {
        inline constexpr uintptr_t Dimensions = 0xB10;
        inline constexpr uintptr_t FakeDataModel = 0xAF0;
        inline constexpr uintptr_t Pointer = 0x879A218;
        inline constexpr uintptr_t RenderView = 0xC40;
        inline constexpr uintptr_t ViewMatrix = 0x1B0;
    }

    namespace Weld {
        inline constexpr uintptr_t Part0 = 0x100;
        inline constexpr uintptr_t Part1 = 0x110;
    }

    namespace WeldConstraint {
        inline constexpr uintptr_t Part0 = 0xA0;
        inline constexpr uintptr_t Part1 = 0xB0;
    }

    namespace WindowInputState {
        inline constexpr uintptr_t CapsLock = 0x40;
        inline constexpr uintptr_t CurrentTextBox = 0x48;
    }

    namespace Workspace {
        inline constexpr uintptr_t CurrentCamera = 0x4A0;
        inline constexpr uintptr_t DistributedGameTime = 0x4C0;
        inline constexpr uintptr_t ReadOnlyGravity = 0x9C0;
        inline constexpr uintptr_t World = 0x3F8;
    }

    namespace World {
        inline constexpr uintptr_t AirProperties = 0x230;
        inline constexpr uintptr_t FallenPartsDestroyHeight = 0x20C;
        inline constexpr uintptr_t Gravity = 0x218;
        inline constexpr uintptr_t Primitives = 0x2A0;
        inline constexpr uintptr_t WorldSteps = 0x858;
        inline constexpr uintptr_t worldStepsPerSec = 0x858;
    }

    namespace WorldRoot {
        inline constexpr uintptr_t RaycastBoundDesc = 0x6DCCB80;
        inline constexpr uintptr_t RaycastBoundFn = 0x80;
    }

    namespace lua_State {
        inline constexpr uintptr_t base = 0x30;
        inline constexpr uintptr_t ci = 0x40;
        inline constexpr uintptr_t global = 0x28;
        inline constexpr uintptr_t gt = 0x78;
        inline constexpr uintptr_t stack = 0x20;
        inline constexpr uintptr_t status = 0x3;
        inline constexpr uintptr_t top = 0x18;
        inline constexpr uintptr_t userdata = 0x70;
    }

} // namespace offsets
