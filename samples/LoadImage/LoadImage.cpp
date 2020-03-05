//
// samples\LoadImage
// Copyright(c) 2000 S.Ainoguchi
//
// �ȒP�ȉ摜�r���A�[�ł��Bsusie plug-in �̃f�B���N�g���͕ۑ����܂���
//

#include "stdafx.h"
#include "resource.h"
#include <shlobj.h>
#include <sstream>

static const TCHAR szAppName[] = _T("LoadImage");

static CNxWindow* g_pWindow = NULL;			// �E�B���h�E
static CNxLayerSprite* g_pSprite = NULL;	// �X�v���C�g


////////////////////////////////////////////////////
// �E�B���h�E�N���C�A���g���X�v���C�g�̃T�C�Y�Ƃ��킹��
/////////////////////////////////////////////////////

static void AdjustWindowSize(HWND hWnd, const CNxSprite* pSprite)
{
	// �X�v���C�g�̃T�C�Y�����Ƀt���[���E�B���h�E�̃T�C�Y��ݒ�
	// SetWindowPos() �ɂ���� WM_SIZE ��������ƁACNxWindow ���g�̃T�C�Y���ς��܂�
	RECT rect;
	pSprite->GetRect(&rect);
	HMENU hMenu = ::GetMenu(hWnd);
	DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
	::AdjustWindowRectEx(&rect, dwStyle, hMenu != NULL, dwExStyle);
	::SetWindowPos(hWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER|SWP_NOMOVE);
	// �E�B���h�E�̃N���C�A���g�̈�𖳌���
	InvalidateRect(hWnd, NULL, FALSE);
}

////////////////////////////////////////////////////
// �摜�t�@�C������ CNxLayerSprite ���\�z
////////////////////////////////////////////////////

static BOOL OpenFile(HWND hWnd, LPCTSTR lpszFileName)
{
	// �ꎞ�I�� CNxLayerSprite ���\�z���A
	// ����ɉ摜�t�@�C�����ǂݍ��߂������m�F������A
	// g_pSprite ���폜���Ă֍����ւ��܂�

	// �t�@�C������ CNxLayerSprite ���\�z
	CNxLayerSprite* pSprite = new CNxLayerSprite(g_pWindow, 32);
	if (!pSprite->Create(CNxFile(lpszFileName)))
	{	// ���s
		std::basic_ostringstream<TCHAR> strError;
		strError << lpszFileName << std::endl;
		strError << _T("̧�ق��J���܂���.") << std::endl << std::ends;
		MessageBox(hWnd, strError.str().c_str(), NULL, MB_OK|MB_ICONEXCLAMATION);
		delete pSprite;
		pSprite = NULL;
		return FALSE;
	}

	// �ȑO�̃I�u�W�F�N�g���폜���āA�V�����\�z���� CNxLayerSprite ��ݒ�
	if (g_pSprite != NULL)
	{
		delete g_pSprite;
		g_pSprite = NULL;
	}
	g_pSprite = pSprite;

	// �]�����A���t�@���ߎw��
	NxBlt nxb;
	nxb.dwFlags = NxBlt::srcAlpha;
	g_pSprite->SetNxBlt(&nxb);

	AdjustWindowSize(hWnd, g_pSprite);
	
	// �E�B���h�E�̃L���v�V��������������
	std::basic_ostringstream<TCHAR> strCaption;
	strCaption << szAppName << _T(" - ") << lpszFileName << std::ends;
	::SetWindowText(hWnd, strCaption.str().c_str());
	
	return TRUE;
}

////////////////////////////////////////////////////
// ���j���[����̃t�@�C���� open
////////////////////////////////////////////////////

static void OnFileOpen(HWND hWnd)
{
	TCHAR szFileName[MAX_PATH];
	memset(szFileName, 0, sizeof(szFileName));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("�摜̧��\0*.*\0");
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
		OpenFile(hWnd, szFileName);
}

////////////////////////////////////////////////////
// �t�@�C���� drop
////////////////////////////////////////////////////

static void OnDropFile(HWND hWnd, HDROP hDrop)
{
	TCHAR szFileName[MAX_PATH];
	if (DragQueryFile(hDrop, 0, szFileName, MAX_PATH) != 0)
		OpenFile(hWnd, szFileName);
	DragFinish(hDrop);
}

////////////////////////////////////////////////////
// SHBrowseForFolder �̃R�[���o�b�N�֐�
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
// SUSIE plug-in �f�B���N�g���̎w��
///////////////////////////////////////////////////

static void OnOptionSusieplugin(HWND hWnd)
{
	LPMALLOC lpMalloc;
	if (FAILED(::SHGetMalloc(&lpMalloc)))
		return;

	// ���݂̐ݒ�� CNxDraw ���擾
	std::basic_string<TCHAR> strPlugin;
	CNxDraw::GetInstance()->GetSPIDirectory(strPlugin);
	
	BROWSEINFO bi;
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = _T("susie plug-in �̌����ިڸ�؂�I�����ĉ�����");
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = reinterpret_cast<LPARAM>(strPlugin.c_str());	// �����t�H���_
	bi.iImage = NULL;
	LPITEMIDLIST lpid = ::SHBrowseForFolder(&bi);
	if (lpid != NULL)
	{
		TCHAR szPath[MAX_PATH];
		::SHGetPathFromIDList(lpid, szPath);
		lpMalloc->Free(lpid);
		// �I�����ꂽ�f�B���N�g����ݒ�
		CNxDraw::GetInstance()->SetSPIDirectory(szPath);
	}
	lpMalloc->Release();
}

////////////////////////////////////////////////////
// WM_SYSCOLORCHANGE �����֐�
////////////////////////////////////////////////////

static void OnSysColorChange()
{
	// CNxWindow �̔w�i�F�֔��f
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
// �摜�� 90����]
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
// �E�B���h�E�v���V�[�W��
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
			// �I��
			DestroyWindow(hWnd);
			return 0L;
		case ID_FILE_OPEN2:		// �Ȃ��� afxres.h �� ID_FILE_OPEN ��...
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

	// �T�C�Y�͓K���ɃE�B���h�E���쐬
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
							   szAppName, szAppName,
							   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							   CW_USEDEFAULT, CW_USEDEFAULT, 320 /*�K��*/, 240 /*�K��*/,
							   NULL, NULL, hInstance, NULL);

	// CNxWindow �I�u�W�F�N�g�̍쐬�� attach
	g_pWindow = new CNxWindow;
	if (!g_pWindow->Attach(hWnd))
	{	// ���s...
		delete g_pWindow;
		DestroyWindow(hWnd);
		return FALSE;
	}
	
	// �t�@�C���� drop ������
	DragAcceptFiles(hWnd, TRUE);
	
	// �E�B���h�E��\��
	ShowWindow(hWnd, nCmdShow);

	// ���b�Z�[�W���[�v
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
