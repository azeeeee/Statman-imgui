#include "Commons.h"

#include <cstdarg>
#include <cstring>

static const ULONGLONG g_MaxLogBytes = 1024 * 1024;

static SRWLOCK g_LogLock = SRWLOCK_INIT;
static char g_LogPath[MAX_PATH] = { 0 };

static const char* GetLogPath()
{
	// The path is only ever read after being written by a single thread, so a racing
	// second writer can at worst recompute the same string.
	if (g_LogPath[0] != '\0')
		return g_LogPath;

	// HM3.dll is loaded from the same folder as Statman.exe, so keep hm3.log there
	// as well. This keeps both diagnostic logs together with the application.
	HMODULE s_Module = nullptr;

	if (!GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCSTR>(&StatmanLog),
		&s_Module))
	{
		return nullptr;
	}

	char s_ModulePath[MAX_PATH] = { 0 };

	if (GetModuleFileNameA(s_Module, s_ModulePath, MAX_PATH) == 0)
		return nullptr;

	char* s_LastSlash = strrchr(s_ModulePath, '\\');

	if (!s_LastSlash)
		return nullptr;

	*s_LastSlash = '\0';

	char s_Path[MAX_PATH] = { 0 };
	sprintf_s(s_Path, sizeof(s_Path), "%s\\hm3.log", s_ModulePath);

	// Publish only fully formed paths.
	AcquireSRWLockExclusive(&g_LogLock);
	if (g_LogPath[0] == '\0')
		strcpy_s(g_LogPath, sizeof(g_LogPath), s_Path);
	ReleaseSRWLockExclusive(&g_LogLock);

	return g_LogPath;
}

// Runtime switch set through the Statman IPC ("VL" message). -1 keeps the
// environment-variable behavior, 0/1 force verbose logging off/on.
static volatile LONG g_RuntimeVerbose = -1;

void StatmanSetLogVerbose(bool p_Enabled)
{
	InterlockedExchange(&g_RuntimeVerbose, p_Enabled ? 1 : 0);
}

bool StatmanLogVerboseEnabled()
{
	const LONG s_RuntimeState = InterlockedCompareExchange(&g_RuntimeVerbose, -1, -1);

	if (s_RuntimeState >= 0)
		return s_RuntimeState == 1;

	// -1 unknown, 0 disabled, 1 enabled. A racing first read can only repeat the
	// same environment lookup, so no lock is required.
	static int s_State = -1;

	if (s_State < 0)
	{
		char s_Buffer[8] = { 0 };

		s_State = (GetEnvironmentVariableA("STATMAN_VERBOSE", s_Buffer, sizeof(s_Buffer)) > 0 && s_Buffer[0] == '1')
			? 1
			: 0;
	}

	return s_State == 1;
}

static void RotateLogIfNeeded(const char* p_Path)
{
	WIN32_FILE_ATTRIBUTE_DATA s_Info;

	if (!GetFileAttributesExA(p_Path, GetFileExInfoStandard, &s_Info))
		return;

	const ULONGLONG s_Size = (static_cast<ULONGLONG>(s_Info.nFileSizeHigh) << 32) | s_Info.nFileSizeLow;

	if (s_Size < g_MaxLogBytes)
		return;

	char s_Backup[MAX_PATH] = { 0 };
	sprintf_s(s_Backup, sizeof(s_Backup), "%s.old", p_Path);

	MoveFileExA(p_Path, s_Backup, MOVEFILE_REPLACE_EXISTING);
}

void StatmanLog(const char* p_Format, ...)
{
	const char* s_Path = GetLogPath();

	if (s_Path == nullptr)
		return;

	char s_Message[2048] = { 0 };

	va_list s_Arguments;
	va_start(s_Arguments, p_Format);
	_vsnprintf_s(s_Message, sizeof(s_Message), _TRUNCATE, p_Format, s_Arguments);
	va_end(s_Arguments);

	SYSTEMTIME s_Time;
	GetLocalTime(&s_Time);

	AcquireSRWLockExclusive(&g_LogLock);

	RotateLogIfNeeded(s_Path);

	FILE* s_File = nullptr;
	if (fopen_s(&s_File, s_Path, "a") == 0 && s_File != nullptr)
	{
		fprintf(s_File, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] [pid %5lu] [tid %5lu] %s",
			s_Time.wYear, s_Time.wMonth, s_Time.wDay,
			s_Time.wHour, s_Time.wMinute, s_Time.wSecond, s_Time.wMilliseconds,
			GetCurrentProcessId(), GetCurrentThreadId(), s_Message);

		fclose(s_File);
	}

	ReleaseSRWLockExclusive(&g_LogLock);
}
