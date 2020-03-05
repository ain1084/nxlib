//
// samples\crossfaden
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("crossfaden");

static CNxScreen* g_pScreen;			// �X�N���[��
static CNxLayerSprite* g_pSpriteA;		// �X�v���C�g����1
static CNxLayerSprite* g_pSpriteB;		// �X�v���C�g����2
static BOOL g_bActive;					// �A�v���P�[�V�������A�N�e�B�u�ł���� TRUE


//////////////////////////////////////////////////////////
// �t���[���̍X�V
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	// FPS �X�V
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
	
	// �O�i�X�v���C�g�� NxBlt �\���̂��擾
	NxBlt nxb;
	g_pSpriteA->GetNxBlt(&nxb);

	// �O�i(SpriteA) �����S�s�����ɂȂ�����A�w�i(SpriteB)�Ƃ���ւ���
	if (nxb.uOpacity >= 255)
	{
		g_pSpriteA->SetZPos(0);		// SpriteA �� Z��������
		g_pSpriteB->SetZPos(1);		// SPriteB �� Z������O��

		// �|�C���^���ꊷ��
		CNxLayerSprite* pTemp = g_pSpriteA;
		g_pSpriteA = g_pSpriteB;
		g_pSpriteB = pTemp;

		nxb.uOpacity = 0;
	}
	else
		nxb.uOpacity += 4;
	nxb.uOpacity = min(nxb.uOpacity, 255);	// �O�̂���
	g_pSpriteA->SetNxBlt(&nxb);

	// ��ʍX�V
	// �������� FALSE ���w�肵�Ă���ׁA�E�B���h�E�N���C�A���g�̈�̔j���O��Ƃ����A
	// CNxScreen �N���X�ɂƂ��ĕK�v�ȋ�`(�ύX�_)�݂̂���ʂ֔��f����܂��B
	// �������A���̃T���v���ł̓X�v���C�g���N���C�A���g�̈�Ɠ����T�C�Y�ł��邽�߁ATRUE
	// ���w�肵�Ă����f�����͈͕͂ς��܂���B
	//
	// ���������ȗ������ꍇ�ACNxScreen::Attach() �֐��œn���ꂽ HWND ���p�����܂�
	// (���̊֐��� hWnd �Ɠ����ł��Bg_pScreen->Refresh(hWnd) �ł����ʂ͓����ł�)�B
	// ���A���^�C���ŃX�v���C�g����ʂ֔��f�������ꍇ�́A��{�I�ɂ��̌`�����g�p���܂��B
	//	
	// WndProc() �֐��� WM_PAINT ���̃R�����g���Q�Ƃ��ĉ������B
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
		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_MMX,
			CNxDraw::GetInstance()->IsMMXEnabled() ? MF_CHECKED : MF_UNCHECKED);
		return 0L;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FULLSCREEN:
			// �t���X�N���[����(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_VIEW_MMX:
			// MMX enable/disable
			CNxDraw::GetInstance()->EnableMMX(!CNxDraw::GetInstance()->IsMMXEnabled());
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
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);

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

	// CNxScreen �I�u�W�F�N�g�̍쐬�� attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// ���s...
		delete g_pScreen;
		DestroyWindow(hWnd);
		return FALSE;
	}
	// �ŏ��̓E�B���h�E���[�h
	g_pScreen->SetScreenMode(FALSE);

	// ���j���[�o�[�������I�ɉB���l��...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// �X�v���C�g�̍쐬
	g_pSpriteA = new CNxLayerSprite(g_pScreen);
	g_pSpriteA->Create(CNxFile(_T("backA.jpg")));
	g_pSpriteB = new CNxLayerSprite(g_pScreen);
	g_pSpriteB->Create(CNxFile(_T("backB.jpg")));

	// �J�n���� SpriteA ����O(����)�ASpriteB �͉�(�s����)�ɂȂ�
	g_pSpriteA->SetZPos(1);
	g_pSpriteB->SetZPos(0);

	NxBlt nxb;
	// ����: blendNormal �ȊO�ł͐������\���ł��܂���
	nxb.dwFlags = NxBlt::blendNormal|NxBlt::opacity;

	// SpriteA �͓���
	nxb.uOpacity = 0;
	g_pSpriteA->SetNxBlt(&nxb);

	// SpriteB �͕s����
	nxb.uOpacity = 255;
	g_pSpriteB->SetNxBlt(&nxb);

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
	// CNxScreen �I�u�W�F�N�g���폜����ƁA
	// �����ɂ��̎q�X�v���C�g(g_pSpriteA , g_pSpriteB)�� delete ����܂�
	delete g_pScreen;
	CoUninitialize();
	return msg.wParam;
}
