#include "HM3Hooks.h"
#include "HM3Module.h"

#include "Utils.h"

DECLARE_THISCALL_DETOUR(HM3Hooks, void, ZWINDOWS_Update, void* th)
{
	Overlay_SetZWINDOWSInstance(th);
	o_ZWINDOWS_Update(th);
}

HM3Hooks::HM3Hooks() :
	m_Installed(false)
{
	Install();
}

HM3Hooks::~HM3Hooks()
{
	Uninstall();
}

bool HM3Hooks::Install()
{
	if (m_Installed)
		return true;

	LogVerbose("Installing HM3 hooks (game module base 0x%p, code size 0x%x).\n",
		reinterpret_cast<void*>(g_Module->GetModuleBase()), g_Module->GetSizeOfCode());

	if (MH_Initialize() != MH_OK)
	{
		Log("Failed to initialize MinHook.\n");
		return false;
	}

	// Mark as installed before touching any hook so Uninstall() always gets a chance
	// to remove whatever was installed if a later step fails.
	m_Installed = true;
	
	DECLARE_PATTERN_HOOK(
		ZScene_Load,
		"\x56\x8B\x74\x24\x08\x8B\xC6\x57\xC6\x01",
		"xxxxxxxxxx"
	)

	DECLARE_PATTERN_HOOK(
		ZHM3LevelControl_FrameUpdate,
		"\x64\xA1\x00\x00\x00\x00\x6A\x00\x68\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\x56\x57\x8B\xF1\xE8\x00\x00\x00\x00\xD9\x46\x30",
		"xx????x?x????xxxx????xx????xxxxx????xxx"
	)

	DECLARE_PATTERN_HOOK(
		ZHM3LevelControl_MissionCompleted2,
		"\x55\x8B\xEC\x83\xE4\x00\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x53\x55\x56\x8B\xF1",
		"xxxxx?x?x????xx????xxxx????xx????x????xxxxx"
	)

	DECLARE_PATTERN_HOOK(
		ZLimitSaves_Click,
		"\x53\x8B\x5C\x24\x08\x56\x8B\xF1\x3B\x5E\x6C",
		"xxxxxxxxxxx"
	)

	// ZSysInterface::SetTimeMultiplier(float). Every game-driven change of the
	// time scale goes through this function, including the dying slow motion
	// that animates the multiplier down and back up.
	DECLARE_PATTERN_HOOK(
		ZSysInterface_SetTimeMultiplier,
		"\x8A\x81\x2C\x0B\x00\x00\xD9\x81\x28\x0B\x00\x00\x84\xC0\x75\x0A\x8B\x44\x24\x04\x89\x81\x28\x0B\x00\x00",
		"xxxxxxxxxxxxxxxxxxxxxxxxxx"
	)

	{
		uint8_t s_Pattern[] = "\x83\xEC\x08\x53\x55\x56\x8B\xF1\x8B\x06\x57\xFF\x90\x7C\x02\x00\x00";
		m_ZWINDOWS_Update = reinterpret_cast<void*>(Utils::SearchPattern(g_Module->GetModuleBase(), g_Module->GetSizeOfCode(), s_Pattern, "xxxxxxxxxxxxxxxxx"));

		if (m_ZWINDOWS_Update)
		{
			MH_STATUS s_Result = MH_CreateHook(m_ZWINDOWS_Update, &c_ZWINDOWS_Update, reinterpret_cast<LPVOID*>(&o_ZWINDOWS_Update));

			if (s_Result != MH_OK)
			{
				Log("Failed to create hook for function 'ZWINDOWS_Update' with status %d.\n", s_Result);
				m_ZWINDOWS_Update = nullptr;
			}
			else
			{
				s_Result = MH_EnableHook(m_ZWINDOWS_Update);

				if (s_Result != MH_OK)
					Log("Failed to install hook for 'ZWINDOWS_Update' with status %d.\n", s_Result);
				else
					Log("Successfully detoured 'ZWINDOWS_Update' at 0x%p.\n", m_ZWINDOWS_Update);
			}
		}
		else
		{
			Log("Could not find address for function 'ZWINDOWS_Update'; game cursor freeze disabled.\n");
		}
	}

	// The Direct3D hooks are installed separately, once the game has loaded its first
	// scene (see HM3Module::Init). Creating a dummy D3D9 device and detouring
	// EndScene/Reset while the game is still bringing up its renderer means our detours
	// end up inside the renderer's own initialisation, so they are deferred until the
	// renderer is known to be running.
	Log("Game hooks installed successfully.\n");

	return true;
}

bool HM3Hooks::InstallD3D9Hooks()
{
	if (m_D3D9HooksInstalled)
		return true;

	m_D3D9HooksInstalled = true;

	if (!HookD3D9())
	{
		Log("D3D9 hooks were not installed; the in-game overlay will be disabled.\n");
		return false;
	}

	Log("D3D9 hooks installed successfully.\n");
	return true;
}

bool HM3Hooks::HookD3D9()
{
	// Resolve Direct3DCreate9 lazily: see the note in HM3Hooks.h. At this point d3d9.dll
	// is present because the game itself loaded it (or because the game is already
	// running), never because of HM3.dll's import table.
	HMODULE s_D3D9Module = GetModuleHandleA("d3d9.dll");

	if (s_D3D9Module == nullptr)
	{
		Log("d3d9.dll is not loaded in the game yet. Stats overlay won't work.\n");
		return false;
	}

	auto s_Direct3DCreate9 = reinterpret_cast<decltype(&Direct3DCreate9)>(
		GetProcAddress(s_D3D9Module, "Direct3DCreate9"));

	if (s_Direct3DCreate9 == nullptr)
	{
		Log("Could not resolve Direct3DCreate9 in d3d9.dll (%lu). Stats overlay won't work.\n", GetLastError());
		return false;
	}

	// Create the D3D9 interface.
	auto* s_D3D = s_Direct3DCreate9(D3D_SDK_VERSION);

	if (!s_D3D) {
		Log("Could not create D3D9 interface. Stats overlay won't work.\n");
		return false;
	}

	// Create a window for our dummy device.
	WNDCLASSEX s_WindowClass{};
	s_WindowClass.cbSize = sizeof(WNDCLASSEX);
	s_WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	s_WindowClass.lpfnWndProc = DefWindowProc;
	s_WindowClass.lpszClassName = "Statman";
	RegisterClassEx(&s_WindowClass);

	HWND s_Window = CreateWindowEx(NULL, s_WindowClass.lpszClassName, "Statman", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, nullptr, nullptr);

	if (!s_Window) {
		Log("Could not create window for D3D9 device. Stats overlay won't work.\n");
		s_D3D->Release();
		return false;
	}

	// Create our dummy device.
	D3DPRESENT_PARAMETERS s_Params{};
	ZeroMemory(&s_Params, sizeof(s_Params));
	s_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	s_Params.hDeviceWindow = s_Window;
	s_Params.Windowed = true;

	IDirect3DDevice9* s_Device = nullptr;
	const auto s_Result = s_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, s_Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &s_Params, &s_Device);

	if (!SUCCEEDED(s_Result)) {
		Log("Could not create D3D device with error %x. Stats overlay won't work.\n", s_Result);
		DestroyWindow(s_Window);
		s_D3D->Release();
		return false;
	}

	// Get the addresses to the EndScene and Reset methods.
	const auto* s_VTable = *reinterpret_cast<uintptr_t**>(s_Device);
	const intptr_t s_EndSceneOffset = 168;
	const intptr_t s_ResetOffset = 64;
	const auto s_EndSceneAddress = s_VTable[s_EndSceneOffset / sizeof(uintptr_t)];
	const auto s_ResetAddress = s_VTable[s_ResetOffset / sizeof(uintptr_t)];

	// Install the hooks.
	DECLARE_OFFSET_HOOK(IDirect3DDevice9__EndScene, s_EndSceneAddress);
	DECLARE_OFFSET_HOOK(IDirect3DDevice9__Reset, s_ResetAddress);

	// Clean things up.
	DestroyWindow(s_Window);
	s_Device->Release();
	s_D3D->Release();

	return true;
}

void HM3Hooks::Uninstall()
{
	if (!m_Installed)
		return;
	
	m_Installed = false;

	Overlay_Shutdown();

	REMOVE_HOOK(ZScene_Load)
	REMOVE_HOOK(ZHM3LevelControl_FrameUpdate)
	REMOVE_HOOK(ZHM3LevelControl_MissionCompleted2)
	REMOVE_HOOK(ZLimitSaves_Click)
	REMOVE_HOOK(ZSysInterface_SetTimeMultiplier)
	if (m_ZWINDOWS_Update)
	{
		REMOVE_HOOK(ZWINDOWS_Update)
	}
	REMOVE_HOOK(IDirect3DDevice9__EndScene)
	REMOVE_HOOK(IDirect3DDevice9__Reset)
	
	if (MH_Uninitialize() != MH_OK)
	{
		Log("Failed to uninitialize MinHook.\n");
	}
}