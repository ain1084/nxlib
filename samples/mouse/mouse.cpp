//
// samples\mouse
// Copyright(c) 2000 S.Ainoguchi
//
// CNxMouseSprite と CNxTileSprite の使用例

#include "stdafx.h"
#include "NxTileSprite.h"
#include "NxFPSSprite.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("mouse と西瓜");

static CNxWindow* g_pWindow = NULL;			// CNxScreen オブジェクト
static CNxSurface* g_pSurfaceCursor = NULL;
static CNxSurface* g_pSurfaceBG = NULL;
static CNxMouseSprite* g_pSpriteMouse = NULL;
static CNxTileSprite* g_pSpriteBG = NULL;
static BOOL g_bActive = FALSE;				// アプリケーションがアクティブであれば TRUE

//////////////////////////////////////////////////////////
// フレームの更新
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	g_pWindow->Refresh();
}

//////////////////////////////////////////////////////////
// ウィンドウのサイズ変更
//////////////////////////////////////////////////////////

static void OnSize(int cx, int cy)
{
	g_pSpriteBG->SetSize(cx, cy);
}

////////////////////////////////////////////////////
// ウィンドウプロシージャ
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0L;
	case WM_NCMOUSEMOVE:
		// ウィンドウのクライアント領域以外ならば、スプライトのカーソルを消去
		g_pSpriteMouse->SetVisible(FALSE);
		return 0L;
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT)
		{	// ウィンドウのクライアント領域ならば、Windows のカーソルを消去して
			// スプライトのカーソルを表示
			::SetCursor(NULL);
			g_pSpriteMouse->SetVisible(TRUE);
			return TRUE;
		}
		// ウィンドウのクライアント領域以外ならば、スプライトのカーソルを消去
		g_pSpriteMouse->SetVisible(FALSE);

		// メニューループへ入る前に、カーソルの状態を反映
		if (HIWORD(lParam) == 0)
			g_pWindow->Refresh();

		break; // DefWindowPrc() へ
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			// 終了
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
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
							   szAppName, szAppName,
							   WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 200, 200,
							   NULL, NULL, hInstance, NULL);

	// CNxWindow オブジェクトの作成と attach
	g_pWindow = new CNxWindow;
	if (!g_pWindow->Attach(hWnd))
	{	// 失敗...
		delete g_pWindow;
		DestroyWindow(hWnd);
		return FALSE;
	}
	
	// 背景画像の読み込み
	g_pSurfaceBG = new CNxSurface;
	g_pSurfaceBG->Create(CNxResourceFile(NULL, MAKEINTRESOURCE(IDR_PNG_SUIKA), _T("PNG")));	// スイカ...
	
	// タイルスプライト作成
	g_pSpriteBG = new CNxTileSprite(g_pWindow, g_pSurfaceBG);
	
	// マウスカーソル用画像の読み込み
	g_pSurfaceCursor = new CNxSurface;
	g_pSurfaceCursor->Create(CNxResourceFile(NULL, MAKEINTRESOURCE(IDR_PNG_CURSOR), _T("PNG")));

	// マウスカーソルスプライト作成
	g_pSpriteMouse = new CNxMouseSprite(g_pWindow, g_pSurfaceCursor);
	NxBlt nxb;
	nxb.dwFlags = NxBlt::srcAlpha;	// 転送元アルファ使用
	g_pSpriteMouse->SetNxBlt(&nxb);
	g_pSpriteMouse->SetZPos(1);		// Z 順を上げる

	// ウィンドウを表示
	ShowWindow(hWnd, nCmdShow);

	new CNxFPSSprite(g_pWindow);
	
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
		{
			UpdateFrame(hWnd);
		}
	}

	delete g_pSurfaceCursor;
	delete g_pSurfaceBG;
	
	// 親スプライトを削除すると
	// 同時にその子スプライトも delete されます
	delete g_pWindow;

	return msg.wParam;
}
