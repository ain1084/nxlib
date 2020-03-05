//
// samples\alphamask
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("alphamask");

static CNxScreen* g_pScreen;			// スクリーン
static CNxLayerSprite* g_pSprite;		// スプライト

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
		case ID_SCREEN:
			// フルスクリーン化(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_FILE_EXIT:
			// 終了
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

	// COM の初期化
	// CNxScreen クラスで DirectDraw を使用する為に必要です。
	CoInitialize(NULL);

	// CNxScreen オブジェクトの作成と attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// 失敗...
		delete g_pScreen;
		DestroyWindow(hWnd);
		return FALSE;
	}
	
	// 最初はウィンドウモードで
	g_pScreen->SetScreenMode(FALSE);

	// メニューバーを自動的に隠す様に...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// 背景色
	g_pScreen->SetBkColor(CNxColor::black);

	// スプライトの作成
	g_pSprite = new CNxLayerSprite(g_pScreen);
	g_pSprite->Create(CNxFile(_T("backA.jpg")));

	// マスク用アルファ画像の読み込み
	CNxSurface* pAlpha = new CNxSurface(8);
	pAlpha->Create(CNxFile(_T("alpha.png")));

	// アルファ画像を 32bpp 画像のアルファチャンネルへ転送
	NxBlt nxb;
	nxb.dwFlags = NxBlt::rgbaMask;
	nxb.nxbRGBAMask.byBlue = 0x00;
	nxb.nxbRGBAMask.byGreen = 0x00;
	nxb.nxbRGBAMask.byRed = 0x00;
	nxb.nxbRGBAMask.byAlpha = 0xff;
	g_pSprite->Blt(NULL, pAlpha, NULL, &nxb);

	// アルファ画像の破棄
	delete pAlpha;

	// スプライトの表示方法を設定
	nxb.dwFlags = NxBlt::srcAlpha;
	g_pSprite->SetNxBlt(&nxb);
	
	// ウィンドウを表示
	::ShowWindow(hWnd, nCmdShow);
	
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
		else
			WaitMessage();
	}

	delete g_pScreen;
	CoUninitialize();
	return msg.wParam;
}
