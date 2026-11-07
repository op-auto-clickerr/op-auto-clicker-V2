local S = getgenv and getgenv().PotassiumHub
if not S then
    print("[KHUB] hub is not loaded - load test failed")
    return
end
S.debug.logfire = true
S.silent.on = true
S.weapon.rapid = true
S.weapon.range = true
S.weapon.ammo = true
print("[KHUB] armed: silent aim, rapid fire, infinite range/ammo, fire logging")
print("[KHUB] now shoot your gun once so the fire remote args get logged")
