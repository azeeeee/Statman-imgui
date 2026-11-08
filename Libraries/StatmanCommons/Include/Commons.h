#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdint.h>
#include <stdio.h>

#ifndef CONCAT_IMPL
#define CONCAT_IMPL(x, y) x##y
#endif

#ifndef MACRO_CONCAT
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#endif

#ifndef PAD
#define PAD(SIZE) unsigned char MACRO_CONCAT(_pad, __COUNTER__)[SIZE];
#endif

#ifndef ALIGN
#define ALIGN(n) __declspec(align(n)) 
#endif

#ifndef ALIGNMENT
#ifdef _WIN64
#	define ALIGNMENT 8
#else
#	ifdef _WIN32
#		define ALIGNMENT 4
#	else
#		define ALIGNMENT 1
#	endif
#endif
#endif

// Appends to hm3.log next to Statman.exe/HM3.dll in both Debug and Release builds.
// Release logging is required to diagnose end-user reports. Per-symbol and per-hook
// detail is only written when verbose logging is enabled, either through the
// STATMAN_VERBOSE=1 environment variable or at runtime via StatmanSetLogVerbose().
void StatmanLog(const char* p_Format, ...);
bool StatmanLogVerboseEnabled();
void StatmanSetLogVerbose(bool p_Enabled);

#ifdef _DEBUG
#define Log(...) (StatmanLog(__VA_ARGS__), printf(__VA_ARGS__))
#else
#define Log(...) StatmanLog(__VA_ARGS__)
#endif

#define LogVerbose(...) do { if (StatmanLogVerboseEnabled()) StatmanLog(__VA_ARGS__); } while (0)