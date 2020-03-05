//
// samples\raster
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "NxRasterSprite.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("raster");
static CNxScreen* g_pScreen = NULL;						// CNxScreen �I�u�W�F�N�g
static CNxSurface* g_pSurface = NULL;					// �摜
static BOOL g_bActive = FALSE;							// �A�v���P�[�V�������A�N�e�B�u�ł���� TRUE

// �X�v���C�g�̖����B���₷�ꍇ�́A���� g_spriteParameter ���ύX���ĉ�����
static const int g_nSprites = 2;

static const struct SpriteParameter
{
	DWORD dwFlags;		// NxBlt �\���̂� dwFlags �����o
	UINT uOpacity;		// NxBlt �\���̂� uOpacity �����o
	int  nStep;			// CNxRasterSprite �� Step
	int  nAmplitude;	// CNxRasterSprite �� MaxAmpliude (�ő�U��)
} g_spriteParameter[g_nSprites] =
{
	{ NxBlt::blendNormal, 255,   2,  10 },
	{ NxBlt::blendDarken, 255,   2,  50 },
};
static CNxRasterSprite* g_pSprite[g_nSprites];

//////////////////////////////////////////////////////////
// �t���[���̍X�V
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	// FPS �̍X�V
	int nFPS = g_pScreen->GetFPS();
	if (nFPS != -1)
	{
		TCHAR szBuf[128];
		if (nFPS < 10000 * 1000)
			_stprintf(szBuf, _T("%s - %3.2f FPS"), szAppName, static_cast<float>(nFPS) / 1000);	// 10000fps �ȉ�
		else
			_stprintf(szBuf, _T("%s - %5d FPS"), szAppName, nFPS / 1000);						// 10000fps �ȏ�

		SetWindowText(hWnd, szBuf);
	}
	g_pScreen->Refresh();
}

////////////////////////////////////////////////////
// �E�B���h�E�v���V�[�W��
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
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

	// �摜�̓ǂݍ���(�G���[�����͏ȗ�...)
	g_pSurface = new CNxSurface;
	g_pSurface->Create(CNxFile(_T("BackA.jpg")));

	// CNxScreen �I�u�W�F�N�g�̍쐬�� attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// ���s...
		delete g_pScreen;
		delete g_pSurface;
		DestroyWindow(hWnd);
		return FALSE;
	}

	// �E�B���h�E���[�h
	g_pScreen->SetScreenMode(FALSE);
	
	// ���j���[�o�[�������I�ɉB���l��...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// �w�i�F��ݒ�
	g_pScreen->SetBkColor(CNxColor::black);

	// �X�v���C�g�̍쐬�Ɛݒ�
	for (int i = 0; i < g_nSprites; i++)
	{
		g_pSprite[i] = new CNxRasterSprite(g_pScreen, g_pSurface);
		g_pSprite[i]->SetZPos(i);
		g_pSprite[i]->SetStep(g_spriteParameter[i].nStep);
		g_pSprite[i]->SetMaxAmplitude(g_spriteParameter[i].nAmplitude);
		NxBlt nxb;
		nxb.dwFlags = g_spriteParameter[i].dwFlags;
		nxb.uOpacity = g_spriteParameter[i].uOpacity;
		g_pSprite[i]->SetNxBlt(&nxb);
	}

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

	delete g_pScreen;
	delete g_pSurface;

	return msg.wParam;
}
