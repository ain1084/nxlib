// MainFrm.cpp : CMainFrame クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "crossfade.h"
#include "MainFrm.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_ACTIVATEAPP()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_COMMAND(ID_SCREEN, OnScreen)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_MMX, OnViewMmx)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MMX, OnUpdateViewMmx)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの構築/消滅

CMainFrame::CMainFrame()
{
	m_bActive = FALSE;
	m_pScreen = NULL;
	m_pSpriteA = NULL;
	m_pSpriteB = NULL;
	m_pLayerFPS = NULL;
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して、Window クラスやスタイルを
	//       修正してください。
	cs.style &= ~(WS_THICKFRAME|WS_MAXIMIZEBOX);
	cs.dwExStyle = WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE;
	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// CNxScreen 作成
	m_pScreen = new CNxScreen;
	if (!m_pScreen->Attach(m_hWnd))
	{
		delete m_pScreen;
		m_pScreen = NULL;
		return -1;
	}
	m_pScreen->SetScreenMode(FALSE);	// ウィンドウモード
	m_pScreen->SetBkColor(CNxColor(CNxColor::black).SetAlpha(0));	// 背景消去を無効化
	m_pScreen->EnableAutoHideMenuBar(TRUE);		// メニューバーを自動的に隠す

	// CNxLayerSprite(CNxSprite + CNxSurface の多重継承クラス) オブジェクトを作成
	m_pSpriteA = new CNxLayerSprite(m_pScreen);
	m_pSpriteA->Create(CNxFile(_T("backA.jpg")));
	m_pSpriteB = new CNxLayerSprite(m_pScreen);
	m_pSpriteB->Create(CNxFile(_T("backB.jpg")));
	
	// 開始時は SpriteA が手前(透明)、SpriteB は奥(不透明)になる
	m_pSpriteA->SetZPos(1);
	m_pSpriteB->SetZPos(0);

	NxBlt nxb;
	// 注意: blendNormal 以外では正しく表示できません
	nxb.dwFlags = NxBlt::blendNormal|NxBlt::opacity;

	// SpriteA は透明
	nxb.uOpacity = 0;
	m_pSpriteA->SetNxBlt(&nxb);

	// SpriteB は不透明
	nxb.uOpacity = 255;
	m_pSpriteB->SetNxBlt(&nxb);

	// FPS 表示スプライト初期化
	InitializeFPS();

	// ウィンドウ表示
	ShowWindow(SW_SHOW);
	return 0;
}

BOOL CMainFrame::OnIdle()
{
	if (!m_bActive)
		return FALSE;

	UpdateFPS();
	
	NxBlt nxb;
	m_pSpriteA->GetNxBlt(&nxb);
	if (nxb.uOpacity >= 255)
	{	// 前景(SpriteA) が完全不透明になったら、背景(SpriteB)とすり替える
		m_pSpriteA->SetZPos(0);		// SpriteA の Z順を奥へ
		m_pSpriteB->SetZPos(1);		// SPriteB の Z順を手前へ
		std::swap(m_pSpriteA, m_pSpriteB);	// ポインタすり替え
		nxb.uOpacity = 0;
	}
	else
		nxb.uOpacity += 4;
	nxb.uOpacity = min(nxb.uOpacity, 255);
	m_pSpriteA->SetNxBlt(&nxb);

	m_pScreen->Refresh();
	return TRUE;
}

BOOL CMainFrame::DestroyWindow() 
{
	// m_pScreen を削除すると、
	// その子スプライト(m_pSpriteA, m_pSpriteB, m_pLayerFPS)も削除されます
	delete m_pScreen;
	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD task)
{
	m_bActive = bActive;
	CFrameWnd::OnActivateApp(bActive, task);
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // 描画用のデバイス コンテキスト
	m_pScreen->Refresh(dc.m_hDC, TRUE);
}

// FPS 更新
void CMainFrame::UpdateFPS()
{
	int nFPS = m_pScreen->GetFPS();
	if (nFPS != -1)
	{
		CString str;
		if (nFPS < 10000 * 1000)
			str.Format(_T("%3.2f FPS"), (float)nFPS / 1000);
		else
			str.Format(_T("%5d FPS"), (int)nFPS / 1000);

		m_pLayerFPS->FillRect(NULL, 0);
		m_pLayerFPS->DrawText(0, 0, NULL, (LPCTSTR)str, (const NxBlt*)NULL);
		m_pLayerFPS->SetUpdate();
	}
}

// FPS 表示用サーフェスとスプライトの初期化
// 8bpp サーフェスを作る
void CMainFrame::InitializeFPS()
{
	m_pLayerFPS = new CNxLayerSprite(m_pScreen, 8);	// 8bpp で作成
	m_pLayerFPS->Create(220, 24);			// サイズ適当
	m_pLayerFPS->SetZPos(INT_MAX);			// 最上位

	NxBlt nxb;
	nxb.dwFlags = NxBlt::colorFill | NxBlt::blendNormal | NxBlt::srcAlpha;
	nxb.nxbColor = CNxColor(0, 255, 255);
	m_pLayerFPS->SetNxBlt(&nxb);
	
	// スムージングを行う
	m_pLayerFPS->SetTextSmoothing(TRUE);

	// フォントを作成して設定
	m_fontFPS = CNxFont(_T("Arial"), -24);
	m_pLayerFPS->SetFont(&m_fontFPS);
}

void CMainFrame::OnScreen() 
{
	m_pScreen->SetScreenMode(!m_pScreen->IsFullScreenMode());
}

void CMainFrame::OnViewMmx() 
{
	CNxDraw* pNxDraw = CNxDraw::GetInstance();
	pNxDraw->EnableMMX(!pNxDraw->IsMMXEnabled());
}

void CMainFrame::OnUpdateViewMmx(CCmdUI* pCmdUI) 
{
	CNxDraw* pNxDraw = CNxDraw::GetInstance();
	pCmdUI->SetCheck(pNxDraw->IsMMXEnabled());
	pCmdUI->Enable(pNxDraw->IsMMXSupported());
}
	
