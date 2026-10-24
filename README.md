# Statman - Unofficial HM3 In-Game Overlay Fork
Not affiliated with or endorsed by OrfeasZ or IO Interactive.

An attempt of improving the in-game overlay from the original OrfeasZ's version.

Heavily AI-Assisted code. Just like this README. 

Don't be afraid to call it a vibe coded slop, because it is, in fact, a vibe coded slop. But it does its job. 

## Please do not report fork-specific issues to the original Statman project.

Forked from: https://github.com/OrfeasZ/Statman

## What's Different

This fork keeps Statman's original HM3 stat tracking and rating logic, but replaces the in-game display with a redesigned ImGui/D3D9 overlay.

- The HUD is split into movable panels: left stats, right stats, rating, timer, and level info.
- Added a top-left toolbar that appears whenever you press INSERT. There you can select three windows: Show Editor, Show Game, Show Settings.
- Added Overlay Editor for toggling HUD movable panels. You can also toggle on/off showing statistics only in game.
- Added Overlay Settings window for visibility, fonts, sizes, colors, and row spacing.
- Added Game window where you can toggle: EnableCheats flag, Unlimited saves, Hitman 2016 mode & its debug info.
- Added HUD indicators when Unlimited saves or Cheats are enabled. 

- Panels can be repositioned while cursor capture is active. Positions persist between sessions. `INSERT` toggles cursor capture. 
- Overlay settings are stored in `%APPDATA%\Statman\HM3Overlay.json` and created automatically on first use.
- Fonts are loaded from the Windows Fonts folder.
- Hitman 2016 debug information is now rendered through the ImGui overlay.

## Video showcase & Screenshots

[Click here to watch a video](https://www.youtube.com/watch?v=i1WjK6aLTNU)

![Overlay](https://i.imgur.com/6h4o83K.jpeg)

![Overlay Menu](https://i.imgur.com/cHSAuXc.jpeg)

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
1. Open Statman.sln in Visual Studio.
2. Restore NuGet packages if Visual Studio does not do it automatically.
3. Select Release | x86.
4. Build the whole solution.
5. The output is written to _build\bin32\Release\, including Statman.exe and HM3.dll.
   

## Usage
1. Run `Statman.exe`.
2. Start Hitman: Blood Money.
3. Make sure the "In-Game overlay" option is enabled in Statman - Right click the top of the Statman window and then select "In-game overlay".
4. Press `INSERT` in-game to toggle cursor capture for the overlay.
5. While cursor capture is active, use the top-left toolbar to open the `Overlay editor`, `Overlay settings`, `Game` windows.
6. Drag HUD panels to reposition them. Changes apply immediately. You can reset positions to the default values in Overlay Editor window. 
7. Press `INSERT` again to release the cursor.

If the overlay does not appear, you might try to run Statman as administrator, make sure the overlay option is enabled, and `HM3.dll` is next to `Statman.exe`.

## Configuration Paths
- statman.json — stored next to Statman.exe. Contains application options such as dark mode and HM3 features like cheats, unlimited saves, 2016 mode, debug mode, and the overlay toggle.
- %APPDATA%\Statman\HM3Overlay.json — contains overlay visibility, panel positions, fonts, sizes, and colors. It is created automatically on first overlay initialization. Delete it to reset, or use the editor's reset buttons.
- HM3_locations.json — used by the inherited Hitman 2016 mode. It is not required for normal HM3 tracking.
External edits to HM3Overlay.json are read when the overlay initializes. Restart the game to reload them.

## Known Limitations
- This fork is focused on Hitman: Blood Money (HM3 engine). HM5/HM52021 engines are removed. 
- The inherited Hitman 2016 mode has not been tested with the redesigned overlay.
- Administrator privileges might be required, e.g. for proper overlay injection into the game.
- External edits to the overlay JSON require restarting the game.
- Other mods or overlays may potentially conflict with injection or rendering.

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

  
