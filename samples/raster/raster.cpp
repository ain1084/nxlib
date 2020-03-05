//
// samples\raster
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "NxRasterSprite.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("raster");
static CNxScreen* g_pScreen = NULL;						// CNxScreen オブジェクト
static CNxSurface* g_pSurface = NULL;					// 画像
static BOOL g_bActive = FALSE;							// アプリケーションがアクティブであれば TRUE

// スプライトの枚数。増やす場合は、下の g_spriteParameter も変更して下さい
static const int g_nSprites = 2;

static const struct SpriteParameter
{
	DWORD dwFlags;		// NxBlt 構造体の dwFlags メンバ
	UINT uOpacity;		// NxBlt 構造体の uOpacity メンバ
	int  nStep;			// CNxRasterSprite の Step
	int  nAmplitude;	// CNxRasterSprite の MaxAmpliude (最大振幅)
} g_spriteParameter[g_nSprites] =
{
	{ NxBlt::blendNormal, 255,   2,  10 },
	{ NxBlt::blendDarken, 255,   2,  50 },
};
static CNxRasterSprite* g_pSprite[g_nSprites];

//////////////////////////////////////////////////////////
// フレームの更新
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	// FPS の更新
	int nFPS = g_pScreen->GetFPS();
	if (nFPS != -1)
	{
		TCHAR szBuf[128];
		if (nFPS < 10000 * 1000)
			_stprintf(szBuf, _T("%s - %3.2f FPS"), szAppName, static_cast<float>(nFPS) / 1000);	// 10000fps 以下
		else
			_stprintf(szBuf, _T("%s - %5d FPS"), szAppName, nFPS / 1000);						// 10000fps 以上

		SetWindowText(hWnd, szBuf);
	}
	g_pScreen->Refresh();
}

////////////////////////////////////////////////////
// ウィンドウプロシージャ
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
			// フルスクリーン化(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_FILE_EXIT:
			// 終了
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

	// サイズは適当にウィンドウを作成
	// CNxScreen クラスがウィンドウを Attach() された時点で、適切なサイズへ変更します。
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
							   szAppName, szAppName,
							   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, hInstance, NULL);

	// 画像の読み込み(エラー処理は省略...)
	g_pSurface = new CNxSurface;
	g_pSurface->Create(CNxFile(_T("BackA.jpg")));

	// CNxScreen オブジェクトの作成と attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// 失敗...
		delete g_pScreen;
		delete g_pSurface;
		DestroyWindow(hWnd);
		return FALSE;
	}

	// ウィンドウモード
	g_pScreen->SetScreenMode(FALSE);
	
	// メニューバーを自動的に隠す様に...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// 背景色を設定
	g_pScreen->SetBkColor(CNxColor::black);

	// スプライトの作成と設定
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

	// ウィンドウを表示
	ShowWindow(hWnd, nCmdShow);

	// メッセージループ
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
