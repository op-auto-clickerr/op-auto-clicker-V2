local function out(k,v) print("[RECON] "..tostring(k)..": "..tostring(v)) end
out("PlaceId", game.PlaceId)
out("GameId", game.GameId)
local ok,p = pcall(function() return game:GetService("MarketplaceService"):GetProductInfo(game.PlaceId) end)
if ok and p then out("GameName", p.Name); out("Creator", p.Creator and p.Creator.Name) end
local Players=game:GetService("Players")
local lp=Players.LocalPlayer
out("LocalPlayer", lp and lp.Name)
out("PlayerCount", #Players:GetPlayers())
local names={} for _,pl in ipairs(Players:GetPlayers()) do table.insert(names, pl.Name) end
out("Players", table.concat(names,", "))
local char=lp and lp.Character
out("HasCharacter", char~=nil)
if char then
  local hum=char:FindFirstChildOfClass("Humanoid")
  out("Health", hum and (tostring(hum.Health).."/"..tostring(hum.MaxHealth)))
  local parts={} for _,c in ipairs(char:GetChildren()) do if c:IsA("BasePart") then table.insert(parts,c.Name) end end
  out("CharacterParts", table.concat(parts,","))
end
local bp = lp and lp:FindFirstChildOfClass("Backpack")
if bp then
  local ts={} for _,t in ipairs(bp:GetChildren()) do if t:IsA("Tool") then table.insert(ts,t.Name) end end
  out("BackpackTools", table.concat(ts,", "))
end
if char then
  local ts={} for _,t in ipairs(char:GetChildren()) do if t:IsA("Tool") then table.insert(ts,t.Name) end end
  out("CharacterTools", table.concat(ts,", "))
end
local rs=game:GetService("ReplicatedStorage")
local remotes={}
for _,d in ipairs(rs:GetDescendants()) do
  if d:IsA("RemoteEvent") or d:IsA("RemoteFunction") then
    table.insert(remotes, d:GetFullName())
    if #remotes>=150 then break end
  end
end
out("RemoteCount", #remotes)
print("[RECON-REMOTES] "..table.concat(remotes," | "))
local ws=game:GetService("Workspace")
local kids=0; local sample={}
for _,c in ipairs(ws:GetChildren()) do kids=kids+1; if #sample<25 then table.insert(sample, c.Name.."("..c.ClassName..")") end end
out("WorkspaceChildren", kids)
out("WorkspaceSample", table.concat(sample,", "))
out("LightingTech", tostring(game:GetService("Lighting").Technology))
print("[RECON-DONE]")
