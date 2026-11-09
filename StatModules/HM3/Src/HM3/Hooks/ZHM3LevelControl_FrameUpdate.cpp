#include <algorithm>
#include <HM3/HM3Hooks.h>

#include <HM3/HM3Module.h>
#include <HM3/HM3Functions.h>
#include <HM3/HM3Pointers.h>

#include <HM3/Structs/ZHM3Actor.h>
#include <HM3/Structs/HM3Stats.h>
#include <HM3/Structs/ZHM3LevelControl.h>
#include <HM3/Structs/UnknownClass02.h>
#include <HM3/Structs/ZHM3GameData.h>
#include <HM3/Structs/ZHM3Actor.h>
#include <HM3/Structs/ZRender.h>

#include <directxmath.h>
#include <Pipeman.h>

namespace
{
	// The level control keeps a live "weapon usage" container inline at +0x68.
	// It tracks how many kills were made with each weapon template (ZREF).
	// This is the same container the game's own weapon-kill getter iterates at
	// 0x006ADF10; the special-rating code uses it before the mission ends.
	constexpr uintptr_t c_LiveWeaponContainerOffset = 0x68;

	struct LiveWeaponContainer
	{
		void* m_VTable;               // 0x0000 - container vtable
		uint32_t* m_PresenceBits;     // 0x0004 - one bit per used slot
		int32_t m_SlotCount;          // 0x0008 - number of slots (typically 128)
		int32_t m_Capacity;           // 0x000C - allocated capacity
		int32_t m_UsedCount;          // 0x0010 - number of occupied slots
		int32_t m_Unknown01;          // 0x0014
		void* m_Entries;              // 0x0018 - array of LiveWeaponEntry
	};

	struct LiveWeaponEntry
	{
		ZREF m_Ref;                   // 0x0000 - reference to the weapon template
		int32_t m_Count;              // 0x0004 - kills made with this weapon
	};

	// The native CheatsActive() also knows which time-scale changes came from
	// the game itself (the dying slow motion animates the multiplier down to
	// 0.2). The C# UI reads the cheat flags on its own, so mirror the
	// authoritative value and let it override the rating warning.
	void UpdateCheatNotification()
	{
		static bool s_Last = false;
		static bool s_Sent = false;

		const bool s_Active = g_Module->CheatsActive();

		if (s_Sent && s_Active == s_Last)
			return;

		s_Sent = true;
		s_Last = s_Active;

		if (g_Module->Pipe())
			g_Module->Pipe()->SendPipeMessage("CA", s_Active ? "true" : "false");
	}

	// The C# UI needs the level's total NPC count for its special-rating
	// prediction. Send it from here so the C# side does not have to walk
	// hardcoded addresses that only match one game build.
	void UpdateLevelNpcNotification()
	{
		static int s_Last = -1;
		static DWORD s_LastTick = 0;

		const DWORD s_Now = GetTickCount();

		if (s_LastTick != 0 && s_Now - s_LastTick < 1000)
			return;

		s_LastTick = s_Now;

		const int s_Count = g_Module->GetTotalLevelNpcs();

		if (s_Count == s_Last)
			return;

		s_Last = s_Count;

		LogVerbose("Level NPCs: %d alive / %d total.\n",
			g_Module->GetCachedAliveLevelNpcs(), s_Count);

		if (g_Module->Pipe())
			g_Module->Pipe()->SendPipeMessage("NP", std::to_string(s_Count));
	}

	void UpdateLiveWeaponKills(ZHM3LevelControl* th)
	{
		// The container does not change every frame; reading it a few times per
		// second is enough for the overlay and avoids calling into the game
		// hundreds of times per second.
		static int s_FrameCounter = 0;

		if ((s_FrameCounter++ % 15) != 0)
			return;

		LiveWeaponKill s_Kills[HM3Module::c_MaxLiveWeaponKills];
		int s_Count = 0;

		auto* s_Container = reinterpret_cast<LiveWeaponContainer*>(
			reinterpret_cast<char*>(th) + c_LiveWeaponContainerOffset);

		if (!IsBadReadPtr(s_Container, sizeof(LiveWeaponContainer)) &&
			s_Container->m_PresenceBits &&
			s_Container->m_Entries &&
			s_Container->m_SlotCount > 0 &&
			s_Container->m_SlotCount <= HM3Module::c_MaxLiveWeaponKills &&
			!IsBadReadPtr(s_Container->m_PresenceBits, (s_Container->m_SlotCount + 7) / 8) &&
			!IsBadReadPtr(s_Container->m_Entries, s_Container->m_SlotCount * sizeof(LiveWeaponEntry)))
		{
			auto* s_Entries = reinterpret_cast<LiveWeaponEntry*>(s_Container->m_Entries);

			for (int i = 0; i < s_Container->m_SlotCount; ++i)
			{
				if ((s_Container->m_PresenceBits[i >> 5] & (1u << (i & 31))) == 0)
					continue;

				const LiveWeaponEntry& s_Entry = s_Entries[i];

				if (!s_Entry.m_Ref || s_Entry.m_Count <= 0)
					continue;

				auto* s_Object = g_Module->Functions()->ZGEOM_RefToPtr(s_Entry.m_Ref);

				if (!s_Object || IsBadReadPtr(s_Object, sizeof(void*)))
					continue;

				auto** s_VTable = *reinterpret_cast<void***>(s_Object);

				if (!s_VTable || IsBadReadPtr(&s_VTable[0x300 / 4], sizeof(void*)))
					continue;

				// Virtual getter used by the game itself to turn the weapon
				// template object into its item/weapon ID.
				using GetItemIdFn = int(__thiscall*)(void*);
				const auto s_GetItemId = reinterpret_cast<GetItemIdFn>(s_VTable[0x300 / 4]);

				if (!s_GetItemId)
					continue;

				const int s_ItemId = s_GetItemId(s_Object);

				if (s_ItemId <= 0)
					continue;

				// Merge duplicate item IDs just in case the container ever
				// tracks the same weapon through multiple slots.
				bool s_Merged = false;

				for (int j = 0; j < s_Count; ++j)
				{
					if (s_Kills[j].ItemId == s_ItemId)
					{
						s_Kills[j].Count += s_Entry.m_Count;
						s_Merged = true;
						break;
					}
				}

				if (!s_Merged)
				{
					s_Kills[s_Count].ItemId = s_ItemId;
					s_Kills[s_Count].Count = s_Entry.m_Count;
					++s_Count;
				}
			}
		}

		g_Module->SetLiveWeaponKills(s_Kills, s_Count);

		// Mirror the live weapon data to the Statman UI, which cannot resolve
		// the weapon template references on its own. The message uses the compact
		// "itemId:count,itemId:count" format and is sent a few times per second.
		static DWORD s_LastPipeMessage = 0;
		const DWORD s_Now = GetTickCount();

		if (s_Now - s_LastPipeMessage >= 500)
		{
			std::string s_Data;

			for (int i = 0; i < s_Count; ++i)
			{
				if (!s_Data.empty())
					s_Data += ",";

				s_Data += std::to_string(s_Kills[i].ItemId);
				s_Data += ":";
				s_Data += std::to_string(s_Kills[i].Count);
			}

			if (g_Module->Pipe())
				g_Module->Pipe()->SendPipeMessage("LW", s_Data);

			s_LastPipeMessage = s_Now;
		}
	}
}

DECLARE_THISCALL_DETOUR(HM3Hooks, void, ZHM3LevelControl_FrameUpdate, ZHM3LevelControl* th)
{
	if (!g_Module || !g_Module->Pointers() || !g_Module->Functions())
		return o_ZHM3LevelControl_FrameUpdate(th);

	// VTable: 0x00755ADC
	auto s_Class02 = reinterpret_cast<UnknownClass02*>(reinterpret_cast<char*>(th) + sizeof(ZHM3LevelControl));

	// Calculate Stats
	int s_Witnesses = 0;

	DetectionIterator s_DetectionIt;
	s_Class02->InitDetectionIterator(&s_DetectionIt);

	for (ZREF s_NPCID = s_Class02->NextDetectionNPC(&s_DetectionIt); s_DetectionIt.m_Unk01 > 0; s_NPCID = s_Class02->NextDetectionNPC(&s_DetectionIt))
	{
		auto s_NPC = reinterpret_cast<ZHM3Actor*>(g_Module->Functions()->ZGEOM_RefToPtr(s_NPCID));

		if (!s_NPC)
			continue;

		if (!s_NPC->IsDead())
			++s_Witnesses;
	}

	int s_WeaponsInHand = 0;

	auto s_GameData = *g_Module->Pointers()->g_pGameData;

	if (s_GameData && th->m_pWeaponStorage && s_GameData->m_pLevelLinking)
	{
		g_Module->Functions()->ZHitmanWeaponStorage_EmptyStorage(th->m_pWeaponStorage, s_GameData->m_pLevelLinking);
		s_WeaponsInHand += g_Module->Functions()->ZHitmanWeaponStorage_NrOfCustomWeaponsInStorage(th->m_pWeaponStorage);
	}

	if (s_GameData && th->m_pWeaponStorage2 && s_GameData->m_pLevelLinking)
	{
		g_Module->Functions()->ZHitmanWeaponStorage_EmptyStorage(th->m_pWeaponStorage2, s_GameData->m_pLevelLinking);
		s_WeaponsInHand += g_Module->Functions()->ZHitmanWeaponStorage_NrOfCustomWeaponsInStorage(th->m_pWeaponStorage2);
	}

	// Update stats.
	if (g_Module->Pointers()->ZHM3LevelControl__m_stats)
	{
		g_Module->Pointers()->ZHM3LevelControl__m_stats->m_Witnesses = s_Witnesses;
		g_Module->Pointers()->ZHM3LevelControl__m_stats->m_CustomWeaponsLeftOnLevel = th->m_WeaponsLeftOnLevel - s_WeaponsInHand - g_Module->Functions()->ZHM3LevelControl_GetCustomWeaponsInventoryCount();

		if (s_GameData && s_GameData->m_pHitman)
			g_Module->Pointers()->ZHM3LevelControl__m_stats->m_SuitLeftOnLevel = s_GameData->m_pHitman->m_Unknown01 != s_GameData->m_pHitman->m_Unknown02;

		if (g_Module && g_Module->Hitman2016Mode())
			g_Module->Pointers()->ZHM3LevelControl__m_stats->m_SuitLeftOnLevel = false;
	}

	// Consume the scene-load event even when Hitman 2016 mode is disabled. This
	// prevents a stale event from teleporting the player the moment the mode is
	// enabled; a level load/restart is required instead.
	if (g_Module->m_SceneLoaded)
	{
		g_Module->m_SceneLoaded = false;

		if (g_Module->Hitman2016Mode())
		{
			for (const auto& [s_Level, s_Locations] : g_Module->GetLevelStartingLocations())
			{
				auto s_LevelLower = s_Level;
				std::transform(s_LevelLower.begin(), s_LevelLower.end(), s_LevelLower.begin(), ::tolower);

				// Check if s_Level is a substring of the loaded scene and if we have any locations for that level.
				if (s_Locations.empty() || g_Module->m_SceneName.find(s_LevelLower) == std::string::npos)
					continue;

				// Restore the last location the user loaded for this level. If there
				// is none, fall back to a random saved location.
				const std::string s_LastLocation = g_Module->GetLastLocation(s_Level);
				const LevelStartingLocation* s_Location = nullptr;

				for (const auto& s_Candidate : s_Locations)
				{
					if (!s_LastLocation.empty() && s_Candidate.Name == s_LastLocation)
					{
						s_Location = &s_Candidate;
						break;
					}
				}

				if (!s_Location)
					s_Location = &s_Locations[rand() % s_Locations.size()];

				g_Module->ApplyStartingLocation(*s_Location);

				break;
			}
		}
	}

	o_ZHM3LevelControl_FrameUpdate(th);

	UpdateCheatNotification();
	UpdateLevelNpcNotification();
	UpdateLiveWeaponKills(th);
}