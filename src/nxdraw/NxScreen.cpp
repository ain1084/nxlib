// NxScreen.cpp: CNxScreen クラスのインプリメンテーション
// Copyright(c) 2000,2001 S.Ainoguchi
//
// 概要: 画面全体を管理する CNxWndow 派生クラス
// 
// 2000/11/17 : フルスクリーンへの切り替えに DirectX ではなく
//              ChangeDisplaySettings() を使用する様に変更 
//
// 2000/02/28 : 一部関数を virtual 化
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxScreen.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxScreen::CNxScreen()
 : m_bInitialized(FALSE)		// 一度でも SetScreenMode() が成功したならば TRUE
 , m_hMenuSave(NULL)			// m_bAutoHideMenuBar == TRUE 時のメニュー保存用 
 , m_bAutoHideMenuBar(FALSE)	// フルスクリーン時にメニューバーを自動的に隠す
 , m_bFullScreenMode(FALSE)		// フルスクリーンならば TRUE
 , m_uWidth(640)				// デフォルトの幅
 , m_uHeight(480)				//             高さ
{
	memset(&m_rsi, 0, sizeof(m_rsi));
}

CNxScreen::~CNxScreen()
{
	Detach();
}

//////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxScreen::SetScreenMode(BOOL bFullScreen)
// 概要: スクリーンモードを切り換える(解像度省略)
// 引数: BOOL bFullScreen ... フルスクリーンならば TRUE
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::SetScreenMode(BOOL bFullScreen)
{
	return SetScreenMode(m_uWidth, m_uHeight, bFullScreen);
}

//////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxScreen::SetScreenMode(UINT uWidth, UINT uHeight, BOOL bFullScreen)
// 概要: スクリーンモードを切り換える
// 引数: UINT uWidth      ... 画面の幅
//       UINT uHeight     ... 画面の高さ
//       BOOL bFullScreen ... フルスクリーンならば TRUE
// 戻値: 成功ならば TRUE
//////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::SetScreenMode(UINT uWidth, UINT uHeight, BOOL bFullScreen)
{
	_ASSERT(GetHWND() != NULL);

	if (m_bInitialized)
	{
		if (m_uWidth == uWidth && m_uHeight == uHeight && m_bFullScreenMode == bFullScreen)
		{
			return TRUE;			// 既に初期化済み且つ、同じ画面モード
		}
	}

	/////////////////////////////////
	// 指定されたモードが使用可能か?

	if (bFullScreen)
	{	// フルスクリーンを要求
		if (!CanUseFullScreenMode(uWidth, uHeight))
		{
			return FALSE;
		}
	}
	else
	{	// ウィンドウモードを要求
		if (!CanUseWindowMode())
		{
			return FALSE;
		}
	}
		
	//////////////////////////////////
	// 切り替え開始
	
	// フルスクリーンであれば一度戻す
	if (m_bInitialized)
	{
		if (m_bFullScreenMode)
		{
			RestoreScreenMode(m_rsi);
			restoreMenuBar();
			m_bFullScreenMode = FALSE;
		}
		m_bInitialized = FALSE;
	}

	// ウィンドウサイズを調整
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	adjustWindowSize();

	// 画面モード切り替え
	if (bFullScreen)
	{	// フルスクリーン化

		// 現在のウィンドウ位置とスタイルを保存
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		::GetWindowPlacement(GetHWND(), &wp);
		m_rsi.dwStyle = ::GetWindowLong(GetHWND(), GWL_STYLE);
		m_rsi.dwExStyle = ::GetWindowLong(GetHWND(), GWL_EXSTYLE);
		m_rsi.rcWindow = wp.rcNormalPosition;

		if (!SetFullScreenMode())
		{
			return FALSE;		// 失敗
		}

		// m_bAutoHideMenuBar == TRUE ならば、メニューバーの消去
		if (m_bAutoHideMenuBar)
		{
			HMENU hMenuWnd = ::GetMenu(GetHWND());
			if (hMenuWnd != NULL)
			{
				m_hMenuSave = hMenuWnd;
				::SetMenu(GetHWND(), NULL);
				::DrawMenuBar(GetHWND());
			}
		}
		m_bFullScreenMode = TRUE;
	}
	m_bInitialized = TRUE;				// 初期化済みであるというフラグ
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual HWND CNxScreen::Detach()
// 概要: ウィンドウを切り離す
// 引数: なし
// 戻値: 切り離されたウィンドウハンドル
///////////////////////////////////////////////////////////////////////////////////

HWND CNxScreen::Detach()
{
	if (m_bInitialized)
	{
		if (m_bFullScreenMode)
		{
			RestoreScreenMode(m_rsi);
			restoreMenuBar();
			m_bFullScreenMode = FALSE;
		}
		m_bInitialized = FALSE;
	}
	return CNxWindow::Detach();
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxScreen::EnableAutoHideMenuBar(BOOL bAutoHide)
// 概要: フルスクリーンにおいて自動的にメニューバーを隠す機能を有効又は無効に
// 引数: BOOL bAutoHide  .... TRUE ならメニューバーを隠す
// 戻値: 以前の状態
//////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::EnableAutoHideMenuBar(BOOL bAutoHide)
{
	if (m_bInitialized && m_bFullScreenMode && (bAutoHide != m_bAutoHideMenuBar))
	{
		if (bAutoHide)
		{	// 隠す
			HMENU hMenuWnd = ::GetMenu(GetHWND());
			if (hMenuWnd != NULL)
			{
				m_hMenuSave = hMenuWnd;
				::SetMenu(GetHWND(), NULL);
				::DrawMenuBar(GetHWND());
			}
		}
		else
		{	// 出現
			HMENU hMenuWnd = ::GetMenu(GetHWND());
			if (m_hMenuSave != NULL && hMenuWnd == NULL)
			{
				::SetMenu(GetHWND(), m_hMenuSave);
				::DrawMenuBar(GetHWND());
				m_hMenuSave = NULL;
			}
		}
	}
	std::swap(bAutoHide, m_bAutoHideMenuBar);
	return bAutoHide;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxScreen::OnWndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
// 概要: ウィンドウメッセージを処理
// 引数: UINT uMsg     ... メッセージ
//       WPARAM wParam ... メッセージの追加情報1
//       LPARAM lParam ... メッセージの追加情報2
//////////////////////////////////////////////////////////////////////////////////////////////////

void CNxScreen::OnWndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATEAPP)
	{
		OnActivateApp(static_cast<BOOL>(wParam), static_cast<DWORD>(lParam));
	}
	else
	{
		// フルスクリーンでなければ何もしない
		if (IsFullScreenMode())
		{
			// メニューの処理
			if (m_bAutoHideMenuBar)
			{
				HMENU hMenuWnd = ::GetMenu(GetHWND());
				if (uMsg == WM_SYSCOMMAND)
				{
					UINT uCmdType = wParam;
					if (uCmdType == SC_KEYMENU)
					{	// Alt key が押された
						if (hMenuWnd == NULL && m_hMenuSave != NULL)
						{	// メニューを表示
							::SetMenu(GetHWND(), m_hMenuSave);
							::DrawMenuBar(GetHWND());
							m_hMenuSave = NULL;
							adjustWindowSize();
						}
					}
				} else if (uMsg == WM_MOUSEMOVE)
				{
					if (HIWORD(lParam) == 0 && hMenuWnd == NULL && m_hMenuSave != NULL)
					{	// マウスが一番上にあり & 現在ウィンドウにメニューバー無し &  保存したメニューバー有り
						// 自分で隠したので復帰
						::SetMenu(GetHWND(), m_hMenuSave);
						::DrawMenuBar(GetHWND());
						m_hMenuSave = NULL;
						adjustWindowSize();
					}
					else if (hMenuWnd != NULL)
					{	// マウスは一番上ではない & メニューは現在付いている
						// 自分へ WM_NCHITTEST を送りつけてマウスカーソルの下にメニューバーがあるか調べる
						POINT ptScreen;
						::GetCursorPos(&ptScreen);
						UINT uHit = SendMessage(GetHWND(), WM_NCHITTEST, 0, MAKELPARAM(ptScreen.x, ptScreen.y));
						if (uHit != HTMENU)
						{	// メニューバーの下ではないので、ウィンドウのメニューバーを削除する
							::SetMenu(GetHWND(), NULL);
							::DrawMenuBar(GetHWND());
							m_hMenuSave = hMenuWnd;		// 戻せる様に保存
							adjustWindowSize();
						}
					}
				}
			}
		}
	}
	CNxWindow::OnWndMessage(uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxScreen::adjustWindowSize()
// 概要: メインウィンドウのサイズを補正
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////////////

void CNxScreen::adjustWindowSize()
{
	// ウィンドウサイズを計算
	RECT rcAdjust;
	::SetRect(&rcAdjust, 0, 0, m_uWidth, m_uHeight);
	DWORD dwStyle = ::GetWindowLong(GetHWND(), GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong(GetHWND(), GWL_EXSTYLE);
	::AdjustWindowRectEx(&rcAdjust, dwStyle, ::GetMenu(GetHWND()) != NULL, dwExStyle);

	// ウィンドウサイズを変更
	int nWidth  = rcAdjust.right - rcAdjust.left;
	int nHeight = rcAdjust.bottom - rcAdjust.top;
	::SetWindowPos(GetHWND(), NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
}

///////////////////////////////////////////////////////////////////////
// private:
//	void CNxScreen::restoreMenuBar()
// 概要: メニューバーが消されているならば元に戻す
// 引数: なし
// 戻値: なし
///////////////////////////////////////////////////////////////////////

void CNxScreen::restoreMenuBar()
{
	if (m_bAutoHideMenuBar)
	{
		HMENU hMenuWnd = ::GetMenu(GetHWND());
		if (m_hMenuSave != NULL && hMenuWnd == NULL)
		{
			::SetMenu(GetHWND(), m_hMenuSave);
			::DrawMenuBar(GetHWND());
			m_hMenuSave = NULL;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static UINT CNxScreen::getDisplayBitCount()
// 概要: ディスプレイのピクセルビット数を取得
// 引数: なし
// 戻値: ピクセルビット数
///////////////////////////////////////////////////////////////////////////////////////////////

UINT CNxScreen::getDisplayBitCount()
{
	HDC hdcScreen = ::CreateIC(_T("Display"), NULL, NULL, NULL);
	int nResult = ::GetDeviceCaps(hdcScreen, BITSPIXEL);
	::DeleteDC(hdcScreen);
	return nResult;
}

//////////////////////////////////////////////////////////////////////////////////////
// private:
//	static BOOL CNxScreen::changeDisplay(UINT uWidth, UINT uHeight, DWORD dwFlags)
// 概要: ::ChangeDisplaySettings() を呼び出してディスプレイ解像度を変更
// 引数: UINT uWidth   ... 横幅
//       UINT uHeight  ... 高さ
//       DWORD dwFlags ... ::ChangeDisplaySettings() へ渡す dwFlags
//                         (CDS_TEST, CDS_FULLSCREEN 等)
// 戻値: 成功ならば TRUE
// 備考: 色数は変更しない
//////////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::changeDisplay(UINT uWidth, UINT uHeight, DWORD dwFlags)
{
	// デスクトップの色数を取得
	UINT uDisplayBitCount = getDisplayBitCount();
	if (uDisplayBitCount < 16)
	{
		return FALSE;		// 16bpp 以下ならば失敗
	}
	DEVMODE dm;
	memset(&dm, 0, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
	dm.dmBitsPerPel = uDisplayBitCount;
	dm.dmPelsWidth = uWidth;
	dm.dmPelsHeight = uHeight;
	return ::ChangeDisplaySettings(&dm, dwFlags) == DISP_CHANGE_SUCCESSFUL;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	static BOOL CNxScreen::CanUseWindowMode()
// 概要: ウィンドウモードが利用できれば TRUE を返す
// 引数: なし
//////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::CanUseWindowMode()
{
	// 16bpp 以下ならば失敗
	return (getDisplayBitCount() < 16) ? FALSE : TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	static BOOL CNxScreen::CanUseFullScreenMode(UINT uWidth, UINT uHeight)
// 概要: 指定解像度でフルスクリーンモードが利用できれば TRUE を返す
// 引数: UINT uWidth ... 画面の幅
//       UINT uHeight ... 画面の高さ
//////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::CanUseFullScreenMode(UINT uWidth, UINT uHeight)
{
	return changeDisplay(uWidth, uHeight, CDS_TEST);
}

////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxScreen::OnActivateApp(BOOL bActive, DWORD dwThreadID)
// 概要: アプリケーションのアクティブ化、非アクティブ化メッセージ応答
// 引数: BOOL bActive     ... アクティブ化されるならば TRUE
//       DWORD dwThreadID ... アクティブ化又は非アクティブ化されるスレッドID
// 戻値: なし
////////////////////////////////////////////////////////////////////////////

void CNxScreen::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	// フルスクリーンではない、又はアクティブ化ならば何もしない
	if (!IsFullScreenMode() || bActive)
	{
		return;
	}

	// Windows NT / Windows2000 において、dwThreadID == 0 && bActive == FALSE
	// ならば CTRL+ALT+DEL が押されたと仮定。
	//
	// Windows2000 の場合、解像度は切り替わらないので何もせずに戻る。
	// Windows NT(4.0) の場合、システムによって解像度が切り替えられてしまうの
	// で、普通にウィンドウモードへ復帰させる。

	if (dwThreadID == 0)
	{
		DWORD dwVersion = ::GetVersion();
		if (dwVersion < 0x80000000 && LOBYTE(LOWORD(dwVersion)) >= 5)
		{
			return;		// Windows2000(以降) ならば何もしない
		}
	}
	// ウィンドウモードへ
	SetScreenMode(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual BOOL CNxScreen::SetFullScreenMode()
// 概要: フルスクリーンモード設定
// 戻値: 成功ならば TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::SetFullScreenMode()
{
	if (!changeDisplay(GetScreenWidth(), GetScreenHeight(), CDS_FULLSCREEN))
	{
		_RPTF0(_CRT_WARN, "CNxScreen::SetFullScreenMode() : 解像度の切り替えに失敗しました.\n");
		return FALSE;
	}

	// ウィンドウスタイルと位置を調整
	::SetWindowLong(GetHWND(), GWL_STYLE, WS_POPUP | WS_SYSMENU | ((::IsWindowVisible(GetHWND())) ? WS_VISIBLE : 0));
	::SetWindowLong(GetHWND(), GWL_EXSTYLE, WS_EX_TOPMOST);
	// ウィンドウのZ順を最上位へ
	::SetWindowPos(GetHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_DRAWFRAME);

	// カーソルを中央へ
	::SetCursorPos(GetScreenWidth() / 2, GetScreenHeight() / 2);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxScreen::RestoreScreenMode()
// 概要: フルスクリーンからウィンドウモードへ戻す
// 引数: なし
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////

void CNxScreen::RestoreScreenMode(const RestoreScreenInfo& rsi)
{
	// このウィンドウの TOPMOST を外す際に、
	// 強制的に前景にされてしまう為、
	// 事前に現在の前景ウィンドウを取得しておき、後で戻す
	HWND hWndForeground = ::GetForegroundWindow();

	// スタイルの復帰
	// 保存したスタイル中の WS_VISIBLE は無視し、現在の表示状態を反映
	::SetWindowLong(GetHWND(), GWL_STYLE, (rsi.dwStyle & ~WS_VISIBLE) | (::IsWindowVisible(GetHWND()) ? WS_VISIBLE : 0));
	::SetWindowLong(GetHWND(), GWL_EXSTYLE, rsi.dwExStyle);

	// ディスプレイモードを復帰
	::ChangeDisplaySettings(NULL, 0);

	// ウィンドウ位置とサイズを復帰し、Z 順を TOPMOST から下げる
	const RECT& rect = rsi.rcWindow;
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;
	::SetWindowPos(GetHWND(), HWND_NOTOPMOST, rect.left, rect.top, nWidth, nHeight, SWP_DRAWFRAME | SWP_NOACTIVATE);

	// 取得しておいた前景ウィンドウの Z 順を上位へ
	::SetWindowPos(hWndForeground, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

