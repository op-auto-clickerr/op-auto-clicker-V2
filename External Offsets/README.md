# External Offsets

Versioned Roblox dumps used by the overlay.

`versions/version-XXXX/` contains `offsets.hpp`, `offsets.json`, `fflags.hpp`, `fflags.json`.

The overlay detects the running Roblox version and loads the matching folder.
Unknown versions fall back to Imtheo.
