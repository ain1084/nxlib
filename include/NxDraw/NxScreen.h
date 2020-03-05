// NxScreen.h: CNxScreen �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: ��ʑS�̂��Ǘ����� CNxWindow �h���N���X
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
	{	// ���C���t���[���E�B���h�E���
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
	BOOL m_bInitialized;			// ��ʂ����������ꂽ�Ȃ� TRUE
	BOOL m_bFullScreenMode;			// �t���X�N���[�����[�h�Ȃ� TRUE
	UINT m_uWidth;					// �t���X�N���[�����̉𑜓x(��)
	UINT m_uHeight;					// �t���X�N���[�����̉𑜓x(����)
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
