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

static CNxScreen* g_pScreen = NULL;			// CNxScreen オブジェクト
static BOOL g_bActive = FALSE;				// アプリケーションがアクティブであれば TRUE

//////////////////////////////////////////////////////////
// フレームの更新
// メッセージループから呼び出されます
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	g_pScreen->Refresh();
}

////////////////////////////////////////////////////
// ウィンドウプロシージャ
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
			// フルスクリーン化(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
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

	// CNxScreen オブジェクトの作成と attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// 失敗...
		delete g_pScreen;
		DestroyWindow(hWnd);
		return FALSE;
	}

	::ShowCursor(FALSE);

	// ウィンドウモード
	g_pScreen->SetScreenMode(640, 480, TRUE);// /*FALSE*/);
	
	// メニューバーを自動的に隠す様に...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	CNxLayerSprite* g_pSurface = new CNxLayerSprite(g_pScreen);
	g_pSurface->Create(CNxResourceFile(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA));

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
	// CNxScreen オブジェクトを削除すると
	// 同時にその子スプライトも delete されます
	delete g_pScreen;

	return msg.wParam;
}
