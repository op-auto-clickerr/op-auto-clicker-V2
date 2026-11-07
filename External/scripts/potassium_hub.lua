--[[ ---------------------------------------------------------------
     Potassium Hub  ·  universal client-side suite (single file)
     Re-executing unloads the previous instance.
     Combat   : camlock, silent aim (namecall / mouse / ray spoof)
     Weapons  : rapid fire, infinite range, no spread, no recoil, fast reload
     Visuals  : ESP, skeleton, chams, FOV circle, fullbright, no fog
     Movement : speed, jump, fly, noclip, infinite jump, click teleport
     Extra    : game analyser, config save/load, player list, watermark
     --------------------------------------------------------------- ]]

local cloneref = cloneref or clonereference or function(o) return o end
local function svc(n) return cloneref(game:GetService(n)) end

local Players     = svc("Players")
local RS          = svc("ReplicatedStorage")
local WS          = svc("Workspace")
local UIS         = svc("UserInputService")
local RunSvc      = svc("RunService")
local Lighting    = svc("Lighting")
local TweenSvc    = svc("TweenService")
local VirtualUser = svc("VirtualUser")
local HttpSvc     = svc("HttpService")
local MarketSvc   = svc("MarketplaceService")

local LP     = Players.LocalPlayer
local Camera = WS.CurrentCamera
local Mouse  = LP:GetMouse()

local cclosure = newcclosure or function(f) return f end
local getNCM   = getnamecallmethod or function() return nil end
local isCaller = checkcaller or function() return false end
local gethui_  = gethui or function() return game:GetService("CoreGui") end
local function genv() return (getgenv and getgenv()) or _G end

if genv().PotassiumHubUnload then pcall(genv().PotassiumHubUnload) end

----------------------------------------------------------------- state
local S = {
    aimbot = { on = false, key = "MouseButton2", keyMode = "Hold", fov = 150, smooth = 6,
               part = "Head", predict = 0.10, wallcheck = false, teamcheck = true },
    silent = { on = false, fov = 250, part = "Head", predict = 0.10, wallcheck = false,
               teamcheck = true, hit = 100, namecall = true, mouse = true, rays = false },
    weapon = { rapid = false, burst = 4, range = false, spread = false, recoil = false, reload = false, ammo = true },
    vis    = { esp = false, box = true, name = true, health = true, dist = true, tracer = true,
               skeleton = true, chams = false, fovcircle = true, maxdist = 1200 },
    move   = { speed = 16, speedon = false, jump = 50, jumpon = false, fly = false, flyspeed = 60,
               noclip = false, infjump = false, tpclick = false, gravity = 196.2, gravon = false },
    misc   = { fullbright = false, nofog = false, antiafk = false, fpsboost = false,
               watermark = true, hitbox = 0, antikick = true },
    debug  = { logfire = false },
}
local engaged = false
local aimTarget = nil
local conns = {}
local refreshers = {}

local function track(c) table.insert(conns, c); return c end
local function addRefresh(f) table.insert(refreshers, f) end
local function refreshAll() for _, f in ipairs(refreshers) do pcall(f) end end

----------------------------------------------------------------- character helpers
local function char() local c = LP.Character; if c and c.Parent then return c end end
local function hum() local c = char(); if c then return c:FindFirstChildOfClass("Humanoid") end end
local function root() local c = char(); if c then return c:FindFirstChild("HumanoidRootPart") or c:FindFirstChild("Torso") end end
local function charOf(pl) local c = pl.Character; if c and c.Parent then return c end end
local function humOf(pl) local c = charOf(pl); if c then return c:FindFirstChildOfClass("Humanoid") end end
local function alive(pl) local h = humOf(pl); return h ~= nil and h.Health > 0 end

local PARTS = {
    Head  = {"Head", "UpperTorso", "HumanoidRootPart", "Torso"},
    Torso = {"UpperTorso", "HumanoidRootPart", "Torso", "Head"},
    Root  = {"HumanoidRootPart", "Torso", "UpperTorso", "Head"},
}

local function pickPart(pl, which)
    if not alive(pl) then return nil end
    local c = charOf(pl)
    if not c then return nil end
    if which == "Nearest" then
        local best, bd
        for _, p in ipairs(c:GetChildren()) do
            if p:IsA("BasePart") then
                local d = (p.Position - Camera.CFrame.Position).Magnitude
                if not bd or d < bd then best, bd = p, d end
            end
        end
        return best
    end
    for _, n in ipairs(PARTS[which] or PARTS.Head) do
        local p = c:FindFirstChild(n)
        if p and p:IsA("BasePart") then return p end
    end
end

local rayParams = RaycastParams.new()
pcall(function() rayParams.FilterType = Enum.RaycastFilterType.Exclude end)

local function visCheck(pos)
    local ignore = char()
    rayParams.FilterDescendantsInstances = ignore and {ignore} or {}
    local origin = Camera.CFrame.Position
    local dir = pos - origin
    local hit = WS:Raycast(origin, dir, rayParams)
    if not hit then return true end
    return (hit.Position - origin).Magnitude >= dir.Magnitude - 1.5
end

local function teamBlocked(pl, check)
    if not check then return false end
    if not pl.Team or not LP.Team then return false end
    return pl.Team == LP.Team
end

local function acquire(cfg)
    local vp = Camera.ViewportSize
    local cx, cy = vp.X * 0.5, vp.Y * 0.5
    local best, bestd
    for _, pl in ipairs(Players:GetPlayers()) do
        if pl ~= LP and not teamBlocked(pl, cfg.teamcheck) then
            local part = pickPart(pl, cfg.part)
            if part then
                local p = part.Position
                if cfg.predict and cfg.predict > 0 then
                    local ok, v = pcall(function() return part.AssemblyLinearVelocity end)
                    if ok and v then p = p + v * cfg.predict end
                end
                local sp, onscreen = Camera:WorldToViewportPoint(p)
                if onscreen and sp.Z > 0 then
                    local d = (Vector2.new(sp.X, sp.Y) - Vector2.new(cx, cy)).Magnitude
                    if d <= cfg.fov and (not bestd or d < bestd) then
                        if (not cfg.wallcheck) or visCheck(p) then
                            best = { pl = pl, part = part, pos = p }
                            bestd = d
                        end
                    end
                end
            end
        end
    end
    return best
end

----------------------------------------------------------------- ui shell
local gui = Instance.new("ScreenGui")
gui.Name = "PotassiumHub"
gui.ResetOnSpawn = false
gui.IgnoreGuiInset = true
gui.ZIndexBehavior = Enum.ZIndexBehavior.Sibling
if syn and syn.protect_gui then pcall(syn.protect_gui, gui) end
local okParent = pcall(function() gui.Parent = gethui_() end)
if not okParent or not gui.Parent then gui.Parent = LP:WaitForChild("PlayerGui") end

local ACCENT = Color3.fromRGB(120, 140, 255)
local BG     = Color3.fromRGB(18, 19, 24)
local BG2    = Color3.fromRGB(26, 28, 35)
local TXT    = Color3.fromRGB(226, 228, 236)
local DIM    = Color3.fromRGB(45, 48, 60)

local function corner(o, r)
    local c = Instance.new("UICorner"); c.CornerRadius = UDim.new(0, r or 6); c.Parent = o; return c
end
local function stroke(o, col, t)
    local s = Instance.new("UIStroke"); s.Color = col or DIM; s.Thickness = t or 1; s.Parent = o; return s
end
local function label(parent, text, size, col)
    local l = Instance.new("TextLabel", parent)
    l.BackgroundTransparency = 1
    l.Font = Enum.Font.Gotham
    l.TextSize = size or 12
    l.TextColor3 = col or TXT
    l.TextXAlignment = Enum.TextXAlignment.Left
    l.Text = text
    return l
end

local toastHolder = Instance.new("Frame")
toastHolder.Size = UDim2.new(0, 300, 1, 0)
toastHolder.Position = UDim2.new(1, -320, 0, 0)
toastHolder.BackgroundTransparency = 1
toastHolder.Parent = gui
local toastLayout = Instance.new("UIListLayout")
toastLayout.Padding = UDim.new(0, 6)
toastLayout.HorizontalAlignment = Enum.HorizontalAlignment.Right
toastLayout.VerticalAlignment = Enum.VerticalAlignment.Bottom
toastLayout.SortOrder = Enum.SortOrder.LayoutOrder
toastLayout.Parent = toastHolder

local function notify(msg, dur)
    local f = Instance.new("Frame")
    f.Size = UDim2.new(1, 0, 0, 28)
    f.BackgroundColor3 = BG2
    f.Parent = toastHolder
    corner(f, 6)
    stroke(f, ACCENT)
    local t = label(f, tostring(msg), 12)
    t.Size = UDim2.new(1, -16, 1, 0)
    t.Position = UDim2.new(0, 8, 0, 0)
    t.Font = Enum.Font.GothamMedium
    task.delay(dur or 3, function()
        if f.Parent then
            TweenSvc:Create(f, TweenInfo.new(0.25), { BackgroundTransparency = 1 }):Play()
            TweenSvc:Create(t, TweenInfo.new(0.25), { TextTransparency = 1 }):Play()
            task.wait(0.3)
            f:Destroy()
        end
    end)
end

----------------------------------------------------------------- esp
local drawEsp = Drawing ~= nil
local esp = {}

local function newDrawing(cls, props)
    local ok, d = pcall(function() return Drawing.new(cls) end)
    if not ok or not d then return nil end
    for k, v in pairs(props or {}) do pcall(function() d[k] = v end) end
    return d
end

local function espFor(pl)
    if esp[pl] then return esp[pl] end
    local e = {}
    e.box        = newDrawing("Square", { Thickness = 1, Filled = false, Color = ACCENT, Transparency = 1 })
    e.boxOutline = newDrawing("Square", { Thickness = 3, Filled = false, Color = Color3.new(0, 0, 0), Transparency = 0.5 })
    e.name       = newDrawing("Text",   { Size = 13, Center = true, Outline = true, Color = Color3.new(1, 1, 1) })
    e.dist       = newDrawing("Text",   { Size = 12, Center = true, Outline = true, Color = Color3.fromRGB(190, 195, 210) })
    e.healthBar  = newDrawing("Square", { Thickness = 1, Filled = true, Color = Color3.fromRGB(80, 230, 120) })
    e.healthBack = newDrawing("Square", { Thickness = 1, Filled = true, Color = Color3.fromRGB(30, 32, 40) })
    e.tracer     = newDrawing("Line",   { Thickness = 1, Color = ACCENT, Transparency = 0.7 })
    e.bones = {}
    for i = 1, 20 do
        e.bones[i] = newDrawing("Line", { Thickness = 1, Color = Color3.fromRGB(235, 235, 245), Transparency = 0.6 })
    end
    esp[pl] = e
    return e
end

local function hideEsp(e)
    local function kill(d)
        if d and d.Remove then pcall(function() d:Remove() end) end
    end
    for _, d in pairs(e) do
        if type(d) == "table" then
            for _, x in pairs(d) do kill(x) end
        else
            kill(d)
        end
    end
end

local BONES = {
    {"Head", "UpperTorso"}, {"UpperTorso", "LowerTorso"},
    {"UpperTorso", "LeftUpperArm"}, {"LeftUpperArm", "LeftLowerArm"}, {"LeftLowerArm", "LeftHand"},
    {"UpperTorso", "RightUpperArm"}, {"RightUpperArm", "RightLowerArm"}, {"RightLowerArm", "RightHand"},
    {"LowerTorso", "LeftUpperLeg"}, {"LeftUpperLeg", "LeftLowerLeg"}, {"LeftLowerLeg", "LeftFoot"},
    {"LowerTorso", "RightUpperLeg"}, {"RightUpperLeg", "RightLowerLeg"}, {"RightLowerLeg", "RightFoot"},
}
local R6BONES = {
    {"Head", "Torso"}, {"Torso", "Left Arm"}, {"Torso", "Right Arm"},
    {"Torso", "Left Leg"}, {"Torso", "Right Leg"},
}

local function hideAll(e)
    for _, k in ipairs({"box", "boxOutline", "name", "dist", "healthBar", "healthBack", "tracer"}) do
        local d = e[k]; if d then d.Visible = false end
    end
    for _, b in ipairs(e.bones) do b.Visible = false end
end

local function espStep()
    if not drawEsp then return end
    if not S.vis.esp then
        for pl, e in pairs(esp) do hideEsp(e); esp[pl] = nil end
        return
    end
    local vp = Camera.ViewportSize
    for _, pl in ipairs(Players:GetPlayers()) do
        local e = espFor(pl)
        local c = charOf(pl)
        local h = humOf(pl)
        local rp = c and (c:FindFirstChild("HumanoidRootPart") or c:FindFirstChild("Torso"))
        local show = c ~= nil and h ~= nil and h.Health > 0 and pl ~= LP and rp ~= nil
        if show and (rp.Position - Camera.CFrame.Position).Magnitude > S.vis.maxdist then show = false end
        if not show then
            hideAll(e)
        else
            local head = c:FindFirstChild("Head") or rp
            local top = head.Position + Vector3.new(0, head.Size.Y * 0.5 + 0.35, 0)
            local bot = rp.Position - Vector3.new(0, (h.HipHeight or 2) + 1, 0)
            local tl, ton = Camera:WorldToViewportPoint(top)
            local bl, bon = Camera:WorldToViewportPoint(bot)
            if ton and bon then
                local height = math.max(math.abs(bl.Y - tl.Y), 8)
                local width = height * 0.53
                local x = math.min(tl.X, bl.X) - width * 0.5
                local y = math.min(tl.Y, bl.Y)
                local col = Color3.fromRGB(120, 140, 255)
                if pl.Team and LP.Team then
                    col = (pl.Team == LP.Team) and Color3.fromRGB(90, 220, 120) or Color3.fromRGB(245, 105, 105)
                end
                if e.box then
                    e.box.Visible = S.vis.box
                    e.box.Size = Vector2.new(width, height)
                    e.box.Position = Vector2.new(x, y)
                    e.box.Color = col
                end
                if e.boxOutline then
                    e.boxOutline.Visible = S.vis.box
                    e.boxOutline.Size = Vector2.new(width, height)
                    e.boxOutline.Position = Vector2.new(x, y)
                end
                if e.name then
                    e.name.Visible = S.vis.name
                    e.name.Text = pl.Name
                    e.name.Position = Vector2.new(tl.X, y - 16)
                    e.name.Color = col
                end
                if e.dist then
                    e.dist.Visible = S.vis.dist
                    e.dist.Text = string.format("%d studs", math.floor((rp.Position - Camera.CFrame.Position).Magnitude))
                    e.dist.Position = Vector2.new(tl.X, y + height + 2)
                end
                if e.healthBar and e.healthBack then
                    local frac = math.clamp(h.Health / math.max(h.MaxHealth, 1), 0, 1)
                    e.healthBack.Visible = S.vis.health
                    e.healthBack.Size = Vector2.new(3, height)
                    e.healthBack.Position = Vector2.new(x - 6, y)
                    e.healthBar.Visible = S.vis.health
                    e.healthBar.Size = Vector2.new(3, height * frac)
                    e.healthBar.Position = Vector2.new(x - 6, y + height * (1 - frac))
                    e.healthBar.Color = Color3.fromRGB(80 + 160 * (1 - frac), 90 + 140 * frac, 110)
                end
                if e.tracer then
                    e.tracer.Visible = S.vis.tracer
                    e.tracer.From = Vector2.new(vp.X * 0.5, vp.Y)
                    e.tracer.To = Vector2.new(tl.X, y + height * 0.5)
                    e.tracer.Color = col
                end
                local list = c:FindFirstChild("UpperTorso") and BONES or R6BONES
                local i = 1
                for _, pair in ipairs(list) do
                    local a, b = c:FindFirstChild(pair[1]), c:FindFirstChild(pair[2])
                    local line = e.bones[i]
                    if line then
                        if a and b then
                            local p1, o1 = Camera:WorldToViewportPoint(a.Position)
                            local p2, o2 = Camera:WorldToViewportPoint(b.Position)
                            if o1 and o2 then
                                line.Visible = S.vis.skeleton
                                line.From = Vector2.new(p1.X, p1.Y)
                                line.To = Vector2.new(p2.X, p2.Y)
                            else
                                line.Visible = false
                            end
                        else
                            line.Visible = false
                        end
                    end
                    i = i + 1
                end
            end
        end
    end
    for pl, e in pairs(esp) do
        if not pl.Parent then hideEsp(e); esp[pl] = nil end
    end
end

local fovCircle = drawEsp and newDrawing("Circle", { Thickness = 1, Filled = false, Transparency = 0.8, NumSides = 64 }) or nil

local function fovStep()
    if not fovCircle then return end
    local cfg
    if S.silent.on then cfg = S.silent elseif S.aimbot.on then cfg = S.aimbot end
    if cfg and S.vis.fovcircle then
        local vp = Camera.ViewportSize
        fovCircle.Visible = true
        fovCircle.Color = ACCENT
        fovCircle.Position = Vector2.new(vp.X * 0.5, vp.Y * 0.5)
        fovCircle.Radius = cfg.fov
    else
        fovCircle.Visible = false
    end
end

----------------------------------------------------------------- silent aim hooks
local oldNamecall
local oldIndex
local rapidClock = 0

local fireMethods = {}
for _, n in ipairs({
    "Fire", "fire", "FireServer", "Shoot", "shoot", "ShootServer", "Attack", "attack",
    "FireWeapon", "Swing", "Melee", "Hit", "Cast", "CastRay", "Throw", "GunFire",
    "FireGun", "Primary", "Secondary", "FireGunServer", "ShootGun", "Damage",
}) do fireMethods[n] = true end

local function mouseWorld()
    if not oldNamecall then
        return Camera.CFrame.Position + Camera.CFrame.LookVector * 60
    end
    local ok, ray = pcall(function()
        local mp = UIS:GetMouseLocation()
        return oldNamecall(Camera, "ScreenPointToRay", mp.X, mp.Y)
    end)
    if ok and typeof(ray) == "Ray" then
        return ray.Origin + ray.Direction * 200
    end
    return Camera.CFrame.Position + Camera.CFrame.LookVector * 60
end

local function rewriteValue(v, tgt, mw)
    if typeof(v) == "Vector3" then
        if (v - mw).Magnitude <= 30 then return tgt end
    elseif typeof(v) == "CFrame" then
        if (v.Position - mw).Magnitude <= 30 then
            return CFrame.new(tgt) * (v - v.Position)
        end
    end
    return v
end

local function argSummary(args)
    local out = {}
    for i = 1, #args do
        local a = args[i]
        local t = typeof(a)
        if t == "Vector3" then
            out[#out + 1] = i .. ":V3(" .. string.format("%.0f,%.0f,%.0f", a.X, a.Y, a.Z) .. ")"
        elseif t == "CFrame" then
            local p = a.Position
            out[#out + 1] = i .. ":CF(" .. string.format("%.0f,%.0f,%.0f", p.X, p.Y, p.Z) .. ")"
        elseif t == "Instance" then
            out[#out + 1] = i .. ":inst " .. a.Name
        else
            out[#out + 1] = i .. ":" .. tostring(a)
        end
        if #out >= 8 then break end
    end
    return table.concat(out, "  ")
end

local function installNamecall()
    if not hookmetamethod then return false end
    oldNamecall = hookmetamethod(game, "__namecall", cclosure(function(self, ...)
        local method = getNCM()
        local tgt = S.silent.on and aimTarget and aimTarget.pos or nil
        if tgt and method and fireMethods[method] then
            local args = { ... }
            if S.debug.logfire then
                print("[KHUB-FIRE] " .. tostring(method) .. "  " .. argSummary(args))
            end
            if S.silent.namecall and math.random(1, 100) <= S.silent.hit then
                local mw = mouseWorld()
                for i = 1, #args do
                    local a = args[i]
                    if type(a) == "table" then
                        for k, v in pairs(a) do a[k] = rewriteValue(v, tgt, mw) end
                    else
                        args[i] = rewriteValue(a, tgt, mw)
                    end
                end
            end
            oldNamecall(self, table.unpack(args))
            if S.weapon.rapid and (os.clock() - rapidClock) > 0.02 then
                rapidClock = os.clock()
                local n = math.clamp(S.weapon.burst, 1, 15)
                for _ = 2, n do oldNamecall(self, table.unpack(args)) end
            end
            return
        end
        if tgt and S.silent.rays and self == Camera and
            (method == "ScreenPointToRay" or method == "ViewportPointToRay") then
            local o = Camera.CFrame.Position
            return Ray.new(o, tgt - o)
        end
        if tgt and S.silent.rays and self == WS and method == "Raycast" then
            local o, d = select(1, ...), select(2, ...)
            if typeof(o) == "Vector3" and typeof(d) == "Vector3" then
                local want = tgt - o
                if d.Unit:Dot(want.Unit) > 0.85 then
                    return oldNamecall(self, o, want, select(3, ...))
                end
            end
        end
        return oldNamecall(self, ...)
    end))
    return true
end

local function installMouseHook()
    if not hookmetamethod then return false end
    oldIndex = hookmetamethod(game, "__index", cclosure(function(self, key)
        if not isCaller() and S.silent.on and S.silent.mouse and aimTarget then
            local isMouse = false
            pcall(function() isMouse = (self == Mouse) or (self.ClassName == "Mouse") end)
            if isMouse then
                if key == "Hit" then return CFrame.new(aimTarget.pos) end
                if key == "Target" then return aimTarget.part end
                if key == "UnitRay" then
                    local o = Camera.CFrame.Position
                    return Ray.new(o, (aimTarget.pos - o).Unit)
                end
            end
        end
        return oldIndex(self, key)
    end))
    return true
end

----------------------------------------------------------------- weapon patches
local RAPID_RATE  = { "FireRate", "FireRateMultiplier", "AttackSpeed", "ShootRate", "RoundsPerMinute" }
local RAPID_DELAY = { "FireDelay", "FireDelayTime", "Cooldown", "CooldownTime", "ShootCooldown",
                      "FireInterval", "AttackCooldown", "ShootDelay", "ReloadTime", "ReloadDelay" }
local RANGE_PROPS = { "Range", "MaxRange", "MinRange", "MaxDistance", "Distance", "Reach",
                      "RangeValue", "FalloffDistance", "EffectiveRange" }
local SPREAD_PROPS = { "Spread", "SpreadAngle", "SpreadAngleBase", "Bloom", "Inaccuracy", "AimSpread",
                       "BulletSpread", "VerticalSpread", "HorizontalSpread" }
local RECOIL_PROPS = { "Recoil", "RecoilAmount", "CameraRecoil", "Shake", "Kickback", "RecoilPower" }

local function tuneValues(o)
    local okC, kids = pcall(function() return o:GetChildren() end)
    if not okC or not kids then return end
    for _, c in ipairs(kids) do
        if c:IsA("NumberValue") or c:IsA("IntValue") then
            local n = string.lower(c.Name)
            if S.weapon.rapid then
                if string.find(n, "rate") then
                    pcall(function() c.Value = 999 end)
                elseif string.find(n, "cooldown") or string.find(n, "delay") or string.find(n, "interval") then
                    pcall(function() c.Value = 0 end)
                end
            elseif S.weapon.reload and string.find(n, "reload") then
                pcall(function() c.Value = 0 end)
            end
            if S.weapon.range and (string.find(n, "range") or string.find(n, "distance") or string.find(n, "reach")) then
                pcall(function() c.Value = 100000 end)
            end
            if S.weapon.spread and (string.find(n, "spread") or string.find(n, "bloom") or string.find(n, "inacc")) then
                pcall(function() c.Value = 0 end)
            end
        end
    end
    if S.weapon.ammo then
        local ammo, maxa = nil, nil
        pcall(function()
            ammo = o:FindFirstChild("Ammo")
            maxa = o:FindFirstChild("MaxAmmo")
        end)
        if ammo and maxa and (ammo:IsA("IntValue") or ammo:IsA("NumberValue")) then
            pcall(function() ammo.Value = maxa.Value end)
        end
    end
end

local function setProp(obj, name, value)
    local ok, cur = pcall(function() return obj[name] end)
    if not ok or type(cur) ~= "number" then return end
    pcall(function() obj[name] = value end)
end

local function tuneObject(o)
    tuneValues(o)
    if S.weapon.rapid then
        for _, n in ipairs(RAPID_RATE) do setProp(o, n, 999) end
        for _, n in ipairs(RAPID_DELAY) do setProp(o, n, 0) end
    elseif S.weapon.reload then
        setProp(o, "ReloadTime", 0)
        setProp(o, "ReloadDelay", 0)
    end
    if S.weapon.range then
        for _, n in ipairs(RANGE_PROPS) do setProp(o, n, 100000) end
    end
    if S.weapon.spread then
        for _, n in ipairs(SPREAD_PROPS) do setProp(o, n, 0) end
    end
    if S.weapon.recoil then
        for _, n in ipairs(RECOIL_PROPS) do setProp(o, n, 0) end
    end
    local okA, attrs = pcall(function() return o:GetAttributes() end)
    if okA and type(attrs) == "table" then
        for k, v in pairs(attrs) do
            if type(v) == "number" then
                local lk = string.lower(k)
                if S.weapon.rapid then
                    if string.find(lk, "firerate") or string.find(lk, "shootrate") then
                        pcall(function() o:SetAttribute(k, 999) end)
                    elseif string.find(lk, "delay") or string.find(lk, "cooldown") or string.find(lk, "interval") then
                        pcall(function() o:SetAttribute(k, 0) end)
                    end
                end
                if S.weapon.range and (string.find(lk, "range") or string.find(lk, "distance") or string.find(lk, "reach")) then
                    pcall(function() o:SetAttribute(k, 100000) end)
                end
                if S.weapon.spread and (string.find(lk, "spread") or string.find(lk, "bloom") or string.find(lk, "inacc")) then
                    pcall(function() o:SetAttribute(k, 0) end)
                end
            end
        end
    end
end

local function weaponStep()
    if not (S.weapon.rapid or S.weapon.range or S.weapon.spread or S.weapon.recoil or S.weapon.reload) then return end
    local c = char()
    if not c then return end
    for _, o in ipairs(c:GetChildren()) do
        if o:IsA("Tool") then
            tuneObject(o)
            local ok, descs = pcall(function() return o:GetDescendants() end)
            if ok and descs then
                for _, d in ipairs(descs) do tuneObject(d) end
            end
        end
    end
end

----------------------------------------------------------------- movement
local flyBV, flyBG

local function flySet(on)
    if on then
        local rp = root()
        if not rp then return end
        if not flyBV then
            flyBV = Instance.new("BodyVelocity")
            flyBV.Name = "KHubFly"
            flyBV.MaxForce = Vector3.new(1e5, 1e5, 1e5)
            flyBV.Velocity = Vector3.zero
            flyBV.Parent = rp
        end
        if not flyBG then
            flyBG = Instance.new("BodyGyro")
            flyBG.Name = "KHubFlyGyro"
            flyBG.MaxTorque = Vector3.new(1e5, 1e5, 1e5)
            flyBG.P = 2500
            flyBG.Parent = rp
        end
    else
        if flyBV then pcall(function() flyBV:Destroy() end); flyBV = nil end
        if flyBG then pcall(function() flyBG:Destroy() end); flyBG = nil end
    end
end

local function movementStep(dt)
    local h = hum()
    if not h then
        flySet(false)
        return
    end
    if S.move.speedon and math.abs(h.WalkSpeed - S.move.speed) > 0.2 then
        h.WalkSpeed = S.move.speed
    end
    if S.move.jumpon then
        local ok = pcall(function()
            h.UseJumpPower = true
            h.JumpPower = S.move.jump
        end)
        if not ok then pcall(function() h.JumpHeight = S.move.jump / 4 end) end
    end
    if S.move.noclip then
        local c = char()
        if c then
            for _, p in ipairs(c:GetDescendants()) do
                if p:IsA("BasePart") and p.CanCollide then p.CanCollide = false end
            end
        end
    end
    if S.move.gravon then pcall(function() WS.Gravity = S.move.gravity end) end
    if S.move.fly and flyBV and flyBG then
        local dir = Vector3.zero
        local cf = Camera.CFrame
        if UIS:IsKeyDown(Enum.KeyCode.W) then dir = dir + cf.LookVector end
        if UIS:IsKeyDown(Enum.KeyCode.S) then dir = dir - cf.LookVector end
        if UIS:IsKeyDown(Enum.KeyCode.A) then dir = dir - cf.RightVector end
        if UIS:IsKeyDown(Enum.KeyCode.D) then dir = dir + cf.RightVector end
        if UIS:IsKeyDown(Enum.KeyCode.Space) then dir = dir + Vector3.new(0, 1, 0) end
        if UIS:IsKeyDown(Enum.KeyCode.LeftControl) then dir = dir - Vector3.new(0, 1, 0) end
        if dir.Magnitude > 0 then dir = dir.Unit end
        flyBV.Velocity = dir * S.move.flyspeed
        flyBG.CFrame = cf
    end
end

----------------------------------------------------------------- lighting / chams
local savedLight = {}

local function setLight(on)
    if on then
        if not savedLight.done then
            savedLight = {
                Brightness = Lighting.Brightness,
                ClockTime = Lighting.ClockTime,
                Ambient = Lighting.Ambient,
                OutdoorAmbient = Lighting.OutdoorAmbient,
                GlobalShadows = Lighting.GlobalShadows,
                done = true,
            }
        end
        pcall(function()
            Lighting.Brightness = 3
            Lighting.ClockTime = 14
            Lighting.Ambient = Color3.fromRGB(170, 170, 170)
            Lighting.OutdoorAmbient = Color3.fromRGB(170, 170, 170)
            Lighting.GlobalShadows = false
        end)
    elseif savedLight.done then
        pcall(function()
            Lighting.Brightness = savedLight.Brightness
            Lighting.ClockTime = savedLight.ClockTime
            Lighting.Ambient = savedLight.Ambient
            Lighting.OutdoorAmbient = savedLight.OutdoorAmbient
            Lighting.GlobalShadows = savedLight.GlobalShadows
        end)
        savedLight.done = false
    end
end

local savedFog = {}
local function setFog(on)
    if on then
        if not savedFog.done then
            savedFog = { Start = Lighting.FogStart, End = Lighting.FogEnd, done = true }
        end
        pcall(function()
            Lighting.FogStart = 1e6
            Lighting.FogEnd = 1e6
        end)
    elseif savedFog.done then
        pcall(function()
            Lighting.FogStart = savedFog.Start
            Lighting.FogEnd = savedFog.End
        end)
        savedFog.done = false
    end
end

local chams = {}

local function chamsStep()
    if not S.vis.chams then
        for pl, h in pairs(chams) do pcall(function() h:Destroy() end); chams[pl] = nil end
        return
    end
    for _, pl in ipairs(Players:GetPlayers()) do
        local c = charOf(pl)
        if pl ~= LP and c and not chams[pl] then
            local h = Instance.new("Highlight")
            h.Name = "KHubCham"
            h.FillTransparency = 0.55
            h.OutlineTransparency = 0
            h.Parent = c
            chams[pl] = h
        end
        if chams[pl] then
            local col = Color3.fromRGB(120, 140, 255)
            if pl.Team and LP.Team then
                col = (pl.Team == LP.Team) and Color3.fromRGB(90, 220, 120) or Color3.fromRGB(245, 105, 105)
            end
            chams[pl].FillColor = col
            chams[pl].OutlineColor = col
            if not c then pcall(function() chams[pl]:Destroy() end); chams[pl] = nil end
        end
    end
end

local function hitboxStep()
    if S.misc.hitbox <= 0 then return end
    local mult = 1 + S.misc.hitbox / 100
    for _, pl in ipairs(Players:GetPlayers()) do
        if pl ~= LP then
            local c = charOf(pl)
            if c then
                for _, p in ipairs(c:GetDescendants()) do
                    if p:IsA("BasePart") and p.Name ~= "HumanoidRootPart" then
                        local ok, size = pcall(function() return p.Size end)
                        if ok and size then
                            pcall(function() p.Size = size * mult end)
                        end
                    end
                end
            end
        end
    end
end

----------------------------------------------------------------- misc
local function antiAfk()
    track(LP.Idled:Connect(function()
        if S.misc.antiafk then
            pcall(function()
                VirtualUser:CaptureController()
                VirtualUser:ClickButton2(Vector2.new())
            end)
        end
    end))
end

local function fpsBoost()
    local c = 0
    for _, d in ipairs(WS:GetDescendants()) do
        if d:IsA("ParticleEmitter") or d:IsA("Smoke") or d:IsA("Fire") or d:IsA("Sparkles")
            or d:IsA("Decal") or d:IsA("Texture") or d:IsA("Trail") then
            pcall(function() d:Destroy() end)
            c = c + 1
        end
    end
    return c
end

local function analyze()
    local out = {}
    local function add(k, v) table.insert(out, tostring(k) .. ": " .. tostring(v)) end
    add("PlaceId", game.PlaceId)
    add("JobId", game.JobId)
    local ok, p = pcall(function() return MarketSvc:GetProductInfo(game.PlaceId) end)
    if ok and p then
        add("Game", p.Name)
        add("Creator", p.Creator and p.Creator.Name)
        add("Playing", p.Playing)
    end
    add("Players", #Players:GetPlayers())
    local names = {}
    for _, pl in ipairs(Players:GetPlayers()) do
        local h = humOf(pl)
        table.insert(names, pl.Name .. (h and ("(" .. math.floor(h.Health) .. ")") or ""))
    end
    add("PlayerList", table.concat(names, ", "))
    local c = char()
    if c then
        local h = hum()
        add("Health", h and (h.Health .. "/" .. h.MaxHealth))
        add("Rig", c:FindFirstChild("UpperTorso") and "R15" or "R6")
        local tools = {}
        for _, o in ipairs(c:GetChildren()) do
            if o:IsA("Tool") then table.insert(tools, o.Name) end
        end
        local bp = LP:FindFirstChildOfClass("Backpack")
        if bp then
            for _, o in ipairs(bp:GetChildren()) do
                if o:IsA("Tool") then table.insert(tools, o.Name .. "(bag)") end
            end
        end
        add("Tools", #tools > 0 and table.concat(tools, ", ") or "none")
        local live = tools[1]
        if live then
            local t = c:FindFirstChild(live, true)
            if t then
                local props = {}
                for _, n in ipairs(RAPID_RATE) do
                    local ok2, v = pcall(function() return t[n] end)
                    if ok2 and type(v) == "number" then table.insert(props, n .. "=" .. v) end
                end
                for _, n in ipairs(RANGE_PROPS) do
                    local ok2, v = pcall(function() return t[n] end)
                    if ok2 and type(v) == "number" then table.insert(props, n .. "=" .. v) end
                end
                for _, n in ipairs(SPREAD_PROPS) do
                    local ok2, v = pcall(function() return t[n] end)
                    if ok2 and type(v) == "number" then table.insert(props, n .. "=" .. v) end
                end
                if #props > 0 then add("WeaponStats", table.concat(props, ", ")) end
            end
        end
    end
    local remotes = {}
    for _, d in ipairs(RS:GetDescendants()) do
        if d:IsA("RemoteEvent") or d:IsA("RemoteFunction") or d:IsA("UnreliableRemoteEvent") then
            table.insert(remotes, d:GetFullName())
            if #remotes >= 120 then break end
        end
    end
    add("RemoteCount", #remotes)
    add("Remotes", table.concat(remotes, " | "))
    local wsKids = {}
    local n = 0
    for _, k in ipairs(WS:GetChildren()) do
        n = n + 1
        if #wsKids < 25 then table.insert(wsKids, k.Name .. "(" .. k.ClassName .. ")") end
    end
    add("WorkspaceChildren", n)
    add("WorkspaceSample", table.concat(wsKids, ", "))
    for _, line in ipairs(out) do print("[KHUB] " .. line) end
    notify("Game analysed - see console")
end

local CONFIG_FILE = "PotassiumHub.json"
local function saveCfg()
    local data = {}
    for g, t in pairs(S) do
        data[g] = {}
        for k, v in pairs(t) do
            if type(v) ~= "table" then data[g][k] = v end
        end
    end
    if not writefile then notify("executor has no writefile"); return end
    local ok, enc = pcall(function() return HttpSvc:JSONEncode(data) end)
    if not ok then notify("config encode failed"); return end
    pcall(function() writefile(CONFIG_FILE, enc) end)
    notify("config saved")
end

local function loadCfg()
    if not (isfile and readfile) then notify("executor has no readfile"); return end
    local exists = false
    pcall(function() exists = isfile(CONFIG_FILE) end)
    if not exists then notify("no saved config"); return end
    local ok, dec = pcall(function() return HttpSvc:JSONDecode(readfile(CONFIG_FILE)) end)
    if not ok or type(dec) ~= "table" then notify("config decode failed"); return end
    for g, t in pairs(dec) do
        if S[g] and type(t) == "table" then
            for k, v in pairs(t) do
                if type(S[g][k]) == type(v) then S[g][k] = v end
            end
        end
    end
    refreshAll()
    notify("config loaded")
end

----------------------------------------------------------------- window
local win = Instance.new("Frame")
win.Size = UDim2.new(0, 520, 0, 366)
win.Position = UDim2.new(0.5, -260, 0.5, -183)
win.BackgroundColor3 = BG
win.BorderSizePixel = 0
win.Parent = gui
corner(win, 10)
stroke(win, Color3.fromRGB(52, 56, 74))

local bar = Instance.new("Frame")
bar.Size = UDim2.new(1, 0, 0, 34)
bar.BackgroundTransparency = 1
bar.Parent = win

local title = label(bar, "POTASSIUM HUB", 13, ACCENT)
title.Font = Enum.Font.GothamBold
title.Size = UDim2.new(0, 200, 1, 0)
title.Position = UDim2.new(0, 12, 0, 0)

local hint = label(bar, "right shift hides", 11, Color3.fromRGB(118, 122, 140))
hint.Size = UDim2.new(0, 140, 1, 0)
hint.Position = UDim2.new(0, 150, 0, 0)

local closeBtn = Instance.new("TextButton")
closeBtn.Size = UDim2.new(0, 22, 0, 22)
closeBtn.Position = UDim2.new(1, -30, 0, 6)
closeBtn.BackgroundColor3 = Color3.fromRGB(58, 38, 44)
closeBtn.Text = "x"
closeBtn.Font = Enum.Font.GothamBold
closeBtn.TextSize = 12
closeBtn.TextColor3 = TXT
closeBtn.Parent = bar
corner(closeBtn, 5)

local dragging, dragStart, winStart = false, nil, nil
bar.InputBegan:Connect(function(i)
    if i.UserInputType == Enum.UserInputType.MouseButton1 then
        dragging = true
        dragStart = i.Position
        winStart = win.Position
    end
end)
track(UIS.InputChanged:Connect(function(i)
    if dragging and i.UserInputType == Enum.UserInputType.MouseMovement then
        local d = i.Position - dragStart
        win.Position = UDim2.new(winStart.X.Scale, winStart.X.Offset + d.X, winStart.Y.Scale, winStart.Y.Offset + d.Y)
    end
end))
track(UIS.InputEnded:Connect(function(i)
    if i.UserInputType == Enum.UserInputType.MouseButton1 then dragging = false end
end))

local tabHolder = Instance.new("Frame")
tabHolder.Size = UDim2.new(0, 112, 1, -44)
tabHolder.Position = UDim2.new(0, 10, 0, 40)
tabHolder.BackgroundTransparency = 1
tabHolder.Parent = win
local tabList = Instance.new("UIListLayout")
tabList.Padding = UDim.new(0, 5)
tabList.SortOrder = Enum.SortOrder.LayoutOrder
tabList.Parent = tabHolder

local contentHolder = Instance.new("Frame")
contentHolder.Size = UDim2.new(1, -132, 1, -44)
contentHolder.Position = UDim2.new(0, 128, 0, 40)
contentHolder.BackgroundColor3 = Color3.fromRGB(23, 24, 30)
contentHolder.Parent = win
corner(contentHolder, 8)

local pages = {}
local function showPage(target)
    for _, q in ipairs(pages) do
        q.frame.Visible = (q == target)
        q.btn.BackgroundColor3 = (q == target) and ACCENT or BG2
        q.btn.BackgroundTransparency = (q == target) and 0 or 0.35
    end
end
local function newPage(name)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(1, 0, 0, 27)
    b.BackgroundColor3 = BG2
    b.BackgroundTransparency = 0.35
    b.Text = name
    b.Font = Enum.Font.GothamMedium
    b.TextSize = 12
    b.TextColor3 = TXT
    b.Parent = tabHolder
    corner(b, 6)
    local page = Instance.new("ScrollingFrame")
    page.Size = UDim2.new(1, -12, 1, -12)
    page.Position = UDim2.new(0, 6, 0, 6)
    page.BackgroundTransparency = 1
    page.BorderSizePixel = 0
    page.ScrollBarThickness = 3
    page.ScrollBarImageColor3 = ACCENT
    page.CanvasSize = UDim2.new(0, 0, 0, 0)
    page.AutomaticCanvasSize = Enum.AutomaticSize.Y
    page.Visible = false
    page.Parent = contentHolder
    local lay = Instance.new("UIListLayout")
    lay.Padding = UDim.new(0, 5)
    lay.SortOrder = Enum.SortOrder.LayoutOrder
    lay.Parent = page
    local p = { frame = page, btn = b, name = name }
    b.MouseButton1Click:Connect(function() showPage(p) end)
    table.insert(pages, p)
    return p
end

----------------------------------------------------------------- widgets
local function row(page, h)
    local f = Instance.new("Frame")
    f.Size = UDim2.new(1, -4, 0, h)
    f.BackgroundColor3 = BG2
    f.Parent = page.frame
    corner(f, 5)
    return f
end

local function toggleRow(page, text, get, set)
    local f = row(page, 28)
    local l = label(f, text, 12)
    l.Size = UDim2.new(1, -70, 1, 0)
    l.Position = UDim2.new(0, 9, 0, 0)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 44, 0, 19)
    b.Position = UDim2.new(1, -52, 0.5, -10)
    b.BackgroundColor3 = DIM
    b.Font = Enum.Font.GothamMedium
    b.TextSize = 11
    b.TextColor3 = TXT
    b.Parent = f
    corner(b, 4)
    local function refresh()
        local on = get()
        b.Text = on and "ON" or "OFF"
        b.BackgroundColor3 = on and ACCENT or DIM
    end
    b.MouseButton1Click:Connect(function()
        set(not get())
        refresh()
    end)
    refresh()
    addRefresh(refresh)
    return f
end

local function sliderRow(page, text, minv, maxv, get, set, step)
    local f = row(page, 38)
    local l = label(f, text, 12)
    l.Size = UDim2.new(1, -70, 0, 18)
    l.Position = UDim2.new(0, 9, 0, 2)
    local val = label(f, tostring(get()), 11, ACCENT)
    val.Size = UDim2.new(0, 60, 0, 18)
    val.Position = UDim2.new(1, -66, 0, 2)
    val.TextXAlignment = Enum.TextXAlignment.Right
    local back = Instance.new("Frame")
    back.Size = UDim2.new(1, -18, 0, 5)
    back.Position = UDim2.new(0, 9, 0, 25)
    back.BackgroundColor3 = DIM
    back.Parent = f
    corner(back, 3)
    local fill = Instance.new("Frame")
    fill.Size = UDim2.new(0, 0, 1, 0)
    fill.BackgroundColor3 = ACCENT
    fill.BorderSizePixel = 0
    fill.Parent = back
    corner(fill, 3)
    local dragging = false
    local function paint()
        local frac = (get() - minv) / math.max(maxv - minv, 1e-6)
        fill.Size = UDim2.new(math.clamp(frac, 0, 1), 0, 1, 0)
        val.Text = tostring(get())
    end
    local function apply(x)
        local frac = math.clamp((x - back.AbsolutePosition.X) / math.max(back.AbsoluteSize.X, 1), 0, 1)
        local v = minv + (maxv - minv) * frac
        if step then v = math.floor(v / step + 0.5) * step end
        v = math.floor(v * 1000 + 0.5) / 1000
        set(v)
        paint()
    end
    back.InputBegan:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 then
            dragging = true
            apply(i.Position.X)
        end
    end)
    track(UIS.InputChanged:Connect(function(i)
        if dragging and i.UserInputType == Enum.UserInputType.MouseMovement then
            apply(i.Position.X)
        end
    end))
    track(UIS.InputEnded:Connect(function(i)
        if i.UserInputType == Enum.UserInputType.MouseButton1 then dragging = false end
    end))
    paint()
    addRefresh(paint)
    return f
end

local function dropRow(page, text, options, get, set)
    local f = row(page, 28)
    local l = label(f, text, 12)
    l.Size = UDim2.new(1, -110, 1, 0)
    l.Position = UDim2.new(0, 9, 0, 0)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 96, 0, 19)
    b.Position = UDim2.new(1, -104, 0.5, -10)
    b.BackgroundColor3 = DIM
    b.Font = Enum.Font.GothamMedium
    b.TextSize = 11
    b.TextColor3 = TXT
    b.Text = tostring(get())
    b.Parent = f
    corner(b, 4)
    b.MouseButton1Click:Connect(function()
        local idx = 1
        for i, v in ipairs(options) do
            if tostring(v) == tostring(get()) then idx = i end
        end
        idx = idx % #options + 1
        set(options[idx])
        b.Text = tostring(options[idx])
    end)
    addRefresh(function() b.Text = tostring(get()) end)
    return f
end

local function buttonRow(page, text, cb, col)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(1, -4, 0, 28)
    b.BackgroundColor3 = col or Color3.fromRGB(34, 36, 45)
    b.Font = Enum.Font.GothamMedium
    b.TextSize = 12
    b.TextColor3 = TXT
    b.Text = text
    b.Parent = page.frame
    corner(b, 5)
    b.MouseButton1Click:Connect(function() pcall(cb) end)
    return b
end

local captureKey = nil
local function keyRow(page, text, get, set)
    local f = row(page, 28)
    local l = label(f, text, 12)
    l.Size = UDim2.new(1, -110, 1, 0)
    l.Position = UDim2.new(0, 9, 0, 0)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 96, 0, 19)
    b.Position = UDim2.new(1, -104, 0.5, -10)
    b.BackgroundColor3 = DIM
    b.Font = Enum.Font.GothamMedium
    b.TextSize = 11
    b.TextColor3 = TXT
    b.Text = tostring(get())
    b.Parent = f
    corner(b, 4)
    b.MouseButton1Click:Connect(function()
        b.Text = "press key"
        captureKey = function(name)
            set(name)
            b.Text = tostring(name)
        end
    end)
    addRefresh(function() b.Text = tostring(get()) end)
    return f
end

----------------------------------------------------------------- pages
local pCombat  = newPage("Combat")
local pSilent  = newPage("Silent")
local pWeapon  = newPage("Weapons")
local pVisual  = newPage("Visuals")
local pMove    = newPage("Movement")
local pPlayers = newPage("Players")
local pMisc    = newPage("Misc")

-- combat
toggleRow(pCombat, "Camlock", function() return S.aimbot.on end, function(v) S.aimbot.on = v end)
dropRow(pCombat, "Activation", { "Hold", "Toggle", "Always" },
    function() return S.aimbot.keyMode end, function(v) S.aimbot.keyMode = v; engaged = false end)
keyRow(pCombat, "Aim key", function() return S.aimbot.key end, function(v) S.aimbot.key = v end)
sliderRow(pCombat, "FOV", 20, 800, function() return S.aimbot.fov end, function(v) S.aimbot.fov = v end)
sliderRow(pCombat, "Smoothing", 1, 20, function() return S.aimbot.smooth end, function(v) S.aimbot.smooth = v end, 1)
sliderRow(pCombat, "Prediction", 0, 0.30, function() return S.aimbot.predict end, function(v) S.aimbot.predict = v end, 0.01)
dropRow(pCombat, "Hit part", { "Head", "Torso", "Root", "Nearest" },
    function() return S.aimbot.part end, function(v) S.aimbot.part = v end)
toggleRow(pCombat, "Wall check", function() return S.aimbot.wallcheck end, function(v) S.aimbot.wallcheck = v end)
toggleRow(pCombat, "Team check", function() return S.aimbot.teamcheck end, function(v) S.aimbot.teamcheck = v end)

-- silent
toggleRow(pSilent, "Silent aim", function() return S.silent.on end, function(v) S.silent.on = v end)
sliderRow(pSilent, "Hit chance %", 0, 100, function() return S.silent.hit end, function(v) S.silent.hit = v end, 1)
sliderRow(pSilent, "FOV", 20, 800, function() return S.silent.fov end, function(v) S.silent.fov = v end)
sliderRow(pSilent, "Prediction", 0, 0.30, function() return S.silent.predict end, function(v) S.silent.predict = v end, 0.01)
dropRow(pSilent, "Hit part", { "Head", "Torso", "Root", "Nearest" },
    function() return S.silent.part end, function(v) S.silent.part = v end)
toggleRow(pSilent, "Rewrite remote args", function() return S.silent.namecall end, function(v) S.silent.namecall = v end)
toggleRow(pSilent, "Spoof Mouse.Hit/Target", function() return S.silent.mouse end, function(v) S.silent.mouse = v end)
toggleRow(pSilent, "Spoof camera rays", function() return S.silent.rays end, function(v) S.silent.rays = v end)
toggleRow(pSilent, "Wall check (aim)", function() return S.silent.wallcheck end, function(v) S.silent.wallcheck = v end)
toggleRow(pSilent, "Team check", function() return S.silent.teamcheck end, function(v) S.silent.teamcheck = v end)
toggleRow(pSilent, "Log fire remotes (debug)", function() return S.debug.logfire end, function(v) S.debug.logfire = v end)

-- weapons
toggleRow(pWeapon, "Rapid fire", function() return S.weapon.rapid end, function(v) S.weapon.rapid = v end)
sliderRow(pWeapon, "Rapid burst", 1, 15, function() return S.weapon.burst end, function(v) S.weapon.burst = v end, 1)
toggleRow(pWeapon, "Infinite range", function() return S.weapon.range end, function(v) S.weapon.range = v end)
toggleRow(pWeapon, "No spread", function() return S.weapon.spread end, function(v) S.weapon.spread = v end)
toggleRow(pWeapon, "No recoil", function() return S.weapon.recoil end, function(v) S.weapon.recoil = v end)
toggleRow(pWeapon, "Instant reload", function() return S.weapon.reload end, function(v) S.weapon.reload = v end)
toggleRow(pWeapon, "Infinite ammo", function() return S.weapon.ammo end, function(v) S.weapon.ammo = v end)
buttonRow(pWeapon, "Patch equipped tool now", function()
    weaponStep()
    notify("weapon stats patched")
end)

-- visuals
toggleRow(pVisual, "Player ESP", function() return S.vis.esp end, function(v) S.vis.esp = v end)
toggleRow(pVisual, "Box", function() return S.vis.box end, function(v) S.vis.box = v end)
toggleRow(pVisual, "Name", function() return S.vis.name end, function(v) S.vis.name = v end)
toggleRow(pVisual, "Health bar", function() return S.vis.health end, function(v) S.vis.health = v end)
toggleRow(pVisual, "Distance", function() return S.vis.dist end, function(v) S.vis.dist = v end)
toggleRow(pVisual, "Tracers", function() return S.vis.tracer end, function(v) S.vis.tracer = v end)
toggleRow(pVisual, "Skeleton", function() return S.vis.skeleton end, function(v) S.vis.skeleton = v end)
toggleRow(pVisual, "Chams", function() return S.vis.chams end, function(v) S.vis.chams = v end)
sliderRow(pVisual, "Max distance", 50, 3000, function() return S.vis.maxdist end, function(v) S.vis.maxdist = v end, 10)
toggleRow(pVisual, "FOV circle", function() return S.vis.fovcircle end, function(v) S.vis.fovcircle = v end)
toggleRow(pVisual, "Fullbright", function() return S.misc.fullbright end, function(v) S.misc.fullbright = v; setLight(v) end)
toggleRow(pVisual, "No fog", function() return S.misc.nofog end, function(v) S.misc.nofog = v; setFog(v) end)
sliderRow(pVisual, "Hitbox expand %", 0, 300, function() return S.misc.hitbox end, function(v) S.misc.hitbox = v end, 5)

-- movement
toggleRow(pMove, "Walk speed", function() return S.move.speedon end, function(v) S.move.speedon = v end)
sliderRow(pMove, "Speed", 16, 300, function() return S.move.speed end, function(v) S.move.speed = v end, 1)
toggleRow(pMove, "Jump power", function() return S.move.jumpon end, function(v) S.move.jumpon = v end)
sliderRow(pMove, "Jump", 50, 400, function() return S.move.jump end, function(v) S.move.jump = v end, 1)
toggleRow(pMove, "Fly", function() return S.move.fly end, function(v) S.move.fly = v; flySet(v) end)
sliderRow(pMove, "Fly speed", 10, 400, function() return S.move.flyspeed end, function(v) S.move.flyspeed = v end, 5)
toggleRow(pMove, "Noclip", function() return S.move.noclip end, function(v) S.move.noclip = v end)
toggleRow(pMove, "Infinite jump", function() return S.move.infjump end, function(v) S.move.infjump = v end)
toggleRow(pMove, "Click teleport", function() return S.move.tpclick end, function(v) S.move.tpclick = v end)
toggleRow(pMove, "Custom gravity", function() return S.move.gravon end, function(v) S.move.gravon = v end)
sliderRow(pMove, "Gravity", 10, 400, function() return S.move.gravity end, function(v) S.move.gravity = v end, 1)

-- players
local playerList = Instance.new("ScrollingFrame")
playerList.Size = UDim2.new(1, -4, 1, -4)
playerList.Position = UDim2.new(0, 2, 0, 2)
playerList.BackgroundTransparency = 1
playerList.BorderSizePixel = 0
playerList.ScrollBarThickness = 3
playerList.CanvasSize = UDim2.new(0, 0, 0, 0)
playerList.AutomaticCanvasSize = Enum.AutomaticSize.Y
playerList.Parent = pPlayers.frame
local plLayout = Instance.new("UIListLayout")
plLayout.Padding = UDim.new(0, 4)
plLayout.Parent = playerList

local function rootOf(pl)
    local c = charOf(pl)
    if c then return c:FindFirstChild("HumanoidRootPart") or c:FindFirstChild("Torso") end
end

local function tpTo(pos)
    local rp = root()
    if rp then rp.CFrame = CFrame.new(pos + Vector3.new(0, 3, 0)) end
end

local function rebuildPlayers()
    for _, c in ipairs(playerList:GetChildren()) do
        if c:IsA("TextButton") then c:Destroy() end
    end
    for _, pl in ipairs(Players:GetPlayers()) do
        local b = Instance.new("TextButton")
        b.Size = UDim2.new(1, -6, 0, 27)
        b.BackgroundColor3 = BG2
        b.Font = Enum.Font.GothamMedium
        b.TextSize = 12
        b.TextColor3 = TXT
        b.TextXAlignment = Enum.TextXAlignment.Left
        local h = humOf(pl)
        b.Text = "  " .. pl.Name .. (h and string.format("  [%d]", math.floor(h.Health)) or "")
        b.Parent = playerList
        corner(b, 5)
        b.MouseButton1Click:Connect(function()
            local rp = rootOf(pl)
            if rp then
                tpTo(rp.Position)
                notify("teleported to " .. pl.Name)
            else
                notify(pl.Name .. " has no character")
            end
        end)
    end
end
task.spawn(function()
    while gui.Parent do
        pcall(rebuildPlayers)
        task.wait(2)
    end
end)

-- misc
buttonRow(pMisc, "Analyse current game", analyze, Color3.fromRGB(38, 44, 66))
toggleRow(pMisc, "Watermark", function() return S.misc.watermark end, function(v) S.misc.watermark = v end)
toggleRow(pMisc, "Anti AFK", function() return S.misc.antiafk end, function(v) S.misc.antiafk = v end)
buttonRow(pMisc, "FPS boost (removes particles/decals)", function()
    local n = fpsBoost()
    notify("removed " .. n .. " effects")
end)
buttonRow(pMisc, "Save config", saveCfg)
buttonRow(pMisc, "Load config", loadCfg)
buttonRow(pMisc, "Unload hub", function() unload() end, Color3.fromRGB(58, 34, 40))
closeBtn.MouseButton1Click:Connect(function() win.Visible = false end)

----------------------------------------------------------------- watermark
local wm = Instance.new("TextLabel")
wm.Size = UDim2.new(0, 300, 0, 20)
wm.Position = UDim2.new(0, 12, 0, 8)
wm.BackgroundTransparency = 1
wm.Font = Enum.Font.GothamMedium
wm.TextSize = 12
wm.TextXAlignment = Enum.TextXAlignment.Left
wm.TextColor3 = ACCENT
wm.Text = "potassium hub"
wm.Parent = gui

----------------------------------------------------------------- input
local function matchesAimKey(i)
    if S.aimbot.key == "MouseButton1" or S.aimbot.key == "MouseButton2" then
        return i.UserInputType.Name == S.aimbot.key
    end
    if i.UserInputType == Enum.UserInputType.Keyboard and i.KeyCode then
        return i.KeyCode.Name == S.aimbot.key
    end
    return false
end

track(UIS.InputBegan:Connect(function(i, gp)
    if captureKey then
        local name = (i.UserInputType == Enum.UserInputType.Keyboard) and i.KeyCode.Name or i.UserInputType.Name
        local f = captureKey
        captureKey = nil
        f(name)
        return
    end
    if gp then return end
    if i.UserInputType == Enum.UserInputType.Keyboard and i.KeyCode == Enum.KeyCode.RightShift then
        win.Visible = not win.Visible
        return
    end
    if matchesAimKey(i) then
        if S.aimbot.keyMode == "Toggle" then
            engaged = not engaged
        else
            engaged = true
        end
    end
    if S.move.tpclick and i.UserInputType == Enum.UserInputType.MouseButton1 then
        local wp = mouseWorld()
        if typeof(wp) == "Vector3" then tpTo(wp) end
    end
end))

track(UIS.InputEnded:Connect(function(i)
    if matchesAimKey(i) and S.aimbot.keyMode == "Hold" then engaged = false end
end))

track(UIS.JumpRequest:Connect(function()
    if S.move.infjump then
        local h = hum()
        if h then pcall(function() h:ChangeState(Enum.HumanoidStateType.Jumping) end) end
    end
end))

----------------------------------------------------------------- loops
antiAfk()

local okNamecall = installNamecall()
local okMouse = installMouseHook()

local frameCount, fpsValue = 0, 0
local fpsClock = os.clock()

track(RunSvc.RenderStepped:Connect(function(dt)
    frameCount = frameCount + 1
    if os.clock() - fpsClock >= 1 then
        fpsValue = math.floor(frameCount / (os.clock() - fpsClock))
        frameCount = 0
        fpsClock = os.clock()
    end

    aimTarget = acquire(S.silent)

    if S.aimbot.on and (engaged or S.aimbot.keyMode == "Always") then
        local t = acquire(S.aimbot)
        if t then
            local want = CFrame.new(Camera.CFrame.Position, t.pos)
            if S.aimbot.smooth > 1 then
                local a = math.clamp(1 - math.exp(-S.aimbot.smooth * dt * 2.5), 0, 1)
                Camera.CFrame = Camera.CFrame:Lerp(want, a)
            else
                Camera.CFrame = want
            end
        end
    end

    movementStep(dt)
    espStep()
    fovStep()

    if S.misc.watermark then
        local tgt = aimTarget and aimTarget.pl and aimTarget.pl.Name or "none"
        local ping = "?"
        pcall(function() ping = math.floor(Stats.Network.ServerStatsItem["Data Ping"]:GetValue()) end)
        wm.Text = string.format("potassium hub  |  %d fps  |  %s ms  |  target: %s",
            fpsValue, tostring(ping), tgt)
        wm.Visible = true
    else
        wm.Visible = false
    end
end))

task.spawn(function()
    while gui.Parent do
        pcall(weaponStep)
        pcall(chamsStep)
        pcall(hitboxStep)
        task.wait(0.7)
    end
end)

track(LP.CharacterAdded:Connect(function()
    task.wait(1)
    flySet(S.move.fly)
end))

----------------------------------------------------------------- unload
function unload()
    for _, c in ipairs(conns) do pcall(function() c:Disconnect() end) end
    for pl, e in pairs(esp) do pcall(hideEsp, e) end
    for pl, h in pairs(chams) do pcall(function() h:Destroy() end) end
    pcall(function() setLight(false) end)
    pcall(function() setFog(false) end)
    pcall(function() flySet(false) end)
    pcall(function() WS.Gravity = 196.2 end)
    if oldNamecall and hookmetamethod then pcall(function() hookmetamethod(game, "__namecall", oldNamecall) end) end
    if oldIndex and hookmetamethod then pcall(function() hookmetamethod(game, "__index", oldIndex) end) end
    pcall(function() gui:Destroy() end)
    genv().PotassiumHubUnload = nil
    genv().PotassiumHub = nil
    print("[KHUB] unloaded")
end
genv().PotassiumHubUnload = unload
genv().PotassiumHub = S

showPage(pCombat)
win.Visible = false
notify("potassium hub ready - right shift", 4)
print("[KHUB] loaded | place " .. tostring(game.PlaceId) .. " | namecall hook " .. tostring(okNamecall)
    .. " | mouse hook " .. tostring(okMouse) .. " | drawing " .. tostring(drawEsp))
if not drawEsp then
    notify("no Drawing API - ESP disabled", 5)
end
