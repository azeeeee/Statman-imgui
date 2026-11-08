#include "HM3Module.h"

#include "HM3Hooks.h"
#include "HM3Pointers.h"
#include "HM3Functions.h"
#include "Structs/ZHM3GameData.h"

#include <Pipeman.h>
#include <functional>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include "Utils.h"

extern HMODULE g_ModuleHandle;

HM3Module::HM3Module()
{
}

HM3Module::~HM3Module()
{
	if (m_Pipeman)
		delete m_Pipeman;

	if (m_Hooks)
		delete m_Hooks;

	if (m_Pointers)
		delete m_Pointers;

	if (m_Functions)
		delete m_Functions;

	m_Pipeman = nullptr;
	m_Hooks = nullptr;
	m_Pointers = nullptr;
	m_Functions = nullptr;

	SetEnvironmentVariable("Statman_HM3", "\x02\x00");
}

bool HM3Module::Init()
{
	if (!CheckInstance())
	{
		Log("The current stat module is already loaded. Exiting.\n");
		return false;
	}

#ifdef _DEBUG
	char s_Buffer[2];
	if (!GetEnvironmentVariableA("Statman_HM3", s_Buffer, 2) || s_Buffer[0] != 0x02)
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		SetConsoleTitleA("Statman - Debug Console");
		freopen("CON", "w", stdout);
	}
#endif

	Log("Initializing Statman HM3 stat module (built %s %s)...\n", __DATE__, __TIME__);

	SetEnvironmentVariable("Statman_HM3", "\x01\x00");

	Log("Waiting for the game to be ready (d3d9.dll)...\n");

	// Make sure the game is fully loaded. d3d9.dll has to come from the game itself:
	// HM3.dll no longer imports it statically (see HM3Hooks.h), so seeing it here means
	// the game finished initializing its renderer - i.e. for DRM protected builds the
	// executable has already unpacked itself.
	const DWORD s_ReadyStart = GetTickCount();

	while (GetModuleHandleA("d3d9.dll") == nullptr && GetModuleHandleA("d3d8.dll") == nullptr)
	{
		Sleep(10);

		if (GetTickCount() - s_ReadyStart >= 60000)
		{
			Log("Timed out after 60000 ms waiting for the game's renderer; continuing anyway.\n");
			break;
		}
	}

	{
		const DWORD s_ReadyTime = GetTickCount() - s_ReadyStart;

		Log("Game reported ready after %lu ms (d3d9=%s, d3d8=%s).\n", s_ReadyTime,
			GetModuleHandleA("d3d9.dll") ? "loaded" : "missing",
			GetModuleHandleA("d3d8.dll") ? "loaded" : "missing");
	}

	Log("Initialized!\n");

	// Get the module base address and size.
	HMODULE s_Module = GetModuleHandleA(nullptr);
	m_ModuleBase = reinterpret_cast<uintptr_t>(s_Module) + Utils::GetBaseOfCode(s_Module);
	m_SizeOfCode = Utils::GetSizeOfCode(s_Module);

	LogVerbose("Module base address: %x\n", m_ModuleBase);
	LogVerbose("Module size: %x\n", m_SizeOfCode);

	// Patch required data.
	PerformPatches();

	// Setup pointers, functions, and hooks.
	m_Pointers = new HM3Pointers();
	m_Functions = new HM3Functions();
	m_Hooks = new HM3Hooks();

	// Setup Pipeman.
	m_Pipeman = new Pipeman(R"(\\.\pipe\Statman_IPC)", "H3");
	m_Pipeman->SetMessageCallback(std::bind(&HM3Module::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_Pipeman->SetConnectedCallback([this]()
	{
		m_Pipeman->SendPipeMessage("HI", "");
	});

	// Install the Direct3D hooks only after the game has loaded its first scene, so the
	// dummy D3D9 device is created and EndScene/Reset are detoured while the renderer is
	// already running instead of during its initialisation. The timeout is a safety net
	// for start-up paths that never report a scene.
	const DWORD s_D3D9WaitStart = GetTickCount();

	Log("Waiting for the first scene load before installing D3D9 hooks (max 6000 ms)...\n");

	while (!m_FirstSceneLoaded && GetTickCount() - s_D3D9WaitStart < 6000)
		Sleep(25);

	// Let the scene transition finish before touching Direct3D.
	Sleep(750);

	Log("Installing D3D9 hooks (scene loaded=%s, waited %lu ms).\n",
		m_FirstSceneLoaded ? "yes" : "no (timeout)", GetTickCount() - s_D3D9WaitStart);

	m_Hooks->InstallD3D9Hooks();

	Log("Stat module initialization finished.\n");

	return true;
}

bool HM3Module::CheckInstance()
{
	char s_Buffer[2];
	if (GetEnvironmentVariableA("Statman_HM3", s_Buffer, 2) && s_Buffer[0] == 0x01)
		return false;

	return true;
}

void HM3Module::PerformPatches()
{
	// Intentionally empty. This is the single place for executable-version specific
	// byte patches; the original project applied its Steam/retail differences here.
}

void HM3Module::SetCheatsEnabled(bool p_Enabled)
{
	m_CheatsEnabled = p_Enabled;

	if (m_Pointers && m_Pointers->CConfiguration__m_bCheatsEnabled)
		*m_Pointers->CConfiguration__m_bCheatsEnabled = p_Enabled;
}

void HM3Module::SetUnlimitedSaves(bool p_Enabled)
{
	m_UnlimitedSaves = p_Enabled;
}

void HM3Module::SetHitman2016Mode(bool p_Enabled)
{
	m_Hitman2016Mode = p_Enabled;
}

void HM3Module::SetDebug2016(bool p_Enabled)
{
	m_Debug2016 = p_Enabled;
}

CheatState HM3Module::GetCheatState() const
{
	CheatState s_State;

	if (!m_Pointers)
		return s_State;

	const auto s_IsSet = [](const int32_t* p_Flag)
	{
		return p_Flag != nullptr && *p_Flag != 0;
	};

	s_State.EnableCheats = m_Pointers->CConfiguration__m_bCheatsEnabled != nullptr
		&& *m_Pointers->CConfiguration__m_bCheatsEnabled;

	s_State.GodMode = s_IsSet(m_Pointers->g_bGodMode);
	s_State.InvisibleMode = s_IsSet(m_Pointers->g_bInvisibleMode);
	s_State.ShowEnemyVision = s_IsSet(m_Pointers->g_bShowEnemyVision);
	s_State.InfClip = s_IsSet(m_Pointers->g_bInfClip);

	// The OSD is expected to be on; only treat it as disabled when the game
	// actually exposes the flag. This keeps the state default at "on" if the
	// optional pattern was not found.
	if (m_Pointers->g_bShowOSD)
		s_State.ShowOSD = *m_Pointers->g_bShowOSD != 0;

	if (m_Pointers->g_pSysInterface && *m_Pointers->g_pSysInterface)
	{
		auto* s_SysInterface = *m_Pointers->g_pSysInterface;

		// Infinite ammo is a field of the engine data base, not a standalone global.
		if (s_SysInterface->m_pEngineDb)
		{
			s_State.InfAmmo = *reinterpret_cast<int32_t*>(
				reinterpret_cast<char*>(s_SysInterface->m_pEngineDb) + 0xB4) != 0;
		}

		s_State.TimeMultiplier = *reinterpret_cast<float*>(
			reinterpret_cast<char*>(s_SysInterface) + 0xB28);
	}

	return s_State;
}

void HM3Module::SetCheatState(const CheatState& p_State)
{
	if (!m_Pointers)
		return;

	if (m_Pointers->CConfiguration__m_bCheatsEnabled)
		*m_Pointers->CConfiguration__m_bCheatsEnabled = p_State.EnableCheats;

	const auto s_WriteFlag = [](int32_t* p_Flag, bool p_Enabled)
	{
		if (p_Flag)
			*p_Flag = p_Enabled ? 1 : 0;
	};

	s_WriteFlag(m_Pointers->g_bGodMode, p_State.GodMode);
	s_WriteFlag(m_Pointers->g_bInvisibleMode, p_State.InvisibleMode);
	s_WriteFlag(m_Pointers->g_bShowEnemyVision, p_State.ShowEnemyVision);
	s_WriteFlag(m_Pointers->g_bShowOSD, p_State.ShowOSD);
	s_WriteFlag(m_Pointers->g_bInfClip, p_State.InfClip);

	if (m_Pointers->g_pSysInterface && *m_Pointers->g_pSysInterface)
	{
		auto* s_SysInterface = *m_Pointers->g_pSysInterface;

		if (s_SysInterface->m_pEngineDb)
		{
			*reinterpret_cast<int32_t*>(
				reinterpret_cast<char*>(s_SysInterface->m_pEngineDb) + 0xB4) = p_State.InfAmmo ? 1 : 0;
		}

		*reinterpret_cast<float*>(
			reinterpret_cast<char*>(s_SysInterface) + 0xB28) = p_State.TimeMultiplier;
	}

	// Keep the Statman-side flag in sync so a later scene load does not clear it.
	m_CheatsEnabled = p_State.EnableCheats;
}

bool HM3Module::CheatsActive() const
{
	if (m_CheatsEnabled || m_Debug2016)
		return true;

	const CheatState s_State = GetCheatState();

	return s_State.EnableCheats
		|| s_State.InfAmmo
		|| s_State.InfClip
		|| s_State.GodMode
		|| s_State.InvisibleMode
		|| s_State.ShowEnemyVision
		|| !s_State.ShowOSD
		|| s_State.TimeMultiplier < 0.999f
		|| s_State.TimeMultiplier > 1.001f;
}

bool HM3Module::ApplyStartingLocation(const LevelStartingLocation& p_Location)
{
	if (!m_Pointers || !m_Functions || !m_Pointers->g_pGameData || !*m_Pointers->g_pGameData)
		return false;

	auto* s_GameData = *m_Pointers->g_pGameData;

	if (!s_GameData || !s_GameData->m_pHitman)
		return false;

	auto* s_Hitman = s_GameData->m_pHitman;

	if (p_Location.Outfit >= 0)
		ApplyOutfit(p_Location.Outfit);

	if (s_Hitman->m_pBaseGeom && s_Hitman->m_pBaseGeom->m_pParent)
	{
		s_Hitman->m_pBaseGeom->m_pParent->m_mMat = p_Location.Rotation;
		s_Hitman->m_pBaseGeom->m_pParent->m_vPos = p_Location.Position;
	}

	return true;
}

bool HM3Module::ApplyOutfit(int p_ActorIndex)
{
	if (!m_Pointers || !m_Functions || !m_Pointers->g_pGameData || !*m_Pointers->g_pGameData)
		return false;

	auto* s_GameData = *m_Pointers->g_pGameData;

	if (!s_GameData || !s_GameData->m_pHitman)
		return false;

	if (p_ActorIndex < 0 || static_cast<uint32_t>(p_ActorIndex) >= s_GameData->m_nActorCount)
		return false;

	auto* s_Actor = s_GameData->m_aActors[p_ActorIndex];

	if (!s_Actor)
		return false;

	auto* s_Hitman = s_GameData->m_pHitman;

	s_Actor->EnablePickupClothes();

	const auto s_HitmanAs = s_Actor->GetHitmanAs();

	if (!s_HitmanAs)
		return false;

	auto* s_HitmanAsPtr = reinterpret_cast<ZHM3HmAs*>(m_Functions->ZGEOM_RefToPtr(s_HitmanAs));

	if (!s_HitmanAsPtr)
		return false;

	m_Functions->ZHitman3_ChangeIntoNewClothes(s_Hitman, s_HitmanAsPtr, nullptr, false, nullptr, 0);

	if (m_Pointers->g_pSysInterface && *m_Pointers->g_pSysInterface && m_Pointers->ZLIST__m_TrackLinkObjectsInstance)
	{
		auto s_Msg = (*m_Pointers->g_pSysInterface)->m_pEngineDb->RegisterZMsg("Hitman_ChangedClothes", 0, nullptr, 0);
		(*m_Pointers->ZLIST__m_TrackLinkObjectsInstance)->SendCommandRecursive(s_Msg, nullptr, nullptr);
	}

	return true;
}

bool HM3Module::GetPlayerTransform(SMatrix33& p_Rotation, SVector3& p_Position) const
{
	if (!m_Pointers || !m_Pointers->g_pGameData || !*m_Pointers->g_pGameData)
		return false;

	auto* s_GameData = *m_Pointers->g_pGameData;

	if (!s_GameData || !s_GameData->m_pHitman)
		return false;

	auto* s_Hitman = s_GameData->m_pHitman;

	if (!s_Hitman->m_pBaseGeom || !s_Hitman->m_pBaseGeom->m_pParent)
		return false;

	p_Rotation = s_Hitman->m_pBaseGeom->m_pParent->m_mMat;
	p_Position = s_Hitman->m_pBaseGeom->m_pParent->m_vPos;
	return true;
}

int HM3Module::GetCurrentOutfitIndex() const
{
	if (!m_Pointers || !m_Functions || !m_Pointers->g_pGameData || !*m_Pointers->g_pGameData)
		return -1;

	auto* s_GameData = *m_Pointers->g_pGameData;

	if (!s_GameData || !s_GameData->m_pHitman)
		return -1;

	auto* s_Hitman = s_GameData->m_pHitman;

	if (!s_Hitman->m_Unknown01)
		return -1;

	for (uint32_t i = 0; i < s_GameData->m_nActorCount; ++i)
	{
		auto* s_Actor = s_GameData->m_aActors[i];

		if (!s_Actor)
			continue;

		const auto s_HitmanAs = s_Actor->GetHitmanAs();

		if (!s_HitmanAs)
			continue;

		if (m_Functions->ZGEOM_RefToPtr(s_HitmanAs) == s_Hitman->m_Unknown01)
			return static_cast<int>(i);
	}

	return -1;
}

std::string HM3Module::GetCurrentLevelKey() const
{
	std::string s_Scene = m_SceneName;
	std::transform(s_Scene.begin(), s_Scene.end(), s_Scene.begin(), [](unsigned char p_Char)
	{
		return static_cast<char>(std::tolower(p_Char));
	});
	std::replace(s_Scene.begin(), s_Scene.end(), '/', '\\');

	for (const auto& [s_Level, s_Locations] : m_LevelStartingLocations)
	{
		std::string s_LevelLower = s_Level;
		std::transform(s_LevelLower.begin(), s_LevelLower.end(), s_LevelLower.begin(), [](unsigned char p_Char)
		{
			return static_cast<char>(std::tolower(p_Char));
		});

		if (s_Scene.find(s_LevelLower) != std::string::npos)
			return s_Level;
	}

	return {};
}

bool HM3Module::AddOrUpdateLocation(const std::string& p_Level, const LevelStartingLocation& p_Location)
{
	auto& s_Locations = m_LevelStartingLocations[p_Level];

	for (auto& s_Location : s_Locations)
	{
		if (s_Location.Name == p_Location.Name)
		{
			s_Location = p_Location;
			return true;
		}
	}

	s_Locations.push_back(p_Location);
	return true;
}

bool HM3Module::DeleteLocation(const std::string& p_Level, const std::string& p_Name)
{
	const auto s_LevelIt = m_LevelStartingLocations.find(p_Level);

	if (s_LevelIt == m_LevelStartingLocations.end())
		return false;

	auto& s_Locations = s_LevelIt->second;
	s_Locations.erase(
		std::remove_if(s_Locations.begin(), s_Locations.end(), [&](const LevelStartingLocation& p_Location)
		{
			return p_Location.Name == p_Name;
		}),
		s_Locations.end());

	if (m_LevelLastLocations[p_Level] == p_Name)
		m_LevelLastLocations.erase(p_Level);

	return true;
}

bool HM3Module::SetLastLocation(const std::string& p_Level, const std::string& p_Name)
{
	m_LevelLastLocations[p_Level] = p_Name;
	return true;
}

std::string HM3Module::GetLastLocation(const std::string& p_Level) const
{
	const auto s_It = m_LevelLastLocations.find(p_Level);
	return s_It != m_LevelLastLocations.end() ? s_It->second : std::string();
}

// Formats a filesystem path for the log. path::string() can throw for paths
// that contain characters outside the current locale, so convert explicitly.
std::string ToUtf8(const std::filesystem::path& p_Path)
{
	const std::wstring s_Wide = p_Path.wstring();

	if (s_Wide.empty())
		return {};

	const int s_Size = WideCharToMultiByte(CP_UTF8, 0, s_Wide.c_str(),
		static_cast<int>(s_Wide.size()), nullptr, 0, nullptr, nullptr);

	if (s_Size <= 0)
		return {};

	std::string s_Result(static_cast<size_t>(s_Size), '\0');

	WideCharToMultiByte(CP_UTF8, 0, s_Wide.c_str(), static_cast<int>(s_Wide.size()),
		s_Result.data(), s_Size, nullptr, nullptr);

	return s_Result;
}

// HM3_locations.json lives in the config folder next to Statman.exe so the
// application folder only contains executables and DLLs. Older builds kept it
// next to HM3.dll; that file is moved on first use to keep saved locations.
std::filesystem::path GetHitman2016DataPath()
{
	wchar_t s_ModulePath[MAX_PATH] = { 0 };

	if (!GetModuleFileNameW(g_ModuleHandle, s_ModulePath, MAX_PATH))
		return {};

	return std::filesystem::path(s_ModulePath).parent_path() / L"config" / L"HM3_locations.json";
}

bool HM3Module::SaveHitman2016Data()
{
	nlohmann::json s_Json;

	for (const auto& [s_Level, s_Locations] : m_LevelStartingLocations)
	{
		auto& s_LevelJson = s_Json[s_Level];

		if (!s_LevelJson.is_object())
			s_LevelJson = nlohmann::json::object();

		const auto s_LastIt = m_LevelLastLocations.find(s_Level);

		if (s_LastIt != m_LevelLastLocations.end() && !s_LastIt->second.empty())
			s_LevelJson["__last"] = s_LastIt->second;

		for (const auto& s_Location : s_Locations)
		{
			auto& s_LocationJson = s_LevelJson[s_Location.Name];

			s_LocationJson["position"] = {
				{ s_Location.Rotation.xAxis.x, s_Location.Rotation.xAxis.y, s_Location.Rotation.xAxis.z },
				{ s_Location.Rotation.yAxis.x, s_Location.Rotation.yAxis.y, s_Location.Rotation.yAxis.z },
				{ s_Location.Rotation.zAxis.x, s_Location.Rotation.zAxis.y, s_Location.Rotation.zAxis.z },
				{ s_Location.Position.x, s_Location.Position.y, s_Location.Position.z }
			};

			s_LocationJson["outfit"] = s_Location.Outfit;
		}
	}

	const auto s_Path = GetHitman2016DataPath();

	if (s_Path.empty())
	{
		Log("Could not resolve the Hitman 2016 data path.\n");
		return false;
	}

	std::error_code s_DirectoryError;
	std::filesystem::create_directories(s_Path.parent_path(), s_DirectoryError);

	std::ofstream s_File(s_Path);

	if (!s_File.is_open())
	{
		Log("Could not write '%s'.\n", ToUtf8(s_Path).c_str());
		return false;
	}

	s_File << s_Json.dump(2);
	s_File.close();

	Log("Saved Hitman 2016 locations to '%s'.\n", ToUtf8(s_Path).c_str());
	return true;
}

// Moves HM3_locations.json from the application folder into the config folder
// the first time this build runs so saved locations survive the upgrade.
void MigrateLegacyHitman2016Data(const std::filesystem::path& p_Target)
{
	std::error_code s_Error;

	if (std::filesystem::exists(p_Target, s_Error))
		return;

	wchar_t s_ModulePath[MAX_PATH] = { 0 };

	if (!GetModuleFileNameW(g_ModuleHandle, s_ModulePath, MAX_PATH))
		return;

	std::filesystem::path s_LegacyPath(s_ModulePath);
	s_LegacyPath.replace_filename(L"HM3_locations.json");

	if (!std::filesystem::exists(s_LegacyPath, s_Error))
		return;

	std::filesystem::create_directories(p_Target.parent_path(), s_Error);
	std::filesystem::rename(s_LegacyPath, p_Target, s_Error);

	if (s_Error)
		std::filesystem::copy_file(s_LegacyPath, p_Target, std::filesystem::copy_options::overwrite_existing, s_Error);
}

bool HM3Module::ReloadHitman2016Data()
{
	const auto s_Path = GetHitman2016DataPath();

	if (s_Path.empty())
		return false;

	MigrateLegacyHitman2016Data(s_Path);

	std::ifstream s_File(s_Path);

	if (!s_File.is_open())
		return false;

	std::stringstream s_Buffer;
	s_Buffer << s_File.rdbuf();
	ParseHitman2016Data(s_Buffer.str());
	return true;
}

void HM3Module::OnMessage(const std::string& p_Type, const std::string& p_Content)
{
	// Payload contents are deliberately not logged: the Hitman 2016 message ('2J')
	// carries the whole HM3_locations.json and would flood the log on every launch.
	LogVerbose("Got message '%s' from IPC (%zu bytes).\n", p_Type.c_str(), p_Content.size());

	if (p_Type == "EC")
	{
		SetCheatsEnabled(p_Content == "true");
		return;
	}

	if (p_Type == "US")
	{
		SetUnlimitedSaves(p_Content == "true");
		return;
	}

	if (p_Type == "H2")
	{
		SetHitman2016Mode(p_Content == "true");
		return;
	}

	if (p_Type == "OV")
	{
		// Overlay toggle
		m_OverlayEnabled = p_Content == "true";
		return;
	}

	if (p_Type == "2D")
	{
		SetDebug2016(p_Content == "true");
		return;
	}

	if (p_Type == "2J")
	{
		// Parse Hitman 2016 data.
		ParseHitman2016Data(p_Content);
		return;
	}

	if (p_Type == "VL")
	{
		// Runtime verbose logging toggle from the Statman UI.
		const bool s_Enabled = p_Content == "true";

		StatmanSetLogVerbose(s_Enabled);
		Log("Verbose logging %s.\n", s_Enabled ? "enabled" : "disabled");
		return;
	}

	if (p_Type == "XX")
	{
		Log("Statman requested exit.\n");

		std::thread s_ExitThread([]()
		{
			delete g_Module;
			g_Module = nullptr;

			if (g_ModuleHandle)
				FreeLibraryAndExitThread(g_ModuleHandle, 0);
		});

		s_ExitThread.detach();

		return;
	}
}
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

void HM3Module::ParseHitman2016Data(const std::string& p_Json) {
	try {
		m_LevelStartingLocations.clear();
		m_LevelLastLocations.clear();
		
		nlohmann::json s_Json = nlohmann::json::parse(p_Json);

		if (s_Json.is_null())
			return;

		// Parse level starting locations.
		for (auto& [s_Level, s_Locations] : s_Json.items()) {
			if (!s_Locations.is_object()) {
				m_LevelStartingLocations[s_Level] = {};
				continue;
			}

			std::vector<LevelStartingLocation> s_LevelLocations;

			for (auto& [s_Name, s_Location] : s_Locations.items()) {
				// Per-level metadata: the location that should be restored when
				// the level is restarted.
				if (s_Name == "__last" && s_Location.is_string()) {
					m_LevelLastLocations[s_Level] = s_Location.get<std::string>();
					continue;
				}

				if (!s_Location.is_object() || s_Location.find("position") == s_Location.end())
					continue;

				const auto s_Positions = s_Location["position"];

				if (!s_Positions.is_array() || s_Positions.size() < 4)
					continue;

				LevelStartingLocation s_LocationData;
				s_LocationData.Name = s_Name;

				s_LocationData.Rotation = {
					.xAxis = { s_Positions[0][0], s_Positions[0][1], s_Positions[0][2] },
					.yAxis = { s_Positions[1][0], s_Positions[1][1], s_Positions[1][2] },
					.zAxis = { s_Positions[2][0], s_Positions[2][1], s_Positions[2][2] }
				};

				s_LocationData.Position = {
					s_Positions[3][0],
					s_Positions[3][1],
					s_Positions[3][2]
				};

				s_LocationData.Outfit = s_Location.value("outfit", -1);

				s_LevelLocations.push_back(s_LocationData);
			}

			m_LevelStartingLocations[s_Level] = s_LevelLocations;
		}
	}
	catch (std::exception& e) {
		Log("Could not parse Hitman 2016 data: %s\n", e.what());
	}
}