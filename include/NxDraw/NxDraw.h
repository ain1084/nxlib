//
// NxDraw.h
// Copyright(c) 2000,2001 S.Ainoguchi
// version 2001/05/22

#pragma once
#pragma warning (disable : 4786)

//#define NXDRAW_LOADIMAGE_NO_JPEG      // CNxImageLoader で JPEG 形式の読み込みを行わない
//#define NXDRAW_LOADIMAGE_NO_PNG       //                   PNG 形式の読み込みを行わない
//#define NXDRAW_LOADIMAGE_NO_MAG		//					 MAG
//#define NXDRAW_LOADIMAGE_NO_SUSIE_SPI //                   susie 32bit plug-in を使用しない

#undef STRICT
#define STRICT
#include <windows.h>
#include <tchar.h>
#include <crtdbg.h>

/////////////////////////////////////////////
// class CNxDraw declaration

#include <string>
#include <memory>

class CNxSurface;
class CNxDIBImage;
class CNxFile;
namespace NxDrawLocal { class CNxImageLoader; }

class CNxDraw
{
private:
	CNxDraw();
	~CNxDraw();

public:
	static CNxDraw* GetInstance();
	static void DestroyInstance();

	BOOL IsMMXEnabled() const;
	BOOL EnableMMX(BOOL bEnable);
	BOOL IsMMXSupported() const;
	BOOL Is3DNowEnabled() const;
	BOOL Enable3DNow(BOOL bEnable);
	BOOL Is3DNowSupported() const;

	CNxDIBImage* LoadImage(CNxFile& nxfile) const;

#if !defined(NXDRAW_NO_SUPPORT_SPI)
	int GetSPIDirectory(LPTSTR lpBuf, int nCount) const;
	void GetSPIDirectory(std::basic_string<TCHAR>& rString) const;
	void SetSPIDirectory(LPCTSTR lpszDirectory);
#endif	// #if !defined(NXDRAW_NO_SUPPORT_SPI)

	HWND GetFrameWnd() const;
	HWND SetFrameWnd(HWND hWndFrame);
	//
	// 以下内部用。使用しないで下さい
	//
	CNxSurface* GetTextTemporarySurface(UINT uWidth, UINT uHeight);
	void CompactTextTemporarySurface();

private:
	static CNxDraw* m_pInstance;
	
	std::auto_ptr<CNxSurface> m_pTextTemporarySurface;
	std::auto_ptr<NxDrawLocal::CNxImageLoader> m_pImageLoader;
#if !defined(NXDRAW_MMX_ONLY)
	BOOL m_bEnableMMX;					// MMX 命令を使用するならば TRUE
#endif	// #if !defined(NXDRAW_MMX_ONLY)
	BOOL m_bSupportMMX;					// MMX 命令が使用可能であれば TRUE
	BOOL m_bEnable3DNow;					// 3DNow! 命令を使用するならば TRUE
	BOOL m_bSupport3DNow;				// 3DNow! 命令が使用可能であれば TRUE
#if !defined(NXDRAW_NO_SUPPORT_SPI)
	std::basic_string<TCHAR> m_strSPIDirectory;
#endif	// #if !defined(NXDRAW_NO_SUPPORT_SPI)
	HWND m_hWndFrame;
};

inline BOOL CNxDraw::IsMMXEnabled() const {
#if defined(NXDRAW_MMX_ONLY)
	return TRUE; }		// 常に TRUE を返す
#else
	return m_bEnableMMX; }
#endif // #if defined(NXDRAW_MMX_ONLY)

#if defined(NXDRAW_MMX_ONLY)
inline BOOL CNxDraw::EnableMMX(BOOL /*bEnable*/) {
		return TRUE; }		// 常に有効
#endif	// #if defined(NXDRAW_MMX_ONLY)
	
inline BOOL CNxDraw::IsMMXSupported() const {
	return m_bSupportMMX; }

inline BOOL CNxDraw::Is3DNowEnabled() const {
	return m_bEnable3DNow; }

inline BOOL CNxDraw::Is3DNowSupported() const {
	return m_bSupport3DNow; }

inline HWND CNxDraw::GetFrameWnd() const {
	return m_hWndFrame; }

/////////////////// end of class CNxDraw

#include <NxShared/NxShared.h>

#if !defined(NXDRAW_BUILD)
#if defined(_DEBUG)
// Debug
#if defined(_DLL)
#pragma comment (lib, "nxdrawdd.lib")	// Debug Dynamic
#else
#pragma comment (lib, "nxdrawds.lib")	// Debug Static
#endif	// #if defined(_DLL)
#else
// Release
#if defined(_DLL)
#pragma comment (lib, "nxdrawrd.lib")	// Release Dynamic
#else
#pragma comment (lib, "nxdrawrs.lib")	// Release Static
#endif	// #if defined(_DLL)
#endif	// #if defined(_DEBUG)
#endif	// #if !defined(NXDRAW_BUILD)
