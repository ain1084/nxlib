//
// samples\screen
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("screen");

#include <NxDraw/NxScreen.h>
#include <NxDraw/NxLayerSprite.h>
#include <NxStorage/NxResourceFile.h>

static CNxScreen* g_pScreen = NULL;			// CNxScreen �I�u�W�F�N�g
static BOOL g_bActive = FALSE;				// �A�v���P�[�V�������A�N�e�B�u�ł���� TRUE

//////////////////////////////////////////////////////////
// �t���[���̍X�V
// ���b�Z�[�W���[�v����Ăяo����܂�
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	g_pScreen->Refresh();
}

////////////////////////////////////////////////////
// �E�B���h�E�v���V�[�W��
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hWnd);
			return 0L;
		}
		break;
	case WM_INITMENU:
		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_FULL_SCREEN,
			g_pScreen->IsFullScreenMode() ? MF_CHECKED : MF_UNCHECKED);
		return 0L;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_VIEW_FULL_SCREEN:
			// �t���X�N���[����(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_FILE_EXIT:
			// �I��
			DestroyWindow(hWnd);
			return 0L;
		}
		return 0L;
	case WM_ACTIVATEAPP:
		g_bActive = static_cast<BOOL>(wParam);
		return 0L;
	case WM_PAINT:
		// CNxScreen::Refresh() �֐��̑������� TRUE ���w�肷�鎖�͏d�v�ł��B�ȗ�(FALSE ���w��)����ƁA
		// Refresh() �֐��́A�������g�ŕK�v�ȋ�`�������X�V���܂��B�������A�E�B���h�E�ĕ`��ŕK�v�ȋ�`
		// �ƁACNxScreen �N���X���ōX�V���K�v�ȋ�`�Ƃ́A�����֌W������܂���̂ŁA�E�B���h�E�ɂƂ��Đ�
		// �����ĕ`�悪�s����Ƃ͌���܂���B
		//
		// �������� TRUE ���w�肵���ꍇ�A�q���܂ޑS�ẴX�v���C�g�� PreUpdate() �֐��͌Ăяo����܂���B
		// ����́APreUpdate() �֐����ŃX�v���C�g�̈ړ������s���Ă���ꍇ�ɁA�E�B���h�E�ĕ`��̓x�ɃX�v
		// ���C�g���ړ����Ă��܂��̂�h�����߂ł��B
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

	// CNxScreen �I�u�W�F�N�g�̍쐬�� attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// ���s...
		delete g_pScreen;
		DestroyWindow(hWnd);
		return FALSE;
	}

	::ShowCursor(FALSE);

	// �E�B���h�E���[�h
	g_pScreen->SetScreenMode(640, 480, TRUE);// /*FALSE*/);
	
	// ���j���[�o�[�������I�ɉB���l��...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	CNxLayerSprite* g_pSurface = new CNxLayerSprite(g_pScreen);
	g_pSurface->Create(CNxResourceFile(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA));

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
		else if (g_bActive)
			UpdateFrame(hWnd);
		else
			WaitMessage();
	}
	// CNxScreen �I�u�W�F�N�g���폜�����
	// �����ɂ��̎q�X�v���C�g�� delete ����܂�
	delete g_pScreen;

	return msg.wParam;
}
