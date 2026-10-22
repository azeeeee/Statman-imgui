# Statman - Unofficial HM3 In-Game Overlay Fork
Not affiliated with or endorsed by OrfeasZ or IO Interactive.

An attempt of improving the in-game overlay from the original OrfeasZ's version.

Heavily AI-Assisted code. Just like this README. 

Don't be afraid to call it a vibe coded slop, because it is, in fact, a vibe coded slop. But it does its job. 

Potentially this fork might have more features in the future. 


Forked from: https://github.com/OrfeasZ/Statman

Original License:

https://github.com/OrfeasZ/Statman/blob/master/LICENSE

## What's Different

This fork keeps Statman's original HM3 stat tracking and rating logic, but replaces the in-game display with a redesigned ImGui/D3D9 overlay.
![Overlay](https://i.imgur.com/IO6nWWR.png)

![Overlay Menu](https://i.imgur.com/AwFam8K.png)

- The HUD is split into movable panels: left stats, right stats, rating, timer, and level info.
- Added a top-left toolbar that appears when cursor capture is enabled.
- Added an in-game Overlay Editor and Settings window for visibility, fonts, sizes, colors, and row spacing.
- Panels can be repositioned while cursor capture is active. Positions persist between sessions. `INSERT` toggles cursor capture. 
- Added per-element visibility toggles and optional highlighting for bad stats.
- Overlay settings are stored in `%APPDATA%\Statman\HM3Overlay.json` and created automatically on first use.
- Fonts are loaded from the Windows Fonts folder.
- Hitman 2016 debug information is now rendered through the ImGui overlay.

## Requirements

- Windows with DirectX 9 support
- Hitman: Blood Money (Overlay was tested only on Steam version) 
- .NET Framework 4.8
- Administrator privileges

To build from source:
- Visual Studio 2022 with the **Desktop development with C++** and **.NET desktop development** workloads
- DirectX SDK (June 2010)
- Git
- NuGet package restore

## Build Instructions

Clone the repository with submodules:

```bat
git clone --recursive https://github.com/azeeeee/Statman-imgui.git
cd Statman-imgui
```

If you already cloned it:

`git submodule update --init --recursive`

ImGui is vendored directly in this fork. MinHook is still included as a submodule.
1. Open Statman.sln in Visual Studio 2022.
2. Restore NuGet packages if Visual Studio does not do it automatically.
3. Select Release | x86. This platform is required for the native HM3 module.
4. Build the whole solution, not only Statman.csproj, because the native project uses $(SolutionDir).
5. The output is written to _build\bin\Release\, including Statman.exe and HM3.dll.

The DirectX SDK is only required when building the native HM3 module. It is not required to run the compiled overlay.


## Usage
1. Start Hitman: Blood Money.
2. Run `Statman.exe` as administrator. Currently it is recommended to start it AFTER launching Hitman: Blood Money.
3. Make sure the HM3 Overlay option is enabled in Statman.
4. Press `INSERT` in-game to toggle cursor capture.
5. While cursor capture is active, use the top-left toolbar to open the Overlay Editor or Settings.
6. Drag HUD panels to reposition them. Changes apply immediately.
7. Press `INSERT` again to release the cursor and save the overlay settings.

If the overlay does not appear, check that Statman is running as administrator, the overlay option is enabled, the game is running, and `HM3.dll` is next to `Statman.exe`.

## Configuration Paths
- statman.json — stored next to Statman.exe. Contains application options such as dark mode and HM3 features like cheats, unlimited saves, 2016 mode, debug mode, and the overlay toggle.
- %APPDATA%\Statman\HM3Overlay.json — contains overlay visibility, panel positions, fonts, sizes, and colors. It is created automatically on first overlay initialization. Delete it to reset, or use the editor's reset buttons.
- HM3_locations.json — used by the inherited Hitman 2016 mode. It is not required for normal HM3 tracking.
External edits to HM3Overlay.json are read when the overlay initializes. Restart the game to reload them.

## Known Limitations
- This fork is focused on Hitman: Blood Money. The inherited Hitman 2016 mode and other engines have not been tested with the redesigned overlay.
- Administrator privileges are required for injection.
- There might be two mouse cursor appearing while trying to move around the in-game overlay. It's better to customize the overlay whenever you're playing the level.
- Currently the stats on the overlay are also being shown even when the player is not on the level.
- You might need to use a higher resolution (16:9, 1600x900 preferred) to see all the options from the overlay. 
- External edits to the overlay JSON require restarting the game.
- Other mods or overlays may conflict with injection or rendering.
- This is an unofficial fork. Please do not report fork-specific issues to the original Statman project.

## Credits
- OrfeasZ - Orfeas Zafeiris — original Statman project.
- ReGlacier - for ReHitman project, which was also helpful for this fork.
- ocornut and contributors — Dear ImGui.
- TsudaKageyu — MinHook.
- secana — PeNet.
- James Newton-King — Newtonsoft.Json.
- Microsoft — .NET Framework and DirectX SDK.

This fork is based on Statman by Orfeas Zafeiris and is licensed under the MIT license. See LICENSE for details. It is not affiliated with or endorsed by the original author or IO Interactive.

Parts of this fork were developed with AI-assisted tooling and reviewed by a human.
