#pragma once

#include <stdafx.h>
#include <Hooks.h>
#include <d3d9.h>

// NOTE: d3d9.lib is deliberately NOT linked. A static import of d3d9.dll would make
// LoadLibrary(HM3.dll) pull d3d9.dll into the game process by itself, which defeats
// the "wait until the game is ready" gate in HM3Module::Init (it checks for d3d9) and
// injects Direct3D state into a process that may still be unpacking itself (Steam DRM).
// Direct3DCreate9 is resolved lazily in HM3Hooks::HookD3D9 instead.

void Overlay_Shutdown();
void Overlay_SetZWINDOWSInstance(void* p_Instance);

struct DetectionIterator;
class ZScene;
class ZHM3LevelControl;
class ZLimitSaves;
class ZXMLGUISystem;
enum eZWUserEvents : int32_t;

class HM3Hooks
{
public:
	HM3Hooks();
	~HM3Hooks();

	/// Installs the game-code hooks only (no Direct3D work).
	bool Install();

	/// Creates the dummy D3D9 device and detours IDirect3DDevice9::EndScene/Reset.
	/// Called from HM3Module::Init once the game has loaded its first scene.
	bool InstallD3D9Hooks();

protected:
	void Uninstall();
	bool HookD3D9();

private:
	bool m_Installed;
	bool m_D3D9HooksInstalled = false;

public:
	DECLARE_THISCALL_HOOK(void, ZScene_Load, ZScene* th, const char* scene);
	DECLARE_THISCALL_HOOK(void, ZHM3LevelControl_FrameUpdate, ZHM3LevelControl* th);
	DECLARE_THISCALL_HOOK(int, ZHM3LevelControl_MissionCompleted2, ZHM3LevelControl* th);
	DECLARE_THISCALL_HOOK(float, ZSysInterface_SetTimeMultiplier, void* th, float p_Value);
	DECLARE_THISCALL_HOOK(void, ZLimitSaves_Click, ZLimitSaves* th, eZWUserEvents a2, int a3, ZXMLGUISystem* a4);
	DECLARE_THISCALL_HOOK(void, ZWINDOWS_Update, void* th);
	DECLARE_STDCALL_HOOK(HRESULT, IDirect3DDevice9__EndScene, IDirect3DDevice9* th);
	DECLARE_STDCALL_HOOK(HRESULT, IDirect3DDevice9__Reset, IDirect3DDevice9* th, D3DPRESENT_PARAMETERS* pPresentationParameters);
};