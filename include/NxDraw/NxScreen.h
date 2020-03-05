// NxScreen.h: CNxScreen クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画面全体を管理する CNxWindow 派生クラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxWindow.h"

class CNxScreen : public CNxWindow
{
public:
	CNxScreen();
	virtual ~CNxScreen();
	virtual HWND Detach();

	BOOL SetScreenMode(BOOL bFullScreen);
	BOOL SetScreenMode(UINT uWidth, UINT uHeight, BOOL bFullScreen);
	BOOL IsFullScreenMode() const;
	UINT GetScreenWidth() const;
	UINT GetScreenHeight() const;
	BOOL EnableAutoHideMenuBar(BOOL bAutoHide);

	static BOOL CanUseFullScreenMode(UINT uWidth, UINT uHeight);
	static BOOL CanUseWindowMode();

protected:
	struct RestoreScreenInfo
	{	// メインフレームウィンドウ情報
		RECT  rcWindow;
		DWORD dwStyle;
		DWORD dwExStyle;
	};

protected:
	virtual void OnWndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	virtual BOOL SetFullScreenMode();
	virtual void RestoreScreenMode(const RestoreScreenInfo& rsi);

private:
	static BOOL changeDisplay(UINT uWidth, UINT uHeight, DWORD dwFlags);
	static UINT getDisplayBitCount();
	void restoreMenuBar();
	void adjustWindowSize();

private:
	BOOL m_bInitialized;			// 画面が初期化されたなら TRUE
	BOOL m_bFullScreenMode;			// フルスクリーンモードなら TRUE
	UINT m_uWidth;					// フルスクリーン時の解像度(幅)
	UINT m_uHeight;					// フルスクリーン時の解像度(高さ)
	HMENU m_hMenuSave;
	BOOL m_bAutoHideMenuBar;
	RestoreScreenInfo m_rsi;

	CNxScreen(const CNxScreen&);
	CNxScreen& operator=(const CNxScreen&);
};

inline BOOL CNxScreen::IsFullScreenMode() const {
	return m_bFullScreenMode; }

inline UINT CNxScreen::GetScreenWidth() const {
	return m_uWidth; }

inline UINT CNxScreen::GetScreenHeight() const {
	return m_uHeight; }
