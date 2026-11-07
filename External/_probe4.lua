local function segs(src, word, n, back, fwd)
  local out, i = {}, 1
  while true do
    local a, b = string.find(src, word, i, true)
    if not a then break end
    out[#out + 1] = string.sub(src, math.max(1, a - back), math.min(#src, b + fwd))
    i = b + 1
    if #out >= n then break end
  end
  return out
end
local words = { "FireServer", "InvokeServer", "Raycast", "Mouse.UnitRay" }
local function dump(obj, label)
  if not obj then print("[P4] " .. label .. " missing") return end
  local ok, src = pcall(decompile, obj)
  if not ok or type(src) ~= "string" then print("[P4] " .. label .. " decompile failed") return end
  print("[P4] === " .. label .. " len " .. #src)
  for _, w in ipairs(words) do
    for _, s in ipairs(segs(src, w, 2, 300, 420)) do
      print("[P4][" .. w .. "] " .. s)
    end
  end
end
local lp = game:GetService("Players").LocalPlayer
local bp = lp:FindFirstChildOfClass("Backpack")
local shotgun, revolver
if bp then
  for _, t in ipairs(bp:GetChildren()) do
    if t:IsA("Tool") then
      if t:FindFirstChild("GunClientShotgun") then shotgun = t end
      if t.Name == "[Revolver]" then revolver = t end
    end
  end
end
dump(shotgun and shotgun:FindFirstChild("GunClientShotgun"), "GunClientShotgun")
dump(revolver and revolver:FindFirstChild("GunClient"), "Revolver.GunClient")
local gh = game:GetService("ReplicatedStorage"):FindFirstChild("Modules")
gh = gh and gh:FindFirstChild("GunHandler")
dump(gh, "Modules.GunHandler")
print("[P4-DONE]")
