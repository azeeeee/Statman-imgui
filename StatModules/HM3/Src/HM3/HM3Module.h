#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Structs/ZCore.h"
#include "Structs/ZHM3Actor.h"

class Pipeman;

class HM3Hooks;
class HM3Pointers;
class HM3Functions;

struct LevelStartingLocation
{
	std::string Name;
	int Outfit;
	SMatrix33 Rotation;
	SVector3 Position;
};

struct LiveWeaponKill
{
	int32_t ItemId;  // item ID (for example 4 = Custom_SniperRifle)
	int32_t Count;   // kills made with this weapon in the current level
};

struct CheatState
{
	bool EnableCheats = false;
	bool InfAmmo = false;
	bool InfClip = false;
	bool GodMode = false;
	bool InvisibleMode = false;
	bool ShowEnemyVision = false;
	bool ShowOSD = true;
	float TimeMultiplier = 1.0f;
};

class HM3Module
{
public:
	HM3Module();
	~HM3Module();

	bool Init();

public:
	Pipeman* Pipe() const { return m_Pipeman; }
	HM3Hooks* Hooks() const { return m_Hooks; }
	HM3Pointers* Pointers() const { return m_Pointers; }
	HM3Functions* Functions() const { return m_Functions; }
	uintptr_t GetModuleBase() const { return m_ModuleBase; }
	uint32_t GetSizeOfCode() const { return m_SizeOfCode; }

public:
	bool CheatsEnabled() const { return m_CheatsEnabled; }
	bool UnlimitedSaves() const { return m_UnlimitedSaves; }
	bool CheatsActive() const;
	CheatState GetCheatState() const;
	void SetCheatState(const CheatState& p_State);

	/// Called from the game's own time-scale setter (ZSysInterface). The dying
	/// slow motion animates the same field Statman exposes as the time
	/// multiplier, so the last game-written value is remembered to keep that
	/// animation from being reported as a cheat.
	void SetGameTimeMultiplier(float p_Value);
	bool Hitman2016Mode() const { return m_Hitman2016Mode; }
	bool OverlayEnabled() const { return m_OverlayEnabled; }
	bool Debug2016() const { return m_Debug2016; }
	const std::unordered_map<std::string, std::vector<LevelStartingLocation>>& GetLevelStartingLocations() const { return m_LevelStartingLocations; }
	bool ApplyStartingLocation(const LevelStartingLocation& p_Location);
	bool ApplyOutfit(int p_ActorIndex);
	bool GetPlayerTransform(SMatrix33& p_Rotation, SVector3& p_Position) const;
	int GetCurrentOutfitIndex() const;
	int GetTotalLevelNpcs() const;

	/// Last value computed by GetTotalLevelNpcs(). Updated once per second from
	/// the game thread so the render thread can read it for free.
	int GetCachedTotalLevelNpcs() const { return m_CachedTotalLevelNpcs.load(std::memory_order_relaxed); }

	/// Number of those NPCs that are still alive. Same refresh cadence.
	int GetCachedAliveLevelNpcs() const { return m_CachedAliveLevelNpcs.load(std::memory_order_relaxed); }

	/// Clears the cached NPC counts (called on scene load).
	void ResetLevelNpcCache()
	{
		m_CachedTotalLevelNpcs.store(0, std::memory_order_relaxed);
		m_CachedAliveLevelNpcs.store(0, std::memory_order_relaxed);
	}
	std::string GetCurrentLevelKey() const;
	bool AddOrUpdateLocation(const std::string& p_Level, const LevelStartingLocation& p_Location);
	bool DeleteLocation(const std::string& p_Level, const std::string& p_Name);
	bool SetLastLocation(const std::string& p_Level, const std::string& p_Name);
	std::string GetLastLocation(const std::string& p_Level) const;
	bool SaveHitman2016Data();
	bool ReloadHitman2016Data();

	void SetCheatsEnabled(bool p_Enabled);
	void SetUnlimitedSaves(bool p_Enabled);
	void SetHitman2016Mode(bool p_Enabled);
	void SetDebug2016(bool p_Enabled);

protected:
	bool CheckInstance();
	void PerformPatches();
	void OnMessage(const std::string& p_Type, const std::string& p_Content);
	void ParseHitman2016Data(const std::string& p_Json);

protected:
	Pipeman* m_Pipeman = nullptr;
	HM3Hooks* m_Hooks = nullptr;
	HM3Pointers* m_Pointers = nullptr;
	HM3Functions* m_Functions = nullptr;
	uintptr_t m_ModuleBase = 0;
	uint32_t m_SizeOfCode = 0;

protected:
	bool m_CheatsEnabled = false;
	bool m_UnlimitedSaves = false;
	bool m_Hitman2016Mode = false;
	bool m_OverlayEnabled = false;
	bool m_Debug2016 = false;

	// Last value the game itself wrote through ZSysInterface::SetTimeMultiplier.
	// Starts at 1.0 so an external trainer that changes the multiplier before
	// the game ever touches it is still detected.
	std::atomic<float> m_GameTimeMultiplier{ 1.0f };

	// Cached total NPC count for the current level, refreshed once per second.
	mutable std::atomic<int> m_CachedTotalLevelNpcs{ 0 };
	mutable std::atomic<int> m_CachedAliveLevelNpcs{ 0 };

	std::unordered_map<std::string, std::vector<LevelStartingLocation>> m_LevelStartingLocations;
	std::unordered_map<std::string, std::string> m_LevelLastLocations;

public:
	std::string m_SceneName = "";
	bool m_SceneLoaded = false;

	// Snapshot of the game's live per-weapon kill counters. It is written from
	// the level-control frame update (game thread) and read by the overlay and
	// the C# UI, so access is guarded by a mutex.
	static constexpr int c_MaxLiveWeaponKills = 512;
	mutable std::mutex m_LiveWeaponKillsMutex;
	LiveWeaponKill m_LiveWeaponKills[c_MaxLiveWeaponKills];
	int m_LiveWeaponKillCount = 0;

	// Replaces the current snapshot. p_Kills may be null when p_Count is zero.
	void SetLiveWeaponKills(const LiveWeaponKill* p_Kills, int p_Count)
	{
		std::lock_guard<std::mutex> s_Lock(m_LiveWeaponKillsMutex);

		if (p_Count < 0)
			p_Count = 0;

		if (p_Count > c_MaxLiveWeaponKills)
			p_Count = c_MaxLiveWeaponKills;

		for (int i = 0; i < p_Count; ++i)
			m_LiveWeaponKills[i] = p_Kills[i];

		m_LiveWeaponKillCount = p_Count;
	}

	// Copies up to p_Max entries into p_Out and returns the number of entries copied.
	int GetLiveWeaponKills(LiveWeaponKill* p_Out, int p_Max) const
	{
		std::lock_guard<std::mutex> s_Lock(m_LiveWeaponKillsMutex);

		int s_Count = m_LiveWeaponKillCount;

		if (s_Count > p_Max)
			s_Count = p_Max;

		for (int i = 0; i < s_Count; ++i)
			p_Out[i] = m_LiveWeaponKills[i];

		return s_Count;
	}

	// Set by the ZScene_Load detour. The Direct3D hooks are only installed after the
	// first scene load, i.e. once the game's renderer is up and running.
	std::atomic<bool> m_FirstSceneLoaded = false;
};