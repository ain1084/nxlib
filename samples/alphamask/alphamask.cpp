//
// samples\alphamask
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("alphamask");

static CNxScreen* g_pScreen;			// �X�N���[��
static CNxLayerSprite* g_pSprite;		// �X�v���C�g

////////////////////////////////////////////////////
// �E�B���h�E�v���V�[�W��
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SCREEN:
			// �t���X�N���[����(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_FILE_EXIT:
			// �I��
			DestroyWindow(hWnd);
			return 0L;
		}
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

	// CNxScreen �I�u�W�F�N�g�̍쐬�� attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// ���s...
		delete g_pScreen;
		DestroyWindow(hWnd);
		return FALSE;
	}
	
	// �ŏ��̓E�B���h�E���[�h��
	g_pScreen->SetScreenMode(FALSE);

	// ���j���[�o�[�������I�ɉB���l��...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// �w�i�F
	g_pScreen->SetBkColor(CNxColor::black);

	// �X�v���C�g�̍쐬
	g_pSprite = new CNxLayerSprite(g_pScreen);
	g_pSprite->Create(CNxFile(_T("backA.jpg")));

	// �}�X�N�p�A���t�@�摜�̓ǂݍ���
	CNxSurface* pAlpha = new CNxSurface(8);
	pAlpha->Create(CNxFile(_T("alpha.png")));

	// �A���t�@�摜�� 32bpp �摜�̃A���t�@�`�����l���֓]��
	NxBlt nxb;
	nxb.dwFlags = NxBlt::rgbaMask;
	nxb.nxbRGBAMask.byBlue = 0x00;
	nxb.nxbRGBAMask.byGreen = 0x00;
	nxb.nxbRGBAMask.byRed = 0x00;
	nxb.nxbRGBAMask.byAlpha = 0xff;
	g_pSprite->Blt(NULL, pAlpha, NULL, &nxb);

	// �A���t�@�摜�̔j��
	delete pAlpha;

	// �X�v���C�g�̕\�����@��ݒ�
	nxb.dwFlags = NxBlt::srcAlpha;
	g_pSprite->SetNxBlt(&nxb);
	
	// �E�B���h�E��\��
	::ShowWindow(hWnd, nCmdShow);
	
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
			WaitMessage();
	}

	delete g_pScreen;
	CoUninitialize();
	return msg.wParam;
}
