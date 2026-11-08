# Statman - Unofficial HM3 In-Game Overlay Fork
Not affiliated with or endorsed by OrfeasZ or IO Interactive.

![Overlay](https://i.imgur.com/3dpTiPu.jpeg)

An attempt of improving the in-game overlay from the original OrfeasZ's version.

Heavily AI-Assisted code. Just like this README.

Don't be afraid to call it a vibe coded slop, because it is, in fact, a vibe coded slop. But it does its job.

Potentially this fork might have more features in the future.

## Please do not report fork-specific issues to the original Statman project.

Forked from: https://github.com/OrfeasZ/Statman

Original License: https://github.com/OrfeasZ/Statman/blob/master/LICENSE

## What's Different

This fork keeps Statman's original HM3 stat tracking and rating logic, but replaces the in-game display with a redesigned ImGui/D3D9 overlay.

- Redesigned in-game overlay with movable, individually toggleable panels for statistics, rating, timer, level info, and the roulette result. Positions are saved between sessions.
- Fully customizable appearance: fonts, sizes, colors, timer format and panel alignment, with optional snapping and separate Minimal/Detailed layouts.
- Minimal statistics mode with summed values such as `Non-Targets Killed` and `Non-Targets Harmed` - Similar to "Statman Compact v1.3.4.0.
- Built-in overlay editor and settings window, available from the toolbar that appears while the cursor is captured (`INSERT`).
- Built-in Hitman Roulette for Blood Money. The generated run is shown in the Roulette window and on the in-game HUD.
- Blood Money special ratings are displayed, with an optional experimental prediction before the mission ends.
- Game settings window for cheats and unlimited saves. Enabling cheats exposes the built-in trainer's individual toggles (infinite ammo/clip, god mode, invisibility and time multiplier) and the `Show OSD` HUD toggle; disabling the HUD is treated as a cheat. The same window holds the experimental special-rating prediction toggle. The HUD marks active cheats and unlimited saves.
- Hitman 2016 mode now has its own window with `Locations`, `Outfits` and `Debug info` tabs. Locations are grouped by level, can be named, and the last loaded location is restored automatically when that level restarts. The Outfits tab merges actors that share the same suit and can apply the outfit. Debug info draws distance-scaled actor IDs above the head, with a configurable maximum distance (default 2500 units, up to 10000); while active it is treated as a cheat.
- Statman can be started before or after Hitman: Blood Money; injection waits until the game is ready.
- Diagnostic logs are written next to `Statman.exe`; verbose logging and the log folder are available from the right-click menu.

## Showcase

[Click here to watch a video](https://www.youtube.com/watch?v=pQZxbgIwI5A)

![Overlay Menu](https://i.imgur.com/cHSAuXc.jpeg)

## Usage

Statman can be started before or after the game; injection waits for Hitman: Blood Money to finish initializing.

1. Run `Statman.exe` as administrator.
2. Start Hitman: Blood Money (or start Statman while it is already running).
3. Make sure the "In-game overlay" option is enabled in Statman — right click the top of the Statman window and select "In-game overlay".
4. Press `INSERT` in-game to toggle cursor capture for the overlay.
5. While cursor capture is active, use the top-left toolbar to open the `Game settings`, `Hitman 2016 mode`, `Roulette`, `Overlay editor` and `Overlay settings` windows.
6. Drag HUD panels to reposition them. Panels use their real in-game appearance while `INSERT` is held. Changes apply immediately; positions can be reset in the Overlay editor window.
7. Press `INSERT` again to release the cursor and save the overlay settings.

If the overlay does not appear, make sure Statman is running as administrator, the overlay option is enabled, and `HM3.dll` is next to `Statman.exe`.

## Diagnostics

Both logs are written next to `Statman.exe`. They are append-only, timestamped, capped at 1 MB (rotated to `.old`) and contain no personal data — only process ids, module addresses and hook results. They are safe to attach to a bug report.

| File | Written by | Contents |
| --- | --- | --- |
| `statman.log` | `Statman.exe` | process detection, injection readiness and result, errors |
| `hm3.log` | `HM3.dll` (inside the game) | module init, game readiness, hook installation, IPC |

Verbose logging can be enabled at runtime from Statman's right-click menu: **Verbose logging (diagnostics)**. That writes the detailed per-symbol and per-hook lines to both logs, and it can be turned off again without restarting.

The same menu has an **Open log folder** entry, which opens the folder containing `Statman.exe` and both log files.

For advanced users, verbose logging can also be forced from the start with the `STATMAN_VERBOSE` environment variable:

```bat
set STATMAN_VERBOSE=1
Statman.exe
```

## Configuration Paths

All user configuration lives in the `config` folder next to `Statman.exe`; the application folder itself only contains the executable, its dependencies and the in-game module.

- `config\statman.json` - application options such as dark mode and HM3 features like cheats, unlimited saves, 2016 mode, debug mode, and the overlay toggle.
- `config\HM3Overlay.json` - overlay visibility, per-stat panel visibility and positions, fonts, sizes and colors, plus the roulette mission/extras selection and HUD visibility. Created automatically on first overlay initialization. Delete it to reset, or use the editor's reset buttons.
- `config\HM3_locations.json` - Hitman 2016 starting locations and the last used location per level. Not required for normal HM3 tracking.

Older builds stored these files directly next to `Statman.exe`. They are moved into `config` automatically the first time this build runs, so saved settings and locations are kept.

External edits to `HM3Overlay.json` are read when the overlay initializes. Restart the game to reload them.

## Known Limitations

- This fork is focused on Hitman: Blood Money (HM3 engine). HM5/HM52021 engines are removed.
- The inherited Hitman 2016 mode remains experimental.
- A higher resolution (16:9, 1600x900 preferred) may be needed to see all overlay options.
- Other mods or overlays may conflict with injection or rendering.
- This is an unofficial fork. Please do not report fork-specific issues to the original Statman project.

## Requirements

- Windows with DirectX 9 support
- Hitman: Blood Money (overlay was only tested on the Steam version)
- .NET Framework 4.8

To build from source:

- Visual Studio 2022 with the **Desktop development with C++** and **.NET desktop development** workloads
- DirectX SDK (June 2010) — only needed for the native HM3 module
- Git
- NuGet package restore

## Build Instructions

Clone the repository with submodules:

```bat
git clone --recursive https://github.com/azeeeee/Statman-imgui.git
cd Statman-imgui
```

If you already cloned it:

```bat
git submodule update --init --recursive
```

ImGui is vendored directly in this fork. MinHook is still included as a submodule.

1. Open `Statman.sln` in Visual Studio 2022.
2. Restore NuGet packages if Visual Studio does not do it automatically.
3. Select `Release | x86`. This platform is required for the native HM3 module.
4. Build the whole solution, not only `Statman.csproj`, because the native project uses `$(SolutionDir)`.
5. The output is written to `_build\bin32\Release\`, including `Statman.exe`, `HM3.dll` and the `config` folder.

The DirectX SDK is only required when building the native HM3 module. It is not required to run the compiled overlay.

### Release Builds

Do not zip `_build\bin32\Release` by hand. `scripts\package.ps1` stages a clean folder from it, containing only the files that are loaded at runtime:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\package.ps1 -Zip
```

Output:

```text
_build\dist\Statman-<version>\
    Statman.exe
    Statman.exe.config
    HM3.dll
    Newtonsoft.Json.dll
    PeNet.dll
    PeNet.Asn1.dll
    System.Buffers.dll
    System.Memory.dll
    System.Numerics.Vectors.dll
    System.Runtime.CompilerServices.Unsafe.dll
    System.Security.Cryptography.Pkcs.dll
    config\HM3_locations.json
_build\dist\Statman-<version>.zip
```

`-Zip` also creates the archive; without it only the folder is staged. The version is read from `Statman.exe`.

Only runtime files are copied. Debug symbols (`*.pdb`), XML documentation (`*.xml`), logs and personal configuration are never included, and `HM3_locations.json` is taken from the source template so testing data cannot leak into a release. Debug symbols are kept in `_build\symbols\Release\` for crash analysis.

## Credits

- OrfeasZ — Orfeas Zafeiris, original Statman project.
- TheKotti — original Hitman Roulette concept and Blood Money roulette rules (https://github.com/TheKotti/thekotti.github.io/).
- ReGlacier — for the ReHitman project, which was also helpful for this fork.
- ocornut and contributors — Dear ImGui.
- TsudaKageyu — MinHook.
- secana — PeNet.
- James Newton-King — Newtonsoft.Json.
- Microsoft — .NET Framework and DirectX SDK.

This fork is based on Statman by Orfeas Zafiris and is licensed under the MIT license. See LICENSE for details. It is not affiliated with or endorsed by the original author or IO Interactive.

All fork-specific code and documentation were developed with AI-assisted tooling and reviewed by a human. 
