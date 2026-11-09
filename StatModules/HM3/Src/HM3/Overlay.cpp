#include <HM3/HM3Hooks.h>
#include <HM3/HM3Module.h>
#include <HM3/HM3Pointers.h>
#include <HM3/HM3Functions.h>
#include <HM3/Roulette.h>
#include <HM3/Structs/HM3Stats.h>
#include <HM3/Structs/ZHM3GameData.h>
#include <HM3/Structs/ZRender.h>

#include <Pipeman.h>

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <tuple>
#include <format>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <json.hpp>

#include <DirectXMath.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern HMODULE g_ModuleHandle;

#include "Structs/ZHM3Actor.h"

using namespace DirectX;

static const double c_Multipliers[] = {
	0.0, 1.0, 1.5, 2.0, 2.5, 3.0, 3.25, 3.5, 3.75, 4.0, 4.1300001, 4.25, 4.3800001, 4.5, 4.6300001, 4.75,
	4.8800001, 5.0, 5.0599999, 5.1300001, 5.1900001, 5.25, 5.3099999, 5.3800001, 5.4400001, 5.5, 5.5599999,
	5.6300001, 5.6900001, 5.75, 5.8099999, 5.8800001, 5.9400001, 6.0
};

uint32_t CalculateRating0() {
	const auto* s_Stats = g_Module->Pointers()->ZHM3LevelControl__m_stats;

	double s_Rating0 = 0.0;

	// Innocents Killed
	int s_InnocentsKilled = s_Stats->m_InnocentsKilled >= 34 ? 33 : s_Stats->m_InnocentsKilled;
	s_Rating0 += c_Multipliers[s_InnocentsKilled] * 12.0;

	// Innocents Wounded
	int s_InnocentsWounded = s_Stats->m_InnocentsWounded >= 34 ? 33 : s_Stats->m_InnocentsWounded;
	s_Rating0 += c_Multipliers[s_InnocentsWounded] * 6.0;

	// Enemies Killed
	int s_EnemiesKilled = s_Stats->m_EnemiesKilled >= 34 ? 33 : s_Stats->m_EnemiesKilled;
	s_Rating0 += c_Multipliers[s_EnemiesKilled] * 6.0;

	// Enemies Wounded
	int s_EnemiesWounded = s_Stats->m_EnemiesWounded >= 34 ? 33 : s_Stats->m_EnemiesWounded;
	s_Rating0 += c_Multipliers[s_EnemiesWounded] * 3.0;

	// Policemen Killed
	int s_PoliceMenKilled = s_Stats->m_PoliceMenKilled >= 34 ? 33 : s_Stats->m_PoliceMenKilled;
	s_Rating0 += c_Multipliers[s_PoliceMenKilled] * 9.0;

	// Policemen Wounded
	int s_PoliceMenWounded = s_Stats->m_PoliceMenWounded >= 34 ? 33 : s_Stats->m_PoliceMenWounded;
	s_Rating0 += c_Multipliers[s_PoliceMenWounded] * 5.0;

	// Max Rating is 100
	s_Rating0 = s_Rating0 > 100.0 ? 100.0 : s_Rating0;

	return static_cast<uint32_t>(ceil((round(s_Rating0) / 100.0) * 6.0));
}

double CalculateRating1Raw() {
	const auto* s_Stats = g_Module->Pointers()->ZHM3LevelControl__m_stats;
	const auto* s_GameData = *g_Module->Pointers()->g_pGameData;

	double s_Rating1 = 0.0;

	// Frisk Failed
	int s_FriskFailed = s_Stats->m_FriskFailed >= 34 ? 33 : s_Stats->m_FriskFailed;
	s_Rating1 += c_Multipliers[s_FriskFailed] * 6.0;

	// Covers Blown
	int s_CoverBlown = s_Stats->m_CoverBlown >= 34 ? 33 : s_Stats->m_CoverBlown;
	s_Rating1 += c_Multipliers[s_CoverBlown] * 6.0;

	// Bodies Found
	int s_BodiesFound;
	if (s_GameData->m_nDifficulty <= 1)
		s_BodiesFound = s_Stats->m_BodiesFound >= 34 ? 33 : s_Stats->m_BodiesFound;
	else
		s_BodiesFound = (s_Stats->m_BodiesFound + s_Stats->m_TargetBodiesFound) >= 34 ? 33 : (s_Stats->m_BodiesFound + s_Stats->m_TargetBodiesFound);

	s_Rating1 += c_Multipliers[s_BodiesFound] * 6.0;

	// Unconscious Bodies Found
	int s_UnconsciousBodiesFound = s_Stats->m_UnconsciousBodiesFound >= 34 ? 33 : s_Stats->m_UnconsciousBodiesFound;
	s_Rating1 += c_Multipliers[s_UnconsciousBodiesFound] * 6.0;

	// Witnesses
	int s_Witnesses = s_Stats->m_Witnesses >= 34 ? 33 : s_Stats->m_Witnesses;
	s_Rating1 += c_Multipliers[s_Witnesses] * 8.0;

	// Caught on Camera
	int s_CameraCaught = s_Stats->m_CameraCaught >= 34 ? 33 : s_Stats->m_CameraCaught;
	s_Rating1 += c_Multipliers[s_CameraCaught] * 10.0;

	// Items Left
	if (s_GameData->m_nDifficulty == 3)
	{
		if (s_Stats->m_CustomWeaponsLeftOnLevel > 0)
			s_Rating1 += 5.0;

		if (s_Stats->m_SuitLeftOnLevel)
			s_Rating1 += 5.0;
	}

	// Max Rating is 100
	return s_Rating1 > 100.0 ? 100.0 : s_Rating1;
}

uint32_t CalculateRating1() {
	return static_cast<uint32_t>(ceil((round(CalculateRating1Raw()) / 100.0) * 6.0));
}

// Names of the special ratings the game can write to HM3Stats::m_SpecialRating.
// The array index corresponds to the ID stored in that field.
const char* GetSpecialRatingName(int32_t p_Rating)
{
	static const char* s_Names[] = {
		"Needle Pumper",
		"The Russian Hare",
		"Insurance Agent",
		"Pianoman",
		"Garrotteer",
		"Insane Florist",
		"Sushi Chef",
		"King of the Jungle",
		"Prince of Darkness",
		"Beelzebub",
		"Swashbuckler",
		"Dr. Death",
		"Skull Splitter",
		"Raging Bluecollar",
		"Frenzied Firefighter",
		"Evil Eye Gouger",
		"Murderous Manicurist",
		"Slugger",
		"Electrocuter"
	};

	constexpr int s_Count = static_cast<int>(sizeof(s_Names) / sizeof(s_Names[0]));

	if (p_Rating >= 1 && p_Rating <= s_Count)
		return s_Names[p_Rating - 1];

	return nullptr;
}

std::tuple<bool, const char*> GetRating() {
	const auto s_Rating0 = CalculateRating0();
	const auto s_Rating1 = CalculateRating1();
	const auto s_PerfectRating = (s_Rating0 + s_Rating1) == 0;

	if (s_Rating0 == 0 && s_Rating1 == 0) {
		return std::make_tuple(s_PerfectRating, "Silent Assassin");
	}
	else if (s_Rating0 == 0 && s_Rating1 == 1) {
		return std::make_tuple(s_PerfectRating, "Professional");
	}
	else if (s_Rating0 == 0 && s_Rating1 == 2) {
		return std::make_tuple(s_PerfectRating, "Hired Killer");
	}
	else if (s_Rating0 == 0 && s_Rating1 == 3) {
		return std::make_tuple(s_PerfectRating, "Murderer");
	}
	else if (s_Rating0 == 0 && s_Rating1 == 4) {
		return std::make_tuple(s_PerfectRating, "Mugger");
	}
	else if (s_Rating0 == 0 && s_Rating1 == 5) {
		return std::make_tuple(s_PerfectRating, "Messy Murderer");
	}
	else if (s_Rating0 == 0 && s_Rating1 == 6) {
		return std::make_tuple(s_PerfectRating, "Exhibitionist");
	}
	else if (s_Rating0 == 1 && s_Rating1 == 0) {
		return std::make_tuple(s_PerfectRating, "Hitman");
	}
	else if (s_Rating0 == 1 && s_Rating1 == 1) {
		return std::make_tuple(s_PerfectRating, "Specialist");
	}
	else if (s_Rating0 == 1 && s_Rating1 == 2) {
		return std::make_tuple(s_PerfectRating, "Contract Killer");
	}
	else if (s_Rating0 == 1 && s_Rating1 == 3) {
		return std::make_tuple(s_PerfectRating, "Gangster");
	}
	else if (s_Rating0 == 1 && s_Rating1 == 4) {
		return std::make_tuple(s_PerfectRating, "Perpetrator");
	}
	else if (s_Rating0 == 1 && s_Rating1 == 5) {
		return std::make_tuple(s_PerfectRating, "Fierce Offender");
	}
	else if (s_Rating0 == 1 && s_Rating1 == 6) {
		return std::make_tuple(s_PerfectRating, "Clumsy Killer");
	}
	else if (s_Rating0 == 2 && s_Rating1 == 0) {
		return std::make_tuple(s_PerfectRating, "Stalker");
	}
	else if (s_Rating0 == 2 && s_Rating1 == 1) {
		return std::make_tuple(s_PerfectRating, "The Ghost");
	}
	else if (s_Rating0 == 2 && s_Rating1 == 2) {
		return std::make_tuple(s_PerfectRating, "Hoodlum");
	}
	else if (s_Rating0 == 2 && s_Rating1 == 3) {
		return std::make_tuple(s_PerfectRating, "The Cleaner");
	}
	else if (s_Rating0 == 2 && s_Rating1 == 4) {
		return std::make_tuple(s_PerfectRating, "Thug");
	}
	else if (s_Rating0 == 2 && s_Rating1 == 5) {
		return std::make_tuple(s_PerfectRating, "Violent Thug");
	}
	else if (s_Rating0 == 2 && s_Rating1 == 6) {
		return std::make_tuple(s_PerfectRating, "Assailant");
	}
	else if (s_Rating0 == 3 && s_Rating1 == 0) {
		return std::make_tuple(s_PerfectRating, "Shadow Killer");
	}
	else if (s_Rating0 == 3 && s_Rating1 == 1) {
		return std::make_tuple(s_PerfectRating, "Phantom Killer");
	}
	else if (s_Rating0 == 3 && s_Rating1 == 2) {
		return std::make_tuple(s_PerfectRating, "The Eraser");
	}
	else if (s_Rating0 == 3 && s_Rating1 == 3) {
		return std::make_tuple(s_PerfectRating, "Killer");
	}
	else if (s_Rating0 == 3 && s_Rating1 == 4) {
		return std::make_tuple(s_PerfectRating, "Lunatic");
	}
	else if (s_Rating0 == 3 && s_Rating1 == 5) {
		return std::make_tuple(s_PerfectRating, "Madman");
	}
	else if (s_Rating0 == 3 && s_Rating1 == 6) {
		return std::make_tuple(s_PerfectRating, "Maniac");
	}
	else if (s_Rating0 == 4 && s_Rating1 == 0) {
		return std::make_tuple(s_PerfectRating, "Invisible killer");
	}
	else if (s_Rating0 == 4 && s_Rating1 == 1) {
		return std::make_tuple(s_PerfectRating, "The Undertaker");
	}
	else if (s_Rating0 == 4 && s_Rating1 == 2) {
		return std::make_tuple(s_PerfectRating, "Lean Killer");
	}
	else if (s_Rating0 == 4 && s_Rating1 == 3) {
		return std::make_tuple(s_PerfectRating, "Killer");
	}
	else if (s_Rating0 == 4 && s_Rating1 == 4) {
		return std::make_tuple(s_PerfectRating, "Loose Cannon");
	}
	else if (s_Rating0 == 4 && s_Rating1 == 5) {
		return std::make_tuple(s_PerfectRating, "Mad Butcher");
	}
	else if (s_Rating0 == 4 && s_Rating1 == 6) {
		return std::make_tuple(s_PerfectRating, "Berserker");
	}
	else if (s_Rating0 == 5 && s_Rating1 == 0) {
		return std::make_tuple(s_PerfectRating, "Serial Killer");
	}
	else if (s_Rating0 == 5 && s_Rating1 == 1) {
		return std::make_tuple(s_PerfectRating, "Serial Killer");
	}
	else if (s_Rating0 == 5 && s_Rating1 == 2) {
		return std::make_tuple(s_PerfectRating, "Deranged Slayer");
	}
	else if (s_Rating0 == 5 && s_Rating1 == 3) {
		return std::make_tuple(s_PerfectRating, "Terrorist");
	}
	else if (s_Rating0 == 5 && s_Rating1 == 4) {
		return std::make_tuple(s_PerfectRating, "Armed Madman");
	}
	else if (s_Rating0 == 5 && s_Rating1 == 5) {
		return std::make_tuple(s_PerfectRating, "Psychopath");
	}
	else if (s_Rating0 == 5 && s_Rating1 == 6) {
		return std::make_tuple(s_PerfectRating, "Psychopath");
	}
	else if (s_Rating0 == 6 && s_Rating1 == 0) {
		return std::make_tuple(s_PerfectRating, "Sociopath");
	}
	else if (s_Rating0 == 6 && s_Rating1 == 1) {
		return std::make_tuple(s_PerfectRating, "Serial Killer");
	}
	else if (s_Rating0 == 6 && s_Rating1 == 2) {
		return std::make_tuple(s_PerfectRating, "Deranged Slayer");
	}
	else if (s_Rating0 == 6 && s_Rating1 == 3) {
		return std::make_tuple(s_PerfectRating, "Terrorist");
	}
	else if (s_Rating0 == 6 && s_Rating1 == 4) {
		return std::make_tuple(s_PerfectRating, "Psychopath");
	}
	else if (s_Rating0 == 6 && s_Rating1 == 5) {
		return std::make_tuple(s_PerfectRating, "Psychopath");
	}
	else if (s_Rating0 == 6 && s_Rating1 == 6) {
		return std::make_tuple(s_PerfectRating, "Mass Murderer");
	}
	else {
		return std::make_tuple(s_PerfectRating, "No Rating");
	}
}

std::string GetRatingIssues() {
	const auto* s_Stats = g_Module->Pointers()->ZHM3LevelControl__m_stats;
	const auto* s_GameData = *g_Module->Pointers()->g_pGameData;

	std::string s_Issues = "";

	if (s_Stats->m_InnocentsKilled > 0 || s_Stats->m_EnemiesKilled > 0 || s_Stats->m_PoliceMenKilled > 0) {
		s_Issues += " | NPCs Killed";
	}

	if (s_Stats->m_InnocentsWounded > 0 || s_Stats->m_EnemiesWounded > 0 || s_Stats->m_PoliceMenWounded > 0) {
		s_Issues += " | NPCs Wounded";
	}

	if (s_Stats->m_FriskFailed > 0) {
		s_Issues += " | Frisk Failed";
	}

	if (s_Stats->m_CoverBlown > 0) {
		s_Issues += " | Cover Blown";
	}

	if (s_GameData->m_nDifficulty <= 1) {
		if (s_Stats->m_BodiesFound > 0 || s_Stats->m_UnconsciousBodiesFound > 0) {
			s_Issues += " | Bodies Found";
		}
	}
	else {
		if (s_Stats->m_BodiesFound > 0 || s_Stats->m_TargetBodiesFound > 0 || s_Stats->m_UnconsciousBodiesFound > 0) {
			s_Issues += " | Bodies Found";
		}
	}

	if (s_Stats->m_Witnesses > 0) {
		s_Issues += " | Witnesses";
	}

	if (s_GameData->m_nDifficulty == 3) {
		if (s_Stats->m_CustomWeaponsLeftOnLevel > 0 || s_Stats->m_SuitLeftOnLevel > 0) {
			s_Issues += " | Items Left";
		}
	}

	if (s_Issues.size() <= 3) {
		return s_Issues;
	}

	return s_Issues.substr(3);
}

struct SVector2 {
	float x;
	float y;
};

struct SCameraData {
	float FovDegrees;
	float Near;
	float Far;
	float ViewWidth;
	float ViewHeight;
	SMatrix33 RotationMatrix;
	SVector3 Position;
};

std::optional<SVector2> WorldToScreen(const SCameraData& p_Cam, const SVector3& p_Pos) {
	const auto s_Forward = XMVectorSet(p_Cam.RotationMatrix.xAxis.x, p_Cam.RotationMatrix.xAxis.y, p_Cam.RotationMatrix.xAxis.z, 0.0f);
	const auto s_Up = XMVectorSet(p_Cam.RotationMatrix.yAxis.x, p_Cam.RotationMatrix.yAxis.y, p_Cam.RotationMatrix.yAxis.z, 0.0f);
	const auto s_CameraPos = XMVectorSet(p_Cam.Position.x, p_Cam.Position.y, p_Cam.Position.z, 1.0f);

	// Compute the "look at" position by adding the forward direction to the camera position
	const auto s_Target = XMVectorAdd(s_CameraPos, s_Forward);

	// Compute view and projection matrices
	const auto s_View = XMMatrixLookAtLH(s_CameraPos, s_Target, s_Up);

	const float s_AspectRatio = p_Cam.ViewHeight > 0.0f
		? p_Cam.ViewWidth / p_Cam.ViewHeight
		: 1.0f;

	const auto s_Projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(p_Cam.FovDegrees),
		s_AspectRatio,
		p_Cam.Near,
		p_Cam.Far
	);

	// Transform the world position to view space
	const auto s_WorldPos = XMVectorSet(p_Pos.x, p_Pos.y, p_Pos.z, 1.0f);
	const auto s_ViewPos = XMVector3Transform(s_WorldPos, s_View);

	// Transform the coordinates from view space to clip space
	const auto s_ClipPos = XMVector3Transform(s_ViewPos, s_Projection);

	// Perform perspective division to get NDC
	XMFLOAT4 s_Ndc;
	XMStoreFloat4(&s_Ndc, s_ClipPos);
	if (s_Ndc.w != 0.0f) {
		s_Ndc.x /= s_Ndc.w;
		s_Ndc.y /= s_Ndc.w;
		s_Ndc.z /= s_Ndc.w;
	}

	// Check if the point is within the clip space volume
	if (s_Ndc.x < -1.0f || s_Ndc.x > 1.0f || s_Ndc.y < -1.0f || s_Ndc.y > 1.0f || s_Ndc.z < 0.0f || s_Ndc.z > 1.0f) {
		return std::nullopt;
	}

	// Map NDC to screen space
	SVector2 s_ScreenPos;
	s_ScreenPos.x = ((s_Ndc.x + 1.0f) / 2.0f) * p_Cam.ViewWidth;
	s_ScreenPos.y = ((1.0f - s_Ndc.y) / 2.0f) * p_Cam.ViewHeight;

	return s_ScreenPos;
}

namespace
{
	const ImVec4 c_Green{ 0.30f, 1.00f, 0.35f, 1.00f };
	const ImVec4 c_Red{ 1.00f, 0.35f, 0.35f, 1.00f };
	const ImVec4 c_Orange{ 1.00f, 0.65f, 0.15f, 1.00f };
	const ImVec4 c_RouletteRed{ 1.00f, 0.30f, 0.30f, 1.00f };
	const ImVec4 c_RouletteBlue{ 0.30f, 0.80f, 1.00f, 1.00f };
	const ImVec4 c_RouletteGreen{ 0.35f, 1.00f, 0.45f, 1.00f };
	const ImVec4 c_ChromeBg{ 0.05f, 0.06f, 0.08f, 0.78f };
	const ImVec4 c_ChromeBorder{ 0.30f, 0.50f, 0.90f, 0.85f };

	constexpr int c_StyleTargetNone = -100;
	constexpr int c_StyleTargetRating = -1;
	constexpr int c_StyleTargetTimer = -2;
	constexpr int c_StyleTargetLevel = -3;

	bool s_Initialized = false;
	bool s_InitFailed = false;
	bool s_CaptureInput = false;
	bool s_PrevToggleKeyDown = false;

	void* s_ZWINDOWS = nullptr;
	uint8_t s_SavedMouseVisible = 1;
	bool s_HasSavedMouseVisible = false;

	HWND s_Window = nullptr;
	WNDPROC s_OriginalWndProc = nullptr;
	DWORD s_RenderThreadId = 0;

	struct SceneInfo
	{
		const char* Name;
		const char* State;
		bool InGame;
		bool MainMenu = false;
	};

	const std::unordered_map<std::string, SceneInfo>& GetSceneNames()
	{
		static const std::unordered_map<std::string, SceneInfo> s_Names = {
			{ "hitmanbloodmoney", { "Main Menu", "Menu", false, true } },
			{ "scenes\\hitmanbloodmoney.gms", { "Main Menu", "Menu", false, true } },

			{ "saveandcontinue", { "Save & Continue", "Menu", false, true } },

			{ "hideout\\hideout_main", { "Hideout", "In-Game", true } },
			{ "scenes\\hideout\\hideout_main.gms", { "Hideout", "In-Game", true } },

			{ "m00\\m00_intro", { "Death of a Showman", "Intro", false } },
			{ "m00\\m00_main", { "Death of a Showman", "In-Game", true } },
			{ "scenes\\m00\\m00_main.gms", { "Death of a Showman", "In-Game", true } },
			{ "m00\\m00_news", { "Death of a Showman", "News", false } },
			{ "m00\\m00_albino", { "Death of a Showman", "Story Intro", false } },

			{ "m01\\m01_premission", { "A Vintage Year", "Pre-Mission", false } },
			{ "m01\\m01_main", { "A Vintage Year", "In-Game", true } },
			{ "scenes\\m01\\m01_main.gms", { "A Vintage Year", "In-Game", true } },
			{ "m01\\m01_postmission", { "A Vintage Year", "Post-Mission", false } },
			{ "m01\\m01_news", { "A Vintage Year", "News", false } },

			{ "m03\\m03_premission", { "Curtains Down", "Pre-Mission", false } },
			{ "m03\\m03_main", { "Curtains Down", "In-Game", true } },
			{ "scenes\\m03\\m03_main.gms", { "Curtains Down", "In-Game", true } },
			{ "m03\\m03_postmission", { "Curtains Down", "Post-Mission", false } },
			{ "m03\\m03_news", { "Curtains Down", "News", false } },

			{ "m04\\m04_premission", { "Flatline", "Pre-Mission", false } },
			{ "m04\\m04_main", { "Flatline", "In-Game", true } },
			{ "scenes\\m04\\m04_main.gms", { "Flatline", "In-Game", true } },
			{ "m04\\m04_postmission", { "Flatline", "Post-Mission", false } },
			{ "m04\\m04_news", { "Flatline", "News", false } },

			{ "m05\\m05_premission", { "A New Life", "Pre-Mission", false } },
			{ "m05\\m05_main", { "A New Life", "In-Game", true } },
			{ "scenes\\m05\\m05_main.gms", { "A New Life", "In-Game", true } },
			{ "m05\\m05_postmission", { "A New Life", "Post-Mission", false } },
			{ "m05\\m05_news", { "A New Life", "News", false } },

			{ "m06\\m06_premission", { "Murder of the Crows", "Pre-Mission", false } },
			{ "m06\\m06_main", { "Murder of the Crows", "In-Game", true } },
			{ "scenes\\m06\\m06_main.gms", { "Murder of the Crows", "In-Game", true } },
			{ "m06\\m06_postmission", { "Murder of the Crows", "Post-Mission", false } },
			{ "m06\\m06_news", { "Murder of the Crows", "News", false } },

			{ "m02\\m02_premission", { "You Better Watch Out", "Pre-Mission", false } },
			{ "m02\\m02_main", { "You Better Watch Out", "In-Game", true } },
			{ "scenes\\m02\\m02_main.gms", { "You Better Watch Out", "In-Game", true } },
			{ "m02\\m02_postmission", { "You Better Watch Out", "Post-Mission", false } },
			{ "m02\\m02_news", { "You Better Watch Out", "News", false } },

			{ "m08\\m08_premission", { "Death on the Mississippi", "Pre-Mission", false } },
			{ "m08\\m08_main", { "Death on the Mississippi", "In-Game", true } },
			{ "scenes\\m08\\m08_main.gms", { "Death on the Mississippi", "In-Game", true } },
			{ "m08\\m08_postmission", { "Death on the Mississippi", "Post-Mission", false } },
			{ "m08\\m08_news", { "Death on the Mississippi", "News", false } },

			{ "m09\\m09_premission", { "Till Death Do Us Part", "Pre-Mission", false } },
			{ "m09\\m09_main", { "Till Death Do Us Part", "In-Game", true } },
			{ "scenes\\m09\\m09_main.gms", { "Till Death Do Us Part", "In-Game", true } },
			{ "m09\\m09_postmission", { "Till Death Do Us Part", "Post-Mission", false } },
			{ "m09\\m09_news", { "Till Death Do Us Part", "News", false } },

			{ "m10\\m10_premission", { "A House of Cards", "Pre-Mission", false } },
			{ "m10\\m10_main", { "A House of Cards", "In-Game", true } },
			{ "scenes\\m10\\m10_main.gms", { "A House of Cards", "In-Game", true } },
			{ "m10\\m10_postmission", { "A House of Cards", "Post-Mission", false } },
			{ "m10\\m10_news", { "A House of Cards", "News", false } },

			{ "m11\\m11_premission", { "A Dance with The Devil", "Pre-Mission", false } },
			{ "m11\\m11_main", { "A Dance with The Devil", "In-Game", true } },
			{ "scenes\\m11\\m11_main.gms", { "A Dance with The Devil", "In-Game", true } },
			{ "m11\\m11_postmission", { "A Dance with The Devil", "Post-Mission", false } },
			{ "m11\\m11_news", { "A Dance with The Devil", "News", false } },

			{ "m12\\m12_premission", { "Amendment XXV", "Pre-Mission", false } },
			{ "m12\\m12_main", { "Amendment XXV", "In-Game", true } },
			{ "scenes\\m12\\m12_main.gms", { "Amendment XXV", "In-Game", true } },
			{ "m12\\m12_postmission", { "Amendment XXV", "Post-Mission", false } },
			{ "m12\\m12_news", { "Amendment XXV", "News", false } },

			{ "m13\\m13_intro", { "Requiem", "Intro", false } },
			{ "m13\\m13_main", { "Requiem", "In-Game", true } },
			{ "scenes\\m13\\m13_main.gms", { "Requiem", "In-Game", true } },
		};

		return s_Names;
	}

	std::string NormalizeSceneName(const std::string& p_Scene)
	{
		std::string s_Scene = p_Scene;
		std::transform(s_Scene.begin(), s_Scene.end(), s_Scene.begin(), [](unsigned char p_Char)
		{
			return static_cast<char>(std::tolower(p_Char));
		});
		std::replace(s_Scene.begin(), s_Scene.end(), '/', '\\');
		return s_Scene;
	}

	const SceneInfo* FindSceneInfo(const std::string& p_Scene)
	{
		const auto& s_Names = GetSceneNames();
		const auto s_It = s_Names.find(NormalizeSceneName(p_Scene));
		return s_It != s_Names.end() ? &s_It->second : nullptr;
	}

	std::string GetLevelDisplayName(const std::string& p_Level)
	{
		std::string s_Level = p_Level;
		std::transform(s_Level.begin(), s_Level.end(), s_Level.begin(), [](unsigned char p_Char)
		{
			return static_cast<char>(std::tolower(p_Char));
		});

		for (const auto& [s_Scene, s_Info] : GetSceneNames())
		{
			if (!s_Info.InGame)
				continue;

			if (s_Scene.find(s_Level) != std::string::npos)
				return s_Info.Name;
		}

		return p_Level;
	}

	struct MouseState
	{
		float X = 0.0f;
		float Y = 0.0f;
		bool Left = false;
		bool Right = false;
		bool Middle = false;
	};

	struct ZMouseWintel
	{
		uint8_t m_Padding[0x290];
		int32_t m_X;
		int32_t m_Y;
		int32_t m_Wheel;
		uint8_t m_LeftButton;
		uint8_t m_RightButton;
		uint8_t m_MiddleButton;
	};

	struct ZSysInputWintel
	{
		void* m_VTable;
		int32_t m_Unknown4;
		int32_t m_Unknown8;
		int32_t m_UnknownC;
		void* m_Devices[32];
		int32_t m_AttachedDevicesCount;
	};

	static_assert(offsetof(ZMouseWintel, m_X) == 0x290, "Unexpected ZMouseWintel layout.");
	static_assert(offsetof(ZMouseWintel, m_LeftButton) == 0x29C, "Unexpected ZMouseWintel layout.");
	static_assert(offsetof(ZSysInputWintel, m_Devices) == 0x10, "Unexpected ZSysInputWintel layout.");
	static_assert(offsetof(ZSysInputWintel, m_AttachedDevicesCount) == 0x90, "Unexpected ZSysInputWintel layout.");

	uintptr_t GetImageBase()
	{
		return reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
	}

	template <typename T>
	bool ReadGameMemory(uintptr_t p_Address, T& p_Out)
	{
		SIZE_T s_Read = 0;
		return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(p_Address), &p_Out, sizeof(T), &s_Read)
			&& s_Read == sizeof(T);
	}

	int GetTotalLevelNpcs()
	{
		const uintptr_t s_GameBase = GetImageBase();

		if (!s_GameBase)
			return 0;

		// The game's special-rating code reads the level object from [0x0081F83C] + 0xA48,
		// then the total NPC count from +0x1B8. Make the address image-base relative.
		const uintptr_t s_GlobalAddress = s_GameBase + (0x0081F83C - 0x00400000);

		uintptr_t s_Game = 0;
		if (!ReadGameMemory(s_GlobalAddress, s_Game) || !s_Game)
			return 0;

		uintptr_t s_LevelControl = 0;
		if (!ReadGameMemory(s_Game + 0xA48, s_LevelControl) || !s_LevelControl)
			return 0;

		int s_TotalNpcs = 0;
		if (!ReadGameMemory(s_LevelControl + 0x1B8, s_TotalNpcs))
			return 0;

		if (s_TotalNpcs > 0)
			return s_TotalNpcs;

		// Fallback: count dressable NPC actors if the level object did not expose a count.
		auto* s_GameData = (g_Module->Pointers()->g_pGameData && *g_Module->Pointers()->g_pGameData)
			? *g_Module->Pointers()->g_pGameData
			: nullptr;

		if (s_GameData)
		{
			for (uint32_t i = 0; i < s_GameData->m_nActorCount && i < 512; ++i)
			{
				auto* s_Actor = s_GameData->m_aActors[i];

				if (!s_Actor || !s_Actor->GetHitmanAs())
					continue;

				if (g_Module->Functions()->ZGEOM_RefToPtr(s_Actor->GetHitmanAs()))
					++s_TotalNpcs;
			}
		}

		return s_TotalNpcs;
	}

	void UpdateGameCursor(bool p_Freeze)
	{
		if (!s_ZWINDOWS)
			return;

		const auto s_Address = reinterpret_cast<uintptr_t>(s_ZWINDOWS) + 0x1DE;

		if (p_Freeze)
		{
			uint8_t s_Visible = 0;
			if (!ReadGameMemory(s_Address, s_Visible))
				return;

			if (!s_HasSavedMouseVisible)
			{
				s_SavedMouseVisible = s_Visible;
				s_HasSavedMouseVisible = true;
			}

			if (s_Visible != 0)
			{
				const uint8_t s_Hidden = 0;
				WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(s_Address), &s_Hidden, sizeof(s_Hidden), nullptr);
			}
		}
		else if (s_HasSavedMouseVisible)
		{
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(s_Address), &s_SavedMouseVisible, sizeof(s_SavedMouseVisible), nullptr);
			s_HasSavedMouseVisible = false;
		}
	}

	void SetZWINDOWSInstance(void* p_Instance)
	{
		if (p_Instance == s_ZWINDOWS)
			return;

		s_ZWINDOWS = p_Instance;
		s_HasSavedMouseVisible = false;
		UpdateGameCursor(s_CaptureInput);
	}

	bool IsInGameImage(uintptr_t p_Address)
	{
		const auto s_Base = GetImageBase();

		if (!s_Base)
			return false;

		const auto* s_DosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(s_Base);
		const auto* s_NtHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(s_Base + s_DosHeader->e_lfanew);

		return p_Address >= s_Base && p_Address < s_Base + s_NtHeaders->OptionalHeader.SizeOfImage;
	}

	void UpdateActionManager(bool p_Capture)
	{
		static void* s_Manager = nullptr;
		static uint8_t s_SavedEnabled = 1;
		static bool s_HasSaved = false;

		if (!g_Module || !g_Module->Pointers() || !g_Module->Pointers()->g_pActionManager)
			return;

		void* s_NewManager = nullptr;
		if (!ReadGameMemory(reinterpret_cast<uintptr_t>(g_Module->Pointers()->g_pActionManager), s_NewManager))
			return;

		if (!s_NewManager)
		{
			s_Manager = nullptr;
			s_HasSaved = false;
			return;
		}

		if (s_NewManager != s_Manager)
		{
			s_Manager = s_NewManager;
			s_HasSaved = false;
		}

		void** s_VTable = nullptr;
		if (!ReadGameMemory(reinterpret_cast<uintptr_t>(s_Manager), s_VTable) ||
			!s_VTable || !IsInGameImage(reinterpret_cast<uintptr_t>(s_VTable)))
			return;

		const auto s_EnabledAddress = reinterpret_cast<uintptr_t>(s_Manager) + 0x4D;

		if (p_Capture)
		{
			uint8_t s_Enabled = 0;
			if (!ReadGameMemory(s_EnabledAddress, s_Enabled))
				return;

			if (!s_HasSaved)
			{
				s_SavedEnabled = s_Enabled;
				s_HasSaved = true;
			}

			if (s_Enabled != 0)
			{
				const uint8_t s_Disabled = 0;
				WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(s_EnabledAddress), &s_Disabled, sizeof(s_Disabled), nullptr);
			}
		}
		else if (s_HasSaved)
		{
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(s_EnabledAddress), &s_SavedEnabled, sizeof(s_SavedEnabled), nullptr);
			s_HasSaved = false;
		}
	}

	MouseState GetMouseState()
	{
		MouseState s_State;

		POINT s_Point{};
		if (s_Window && GetCursorPos(&s_Point) && ScreenToClient(s_Window, &s_Point))
		{
			s_State.X = static_cast<float>(s_Point.x);
			s_State.Y = static_cast<float>(s_Point.y);
		}

		bool s_HasEngineButtons = false;

		ZSysInputWintel* s_SysInput = nullptr;
		if (g_Module && g_Module->Pointers() && g_Module->Pointers()->g_pSysInput &&
			ReadGameMemory(reinterpret_cast<uintptr_t>(g_Module->Pointers()->g_pSysInput), s_SysInput) && s_SysInput)
		{
			ZSysInputWintel s_Input;
			if (ReadGameMemory(reinterpret_cast<uintptr_t>(s_SysInput), s_Input) &&
				IsInGameImage(reinterpret_cast<uintptr_t>(s_Input.m_VTable)))
			{
				int s_Count = s_Input.m_AttachedDevicesCount;
				s_Count = std::clamp(s_Count, 0, 32);

				for (int i = 0; i < s_Count; ++i)
				{
					void* s_Device = s_Input.m_Devices[i];
					if (!s_Device)
						continue;

					void* s_DeviceVTable = nullptr;
					if (!ReadGameMemory(reinterpret_cast<uintptr_t>(s_Device), s_DeviceVTable) ||
						!s_DeviceVTable || !IsInGameImage(reinterpret_cast<uintptr_t>(s_DeviceVTable)))
						continue;

					int32_t s_Type = -1;
					if (!ReadGameMemory(reinterpret_cast<uintptr_t>(s_Device) + 4, s_Type) || s_Type != 0)
						continue;

					ZMouseWintel s_Mouse;
					if (!ReadGameMemory(reinterpret_cast<uintptr_t>(s_Device), s_Mouse))
						break;

					if (s_Mouse.m_LeftButton > 1 || s_Mouse.m_RightButton > 1 || s_Mouse.m_MiddleButton > 1)
						break;

					s_State.Left = s_Mouse.m_LeftButton != 0;
					s_State.Right = s_Mouse.m_RightButton != 0;
					s_State.Middle = s_Mouse.m_MiddleButton != 0;
					s_HasEngineButtons = true;
					break;
				}
			}
		}

		if (!s_HasEngineButtons)
		{
			s_State.Left = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			s_State.Right = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
			s_State.Middle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
		}

		return s_State;
	}

	struct WindowSearchResult
	{
		HWND Window = nullptr;
		LONG Area = 0;
	};

	BOOL CALLBACK FindGameWindowProc(HWND p_Window, LPARAM p_Param)
	{
		DWORD s_ProcessId = 0;
		GetWindowThreadProcessId(p_Window, &s_ProcessId);

		if (s_ProcessId != GetCurrentProcessId() || !IsWindowVisible(p_Window))
			return TRUE;

		RECT s_Client{};
		if (!GetClientRect(p_Window, &s_Client))
			return TRUE;

		const LONG s_Area = (s_Client.right - s_Client.left) * (s_Client.bottom - s_Client.top);
		auto* s_Result = reinterpret_cast<WindowSearchResult*>(p_Param);

		if (s_Area > s_Result->Area)
		{
			s_Result->Area = s_Area;
			s_Result->Window = p_Window;
		}

		return TRUE;
	}

	HWND FindGameWindow(HWND p_Preferred)
	{
		if (p_Preferred && IsWindow(p_Preferred) && IsWindowVisible(p_Preferred))
		{
			RECT s_Client{};
			if (GetClientRect(p_Preferred, &s_Client) && s_Client.right > 0 && s_Client.bottom > 0)
				return p_Preferred;
		}

		WindowSearchResult s_Result;
		EnumWindows(&FindGameWindowProc, reinterpret_cast<LPARAM>(&s_Result));

		return s_Result.Window ? s_Result.Window : p_Preferred;
	}

	enum class StatGroup
	{
		Detailed,
		Minimal
	};

	struct StatPanelDefinition
	{
		const char* Key;
		const char* Label;
		const char* WindowId;
		StatGroup Group;
		bool DefaultVisible;
		bool InMinimalMode;
		float DetailedX;
		float DetailedY;
		float MinimalX;
		float MinimalY;
	};

	constexpr StatPanelDefinition c_StatPanels[] = {
		{ "witnesses", "Witnesses", "##hud_stat_witnesses", StatGroup::Detailed, true, true, 39.0f, 299.0f, 39.0f, 299.0f },
		{ "bodiesFound", "Bodies Found", "##hud_stat_bodies_found", StatGroup::Detailed, true, true, 30.0f, 337.0f, 30.0f, 331.0f },
		{ "coversBlown", "Covers Blown", "##hud_stat_covers_blown", StatGroup::Detailed, true, true, 30.0f, 375.0f, 30.0f, 363.0f },
		{ "caughtOnCamera", "Caught on Camera", "##hud_stat_caught_on_camera", StatGroup::Detailed, true, true, 17.0f, 413.0f, 117.0f, 363.0f },
		{ "friskFailed", "Frisk Failed", "##hud_stat_frisk_failed", StatGroup::Detailed, true, false, 36.0f, 451.0f, 36.0f, 451.0f },
		{ "itemsLeft", "Items Left on Level", "##hud_stat_items_left", StatGroup::Detailed, true, true, 16.0f, 489.0f, 231.0f, 363.0f },
		{ "accidents", "Accidents", "##hud_stat_accidents", StatGroup::Detailed, true, true, 40.0f, 527.0f, 255.0f, 299.0f },
		{ "enemiesKilled", "Enemies Killed", "##hud_stat_enemies_killed", StatGroup::Detailed, true, false, 1142.0f, 299.0f, 1142.0f, 299.0f },
		{ "enemiesWounded", "Enemies Wounded", "##hud_stat_enemies_wounded", StatGroup::Detailed, true, false, 1131.0f, 337.0f, 1131.0f, 337.0f },
		{ "policemenKilled", "Policemen Killed", "##hud_stat_policemen_killed", StatGroup::Detailed, true, false, 1137.0f, 375.0f, 1137.0f, 375.0f },
		{ "policemenWounded", "Policemen Wounded", "##hud_stat_policemen_wounded", StatGroup::Detailed, true, false, 1126.0f, 413.0f, 1126.0f, 413.0f },
		{ "innocentsKilled", "Innocents Killed", "##hud_stat_innocents_killed", StatGroup::Detailed, true, false, 1138.0f, 451.0f, 1138.0f, 451.0f },
		{ "innocentsWounded", "Innocents Wounded", "##hud_stat_innocents_wounded", StatGroup::Detailed, true, false, 1127.0f, 489.0f, 1127.0f, 489.0f },
		{ "shotsHit", "Shots Hit", "##hud_stat_shots_hit", StatGroup::Detailed, true, true, 1154.0f, 527.0f, 257.0f, 331.0f },
		{ "nonTargetsKilled", "Non-Targets Killed", "##hud_stat_non_targets_killed", StatGroup::Minimal, false, true, 117.0f, 299.0f, 117.0f, 299.0f },
		{ "nonTargetsHarmed", "Non-Targets Harmed", "##hud_stat_non_targets_harmed", StatGroup::Minimal, false, true, 110.0f, 331.0f, 110.0f, 331.0f }
	};

	constexpr int c_StatPanelCount = static_cast<int>(sizeof(c_StatPanels) / sizeof(c_StatPanels[0]));

	struct OverlayPositions
	{
		ImVec2 Stat[c_StatPanelCount];
		ImVec2 Rating;
		ImVec2 Timer;
		ImVec2 Level;
		ImVec2 Roulette;

		bool StatValid[c_StatPanelCount] = {};
		bool RatingValid = false;
		bool TimerValid = false;
		bool LevelValid = false;
		bool RouletteValid = false;
	};

	struct OverlaySettings
	{
		bool ShowStat[c_StatPanelCount];
		float StatLabelSize[c_StatPanelCount] = {};
		float StatValueSize[c_StatPanelCount] = {};
		int StatFontStyle[c_StatPanelCount];
		bool ShowRating = true;
		bool ShowTimer = true;
		bool ShowLevelInfo = true;
		bool StatsOnlyInGame = false;
		bool PredictSpecialRatings = false;
		bool Debug2016DressableOnly = true;
		float Debug2016HeadHeight = 180.0f;
		float Debug2016DistanceScale = 1.0f;
		float Debug2016MaxDistance = 2500.0f;
		bool SnapToGrid = false;
		bool SnapToPanels = true;
		bool DragStatsAsGroup = false;
		float GridSize = 10.0f;
		float SnapDistance = 6.0f;

		OverlaySettings()
		{
			for (int i = 0; i < c_StatPanelCount; ++i)
			{
				// First-run defaults mirror the "Arrange detailed" preset: every
				// detailed panel is visible and the minimal-only ones are hidden.
				// Panel positions fall back to DetailedX/DetailedY because
				// s_ArrangeMode starts as None, i.e. not Minimal.
				ShowStat[i] = c_StatPanels[i].DefaultVisible;
				StatFontStyle[i] = -1;
			}
		}

		bool ShowRouletteHud = true;
		bool RouletteOnlyInGame = false;
		int RouletteMission = 0;
		int RouletteExtras = 0;

		std::string RegularFont = "segoeui.ttf";
		std::string BoldFont = "segoeuib.ttf";
		std::string TimerFont = "segoeuib.ttf";
		std::string Debug2016Font = "segoeuib.ttf";

		float LabelSize = 15.0f;
		float ValueSize = 17.0f;
		float RatingSize = 22.0f;
		float DifficultySize = 14.0f;
		float TimerSize = 28.0f;
		float UISize = 16.0f;
		float RowSpacing = 7.0f;
		float StatPanelPadding = 8.0f;
		std::string TimerFormat = "MM:SS.00";
		float Debug2016FontSize = 20.0f;
		int Debug2016FontStyle = -1;
		float RatingLabelSizeOverride = 0.0f;
		float RatingValueSizeOverride = 0.0f;
		int RatingFontStyle = -1;
		float TimerSizeOverride = 0.0f;
		int TimerFontStyle = -1;
		float LevelSizeOverride = 0.0f;
		int LevelFontStyle = -1;

		bool BoldLabels = false;
		bool BoldValues = true;
		bool BoldRating = true;

		bool HighlightBadStats = true;

		ImVec4 LabelColor = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
		ImVec4 ValueColor = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
		ImVec4 HighlightColor = ImVec4(1.00f, 0.24313726f, 0.24313726f, 1.0f);
		ImVec4 TimerColor = ImVec4(0.30f, 1.00f, 0.35f, 1.0f);
		ImVec4 RatingColor = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
		ImVec4 RatingBadColor = ImVec4(1.00f, 0.35f, 0.35f, 1.0f);
		ImVec4 SpecialRatingColor = ImVec4(1.00f, 0.6549f, 0.1490f, 1.0f);
		ImVec4 DifficultyColor = ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
		ImVec4 HudBackground = ImVec4(0.39215687f, 0.39215687f, 0.39215687f, 0.44705882f);
	};

	struct OverlayFonts
	{
		ImFont* Regular = nullptr;
		ImFont* Bold = nullptr;
		ImFont* Timer = nullptr;
		ImFont* Debug = nullptr;
	};

	static OverlaySettings s_Settings;
	static OverlayPositions s_Positions;
	static OverlayFonts s_Fonts;
	static std::vector<std::string> s_FontFiles;

	static bool s_FontsDirty = true;
	static bool s_ResetPositions = false;
	static bool s_ReloadPositions = false;
	static bool s_ForceStatPositions = false;

	enum class StatArrangeMode
	{
		None,
		Minimal,
		Detailed
	};

	static StatArrangeMode s_ArrangeMode = StatArrangeMode::None;
	static bool s_GroupDragActive = false;
	static ImVec2 s_GroupDragStartWindow;
	static ImVec2 s_GroupDragStartPositions[c_StatPanelCount];
	static bool s_ResetRoulettePosition = false;
	static bool s_ShowEditor = false;
	static bool s_ShowSettings = false;
	static bool s_ShowGameOptions = false;
	static bool s_ShowLocations = false;
	static bool s_ShowRoulette = false;
	static bool s_HasRouletteResult = false;
	static Roulette::Result s_RouletteResult;
	static int s_StyleTarget = c_StyleTargetNone;
	static bool s_ShowStyleWindow = false;
	static ImGuiID s_DragWindow = 0;
	static ImGuiID s_SelectedWindow = 0;
	static ImVec2 s_DragOffset;

	// Folder that holds user configuration next to Statman.exe. Keeping it in a
	// subfolder leaves the application folder limited to executables and DLLs.
	std::filesystem::path GetConfigDirectory()
	{
		wchar_t s_ModulePath[MAX_PATH]{};
		const DWORD s_Length = GetModuleFileNameW(g_ModuleHandle, s_ModulePath, MAX_PATH);

		if (s_Length > 0 && s_Length < MAX_PATH)
			return std::filesystem::path(s_ModulePath).parent_path() / L"config";

		return std::filesystem::path(L"config");
	}

	std::filesystem::path GetSettingsPath()
	{
		return GetConfigDirectory() / L"HM3Overlay.json";
	}

	// Only builds that stored the overlay settings next to Statman.exe are
	// migrated. The pre-1.5 %APPDATA%\Statman location is deliberately not
	// imported: doing so made fresh installs inherit stale layouts instead of
	// starting from the built-in defaults.
	std::filesystem::path GetLegacySettingsPath()
	{
		wchar_t s_ModulePath[MAX_PATH]{};
		const DWORD s_Length = GetModuleFileNameW(g_ModuleHandle, s_ModulePath, MAX_PATH);

		if (s_Length > 0 && s_Length < MAX_PATH)
			return std::filesystem::path(s_ModulePath).parent_path() / L"HM3Overlay.json";

		return {};
	}

	nlohmann::json ColorToJson(const ImVec4& p_Color)
	{
		return nlohmann::json::array({ p_Color.x, p_Color.y, p_Color.z, p_Color.w });
	}

	bool JsonHas(const nlohmann::json& p_Json, const char* p_Key)
	{
		return p_Json.find(p_Key) != p_Json.end();
	}

	bool ColorFromJson(const nlohmann::json& p_Json, const char* p_Key, ImVec4& p_Color)
	{
		if (!JsonHas(p_Json, p_Key))
			return false;

		const auto& s_Value = p_Json[p_Key];

		if (!s_Value.is_array() || s_Value.size() != 4)
			return false;

		p_Color = ImVec4(
			s_Value[0].get<float>(),
			s_Value[1].get<float>(),
			s_Value[2].get<float>(),
			s_Value[3].get<float>());

		return true;
	}

	bool ColorNear(const ImVec4& p_A, const ImVec4& p_B, float p_Tolerance)
	{
		return std::fabs(p_A.x - p_B.x) <= p_Tolerance
			&& std::fabs(p_A.y - p_B.y) <= p_Tolerance
			&& std::fabs(p_A.z - p_B.z) <= p_Tolerance
			&& std::fabs(p_A.w - p_B.w) <= p_Tolerance;
	}

	nlohmann::json Vec2ToJson(const ImVec2& p_Position)
	{
		return nlohmann::json::array({ p_Position.x, p_Position.y });
	}

	bool Vec2FromJson(const nlohmann::json& p_Json, const char* p_Key, ImVec2& p_Position)
	{
		if (!JsonHas(p_Json, p_Key))
			return false;

		const auto& s_Value = p_Json[p_Key];

		if (!s_Value.is_array() || s_Value.size() != 2)
			return false;

		p_Position = ImVec2(s_Value[0].get<float>(), s_Value[1].get<float>());
		return true;
	}

	// Moves HM3Overlay.json out of the application folder the first time this
	// build runs so an upgrade keeps its layout.
	void MigrateLegacySettings()
	{
		std::error_code s_Error;
		const auto s_Target = GetSettingsPath();

		if (std::filesystem::exists(s_Target, s_Error))
			return;

		const auto s_LegacyPath = GetLegacySettingsPath();

		if (s_LegacyPath.empty() || !std::filesystem::exists(s_LegacyPath, s_Error))
			return;

		std::filesystem::create_directories(s_Target.parent_path(), s_Error);
		std::filesystem::rename(s_LegacyPath, s_Target, s_Error);

		if (s_Error)
			std::filesystem::copy_file(s_LegacyPath, s_Target, std::filesystem::copy_options::overwrite_existing, s_Error);
	}

	void LoadSettings()
	{
		try
		{
			MigrateLegacySettings();

			std::filesystem::path s_SettingsPath = GetSettingsPath();
			std::error_code s_ExistsError;

			if (!std::filesystem::exists(s_SettingsPath, s_ExistsError))
			{
				const auto s_LegacyPath = GetLegacySettingsPath();
				std::error_code s_LegacyError;

				if (!s_LegacyPath.empty() && std::filesystem::exists(s_LegacyPath, s_LegacyError))
				{
					s_SettingsPath = s_LegacyPath;
				}
			}

			std::ifstream s_File(s_SettingsPath);

			if (!s_File)
				return;

			nlohmann::json s_Json;
			s_File >> s_Json;

			if (!s_Json.is_object())
				return;

			const int s_SettingsVersion = s_Json.value("version", 1);

			if (JsonHas(s_Json, "show") && s_Json["show"].is_object())
			{
				const auto& s_Show = s_Json["show"];
				s_Settings.ShowRating = s_Show.value("rating", s_Settings.ShowRating);
				s_Settings.ShowTimer = s_Show.value("timer", s_Settings.ShowTimer);
				s_Settings.ShowLevelInfo = s_Show.value("levelInfo", s_Settings.ShowLevelInfo);
				s_Settings.StatsOnlyInGame = s_Show.value("statsOnlyInGame", s_Settings.StatsOnlyInGame);
				s_Settings.HighlightBadStats = s_Show.value("highlightBadStats", s_Settings.HighlightBadStats);
			}

			if (JsonHas(s_Json, "stats") && s_Json["stats"].is_object())
			{
				const auto& s_StatsJson = s_Json["stats"];

				for (int i = 0; i < c_StatPanelCount; ++i)
					s_Settings.ShowStat[i] = s_StatsJson.value(c_StatPanels[i].Key, s_Settings.ShowStat[i]);
			}

			if (JsonHas(s_Json, "statStyles") && s_Json["statStyles"].is_object())
			{
				const auto& s_StylesJson = s_Json["statStyles"];

				for (int i = 0; i < c_StatPanelCount; ++i)
				{
					if (!JsonHas(s_StylesJson, c_StatPanels[i].Key) || !s_StylesJson[c_StatPanels[i].Key].is_object())
						continue;

					const auto& s_StyleJson = s_StylesJson[c_StatPanels[i].Key];
					s_Settings.StatLabelSize[i] = s_StyleJson.value("labelSize", s_Settings.StatLabelSize[i]);
					s_Settings.StatValueSize[i] = s_StyleJson.value("valueSize", s_Settings.StatValueSize[i]);
					s_Settings.StatFontStyle[i] = (std::clamp)(s_StyleJson.value("font", s_Settings.StatFontStyle[i]), -1, 1);
				}
			}

			if (JsonHas(s_Json, "hudStyles") && s_Json["hudStyles"].is_object())
			{
				const auto& s_HudStylesJson = s_Json["hudStyles"];

				if (JsonHas(s_HudStylesJson, "rating") && s_HudStylesJson["rating"].is_object())
				{
					const auto& s_StyleJson = s_HudStylesJson["rating"];
					s_Settings.RatingLabelSizeOverride = s_StyleJson.value("labelSize", s_Settings.RatingLabelSizeOverride);
					s_Settings.RatingValueSizeOverride = s_StyleJson.value("valueSize", s_Settings.RatingValueSizeOverride);
					s_Settings.RatingFontStyle = (std::clamp)(s_StyleJson.value("font", s_Settings.RatingFontStyle), -1, 1);
				}

				if (JsonHas(s_HudStylesJson, "timer") && s_HudStylesJson["timer"].is_object())
				{
					const auto& s_StyleJson = s_HudStylesJson["timer"];
					s_Settings.TimerSizeOverride = s_StyleJson.value("valueSize", s_Settings.TimerSizeOverride);
					s_Settings.TimerFontStyle = (std::clamp)(s_StyleJson.value("font", s_Settings.TimerFontStyle), -1, 1);
				}

				if (JsonHas(s_HudStylesJson, "level") && s_HudStylesJson["level"].is_object())
				{
					const auto& s_StyleJson = s_HudStylesJson["level"];
					s_Settings.LevelSizeOverride = s_StyleJson.value("labelSize", s_Settings.LevelSizeOverride);
					s_Settings.LevelFontStyle = (std::clamp)(s_StyleJson.value("font", s_Settings.LevelFontStyle), -1, 1);
				}
			}

			if (JsonHas(s_Json, "editor") && s_Json["editor"].is_object())
			{
				const auto& s_EditorJson = s_Json["editor"];
				s_Settings.SnapToGrid = s_EditorJson.value("snapToGrid", s_Settings.SnapToGrid);
				s_Settings.SnapToPanels = s_EditorJson.value("snapToPanels", s_Settings.SnapToPanels);
				s_Settings.DragStatsAsGroup = s_EditorJson.value("dragStatsAsGroup", s_Settings.DragStatsAsGroup);
				s_Settings.GridSize = s_EditorJson.value("gridSize", s_Settings.GridSize);
				s_Settings.SnapDistance = s_EditorJson.value("snapDistance", s_Settings.SnapDistance);
			}

			if (JsonHas(s_Json, "experimental") && s_Json["experimental"].is_object())
			{
				const auto& s_ExperimentalJson = s_Json["experimental"];
				s_Settings.PredictSpecialRatings = s_ExperimentalJson.value("predictSpecialRatings", s_Settings.PredictSpecialRatings);
			}

			if (JsonHas(s_Json, "debug2016") && s_Json["debug2016"].is_object())
			{
				const auto& s_DebugJson = s_Json["debug2016"];
				s_Settings.Debug2016DressableOnly = s_DebugJson.value("dressableOnly", s_Settings.Debug2016DressableOnly);
				s_Settings.Debug2016HeadHeight = s_DebugJson.value("headHeight", s_Settings.Debug2016HeadHeight);
				s_Settings.Debug2016DistanceScale = s_DebugJson.value("distanceScale", s_Settings.Debug2016DistanceScale);
				s_Settings.Debug2016MaxDistance = s_DebugJson.value("maxDistance", s_Settings.Debug2016MaxDistance);
			}

			if (JsonHas(s_Json, "roulette") && s_Json["roulette"].is_object())
			{
				const auto& s_RouletteJson = s_Json["roulette"];
				s_Settings.ShowRouletteHud = s_RouletteJson.value("showHud", s_Settings.ShowRouletteHud);
				s_Settings.RouletteOnlyInGame = s_RouletteJson.value("onlyInGame", s_Settings.RouletteOnlyInGame);
				s_Settings.RouletteMission = s_RouletteJson.value("mission", s_Settings.RouletteMission);
				s_Settings.RouletteExtras = s_RouletteJson.value("extras", s_Settings.RouletteExtras);
			}

			if (JsonHas(s_Json, "fonts") && s_Json["fonts"].is_object())
			{
				const auto& s_FontJson = s_Json["fonts"];
				s_Settings.RegularFont = s_FontJson.value("regular", s_Settings.RegularFont);
				s_Settings.BoldFont = s_FontJson.value("bold", s_Settings.BoldFont);
				s_Settings.TimerFont = s_FontJson.value("timer", s_Settings.TimerFont);
				s_Settings.BoldLabels = s_FontJson.value("boldLabels", s_Settings.BoldLabels);
				s_Settings.BoldValues = s_FontJson.value("boldValues", s_Settings.BoldValues);
				s_Settings.BoldRating = s_FontJson.value("boldRating", s_Settings.BoldRating);
				s_Settings.Debug2016Font = s_FontJson.value("debug", s_Settings.Debug2016Font);
				s_Settings.Debug2016FontSize = s_FontJson.value("debugSize", s_Settings.Debug2016FontSize);
				s_Settings.Debug2016FontStyle = (std::clamp)(s_FontJson.value("debugStyle", s_Settings.Debug2016FontStyle), -1, 1);

				// The old default timer font was Consolas. The overlay now defaults to the bold font
				// so the timer keeps the same style whether or not the old Bold timer option was used.
				if (s_Settings.TimerFont == "consola.ttf")
					s_Settings.TimerFont = s_Settings.BoldFont;
			}

			if (JsonHas(s_Json, "sizes") && s_Json["sizes"].is_object())
			{
				const auto& s_SizeJson = s_Json["sizes"];
				s_Settings.LabelSize = s_SizeJson.value("label", s_Settings.LabelSize);
				s_Settings.ValueSize = s_SizeJson.value("value", s_Settings.ValueSize);
				s_Settings.RatingSize = s_SizeJson.value("rating", s_Settings.RatingSize);
				s_Settings.DifficultySize = s_SizeJson.value("difficulty", s_Settings.DifficultySize);
				s_Settings.TimerSize = s_SizeJson.value("timer", s_Settings.TimerSize);
				s_Settings.UISize = s_SizeJson.value("ui", s_Settings.UISize);
				s_Settings.RowSpacing = s_SizeJson.value("rowSpacing", s_Settings.RowSpacing);
				s_Settings.StatPanelPadding = s_SizeJson.value("statPanelPadding", s_Settings.StatPanelPadding);
			}

			s_Settings.TimerFormat = s_Json.value("timerFormat", s_Settings.TimerFormat);

			if (JsonHas(s_Json, "colors") && s_Json["colors"].is_object())
			{
				const auto& s_ColorJson = s_Json["colors"];
				ColorFromJson(s_ColorJson, "label", s_Settings.LabelColor);
				ColorFromJson(s_ColorJson, "value", s_Settings.ValueColor);
				ColorFromJson(s_ColorJson, "highlight", s_Settings.HighlightColor);
				ColorFromJson(s_ColorJson, "timer", s_Settings.TimerColor);
				ColorFromJson(s_ColorJson, "rating", s_Settings.RatingColor);
				ColorFromJson(s_ColorJson, "ratingBad", s_Settings.RatingBadColor);
				ColorFromJson(s_ColorJson, "specialRating", s_Settings.SpecialRatingColor);
				ColorFromJson(s_ColorJson, "difficulty", s_Settings.DifficultyColor);
				ColorFromJson(s_ColorJson, "hudBackground", s_Settings.HudBackground);
			}

			// Migrate the old default colors without bumping the settings version.
			const ImVec4 s_LegacyHighlight(1.00f, 0.65f, 0.15f, 1.00f);
			if (ColorNear(s_Settings.HighlightColor, s_LegacyHighlight, 0.02f))
				s_Settings.HighlightColor = ImVec4(1.00f, 0.24313726f, 0.24313726f, 1.00f);

			const ImVec4 s_LegacyHudBackground(0.39215687f, 0.39215687f, 0.39215687f, 0.00f);
			if (ColorNear(s_Settings.HudBackground, s_LegacyHudBackground, 0.02f))
				s_Settings.HudBackground = ImVec4(0.39215687f, 0.39215687f, 0.39215687f, 0.44705882f);

			if (JsonHas(s_Json, "positions") && s_Json["positions"].is_object())
			{
				const auto& s_PositionJson = s_Json["positions"];

				if (JsonHas(s_PositionJson, "stats") && s_PositionJson["stats"].is_object())
				{
					const auto& s_StatPositions = s_PositionJson["stats"];

					for (int i = 0; i < c_StatPanelCount; ++i)
						s_Positions.StatValid[i] = Vec2FromJson(s_StatPositions, c_StatPanels[i].Key, s_Positions.Stat[i]);
				}

				s_Positions.RatingValid = Vec2FromJson(s_PositionJson, "rating", s_Positions.Rating);
				s_Positions.TimerValid = Vec2FromJson(s_PositionJson, "timer", s_Positions.Timer);
				s_Positions.LevelValid = Vec2FromJson(s_PositionJson, "level", s_Positions.Level);
				s_Positions.RouletteValid = Vec2FromJson(s_PositionJson, "roulette", s_Positions.Roulette);
			}

			// Older versions defaulted these to different values. Apply the new defaults once.
			if (s_SettingsVersion < 2)
			{
				s_Settings.StatsOnlyInGame = false;
				s_Settings.SnapToGrid = false;
				s_Settings.SnapToPanels = true;
				s_Settings.SnapDistance = 6.0f;
			}
		}
		catch (...)
		{
		}
	}

	void SaveSettings()
	{
		try
		{
			const auto s_Path = GetSettingsPath();
			std::error_code s_Error;
			std::filesystem::create_directories(s_Path.parent_path(), s_Error);

			nlohmann::json s_Json;
			s_Json["version"] = 2;

			{
				auto& s_Show = s_Json["show"];
				s_Show["rating"] = s_Settings.ShowRating;
				s_Show["timer"] = s_Settings.ShowTimer;
				s_Show["levelInfo"] = s_Settings.ShowLevelInfo;
				s_Show["statsOnlyInGame"] = s_Settings.StatsOnlyInGame;
				s_Show["highlightBadStats"] = s_Settings.HighlightBadStats;
			}

			{
				auto& s_StatsJson = s_Json["stats"];

				for (int i = 0; i < c_StatPanelCount; ++i)
					s_StatsJson[c_StatPanels[i].Key] = s_Settings.ShowStat[i];
			}

			{
				auto& s_StylesJson = s_Json["statStyles"];

				for (int i = 0; i < c_StatPanelCount; ++i)
				{
					auto& s_StyleJson = s_StylesJson[c_StatPanels[i].Key];
					s_StyleJson["labelSize"] = s_Settings.StatLabelSize[i];
					s_StyleJson["valueSize"] = s_Settings.StatValueSize[i];
					s_StyleJson["font"] = s_Settings.StatFontStyle[i];
				}
			}

			{
				auto& s_HudStylesJson = s_Json["hudStyles"];

				auto& s_RatingStyleJson = s_HudStylesJson["rating"];
				s_RatingStyleJson["labelSize"] = s_Settings.RatingLabelSizeOverride;
				s_RatingStyleJson["valueSize"] = s_Settings.RatingValueSizeOverride;
				s_RatingStyleJson["font"] = s_Settings.RatingFontStyle;

				auto& s_TimerStyleJson = s_HudStylesJson["timer"];
				s_TimerStyleJson["valueSize"] = s_Settings.TimerSizeOverride;
				s_TimerStyleJson["font"] = s_Settings.TimerFontStyle;

				auto& s_LevelStyleJson = s_HudStylesJson["level"];
				s_LevelStyleJson["labelSize"] = s_Settings.LevelSizeOverride;
				s_LevelStyleJson["font"] = s_Settings.LevelFontStyle;
			}

			{
				auto& s_EditorJson = s_Json["editor"];
				s_EditorJson["snapToGrid"] = s_Settings.SnapToGrid;
				s_EditorJson["snapToPanels"] = s_Settings.SnapToPanels;
				s_EditorJson["dragStatsAsGroup"] = s_Settings.DragStatsAsGroup;
				s_EditorJson["gridSize"] = s_Settings.GridSize;
				s_EditorJson["snapDistance"] = s_Settings.SnapDistance;
			}

			{
				auto& s_ExperimentalJson = s_Json["experimental"];
				s_ExperimentalJson["predictSpecialRatings"] = s_Settings.PredictSpecialRatings;
			}

			{
				auto& s_DebugJson = s_Json["debug2016"];
				s_DebugJson["dressableOnly"] = s_Settings.Debug2016DressableOnly;
				s_DebugJson["headHeight"] = s_Settings.Debug2016HeadHeight;
				s_DebugJson["distanceScale"] = s_Settings.Debug2016DistanceScale;
				s_DebugJson["maxDistance"] = s_Settings.Debug2016MaxDistance;
			}

			{
				auto& s_RouletteJson = s_Json["roulette"];
				s_RouletteJson["showHud"] = s_Settings.ShowRouletteHud;
				s_RouletteJson["onlyInGame"] = s_Settings.RouletteOnlyInGame;
				s_RouletteJson["mission"] = s_Settings.RouletteMission;
				s_RouletteJson["extras"] = s_Settings.RouletteExtras;
			}

			{
				auto& s_FontJson = s_Json["fonts"];
				s_FontJson["regular"] = s_Settings.RegularFont;
				s_FontJson["bold"] = s_Settings.BoldFont;
				s_FontJson["timer"] = s_Settings.TimerFont;
				s_FontJson["boldLabels"] = s_Settings.BoldLabels;
				s_FontJson["boldValues"] = s_Settings.BoldValues;
				s_FontJson["boldRating"] = s_Settings.BoldRating;
				s_FontJson["debug"] = s_Settings.Debug2016Font;
				s_FontJson["debugSize"] = s_Settings.Debug2016FontSize;
				s_FontJson["debugStyle"] = s_Settings.Debug2016FontStyle;
			}

			{
				auto& s_SizeJson = s_Json["sizes"];
				s_SizeJson["label"] = s_Settings.LabelSize;
				s_SizeJson["value"] = s_Settings.ValueSize;
				s_SizeJson["rating"] = s_Settings.RatingSize;
				s_SizeJson["difficulty"] = s_Settings.DifficultySize;
				s_SizeJson["timer"] = s_Settings.TimerSize;
				s_SizeJson["ui"] = s_Settings.UISize;
				s_SizeJson["rowSpacing"] = s_Settings.RowSpacing;
				s_SizeJson["statPanelPadding"] = s_Settings.StatPanelPadding;
			}

			s_Json["timerFormat"] = s_Settings.TimerFormat;

			{
				auto& s_ColorJson = s_Json["colors"];
				s_ColorJson["label"] = ColorToJson(s_Settings.LabelColor);
				s_ColorJson["value"] = ColorToJson(s_Settings.ValueColor);
				s_ColorJson["highlight"] = ColorToJson(s_Settings.HighlightColor);
				s_ColorJson["timer"] = ColorToJson(s_Settings.TimerColor);
				s_ColorJson["rating"] = ColorToJson(s_Settings.RatingColor);
				s_ColorJson["ratingBad"] = ColorToJson(s_Settings.RatingBadColor);
				s_ColorJson["specialRating"] = ColorToJson(s_Settings.SpecialRatingColor);
				s_ColorJson["difficulty"] = ColorToJson(s_Settings.DifficultyColor);
				s_ColorJson["hudBackground"] = ColorToJson(s_Settings.HudBackground);
			}

			{
				auto& s_PositionJson = s_Json["positions"];
				auto& s_StatPositionJson = s_PositionJson["stats"];

				for (int i = 0; i < c_StatPanelCount; ++i)
				{
					if (s_Positions.StatValid[i])
						s_StatPositionJson[c_StatPanels[i].Key] = Vec2ToJson(s_Positions.Stat[i]);
				}

				if (s_Positions.RatingValid)
					s_PositionJson["rating"] = Vec2ToJson(s_Positions.Rating);

				if (s_Positions.TimerValid)
					s_PositionJson["timer"] = Vec2ToJson(s_Positions.Timer);

				if (s_Positions.LevelValid)
					s_PositionJson["level"] = Vec2ToJson(s_Positions.Level);

				if (s_Positions.RouletteValid)
					s_PositionJson["roulette"] = Vec2ToJson(s_Positions.Roulette);
			}

			std::ofstream s_File(s_Path);

			if (s_File)
				s_File << s_Json.dump(2);
		}
		catch (...)
		{
		}
	}

	void ReloadSettingsFromDisk()
	{
		s_Settings = OverlaySettings{};
		s_Positions = OverlayPositions{};
		LoadSettings();
		s_FontsDirty = true;
		s_ReloadPositions = true;
	}

	void EnsureFontFiles()
	{
		if (!s_FontFiles.empty())
			return;

		wchar_t s_WindowsDir[MAX_PATH]{};

		if (!GetWindowsDirectoryW(s_WindowsDir, MAX_PATH))
			return;

		const std::filesystem::path s_FontsDir = std::filesystem::path(s_WindowsDir) / L"Fonts";
		std::error_code s_Error;

		for (std::filesystem::directory_iterator s_It(s_FontsDir, s_Error), s_End; !s_Error && s_It != s_End; s_It.increment(s_Error))
		{
			const auto& s_Path = s_It->path();
			std::string s_Extension = s_Path.extension().string();

			std::transform(s_Extension.begin(), s_Extension.end(), s_Extension.begin(), [](unsigned char p_Char)
			{
				return static_cast<char>(std::tolower(p_Char));
			});

			if (s_Extension == ".ttf" || s_Extension == ".otf")
				s_FontFiles.push_back(s_Path.filename().string());
		}

		std::sort(s_FontFiles.begin(), s_FontFiles.end());

		if (s_FontFiles.empty())
		{
			s_FontFiles.push_back("segoeui.ttf");
			s_FontFiles.push_back("segoeuib.ttf");
			s_FontFiles.push_back("consola.ttf");
		}
	}

	std::filesystem::path GetFontPath(const std::string& p_File)
	{
		wchar_t s_WindowsDir[MAX_PATH]{};

		if (!GetWindowsDirectoryW(s_WindowsDir, MAX_PATH))
			return {};

		return std::filesystem::path(s_WindowsDir) / L"Fonts" / std::filesystem::path(p_File);
	}

	ImFont* AddFont(const std::string& p_File, float p_BaseSize)
	{
		ImGuiIO& s_Io = ImGui::GetIO();

		if (!p_File.empty())
		{
			const auto s_Path = GetFontPath(p_File);
			std::error_code s_Error;

			if (!s_Path.empty() && std::filesystem::exists(s_Path, s_Error))
			{
				ImFontConfig s_Config;
				s_Config.OversampleH = 2;
				s_Config.OversampleV = 1;
				s_Config.PixelSnapH = false;

				if (auto* s_Font = s_Io.Fonts->AddFontFromFileTTF(s_Path.string().c_str(), p_BaseSize, &s_Config))
					return s_Font;
			}
		}

		ImFontConfig s_Config;
		s_Config.SizePixels = p_BaseSize;

		return s_Io.Fonts->AddFontDefaultVector(&s_Config);
	}

	void RebuildFonts()
	{
		if (!s_Initialized)
		{
			s_FontsDirty = true;
			return;
		}

		EnsureFontFiles();

		ImGuiIO& s_Io = ImGui::GetIO();

		ImGui_ImplDX9_InvalidateDeviceObjects();
		s_Io.Fonts->Clear();

		s_Fonts.Regular = AddFont(s_Settings.RegularFont, 16.0f);
		s_Fonts.Bold = AddFont(s_Settings.BoldFont.empty() ? s_Settings.RegularFont : s_Settings.BoldFont, 16.0f);
		s_Fonts.Timer = AddFont(s_Settings.TimerFont.empty() ? s_Settings.BoldFont : s_Settings.TimerFont, 16.0f);
		s_Fonts.Debug = AddFont(s_Settings.Debug2016Font.empty() ? s_Settings.BoldFont : s_Settings.Debug2016Font, 16.0f);

		s_Io.FontDefault = s_Fonts.Regular;

		s_FontsDirty = false;
	}

	void DrawHudText(ImFont* p_Font, float p_Size, const char* p_Text, const ImVec4& p_Color, bool p_Centered)
	{
		ImFont* s_Font = p_Font ? p_Font : ImGui::GetFont();
		const float s_Size = p_Size > 0.0f ? p_Size : ImGui::GetFontSize();
		const ImVec2 s_TextSize = s_Font->CalcTextSizeA(s_Size, FLT_MAX, 0.0f, p_Text);
		const ImVec2 s_Position = ImGui::GetCursorScreenPos();

		float s_X = s_Position.x;

		if (p_Centered)
			s_X += (ImGui::GetContentRegionAvail().x - s_TextSize.x) * 0.5f;

		ImDrawList* s_DrawList = ImGui::GetWindowDrawList();
		const ImU32 s_Color = ImGui::ColorConvertFloat4ToU32(p_Color);

		s_DrawList->AddText(s_Font, s_Size, ImVec2(s_X + 1.0f, s_Position.y + 1.0f), IM_COL32(0, 0, 0, 180), p_Text);
		s_DrawList->AddText(s_Font, s_Size, ImVec2(s_X, s_Position.y), s_Color, p_Text);

		ImGui::Dummy(ImVec2(s_TextSize.x, s_TextSize.y));
	}

	float GetUiScale()
	{
		const ImVec2 s_Display = ImGui::GetIO().DisplaySize;

		if (s_Display.x <= 0.0f || s_Display.y <= 0.0f)
			return 1.0f;

		const float s_Scale = (std::min)(s_Display.x / 1600.0f, s_Display.y / 900.0f);
		return (std::clamp)(s_Scale, 0.75f, 1.25f);
	}

	ImVec2 ScalePresetPosition(float p_X, float p_Y)
	{
		// The built-in presets were authored against a 1280x960 game viewport.
		constexpr float s_ReferenceWidth = 1280.0f;
		constexpr float s_ReferenceHeight = 960.0f;

		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		return ImVec2(
			s_Viewport->WorkPos.x + (p_X / s_ReferenceWidth) * s_Viewport->WorkSize.x,
			s_Viewport->WorkPos.y + (p_Y / s_ReferenceHeight) * s_Viewport->WorkSize.y);
	}

	std::string FormatTimer(double p_Seconds, const std::string& p_Format)
	{
		if (p_Seconds < 0.0)
			p_Seconds = 0.0;

		const int s_WholeSeconds = static_cast<int>(p_Seconds);
		const int s_Milliseconds = static_cast<int>((p_Seconds - s_WholeSeconds) * 1000.0);
		const int s_Hours = s_WholeSeconds / 3600;
		const int s_Minutes = (s_WholeSeconds / 60) % 60;
		const int s_Seconds = s_WholeSeconds % 60;

		const std::string s_Format = p_Format.empty() ? "HH:MM:SS.000" : p_Format;
		std::string s_Result;
		s_Result.reserve(s_Format.size() + 16);

		for (size_t i = 0; i < s_Format.size();)
		{
			const char s_Char = s_Format[i];
			size_t s_Run = 1;

			while (i + s_Run < s_Format.size() && s_Format[i + s_Run] == s_Char)
				++s_Run;

			if (s_Char == 'H' || s_Char == 'M' || s_Char == 'S')
			{
				const int s_Value = s_Char == 'H' ? s_Hours : s_Char == 'M' ? s_Minutes : s_Seconds;

				if (s_Run >= 2)
					s_Result += std::format("{:02d}", s_Value);
				else
					s_Result += std::to_string(s_Value);

				i += s_Run;
			}
			else if (s_Char == '0')
			{
				const size_t s_Digits = (std::min)(s_Run, static_cast<size_t>(3));
				const std::string s_Fraction = std::format("{:03d}", s_Milliseconds);
				s_Result += s_Fraction.substr(0, s_Digits);
				i += s_Run;
			}
			else
			{
				s_Result += s_Char;
				++i;
			}
		}

		return s_Result;
	}

	void SendOverlayOption(const char* p_Type, bool p_Enabled)
	{
		if (g_Module && g_Module->Pipe())
			g_Module->Pipe()->SendPipeMessage(p_Type, p_Enabled ? "true" : "false");
	}

	void SetCheatsFromOverlay(bool p_Enabled)
	{
		if (g_Module->CheatsEnabled() == p_Enabled)
			return;

		g_Module->SetCheatsEnabled(p_Enabled);
		SendOverlayOption("EC", p_Enabled);
	}

	void SetUnlimitedSavesFromOverlay(bool p_Enabled)
	{
		if (g_Module->UnlimitedSaves() == p_Enabled)
			return;

		g_Module->SetUnlimitedSaves(p_Enabled);
		SendOverlayOption("US", p_Enabled);
	}

	void SetHitman2016ModeFromOverlay(bool p_Enabled)
	{
		if (g_Module->Hitman2016Mode() == p_Enabled)
			return;

		g_Module->SetHitman2016Mode(p_Enabled);
		SendOverlayOption("H2", p_Enabled);
	}

	void SetDebug2016FromOverlay(bool p_Enabled)
	{
		if (g_Module->Debug2016() == p_Enabled)
			return;

		g_Module->SetDebug2016(p_Enabled);
		SendOverlayOption("2D", p_Enabled);
	}

	void SnapHudPosition(ImGuiWindow* p_Window, ImVec2& p_Position)
	{
		if (!s_Settings.SnapToPanels)
			return;

		const float s_Tolerance = (std::max)(1.0f, s_Settings.SnapDistance);
		const ImVec2 s_Size = p_Window->Size;

		float s_BestX = FLT_MAX;
		float s_BestY = FLT_MAX;
		float s_LineX = 0.0f;
		float s_LineY = 0.0f;
		bool s_SnapX = false;
		bool s_SnapY = false;

		const float s_AnchorsX[3] = { p_Position.x, p_Position.x + s_Size.x * 0.5f, p_Position.x + s_Size.x };
		const float s_AnchorsY[3] = { p_Position.y, p_Position.y + s_Size.y * 0.5f, p_Position.y + s_Size.y };

		const auto s_CheckX = [&](float p_Line)
		{
			for (const float s_Anchor : s_AnchorsX)
			{
				const float s_Delta = p_Line - s_Anchor;

				if (std::fabs(s_Delta) <= s_Tolerance && std::fabs(s_Delta) < std::fabs(s_BestX))
				{
					s_BestX = s_Delta;
					s_LineX = p_Line;
					s_SnapX = true;
				}
			}
		};

		const auto s_CheckY = [&](float p_Line)
		{
			for (const float s_Anchor : s_AnchorsY)
			{
				const float s_Delta = p_Line - s_Anchor;

				if (std::fabs(s_Delta) <= s_Tolerance && std::fabs(s_Delta) < std::fabs(s_BestY))
				{
					s_BestY = s_Delta;
					s_LineY = p_Line;
					s_SnapY = true;
				}
			}
		};

		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const float s_WorkRight = s_Viewport->WorkPos.x + s_Viewport->WorkSize.x;
		const float s_WorkBottom = s_Viewport->WorkPos.y + s_Viewport->WorkSize.y;

		s_CheckX(s_Viewport->WorkPos.x);
		s_CheckX(s_Viewport->WorkPos.x + s_Viewport->WorkSize.x * 0.5f);
		s_CheckX(s_WorkRight);

		s_CheckY(s_Viewport->WorkPos.y);
		s_CheckY(s_Viewport->WorkPos.y + s_Viewport->WorkSize.y * 0.5f);
		s_CheckY(s_WorkBottom);

		if (ImGuiContext* s_Context = ImGui::GetCurrentContext())
		{
			const int s_Frame = ImGui::GetFrameCount();

			for (int i = 0; i < s_Context->Windows.Size; ++i)
			{
				ImGuiWindow* s_Other = s_Context->Windows[i];

				if (!s_Other || s_Other == p_Window)
					continue;

				if (!s_Other->Name || strncmp(s_Other->Name, "##hud_", 6) != 0)
					continue;

				if (s_Other->LastFrameActive < s_Frame - 1)
					continue;

				s_CheckX(s_Other->Pos.x);
				s_CheckX(s_Other->Pos.x + s_Other->Size.x * 0.5f);
				s_CheckX(s_Other->Pos.x + s_Other->Size.x);

				s_CheckY(s_Other->Pos.y);
				s_CheckY(s_Other->Pos.y + s_Other->Size.y * 0.5f);
				s_CheckY(s_Other->Pos.y + s_Other->Size.y);
			}
		}

		if (s_SnapX)
			p_Position.x += s_BestX;

		if (s_SnapY)
			p_Position.y += s_BestY;

		if (s_SnapX || s_SnapY)
		{
			ImDrawList* s_DrawList = ImGui::GetForegroundDrawList();

			if (s_SnapX)
			{
				s_DrawList->AddLine(
					ImVec2(s_LineX, s_Viewport->WorkPos.y),
					ImVec2(s_LineX, s_WorkBottom),
					IM_COL32(120, 220, 255, 170),
					1.0f);
			}

			if (s_SnapY)
			{
				s_DrawList->AddLine(
					ImVec2(s_Viewport->WorkPos.x, s_LineY),
					ImVec2(s_WorkRight, s_LineY),
					IM_COL32(120, 220, 255, 170),
					1.0f);
			}
		}
	}

	void BeginHudWindow(
		const char* p_Id,
		const ImVec2& p_DefaultPosition,
		const ImVec2& p_DefaultPivot,
		float p_Width,
		ImVec2& p_SavedPosition,
		bool& p_PositionValid,
		bool p_Capture,
		bool p_ForcePosition = false,
		int p_StyleTarget = c_StyleTargetNone)
	{
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const ImVec2 s_WorkMin = s_Viewport->WorkPos;
		const ImVec2 s_WorkMax(
			s_Viewport->WorkPos.x + s_Viewport->WorkSize.x,
			s_Viewport->WorkPos.y + s_Viewport->WorkSize.y);

		if (s_ResetPositions || p_ForcePosition)
		{
			ImGui::SetNextWindowPos(p_DefaultPosition, ImGuiCond_Always, p_DefaultPivot);
		}
		else if (s_ReloadPositions)
		{
			if (p_PositionValid)
			{
				const ImVec2 s_Position(
					(std::clamp)(p_SavedPosition.x, s_WorkMin.x, s_WorkMax.x - 32.0f),
					(std::clamp)(p_SavedPosition.y, s_WorkMin.y, s_WorkMax.y - 24.0f));

				ImGui::SetNextWindowPos(s_Position, ImGuiCond_Always);
			}
			else
			{
				ImGui::SetNextWindowPos(p_DefaultPosition, ImGuiCond_Always, p_DefaultPivot);
			}
		}
		else if (!p_PositionValid)
		{
			ImGui::SetNextWindowPos(p_DefaultPosition, ImGuiCond_FirstUseEver, p_DefaultPivot);
		}
		else
		{
			const ImVec2 s_Position(
				(std::clamp)(p_SavedPosition.x, s_WorkMin.x, s_WorkMax.x - 32.0f),
				(std::clamp)(p_SavedPosition.y, s_WorkMin.y, s_WorkMax.y - 24.0f));

			ImGui::SetNextWindowPos(s_Position, ImGuiCond_FirstUseEver);
		}

		if (p_Width > 0.0f)
			ImGui::SetNextWindowSizeConstraints(ImVec2(p_Width, 0.0f), ImVec2(p_Width, FLT_MAX));

		ImGuiWindowFlags s_Flags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoMove;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		ImVec4 s_PanelBackground = s_Settings.HudBackground;

		// The panel fill is an editing aid only. While playing, the HUD is always fully transparent.
		if (!p_Capture)
			s_PanelBackground.w = 0.0f;

		ImGui::PushStyleColor(ImGuiCol_WindowBg, s_PanelBackground);

		ImGui::Begin(p_Id, nullptr, s_Flags);

		if (p_Capture)
		{
			ImGuiWindow* s_Window = ImGui::GetCurrentWindow();
			const bool s_Hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

			if (s_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (p_StyleTarget != c_StyleTargetNone && ImGui::GetIO().KeyCtrl)
				{
					s_StyleTarget = p_StyleTarget;
					s_ShowStyleWindow = true;
					s_SelectedWindow = s_Window->ID;
				}
				else
				{
					s_DragWindow = s_Window->ID;
					s_SelectedWindow = s_Window->ID;

					const ImVec2 s_Mouse = ImGui::GetIO().MousePos;
					s_DragOffset = ImVec2(s_Mouse.x - s_Window->Pos.x, s_Mouse.y - s_Window->Pos.y);

					if (p_StyleTarget >= 0 && s_Settings.DragStatsAsGroup)
					{
						s_GroupDragActive = true;
						s_GroupDragStartWindow = s_Window->Pos;

						for (int i = 0; i < c_StatPanelCount; ++i)
						{
							if (!s_Settings.ShowStat[i])
								continue;

							if (ImGuiWindow* s_Other = ImGui::FindWindowByName(c_StatPanels[i].WindowId))
								s_GroupDragStartPositions[i] = s_Other->Pos;
							else
								s_GroupDragStartPositions[i] = s_Positions.Stat[i];
						}
					}
					else
					{
						s_GroupDragActive = false;
					}
				}
			}

			if (s_DragWindow == s_Window->ID && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				const ImVec2 s_Mouse = ImGui::GetIO().MousePos;
				ImVec2 s_Position(s_Mouse.x - s_DragOffset.x, s_Mouse.y - s_DragOffset.y);

				if (s_Settings.SnapToGrid)
				{
					const float s_Grid = (std::max)(1.0f, s_Settings.GridSize);
					s_Position.x = std::round(s_Position.x / s_Grid) * s_Grid;
					s_Position.y = std::round(s_Position.y / s_Grid) * s_Grid;
				}

				SnapHudPosition(s_Window, s_Position);

				if (s_GroupDragActive && p_StyleTarget >= 0)
				{
					const ImVec2 s_Delta(
						s_Position.x - s_GroupDragStartWindow.x,
						s_Position.y - s_GroupDragStartWindow.y);

					for (int i = 0; i < c_StatPanelCount; ++i)
					{
						if (!s_Settings.ShowStat[i])
							continue;

						const ImVec2 s_GroupPosition(
							s_GroupDragStartPositions[i].x + s_Delta.x,
							s_GroupDragStartPositions[i].y + s_Delta.y);

						s_Positions.Stat[i] = s_GroupPosition;
						s_Positions.StatValid[i] = true;

						if (ImGuiWindow* s_Other = ImGui::FindWindowByName(c_StatPanels[i].WindowId))
							ImGui::SetWindowPos(s_Other, s_GroupPosition);
					}
				}
				else
				{
					ImGui::SetWindowPos(s_Position);
				}
			}

			if (s_DragWindow == s_Window->ID && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				s_DragWindow = 0;
				s_GroupDragActive = false;
			}
		}

		p_SavedPosition = ImGui::GetWindowPos();
		p_PositionValid = true;
	}

	void EndHudWindow()
	{
		ImGui::PopStyleColor(1);
		ImGui::PopStyleVar(4);
		ImGui::End();
	}

	bool ContainsCaseInsensitive(const std::string& p_Haystack, const char* p_Needle)
	{
		if (!p_Needle || p_Needle[0] == '\0')
			return true;

		std::string s_Haystack = p_Haystack;
		std::string s_Needle = p_Needle;

		std::transform(s_Haystack.begin(), s_Haystack.end(), s_Haystack.begin(), [](unsigned char p_Char)
		{
			return static_cast<char>(std::tolower(p_Char));
		});

		std::transform(s_Needle.begin(), s_Needle.end(), s_Needle.begin(), [](unsigned char p_Char)
		{
			return static_cast<char>(std::tolower(p_Char));
		});

		return s_Haystack.find(s_Needle) != std::string::npos;
	}

	bool FontCombo(const char* p_Label, std::string& p_Value, char* p_Filter, size_t p_FilterSize)
	{
		bool s_Changed = false;

		if (ImGui::BeginCombo(p_Label, p_Value.empty() ? "(default)" : p_Value.c_str()))
		{
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputTextWithHint("##filter", "Filter fonts...", p_Filter, p_FilterSize);

			for (const auto& s_File : s_FontFiles)
			{
				if (p_Filter[0] != '\0' && !ContainsCaseInsensitive(s_File, p_Filter))
					continue;

				if (ImGui::Selectable(s_File.c_str(), s_File == p_Value))
				{
					p_Value = s_File;
					s_Changed = true;
				}
			}

			ImGui::EndCombo();
		}

		return s_Changed;
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		const bool s_OverlayReady = s_Initialized && s_CaptureInput
			&& g_Module && g_Module->OverlayEnabled()
			&& GetCurrentThreadId() == s_RenderThreadId;

		if (s_OverlayReady)
		{
			ImGuiIO& s_Io = ImGui::GetIO();

			switch (msg)
			{
			case WM_SETCURSOR:
				if (LOWORD(lParam) == HTCLIENT)
				{
					ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
					SetCursor(nullptr);
					return TRUE;
				}
				break;

			case WM_MOUSEMOVE:
				if (s_Io.WantCaptureMouse)
					return TRUE;
				break;

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:
			case WM_MOUSEWHEEL:
			case WM_MOUSEHWHEEL:
				ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

				if (s_Io.WantCaptureMouse)
					return TRUE;
				break;

			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_CHAR:
			case WM_SYSCHAR:
				ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

				if (s_Io.WantCaptureKeyboard)
					return TRUE;
				break;

			default:
				ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
				break;
			}
		}

		if (s_OriginalWndProc)
			return CallWindowProcW(s_OriginalWndProc, hWnd, msg, wParam, lParam);

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	bool Initialize(IDirect3DDevice9* p_Device)
	{
		D3DDEVICE_CREATION_PARAMETERS s_Params{};
		if (FAILED(p_Device->GetCreationParameters(&s_Params)))
		{
			Log("Could not query D3D9 creation parameters. Overlay won't work.\n");
			return false;
		}

		s_Window = FindGameWindow(s_Params.hFocusWindow);

		if (!s_Window)
		{
			Log("Could not determine the game window. Overlay won't work.\n");
			return false;
		}

		s_RenderThreadId = GetCurrentThreadId();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		LoadSettings();

		std::error_code s_SettingsError;
		if (!std::filesystem::exists(GetSettingsPath(), s_SettingsError))
			SaveSettings();

		EnsureFontFiles();
		s_FontsDirty = true;

		ImGuiIO& s_Io = ImGui::GetIO();
		s_Io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NoMouseCursorChange;
		s_Io.IniFilename = nullptr;
		s_Io.LogFilename = nullptr;

		ImGui::StyleColorsDark();

		ImGuiStyle& s_Style = ImGui::GetStyle();
		s_Style.WindowRounding = 6.0f;
		s_Style.FrameRounding = 4.0f;
		s_Style.WindowPadding = ImVec2(12.0f, 10.0f);
		s_Style.ItemSpacing = ImVec2(8.0f, 4.0f);

		if (!ImGui_ImplWin32_Init(s_Window))
		{
			Log("Could not initialize the ImGui Win32 backend. Overlay won't work.\n");
			ImGui::DestroyContext();
			return false;
		}

		if (!ImGui_ImplDX9_Init(p_Device))
		{
			Log("Could not initialize the ImGui D3D9 backend. Overlay won't work.\n");
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		s_OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(s_Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProc)));

		if (s_OriginalWndProc == &WndProc)
		{
			Log("The game window procedure is already hooked by a previous HM3 module. Overlay input won't work.\n");
			s_OriginalWndProc = nullptr;
		}
		else if (!s_OriginalWndProc)
		{
			Log("Could not install the ImGui window procedure. Overlay input won't work.\n");
		}

		s_Initialized = true;
		Log("ImGui overlay initialized.\n");

		return true;
	}

	const char* GetDifficultyName(uint8_t p_Difficulty)
	{
		switch (p_Difficulty)
		{
		case 0: return "Rookie";
		case 1: return "Normal";
		case 2: return "Expert";
		case 3: return "Professional";
		default: return "Unknown";
		}
	}

	enum class ItemState
	{
		None,
		Weapons,
		Suit,
		Both
	};

	struct HudStats
	{
		int Witnesses = 0;
		int BodiesFound = 0;
		int CoversBlown = 0;
		int Accidents = 0;
		bool CameraCaught = false;
		bool FriskFailed = false;
		ItemState ItemsLeft = ItemState::None;
		int EnemiesKilled = 0;
		int EnemiesWounded = 0;
		int PolicemenKilled = 0;
		int PolicemenWounded = 0;
		int InnocentsKilled = 0;
		int InnocentsWounded = 0;
		int ShotsHit = 0;
		int PreferredWeapon = 0;
		int PreferredWeaponKills = 0;
		int Rating1Total = 0;
	};

	const char* GetItemsLeftText(ItemState p_State)
	{
		switch (p_State)
		{
		case ItemState::Weapons: return "Weapons";
		case ItemState::Suit: return "Suit";
		case ItemState::Both: return "Weapons & Suit";
		default: return "No";
		}
	}

	HudStats GetHudStats(const HM3Stats* p_Stats, const ZHM3GameData* p_GameData)
	{
		HudStats s_Stats;

		if (!p_Stats)
			return s_Stats;

		s_Stats.Witnesses = p_Stats->m_Witnesses;
		s_Stats.BodiesFound = p_Stats->m_BodiesFound + p_Stats->m_UnconsciousBodiesFound;

		if (p_GameData && p_GameData->m_nDifficulty > 1)
			s_Stats.BodiesFound += p_Stats->m_TargetBodiesFound;

		s_Stats.CoversBlown = p_Stats->m_CoverBlown;
		s_Stats.CameraCaught = p_Stats->m_CameraCaught > 0;
		s_Stats.FriskFailed = p_Stats->m_FriskFailed > 0;
		s_Stats.Accidents = p_Stats->m_AccidentKills;

		if (p_GameData && p_GameData->m_nDifficulty == 3)
		{
			const bool s_Weapons = p_Stats->m_CustomWeaponsLeftOnLevel > 0;
			const bool s_Suit = p_Stats->m_SuitLeftOnLevel != 0;

			if (s_Weapons && s_Suit)
				s_Stats.ItemsLeft = ItemState::Both;
			else if (s_Weapons)
				s_Stats.ItemsLeft = ItemState::Weapons;
			else if (s_Suit)
				s_Stats.ItemsLeft = ItemState::Suit;
		}

		s_Stats.EnemiesKilled = p_Stats->m_EnemiesKilled;
		s_Stats.EnemiesWounded = p_Stats->m_EnemiesWounded;
		s_Stats.PolicemenKilled = p_Stats->m_PoliceMenKilled;
		s_Stats.PolicemenWounded = p_Stats->m_PoliceMenWounded;
		s_Stats.InnocentsKilled = p_Stats->m_InnocentsKilled;
		s_Stats.InnocentsWounded = p_Stats->m_InnocentsWounded;
		s_Stats.ShotsHit = p_Stats->m_ShotsHit;
		s_Stats.PreferredWeapon = p_Stats->m_PreferredWeapon;
		s_Stats.PreferredWeaponKills = p_Stats->m_Unknown05;
		s_Stats.Rating1Total = p_Stats->m_Rating1Total;

		return s_Stats;
	}

	struct StatValue
	{
		std::string Text;
		bool Bad = false;
	};

	StatValue GetStatValue(int p_Index, const HudStats& p_Stats)
	{
		switch (p_Index)
		{
		case 0: return { std::to_string(p_Stats.Witnesses), p_Stats.Witnesses > 0 };
		case 1: return { std::to_string(p_Stats.BodiesFound), p_Stats.BodiesFound > 0 };
		case 2: return { std::to_string(p_Stats.CoversBlown), p_Stats.CoversBlown > 0 };
		case 3: return { p_Stats.CameraCaught ? "Yes" : "No", p_Stats.CameraCaught };
		case 4: return { p_Stats.FriskFailed ? "Yes" : "No", p_Stats.FriskFailed };
		case 5: return { GetItemsLeftText(p_Stats.ItemsLeft), p_Stats.ItemsLeft != ItemState::None };
		case 6: return { std::to_string(p_Stats.Accidents), false };
		case 7: return { std::to_string(p_Stats.EnemiesKilled), p_Stats.EnemiesKilled > 0 };
		case 8: return { std::to_string(p_Stats.EnemiesWounded), p_Stats.EnemiesWounded > 0 };
		case 9: return { std::to_string(p_Stats.PolicemenKilled), p_Stats.PolicemenKilled > 0 };
		case 10: return { std::to_string(p_Stats.PolicemenWounded), p_Stats.PolicemenWounded > 0 };
		case 11: return { std::to_string(p_Stats.InnocentsKilled), p_Stats.InnocentsKilled > 0 };
		case 12: return { std::to_string(p_Stats.InnocentsWounded), p_Stats.InnocentsWounded > 0 };
		case 13: return { std::to_string(p_Stats.ShotsHit), false };
		case 14:
		{
			const int s_Killed = p_Stats.EnemiesKilled + p_Stats.PolicemenKilled + p_Stats.InnocentsKilled;
			return { std::to_string(s_Killed), s_Killed > 0 };
		}
		case 15:
		{
			const int s_Harmed = p_Stats.EnemiesWounded + p_Stats.PolicemenWounded + p_Stats.InnocentsWounded;
			return { std::to_string(s_Harmed), s_Harmed > 0 };
		}
		default: return { "-", false };
		}
	}

	float GetStatLabelSize(int p_Index)
	{
		const float s_Size = s_Settings.StatLabelSize[p_Index];
		return s_Size > 0.0f ? s_Size : s_Settings.LabelSize;
	}

	float GetStatValueSize(int p_Index)
	{
		const float s_Size = s_Settings.StatValueSize[p_Index];
		return s_Size > 0.0f ? s_Size : s_Settings.ValueSize;
	}

	ImFont* GetStatLabelFont(int p_Index)
	{
		const int s_Style = s_Settings.StatFontStyle[p_Index];
		ImFont* s_Font = nullptr;

		if (s_Style < 0)
			s_Font = s_Settings.BoldLabels ? s_Fonts.Bold : s_Fonts.Regular;
		else
			s_Font = s_Style == 1 ? s_Fonts.Bold : s_Fonts.Regular;

		return s_Font ? s_Font : ImGui::GetFont();
	}

	ImFont* GetStatValueFont(int p_Index)
	{
		const int s_Style = s_Settings.StatFontStyle[p_Index];
		ImFont* s_Font = nullptr;

		if (s_Style < 0)
			s_Font = s_Settings.BoldValues ? s_Fonts.Bold : s_Fonts.Regular;
		else
			s_Font = s_Style == 1 ? s_Fonts.Bold : s_Fonts.Regular;

		return s_Font ? s_Font : ImGui::GetFont();
	}

	float GetStatPanelWidth(int p_Index, const HudStats& p_Stats)
	{
		if (p_Index < 0 || p_Index >= c_StatPanelCount)
			return 0.0f;

		const StatValue s_Value = GetStatValue(p_Index, p_Stats);
		ImFont* s_LabelFont = GetStatLabelFont(p_Index);
		ImFont* s_ValueFont = GetStatValueFont(p_Index);

		const float s_LabelWidth = s_LabelFont->CalcTextSizeA(
			GetStatLabelSize(p_Index),
			FLT_MAX,
			0.0f,
			c_StatPanels[p_Index].Label).x;

		const float s_ValueWidth = s_ValueFont->CalcTextSizeA(
			GetStatValueSize(p_Index),
			FLT_MAX,
			0.0f,
			s_Value.Text.c_str()).x;

		const float s_Padding = (std::max)(0.0f, s_Settings.StatPanelPadding);
		return (std::max)(s_LabelWidth, s_ValueWidth) + s_Padding * 2.0f;
	}

	void DrawStatPanel(int p_Index, const HudStats& p_Stats)
	{
		if (p_Index < 0 || p_Index >= c_StatPanelCount)
			return;

		const StatValue s_Value = GetStatValue(p_Index, p_Stats);
		ImFont* s_LabelFont = GetStatLabelFont(p_Index);
		ImFont* s_ValueFont = GetStatValueFont(p_Index);
		const ImVec4& s_ValueColor = (s_Value.Bad && s_Settings.HighlightBadStats)
			? s_Settings.HighlightColor
			: s_Settings.ValueColor;

		DrawHudText(s_LabelFont, GetStatLabelSize(p_Index), c_StatPanels[p_Index].Label, s_Settings.LabelColor, true);
		DrawHudText(s_ValueFont, GetStatValueSize(p_Index), s_Value.Text.c_str(), s_ValueColor, true);
	}

	ImFont* GetHudFont(int p_Style, ImFont* p_Fallback)
	{
		if (p_Style < 0)
			return p_Fallback ? p_Fallback : ImGui::GetFont();

		ImFont* s_Font = p_Style == 1 ? s_Fonts.Bold : s_Fonts.Regular;
		return s_Font ? s_Font : ImGui::GetFont();
	}

	float GetHudSize(float p_Override, float p_Default)
	{
		return p_Override > 0.0f ? p_Override : p_Default;
	}

	const char* PredictSpecialRating(const HudStats& p_Stats, int p_TotalNpcs)
	{
		if (p_TotalNpcs <= 0)
			return nullptr;

		const int s_Stealth = static_cast<int>(std::round(CalculateRating1Raw()));
		const int s_Accidents = p_Stats.Accidents;

		// Live per-weapon kill counts, read from the level control's weapon
		// usage container on the game thread. These are available during play,
		// unlike the stats fields the game only fills in at mission end.
		LiveWeaponKill s_LiveKills[HM3Module::c_MaxLiveWeaponKills];
		const int s_LiveKillCount = g_Module->GetLiveWeaponKills(s_LiveKills, HM3Module::c_MaxLiveWeaponKills);

		int s_PreferredWeapon = p_Stats.PreferredWeapon;
		int s_PreferredWeaponKills = p_Stats.PreferredWeaponKills;

		if (s_LiveKillCount > 0)
		{
			s_PreferredWeapon = 0;
			s_PreferredWeaponKills = 0;

			for (int i = 0; i < s_LiveKillCount; ++i)
			{
				if (s_LiveKills[i].Count > s_PreferredWeaponKills)
				{
					s_PreferredWeapon = s_LiveKills[i].ItemId;
					s_PreferredWeaponKills = s_LiveKills[i].Count;
				}
			}
		}

		const auto s_GetKills = [&](std::initializer_list<int> p_Weapons)
		{
			if (s_LiveKillCount <= 0)
			{
				// Fall back to the mission-end values when the live container
				// has not been populated yet.
				for (const int s_WeaponId : p_Weapons)
				{
					if (s_PreferredWeapon == s_WeaponId)
						return s_PreferredWeaponKills;
				}

				return 0;
			}

			int s_Kills = 0;

			for (int i = 0; i < s_LiveKillCount; ++i)
			{
				for (const int s_WeaponId : p_Weapons)
				{
					if (s_LiveKills[i].ItemId == s_WeaponId)
					{
						s_Kills += s_LiveKills[i].Count;
						break;
					}
				}
			}

			return s_Kills;
		};

		const auto s_KillsAbove = [&](std::initializer_list<int> p_Weapons, double p_Fraction)
		{
			return s_GetKills(p_Weapons) > p_TotalNpcs * p_Fraction;
		};

		const auto s_StealthBetween = [&](int p_Min, int p_Max)
		{
			return s_Stealth > p_Min && s_Stealth < p_Max;
		};

		// Evaluated in the same order as the game's special-rating function.
		// The numbers are Blood Money item IDs:
		//   4, 28-31 = sniper rifles   16 = nail gun      38 = fiber wire
		//   55/317 = hedge cutter      42 = kitchen knife 40 = bowie knife
		//   56 = pitchfork             50 = marine sword  51-54 = syringes
		//   46/318 = pickaxe           201/316 = hammer   39/314 = fire extinguisher
		//   235/319 = screwdriver      44 = nail file     37/313 = baseball bat
		//   47 = stun gun
		if (s_KillsAbove({ 16 }, 0.80) && s_StealthBetween(20, 80))
			return "Needle Pumper";

		if (s_KillsAbove({ 31, 30, 29, 28, 4 }, 0.50) && s_StealthBetween(30, 80))
			return "The Russian Hare";

		if (s_Accidents > p_TotalNpcs * 0.50 && s_StealthBetween(20, 70))
			return "Insurance Agent";

		if (s_KillsAbove({ 38 }, 0.50) && s_Stealth < 40)
			return "Pianoman";

		if (s_KillsAbove({ 38 }, 0.50) && s_Stealth > 40)
			return "Garrotteer";

		if (s_KillsAbove({ 55, 317 }, 0.65) && s_StealthBetween(20, 80))
			return "Insane Florist";

		if (s_KillsAbove({ 42 }, 0.65) && s_StealthBetween(20, 80))
			return "Sushi Chef";

		if (s_KillsAbove({ 40 }, 0.65) && s_StealthBetween(20, 65))
			return "King of the Jungle";

		if (s_KillsAbove({ 56 }, 0.65) && s_StealthBetween(15, 45))
			return "Prince of Darkness";

		if (s_KillsAbove({ 56 }, 0.65) && s_StealthBetween(45, 80))
			return "Beelzebub";

		if (s_KillsAbove({ 50 }, 0.75) && s_StealthBetween(20, 70))
			return "Swashbuckler";

		if (s_KillsAbove({ 54, 53, 52, 51 }, 0.65) && s_StealthBetween(15, 55))
			return "Dr. Death";

		if (s_KillsAbove({ 46, 318 }, 0.75) && s_StealthBetween(30, 85))
			return "Skull Splitter";

		if (s_KillsAbove({ 201, 316 }, 0.75) && s_StealthBetween(20, 75))
			return "Raging Bluecollar";

		if (s_KillsAbove({ 39, 314 }, 0.65) && s_StealthBetween(20, 60))
			return "Frenzied Firefighter";

		if (s_KillsAbove({ 235, 319 }, 0.65) && s_StealthBetween(20, 60))
			return "Evil Eye Gouger";

		if (s_KillsAbove({ 44 }, 0.65) && s_StealthBetween(15, 55))
			return "Murderous Manicurist";

		if (s_KillsAbove({ 37, 313 }, 0.75) && s_StealthBetween(25, 80))
			return "Slugger";

		if (s_KillsAbove({ 47 }, 0.75) && s_StealthBetween(15, 65))
			return "Electrocuter";

		return nullptr;
	}

	void DrawRating(const ZHM3GameData* p_GameData, const HudStats& p_HudStats)
	{
		bool s_UseGoodColor = false;
		bool s_IsSpecialRating = false;
		const char* s_RatingText = "No Rating";
		const auto* s_Stats = g_Module->Pointers()->ZHM3LevelControl__m_stats;
		const char* s_SpecialRatingText = s_Stats ? GetSpecialRatingName(s_Stats->m_SpecialRating) : nullptr;

		if (s_SpecialRatingText)
		{
			s_RatingText = s_SpecialRatingText;
			s_UseGoodColor = true;
			s_IsSpecialRating = true;
		}
		else if (s_Stats && p_GameData)
		{
			const char* s_PredictedRating = s_Settings.PredictSpecialRatings
				? PredictSpecialRating(p_HudStats, GetTotalLevelNpcs())
				: nullptr;

			if (s_PredictedRating)
			{
				s_RatingText = s_PredictedRating;
				s_UseGoodColor = true;
				s_IsSpecialRating = true;
			}
			else
			{
				const auto [s_Perfect, s_Rating] = GetRating();
				s_UseGoodColor = s_Perfect;
				s_RatingText = s_Rating;
			}
		}

		const ImVec4& s_RatingColor = s_IsSpecialRating
			? s_Settings.SpecialRatingColor
			: (s_UseGoodColor ? s_Settings.RatingColor : s_Settings.RatingBadColor);
		ImFont* s_RatingFont = GetHudFont(
			s_Settings.RatingFontStyle,
			s_Settings.BoldRating ? s_Fonts.Bold : s_Fonts.Regular);
		ImFont* s_DifficultyFont = GetHudFont(s_Settings.RatingFontStyle, s_Fonts.Regular);
		const float s_RatingDrawSize = GetHudSize(s_Settings.RatingValueSizeOverride, s_Settings.RatingSize);
		const float s_DifficultyDrawSize = GetHudSize(s_Settings.RatingLabelSizeOverride, s_Settings.DifficultySize);

		DrawHudText(s_RatingFont, s_RatingDrawSize, s_RatingText, s_RatingColor, false);

		if (g_Module->CheatsActive() || g_Module->UnlimitedSaves())
		{
			ImGui::SameLine(0.0f, 0.0f);
			DrawHudText(s_RatingFont, s_RatingDrawSize, " (!)", s_Settings.HighlightColor, false);
		}

		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		const char* s_Difficulty = p_GameData ? GetDifficultyName(p_GameData->m_nDifficulty) : "Unknown";
		DrawHudText(s_DifficultyFont, s_DifficultyDrawSize, s_Difficulty, s_Settings.DifficultyColor, false);
	}

	void DrawDebug2016()
	{
		if (!g_Module->Debug2016())
			return;

		auto* s_GameData = (g_Module->Pointers()->g_pGameData && *g_Module->Pointers()->g_pGameData)
			? *g_Module->Pointers()->g_pGameData
			: nullptr;

		if (!s_GameData || !s_GameData->m_pHitman)
			return;

		const auto* s_Hitman = s_GameData->m_pHitman;
		auto** s_SysInterface = g_Module->Pointers()->g_pSysInterface;

		SVector3 s_PlayerPos{};
		bool s_HasPlayerTransform = false;

		if (s_Hitman->m_pBaseGeom && s_Hitman->m_pBaseGeom->m_pParent)
		{
			s_PlayerPos = s_Hitman->m_pBaseGeom->m_pParent->m_vPos;
			s_HasPlayerTransform = true;
		}

		if (!(s_SysInterface && *s_SysInterface && (*s_SysInterface)->m_pRenderer))
			return;

		const auto s_MainCamera = (*s_SysInterface)->m_pRenderer->GetCamera(0);

		if (!s_MainCamera)
			return;

		const auto s_CameraPtr = reinterpret_cast<ZCAMERA*>(g_Module->Functions()->ZGEOM_RefToPtr(s_MainCamera));

		if (!s_CameraPtr || !s_CameraPtr->m_pBaseGeom)
			return;

		const SCameraData s_CameraData = {
			.FovDegrees = s_CameraPtr->GetFOV(),
			.Near = s_CameraPtr->m_fNear,
			.Far = s_CameraPtr->m_fFar,
			.ViewWidth = ImGui::GetIO().DisplaySize.x,
			.ViewHeight = ImGui::GetIO().DisplaySize.y,
			.RotationMatrix = s_CameraPtr->m_pBaseGeom->m_mMat,
			.Position = s_CameraPtr->m_pBaseGeom->m_vPos,
		};

		ImDrawList* s_DrawList = ImGui::GetBackgroundDrawList();
		ImFont* s_LabelFont = GetHudFont(s_Settings.Debug2016FontStyle, s_Fonts.Debug);
		const float s_BaseSize = (std::max)(8.0f, s_Settings.Debug2016FontSize * GetUiScale());
		const ImU32 s_DressableColor = IM_COL32(120, 255, 150, 230);
		const ImU32 s_OtherColor = IM_COL32(190, 190, 190, 170);
		const float s_Height = (std::max)(1.0f, s_Settings.Debug2016HeadHeight);

		for (uint32_t i = 0; i < s_GameData->m_nActorCount; ++i)
		{
			const auto s_Actor = s_GameData->m_aActors[i];

			if (!s_Actor)
				continue;

			const bool s_Dressable = s_Actor->GetHitmanAs()
				&& g_Module->Functions()->ZGEOM_RefToPtr(s_Actor->GetHitmanAs());

			if (s_Settings.Debug2016DressableOnly && !s_Dressable)
				continue;

			if (!s_Actor->m_pBaseGeom || !s_Actor->m_pBaseGeom->m_pParent)
				continue;

			const auto& s_Mat = s_Actor->m_pBaseGeom->m_pParent->m_mMat;
			const auto& s_Feet = s_Actor->m_pBaseGeom->m_pParent->m_vPos;

			float s_Distance = 0.0f;

			if (s_HasPlayerTransform)
			{
				const auto s_Delta = XMVectorSet(
					s_Feet.x - s_PlayerPos.x,
					s_Feet.y - s_PlayerPos.y,
					s_Feet.z - s_PlayerPos.z,
					0.0f);

				s_Distance = std::sqrt(XMVectorGetX(XMVector3LengthSq(s_Delta)));
			}

			if (s_HasPlayerTransform
				&& s_Settings.Debug2016MaxDistance > 0.0f
				&& s_Distance > s_Settings.Debug2016MaxDistance)
			{
				continue;
			}
			const float s_DistanceScale = (std::clamp)(
				0.5f + 600.0f / (s_Distance + 100.0f),
				0.35f,
				6.0f) * (std::max)(0.1f, s_Settings.Debug2016DistanceScale);
			const float s_Size = (std::max)(8.0f, s_BaseSize * s_DistanceScale);

			const SVector3 s_Head = {
				s_Feet.x + s_Mat.yAxis.x * s_Height,
				s_Feet.y + s_Mat.yAxis.y * s_Height,
				s_Feet.z + s_Mat.yAxis.z * s_Height,
			};

			const auto s_HeadScreen = WorldToScreen(s_CameraData, s_Head);
			const auto s_FeetScreen = WorldToScreen(s_CameraData, s_Feet);

			if (!s_HeadScreen && !s_FeetScreen)
				continue;

			const ImU32 s_Color = s_Dressable ? s_DressableColor : s_OtherColor;

			const std::string s_ActorId = std::to_string(i);
			const ImVec2 s_TextSize = s_LabelFont->CalcTextSizeA(s_Size, FLT_MAX, 0.0f, s_ActorId.c_str());
			ImVec2 s_TextPos;

			if (s_HeadScreen)
				s_TextPos = ImVec2(s_HeadScreen->x - s_TextSize.x * 0.5f, s_HeadScreen->y - s_TextSize.y);
			else
				s_TextPos = ImVec2(s_FeetScreen->x - s_TextSize.x * 0.5f, s_FeetScreen->y - s_TextSize.y * 2.5f);

			s_DrawList->AddText(
				s_LabelFont,
				s_Size,
				ImVec2(s_TextPos.x + 1.0f, s_TextPos.y + 1.0f),
				IM_COL32(0, 0, 0, 200),
				s_ActorId.c_str());

			s_DrawList->AddText(s_LabelFont, s_Size, s_TextPos, s_Color, s_ActorId.c_str());
		}
	}

	ImVec4 GetRouletteColor(Roulette::Color p_Color)
	{
		switch (p_Color)
		{
		case Roulette::Color::Highlight: return c_RouletteRed;
		case Roulette::Color::Blue: return c_RouletteBlue;
		case Roulette::Color::Green: return c_RouletteGreen;
		default: return s_Settings.ValueColor;
		}
	}

	struct RouletteToken
	{
		std::string Text;
		Roulette::Color Color = Roulette::Color::Normal;
		bool SpaceBefore = false;
	};

	std::vector<RouletteToken> TokenizeRouletteLine(const std::vector<Roulette::Segment>& p_Segments)
	{
		std::vector<RouletteToken> s_Tokens;
		bool s_PendingSpace = false;

		for (const auto& s_Segment : p_Segments)
		{
			const std::string& s_Text = s_Segment.Text;
			size_t s_Index = 0;

			while (s_Index < s_Text.size())
			{
				if (s_Text[s_Index] == ' ')
				{
					s_PendingSpace = true;
					++s_Index;
					continue;
				}

				const size_t s_Start = s_Index;

				while (s_Index < s_Text.size() && s_Text[s_Index] != ' ')
					++s_Index;

				s_Tokens.push_back({
					s_Text.substr(s_Start, s_Index - s_Start),
					s_Segment.Color,
					s_PendingSpace });

				s_PendingSpace = false;
			}
		}

		return s_Tokens;
	}

	void DrawRouletteSegments(ImFont* p_Font, float p_Size, const std::vector<Roulette::Segment>& p_Segments, float p_WrapWidth)
	{
		const auto s_Tokens = TokenizeRouletteLine(p_Segments);

		if (s_Tokens.empty())
		{
			ImGui::Dummy(ImVec2(0.0f, p_Size));
			return;
		}

		ImFont* s_Font = p_Font ? p_Font : ImGui::GetFont();
		ImDrawList* s_DrawList = ImGui::GetWindowDrawList();
		const ImVec2 s_Start = ImGui::GetCursorScreenPos();

		float s_X = s_Start.x;
		float s_Y = s_Start.y;
		const float s_LineHeight = s_Font->CalcTextSizeA(p_Size, FLT_MAX, 0.0f, "Ag").y + 1.0f;
		const float s_SpaceWidth = s_Font->CalcTextSizeA(p_Size, FLT_MAX, 0.0f, " ").x;
		const float s_MaxX = s_Start.x + (std::max)(p_WrapWidth, 32.0f);

		bool s_LineEmpty = true;

		for (const auto& s_Token : s_Tokens)
		{
			const float s_Width = s_Font->CalcTextSizeA(p_Size, FLT_MAX, 0.0f, s_Token.Text.c_str()).x;

			if (!s_LineEmpty && s_Token.SpaceBefore)
				s_X += s_SpaceWidth;

			if (!s_LineEmpty && s_X + s_Width > s_MaxX)
			{
				s_X = s_Start.x;
				s_Y += s_LineHeight;
				s_LineEmpty = true;
			}

			const ImU32 s_Color = ImGui::ColorConvertFloat4ToU32(GetRouletteColor(s_Token.Color));

			s_DrawList->AddText(
				s_Font,
				p_Size,
				ImVec2(s_X + 1.0f, s_Y + 1.0f),
				IM_COL32(0, 0, 0, 180),
				s_Token.Text.c_str());

			s_DrawList->AddText(
				s_Font,
				p_Size,
				ImVec2(s_X, s_Y),
				s_Color,
				s_Token.Text.c_str());

			s_X += s_Width;
			s_LineEmpty = false;
		}

		ImGui::Dummy(ImVec2(0.0f, (s_Y + s_LineHeight) - s_Start.y));
	}

	void DrawRouletteResult(const Roulette::Result& p_Result, float p_TextSize)
	{
		const float s_WrapWidth = (std::max)(32.0f, ImGui::GetContentRegionAvail().x);

		for (const auto& s_Line : p_Result.Lines)
		{
			if (s_Line.BlankBefore)
				ImGui::Dummy(ImVec2(0.0f, s_Settings.RowSpacing));

			DrawRouletteSegments(s_Fonts.Regular, p_TextSize, s_Line.Segments, s_WrapWidth);
			ImGui::Dummy(ImVec2(0.0f, s_Settings.RowSpacing * 0.35f));
		}

		if (!p_Result.Extras.empty())
		{
			ImGui::Dummy(ImVec2(0.0f, s_Settings.RowSpacing));

			for (const auto& s_Extra : p_Result.Extras)
			{
				std::vector<Roulette::Segment> s_Line;
				s_Line.push_back({ "- ", Roulette::Color::Normal });
				s_Line.push_back(s_Extra);

				DrawRouletteSegments(s_Fonts.Regular, p_TextSize, s_Line, s_WrapWidth);
				ImGui::Dummy(ImVec2(0.0f, s_Settings.RowSpacing * 0.35f));
			}
		}
	}

	void DrawRouletteHud(bool p_Capture, bool p_InGame)
	{
		if (!s_Settings.ShowRouletteHud || !s_HasRouletteResult || s_RouletteResult.Empty())
			return;

		if (s_Settings.RouletteOnlyInGame && !p_InGame)
			return;

		const float s_Scale = GetUiScale();
		const float s_Width = 340.0f * s_Scale;
		const ImVec2 s_DefaultPosition = ScalePresetPosition(977.0f, 570.0f);

		BeginHudWindow(
			"##hud_roulette",
			s_DefaultPosition,
			ImVec2(0.0f, 0.0f),
			s_Width,
			s_Positions.Roulette,
			s_Positions.RouletteValid,
			p_Capture,
			s_ResetRoulettePosition);

		s_ResetRoulettePosition = false;

		const float s_TextSize = (std::max)(10.0f, s_Settings.LabelSize);
		DrawRouletteResult(s_RouletteResult, s_TextSize);

		EndHudWindow();
	}

	void DrawRouletteWindow()
	{
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const float s_Scale = GetUiScale();

		ImGui::SetNextWindowPos(
			ImVec2(
				s_Viewport->WorkPos.x + 40.0f * s_Scale,
				s_Viewport->WorkPos.y + 150.0f * s_Scale),
			ImGuiCond_FirstUseEver);

		ImGui::SetNextWindowSize(ImVec2(380.0f * s_Scale, 0.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Roulette - Originally made by TheKotti##roulette", &s_ShowRoulette, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize * s_Scale);

		ImGui::TextWrapped("Roulette is a tool that aims to give new ideas on how to complete missions. Original concept is made by TheKotti. See: github.com/TheKotti/thekotti.github.io");
		ImGui::Separator();

		const auto s_MissionNames = Roulette::GetMissionNames();
		const int s_MissionCount = Roulette::GetMissionCount();

		if (s_Settings.RouletteMission < 0 || s_Settings.RouletteMission >= s_MissionCount)
			s_Settings.RouletteMission = 0;

		ImGui::Text("Mission");
		ImGui::SetNextItemWidth(-FLT_MIN);

		if (ImGui::BeginCombo("##roulette_mission", s_MissionNames[s_Settings.RouletteMission]))
		{
			for (int i = 0; i < s_MissionCount; ++i)
			{
				if (ImGui::Selectable(s_MissionNames[i], s_Settings.RouletteMission == i))
					s_Settings.RouletteMission = i;
			}

			ImGui::EndCombo();
		}

		const auto s_ExtraNames = Roulette::GetExtraNames();
		const int s_ExtraCount = Roulette::GetExtraCount();

		if (s_Settings.RouletteExtras < 0 || s_Settings.RouletteExtras >= s_ExtraCount)
			s_Settings.RouletteExtras = 0;

		ImGui::Text("Extras");
		ImGui::SetNextItemWidth(-FLT_MIN);

		if (ImGui::BeginCombo("##roulette_extras", s_ExtraNames[s_Settings.RouletteExtras]))
		{
			for (int i = 0; i < s_ExtraCount; ++i)
			{
				if (ImGui::Selectable(s_ExtraNames[i], s_Settings.RouletteExtras == i))
					s_Settings.RouletteExtras = i;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::Button(s_HasRouletteResult ? "Randomize again" : "Make it go", ImVec2(-FLT_MIN, 0.0f)))
		{
			s_RouletteResult = Roulette::Generate(s_Settings.RouletteMission, s_Settings.RouletteExtras);
			s_HasRouletteResult = !s_RouletteResult.Empty();
		}

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::Checkbox("Show in-game HUD", &s_Settings.ShowRouletteHud);
		ImGui::Checkbox("Only show HUD while in a level", &s_Settings.RouletteOnlyInGame);

		if (s_HasRouletteResult)
		{
			ImGui::SeparatorText("Current roll");
			ImGui::TextColored(c_RouletteRed, "%s", s_RouletteResult.Mission.c_str());

			ImGui::BeginChild("##roulette_result", ImVec2(0.0f, 240.0f * s_Scale), true);
			DrawRouletteResult(s_RouletteResult, s_Settings.UISize * s_Scale);
			ImGui::EndChild();

			if (ImGui::Button("Copy result"))
			{
				const std::string s_Text = Roulette::ToPlainText(s_RouletteResult);
				ImGui::SetClipboardText(s_Text.c_str());
			}

			ImGui::SameLine();

			if (ImGui::Button("Clear"))
			{
				s_RouletteResult = Roulette::Result{};
				s_HasRouletteResult = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Reset HUD position"))
				s_ResetRoulettePosition = true;
		}
		else
		{
			ImGui::TextDisabled("No roulette result yet.");
		}

		ImGui::PopFont();
		ImGui::End();
	}

	void DrawOverlayToolbar()
	{
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const float s_Scale = GetUiScale();

		ImGui::SetNextWindowPos(
			ImVec2(s_Viewport->WorkPos.x + 16.0f * s_Scale, s_Viewport->WorkPos.y + 16.0f * s_Scale),
			ImGuiCond_FirstUseEver);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * s_Scale, 6.0f * s_Scale));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f * s_Scale);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, c_ChromeBg);
		ImGui::PushStyleColor(ImGuiCol_Border, c_ChromeBorder);

		if (ImGui::Begin(
			"##overlay_toolbar",
			nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize * s_Scale);

			if (ImGui::Button(s_ShowGameOptions ? "Hide Game Settings" : "Game settings"))
				s_ShowGameOptions = !s_ShowGameOptions;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Enable cheats, unlimited saves, and the game's built-in trainer options.");

			ImGui::SameLine();

			if (ImGui::Button(s_ShowLocations ? "Hide Hitman 2016 Mode" : "Hitman 2016 mode"))
				s_ShowLocations = !s_ShowLocations;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Hitman 2016 mode: custom starting locations, outfits, and debug info.");

			ImGui::SameLine();

			if (ImGui::Button(s_ShowRoulette ? "Hide Roulette" : "Roulette"))
				s_ShowRoulette = !s_ShowRoulette;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("The original concept was made by TheKotti. Fun mode: randomize instructions on how to complete certain levels.");

			ImGui::SameLine();

			if (ImGui::Button(s_ShowEditor ? "Hide Overlay Editor" : "Overlay editor"))
				s_ShowEditor = !s_ShowEditor;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Toggle visible statistics and other HUD panels.");

			ImGui::SameLine();

			if (ImGui::Button(s_ShowSettings ? "Hide Overlay Settings" : "Overlay settings"))
				s_ShowSettings = !s_ShowSettings;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Edit fonts, sizes, colors, timer and alignment.");

			ImGui::SameLine();
			ImGui::TextDisabled("INSERT toggles the overlay windows");

			ImGui::PopFont();
		}

		ImGui::End();

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
	}

	void DrawEditor()
	{
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const float s_Scale = GetUiScale();
		const bool s_Narrow = s_Scale < 0.9f;

		ImGui::SetNextWindowPos(
			s_Narrow
				? ImVec2(s_Viewport->WorkPos.x + 16.0f * s_Scale, s_Viewport->WorkPos.y + 64.0f * s_Scale)
				: ImVec2(s_Viewport->WorkPos.x + s_Viewport->WorkSize.x * 0.5f, s_Viewport->WorkPos.y + 16.0f * s_Scale),
			ImGuiCond_FirstUseEver,
			s_Narrow ? ImVec2(0.0f, 0.0f) : ImVec2(0.5f, 0.0f));

		ImGui::SetNextWindowSize(ImVec2(440.0f * s_Scale, 0.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Overlay Editor##editor", &s_ShowEditor, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize * s_Scale);

		ImGui::TextDisabled("Use the Overlay Editor toolbar button to show or hide this window.");
		ImGui::Separator();

		ImGui::SeparatorText("Statistics panels");

		if (ImGui::Button("Hide all stats"))
		{
			for (int i = 0; i < c_StatPanelCount; ++i)
				s_Settings.ShowStat[i] = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Arrange minimal"))
		{
			s_ArrangeMode = StatArrangeMode::Minimal;
			s_ForceStatPositions = true;

			for (int i = 0; i < c_StatPanelCount; ++i)
				s_Settings.ShowStat[i] = c_StatPanels[i].InMinimalMode;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Enable minimal mode and arrange its statistic panels using the minimal layout.");

		ImGui::SameLine();

		if (ImGui::Button("Arrange detailed"))
		{
			s_ArrangeMode = StatArrangeMode::Detailed;
			s_ForceStatPositions = true;

			for (int i = 0; i < c_StatPanelCount; ++i)
				s_Settings.ShowStat[i] = c_StatPanels[i].Group == StatGroup::Detailed;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Enable detailed mode and arrange its statistic panels using the detailed layout.");

		if (ImGui::Button("Minimal mode"))
		{
			for (int i = 0; i < c_StatPanelCount; ++i)
				s_Settings.ShowStat[i] = c_StatPanels[i].InMinimalMode;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Enable the compact statistic set and hide the rest.");

		ImGui::SameLine();

		if (ImGui::Button("Detailed mode"))
		{
			for (int i = 0; i < c_StatPanelCount; ++i)
				s_Settings.ShowStat[i] = c_StatPanels[i].Group == StatGroup::Detailed;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Enable the detailed statistics and hide the minimal ones.");

		const auto s_DrawStatGroup = [](const char* p_TableId, StatGroup p_Group)
		{
			if (!ImGui::BeginTable(p_TableId, 2, ImGuiTableFlags_SizingStretchSame))
				return;

			for (int i = 0; i < c_StatPanelCount; ++i)
			{
				if (c_StatPanels[i].Group != p_Group)
					continue;

				ImGui::TableNextColumn();
				ImGui::Checkbox(c_StatPanels[i].Label, &s_Settings.ShowStat[i]);
			}

			ImGui::EndTable();
		};

		ImGui::SeparatorText("Detailed statistics");
		s_DrawStatGroup("##stat_visibility_detailed", StatGroup::Detailed);

		ImGui::SeparatorText("Minimal statistics");
		s_DrawStatGroup("##stat_visibility_minimal", StatGroup::Minimal);

		ImGui::TextDisabled("Ctrl+Click a statistic panel to edit its font.");

		ImGui::SeparatorText("Other HUD panels");
		ImGui::Checkbox("Rating", &s_Settings.ShowRating);
		ImGui::SameLine();
		ImGui::Checkbox("Timer", &s_Settings.ShowTimer);
		ImGui::SameLine();
		ImGui::Checkbox("Level info", &s_Settings.ShowLevelInfo);
		ImGui::Checkbox("Show stats only in game", &s_Settings.StatsOnlyInGame);

		ImGui::Separator();

		if (ImGui::Button("Reset positions"))
		{
			s_ResetPositions = true;

			for (int i = 0; i < c_StatPanelCount; ++i)
				s_Positions.StatValid[i] = false;

			s_Positions.RatingValid = false;
			s_Positions.TimerValid = false;
			s_Positions.LevelValid = false;
			s_Positions.RouletteValid = false;
			s_ResetRoulettePosition = true;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Move every HUD panel back to its default position and discard saved panel positions.");

		ImGui::SameLine();

		if (ImGui::Button("Save settings"))
			SaveSettings();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Write all current overlay settings to HM3Overlay.json.\nIncludes HUD positions, fonts, sizes, colors, timer format and per-panel styles.\nThe same save action is available from the other overlay window.");

		ImGui::SameLine();

		if (ImGui::Button("Reload settings"))
			ReloadSettingsFromDisk();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Reload overlay settings and HUD positions from HM3Overlay.json.\nUnsaved changes will be discarded.");

		ImGui::PopFont();
		ImGui::End();
	}

	void DrawHudStyleWindow()
	{
		const bool s_IsStat = s_StyleTarget >= 0 && s_StyleTarget < c_StatPanelCount;

		if (!s_IsStat
			&& s_StyleTarget != c_StyleTargetRating
			&& s_StyleTarget != c_StyleTargetTimer
			&& s_StyleTarget != c_StyleTargetLevel)
		{
			s_ShowStyleWindow = false;
			return;
		}

		const char* s_TitleText = "";
		float* s_LabelSizePtr = nullptr;
		float s_DefaultLabelSize = 0.0f;
		const char* s_LabelSizeTitle = "Label size";
		const char* s_LabelOverrideText = "Override label size";
		float* s_ValueSizePtr = nullptr;
		float s_DefaultValueSize = 0.0f;
		const char* s_ValueSizeTitle = "Value size";
		const char* s_ValueOverrideText = "Override value size";
		int* s_FontStylePtr = nullptr;

		if (s_IsStat)
		{
			s_TitleText = c_StatPanels[s_StyleTarget].Label;
			s_LabelSizePtr = &s_Settings.StatLabelSize[s_StyleTarget];
			s_DefaultLabelSize = s_Settings.LabelSize;
			s_ValueSizePtr = &s_Settings.StatValueSize[s_StyleTarget];
			s_DefaultValueSize = s_Settings.ValueSize;
			s_FontStylePtr = &s_Settings.StatFontStyle[s_StyleTarget];
		}
		else if (s_StyleTarget == c_StyleTargetRating)
		{
			s_TitleText = "Rating";
			s_LabelSizePtr = &s_Settings.RatingLabelSizeOverride;
			s_DefaultLabelSize = s_Settings.DifficultySize;
			s_LabelSizeTitle = "Difficulty size";
			s_LabelOverrideText = "Override difficulty size";
			s_ValueSizePtr = &s_Settings.RatingValueSizeOverride;
			s_DefaultValueSize = s_Settings.RatingSize;
			s_ValueSizeTitle = "Rating size";
			s_ValueOverrideText = "Override rating size";
			s_FontStylePtr = &s_Settings.RatingFontStyle;
		}
		else if (s_StyleTarget == c_StyleTargetTimer)
		{
			s_TitleText = "Timer";
			s_ValueSizePtr = &s_Settings.TimerSizeOverride;
			s_DefaultValueSize = s_Settings.TimerSize;
			s_ValueSizeTitle = "Timer size";
			s_ValueOverrideText = "Override timer size";
			s_FontStylePtr = &s_Settings.TimerFontStyle;
		}
		else
		{
			s_TitleText = "Level info";
			s_LabelSizePtr = &s_Settings.LevelSizeOverride;
			s_DefaultLabelSize = s_Settings.LabelSize;
			s_FontStylePtr = &s_Settings.LevelFontStyle;
		}

		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const float s_Scale = GetUiScale();
		const std::string s_Title = std::format("{} style##hud_style", s_TitleText);

		ImGui::SetNextWindowPos(
			ImVec2(
				s_Viewport->WorkPos.x + s_Viewport->WorkSize.x * 0.5f,
				s_Viewport->WorkPos.y + s_Viewport->WorkSize.y * 0.35f),
			ImGuiCond_FirstUseEver,
			ImVec2(0.5f, 0.0f));

		ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f * s_Scale, 0.0f), ImVec2(320.0f * s_Scale, FLT_MAX));

		if (!ImGui::Begin(
			s_Title.c_str(),
			&s_ShowStyleWindow,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize * s_Scale);

		ImGui::TextDisabled("Per-panel font override.");
		ImGui::Separator();
		ImGui::Text("%s", s_TitleText);
		ImGui::Separator();

		if (s_LabelSizePtr)
		{
			bool s_LabelOverride = *s_LabelSizePtr > 0.0f;

			if (ImGui::Checkbox(s_LabelOverrideText, &s_LabelOverride))
				*s_LabelSizePtr = s_LabelOverride ? s_DefaultLabelSize : 0.0f;

			if (s_LabelOverride)
			{
				ImGui::SetNextItemWidth(190.0f * s_Scale);
				ImGui::SliderFloat(s_LabelSizeTitle, s_LabelSizePtr, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
			}
		}

		if (s_ValueSizePtr)
		{
			bool s_ValueOverride = *s_ValueSizePtr > 0.0f;

			if (ImGui::Checkbox(s_ValueOverrideText, &s_ValueOverride))
				*s_ValueSizePtr = s_ValueOverride ? s_DefaultValueSize : 0.0f;

			if (s_ValueOverride)
			{
				ImGui::SetNextItemWidth(190.0f * s_Scale);
				ImGui::SliderFloat(s_ValueSizeTitle, s_ValueSizePtr, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
			}
		}

		static const char* s_FontStyles[] = { "Inherit", "Regular", "Bold" };
		int s_FontStyleIndex = *s_FontStylePtr + 1;

		ImGui::SetNextItemWidth(190.0f * s_Scale);

		if (ImGui::BeginCombo("Font style", s_FontStyles[s_FontStyleIndex]))
		{
			for (int i = 0; i < 3; ++i)
			{
				if (ImGui::Selectable(s_FontStyles[i], s_FontStyleIndex == i))
					*s_FontStylePtr = i - 1;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::Button("Reset to global"))
		{
			if (s_LabelSizePtr)
				*s_LabelSizePtr = 0.0f;

			if (s_ValueSizePtr)
				*s_ValueSizePtr = 0.0f;

			*s_FontStylePtr = -1;
		}

		ImGui::PopFont();
		ImGui::End();
	}

	void DrawGameOptions()
	{
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const float s_Scale = GetUiScale();

		ImGui::SetNextWindowPos(
			ImVec2(
				s_Viewport->WorkPos.x + 16.0f * s_Scale,
				s_Viewport->WorkPos.y + s_Viewport->WorkSize.y - 48.0f * s_Scale),
			ImGuiCond_FirstUseEver,
			ImVec2(0.0f, 1.0f));

		ImGui::SetNextWindowSize(ImVec2(300.0f * s_Scale, 0.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Game##gameoptions", &s_ShowGameOptions, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize * s_Scale);

		bool s_Cheats = g_Module->CheatsEnabled();

		if (ImGui::Checkbox("Enable cheats", &s_Cheats))
			SetCheatsFromOverlay(s_Cheats);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Toggle in-game cheats. When enabled, the built-in trainer options appear below. Press C in game to open the game's own trainer menu. Shift+C passes the mission instantly.");

		if (s_Cheats)
		{
			ImGui::SeparatorText("Cheat options");

			CheatState s_CheatState = g_Module->GetCheatState();
			bool s_CheatChanged = false;

			const auto s_CheatCheckbox = [&](const char* p_Label, bool& p_Value, const char* p_Tooltip)
			{
				if (ImGui::Checkbox(p_Label, &p_Value))
					s_CheatChanged = true;

				if (p_Tooltip && ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", p_Tooltip);
			};

			s_CheatCheckbox("Infinite ammo", s_CheatState.InfAmmo, "Weapons never run out of reserve ammunition.");
			s_CheatCheckbox("Infinite clip", s_CheatState.InfClip, "Weapons never need to reload.");
			s_CheatCheckbox("God mode", s_CheatState.GodMode, "Hitman cannot be killed.");
			s_CheatCheckbox("Invisible mode", s_CheatState.InvisibleMode, "NPCs cannot see Hitman.");

			if (ImGui::SliderFloat("Time multiplier", &s_CheatState.TimeMultiplier, 0.0f, 10.0f, "%.2f"))
				s_CheatChanged = true;

			// Dragging can settle on 0.99 or 1.01. A double click (either
			// button) snaps back to the default speed so 1.00 can be restored
			// exactly.
			if (ImGui::IsItemHovered()
				&& (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right)))
			{
				s_CheatState.TimeMultiplier = 1.0f;
				s_CheatChanged = true;
			}

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Game speed. 1.00 is normal speed. Double-click to reset to 1.00. Ctrl+left-click the slider to enter a custom value.");

			if (s_CheatChanged)
			{
				// Keep the main "Enable cheats" flag in the state so toggling an
				// individual option cannot close this section unexpectedly.
				s_CheatState.EnableCheats = s_Cheats;
				g_Module->SetCheatState(s_CheatState);
			}

			// The OSD toggle lives with the cheat options because hiding the
			// game's HUD is treated as a cheat by the detection logic.
			CheatState s_HudState = g_Module->GetCheatState();
			bool s_ShowOSD = s_HudState.ShowOSD;

			if (ImGui::Checkbox("Show OSD", &s_ShowOSD))
			{
				s_HudState.ShowOSD = s_ShowOSD;
				g_Module->SetCheatState(s_HudState);
			}

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Show the game's on-screen HUD. Enabled by default; disabling it is treated as a cheat.");
		}

		bool s_UnlimitedSaves = g_Module->UnlimitedSaves();

		if (ImGui::Checkbox("Unlimited saves", &s_UnlimitedSaves))
			SetUnlimitedSavesFromOverlay(s_UnlimitedSaves);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Toggle unlimited saves. You will be able to save the game even on Pro difficulty.");

		ImGui::Checkbox("Predict Special Ratings [Experimental]", &s_Settings.PredictSpecialRatings);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Estimates the final special rating from the live statistics and the level's NPC count. The real game rating replaces it when the mission ends.");

		if (s_Cheats || s_UnlimitedSaves)
			ImGui::TextColored(s_Settings.HighlightColor, "Active cheats are marked on the HUD.");

		ImGui::PopFont();
		ImGui::End();
	}

	void DrawSettings()
	{
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const float s_Scale = GetUiScale();
		const float s_MaxWidth = (std::min)(720.0f * s_Scale, s_Viewport->WorkSize.x - 32.0f * s_Scale);
		const float s_MinWidth = (std::min)(500.0f * s_Scale, s_MaxWidth);
		const float s_MaxHeight = (std::max)(240.0f * s_Scale, s_Viewport->WorkSize.y - 32.0f * s_Scale);

		ImGui::SetNextWindowPos(
			ImVec2(s_Viewport->WorkPos.x + s_Viewport->WorkSize.x - 24.0f * s_Scale, s_Viewport->WorkPos.y + 16.0f * s_Scale),
			ImGuiCond_FirstUseEver,
			ImVec2(1.0f, 0.0f));

		ImGui::SetNextWindowSizeConstraints(
			ImVec2(s_MinWidth, 0.0f),
			ImVec2(s_MaxWidth, s_MaxHeight));

		if (!ImGui::Begin("Overlay Settings##settings", &s_ShowSettings, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize * s_Scale);

		if (ImGui::BeginTabBar("##overlay_settings_tabs", ImGuiTabBarFlags_FittingPolicyScroll))
		{
			if (ImGui::BeginTabItem("Alignment"))
			{
				ImGui::Checkbox("Snap to panels and screen", &s_Settings.SnapToPanels);

				ImGui::SameLine();
				ImGui::SetNextItemWidth(140.0f * s_Scale);
				ImGui::SliderFloat("Snap distance", &s_Settings.SnapDistance, 1.0f, 24.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);

				ImGui::Checkbox("Snap to grid", &s_Settings.SnapToGrid);

				if (s_Settings.SnapToGrid)
				{
					ImGui::SameLine();
					ImGui::SetNextItemWidth(140.0f * s_Scale);
					ImGui::SliderFloat("Grid size", &s_Settings.GridSize, 2.0f, 50.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				}

				ImGui::Checkbox("Drag visible statistics as a group", &s_Settings.DragStatsAsGroup);

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Dragging any statistic panel moves all currently visible statistic panels together.");

				ImGui::TextDisabled("Drag a panel to align it against other panels, screen edges and center.");
				ImGui::TextDisabled("Ctrl+Arrow nudges the last selected panel; hold Shift for 10 px steps.");

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Fonts"))
			{
				static char s_RegularFilter[64] = "";
				static char s_BoldFilter[64] = "";
				static char s_DebugFilter[64] = "";

				if (FontCombo("Regular", s_Settings.RegularFont, s_RegularFilter, sizeof(s_RegularFilter)))
					s_FontsDirty = true;

				if (FontCombo("Bold", s_Settings.BoldFont, s_BoldFilter, sizeof(s_BoldFilter)))
					s_FontsDirty = true;

				ImGui::SeparatorText("2016 debug info");
				if (FontCombo("Actor label font", s_Settings.Debug2016Font, s_DebugFilter, sizeof(s_DebugFilter)))
					s_FontsDirty = true;

				ImGui::SliderFloat("Actor label size", &s_Settings.Debug2016FontSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);

				static const char* s_DebugStyles[] = { "Inherit", "Regular", "Bold" };
				int s_DebugStyleIndex = s_Settings.Debug2016FontStyle + 1;
				ImGui::SetNextItemWidth(190.0f * s_Scale);

				if (ImGui::BeginCombo("Actor label style", s_DebugStyles[s_DebugStyleIndex]))
				{
					for (int i = 0; i < 3; ++i)
					{
						if (ImGui::Selectable(s_DebugStyles[i], s_DebugStyleIndex == i))
							s_Settings.Debug2016FontStyle = i - 1;
					}

					ImGui::EndCombo();
				}

				ImGui::TextDisabled("Used for the actor ID numbers drawn near NPCs.");

				ImGui::Checkbox("Bold labels", &s_Settings.BoldLabels);
				ImGui::SameLine();
				ImGui::Checkbox("Bold values", &s_Settings.BoldValues);
				ImGui::Checkbox("Bold rating", &s_Settings.BoldRating);

				ImGui::TextDisabled("Fonts are loaded from the Windows Fonts folder.");
				ImGui::TextDisabled("Ctrl+Click a statistic or HUD panel in the overlay to override its font size/style.");

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Sizes"))
			{
				ImGui::SliderFloat("Label size", &s_Settings.LabelSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Value size", &s_Settings.ValueSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Rating size", &s_Settings.RatingSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Difficulty size", &s_Settings.DifficultySize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("UI size", &s_Settings.UISize, 8.0f, 32.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Row spacing", &s_Settings.RowSpacing, 0.0f, 32.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Stat panel padding", &s_Settings.StatPanelPadding, 0.0f, 32.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Colors"))
			{
				constexpr auto s_ColorFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf;

				ImGui::SeparatorText("Text");
				ImGui::ColorEdit4("Label", &s_Settings.LabelColor.x, s_ColorFlags);
				ImGui::ColorEdit4("Value", &s_Settings.ValueColor.x, s_ColorFlags);
				ImGui::ColorEdit4("Highlight", &s_Settings.HighlightColor.x, s_ColorFlags);
				ImGui::ColorEdit4("Timer", &s_Settings.TimerColor.x, s_ColorFlags);
				ImGui::ColorEdit4("Rating", &s_Settings.RatingColor.x, s_ColorFlags);
				ImGui::ColorEdit4("Rating (bad)", &s_Settings.RatingBadColor.x, s_ColorFlags);
				ImGui::ColorEdit4("Special rating", &s_Settings.SpecialRatingColor.x, s_ColorFlags);
				ImGui::ColorEdit4("Difficulty", &s_Settings.DifficultyColor.x, s_ColorFlags);

				ImGui::SeparatorText("Behavior");
				ImGui::Checkbox("Highlight bad stats", &s_Settings.HighlightBadStats);

				ImGui::SeparatorText("HUD panels");
				ImGui::ColorEdit4("Panel background", &s_Settings.HudBackground.x, s_ColorFlags);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Timer"))
			{
				static char s_TimerFilter[64] = "";

				if (FontCombo("Font", s_Settings.TimerFont, s_TimerFilter, sizeof(s_TimerFilter)))
					s_FontsDirty = true;

				ImGui::SliderFloat("Timer size", &s_Settings.TimerSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);

				char s_TimerFormatBuffer[64];
				strncpy_s(s_TimerFormatBuffer, s_Settings.TimerFormat.c_str(), _TRUNCATE);

				if (ImGui::InputText("Timer format", s_TimerFormatBuffer, sizeof(s_TimerFormatBuffer)))
					s_Settings.TimerFormat = s_TimerFormatBuffer;

				ImGui::TextDisabled("H/HH = hours, M/MM = minutes, S/SS = seconds, 0/00/000 = fraction.");
				ImGui::TextDisabled("Preview: %s", FormatTimer(3723.456, s_Settings.TimerFormat).c_str());
				ImGui::TextDisabled("Ctrl+Click the Timer panel in the overlay to override its size/font style.");

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::Separator();

		if (ImGui::Button("Reset defaults"))
		{
			s_Settings = OverlaySettings{};
			s_FontsDirty = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("Save settings"))
			SaveSettings();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Write all current overlay settings to HM3Overlay.json.\nIncludes HUD positions, fonts, sizes, colors, timer format and per-panel styles.\nThe same save action is available from the other overlay window.");

		ImGui::SameLine();

		if (ImGui::Button("Reload settings"))
			ReloadSettingsFromDisk();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Reload overlay settings and HUD positions from HM3Overlay.json.\nUnsaved changes will be discarded.");

		ImGui::PopFont();
		ImGui::End();
	}

	void DrawActiveWarnings()
	{
		std::string s_Warning;

		if (g_Module->CheatsActive())
			s_Warning = "CHEATS ENABLED";

		if (g_Module->UnlimitedSaves())
		{
			if (!s_Warning.empty())
				s_Warning += "  |  ";

			s_Warning += "UNLIMITED SAVES";
		}

		if (s_Warning.empty())
			return;

		ImDrawList* s_DrawList = ImGui::GetBackgroundDrawList();
		ImFont* s_Font = s_Fonts.Regular ? s_Fonts.Regular : ImGui::GetFont();
		const float s_Scale = GetUiScale();
		const float s_Size = (std::max)(11.0f, s_Settings.UISize * 0.8f * s_Scale);
		const ImVec2 s_TextSize = s_Font->CalcTextSizeA(s_Size, FLT_MAX, 0.0f, s_Warning.c_str());
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const ImVec2 s_Position(
			s_Viewport->WorkPos.x + s_Viewport->WorkSize.x - s_TextSize.x - 16.0f * s_Scale,
			s_Viewport->WorkPos.y + 60.0f * s_Scale);

		s_DrawList->AddRectFilled(
			ImVec2(s_Position.x - 6.0f, s_Position.y - 3.0f),
			ImVec2(s_Position.x + s_TextSize.x + 6.0f, s_Position.y + s_TextSize.y + 3.0f),
			IM_COL32(0, 0, 0, 110),
			4.0f);

		s_DrawList->AddText(s_Font, s_Size, s_Position, IM_COL32(255, 190, 90, 210), s_Warning.c_str());
	}

	void DrawHitman2016LocationsTab()
	{
		const float s_Scale = GetUiScale();

		static std::string s_SelectedLevel;
		static std::string s_SelectedLocation;
		static char s_LocationName[64] = "";
		static std::string s_Status;

		const auto& s_LevelMap = g_Module->GetLevelStartingLocations();
		const std::string s_CurrentLevel = g_Module->GetCurrentLevelKey();
		const bool s_Hitman2016 = g_Module->Hitman2016Mode();

		std::vector<std::string> s_Levels;

		for (const auto& [s_Level, s_Locations] : s_LevelMap)
			s_Levels.push_back(s_Level);

		std::sort(s_Levels.begin(), s_Levels.end(), [](const std::string& p_Left, const std::string& p_Right)
		{
			return GetLevelDisplayName(p_Left) < GetLevelDisplayName(p_Right);
		});

		if (s_SelectedLevel.empty() || std::find(s_Levels.begin(), s_Levels.end(), s_SelectedLevel) == s_Levels.end())
		{
			s_SelectedLevel = !s_CurrentLevel.empty()
				? s_CurrentLevel
				: (!s_Levels.empty() ? s_Levels.front() : std::string());
		}

		if (!s_Hitman2016)
			ImGui::TextColored(s_Settings.HighlightColor, "Hitman 2016 mode is disabled.");
		else
			ImGui::TextDisabled("The last loaded location is restored when this level restarts.");

		ImGui::SeparatorText("Category");

		if (!s_Levels.empty())
		{
			if (ImGui::BeginCombo("##hitman2016_category", GetLevelDisplayName(s_SelectedLevel).c_str()))
			{
				for (const auto& s_Level : s_Levels)
				{
					const bool s_Selected = s_Level == s_SelectedLevel;

					if (ImGui::Selectable(GetLevelDisplayName(s_Level).c_str(), s_Selected))
					{
						s_SelectedLevel = s_Level;
						s_SelectedLocation.clear();
						s_LocationName[0] = '\0';
					}

					if (s_Selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::TextDisabled("Level key: %s", s_SelectedLevel.c_str());
		}
		else
		{
			ImGui::TextDisabled("No levels loaded. Make sure config\\HM3_locations.json exists next to Statman.exe.");
		}

		const bool s_CanUseCurrentLevel = !s_CurrentLevel.empty() && s_CurrentLevel == s_SelectedLevel;

		SMatrix33 s_Rotation;
		SVector3 s_Position;

		if (g_Module->GetPlayerTransform(s_Rotation, s_Position))
		{
			const int s_Outfit = g_Module->GetCurrentOutfitIndex();
			ImGui::Text("Position: %.2f, %.2f, %.2f", s_Position.x, s_Position.y, s_Position.z);
			ImGui::Text("Outfit actor: %d%s", s_Outfit, s_Outfit < 0 ? " (default suit / not found)" : "");
		}
		else
		{
			ImGui::TextDisabled("Player position is only available while in a level.");
		}

		ImGui::SeparatorText("Saved locations");

		const auto s_LevelIt = s_LevelMap.find(s_SelectedLevel);

		if (s_LevelIt == s_LevelMap.end() || s_LevelIt->second.empty())
		{
			ImGui::TextDisabled("No saved locations for this category.");
		}
		else
		{
			if (ImGui::BeginChild("##hitman2016_locations", ImVec2(0.0f, 150.0f * s_Scale), true))
			{
				for (size_t i = 0; i < s_LevelIt->second.size(); ++i)
				{
					const auto& s_Location = s_LevelIt->second[i];

					ImGui::PushID(static_cast<int>(i));

					const bool s_Selected = s_Location.Name == s_SelectedLocation;

					if (ImGui::Selectable(s_Location.Name.c_str(), s_Selected))
					{
						s_SelectedLocation = s_Location.Name;
						std::strncpy(s_LocationName, s_Location.Name.c_str(), sizeof(s_LocationName) - 1);
						s_LocationName[sizeof(s_LocationName) - 1] = '\0';
					}

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Outfit actor: %d", s_Location.Outfit);

					ImGui::PopID();
				}
			}
			ImGui::EndChild();
		}

		const std::string s_LastLocation = g_Module->GetLastLocation(s_SelectedLevel);

		if (!s_LastLocation.empty())
			ImGui::TextDisabled("Last loaded: %s", s_LastLocation.c_str());

		ImGui::SeparatorText("Save current location");

		ImGui::InputText("Name", s_LocationName, sizeof(s_LocationName));

		if (ImGui::Button("Save current location"))
		{
			if (s_LocationName[0] == '\0')
			{
				s_Status = "Enter a name first.";
			}
			else if (std::string(s_LocationName) == "__last")
			{
				s_Status = "'__last' is a reserved name.";
			}
			else if (!s_CanUseCurrentLevel)
			{
				s_Status = "Switch to this level before saving its location.";
			}
			else if (!g_Module->GetPlayerTransform(s_Rotation, s_Position))
			{
				s_Status = "Could not read the player position.";
			}
			else
			{
				LevelStartingLocation s_Location;
				s_Location.Name = s_LocationName;
				s_Location.Rotation = s_Rotation;
				s_Location.Position = s_Position;
				s_Location.Outfit = g_Module->GetCurrentOutfitIndex();

				g_Module->AddOrUpdateLocation(s_SelectedLevel, s_Location);
				g_Module->SetLastLocation(s_SelectedLevel, s_Location.Name);
				g_Module->SaveHitman2016Data();

				s_SelectedLocation = s_Location.Name;
				s_Status = "Saved '" + s_Location.Name + "'.";
			}
		}

		const bool s_HasSelection = !s_SelectedLocation.empty();

		if (ImGui::Button("Load selected") && s_HasSelection)
		{
			if (!s_CanUseCurrentLevel)
			{
				s_Status = "Switch to this level before loading its location.";
			}
			else if (s_LevelIt != s_LevelMap.end())
			{
				for (const auto& s_Location : s_LevelIt->second)
				{
					if (s_Location.Name == s_SelectedLocation)
					{
						g_Module->ApplyStartingLocation(s_Location);
						g_Module->SetLastLocation(s_SelectedLevel, s_Location.Name);
						g_Module->SaveHitman2016Data();
						s_Status = "Loaded '" + s_Location.Name + "'.";
						break;
					}
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Set as last") && s_HasSelection)
		{
			g_Module->SetLastLocation(s_SelectedLevel, s_SelectedLocation);
			g_Module->SaveHitman2016Data();
			s_Status = "Last location set to '" + s_SelectedLocation + "'.";
		}

		ImGui::SameLine();

		if (ImGui::Button("Clear last"))
		{
			g_Module->SetLastLocation(s_SelectedLevel, "");
			g_Module->SaveHitman2016Data();
			s_Status = "Last location cleared.";
		}

		ImGui::SameLine();

		if (ImGui::Button("Delete selected") && s_HasSelection)
		{
			g_Module->DeleteLocation(s_SelectedLevel, s_SelectedLocation);
			g_Module->SaveHitman2016Data();
			s_Status = "Deleted '" + s_SelectedLocation + "'.";
			s_SelectedLocation.clear();
		}

		if (ImGui::Button("Reload from disk"))
		{
			if (g_Module->ReloadHitman2016Data())
				s_Status = "Reloaded HM3_locations.json.";
			else
				s_Status = "Could not read HM3_locations.json.";
		}

		if (!s_Status.empty())
			ImGui::TextWrapped("%s", s_Status.c_str());
	}

	void DrawHitman2016OutfitsTab()
	{
		static char s_OutfitFilter[64] = "";
		static uint32_t s_SelectedOutfitKey = 0;

		auto* s_GameData = (g_Module->Pointers()->g_pGameData && *g_Module->Pointers()->g_pGameData)
			? *g_Module->Pointers()->g_pGameData
			: nullptr;

		if (!s_GameData || !s_GameData->m_pHitman)
		{
			ImGui::TextDisabled("Outfits are only available while in a level.");
			return;
		}

		struct OutfitEntry
		{
			int Index;
			int Count;
			float Distance;
			std::string Name;
		};

		std::unordered_map<uint32_t, OutfitEntry> s_OutfitMap;

		auto* s_Hitman = s_GameData->m_pHitman;
		SVector3 s_PlayerPos{};
		bool s_HasPlayer = false;

		if (s_Hitman->m_pBaseGeom && s_Hitman->m_pBaseGeom->m_pParent)
		{
			s_PlayerPos = s_Hitman->m_pBaseGeom->m_pParent->m_vPos;
			s_HasPlayer = true;
		}

		for (uint32_t i = 0; i < s_GameData->m_nActorCount; ++i)
		{
			auto* s_Actor = s_GameData->m_aActors[i];

			if (!s_Actor)
				continue;

			const auto s_HitmanAs = s_Actor->GetHitmanAs();

			if (!s_HitmanAs)
				continue;

			auto* s_HitmanAsPtr = g_Module->Functions()->ZGEOM_RefToPtr(s_HitmanAs);

			if (!s_HitmanAsPtr)
				continue;

			if (!s_Actor->m_pBaseGeom || !s_Actor->m_pBaseGeom->m_pParent)
				continue;

			// Group by the resolved outfit object, not the per-actor reference.
			// Different actors wearing the same suit share this object.
			const uint32_t s_OutfitKey = reinterpret_cast<uint32_t>(s_HitmanAsPtr);

			const auto& s_Pos = s_Actor->m_pBaseGeom->m_pParent->m_vPos;
			float s_Distance = 0.0f;

			if (s_HasPlayer)
			{
				const auto s_Delta = XMVectorSet(
					s_Pos.x - s_PlayerPos.x,
					s_Pos.y - s_PlayerPos.y,
					s_Pos.z - s_PlayerPos.z,
					0.0f);

				s_Distance = std::sqrt(XMVectorGetX(XMVector3LengthSq(s_Delta)));
			}

			std::string s_Name;

			if (s_Actor->m_pBaseGeom->m_szName)
			{
				s_Name = s_Actor->m_pBaseGeom->m_szName;

				if (s_Name == "Ground")
					s_Name.clear();
			}

			const auto s_Existing = s_OutfitMap.find(s_OutfitKey);

			if (s_Existing == s_OutfitMap.end())
			{
				s_OutfitMap[s_OutfitKey] = {
					static_cast<int>(i),
					1,
					s_Distance,
					s_Name
				};
			}
			else
			{
				++s_Existing->second.Count;

				// Keep the first actor as the stable representative; only update
				// the displayed distance if this actor is closer.
				if (s_Distance < s_Existing->second.Distance)
					s_Existing->second.Distance = s_Distance;
			}
		}

		std::vector<std::pair<uint32_t, OutfitEntry>> s_Outfits;

		for (const auto& [s_Key, s_Outfit] : s_OutfitMap)
			s_Outfits.push_back({ s_Key, s_Outfit });

		// Keep the row order stable while distances change; otherwise the item
		// under the cursor can move between frames and clicks get lost.
		std::sort(s_Outfits.begin(), s_Outfits.end(), [](const auto& p_Left, const auto& p_Right)
		{
			return p_Left.second.Index < p_Right.second.Index;
		});

		ImGui::TextDisabled("Unique outfits found on nearby actors. Use the closest actor as the outfit source.");
		ImGui::InputText("Filter", s_OutfitFilter, sizeof(s_OutfitFilter));

		const int s_CurrentOutfit = g_Module->GetCurrentOutfitIndex();
		ImGui::Text("Current outfit actor: %d", s_CurrentOutfit);

		if (ImGui::BeginTable("##hitman2016_outfits", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("Outfit", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Distance", ImGuiTableColumnFlags_WidthFixed, 80.0f * GetUiScale());
			ImGui::TableSetupColumn("Actors", ImGuiTableColumnFlags_WidthFixed, 60.0f * GetUiScale());
			ImGui::TableHeadersRow();

			for (const auto& [s_Key, s_Outfit] : s_Outfits)
			{
				const std::string s_Suffix = s_Outfit.Name.empty()
					? std::string()
					: (" (" + s_Outfit.Name + ")");
				const std::string s_Display = std::format("Outfit actor {}{}", s_Outfit.Index, s_Suffix);

				if (s_OutfitFilter[0] != '\0' && s_Display.find(s_OutfitFilter) == std::string::npos)
					continue;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				// The ### suffix gives the selectable a stable ImGui ID even
				// though the distance and actor-count columns update in place.
				const std::string s_StableLabel = std::format("{}###outfit_{}", s_Display, s_Key);

				if (ImGui::Selectable(s_StableLabel.c_str(), s_SelectedOutfitKey == s_Key, ImGuiSelectableFlags_SpanAllColumns))
					s_SelectedOutfitKey = s_Key;

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.1f m", s_Outfit.Distance);

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%d", s_Outfit.Count);
			}

			ImGui::EndTable();
		}

		const auto s_SelectedOutfitIt = s_OutfitMap.find(s_SelectedOutfitKey);
		const bool s_HasOutfitSelection = s_SelectedOutfitIt != s_OutfitMap.end();

		if (s_HasOutfitSelection)
		{
			const int s_OutfitActor = s_SelectedOutfitIt->second.Index;

			if (ImGui::Button("Wear selected outfit"))
				g_Module->ApplyOutfit(s_OutfitActor);

			ImGui::SameLine();

			if (ImGui::Button("Go to selected actor"))
			{
				auto* s_Actor = s_GameData->m_aActors[s_OutfitActor];

				if (s_Actor && s_Actor->m_pBaseGeom && s_Actor->m_pBaseGeom->m_pParent)
				{
					LevelStartingLocation s_Location;
					s_Location.Name = "outfit_preview";
					s_Location.Rotation = s_Actor->m_pBaseGeom->m_pParent->m_mMat;
					s_Location.Position = s_Actor->m_pBaseGeom->m_pParent->m_vPos;
					s_Location.Outfit = s_OutfitActor;
					g_Module->ApplyStartingLocation(s_Location);
				}
			}
		}

		// Wrap instead of clipping: the window can be narrower than this line.
		ImGui::PushTextWrapPos(0.0f);
		ImGui::TextDisabled("Outfit names come from the game's base geometry and may simply be 'Ground'. Use distance and the actor visuals to identify them.");
		ImGui::PopTextWrapPos();
	}

	void DrawHitman2016DebugTab()
	{
		bool s_Debug2016 = g_Module->Debug2016();

		if (ImGui::Checkbox("Show actor IDs", &s_Debug2016))
			SetDebug2016FromOverlay(s_Debug2016);

		ImGui::TextDisabled("Debug info is treated as a cheat while enabled.");

		ImGui::Checkbox("Only show actors with a stealable outfit", &s_Settings.Debug2016DressableOnly);
		ImGui::SliderFloat("Actor label height (world units)", &s_Settings.Debug2016HeadHeight, 20.0f, 400.0f, "%.0f");
		ImGui::SliderFloat("Distance label scale", &s_Settings.Debug2016DistanceScale, 0.25f, 3.0f, "%.2f");
		ImGui::SliderFloat("Maximum ID distance", &s_Settings.Debug2016MaxDistance, 0.0f, 10000.0f, "%.0f m");
		ImGui::TextDisabled("0 = no distance limit.");

		ImGui::TextDisabled("Actor label font, size and style are configured in Overlay Settings → Fonts.");

		SMatrix33 s_PlayerMat{};
		SVector3 s_PlayerPos{};
		const bool s_HasPlayer = g_Module->GetPlayerTransform(s_PlayerMat, s_PlayerPos);

		ImGui::SeparatorText("Player");

		if (s_HasPlayer)
		{
			ImGui::Text("Position: %.2f, %.2f, %.2f", s_PlayerPos.x, s_PlayerPos.y, s_PlayerPos.z);
			ImGui::Text("Rotation X: %.2f, %.2f, %.2f", s_PlayerMat.xAxis.x, s_PlayerMat.xAxis.y, s_PlayerMat.xAxis.z);
			ImGui::Text("Rotation Y: %.2f, %.2f, %.2f", s_PlayerMat.yAxis.x, s_PlayerMat.yAxis.y, s_PlayerMat.yAxis.z);
			ImGui::Text("Rotation Z: %.2f, %.2f, %.2f", s_PlayerMat.zAxis.x, s_PlayerMat.zAxis.y, s_PlayerMat.zAxis.z);
		}
		else
		{
			ImGui::TextDisabled("Player position unavailable.");
		}

		ImGui::SeparatorText("Closest outfit");

		int s_ClosestActor = -1;
		float s_ClosestDistanceSq = FLT_MAX;

		auto* s_GameData = (g_Module->Pointers()->g_pGameData && *g_Module->Pointers()->g_pGameData)
			? *g_Module->Pointers()->g_pGameData
			: nullptr;

		if (s_GameData && s_HasPlayer)
		{
			for (uint32_t i = 0; i < s_GameData->m_nActorCount; ++i)
			{
				auto* s_Actor = s_GameData->m_aActors[i];

				if (!s_Actor || !s_Actor->GetHitmanAs())
					continue;

				if (!s_Actor->m_pBaseGeom || !s_Actor->m_pBaseGeom->m_pParent)
					continue;

				const auto& s_Pos = s_Actor->m_pBaseGeom->m_pParent->m_vPos;
				const auto s_Delta = XMVectorSet(
					s_Pos.x - s_PlayerPos.x,
					s_Pos.y - s_PlayerPos.y,
					s_Pos.z - s_PlayerPos.z,
					0.0f);
				const float s_DistanceSq = XMVectorGetX(XMVector3LengthSq(s_Delta));

				if (s_DistanceSq < s_ClosestDistanceSq)
				{
					s_ClosestDistanceSq = s_DistanceSq;
					s_ClosestActor = static_cast<int>(i);
				}
			}
		}

		if (s_ClosestActor >= 0)
			ImGui::Text("Closest outfit: %d (%.1f m)", s_ClosestActor, std::sqrt(s_ClosestDistanceSq));
		else
			ImGui::TextDisabled("No outfit candidate nearby.");

		if (s_HasPlayer)
		{
			static char s_LocationName[64] = "";

			ImGui::InputText("Location name", s_LocationName, sizeof(s_LocationName));

			if (ImGui::Button("Copy location JSON"))
			{
				const std::string s_Name = s_LocationName[0] ? s_LocationName : "new_location";
				const std::string s_Json = std::format(
					"\"{}\": {{\n"
					"    \"position\": [[{:.2f}, {:.2f}, {:.2f}], [{:.2f}, {:.2f}, {:.2f}], [{:.2f}, {:.2f}, {:.2f}], [{:.2f}, {:.2f}, {:.2f}]],\n"
					"    \"outfit\": {}\n"
					"}}",
					s_Name,
					s_PlayerMat.xAxis.x, s_PlayerMat.xAxis.y, s_PlayerMat.xAxis.z,
					s_PlayerMat.yAxis.x, s_PlayerMat.yAxis.y, s_PlayerMat.yAxis.z,
					s_PlayerMat.zAxis.x, s_PlayerMat.zAxis.y, s_PlayerMat.zAxis.z,
					s_PlayerPos.x, s_PlayerPos.y, s_PlayerPos.z,
					s_ClosestActor);

				ImGui::SetClipboardText(s_Json.c_str());
			}
		}

		ImGui::Text("Scene: %s", g_Module->m_SceneName.empty() ? "unknown" : g_Module->m_SceneName.c_str());
	}

	void DrawHitman2016ModeWindow()
	{
		const float s_Scale = GetUiScale();

		ImGui::SetNextWindowSize(ImVec2(500.0f * s_Scale, 0.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(460.0f * s_Scale, 0.0f), ImVec2(560.0f * s_Scale, FLT_MAX));

		if (!ImGui::Begin("Hitman 2016 Mode##hitman2016mode", &s_ShowLocations, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize * s_Scale);

		bool s_Hitman2016 = g_Module->Hitman2016Mode();

		if (ImGui::Checkbox("Enable Hitman 2016 mode", &s_Hitman2016))
			SetHitman2016ModeFromOverlay(s_Hitman2016);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Use custom starting locations with alternative disguises. The last loaded location is restored on the next level load; enabling this option does not teleport immediately.");

		ImGui::TextDisabled("Locations are applied on level load/restart, not immediately when the option is enabled.");

		if (ImGui::BeginTabBar("##hitman2016tabs"))
		{
			if (ImGui::BeginTabItem("Locations"))
			{
				DrawHitman2016LocationsTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Outfits"))
			{
				DrawHitman2016OutfitsTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Debug info"))
			{
				DrawHitman2016DebugTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::PopFont();
		ImGui::End();
	}

	void DrawOverlay()
	{
		const bool s_Capture = s_CaptureInput;

		const auto* s_Stats = g_Module->Pointers()->ZHM3LevelControl__m_stats;
		auto* s_GameData = (g_Module->Pointers()->g_pGameData && *g_Module->Pointers()->g_pGameData)
			? *g_Module->Pointers()->g_pGameData
			: nullptr;

		const SceneInfo* s_SceneInfo = FindSceneInfo(g_Module->m_SceneName);
		const bool s_InGame = s_SceneInfo && s_SceneInfo->InGame;
		const bool s_ShowStats = !s_Settings.StatsOnlyInGame
			|| (s_SceneInfo && (s_SceneInfo->InGame || std::strcmp(s_SceneInfo->State, "Post-Mission") == 0));

		double s_CurrentTimeSeconds = 0.0;

		if (s_Stats && (s_Stats->m_Time > 0 || !s_InGame))
			s_CurrentTimeSeconds = static_cast<double>(s_Stats->m_Time) / 1000.0;
		else if (auto** s_SysInterface = g_Module->Pointers()->g_pSysInterface; s_SysInterface && *s_SysInterface)
			s_CurrentTimeSeconds = static_cast<double>((*s_SysInterface)->m_Ticks) * 0.0009765625;

		s_CurrentTimeSeconds = (std::max)(0.0, s_CurrentTimeSeconds);

		const std::string s_TimeText = FormatTimer(s_CurrentTimeSeconds, s_Settings.TimerFormat);

		const HudStats s_HudStats = GetHudStats(s_Stats, s_GameData);

		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const ImVec2 s_Center = s_Viewport->GetCenter();

		if (s_ShowStats && s_Stats)
		{
			for (int i = 0; i < c_StatPanelCount; ++i)
			{
				if (!s_Settings.ShowStat[i])
					continue;

				const bool s_MinimalLayout = s_ArrangeMode == StatArrangeMode::Minimal;
				const ImVec2 s_DefaultPosition = ScalePresetPosition(
					s_MinimalLayout ? c_StatPanels[i].MinimalX : c_StatPanels[i].DetailedX,
					s_MinimalLayout ? c_StatPanels[i].MinimalY : c_StatPanels[i].DetailedY);

				BeginHudWindow(
					c_StatPanels[i].WindowId,
					s_DefaultPosition,
					ImVec2(0.0f, 0.0f),
					GetStatPanelWidth(i, s_HudStats),
					s_Positions.Stat[i],
					s_Positions.StatValid[i],
					s_Capture,
					s_ForceStatPositions,
					i);

				DrawStatPanel(i, s_HudStats);

				EndHudWindow();
			}

			s_ForceStatPositions = false;
			s_ArrangeMode = StatArrangeMode::None;
		}

		if (s_Settings.ShowRating && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_rating",
				ScalePresetPosition(47.0f, 816.0f),
				ImVec2(0.0f, 0.0f),
				0.0f,
				s_Positions.Rating,
				s_Positions.RatingValid,
				s_Capture,
				false,
				c_StyleTargetRating);

			DrawRating(s_GameData, s_HudStats);

			EndHudWindow();
		}

		if (s_Settings.ShowTimer && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_timer",
				ScalePresetPosition(577.0f, 928.0f),
				ImVec2(0.0f, 0.0f),
				0.0f,
				s_Positions.Timer,
				s_Positions.TimerValid,
				s_Capture,
				false,
				c_StyleTargetTimer);

			DrawHudText(
				GetHudFont(s_Settings.TimerFontStyle, s_Fonts.Timer),
				GetHudSize(s_Settings.TimerSizeOverride, s_Settings.TimerSize),
				s_TimeText.c_str(),
				s_Settings.TimerColor,
				false);

			EndHudWindow();
		}

		if (s_Settings.ShowLevelInfo && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_level",
				ScalePresetPosition(47.0f, 927.0f),
				ImVec2(0.0f, 0.0f),
				0.0f,
				s_Positions.Level,
				s_Positions.LevelValid,
				s_Capture,
				false,
				c_StyleTargetLevel);

			const char* s_LevelName = "No Level";
			const char* s_LevelState = "Menu";

			if (s_SceneInfo)
			{
				s_LevelName = s_SceneInfo->Name;
				s_LevelState = s_SceneInfo->State;
			}
			else if (!g_Module->m_SceneName.empty())
			{
				s_LevelName = g_Module->m_SceneName.c_str();
				s_LevelState = "Unknown";
			}

			const std::string s_LevelText = std::format("{}  |  {}", s_LevelName, s_LevelState);
			DrawHudText(
				GetHudFont(s_Settings.LevelFontStyle, s_Settings.BoldLabels ? s_Fonts.Bold : s_Fonts.Regular),
				GetHudSize(s_Settings.LevelSizeOverride, s_Settings.LabelSize),
				s_LevelText.c_str(),
				s_Settings.LabelColor,
				false);

			EndHudWindow();
		}

		DrawRouletteHud(s_Capture, s_InGame);

		if (s_Capture && s_Settings.SnapToGrid && s_DragWindow != 0)
		{
			ImDrawList* s_Background = ImGui::GetBackgroundDrawList();
			const float s_Grid = (std::max)(2.0f, s_Settings.GridSize);
			const ImU32 s_GridColor = IM_COL32(140, 180, 255, 28);
			const ImU32 s_GuideColor = IM_COL32(180, 210, 255, 70);
			const ImVec2 s_WorkEnd(
				s_Viewport->WorkPos.x + s_Viewport->WorkSize.x,
				s_Viewport->WorkPos.y + s_Viewport->WorkSize.y);

			for (float x = std::floor(s_Viewport->WorkPos.x / s_Grid) * s_Grid; x <= s_WorkEnd.x; x += s_Grid)
				s_Background->AddLine(ImVec2(x, s_Viewport->WorkPos.y), ImVec2(x, s_WorkEnd.y), s_GridColor);

			for (float y = std::floor(s_Viewport->WorkPos.y / s_Grid) * s_Grid; y <= s_WorkEnd.y; y += s_Grid)
				s_Background->AddLine(ImVec2(s_Viewport->WorkPos.x, y), ImVec2(s_WorkEnd.x, y), s_GridColor);

			s_Background->AddLine(ImVec2(s_Center.x, s_Viewport->WorkPos.y), ImVec2(s_Center.x, s_WorkEnd.y), s_GuideColor);
			s_Background->AddLine(ImVec2(s_Viewport->WorkPos.x, s_Center.y), ImVec2(s_WorkEnd.x, s_Center.y), s_GuideColor);
		}

		if (s_Capture && s_SelectedWindow != 0 && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().WantTextInput)
		{
			ImGuiWindow* s_Selected = ImGui::FindWindowByID(s_SelectedWindow);

			if (s_Selected && s_Selected->LastFrameActive >= ImGui::GetFrameCount() - 1)
			{
				const float s_Step = ImGui::GetIO().KeyShift ? 10.0f : 1.0f;
				ImVec2 s_Delta(0.0f, 0.0f);

				if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
					s_Delta.x -= s_Step;

				if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
					s_Delta.x += s_Step;

				if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
					s_Delta.y -= s_Step;

				if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
					s_Delta.y += s_Step;

				if (s_Delta.x != 0.0f || s_Delta.y != 0.0f)
					ImGui::SetWindowPos(ImVec2(s_Selected->Pos.x + s_Delta.x, s_Selected->Pos.y + s_Delta.y));
			}
		}

		s_ResetPositions = false;
		s_ReloadPositions = false;

		DrawDebug2016();

		if (s_Capture)
		{
			DrawOverlayToolbar();

			if (s_ShowEditor)
				DrawEditor();

			if (s_ShowStyleWindow)
				DrawHudStyleWindow();

			if (s_ShowSettings)
				DrawSettings();

			if (s_ShowGameOptions)
				DrawGameOptions();

			if (s_ShowLocations)
				DrawHitman2016ModeWindow();

			if (s_ShowRoulette)
				DrawRouletteWindow();
		}

		DrawActiveWarnings();
	}
}

void Overlay_SetZWINDOWSInstance(void* p_Instance)
{
	SetZWINDOWSInstance(p_Instance);
}

void Overlay_Shutdown()
{
	if (!s_Initialized)
		return;

	UpdateGameCursor(false);

	SaveSettings();

	UpdateActionManager(false);

	s_Initialized = false;
	s_CaptureInput = false;
	s_Fonts.Regular = nullptr;
	s_Fonts.Bold = nullptr;
	s_Fonts.Timer = nullptr;
	s_Fonts.Debug = nullptr;

	if (s_Window && s_OriginalWndProc)
	{
		SetWindowLongPtrW(s_Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_OriginalWndProc));
		s_OriginalWndProc = nullptr;
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

DECLARE_STDCALL_DETOUR(HM3Hooks, HRESULT, IDirect3DDevice9__Reset, IDirect3DDevice9* th, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	if (s_Initialized)
		ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto s_Result = o_IDirect3DDevice9__Reset(th, pPresentationParameters);

	if (SUCCEEDED(s_Result) && s_Initialized)
		ImGui_ImplDX9_CreateDeviceObjects();

	return s_Result;
}

DECLARE_STDCALL_DETOUR(HM3Hooks, HRESULT, IDirect3DDevice9__EndScene, IDirect3DDevice9* th)
{
	if (!g_Module->OverlayEnabled())
	{
		UpdateActionManager(false);
		s_CaptureInput = false;
		UpdateGameCursor(false);
		return o_IDirect3DDevice9__EndScene(th);
	}

	if (!s_Initialized)
	{
		if (s_InitFailed || !Initialize(th))
		{
			s_InitFailed = true;
			return o_IDirect3DDevice9__EndScene(th);
		}
	}

	s_RenderThreadId = GetCurrentThreadId();

	const bool s_ToggleKeyDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;

	if (s_ToggleKeyDown && !s_PrevToggleKeyDown)
	{
		s_CaptureInput = !s_CaptureInput;

		if (s_CaptureInput)
		{
			ClipCursor(nullptr);
			UpdateGameCursor(true);

			// Start every capture session with the cursor centered on the game
			// window. This avoids having to hunt for the ImGui cursor when the
			// physical mouse was left on another monitor.
			RECT s_ClientRect;

			if (s_Window && GetClientRect(s_Window, &s_ClientRect))
			{
				POINT s_Center = {
					(s_ClientRect.right - s_ClientRect.left) / 2,
					(s_ClientRect.bottom - s_ClientRect.top) / 2
				};

				ClientToScreen(s_Window, &s_Center);
				SetCursorPos(s_Center.x, s_Center.y);
			}
		}
		else
		{
			UpdateGameCursor(false);
			SaveSettings();
			s_DragWindow = 0;
		}
	}

	s_PrevToggleKeyDown = s_ToggleKeyDown;

	UpdateActionManager(s_CaptureInput);

	if (s_CaptureInput)
	{
		ClipCursor(nullptr);
		SetCursor(nullptr);
	}

	if (s_FontsDirty)
		RebuildFonts();

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGuiIO& s_Io = ImGui::GetIO();
	s_Io.MouseDrawCursor = s_CaptureInput;

	if (s_CaptureInput)
	{
		const MouseState s_Mouse = GetMouseState();
		s_Io.AddMousePosEvent(s_Mouse.X, s_Mouse.Y);
		s_Io.AddMouseButtonEvent(0, s_Mouse.Left);
		s_Io.AddMouseButtonEvent(1, s_Mouse.Right);
		s_Io.AddMouseButtonEvent(2, s_Mouse.Middle);
	}
	else
	{
		s_Io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
		s_Io.AddMouseButtonEvent(0, false);
		s_Io.AddMouseButtonEvent(1, false);
		s_Io.AddMouseButtonEvent(2, false);
	}

	ImGui::NewFrame();

	DrawOverlay();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return o_IDirect3DDevice9__EndScene(th);
}
