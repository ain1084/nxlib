#pragma once
#pragma warning (disable : 4786)

#if !defined(NXSHARED_BUILD)
#if defined(_DEBUG)
// Debug
#if defined(_DLL)
#pragma comment (lib, "nxshareddd.lib")	// Debug Dynamic
#else
#pragma comment (lib, "nxsharedds.lib")	// Debug Static
#endif	// #if defined(_DLL)
#else
// Release
#if defined(_DLL)
#pragma comment (lib, "nxsharedrd.lib")	// Release Dynamic
#else
#pragma comment (lib, "nxsharedrs.lib")	// Release Static
#endif	// #if defined(_DLL)
#endif	// #if defined(_DEUBG)
#endif	// #if !defined(NXSHARED_BUILD)
