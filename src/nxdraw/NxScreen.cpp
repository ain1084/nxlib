// NxScreen.cpp: CNxScreen �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000,2001 S.Ainoguchi
//
// �T�v: ��ʑS�̂��Ǘ����� CNxWndow �h���N���X
// 
// 2000/11/17 : �t���X�N���[���ւ̐؂�ւ��� DirectX �ł͂Ȃ�
//              ChangeDisplaySettings() ���g�p����l�ɕύX 
//
// 2000/02/28 : �ꕔ�֐��� virtual ��
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxScreen.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxScreen::CNxScreen()
 : m_bInitialized(FALSE)		// ��x�ł� SetScreenMode() �����������Ȃ�� TRUE
 , m_hMenuSave(NULL)			// m_bAutoHideMenuBar == TRUE ���̃��j���[�ۑ��p 
 , m_bAutoHideMenuBar(FALSE)	// �t���X�N���[�����Ƀ��j���[�o�[�������I�ɉB��
 , m_bFullScreenMode(FALSE)		// �t���X�N���[���Ȃ�� TRUE
 , m_uWidth(640)				// �f�t�H���g�̕�
 , m_uHeight(480)				//             ����
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
// �T�v: �X�N���[�����[�h��؂芷����(�𑜓x�ȗ�)
// ����: BOOL bFullScreen ... �t���X�N���[���Ȃ�� TRUE
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::SetScreenMode(BOOL bFullScreen)
{
	return SetScreenMode(m_uWidth, m_uHeight, bFullScreen);
}

//////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxScreen::SetScreenMode(UINT uWidth, UINT uHeight, BOOL bFullScreen)
// �T�v: �X�N���[�����[�h��؂芷����
// ����: UINT uWidth      ... ��ʂ̕�
//       UINT uHeight     ... ��ʂ̍���
//       BOOL bFullScreen ... �t���X�N���[���Ȃ�� TRUE
// �ߒl: �����Ȃ�� TRUE
//////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::SetScreenMode(UINT uWidth, UINT uHeight, BOOL bFullScreen)
{
	_ASSERT(GetHWND() != NULL);

	if (m_bInitialized)
	{
		if (m_uWidth == uWidth && m_uHeight == uHeight && m_bFullScreenMode == bFullScreen)
		{
			return TRUE;			// ���ɏ������ς݊��A������ʃ��[�h
		}
	}

	/////////////////////////////////
	// �w�肳�ꂽ���[�h���g�p�\��?

	if (bFullScreen)
	{	// �t���X�N���[����v��
		if (!CanUseFullScreenMode(uWidth, uHeight))
		{
			return FALSE;
		}
	}
	else
	{	// �E�B���h�E���[�h��v��
		if (!CanUseWindowMode())
		{
			return FALSE;
		}
	}
		
	//////////////////////////////////
	// �؂�ւ��J�n
	
	// �t���X�N���[���ł���Έ�x�߂�
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

	// �E�B���h�E�T�C�Y�𒲐�
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	adjustWindowSize();

	// ��ʃ��[�h�؂�ւ�
	if (bFullScreen)
	{	// �t���X�N���[����

		// ���݂̃E�B���h�E�ʒu�ƃX�^�C����ۑ�
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		::GetWindowPlacement(GetHWND(), &wp);
		m_rsi.dwStyle = ::GetWindowLong(GetHWND(), GWL_STYLE);
		m_rsi.dwExStyle = ::GetWindowLong(GetHWND(), GWL_EXSTYLE);
		m_rsi.rcWindow = wp.rcNormalPosition;

		if (!SetFullScreenMode())
		{
			return FALSE;		// ���s
		}

		// m_bAutoHideMenuBar == TRUE �Ȃ�΁A���j���[�o�[�̏���
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
	m_bInitialized = TRUE;				// �������ς݂ł���Ƃ����t���O
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual HWND CNxScreen::Detach()
// �T�v: �E�B���h�E��؂藣��
// ����: �Ȃ�
// �ߒl: �؂藣���ꂽ�E�B���h�E�n���h��
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
// �T�v: �t���X�N���[���ɂ����Ď����I�Ƀ��j���[�o�[���B���@�\��L�����͖�����
// ����: BOOL bAutoHide  .... TRUE �Ȃ烁�j���[�o�[���B��
// �ߒl: �ȑO�̏��
//////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::EnableAutoHideMenuBar(BOOL bAutoHide)
{
	if (m_bInitialized && m_bFullScreenMode && (bAutoHide != m_bAutoHideMenuBar))
	{
		if (bAutoHide)
		{	// �B��
			HMENU hMenuWnd = ::GetMenu(GetHWND());
			if (hMenuWnd != NULL)
			{
				m_hMenuSave = hMenuWnd;
				::SetMenu(GetHWND(), NULL);
				::DrawMenuBar(GetHWND());
			}
		}
		else
		{	// �o��
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
// �T�v: �E�B���h�E���b�Z�[�W������
// ����: UINT uMsg     ... ���b�Z�[�W
//       WPARAM wParam ... ���b�Z�[�W�̒ǉ����1
//       LPARAM lParam ... ���b�Z�[�W�̒ǉ����2
//////////////////////////////////////////////////////////////////////////////////////////////////

void CNxScreen::OnWndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATEAPP)
	{
		OnActivateApp(static_cast<BOOL>(wParam), static_cast<DWORD>(lParam));
	}
	else
	{
		// �t���X�N���[���łȂ���Ή������Ȃ�
		if (IsFullScreenMode())
		{
			// ���j���[�̏���
			if (m_bAutoHideMenuBar)
			{
				HMENU hMenuWnd = ::GetMenu(GetHWND());
				if (uMsg == WM_SYSCOMMAND)
				{
					UINT uCmdType = wParam;
					if (uCmdType == SC_KEYMENU)
					{	// Alt key �������ꂽ
						if (hMenuWnd == NULL && m_hMenuSave != NULL)
						{	// ���j���[��\��
							::SetMenu(GetHWND(), m_hMenuSave);
							::DrawMenuBar(GetHWND());
							m_hMenuSave = NULL;
							adjustWindowSize();
						}
					}
				} else if (uMsg == WM_MOUSEMOVE)
				{
					if (HIWORD(lParam) == 0 && hMenuWnd == NULL && m_hMenuSave != NULL)
					{	// �}�E�X����ԏ�ɂ��� & ���݃E�B���h�E�Ƀ��j���[�o�[���� &  �ۑ��������j���[�o�[�L��
						// �����ŉB�����̂ŕ��A
						::SetMenu(GetHWND(), m_hMenuSave);
						::DrawMenuBar(GetHWND());
						m_hMenuSave = NULL;
						adjustWindowSize();
					}
					else if (hMenuWnd != NULL)
					{	// �}�E�X�͈�ԏ�ł͂Ȃ� & ���j���[�͌��ݕt���Ă���
						// ������ WM_NCHITTEST �𑗂���ă}�E�X�J�[�\���̉��Ƀ��j���[�o�[�����邩���ׂ�
						POINT ptScreen;
						::GetCursorPos(&ptScreen);
						UINT uHit = SendMessage(GetHWND(), WM_NCHITTEST, 0, MAKELPARAM(ptScreen.x, ptScreen.y));
						if (uHit != HTMENU)
						{	// ���j���[�o�[�̉��ł͂Ȃ��̂ŁA�E�B���h�E�̃��j���[�o�[���폜����
							::SetMenu(GetHWND(), NULL);
							::DrawMenuBar(GetHWND());
							m_hMenuSave = hMenuWnd;		// �߂���l�ɕۑ�
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
// �T�v: ���C���E�B���h�E�̃T�C�Y��␳
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////////////

void CNxScreen::adjustWindowSize()
{
	// �E�B���h�E�T�C�Y���v�Z
	RECT rcAdjust;
	::SetRect(&rcAdjust, 0, 0, m_uWidth, m_uHeight);
	DWORD dwStyle = ::GetWindowLong(GetHWND(), GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong(GetHWND(), GWL_EXSTYLE);
	::AdjustWindowRectEx(&rcAdjust, dwStyle, ::GetMenu(GetHWND()) != NULL, dwExStyle);

	// �E�B���h�E�T�C�Y��ύX
	int nWidth  = rcAdjust.right - rcAdjust.left;
	int nHeight = rcAdjust.bottom - rcAdjust.top;
	::SetWindowPos(GetHWND(), NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
}

///////////////////////////////////////////////////////////////////////
// private:
//	void CNxScreen::restoreMenuBar()
// �T�v: ���j���[�o�[��������Ă���Ȃ�Ό��ɖ߂�
// ����: �Ȃ�
// �ߒl: �Ȃ�
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
// �T�v: �f�B�X�v���C�̃s�N�Z���r�b�g�����擾
// ����: �Ȃ�
// �ߒl: �s�N�Z���r�b�g��
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
// �T�v: ::ChangeDisplaySettings() ���Ăяo���ăf�B�X�v���C�𑜓x��ύX
// ����: UINT uWidth   ... ����
//       UINT uHeight  ... ����
//       DWORD dwFlags ... ::ChangeDisplaySettings() �֓n�� dwFlags
//                         (CDS_TEST, CDS_FULLSCREEN ��)
// �ߒl: �����Ȃ�� TRUE
// ���l: �F���͕ύX���Ȃ�
//////////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::changeDisplay(UINT uWidth, UINT uHeight, DWORD dwFlags)
{
	// �f�X�N�g�b�v�̐F�����擾
	UINT uDisplayBitCount = getDisplayBitCount();
	if (uDisplayBitCount < 16)
	{
		return FALSE;		// 16bpp �ȉ��Ȃ�Ύ��s
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
// �T�v: �E�B���h�E���[�h�����p�ł���� TRUE ��Ԃ�
// ����: �Ȃ�
//////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::CanUseWindowMode()
{
	// 16bpp �ȉ��Ȃ�Ύ��s
	return (getDisplayBitCount() < 16) ? FALSE : TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	static BOOL CNxScreen::CanUseFullScreenMode(UINT uWidth, UINT uHeight)
// �T�v: �w��𑜓x�Ńt���X�N���[�����[�h�����p�ł���� TRUE ��Ԃ�
// ����: UINT uWidth ... ��ʂ̕�
//       UINT uHeight ... ��ʂ̍���
//////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::CanUseFullScreenMode(UINT uWidth, UINT uHeight)
{
	return changeDisplay(uWidth, uHeight, CDS_TEST);
}

////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxScreen::OnActivateApp(BOOL bActive, DWORD dwThreadID)
// �T�v: �A�v���P�[�V�����̃A�N�e�B�u���A��A�N�e�B�u�����b�Z�[�W����
// ����: BOOL bActive     ... �A�N�e�B�u�������Ȃ�� TRUE
//       DWORD dwThreadID ... �A�N�e�B�u�����͔�A�N�e�B�u�������X���b�hID
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////

void CNxScreen::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	// �t���X�N���[���ł͂Ȃ��A���̓A�N�e�B�u���Ȃ�Ή������Ȃ�
	if (!IsFullScreenMode() || bActive)
	{
		return;
	}

	// Windows NT / Windows2000 �ɂ����āAdwThreadID == 0 && bActive == FALSE
	// �Ȃ�� CTRL+ALT+DEL �������ꂽ�Ɖ���B
	//
	// Windows2000 �̏ꍇ�A�𑜓x�͐؂�ւ��Ȃ��̂ŉ��������ɖ߂�B
	// Windows NT(4.0) �̏ꍇ�A�V�X�e���ɂ���ĉ𑜓x���؂�ւ����Ă��܂���
	// �ŁA���ʂɃE�B���h�E���[�h�֕��A������B

	if (dwThreadID == 0)
	{
		DWORD dwVersion = ::GetVersion();
		if (dwVersion < 0x80000000 && LOBYTE(LOWORD(dwVersion)) >= 5)
		{
			return;		// Windows2000(�ȍ~) �Ȃ�Ή������Ȃ�
		}
	}
	// �E�B���h�E���[�h��
	SetScreenMode(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual BOOL CNxScreen::SetFullScreenMode()
// �T�v: �t���X�N���[�����[�h�ݒ�
// �ߒl: �����Ȃ�� TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxScreen::SetFullScreenMode()
{
	if (!changeDisplay(GetScreenWidth(), GetScreenHeight(), CDS_FULLSCREEN))
	{
		_RPTF0(_CRT_WARN, "CNxScreen::SetFullScreenMode() : �𑜓x�̐؂�ւ��Ɏ��s���܂���.\n");
		return FALSE;
	}

	// �E�B���h�E�X�^�C���ƈʒu�𒲐�
	::SetWindowLong(GetHWND(), GWL_STYLE, WS_POPUP | WS_SYSMENU | ((::IsWindowVisible(GetHWND())) ? WS_VISIBLE : 0));
	::SetWindowLong(GetHWND(), GWL_EXSTYLE, WS_EX_TOPMOST);
	// �E�B���h�E��Z�����ŏ�ʂ�
	::SetWindowPos(GetHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_DRAWFRAME);

	// �J�[�\���𒆉���
	::SetCursorPos(GetScreenWidth() / 2, GetScreenHeight() / 2);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxScreen::RestoreScreenMode()
// �T�v: �t���X�N���[������E�B���h�E���[�h�֖߂�
// ����: �Ȃ�
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////

void CNxScreen::RestoreScreenMode(const RestoreScreenInfo& rsi)
{
	// ���̃E�B���h�E�� TOPMOST ���O���ۂɁA
	// �����I�ɑO�i�ɂ���Ă��܂��ׁA
	// ���O�Ɍ��݂̑O�i�E�B���h�E���擾���Ă����A��Ŗ߂�
	HWND hWndForeground = ::GetForegroundWindow();

	// �X�^�C���̕��A
	// �ۑ������X�^�C������ WS_VISIBLE �͖������A���݂̕\����Ԃ𔽉f
	::SetWindowLong(GetHWND(), GWL_STYLE, (rsi.dwStyle & ~WS_VISIBLE) | (::IsWindowVisible(GetHWND()) ? WS_VISIBLE : 0));
	::SetWindowLong(GetHWND(), GWL_EXSTYLE, rsi.dwExStyle);

	// �f�B�X�v���C���[�h�𕜋A
	::ChangeDisplaySettings(NULL, 0);

	// �E�B���h�E�ʒu�ƃT�C�Y�𕜋A���AZ ���� TOPMOST ���牺����
	const RECT& rect = rsi.rcWindow;
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;
	::SetWindowPos(GetHWND(), HWND_NOTOPMOST, rect.left, rect.top, nWidth, nHeight, SWP_DRAWFRAME | SWP_NOACTIVATE);

	// �擾���Ă������O�i�E�B���h�E�� Z ������ʂ�
	::SetWindowPos(hWndForeground, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

