//
// samples\rulen
// Copyright(c) 2000 S.Ainoguchi
//
#include "stdafx.h"
#include "resource.h"
#include <vector>
#include <algorithm>

static const TCHAR szAppName[] = _T("rulen");

static CNxScreen* g_pScreen = NULL;			// �X�N���[��
static CNxLayerSprite* g_pSpriteA = NULL;	// �X�v���C�g����1
static CNxLayerSprite* g_pSpriteB = NULL;	// �X�v���C�g����2
static int g_nRuleIndex = 0;				// ���[���摜�ԍ�
static BOOL g_bActive = FALSE;				// �A�v���P�[�V�������A�N�e�B�u�ł���� TRUE
static std::vector<NxBlt> g_rule;			// ���[����ێ����� NxBlt �\���̃R���e�i

///////////////////////////////////////////////////////////
// �w��X�v���C�g�փ��[����ݒ�
///////////////////////////////////////////////////////////

static void SetRule(CNxSurfaceSprite* pSprite)
{
	pSprite->SetNxBlt(&g_rule[g_nRuleIndex]);
	g_nRuleIndex = (g_nRuleIndex + 1) % g_rule.size();
}

//////////////////////////////////////////////////////////
// �t���[���̍X�V
//////////////////////////////////////////////////////////

static void UpdateFrame(HWND hWnd)
{
	// FPS �X�V
	int nFPS = g_pScreen->GetFPS();
	if (nFPS != -1)
	{
		TCHAR szBuf[128];
		if (nFPS < 10000 * 1000)
			_stprintf(szBuf, _T("%s - %3.2f FPS"), szAppName, static_cast<float>(nFPS) / 1000);	// 10000fps �ȉ�
		else
			_stprintf(szBuf, _T("%s - %5d FPS"), szAppName, nFPS / 1000);							// 10000fps �ȏ�

		SetWindowText(hWnd, szBuf);
	}
	
	NxBlt nxb;
	g_pSpriteA->GetNxBlt(&nxb);
#if 1
	if (nxb.nxbRule.uLevel >= 512)
	{	// �O�i(SpriteA) �����S�ɕ\��(256)����ď�����(512)��A���̃��[���摜�֐؂�ւ���
#else
	if (nxbf.nxbRule.uLevel >= 256)
	{	// �O�i(SpriteA) �����S�ɕ\�����ꂽ��A�w�i(SpriteB)�Ƃ���ւ���
		m_pSpriteA->SetNxBlt(NULL);			// �]�����@��������(���[���摜�K�p����)
		m_pSpriteA->SetZPos(0);				// SpriteA �� Z��������
		m_pSpriteB->SetZPos(1);				// SPriteB �� Z������O��
		std::swap(g_pSpriteA, g_pSpriteB);	// �|�C���^����ւ�
#endif
		SetRule(g_pSpriteA);				// ��O�ɂȂ����X�v���C�g�փ��[���摜���g�p����l�ɐݒ�
	}
	else
	{
		nxb.nxbRule.uLevel += 4;			// �]����i�s������
		g_pSpriteA->SetNxBlt(&nxb);
	}
	// ��ʍX�V
	// �������� FALSE ���w�肵�Ă���ׁA�E�B���h�E�N���C�A���g�̈�̔j���O��Ƃ����A
	// CNxScreen �N���X�ɂƂ��ĕK�v�ȋ�`(�ύX�_)�݂̂���ʂ֔��f����܂��B
	// �������A���̃T���v���ł̓X�v���C�g���N���C�A���g�̈�Ɠ����T�C�Y�ł��邽�߁ATRUE
	// ���w�肵�Ă����f�����͈͕͂ς��܂���B
	//
	// ���������ȗ������ꍇ�ACNxScreen::Attach() �֐��œn���ꂽ HWND ���p�����܂�
	// (���̊֐��� hWnd �Ɠ����ł��Bg_pScreen->Refresh(hWnd) �ł����ʂ͓����ł�)�B
	// ���A���^�C���ŃX�v���C�g����ʂ֔��f�������ꍇ�́A��{�I�ɂ��̌`�����g�p���܂��B
	//	
	// WndProc() �֐��� WM_PAINT ���̃R�����g���Q�Ƃ��ĉ������B
	g_pScreen->Refresh();
}

////////////////////////////////////////////////////
// �E�B���h�E�v���V�[�W��
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
			// �t���X�N���[����(Alt+Enter)
			g_pScreen->SetScreenMode(!g_pScreen->IsFullScreenMode());
			return 0L;
		case ID_VIEW_MMX:
			// MMX enable/disable
			CNxDraw::GetInstance()->EnableMMX(!CNxDraw::GetInstance()->IsMMXEnabled());
			return 0L;
		case ID_FILE_EXIT:
			// �I��
			::DestroyWindow(hWnd);
			return 0L;
		}
		break;
	case WM_ACTIVATEAPP:
		g_bActive = static_cast<BOOL>(wParam);
		return 0L;
	case WM_PAINT:
		// CNxScreen::Refresh() �֐��̑������� TRUE ���w�肷�鎖�͏d�v�ł��B�ȗ�(FALSE ���w��)����ƁA
		// Refresh() �֐��́A�������g�ŕK�v�ȋ�`�������X�V���܂��B�������A�E�B���h�E�ĕ`��ŕK�v�ȋ�`
		// �ƁACNxScreen �N���X���ōX�V���K�v�ȋ�`�Ƃ́A�����֌W������܂���̂ŁA�E�B���h�E�ɂƂ��Đ�
		// �����ĕ`�悪�s����Ƃ͌���܂���B
		//
		// �������� TRUE ���w�肵���ꍇ�A�q���܂ޑS�ẴX�v���C�g�� PreUpdate() �֐��͌Ăяo����܂���B
		// ����́APreUpdate() �֐����ŃX�v���C�g�̈ړ������s���Ă���ꍇ�ɁA�E�B���h�E�ĕ`��̓x�ɃX�v
		// ���C�g���ړ����Ă��܂��̂�h�����߂ł��B
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
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, hInstance, NULL);

	// CNxScreen �I�u�W�F�N�g�̍쐬�� attach
	g_pScreen = new CNxScreen;
	if (!g_pScreen->Attach(hWnd))
	{	// ���s...
		delete g_pScreen;
		DestroyWindow(hWnd);
		return FALSE;
	}
	// �E�B���h�E���[�h�ŋN��
	g_pScreen->SetScreenMode(FALSE);
	
	// ���j���[�o�[�������I�ɉB���l��...
	g_pScreen->EnableAutoHideMenuBar(TRUE);

	// ���\�[�X���� zip ���ɂ��J��
	CNxResourceFile resFile(hInstance, MAKEINTRESOURCE(IDR_ZIP_DATA), _T("ZIP"));
	CNxZipArchive zipArchive(&resFile);

	// �X�v���C�g�̍쐬
	g_pSpriteA = new CNxLayerSprite(g_pScreen);
	g_pSpriteA->Create(CNxZipFile(&zipArchive, _T("backA.jpg")));
	g_pSpriteB = new CNxLayerSprite(g_pScreen);
	g_pSpriteB->Create(CNxZipFile(&zipArchive, _T("backB.jpg")));

	// �J�n���� SpriteA ����O(����)�ASpriteB �͉�(�s����)�ɂȂ�
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

		nxb.dwFlags = NxBlt::blendNormal|NxBlt::rule;	// �ʏ�u�����h&���[���摜�g�p
		nxb.nxbRule.uLevel =  0;
		nxb.nxbRule.uVague = ruleList[i].uVague;
		nxb.nxbRule.ptOffset.x = 0;
		nxb.nxbRule.ptOffset.y = 0;
		nxb.nxbRule.pSurface = pRuleSurface;
		g_rule.push_back(nxb);
	}

	// �ŏ��̃��[���摜��ݒ�
	SetRule(g_pSpriteA);

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
		else if (g_bActive)
			UpdateFrame(hWnd);
		else
			WaitMessage();
	}
	// CNxScreen �I�u�W�F�N�g���폜����ƁA
	// �����ɂ��̎q�X�v���C�g(g_pSpriteA , g_pSpriteB)�� delete ����܂�
	delete g_pScreen;
	return msg.wParam;
}
