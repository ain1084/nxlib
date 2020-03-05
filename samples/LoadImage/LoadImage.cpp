//
// samples\LoadImage
// Copyright(c) 2000 S.Ainoguchi
//
// 簡単な画像ビュアーです。susie plug-in のディレクトリは保存しません
//

#include "stdafx.h"
#include "resource.h"
#include <shlobj.h>
#include <sstream>

static const TCHAR szAppName[] = _T("LoadImage");

static CNxWindow* g_pWindow = NULL;			// ウィンドウ
static CNxLayerSprite* g_pSprite = NULL;	// スプライト


////////////////////////////////////////////////////
// ウィンドウクライアントをスプライトのサイズとあわせる
/////////////////////////////////////////////////////

static void AdjustWindowSize(HWND hWnd, const CNxSprite* pSprite)
{
	// スプライトのサイズを元にフレームウィンドウのサイズを設定
	// SetWindowPos() によって WM_SIZE が送られると、CNxWindow 自身のサイズも変わります
	RECT rect;
	pSprite->GetRect(&rect);
	HMENU hMenu = ::GetMenu(hWnd);
	DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
	::AdjustWindowRectEx(&rect, dwStyle, hMenu != NULL, dwExStyle);
	::SetWindowPos(hWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER|SWP_NOMOVE);
	// ウィンドウのクライアント領域を無効化
	InvalidateRect(hWnd, NULL, FALSE);
}

////////////////////////////////////////////////////
// 画像ファイルから CNxLayerSprite を構築
////////////////////////////////////////////////////

static BOOL OpenFile(HWND hWnd, LPCTSTR lpszFileName)
{
	// 一時的な CNxLayerSprite を構築し、
	// 正常に画像ファイルが読み込めた事を確認した後、
	// g_pSprite を削除してへ差し替えます

	// ファイルから CNxLayerSprite を構築
	CNxLayerSprite* pSprite = new CNxLayerSprite(g_pWindow, 32);
	if (!pSprite->Create(CNxFile(lpszFileName)))
	{	// 失敗
		std::basic_ostringstream<TCHAR> strError;
		strError << lpszFileName << std::endl;
		strError << _T("ﾌｧｲﾙを開けません.") << std::endl << std::ends;
		MessageBox(hWnd, strError.str().c_str(), NULL, MB_OK|MB_ICONEXCLAMATION);
		delete pSprite;
		pSprite = NULL;
		return FALSE;
	}

	// 以前のオブジェクトを削除して、新しく構築した CNxLayerSprite を設定
	if (g_pSprite != NULL)
	{
		delete g_pSprite;
		g_pSprite = NULL;
	}
	g_pSprite = pSprite;

	// 転送元アルファ透過指定
	NxBlt nxb;
	nxb.dwFlags = NxBlt::srcAlpha;
	g_pSprite->SetNxBlt(&nxb);

	AdjustWindowSize(hWnd, g_pSprite);
	
	// ウィンドウのキャプションを書き換え
	std::basic_ostringstream<TCHAR> strCaption;
	strCaption << szAppName << _T(" - ") << lpszFileName << std::ends;
	::SetWindowText(hWnd, strCaption.str().c_str());
	
	return TRUE;
}

////////////////////////////////////////////////////
// メニューからのファイルの open
////////////////////////////////////////////////////

static void OnFileOpen(HWND hWnd)
{
	TCHAR szFileName[MAX_PATH];
	memset(szFileName, 0, sizeof(szFileName));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("画像ﾌｧｲﾙ\0*.*\0");
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
		OpenFile(hWnd, szFileName);
}

////////////////////////////////////////////////////
// ファイルの drop
////////////////////////////////////////////////////

static void OnDropFile(HWND hWnd, HDROP hDrop)
{
	TCHAR szFileName[MAX_PATH];
	if (DragQueryFile(hDrop, 0, szFileName, MAX_PATH) != 0)
		OpenFile(hWnd, szFileName);
	DragFinish(hDrop);
}

////////////////////////////////////////////////////
// SHBrowseForFolder のコールバック関数
////////////////////////////////////////////////////

static int WINAPI BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

///////////////////////////////////////////////////
// SUSIE plug-in ディレクトリの指定
///////////////////////////////////////////////////

static void OnOptionSusieplugin(HWND hWnd)
{
	LPMALLOC lpMalloc;
	if (FAILED(::SHGetMalloc(&lpMalloc)))
		return;

	// 現在の設定を CNxDraw より取得
	std::basic_string<TCHAR> strPlugin;
	CNxDraw::GetInstance()->GetSPIDirectory(strPlugin);
	
	BROWSEINFO bi;
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = _T("susie plug-in の検索ﾃﾞｨﾚｸﾄﾘを選択して下さい");
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = reinterpret_cast<LPARAM>(strPlugin.c_str());	// 初期フォルダ
	bi.iImage = NULL;
	LPITEMIDLIST lpid = ::SHBrowseForFolder(&bi);
	if (lpid != NULL)
	{
		TCHAR szPath[MAX_PATH];
		::SHGetPathFromIDList(lpid, szPath);
		lpMalloc->Free(lpid);
		// 選択されたディレクトリを設定
		CNxDraw::GetInstance()->SetSPIDirectory(szPath);
	}
	lpMalloc->Release();
}

////////////////////////////////////////////////////
// WM_SYSCOLORCHANGE 応答関数
////////////////////////////////////////////////////

static void OnSysColorChange()
{
	// CNxWindow の背景色へ反映
	COLORREF color = GetSysColor(COLOR_WINDOW);
	g_pWindow->SetBkColor(CNxColor().SetColorRef(color));
}


static void Rotate90(CNxSurface* pDestSurface, const CNxSurface* pSrcSurface)
{
    const BYTE* lpbSrc = static_cast<const BYTE*>(pSrcSurface->GetBits());
    UINT uSrcHeight = pSrcSurface->GetHeight();
    UINT uSrcWidth = pSrcSurface->GetWidth();

    LONG lDestPitch = pDestSurface->GetPitch();
    LONG lSrcDistance = pSrcSurface->GetPitch() - uSrcWidth * 4;

    for (UINT v = 0; v < uSrcHeight; v++)
    {
        LPBYTE lpbDest = static_cast<LPBYTE>(pDestSurface->GetBits()) + (uSrcHeight - v - 1) * 4;
        for (UINT u = 0; u < uSrcWidth; u++)
        {
            *reinterpret_cast<LPDWORD>(lpbDest) = *reinterpret_cast<const DWORD*>(lpbSrc);
            lpbDest += lDestPitch;
            lpbSrc += 4;
        }
        lpbSrc += lSrcDistance;
    }
}

static void Rotate270(CNxSurface* pDestSurface, const CNxSurface* pSrcSurface)
{
    const BYTE* lpbSrc = static_cast<const BYTE*>(pSrcSurface->GetBits());
    UINT uSrcHeight = pSrcSurface->GetHeight();
    UINT uSrcWidth = pSrcSurface->GetWidth();

    LONG lDestPitch = pDestSurface->GetPitch();
    LONG lSrcDistance = pSrcSurface->GetPitch() - uSrcWidth * 4;
    for (UINT v = 0; v < uSrcHeight; v++)
    {
        LPBYTE lpbDest = static_cast<LPBYTE>(pDestSurface->GetBits()) + v * 4 + lDestPitch * (uSrcWidth - 1);
        for (UINT u = 0; u < uSrcWidth; u++)
        {
            *reinterpret_cast<LPDWORD>(lpbDest) = *reinterpret_cast<const DWORD*>(lpbSrc);
            lpbDest -= lDestPitch;
            lpbSrc += 4;
        }
        lpbSrc += lSrcDistance;
    }
}

////////////////////////////////////////////////////
// 画像の 90°回転
////////////////////////////////////////////////////

static void OnViewRotate(HWND hWnd, BOOL bRight)
{
	CNxLayerSprite* pTemp = new CNxLayerSprite(g_pWindow);
	pTemp->Create(g_pSprite->GetHeight(), g_pSprite->GetWidth());
	if (bRight)
		Rotate90(pTemp, g_pSprite);
	else
		Rotate270(pTemp, g_pSprite);
	delete g_pSprite;
	g_pSprite = pTemp;
	AdjustWindowSize(hWnd, g_pSprite);
}

////////////////////////////////////////////////////
// ウィンドウプロシージャ
////////////////////////////////////////////////////

static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITMENU:
		EnableMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_ROTATE_RIGHT,
			(g_pSprite != NULL) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(reinterpret_cast<HMENU>(wParam), ID_VIEW_ROTATE_LEFT,
			(g_pSprite != NULL) ? MF_ENABLED : MF_GRAYED);
		return 0L;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_VIEW_ROTATE_RIGHT:
			OnViewRotate(hWnd, TRUE);
			return 0L;
		case ID_VIEW_ROTATE_LEFT:
			OnViewRotate(hWnd, FALSE);
			return 0L;
		case ID_FILE_EXIT:
			// 終了
			DestroyWindow(hWnd);
			return 0L;
		case ID_FILE_OPEN2:		// なぜか afxres.h で ID_FILE_OPEN が...
			OnFileOpen(hWnd);
			return 0L;
		case ID_OPTION_SUSIEPLUGIN:
			OnOptionSusieplugin(hWnd);
			return 0L;
		}
		break;
	case WM_SYSCOLORCHANGE:
		OnSysColorChange();
		return 0L;
	case WM_DROPFILES:
		OnDropFile(hWnd, reinterpret_cast<HDROP>(wParam)); 
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
							   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							   CW_USEDEFAULT, CW_USEDEFAULT, 320 /*適当*/, 240 /*適当*/,
							   NULL, NULL, hInstance, NULL);

	// CNxWindow オブジェクトの作成と attach
	g_pWindow = new CNxWindow;
	if (!g_pWindow->Attach(hWnd))
	{	// 失敗...
		delete g_pWindow;
		DestroyWindow(hWnd);
		return FALSE;
	}
	
	// ファイルの drop を許可
	DragAcceptFiles(hWnd, TRUE);
	
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
	}

	delete g_pWindow;
	return msg.wParam;
}
