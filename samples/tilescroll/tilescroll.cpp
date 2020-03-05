//
// samples\tilescroll
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("tilescroll");

static CNxSurface* g_pTexture0;
static CNxSurface* g_pTexture1;
static CNxSurface* g_pDibBuffer;
static BOOL g_bActive;
static POINT g_ptSrcOrg = { 0, 0 };

// FPS �\���p
static DWORD g_dwPrevTime;
static int g_nFrameCount;

// �E�B���h�E�T�C�Y�z��
static const SIZE g_sizeView[] =
{
	{ 256, 256 }, { 320, 240 }, { 640, 480 }, { 800, 600 }
};

///////////////////////////////////////////////////////////
// g_pDibBuffer �̓��e�� hDC �֕��ׂďo��
///////////////////////////////////////////////////////////

static void Refresh(HWND hWnd, HDC hdc)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	HDC hdcDIB = g_pDibBuffer->GetDC();
	for (int y = rect.top; y < rect.bottom; y += g_pDibBuffer->GetHeight())
	{
		for (int x = rect.left; x < rect.right; x += g_pDibBuffer->GetWidth())
		{
			BitBlt(hdc, x, y, g_pDibBuffer->GetWidth(), g_pDibBuffer->GetHeight(), hdcDIB, 0, 0, SRCCOPY);
		}
	}
	g_pDibBuffer->ReleaseDC();
}

//////////////////////////////////////////////////////////
// �t���[���̍X�V
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	// �������̂܂ܓ]��
	g_pDibBuffer->Blt(NULL, g_pTexture0, NULL);

	// ���ʂ�K���ɒʏ�u�����h
	NxBlt nxb;
	nxb.dwFlags = NxBlt::opacity;
	nxb.uOpacity = 80;
	g_pDibBuffer->TileBlt(NULL, g_pTexture1, NULL, g_ptSrcOrg.x, g_ptSrcOrg.y / 2, &nxb);
	g_pDibBuffer->TileBlt(NULL, g_pTexture1, NULL, g_ptSrcOrg.x, g_ptSrcOrg.y, &nxb);

	// �E����
	g_ptSrcOrg.x--;
	g_ptSrcOrg.y--;

	// ��ʂ̍X�V
	HDC hdcClient = GetDC(hWnd);
	Refresh(hWnd, hdcClient);
	ReleaseDC(hWnd, hdcClient);

	// FPS �X�V
	g_nFrameCount++;
	DWORD dwTime = GetTickCount();
	if (g_dwPrevTime + 1000 <= dwTime)
	{
		TCHAR szBuf[128];
		_stprintf(szBuf, _T("%s - %.1f FPS"), szAppName, static_cast<float>(g_nFrameCount * 1000) / (dwTime - g_dwPrevTime));
		SetWindowText(hWnd, szBuf);
		g_dwPrevTime = dwTime;
		g_nFrameCount = 0;
	}
}

////////////////////////////////////////////////////
// �E�B���h�E�̃N���C�A���g�̈�T�C�Y��ύX
////////////////////////////////////////////////////

static void SetClientSize(HWND hWnd, int cx, int cy)
{
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	
	RECT rect;
	SetRect(&rect, 0, 0, cx, cy);
	AdjustWindowRectEx(&rect, dwStyle, GetMenu(hWnd) != NULL, dwExStyle);
	SetWindowPos(hWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE|SWP_NOZORDER);
}

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
		case ID_FILE_EXIT:
			// �I��
			DestroyWindow(hWnd);
			return 0L;
		case ID_VIEW_SIZE_256_256:
		case ID_VIEW_SIZE_320_240:
		case ID_VIEW_SIZE_640_480:
		case ID_VIEW_SIZE_800_600:
			const SIZE& size = g_sizeView[LOWORD(wParam) - ID_VIEW_SIZE_256_256];
			SetClientSize(hWnd, size.cx, size.cy);
			return 0L;
		}
		break;
	case WM_ACTIVATEAPP:
		g_bActive = static_cast<BOOL>(wParam);
		return 0L;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		Refresh(hWnd, ps.hdc);
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

	DWORD dwExStyle = WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX;
	
	// �T�C�Y�͓K���ɃE�B���h�E���쐬
	HWND hWnd = CreateWindowEx(dwExStyle,
							   szAppName, szAppName,
							   dwStyle,
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, hInstance, NULL);

	g_pTexture0 = new CNxSurface;
	g_pTexture0->Create(CNxResourceFile(hInstance, MAKEINTRESOURCE(IDR_PNG_TEX0), _T("PNG")));
	g_pTexture1 = new CNxSurface;
	g_pTexture1->Create(CNxResourceFile(hInstance, MAKEINTRESOURCE(IDR_PNG_TEX1), _T("PNG")));
	
	g_pDibBuffer = new CNxSurface;
	g_pDibBuffer->Create(g_pTexture1->GetWidth(), g_pTexture1->GetHeight());
	
	// �N���C�A���g�̈�̃T�C�Y��ݒ�
	SetClientSize(hWnd, 256, 256);
	
	// �E�B���h�E��\��
	ShowWindow(hWnd, nCmdShow);

	g_dwPrevTime = GetTickCount();

	// ���b�Z�[�W���[�v
	MSG msg;
	for (;;)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			if (hAccel == NULL || TranslateAccelerator(hWnd, hAccel, &msg) == 0)
				TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (g_bActive)
			UpdateFrame(hWnd);
		else
			WaitMessage();
	}
	delete g_pDibBuffer;
	delete g_pTexture1;
	delete g_pTexture0;
	return msg.wParam;
}
