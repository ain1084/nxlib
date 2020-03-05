//
// samples\mouse
// Copyright(c) 2000 S.Ainoguchi
//
// CNxMouseSprite �� CNxTileSprite �̎g�p��

#include "stdafx.h"
#include "NxTileSprite.h"
#include "NxFPSSprite.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("mouse �Ɛ��Z");

static CNxWindow* g_pWindow = NULL;			// CNxScreen �I�u�W�F�N�g
static CNxSurface* g_pSurfaceCursor = NULL;
static CNxSurface* g_pSurfaceBG = NULL;
static CNxMouseSprite* g_pSpriteMouse = NULL;
static CNxTileSprite* g_pSpriteBG = NULL;
static BOOL g_bActive = FALSE;				// �A�v���P�[�V�������A�N�e�B�u�ł���� TRUE

//////////////////////////////////////////////////////////
// �t���[���̍X�V
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	g_pWindow->Refresh();
}

//////////////////////////////////////////////////////////
// �E�B���h�E�̃T�C�Y�ύX
//////////////////////////////////////////////////////////

static void OnSize(int cx, int cy)
{
	g_pSpriteBG->SetSize(cx, cy);
}

////////////////////////////////////////////////////
// �E�B���h�E�v���V�[�W��
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0L;
	case WM_NCMOUSEMOVE:
		// �E�B���h�E�̃N���C�A���g�̈�ȊO�Ȃ�΁A�X�v���C�g�̃J�[�\��������
		g_pSpriteMouse->SetVisible(FALSE);
		return 0L;
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT)
		{	// �E�B���h�E�̃N���C�A���g�̈�Ȃ�΁AWindows �̃J�[�\������������
			// �X�v���C�g�̃J�[�\����\��
			::SetCursor(NULL);
			g_pSpriteMouse->SetVisible(TRUE);
			return TRUE;
		}
		// �E�B���h�E�̃N���C�A���g�̈�ȊO�Ȃ�΁A�X�v���C�g�̃J�[�\��������
		g_pSpriteMouse->SetVisible(FALSE);

		// ���j���[���[�v�֓���O�ɁA�J�[�\���̏�Ԃ𔽉f
		if (HIWORD(lParam) == 0)
			g_pWindow->Refresh();

		break; // DefWindowPrc() ��
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			// �I��
			DestroyWindow(hWnd);
			break;
		}
		return 0L;
	case WM_ACTIVATEAPP:
		g_bActive = static_cast<BOOL>(wParam);
		return 0L;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		g_pWindow->Refresh(ps.hdc, TRUE);
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
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
							   szAppName, szAppName,
							   WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 200, 200,
							   NULL, NULL, hInstance, NULL);

	// CNxWindow �I�u�W�F�N�g�̍쐬�� attach
	g_pWindow = new CNxWindow;
	if (!g_pWindow->Attach(hWnd))
	{	// ���s...
		delete g_pWindow;
		DestroyWindow(hWnd);
		return FALSE;
	}
	
	// �w�i�摜�̓ǂݍ���
	g_pSurfaceBG = new CNxSurface;
	g_pSurfaceBG->Create(CNxResourceFile(NULL, MAKEINTRESOURCE(IDR_PNG_SUIKA), _T("PNG")));	// �X�C�J...
	
	// �^�C���X�v���C�g�쐬
	g_pSpriteBG = new CNxTileSprite(g_pWindow, g_pSurfaceBG);
	
	// �}�E�X�J�[�\���p�摜�̓ǂݍ���
	g_pSurfaceCursor = new CNxSurface;
	g_pSurfaceCursor->Create(CNxResourceFile(NULL, MAKEINTRESOURCE(IDR_PNG_CURSOR), _T("PNG")));

	// �}�E�X�J�[�\���X�v���C�g�쐬
	g_pSpriteMouse = new CNxMouseSprite(g_pWindow, g_pSurfaceCursor);
	NxBlt nxb;
	nxb.dwFlags = NxBlt::srcAlpha;	// �]�����A���t�@�g�p
	g_pSpriteMouse->SetNxBlt(&nxb);
	g_pSpriteMouse->SetZPos(1);		// Z �����グ��

	// �E�B���h�E��\��
	ShowWindow(hWnd, nCmdShow);

	new CNxFPSSprite(g_pWindow);
	
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
		else
		{
			UpdateFrame(hWnd);
		}
	}

	delete g_pSurfaceCursor;
	delete g_pSurfaceBG;
	
	// �e�X�v���C�g���폜�����
	// �����ɂ��̎q�X�v���C�g�� delete ����܂�
	delete g_pWindow;

	return msg.wParam;
}
