#include <HM3/HM3Hooks.h>
#include <HM3/HM3Module.h>
#include <HM3/HM3Pointers.h>
#include <HM3/HM3Functions.h>
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
#include <filesystem>
#include <fstream>
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

uint32_t CalculateRating1() {
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
	s_Rating1 = s_Rating1 > 100.0 ? 100.0 : s_Rating1;

	return static_cast<uint32_t>(ceil((round(s_Rating1) / 100.0) * 6.0));
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

	struct OverlayPositions
	{
		ImVec2 Left;
		ImVec2 Right;
		ImVec2 Rating;
		ImVec2 Timer;
		ImVec2 Level;

		bool LeftValid = false;
		bool RightValid = false;
		bool RatingValid = false;
		bool TimerValid = false;
		bool LevelValid = false;
	};

	struct OverlaySettings
	{
		bool ShowLeftStats = true;
		bool ShowRightStats = true;
		bool ShowRating = true;
		bool ShowTimer = true;
		bool ShowLevelInfo = false;
		bool StatsOnlyInGame = true;

		std::string RegularFont = "segoeui.ttf";
		std::string BoldFont = "segoeuib.ttf";
		std::string TimerFont = "consola.ttf";

		float LabelSize = 15.0f;
		float ValueSize = 17.0f;
		float RatingSize = 22.0f;
		float DifficultySize = 14.0f;
		float TimerSize = 28.0f;
		float UISize = 16.0f;
		float RowSpacing = 7.0f;

		bool BoldLabels = false;
		bool BoldValues = true;
		bool BoldRating = true;
		bool BoldTimer = false;

		bool HighlightBadStats = true;

		ImVec4 LabelColor = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
		ImVec4 ValueColor = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
		ImVec4 HighlightColor = ImVec4(1.00f, 0.65f, 0.15f, 1.0f);
		ImVec4 TimerColor = ImVec4(0.30f, 1.00f, 0.35f, 1.0f);
		ImVec4 RatingColor = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
		ImVec4 RatingBadColor = ImVec4(1.00f, 0.35f, 0.35f, 1.0f);
		ImVec4 DifficultyColor = ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
		ImVec4 ChromeBg = ImVec4(0.05f, 0.06f, 0.08f, 0.78f);
		ImVec4 ChromeBorder = ImVec4(0.30f, 0.50f, 0.90f, 0.85f);
		ImVec4 ChromeText = ImVec4(0.72f, 0.82f, 1.00f, 1.0f);
	};

	struct OverlayFonts
	{
		ImFont* Regular = nullptr;
		ImFont* Bold = nullptr;
		ImFont* Timer = nullptr;
	};

	static OverlaySettings s_Settings;
	static OverlayPositions s_Positions;
	static OverlayFonts s_Fonts;
	static std::vector<std::string> s_FontFiles;

	static bool s_FontsDirty = true;
	static bool s_ResetPositions = false;
	static bool s_ShowEditor = false;
	static bool s_ShowSettings = false;
	static bool s_ShowGameOptions = false;
	static ImGuiID s_DragWindow = 0;

	std::filesystem::path GetSettingsPath()
	{
		wchar_t s_AppData[MAX_PATH]{};
		const DWORD s_Length = GetEnvironmentVariableW(L"APPDATA", s_AppData, MAX_PATH);

		if (s_Length > 0 && s_Length < MAX_PATH)
			return std::filesystem::path(s_AppData) / L"Statman" / L"HM3Overlay.json";

		return std::filesystem::path(L"HM3Overlay.json");
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

	void LoadSettings()
	{
		try
		{
			std::ifstream s_File(GetSettingsPath());

			if (!s_File)
				return;

			nlohmann::json s_Json;
			s_File >> s_Json;

			if (!s_Json.is_object())
				return;

			if (JsonHas(s_Json, "show") && s_Json["show"].is_object())
			{
				const auto& s_Show = s_Json["show"];
				s_Settings.ShowLeftStats = s_Show.value("leftStats", s_Settings.ShowLeftStats);
				s_Settings.ShowRightStats = s_Show.value("rightStats", s_Settings.ShowRightStats);
				s_Settings.ShowRating = s_Show.value("rating", s_Settings.ShowRating);
				s_Settings.ShowTimer = s_Show.value("timer", s_Settings.ShowTimer);
				s_Settings.ShowLevelInfo = s_Show.value("levelInfo", s_Settings.ShowLevelInfo);
				s_Settings.StatsOnlyInGame = s_Show.value("statsOnlyInGame", s_Settings.StatsOnlyInGame);
				s_Settings.HighlightBadStats = s_Show.value("highlightBadStats", s_Settings.HighlightBadStats);
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
				s_Settings.BoldTimer = s_FontJson.value("boldTimer", s_Settings.BoldTimer);
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
			}

			if (JsonHas(s_Json, "colors") && s_Json["colors"].is_object())
			{
				const auto& s_ColorJson = s_Json["colors"];
				ColorFromJson(s_ColorJson, "label", s_Settings.LabelColor);
				ColorFromJson(s_ColorJson, "value", s_Settings.ValueColor);
				ColorFromJson(s_ColorJson, "highlight", s_Settings.HighlightColor);
				ColorFromJson(s_ColorJson, "timer", s_Settings.TimerColor);
				ColorFromJson(s_ColorJson, "rating", s_Settings.RatingColor);
				ColorFromJson(s_ColorJson, "ratingBad", s_Settings.RatingBadColor);
				ColorFromJson(s_ColorJson, "difficulty", s_Settings.DifficultyColor);
				ColorFromJson(s_ColorJson, "chromeBg", s_Settings.ChromeBg);
				ColorFromJson(s_ColorJson, "chromeBorder", s_Settings.ChromeBorder);
				ColorFromJson(s_ColorJson, "chromeText", s_Settings.ChromeText);
			}

			if (JsonHas(s_Json, "positions") && s_Json["positions"].is_object())
			{
				const auto& s_PositionJson = s_Json["positions"];
				s_Positions.LeftValid = Vec2FromJson(s_PositionJson, "left", s_Positions.Left);
				s_Positions.RightValid = Vec2FromJson(s_PositionJson, "right", s_Positions.Right);
				s_Positions.RatingValid = Vec2FromJson(s_PositionJson, "rating", s_Positions.Rating);
				s_Positions.TimerValid = Vec2FromJson(s_PositionJson, "timer", s_Positions.Timer);
				s_Positions.LevelValid = Vec2FromJson(s_PositionJson, "level", s_Positions.Level);
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
			s_Json["version"] = 1;

			{
				auto& s_Show = s_Json["show"];
				s_Show["leftStats"] = s_Settings.ShowLeftStats;
				s_Show["rightStats"] = s_Settings.ShowRightStats;
				s_Show["rating"] = s_Settings.ShowRating;
				s_Show["timer"] = s_Settings.ShowTimer;
				s_Show["levelInfo"] = s_Settings.ShowLevelInfo;
				s_Show["statsOnlyInGame"] = s_Settings.StatsOnlyInGame;
				s_Show["highlightBadStats"] = s_Settings.HighlightBadStats;
			}

			{
				auto& s_FontJson = s_Json["fonts"];
				s_FontJson["regular"] = s_Settings.RegularFont;
				s_FontJson["bold"] = s_Settings.BoldFont;
				s_FontJson["timer"] = s_Settings.TimerFont;
				s_FontJson["boldLabels"] = s_Settings.BoldLabels;
				s_FontJson["boldValues"] = s_Settings.BoldValues;
				s_FontJson["boldRating"] = s_Settings.BoldRating;
				s_FontJson["boldTimer"] = s_Settings.BoldTimer;
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
			}

			{
				auto& s_ColorJson = s_Json["colors"];
				s_ColorJson["label"] = ColorToJson(s_Settings.LabelColor);
				s_ColorJson["value"] = ColorToJson(s_Settings.ValueColor);
				s_ColorJson["highlight"] = ColorToJson(s_Settings.HighlightColor);
				s_ColorJson["timer"] = ColorToJson(s_Settings.TimerColor);
				s_ColorJson["rating"] = ColorToJson(s_Settings.RatingColor);
				s_ColorJson["ratingBad"] = ColorToJson(s_Settings.RatingBadColor);
				s_ColorJson["difficulty"] = ColorToJson(s_Settings.DifficultyColor);
				s_ColorJson["chromeBg"] = ColorToJson(s_Settings.ChromeBg);
				s_ColorJson["chromeBorder"] = ColorToJson(s_Settings.ChromeBorder);
				s_ColorJson["chromeText"] = ColorToJson(s_Settings.ChromeText);
			}

			{
				auto& s_PositionJson = s_Json["positions"];

				if (s_Positions.LeftValid)
					s_PositionJson["left"] = Vec2ToJson(s_Positions.Left);

				if (s_Positions.RightValid)
					s_PositionJson["right"] = Vec2ToJson(s_Positions.Right);

				if (s_Positions.RatingValid)
					s_PositionJson["rating"] = Vec2ToJson(s_Positions.Rating);

				if (s_Positions.TimerValid)
					s_PositionJson["timer"] = Vec2ToJson(s_Positions.Timer);

				if (s_Positions.LevelValid)
					s_PositionJson["level"] = Vec2ToJson(s_Positions.Level);
			}

			std::ofstream s_File(s_Path);

			if (s_File)
				s_File << s_Json.dump(2);
		}
		catch (...)
		{
		}
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
		s_Fonts.Timer = AddFont(s_Settings.TimerFont.empty() ? s_Settings.RegularFont : s_Settings.TimerFont, 16.0f);

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

	void BeginHudWindow(
		const char* p_Id,
		const char* p_Title,
		const ImVec2& p_DefaultPosition,
		const ImVec2& p_DefaultPivot,
		float p_Width,
		ImVec2& p_SavedPosition,
		bool& p_PositionValid,
		bool p_Capture)
	{
		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const ImVec2 s_WorkMin = s_Viewport->WorkPos;
		const ImVec2 s_WorkMax(
			s_Viewport->WorkPos.x + s_Viewport->WorkSize.x,
			s_Viewport->WorkPos.y + s_Viewport->WorkSize.y);

		if (s_ResetPositions || !p_PositionValid)
		{
			ImGui::SetNextWindowPos(
				p_DefaultPosition,
				s_ResetPositions ? ImGuiCond_Always : ImGuiCond_FirstUseEver,
				p_DefaultPivot);
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

		if (!p_Capture)
			s_Flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, p_Capture ? ImVec2(10.0f, 8.0f) : ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, p_Capture ? 5.0f : 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, p_Capture ? 1.0f : 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		if (p_Capture)
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, s_Settings.ChromeBg);
			ImGui::PushStyleColor(ImGuiCol_Border, s_Settings.ChromeBorder);
		}

		ImGui::Begin(p_Id, nullptr, s_Flags);

		if (p_Capture)
		{
			ImGuiWindow* s_Window = ImGui::GetCurrentWindow();
			const bool s_Hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

			if (s_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				s_DragWindow = s_Window->ID;

			if (s_DragWindow == s_Window->ID && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				ImVec2 s_Position = s_Window->Pos;
				s_Position.x += ImGui::GetIO().MouseDelta.x;
				s_Position.y += ImGui::GetIO().MouseDelta.y;
				ImGui::SetWindowPos(s_Position);
			}

			if (s_DragWindow == s_Window->ID && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				s_DragWindow = 0;

			if (s_Hovered)
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

			ImGui::PushFont(s_Fonts.Regular, s_Settings.UISize);
			ImGui::TextColored(s_Settings.ChromeText, "%s", p_Title);
			ImGui::PopFont();

			ImGui::Separator();
		}

		p_SavedPosition = ImGui::GetWindowPos();
		p_PositionValid = true;
	}

	void EndHudWindow(bool p_Capture)
	{
		if (p_Capture)
			ImGui::PopStyleColor(2);

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

		return s_Stats;
	}

	void DrawStatEntry(const char* p_Label, const std::string& p_Value, bool p_Bad)
	{
		ImFont* s_LabelFont = s_Settings.BoldLabels ? s_Fonts.Bold : s_Fonts.Regular;
		ImFont* s_ValueFont = s_Settings.BoldValues ? s_Fonts.Bold : s_Fonts.Regular;
		const ImVec4& s_ValueColor = (p_Bad && s_Settings.HighlightBadStats)
			? s_Settings.HighlightColor
			: s_Settings.ValueColor;

		DrawHudText(s_LabelFont, s_Settings.LabelSize, p_Label, s_Settings.LabelColor, true);
		DrawHudText(s_ValueFont, s_Settings.ValueSize, p_Value.c_str(), s_ValueColor, true);
		ImGui::Dummy(ImVec2(0.0f, s_Settings.RowSpacing));
	}

	void DrawLeftStats(const HudStats& p_Stats)
	{
		DrawStatEntry("Witnesses", std::to_string(p_Stats.Witnesses), p_Stats.Witnesses > 0);
		DrawStatEntry("Bodies Found", std::to_string(p_Stats.BodiesFound), p_Stats.BodiesFound > 0);
		DrawStatEntry("Covers Blown", std::to_string(p_Stats.CoversBlown), p_Stats.CoversBlown > 0);
		DrawStatEntry("Caught on Camera", p_Stats.CameraCaught ? "Yes" : "No", p_Stats.CameraCaught);
		DrawStatEntry("Frisk Failed", p_Stats.FriskFailed ? "Yes" : "No", p_Stats.FriskFailed);
		DrawStatEntry("Items Left on Level", GetItemsLeftText(p_Stats.ItemsLeft), p_Stats.ItemsLeft != ItemState::None);
		DrawStatEntry("Accidents", std::to_string(p_Stats.Accidents), false);
	}

	void DrawRightStats(const HudStats& p_Stats)
	{
		DrawStatEntry("Enemies Killed", std::to_string(p_Stats.EnemiesKilled), p_Stats.EnemiesKilled > 0);
		DrawStatEntry("Enemies Wounded", std::to_string(p_Stats.EnemiesWounded), p_Stats.EnemiesWounded > 0);
		DrawStatEntry("Policemen Killed", std::to_string(p_Stats.PolicemenKilled), p_Stats.PolicemenKilled > 0);
		DrawStatEntry("Policemen Wounded", std::to_string(p_Stats.PolicemenWounded), p_Stats.PolicemenWounded > 0);
		DrawStatEntry("Innocents Killed", std::to_string(p_Stats.InnocentsKilled), p_Stats.InnocentsKilled > 0);
		DrawStatEntry("Innocents Wounded", std::to_string(p_Stats.InnocentsWounded), p_Stats.InnocentsWounded > 0);
		DrawStatEntry("Shots Hit", std::to_string(p_Stats.ShotsHit), false);
	}

	void DrawRating(const ZHM3GameData* p_GameData)
	{
		bool s_PerfectRating = false;
		const char* s_RatingText = "No Rating";

		if (g_Module->Pointers()->ZHM3LevelControl__m_stats && p_GameData)
		{
			const auto [s_Perfect, s_Rating] = GetRating();
			s_PerfectRating = s_Perfect;
			s_RatingText = s_Rating;
		}

		const ImVec4& s_RatingColor = s_PerfectRating ? s_Settings.RatingColor : s_Settings.RatingBadColor;
		ImFont* s_RatingFont = s_Settings.BoldRating ? s_Fonts.Bold : s_Fonts.Regular;

		DrawHudText(s_RatingFont, s_Settings.RatingSize, s_RatingText, s_RatingColor, false);

		if (g_Module->CheatsEnabled() || g_Module->UnlimitedSaves())
		{
			ImGui::SameLine(0.0f, 0.0f);
			DrawHudText(s_RatingFont, s_Settings.RatingSize, " (!)", s_Settings.HighlightColor, false);
		}

		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		const char* s_Difficulty = p_GameData ? GetDifficultyName(p_GameData->m_nDifficulty) : "Unknown";
		DrawHudText(s_Fonts.Regular, s_Settings.DifficultySize, s_Difficulty, s_Settings.DifficultyColor, false);
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

		std::string s_DebugText;
		SMatrix33 s_PlayerMat{};
		SVector3 s_PlayerPos{};
		bool s_HasPlayerTransform = false;
		int s_ClosestActor = -1;
		float s_ClosestActorDistanceSq = FLT_MAX;

		if (s_Hitman->m_pBaseGeom && s_Hitman->m_pBaseGeom->m_pParent)
		{
			s_PlayerMat = s_Hitman->m_pBaseGeom->m_pParent->m_mMat;
			s_PlayerPos = s_Hitman->m_pBaseGeom->m_pParent->m_vPos;
			s_HasPlayerTransform = true;

			s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_PlayerMat.xAxis.x, s_PlayerMat.xAxis.y, s_PlayerMat.xAxis.z);
			s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_PlayerMat.yAxis.x, s_PlayerMat.yAxis.y, s_PlayerMat.yAxis.z);
			s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_PlayerMat.zAxis.x, s_PlayerMat.zAxis.y, s_PlayerMat.zAxis.z);
			s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_PlayerPos.x, s_PlayerPos.y, s_PlayerPos.z);
		}

		if (s_SysInterface && *s_SysInterface && (*s_SysInterface)->m_pRenderer)
		{
			const auto s_MainCamera = (*s_SysInterface)->m_pRenderer->GetCamera(0);

			if (s_MainCamera)
			{
				const auto s_CameraPtr = reinterpret_cast<ZCAMERA*>(g_Module->Functions()->ZGEOM_RefToPtr(s_MainCamera));

				if (s_CameraPtr && s_CameraPtr->m_pBaseGeom)
				{
					const auto& s_Mat = s_CameraPtr->m_pBaseGeom->m_mMat;
					const auto& s_CameraPos = s_CameraPtr->m_pBaseGeom->m_vPos;

					s_DebugText += "\n";
					s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_Mat.xAxis.x, s_Mat.xAxis.y, s_Mat.xAxis.z);
					s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_Mat.yAxis.x, s_Mat.yAxis.y, s_Mat.yAxis.z);
					s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_Mat.zAxis.x, s_Mat.zAxis.y, s_Mat.zAxis.z);
					s_DebugText += std::format("({:.2f}, {:.2f}, {:.2f}) ", s_CameraPos.x, s_CameraPos.y, s_CameraPos.z);

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
					ImFont* s_LabelFont = s_Fonts.Regular ? s_Fonts.Regular : ImGui::GetFont();
					const float s_LabelSize = (std::max)(12.0f, s_Settings.UISize * 0.85f * GetUiScale());

					for (uint32_t i = 0; i < s_GameData->m_nActorCount; ++i)
					{
						const auto s_Actor = s_GameData->m_aActors[i];

						// Only render NPCs we can dress as.
						if (!s_Actor || !s_Actor->GetHitmanAs() || !g_Module->Functions()->ZGEOM_RefToPtr(s_Actor->GetHitmanAs()))
							continue;

						if (!s_Actor->m_pBaseGeom || !s_Actor->m_pBaseGeom->m_pParent)
							continue;

						const auto& s_Pos = s_Actor->m_pBaseGeom->m_pParent->m_vPos;

						if (s_HasPlayerTransform)
						{
							const auto s_Delta = XMVectorSet(
								s_Pos.x - s_PlayerPos.x,
								s_Pos.y - s_PlayerPos.y,
								s_Pos.z - s_PlayerPos.z,
								0.0f);

							const float s_DistanceSq = XMVectorGetX(XMVector3LengthSq(s_Delta));

							if (s_DistanceSq < s_ClosestActorDistanceSq)
							{
								s_ClosestActorDistanceSq = s_DistanceSq;
								s_ClosestActor = static_cast<int>(i);
							}
						}

						if (const auto s_ScreenCoord = WorldToScreen(s_CameraData, s_Pos))
						{
							const std::string s_ActorId = std::to_string(i);

							s_DrawList->AddText(
								s_LabelFont,
								s_LabelSize,
								ImVec2(s_ScreenCoord->x + 1.0f, s_ScreenCoord->y + 1.0f),
								IM_COL32(0, 0, 0, 200),
								s_ActorId.c_str());

							s_DrawList->AddText(
								s_LabelFont,
								s_LabelSize,
								ImVec2(s_ScreenCoord->x, s_ScreenCoord->y),
								IM_COL32(120, 255, 150, 230),
								s_ActorId.c_str());
						}
					}
				}
			}
		}

		if (!s_CaptureInput)
			return;

		const float s_Scale = GetUiScale();

		ImGui::SetNextWindowPos(ImVec2(24.0f * s_Scale, 64.0f * s_Scale), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(380.0f * s_Scale, 0.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("2016 Debug##debug2016", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PushFont(s_Fonts.Regular ? s_Fonts.Regular : ImGui::GetFont(), s_Settings.UISize * s_Scale);

			if (s_DebugText.empty())
				ImGui::TextDisabled("No data.");
			else
				ImGui::TextUnformatted(s_DebugText.c_str());

			ImGui::Separator();
			ImGui::Text("Scene: %s", g_Module->m_SceneName.empty() ? "unknown" : g_Module->m_SceneName.c_str());

			if (s_HasPlayerTransform)
			{
				static char s_LocationName[64] = "";
				ImGui::InputText("Location name", s_LocationName, sizeof(s_LocationName));

				if (s_ClosestActor >= 0)
					ImGui::Text("Closest outfit: %d (%.1f m)", s_ClosestActor, std::sqrt(s_ClosestActorDistanceSq));
				else
					ImGui::TextDisabled("No outfit candidate nearby.");

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
			else
			{
				ImGui::TextDisabled("Player position unavailable.");
			}

			if (ImGui::Button("Reload locations") && g_Module->Pipe())
				g_Module->Pipe()->SendPipeMessage("RL", "");

			ImGui::PopFont();
		}

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
		ImGui::PushStyleColor(ImGuiCol_WindowBg, s_Settings.ChromeBg);
		ImGui::PushStyleColor(ImGuiCol_Border, s_Settings.ChromeBorder);

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

			if (ImGui::Button(s_ShowEditor ? "Hide Editor" : "Overlay Editor"))
				s_ShowEditor = !s_ShowEditor;

			ImGui::SameLine();

			if (ImGui::Button(s_ShowGameOptions ? "Hide Game" : "Game"))
				s_ShowGameOptions = !s_ShowGameOptions;

			ImGui::SameLine();

			if (ImGui::Button(s_ShowSettings ? "Hide Settings" : "Settings"))
				s_ShowSettings = !s_ShowSettings;

			ImGui::SameLine();
			ImGui::TextDisabled("INSERT releases cursor");

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

		ImGui::Checkbox("Left stats", &s_Settings.ShowLeftStats);
		ImGui::SameLine();
		ImGui::Checkbox("Right stats", &s_Settings.ShowRightStats);
		ImGui::Checkbox("Rating", &s_Settings.ShowRating);
		ImGui::SameLine();
		ImGui::Checkbox("Timer", &s_Settings.ShowTimer);
		ImGui::SameLine();
		ImGui::Checkbox("Level info", &s_Settings.ShowLevelInfo);
		ImGui::Checkbox("Show stats only in game", &s_Settings.StatsOnlyInGame);

		ImGui::Separator();

		if (ImGui::Button("Settings"))
			s_ShowSettings = !s_ShowSettings;

		ImGui::SameLine();

		if (ImGui::Button("Reset positions"))
		{
			s_ResetPositions = true;
			s_Positions.LeftValid = false;
			s_Positions.RightValid = false;
			s_Positions.RatingValid = false;
			s_Positions.TimerValid = false;
			s_Positions.LevelValid = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Save settings"))
			SaveSettings();

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

		bool s_UnlimitedSaves = g_Module->UnlimitedSaves();

		if (ImGui::Checkbox("Unlimited saves", &s_UnlimitedSaves))
			SetUnlimitedSavesFromOverlay(s_UnlimitedSaves);

		bool s_Hitman2016 = g_Module->Hitman2016Mode();

		if (ImGui::Checkbox("Hitman 2016 mode", &s_Hitman2016))
			SetHitman2016ModeFromOverlay(s_Hitman2016);

		bool s_Debug2016 = g_Module->Debug2016();

		if (ImGui::Checkbox("2016 debug info", &s_Debug2016))
			SetDebug2016FromOverlay(s_Debug2016);

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
			if (ImGui::BeginTabItem("Fonts"))
			{
				static char s_RegularFilter[64] = "";
				static char s_BoldFilter[64] = "";
				static char s_TimerFilter[64] = "";

				if (FontCombo("Regular", s_Settings.RegularFont, s_RegularFilter, sizeof(s_RegularFilter)))
					s_FontsDirty = true;

				if (FontCombo("Bold", s_Settings.BoldFont, s_BoldFilter, sizeof(s_BoldFilter)))
					s_FontsDirty = true;

				if (FontCombo("Timer", s_Settings.TimerFont, s_TimerFilter, sizeof(s_TimerFilter)))
					s_FontsDirty = true;

				ImGui::Checkbox("Bold labels", &s_Settings.BoldLabels);
				ImGui::SameLine();
				ImGui::Checkbox("Bold values", &s_Settings.BoldValues);
				ImGui::Checkbox("Bold rating", &s_Settings.BoldRating);
				ImGui::SameLine();
				ImGui::Checkbox("Bold timer", &s_Settings.BoldTimer);

				ImGui::TextDisabled("Fonts are loaded from the Windows Fonts folder.");

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Sizes"))
			{
				ImGui::SliderFloat("Label size", &s_Settings.LabelSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Value size", &s_Settings.ValueSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Rating size", &s_Settings.RatingSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Difficulty size", &s_Settings.DifficultySize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Timer size", &s_Settings.TimerSize, 8.0f, 64.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("UI size", &s_Settings.UISize, 8.0f, 32.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);
				ImGui::SliderFloat("Row spacing", &s_Settings.RowSpacing, 0.0f, 32.0f, "%.0f px", ImGuiSliderFlags_AlwaysClamp);

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
				ImGui::ColorEdit4("Difficulty", &s_Settings.DifficultyColor.x, s_ColorFlags);

				ImGui::SeparatorText("Behavior");
				ImGui::Checkbox("Highlight bad stats", &s_Settings.HighlightBadStats);

				ImGui::SeparatorText("Chrome");
				ImGui::ColorEdit4("Background", &s_Settings.ChromeBg.x, s_ColorFlags);
				ImGui::ColorEdit4("Border", &s_Settings.ChromeBorder.x, s_ColorFlags);
				ImGui::ColorEdit4("Text", &s_Settings.ChromeText.x, s_ColorFlags);

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

		ImGui::PopFont();
		ImGui::End();
	}

	void DrawActiveWarnings()
	{
		std::string s_Warning;

		if (g_Module->CheatsEnabled())
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
			s_Viewport->WorkPos.x + 16.0f * s_Scale,
			s_Viewport->WorkPos.y + s_Viewport->WorkSize.y - s_TextSize.y - 16.0f * s_Scale);

		s_DrawList->AddRectFilled(
			ImVec2(s_Position.x - 6.0f, s_Position.y - 3.0f),
			ImVec2(s_Position.x + s_TextSize.x + 6.0f, s_Position.y + s_TextSize.y + 3.0f),
			IM_COL32(0, 0, 0, 110),
			4.0f);

		s_DrawList->AddText(s_Font, s_Size, s_Position, IM_COL32(255, 190, 90, 210), s_Warning.c_str());
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
		const bool s_MainMenu = s_SceneInfo && s_SceneInfo->MainMenu;
		const bool s_ShowStats = !s_Settings.StatsOnlyInGame || !s_MainMenu;

		double s_CurrentTimeSeconds = 0.0;

		if (s_Stats && (s_Stats->m_Time > 0 || !s_InGame))
			s_CurrentTimeSeconds = static_cast<double>(s_Stats->m_Time) / 1000.0;
		else if (auto** s_SysInterface = g_Module->Pointers()->g_pSysInterface; s_SysInterface && *s_SysInterface)
			s_CurrentTimeSeconds = static_cast<double>((*s_SysInterface)->m_Ticks) * 0.0009765625;

		s_CurrentTimeSeconds = (std::max)(0.0, s_CurrentTimeSeconds);

		const int s_Hours = static_cast<int>(s_CurrentTimeSeconds / 3600);
		const int s_Minutes = static_cast<int>((s_CurrentTimeSeconds - (s_Hours * 3600)) / 60);
		const double s_Seconds = s_CurrentTimeSeconds - (s_Hours * 3600) - (s_Minutes * 60);
		const int s_WholeSeconds = static_cast<int>(s_Seconds);
		const int s_Milliseconds = static_cast<int>((s_Seconds - s_WholeSeconds) * 1000);

		const std::string s_TimeText = std::format("{:02d}:{:02d}:{:02d}.{:03d}", s_Hours, s_Minutes, s_WholeSeconds, s_Milliseconds);

		const HudStats s_HudStats = GetHudStats(s_Stats, s_GameData);

		ImGuiViewport* s_Viewport = ImGui::GetMainViewport();
		const ImVec2 s_Center = s_Viewport->GetCenter();

		if (s_Settings.ShowLeftStats && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_left",
				"STATISTICS",
				ImVec2(s_Viewport->WorkPos.x + 40.0f, s_Center.y),
				ImVec2(0.0f, 0.5f),
				200.0f,
				s_Positions.Left,
				s_Positions.LeftValid,
				s_Capture);

			if (s_Stats)
				DrawLeftStats(s_HudStats);
			else
				DrawHudText(s_Fonts.Regular, s_Settings.LabelSize, "No data.", s_Settings.LabelColor, true);

			EndHudWindow(s_Capture);
		}

		if (s_Settings.ShowRightStats && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_right",
				"STATISTICS",
				ImVec2(s_Viewport->WorkPos.x + s_Viewport->WorkSize.x - 40.0f, s_Center.y),
				ImVec2(1.0f, 0.5f),
				200.0f,
				s_Positions.Right,
				s_Positions.RightValid,
				s_Capture);

			if (s_Stats)
				DrawRightStats(s_HudStats);
			else
				DrawHudText(s_Fonts.Regular, s_Settings.LabelSize, "No data.", s_Settings.LabelColor, true);

			EndHudWindow(s_Capture);
		}

		if (s_Settings.ShowRating && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_rating",
				"RATING",
				ImVec2(s_Center.x, 40.0f),
				ImVec2(0.5f, 0.0f),
				0.0f,
				s_Positions.Rating,
				s_Positions.RatingValid,
				s_Capture);

			DrawRating(s_GameData);

			EndHudWindow(s_Capture);
		}

		if (s_Settings.ShowTimer && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_timer",
				"TIMER",
				ImVec2(s_Center.x, s_Viewport->WorkPos.y + s_Viewport->WorkSize.y - 70.0f),
				ImVec2(0.5f, 0.5f),
				0.0f,
				s_Positions.Timer,
				s_Positions.TimerValid,
				s_Capture);

			DrawHudText(
				s_Settings.BoldTimer ? s_Fonts.Bold : s_Fonts.Timer,
				s_Settings.TimerSize,
				s_TimeText.c_str(),
				s_Settings.TimerColor,
				false);

			EndHudWindow(s_Capture);
		}

		if (s_Settings.ShowLevelInfo && s_ShowStats)
		{
			BeginHudWindow(
				"##hud_level",
				"LEVEL",
				ImVec2(s_Center.x, 120.0f),
				ImVec2(0.5f, 0.0f),
				0.0f,
				s_Positions.Level,
				s_Positions.LevelValid,
				s_Capture);

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
			DrawHudText(s_Fonts.Regular, s_Settings.UISize, s_LevelText.c_str(), s_Settings.LabelColor, false);

			if (ImGui::IsItemHovered() && !g_Module->m_SceneName.empty())
				ImGui::SetTooltip("Scene: %s", g_Module->m_SceneName.c_str());

			EndHudWindow(s_Capture);
		}

		s_ResetPositions = false;

		DrawDebug2016();

		if (s_Capture)
		{
			DrawOverlayToolbar();

			if (s_ShowEditor)
				DrawEditor();

			if (s_ShowSettings)
				DrawSettings();

			if (s_ShowGameOptions)
				DrawGameOptions();
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
