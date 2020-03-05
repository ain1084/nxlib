//
// samples\stretch
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include <sstream>
#include <NxDraw/NxScreen.h>
#include <NxDraw/NxSurface.h>
#include <NxStorage/NxFile.h>
#include "NxStretchSprite.h"
#include "NxFPSSprite.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("stretch");


static CNxScreen* g_pScreen = NULL;			// CNxScreen �I�u�W�F�N�g
static BOOL g_bActive = FALSE;				// �A�v���P�[�V�������A�N�e�B�u�ł���� TRUE
static BOOL g_bPause  = FALSE;				// �ꎞ��~���Ȃ�� TRUE
static CNxSurface* g_pSurface = NULL;
static CNxStretchSprite* g_pSprite;
static int g_nDirection = 1;

//////////////////////////////////////////////////////////
// �t���[���̍X�V
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	RECT rcSrc;
	g_pSprite->GetSrcRect(&rcSrc);

	// ���݂̊g���̕��ƍ���
	int nSrcWidth = rcSrc.right - rcSrc.left;
	int nSrcHeight = rcSrc.bottom - rcSrc.top;

	if (g_nDirection > 0)
	{	// �k��(1/2 �{�܂�)
		if (nSrcWidth > g_pSprite->GetWidth() * 2 || nSrcHeight > g_pSprite->GetHeight() * 2)
			g_nDirection = -g_nDirection;
	}
	else
	{	// �g�� (32 �{�܂�)
		if (nSrcWidth < g_pSprite->GetWidth() / 32 || nSrcHeight < g_pSprite->GetHeight() / 32)
			g_nDirection = -g_nDirection;
	}
	// RECT ��L�k(�A�X�y�N�g��� 4:3 ��z��...)
	::InflateRect(&rcSrc, g_nDirection * 4, g_nDirection * 3);

	// �V�����T�C�Y��ݒ�
	g_pSprite->SetSrcRect(&rcSrc);

	g_pScreen->Refresh();
}

/////////////////////////////////////////////////////
// �E�B���h�E�e�L�X�g(�L���v�V����)�̍X�V
/////////////////////////////////////////////////////

static void UpdateWindowText(HWND hWnd)
{
	std::basic_ostringstream<TCHAR> ss;
	ss << szAppName;

	if (g_bPause)
		ss << _T(" [��~]");
	
	NxBlt nxb;
	g_pSprite->GetNxBlt(&nxb);
	if (nxb.dwFlags & NxBlt::linearFilter)
		ss << _T(" - linear filtering");

	ss << std::ends;
			
	::SetWindowText(hWnd, ss.str().c_str());
}

static void ChangeNxBlt(HWND hWnd, CNxSurfaceSprite* pSprite, DWORD dwXOR, DWORD dwOR = 0L)
{
	NxBlt nxb;
	pSprite->GetNxBlt(&nxb);
	nxb.dwFlags ^= dwXOR;
	nxb.dwFlags |= dwOR;
	pSprite->SetNxBlt(&nxb);
	InvalidateRect(hWnd, NULL, FALSE);	// �ꎞ��~���ł��ĕ`�悳���l��...
	UpdateWindowText(hWnd);
}

////////////////////////////////////////////////////
// �E�B���h�E�v���V�[�W��
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	NxBlt nxb;

	switch (uMessage)
	{
	case WM_INITMENU:
		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_FULL_SCREEN,
			g_pScreen->IsFullScreenMode() ? MF_CHECKED : MF_UNCHECKED);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_PAUSE,
			g_bPause ? MF_CHECKED : MF_UNCHECKED);

		g_pSprite->GetNxBlt(&nxb);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_LINEARFILTER,
			(nxb.dwFlags & NxBlt::linearFilter) ? MF_CHECKED : MF_UNCHECKED);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_MIRRORLEFTRIGHT,
			(nxb.dwFlags & NxBlt::mirrorLeftRight) ? MF_CHECKED : MF_UNCHECKED);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_MIRRORTOPDOWN,
			(nxb.dwFlags & NxBlt::mirrorTopDown) ? MF_CHECKED : MF_UNCHECKED);
		
		return 0L;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_VIEW_PAUSE:
			g_bPause = !g_bPause;
			if (!g_bPause)
				g_pScreen->ResetFPS();		// Pause ���������ꂽ�Ȃ�΁AFPS �J�E���^�����Z�b�g
			UpdateWindowText(hWnd);
			return 0L;
		case ID_VIEW_LINEARFILTER:
			ChangeNxBlt(hWnd, g_pSprite, NxBlt::linearFilter);
			return 0L;
		case ID_VIEW_MIRRORLEFTRIGHT:
			ChangeNxBlt(hWnd, g_pSprite, NxBlt::mirrorLeftRight, NxBlt::dynamic);
			return 0L;
		case ID_VIEW_MIRRORTOPDOWN:
			ChangeNxBlt(hWnd, g_pSprite, NxBlt::mirrorTopDown, NxBlt::dynamic);
			return 0L;
		case ID_VIEW_FULL_SCREEN:
			// �t���X�N���[����(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_FILE_EXIT:
			// �I��
			DestroyWindow(hWnd);
			return 0L;
		}
		break;
	case WM_ACTIVATEAPP:
		g_bActive = static_cast<BOOL>(wParam);
		return 0L;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		g_pScreen->Refresh(ps.hdc, TRUE);
		EndPaint(hWnd, &ps);
		return 0L;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0L;
	}
	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

///////////////////////////////////////////////////
// �A�v���P�[�V�����̃G���g���|�C���g
///////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	// �E�B���h�E�N���X�o�^
	WNDCLASS wndclass;
	wndclass.style = CS_DBLCLKS;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wndclass.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINFRAME);
	wndclass.lpszClassName = szAppName;
	if (!RegisterClass(&wndclass))
		return FALSE;

	// �L�[�{�[�h�A�N�Z�����[�^�[�̓ǂݍ���
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));

	// �T�C�Y�͓K���ɃE�B���h�E���쐬
	// CNxScreen �N���X���E�B���h�E�� Attach() ���ꂽ���_�ŁA�K�؂ȃT�C�Y�֕ύX���܂��B
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
							   szAppName, szAppName,
							   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, hInstance, NULL);

	// COM �̏�����
	// CNxScreen �N���X�� DirectDraw ���g�p����ׂɕK�v�ł��B
	CoInitialize(NULL);

	// �T�[�t�F�X�̍쐬
	g_pSurface = new CNxSurface;
	if (!g_pSurface->Create(CNxFile(_T("BackA.jpg"))))
	{	// ���s
		::MessageBox(NULL, _T("�T�[�t�F�X�̍쐬�Ɏ��s���܂���"), NULL, MB_OK|MB_ICONSTOP);
		delete g_pSurface;
		return FALSE;
	}

	// CNxScreen �I�u�W�F�N�g�̍쐬�� attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// ���s...
		delete g_pScreen;
		delete g_pSurface;
		DestroyWindow(hWnd);
		return FALSE;
	}

	// �ŏ��̓E�B���h�E���[�h�ŋN��
	g_pScreen->SetScreenMode(FALSE);

	// ���j���[�o�[�������I�ɉB���l��...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// �w�i�F�ݒ�
	g_pScreen->SetBkColor(CNxColor::black);

	// �X�v���C�g�쐬
	g_pSprite = new CNxStretchSprite(g_pScreen, g_pSurface);

	// FPS �\���X�v���C�g�ǉ�
	// CNxFPSSprite �� �I�[�o�[���C�h���ꂽ CNxSprite::PreUpdate() �֐��ɂ���āA
	// �K�v�ȃ^�C�~���O�Ŏ����I�Ɏ������g���X�V���܂��B
	// �e�X�v���C�g�̍폜�ɂ���āA�q�X�v���C�g�������ɍ폜�����ׁA
	// (�폜�ׂ̈�����)�|�C���^��ۑ����Ă����K�v�͂���܂���B
	new CNxFPSSprite(g_pScreen);

	// �E�B���h�E��\��
	ShowWindow(hWnd, nCmdShow);

	// ���b�Z�[�W���[�v
	MSG msg;
	for (;;)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			if (TranslateAccelerator(hWnd, hAccel, &msg) == 0)
				TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (g_bActive && !g_bPause)
			UpdateFrame(hWnd);
		else
			WaitMessage();
	}

	delete g_pScreen;
	delete g_pSurface;

	CoUninitialize();
	return msg.wParam;
}
