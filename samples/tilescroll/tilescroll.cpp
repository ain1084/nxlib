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

// FPS 表示用
static DWORD g_dwPrevTime;
static int g_nFrameCount;

// ウィンドウサイズ配列
static const SIZE g_sizeView[] =
{
	{ 256, 256 }, { 320, 240 }, { 640, 480 }, { 800, 600 }
};

///////////////////////////////////////////////////////////
// g_pDibBuffer の内容を hDC へ並べて出力
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
// フレームの更新
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	// 床をそのまま転送
	g_pDibBuffer->Blt(NULL, g_pTexture0, NULL);

	// 水面を適当に通常ブレンド
	NxBlt nxb;
	nxb.dwFlags = NxBlt::opacity;
	nxb.uOpacity = 80;
	g_pDibBuffer->TileBlt(NULL, g_pTexture1, NULL, g_ptSrcOrg.x, g_ptSrcOrg.y / 2, &nxb);
	g_pDibBuffer->TileBlt(NULL, g_pTexture1, NULL, g_ptSrcOrg.x, g_ptSrcOrg.y, &nxb);

	// 右下へ
	g_ptSrcOrg.x--;
	g_ptSrcOrg.y--;

	// 画面の更新
	HDC hdcClient = GetDC(hWnd);
	Refresh(hWnd, hdcClient);
	ReleaseDC(hWnd, hdcClient);

	// FPS 更新
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
// ウィンドウのクライアント領域サイズを変更
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
// ウィンドウプロシージャ
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			// 終了
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
// アプリケーションのエントリポイント
///////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	// ウィンドウクラス登録
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

	// キーボードアクセラレーターの読み込み
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));

	DWORD dwExStyle = WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX;
	
	// サイズは適当にウィンドウを作成
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
	
	// クライアント領域のサイズを設定
	SetClientSize(hWnd, 256, 256);
	
	// ウィンドウを表示
	ShowWindow(hWnd, nCmdShow);

	g_dwPrevTime = GetTickCount();

	// メッセージループ
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
