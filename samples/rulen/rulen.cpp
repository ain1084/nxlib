//
// samples\rulen
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"
#include <vector>
#include <algorithm>

static const TCHAR szAppName[] = _T("rulen");

static CNxScreen* g_pScreen = NULL;			// スクリーン
static CNxLayerSprite* g_pSpriteA = NULL;	// スプライトその1
static CNxLayerSprite* g_pSpriteB = NULL;	// スプライトその2
static int g_nRuleIndex = 0;				// ルール画像番号
static BOOL g_bActive = FALSE;				// アプリケーションがアクティブであれば TRUE
static std::vector<NxBlt> g_rule;			// ルールを保持する NxBlt 構造体コンテナ

///////////////////////////////////////////////////////////
// 指定スプライトへルールを設定
///////////////////////////////////////////////////////////

static void SetRule(CNxSurfaceSprite* pSprite)
{
	pSprite->SetNxBlt(&g_rule[g_nRuleIndex]);
	g_nRuleIndex = (g_nRuleIndex + 1) % g_rule.size();
}

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
			_stprintf(szBuf, _T("%s - %5d FPS"), szAppName, nFPS / 1000);							// 10000fps 以上

		SetWindowText(hWnd, szBuf);
	}
	
	NxBlt nxb;
	g_pSpriteA->GetNxBlt(&nxb);
#if 1
	if (nxb.nxbRule.uLevel >= 512)
	{	// 前景(SpriteA) が完全に表示(256)されて消えた(512)ら、次のルール画像へ切り替える
#else
	if (nxbf.nxbRule.uLevel >= 256)
	{	// 前景(SpriteA) が完全に表示されたら、背景(SpriteB)とすり替える
		m_pSpriteA->SetNxBlt(NULL);			// 転送方法を初期化(ルール画像適用解除)
		m_pSpriteA->SetZPos(0);				// SpriteA の Z順を奥へ
		m_pSpriteB->SetZPos(1);				// SPriteB の Z順を手前へ
		std::swap(g_pSpriteA, g_pSpriteB);	// ポインタすり替え
#endif
		SetRule(g_pSpriteA);				// 手前になったスプライトへルール画像を使用する様に設定
	}
	else
	{
		nxb.nxbRule.uLevel += 4;			// 転送を進行させる
		g_pSpriteA->SetNxBlt(&nxb);
	}
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
			::DestroyWindow(hWnd);
			return 0L;
		}
		break;
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
	// ウィンドウモードで起動
	g_pScreen->SetScreenMode(FALSE);
	
	// メニューバーを自動的に隠す様に...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// リソース中の zip 書庫を開く
	CNxResourceFile resFile(hInstance, MAKEINTRESOURCE(IDR_ZIP_DATA), _T("ZIP"));
	CNxZipArchive zipArchive(&resFile);

	// スプライトの作成
	g_pSpriteA = new CNxLayerSprite(g_pScreen);
	g_pSpriteA->Create(CNxZipFile(&zipArchive, _T("backA.jpg")));
	g_pSpriteB = new CNxLayerSprite(g_pScreen);
	g_pSpriteB->Create(CNxZipFile(&zipArchive, _T("backB.jpg")));

	// 開始時は SpriteA が手前(透明)、SpriteB は奥(不透明)になる
	g_pSpriteA->SetZPos(1);
	g_pSpriteB->SetZPos(0);

	static const struct RuleList
	{
			LPCTSTR lpszFileName;
			UINT uVague;
	} ruleList[] =
	{
		// filename	     uVague
		_T("rule1.png"),  1,
		_T("rule2.png"),  4,
		_T("rule3.png"), 15,
		_T("rule4.png"),  5,
		NULL, 0
	};

	for (int i = 0; ruleList[i].lpszFileName != NULL; i++)
	{

		NxBlt nxb;
		CNxSurface* pRuleSurface = new CNxSurface(8);
		pRuleSurface->Create(CNxZipFile(&zipArchive, ruleList[i].lpszFileName));

		nxb.dwFlags = NxBlt::blendNormal|NxBlt::rule;	// 通常ブレンド&ルール画像使用
		nxb.nxbRule.uLevel =  0;
		nxb.nxbRule.uVague = ruleList[i].uVague;
		nxb.nxbRule.ptOffset.x = 0;
		nxb.nxbRule.ptOffset.y = 0;
		nxb.nxbRule.pSurface = pRuleSurface;
		g_rule.push_back(nxb);
	}

	// 最初のルール画像を設定
	SetRule(g_pSpriteA);

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
	return msg.wParam;
}
