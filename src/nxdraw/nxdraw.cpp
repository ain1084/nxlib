// NxDrawGlobal.cpp: namespace NxDraw のインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: NxDraw 全体に関るグローバルな定数、変数
//       クラス宣言は NxDraw.h
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <algorithm>
#include <sstream>
#include "NxSurface.h"
#include "NxDrawLocal.h"		// namespace NxDrawLocal
#include "NxImageLoader.h"		// namespace NxDrawLocal

using namespace NxDrawLocal;

CNxDraw* CNxDraw::m_pInstance = NULL;

namespace
{

// CNxDraw クラスの排他アクセス用 CriticalSection
CRITICAL_SECTION g_criticalSection;

inline void Lock() {
	::EnterCriticalSection(&g_criticalSection); }

inline void Unlock() {
	::LeaveCriticalSection(&g_criticalSection); }

/////////////////////////////////////////////////////////////////////////////////////////
// static BOOL GetProcessorSupport3DNow()
// 概要: 3DNow! 命令が使用可能か否かを調べる
// 引数: なし
// 戻値: 使用可能ならば TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL __declspec(naked) GetProcessorSupport3DNow()
{
	__asm
	{
		push	ebx
		pushfd
		pop		eax
		mov		edx, eax
		xor		eax, 0200000h
		push	eax
		popfd
		pushfd
		pop		eax
		xor		eax, edx
		jz		doesnot_support_CPUID

		mov		eax, 080000000h
		cpuid
		or		eax, eax
		jz		doesnot_support_FeatureFlag

		mov		eax, 080000001h
		cpuid
		test	edx, 080000000h
		jz		doesnot_support_3DNow
		mov		eax, TRUE
		jmp		check3DNow_exit
doesnot_support_CPUID:
doesnot_support_FeatureFlag:
doesnot_support_3DNow:
		mov		eax, FALSE
check3DNow_exit:
		pop		ebx
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// static BOOL GetProcessorSupportMMX()
// 概要: MMX 命令が使用可能か否かを調べる
// 引数: なし
// 戻値: 使用可能ならば TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL __declspec(naked) GetProcessorSupportMMX()
{
	__asm
	{
		push	ebx
		pushfd
		pop		eax
		mov		edx, eax
		xor		eax, 0200000h
		push	eax
		popfd
		pushfd
		pop		eax
		xor		eax, edx
		jz		doesnot_support_CPUID

		mov		eax, 0
		cpuid
		or		eax, eax
		jz		doesnot_support_FeatureFlag

		mov		eax, 1
		cpuid
		test	edx, 00800000h
		jz		doesnot_support_MMX

		mov		eax, TRUE
		jmp		checkMMX_exit
doesnot_support_CPUID:
doesnot_support_FeatureFlag:
doesnot_support_MMX:
		mov		eax, FALSE
checkMMX_exit:
		pop		ebx
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// static int StrCountCopy(LPTSTR lpDest, LPCTSTR lpszSrc, int nCount)
// 概要: lpDest へ最大 nCount 文字を lpszSrc からコピーする
// 引数: LPTSTR  lpDest  ... 文字列を受け取るバッファへのポインタ
//       LPCTSTR lpszSrc ... ヌル文字で終わるコピー元文字列へのポインタ
//       int nCount      ... lpDest の文字単位のサイズ
// 戻値: コピーされた文字数(ヌル文字を除く)
//       nCount <= lpszSrc 文字数ならば、lpDest の最後にヌル文字は付かない
/////////////////////////////////////////////////////////////////////////////////////////

static int StrCountCopy(LPTSTR lpDest, LPCTSTR lpszSrc, int nCount)
{
	if (nCount <= 0)
		return 0;
	
	int nLength;
	int nSrcLength = _tcslen(lpszSrc);
	if (nCount <= nSrcLength)
		nLength = nCount;
	else
		nLength = nSrcLength + 1;

	memcpy(lpDest, lpszSrc, nLength * sizeof(TCHAR));
	return nLength;
}

// CNxDraw オブジェクトを自動的に生成/破棄するクラス
class CNxDrawAutoDestroyer
{
public:
	CNxDrawAutoDestroyer()
	{
		::InitializeCriticalSection(&g_criticalSection);
		CreateTableDynamic();
	}
	~CNxDrawAutoDestroyer()
	{
		CNxDraw::DestroyInstance();
		::DeleteCriticalSection(&g_criticalSection);
	}
} gNxDrawAutoDestroyer;

}	// namespace {


/////////////////////////////////////////////////////////////////////////////////////////
// private:
//	CNxDraw::CNxDraw()
// 概要: CNxDraw クラスのコンストラクタ
// 引数: ---
// 戻値: ---
/////////////////////////////////////////////////////////////////////////////////////////

CNxDraw::CNxDraw()
 : m_pImageLoader(new CNxImageLoader)
{
	// 起動ディレクトリを取得して susie32 plug-in 検索のデフォルトとする
#if !defined(NXDRAW_NO_SUSIE_SPI)
	TCHAR szModuleFileName[_MAX_FNAME];
	TCHAR szBuf[_MAX_FNAME];
	::GetModuleFileName(NULL, szModuleFileName, _MAX_FNAME);
	_tsplitpath_s(szModuleFileName, szBuf, _MAX_FNAME, NULL, 0, NULL, 0, NULL, 0);
	m_strSPIDirectory = szBuf;
	_tsplitpath_s(szModuleFileName, NULL, 0, szBuf, _MAX_FNAME, NULL, 0, NULL, 0);
	m_strSPIDirectory += szBuf;
	TCHAR chLast = m_strSPIDirectory[m_strSPIDirectory.length() - 1];
	if (chLast != '\\' && chLast != '/')
		m_strSPIDirectory += '\\';
#endif	// #if !defined(NXDRAW_NO_SUSIE_SPI)

	// MMX / 3DNow! を CPU がサポートしているか調べる
	m_bSupportMMX = GetProcessorSupportMMX();
	if (m_bSupportMMX)	// これは念のため。MMX が使用不可能で 3DNow! を使用できる CPU は無い
	{
		m_bSupport3DNow = GetProcessorSupport3DNow();
	}
	else
	{
		m_bSupport3DNow = FALSE;
	}
	// デフォルトでは MMX も 3DNow! も使用する
	m_bEnable3DNow = m_bSupport3DNow;
#if !defined(NXDRAW_MMX_ONLY)
	m_bEnableMMX = m_bSupportMMX;
#endif	// #if !defined(NXDRAW_MMX_ONLY)
}

/////////////////////////////////////////////////////////////////////////////////////////
// private:
//	CNxDraw::~CNxDraw()
// 概要: CNxDraw クラスのデストラクタ
// 引数: ---
// 戻値: ---
/////////////////////////////////////////////////////////////////////////////////////////

CNxDraw::~CNxDraw()
{
	Lock();

	// テキスト描画用サーフェスの開放
	CompactTextTemporarySurface();

	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDraw* CNxDraw::GetInstance()
// 概要: CNxDraw のインスタンスを取得
// 引数: なし
// 戻値: CNxDraw へのポインタ
/////////////////////////////////////////////////////////////////////////////////////////

CNxDraw* CNxDraw::GetInstance()
{
	Lock();
	if (m_pInstance == NULL)
	{
		m_pInstance = new CNxDraw;
	}
	Unlock();
	return m_pInstance;
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDraw::DestroyInstance()
// 概要: CNxDraw のインスタンスを破棄
// 引数: なし
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::DestroyInstance()
{
	Lock();
	delete m_pInstance;
	m_pInstance = NULL;
	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL NxDraw::EnableMMX(BOOL bEnable)
// 概要: MMX 命令の使用/不使用を指定する
// 引数: BOOL bEnable ... MMX 命令を使用するならば TRUE
// 戻値: 以前の状態
// 備考: 強制的に MMX を使用する訳ではない。bEnable を TRUE にしても、
//       使用不可能ならば使われることはない。
/////////////////////////////////////////////////////////////////////////////////////////

#if !defined(NXDRAW_MMX_ONLY)
BOOL CNxDraw::EnableMMX(BOOL bEnable)
{
	Lock();
	bEnable &= m_bSupportMMX;
	std::swap(bEnable, m_bEnableMMX);
	Unlock();
	return bEnable;
}
#endif	// #if !defined(NXDRAW_MMX_ONLY)

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDraw::Enable3DNow(BOOL bEnable)
// 概要: 3DNow! 命令の使用/不使用を指定する
// 引数: BOOL bEnable ... 3DNow! 命令を使用するならば TRUE
// 戻値: 以前の状態
// 備考: 強制的に 3DNow を使用する訳ではない。bEnable を TRUE にしても、
//       使用不可能ならば使われることはない。
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDraw::Enable3DNow(BOOL bEnable)
{
	Lock();
	bEnable &= m_bSupport3DNow;
	std::swap(bEnable, m_bEnable3DNow);
	Unlock();
	return bEnable;
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDIBImage* CNxDraw::LoadImage(CNxFile& nxfile) const
// 概要: CNxDIBImageLoader 派生クラスを使用して画像を読み込む
// 引数: CNxFile& nxfile ... 読み込み元ファイルへの参照
// 戻値: CNxDIBImage へのポインタ。失敗ならば NULL
// 備考: 以下の CNxDIBImageLoader 派生クラスが使用される
//       CNxBMPImageLoader
//       CNxPNGImageLoader (NXDRAW_IMAGELOADER_NO_PNG が定義されていない場合)
//       CNxJPEGImageLoader (NXDRAW_IMAGELOADER_NO_JPEG が定義されていない場合)
//		 CNxSPIImageLoader (NXDRAW_IMAGELOADER_NO_SUSIE_SPI が定義されていない場合)
/////////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxDraw::LoadImage(CNxFile& nxfile) const
{
	return m_pImageLoader->CreateDIBImage(nxfile);
}

#if !defined(NXDRAW_LOADIMAGE_NO_SUSIE_SPI)

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxDraw::GetSPIDirectory(LPTSTR lpBuf, int nCount)
// 概要: susie plug-in を検索するディレクトリを取得
// 引数: LPTSTR lpBuf  ... ディレクトリを受けとるバッファ
//		 int    nCount ... バッファの長さ
// 戻値: バッファへコピーされた文字数(ヌル文字除く)
/////////////////////////////////////////////////////////////////////////////////////////

int CNxDraw::GetSPIDirectory(LPTSTR lpBuf, int nCount) const
{
	Lock();
	int nResult = StrCountCopy(lpBuf, m_strSPIDirectory.c_str(), nCount);
	Unlock();
	return nResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxDraw::GetSPIDirectory(std::basic_string<TCHAR>& rString)
// 概要: susie plug-in を検索するディレクトリを取得
// 引数: std::basic_string<TCHAR>& rString ... ディレクトリを受け取る std::string オブジェクトへの参照
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::GetSPIDirectory(std::basic_string<TCHAR>& rString) const
{
	Lock();
	rString = m_strSPIDirectory;
	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDraw::SetSPIDirectory(LPCTSTR lpszDirectory)
// 概要: susie plug-in を検索するディレクトリを設定
// 引数: LPCTSTR lpszDirectory ... 設定するディレクトリ(ドライブ名を含む絶対パス)
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::SetSPIDirectory(LPCTSTR lpszDirectory)
{
	Lock();
	m_strSPIDirectory = lpszDirectory;
	TCHAR chLast = m_strSPIDirectory[m_strSPIDirectory.length() - 1];
	if (chLast != '\\' && chLast != '/')
		m_strSPIDirectory += '\\';

	// CNxImageLoader で現在読み込まれている SPI を開放するために再生成
	m_pImageLoader.reset(new CNxImageLoader);
	Unlock();
}
#endif	// #if !defined(NXDRAW_NO_SUSIE_SPI)

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDraw::CompactTextTemporaySurface()
// 概要: テキスト描画用一時サーフェスを小さく(開放)する
// 引数: なし
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::CompactTextTemporarySurface()
{
	Lock();
	m_pTextTemporarySurface.reset();
	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	 HWND CNxDraw::SetFrameWnd(HWND hWndFrame)
// 概要: メッセージボックスの表示等に使用するオーナーのウィンドウハンドルを設定
// 引数: HWND hWndFrame ... オーナーウィンドウ(通常はフレームウィンドウ)
// 戻値: 以前のハンドル(初期値は NULL)
/////////////////////////////////////////////////////////////////////////////////////////

HWND CNxDraw::SetFrameWnd(HWND hWndFrame)
{
	std::swap(hWndFrame, m_hWndFrame);
	return hWndFrame;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CNxSurface* CNxDraw::GetTextTemporarySurface(UINT uWidth, UINT uHeight)
// 概要: テキスト描画用一時サーフェスを取得(指定された幅と高さを持つサーフェスを返す)
// 引数: UINT uWidth  ... サーフェスに要求する幅
//       UINT uHeight ... サーフェスに要求する高さ
// 戻値: サーフェスへのポインタ。NULL ならば失敗
// 備考: サーフェス使用前と使用後に Lock() / Unlock() を行なう事
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CNxSurface* CNxDraw::GetTextTemporarySurface(UINT uWidth, UINT uHeight)
{
	if (m_pTextTemporarySurface.get() == 0
		|| m_pTextTemporarySurface->GetWidth() < uWidth
		|| m_pTextTemporarySurface->GetHeight() < uHeight)
	{	// 未作成又は要求サイズより小さい
		m_pTextTemporarySurface.reset(new CNxSurface(8));
		if (!m_pTextTemporarySurface->Create(uWidth, uHeight))
		{
			m_pTextTemporarySurface.reset();
			_RPTF0(_CRT_ERROR, "サーフェスの作成に失敗しました.\n");
		}
	}
	return m_pTextTemporarySurface.get();
}
