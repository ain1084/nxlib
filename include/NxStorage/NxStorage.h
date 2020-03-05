//
// NxStorage.h
// Copyright(c) 2000 S.Ainoguchi
//

#pragma once
#pragma warning (disable : 4786)

#undef STRICT
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#include <malloc.h>
#include <crtdbg.h>

#include <NxShared/NxShared.h>

#if !defined(NXSTORAGE_BUILD)
#if defined(_DEBUG)
// Debug
#if defined(_MT)
#if defined(_DLL)
#pragma comment (lib, "nxstoragedd.lib")	// Debug Dynamic
#else
#pragma comment (lib, "nxstorageds.lib")	// Debug Static
#endif	// #if defined(_DLL)
#else
#pragma comment (lib, "nxstoragedt.lib")	// Debug Single
#endif	// #if defined(_MT)
#else
// Release
#if defined(_MT)
#if defined(_DLL)
#pragma comment (lib, "nxstoragerd.lib")	// Release Dynamic
#else
#pragma comment (lib, "nxstoragers.lib")	// Release Static
#endif	// #if defined(_DLL)
#else
#pragma comment (lib, "nxstoragert.lib")	// Release Single
#endif	// #if defined(_MT)
#endif	// #if defined(_DEBUG)
#endif	// #if !defined(NXSTORAGE_BUILD)