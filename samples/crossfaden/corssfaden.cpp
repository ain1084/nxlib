//
// samples\crossfaden
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("crossfaden");

static CNxScreen* g_pScreen;			// スクリーン
static CNxLayerSprite* g_pSpriteA;		// スプライトその1
static CNxLayerSprite* g_pSpriteB;		// スプライトその2
static BOOL g_bActive;					// アプリケーションがアクティブであれば TRUE


//////////////////////////////////////////////////////////
// フレームの更新
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	// FPS 更新
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
	
	// 前景スプライトの NxBlt 構造体を取得
	NxBlt nxb;
	g_pSpriteA->GetNxBlt(&nxb);

	// 前景(SpriteA) が完全不透明になったら、背景(SpriteB)とすり替える
	if (nxb.uOpacity >= 255)
	{
		g_pSpriteA->SetZPos(0);		// SpriteA の Z順を奥へ
		g_pSpriteB->SetZPos(1);		// SPriteB の Z順を手前へ

		// ポインタ入れ換え
		CNxLayerSprite* pTemp = g_pSpriteA;
		g_pSpriteA = g_pSpriteB;
		g_pSpriteB = pTemp;

		nxb.uOpacity = 0;
	}
	else
		nxb.uOpacity += 4;
	nxb.uOpacity = min(nxb.uOpacity, 255);	// 念のため
	g_pSpriteA->SetNxBlt(&nxb);

	// 画面更新
	// 第二引数へ FALSE を指定している為、ウィンドウクライアント領域の破壊を前提とせず、
	// CNxScreen クラスにとって必要な矩形(変更点)のみが画面へ反映されます。
	// ただし、このサンプルではスプライトがクライアント領域と同じサイズであるため、TRUE
	// を指定しても反映される範囲は変わりません。
	//
	// 第一引数を省略した場合、CNxScreen::Attach() 関数で渡された HWND が用いられます
	// (この関数の hWnd と同じです。g_pScreen->Refresh(hWnd) でも結果は同じです)。
	// リアルタイムでスプライトを画面へ反映したい場合は、基本的にこの形式を使用します。
	//	
	// WndProc() 関数の WM_PAINT 部のコメントも参照して下さい。
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
		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_MMX,
			CNxDraw::GetInstance()->IsMMXEnabled() ? MF_CHECKED : MF_UNCHECKED);
		return 0L;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FULLSCREEN:
			// フルスクリーン化(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_VIEW_MMX:
			// MMX enable/disable
			CNxDraw::GetInstance()->EnableMMX(!CNxDraw::GetInstance()->IsMMXEnabled());
			return 0L;
		case ID_FILE_EXIT:
			// 終了
			DestroyWindow(hWnd);
			return 0L;
		}
		return 0L;
	case WM_ACTIVATEAPP:
		g_bActive = static_cast<BOOL>(wParam);
		return 0L;
	case WM_PAINT:
		// CNxScreen::Refresh() 関数の第二引数へ TRUE を指定する事は重要です。省略(FALSE を指定)すると、
		// Refresh() 関数は、自分自身で必要な矩形だけを更新します。しかし、ウィンドウ再描画で必要な矩形
		// と、CNxScreen クラス内で更新が必要な矩形とは、何も関係がありませんので、ウィンドウにとって正
		// しい再描画が行われるとは限りません。
		//
		// 第二引数へ TRUE を指定した場合、子を含む全てのスプライトの PreUpdate() 関数は呼び出されません。
		// これは、PreUpdate() 関数中でスプライトの移動等を行っている場合に、ウィンドウ再描画の度にスプ
		// ライトが移動してしまうのを防ぐためです。
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
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);

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
	// 最初はウィンドウモード
	g_pScreen->SetScreenMode(FALSE);

	// メニューバーを自動的に隠す様に...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// スプライトの作成
	g_pSpriteA = new CNxLayerSprite(g_pScreen);
	g_pSpriteA->Create(CNxFile(_T("backA.jpg")));
	g_pSpriteB = new CNxLayerSprite(g_pScreen);
	g_pSpriteB->Create(CNxFile(_T("backB.jpg")));

	// 開始時は SpriteA が手前(透明)、SpriteB は奥(不透明)になる
	g_pSpriteA->SetZPos(1);
	g_pSpriteB->SetZPos(0);

	NxBlt nxb;
	// 注意: blendNormal 以外では正しく表示できません
	nxb.dwFlags = NxBlt::blendNormal|NxBlt::opacity;

	// SpriteA は透明
	nxb.uOpacity = 0;
	g_pSpriteA->SetNxBlt(&nxb);

	// SpriteB は不透明
	nxb.uOpacity = 255;
	g_pSpriteB->SetNxBlt(&nxb);

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
	// CNxScreen オブジェクトを削除すると、
	// 同時にその子スプライト(g_pSpriteA , g_pSpriteB)も delete されます
	delete g_pScreen;
	CoUninitialize();
	return msg.wParam;
}
