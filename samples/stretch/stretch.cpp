//
// samples\stretch
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include <sstream>
#include <NxDraw/NxScreen.h>
#include <NxDraw/NxSurface.h>
#include <NxStorage/NxFile.h>
#include "NxStretchSprite.h"
#include "NxFPSSprite.h"
#include "resource.h"

static const TCHAR szAppName[] = _T("stretch");


static CNxScreen* g_pScreen = NULL;			// CNxScreen オブジェクト
static BOOL g_bActive = FALSE;				// アプリケーションがアクティブであれば TRUE
static BOOL g_bPause  = FALSE;				// 一時停止中ならば TRUE
static CNxSurface* g_pSurface = NULL;
static CNxStretchSprite* g_pSprite;
static int g_nDirection = 1;

//////////////////////////////////////////////////////////
// フレームの更新
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	RECT rcSrc;
	g_pSprite->GetSrcRect(&rcSrc);

	// 現在の拡大後の幅と高さ
	int nSrcWidth = rcSrc.right - rcSrc.left;
	int nSrcHeight = rcSrc.bottom - rcSrc.top;

	if (g_nDirection > 0)
	{	// 縮小(1/2 倍まで)
		if (nSrcWidth > g_pSprite->GetWidth() * 2 || nSrcHeight > g_pSprite->GetHeight() * 2)
			g_nDirection = -g_nDirection;
	}
	else
	{	// 拡大 (32 倍まで)
		if (nSrcWidth < g_pSprite->GetWidth() / 32 || nSrcHeight < g_pSprite->GetHeight() / 32)
			g_nDirection = -g_nDirection;
	}
	// RECT を伸縮(アスペクト比は 4:3 を想定...)
	::InflateRect(&rcSrc, g_nDirection * 4, g_nDirection * 3);

	// 新しいサイズを設定
	g_pSprite->SetSrcRect(&rcSrc);

	g_pScreen->Refresh();
}

/////////////////////////////////////////////////////
// ウィンドウテキスト(キャプション)の更新
/////////////////////////////////////////////////////

static void UpdateWindowText(HWND hWnd)
{
	std::basic_ostringstream<TCHAR> ss;
	ss << szAppName;

	if (g_bPause)
		ss << _T(" [停止]");
	
	NxBlt nxb;
	g_pSprite->GetNxBlt(&nxb);
	if (nxb.dwFlags & NxBlt::linearFilter)
		ss << _T(" - linear filtering");

	ss << std::ends;
			
	::SetWindowText(hWnd, ss.str().c_str());
}

static void ChangeNxBlt(HWND hWnd, CNxSurfaceSprite* pSprite, DWORD dwXOR, DWORD dwOR = 0L)
{
	NxBlt nxb;
	pSprite->GetNxBlt(&nxb);
	nxb.dwFlags ^= dwXOR;
	nxb.dwFlags |= dwOR;
	pSprite->SetNxBlt(&nxb);
	InvalidateRect(hWnd, NULL, FALSE);	// 一時停止中でも再描画される様に...
	UpdateWindowText(hWnd);
}

////////////////////////////////////////////////////
// ウィンドウプロシージャ
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	NxBlt nxb;

	switch (uMessage)
	{
	case WM_INITMENU:
		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_FULL_SCREEN,
			g_pScreen->IsFullScreenMode() ? MF_CHECKED : MF_UNCHECKED);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_PAUSE,
			g_bPause ? MF_CHECKED : MF_UNCHECKED);

		g_pSprite->GetNxBlt(&nxb);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_LINEARFILTER,
			(nxb.dwFlags & NxBlt::linearFilter) ? MF_CHECKED : MF_UNCHECKED);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_MIRRORLEFTRIGHT,
			(nxb.dwFlags & NxBlt::mirrorLeftRight) ? MF_CHECKED : MF_UNCHECKED);

		CheckMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_MIRRORTOPDOWN,
			(nxb.dwFlags & NxBlt::mirrorTopDown) ? MF_CHECKED : MF_UNCHECKED);
		
		return 0L;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_VIEW_PAUSE:
			g_bPause = !g_bPause;
			if (!g_bPause)
				g_pScreen->ResetFPS();		// Pause が解除されたならば、FPS カウンタをリセット
			UpdateWindowText(hWnd);
			return 0L;
		case ID_VIEW_LINEARFILTER:
			ChangeNxBlt(hWnd, g_pSprite, NxBlt::linearFilter);
			return 0L;
		case ID_VIEW_MIRRORLEFTRIGHT:
			ChangeNxBlt(hWnd, g_pSprite, NxBlt::mirrorLeftRight, NxBlt::dynamic);
			return 0L;
		case ID_VIEW_MIRRORTOPDOWN:
			ChangeNxBlt(hWnd, g_pSprite, NxBlt::mirrorTopDown, NxBlt::dynamic);
			return 0L;
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

	// COM の初期化
	// CNxScreen クラスで DirectDraw を使用する為に必要です。
	CoInitialize(NULL);

	// サーフェスの作成
	g_pSurface = new CNxSurface;
	if (!g_pSurface->Create(CNxFile(_T("BackA.jpg"))))
	{	// 失敗
		::MessageBox(NULL, _T("サーフェスの作成に失敗しました"), NULL, MB_OK|MB_ICONSTOP);
		delete g_pSurface;
		return FALSE;
	}

	// CNxScreen オブジェクトの作成と attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// 失敗...
		delete g_pScreen;
		delete g_pSurface;
		DestroyWindow(hWnd);
		return FALSE;
	}

	// 最初はウィンドウモードで起動
	g_pScreen->SetScreenMode(FALSE);

	// メニューバーを自動的に隠す様に...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// 背景色設定
	g_pScreen->SetBkColor(CNxColor::black);

	// スプライト作成
	g_pSprite = new CNxStretchSprite(g_pScreen, g_pSurface);

	// FPS 表示スプライト追加
	// CNxFPSSprite は オーバーライドされた CNxSprite::PreUpdate() 関数によって、
	// 必要なタイミングで自動的に自分自身を更新します。
	// 親スプライトの削除によって、子スプライトも同時に削除される為、
	// (削除の為だけに)ポインタを保存しておく必要はありません。
	new CNxFPSSprite(g_pScreen);

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
		else if (g_bActive && !g_bPause)
			UpdateFrame(hWnd);
		else
			WaitMessage();
	}

	delete g_pScreen;
	delete g_pSurface;

	CoUninitialize();
	return msg.wParam;
}
