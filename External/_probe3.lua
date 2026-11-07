local env = getfenv and getfenv(0) or _G
local names = {"getscriptbytecode","decompile","getscriptclosure","getrawmetatable","getconnections",
  "setreadonly","getsenv","getloadedmodules","setclipboard","identifyexecutor","getexecutorname",
  "getcustomasset","request","http_request","isfile","writefile","Drawing","hookmetamethod","checkcaller"}
local have = {}
for _, n in ipairs(names) do
  local ok, v = pcall(function() return env[n] end)
  have[#have+1] = n .. "=" .. tostring(ok and v ~= nil)
end
print("[P3] " .. table.concat(have, " "))
local bp = game:GetService("Players").LocalPlayer:FindFirstChildOfClass("Backpack")
local tool = nil
if bp then
  for _, t in ipairs(bp:GetChildren()) do
    if t:IsA("Tool") then tool = t break end
  end
end
print("[P3] tool " .. tostring(tool and tool.Name))
local gs = tool and (tool:FindFirstChild("GunClient") or tool:FindFirstChild("GunClientShotgun"))
print("[P3] script " .. tostring(gs))
if gs then
  local ok, src = pcall(function() return decompile(gs) end)
  print("[P3] decompile ok=" .. tostring(ok))
  if ok and type(src) == "string" then
    print("[P3-LEN] " .. #src)
    print(string.sub(src, 1, 4500))
  elseif getscriptbytecode then
    local ok2, bc = pcall(getscriptbytecode, gs)
    print("[P3] bytecode ok=" .. tostring(ok2) .. " len=" .. tostring(ok2 and #bc or 0))
  end
end
print("[P3-DONE]")
