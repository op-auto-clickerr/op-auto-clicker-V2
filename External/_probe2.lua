local lp = game:GetService("Players").LocalPlayer
local function attrs(o)
  local out = {}
  local ok, a = pcall(function() return o:GetAttributes() end)
  if ok and a then for k, v in pairs(a) do out[#out+1] = k .. "=" .. tostring(v) end end
  return table.concat(out, ", ")
end
local bp = lp:FindFirstChildOfClass("Backpack")
local function dumpTool(t)
  print("[P2] TOOL " .. t.Name .. " (" .. t.ClassName .. ") attrs {" .. attrs(t) .. "}")
  local names = {"FireRate","FireDelay","Damage","Range","MaxRange","Spread","MaxAmmo","Ammo","ReloadTime","Cooldown","BulletSpeed","ShootCooldown"}
  local hits = {}
  for _, n in ipairs(names) do
    local ok, v = pcall(function() return t[n] end)
    if ok and type(v) == "number" then hits[#hits+1] = n .. "=" .. v end
  end
  if #hits > 0 then print("[P2]  props: " .. table.concat(hits, ", ")) end
  local i = 0
  for _, d in ipairs(t:GetDescendants()) do
    i = i + 1
    if i > 40 then break end
    local a = attrs(d)
    print("[P2]   " .. d.Name .. " (" .. d.ClassName .. ")" .. (a ~= "" and (" {" .. a .. "}") or ""))
  end
end
if bp then
  local n = 0
  for _, t in ipairs(bp:GetChildren()) do
    if t:IsA("Tool") and n < 2 then dumpTool(t); n = n + 1 end
  end
end
local s = {}
for _, c in ipairs(game:GetService("ReplicatedStorage"):GetChildren()) do s[#s+1] = c.Name .. "(" .. c.ClassName .. ")" end
print("[P2] RS children: " .. table.concat(s, ", "))
local pf = game:GetService("Workspace"):FindFirstChild("Players")
if pf then
  local q = {}
  for _, c in ipairs(pf:GetChildren()) do
    if #q < 8 then q[#q+1] = c.Name .. "(" .. c.ClassName .. ")" end
  end
  print("[P2] WS.Players: " .. table.concat(q, ", "))
end
print("[P2] LP attrs {" .. attrs(lp) .. "}")
local ls = lp:FindFirstChild("leaderstats")
if ls then
  local q = {}
  for _, c in ipairs(ls:GetChildren()) do q[#q+1] = c.Name .. "=" .. tostring(c.Value) end
  print("[P2] leaderstats: " .. table.concat(q, ", "))
end
print("[P2-DONE]")
