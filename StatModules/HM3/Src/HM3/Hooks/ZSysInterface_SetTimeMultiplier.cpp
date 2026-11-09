#include <HM3/HM3Hooks.h>

#include <HM3/HM3Module.h>

DECLARE_THISCALL_DETOUR(HM3Hooks, float, ZSysInterface_SetTimeMultiplier, void* th, float p_Value)
{
	const float s_Result = o_ZSysInterface_SetTimeMultiplier(th, p_Value);

	// The game drives the same time-scale field Statman exposes as the time
	// multiplier, and the dying/last-stand slow motion animates it down to 0.2
	// and back up. Remember the value the game actually ended up with so
	// CheatsActive() can tell that animation apart from a trainer value. The
	// setter skips the write while the game has locked the time scale, so read
	// the field instead of trusting the argument.
	constexpr uintptr_t c_TimeMultiplierOffset = 0xB28;

	if (g_Module && th)
	{
		const float s_Current = *reinterpret_cast<float*>(
			reinterpret_cast<char*>(th) + c_TimeMultiplierOffset);

		g_Module->SetGameTimeMultiplier(s_Current);
	}

	return s_Result;
}
