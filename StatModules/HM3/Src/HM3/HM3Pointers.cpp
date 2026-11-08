#include "HM3Pointers.h"
#include "HM3Module.h"

#include <Utils.h>
#include <Pointers.h>

HM3Pointers::HM3Pointers()
{
	Setup();
}

HM3Pointers::~HM3Pointers()
{

}

bool HM3Pointers::Setup()
{
	FIND_POINTER_ABSOLUTE(
		HM3Stats*,
		ZHM3LevelControl__m_stats,
		"\xBF\x00\x00\x00\x00\xF3\xAB\xB8\x00\x00\x00\x00\x5F",
		"x????xxx????x",
		1
	)

	FIND_POINTER_ABSOLUTE(
		ZHM3GameData**,
		g_pGameData,
		"\xA1\x00\x00\x00\x00\x85\xC0\x56\x74",
		"x????xxxx",
		1
	)

	FIND_POINTER_ABSOLUTE(
		bool*,
		CConfiguration__m_bCheatsEnabled,
		"\x88\x1D\x00\x00\x00\x00\x8B\x8C\x24\x88\x00\x00\x00",
		"xx????xxxxxxx",
		2
	)

	FIND_POINTER_ABSOLUTE_OPTIONAL(
		int32_t*,
		g_bGodMode,
		"\xA1\x00\x00\x00\x00\x85\xC0\x0F\x95\xC0\xC3",
		"x????xxxxxx",
		1
	)

	FIND_POINTER_ABSOLUTE_OPTIONAL(
		int32_t*,
		g_bInvisibleMode,
		"\xA1\x00\x00\x00\x00\x85\xC0\x74\x0C\x66\x83\x7D\x14\x08",
		"x????xxxxxxxxx",
		1
	)

	FIND_POINTER_ABSOLUTE_OPTIONAL(
		int32_t*,
		g_bShowEnemyVision,
		"\xA1\x00\x00\x00\x00\x85\xC0\x74\x05\xC6\x44\x24\x28\x00",
		"x????xxxxxxxxx",
		1
	)

	FIND_POINTER_ABSOLUTE_OPTIONAL(
		int32_t*,
		g_bShowOSD,
		"\xA1\x00\x00\x00\x00\x85\xC0\x75\x0B\x5F\x5E\xB0\x01\x5D\x83\xC4\x3C\xC2\x08\x00",
		"x????xxxxxxxxxxxxxx",
		1
	)

	FIND_POINTER_ABSOLUTE_OPTIONAL(
		int32_t*,
		g_bInfClip,
		"\xA1\x00\x00\x00\x00\x85\xC0\xB8\xE7\x03\x00\x00\x75\x06",
		"x????xxxxxxxxx",
		1
	)

	FIND_POINTER_ABSOLUTE(
		void**,
		g_pActionManager,
		"\xA1\x00\x00\x00\x00\x85\xC0\x56\x57\x74\x00\xA1\x00\x00\x00\x00\x8B\x30\x8D\x88\xBC\x04\x00\x00",
		"x????xxxxx?x????xxxxxxxx",
		1
	)

	FIND_POINTER_ABSOLUTE(
		ZSysInterface**,
		g_pSysInterface,
		"\xA1\x00\x00\x00\x00\x85\xC0\x56\x57\x74\x00\xA1\x00\x00\x00\x00\x8B\x30\x8D\x88\xBC\x04\x00\x00",
		"x????xxxxx?x????xxxxxxxx",
		12
	)

	FIND_POINTER_ABSOLUTE(
		void**,
		g_pSysInput,
		"\x8B\x0D\x00\x00\x00\x00\x8B\x11\x0F\xBF\xC0\x50\xFF\x52\x10\x0F\xB6\x56\x10\x83\xC2\xFD\x83\xFA\x0F",
		"xx????xxxxxxxxxxxxxxxxxxx",
		2
	)

	FIND_POINTER_ABSOLUTE(
		ZGEOM**,
		ZLIST__m_TrackLinkObjectsInstance,
		"\x39\x1D\x00\x00\x00\x00\x74\x00\x00\xC7\x05",
		"xx????x??xx",
		2
	)

	return true;
}
