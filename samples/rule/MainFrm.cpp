// MainFrm.cpp : CMainFrame クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "rule.h"
#include "MainFrm.h"

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
	m_nRuleIndex = 0;
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
	m_pScreen->SetScreenMode(FALSE);			// デフォルトはウィンドウモード
	m_pScreen->SetBkColor(CNxColor(CNxColor::black).SetAlpha(0));	// 背景消去を無効化
	m_pScreen->EnableAutoHideMenuBar(TRUE);						// メニューバーを自動的に隠す

	// CNxLayerSprite(CNxSprite + CNxSurface の多重継承クラス) オブジェクトを作成
	m_pSpriteA = new CNxLayerSprite(m_pScreen);
	m_pSpriteA->Create(CNxFile(_T("backA.jpg")));
	m_pSpriteB = new CNxLayerSprite(m_pScreen);
	m_pSpriteB->Create(CNxFile(_T("backB.jpg")));
	
	// 開始時は SpriteA が手前(透明)、SpriteB は奥(不透明)になる
	m_pSpriteA->SetZPos(1);
	m_pSpriteB->SetZPos(0);

	// zip 圧縮ファイルを開く
	CNxFile cf(_T("rules.zip"));
	// CNxZipArchive へ関連付け
	CNxZipArchive zipArchive(&cf);
	

	
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
		CNxSurface* pRuleSurface = new CNxSurface(8);	// ルール画像は 8bpp でなくてはなりません
		pRuleSurface->Create(CNxZipFile(&zipArchive, ruleList[i].lpszFileName));

		nxb.dwFlags = NxBlt::blendNormal|NxBlt::rule;	// 通常ブレンド&ルール画像使用
		nxb.nxbRule.uLevel =  0;
		nxb.nxbRule.uVague = ruleList[i].uVague;
		nxb.nxbRule.ptOffset.x = 0;
		nxb.nxbRule.ptOffset.y = 0;
		nxb.nxbRule.pSurface = pRuleSurface;
		m_rule.push_back(nxb);
	}

	// 最初のルール画像を使用する
	SetRule(m_pSpriteA);

	// FPS 表示スプライト初期化
	InitializeFPS();

	// ウィンドウ表示
	ShowWindow(SW_SHOW);
	return 0;
}


void CMainFrame::SetRule(CNxSurfaceSprite* pSprite)
{
	// NxBlt.nxbRule.pSurface == NULL の場合、
	// 転送元サーフェスのアルファをルールとして使用します

	NxBlt nxb;

	// ルール画像を転送元サーフェスのアルファ部分へ転送
	nxb.dwFlags = NxBlt::rgbaMask;
	nxb.nxbRGBAMask.byAlpha = 0xff;
	nxb.nxbRGBAMask.byRed = 0x00;
	nxb.nxbRGBAMask.byGreen = 0x00;
	nxb.nxbRGBAMask.byBlue = 0x00;
	pSprite->GetSrcSurface()->Blt(NULL, m_rule[m_nRuleIndex].nxbRule.pSurface, NULL, &nxb);
	
	// NxBlt.nxbRule.pSurface = NULL にした、NxBlt を設定
	memcpy(&nxb, &m_rule[m_nRuleIndex], sizeof(NxBlt));
	nxb.nxbRule.pSurface = NULL;
	pSprite->SetNxBlt(&nxb);

	// 次のルール画像へ
	m_nRuleIndex = (m_nRuleIndex + 1) % m_rule.size();
}

BOOL CMainFrame::OnIdle()
{
	if (!m_bActive)
		return FALSE;

	UpdateFPS();
	

	NxBlt nxb;
	m_pSpriteA->GetNxBlt(&nxb);
#if 1
	if (nxb.nxbRule.uLevel >= 512)
	{	// 前景(SpriteA) が完全に表示(256)されて消えた(512)ら、次のルール画像へ切り替える
#else
	if (nxbf.nxbRule.uLevel >= 256)
	{	// 前景(SpriteA) が完全に表示されたら、背景(SpriteB)とすり替える
		m_pSpriteA->SetNxBlt(NULL);			// 転送方法を初期化(ルール画像適用解除)
		m_pSpriteA->SetZPos(0);				// SpriteA の Z順を奥へ
		m_pSpriteB->SetZPos(1);				// SPriteB の Z順を手前へ
		std::swap(m_pSpriteA, m_pSpriteB);	// ポインタすり替え
#endif
		SetRule(m_pSpriteA);				// 手前になったスプライトへルール画像を使用する様に設定
	}
	else
	{
		nxb.nxbRule.uLevel += 4;			// 転送を進行させる
		m_pSpriteA->SetNxBlt(&nxb);
	}
	m_pScreen->Refresh();
	return TRUE;
}

BOOL CMainFrame::DestroyWindow() 
{
	// ルール画像の削除
	RuleContainer::iterator it;
	for (it = m_rule.begin(); it != m_rule.end(); it++)
	{
		delete (*it).nxbRule.pSurface;
	}
	
	// m_pScreen を削除すると、
	// その子スプライト(m_pSpriteA, m_pSpriteB, m_pLayerFPS)も削除されます
	delete m_pScreen;
	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD hTask) 
{
	m_bActive = bActive;
	CFrameWnd::OnActivateApp(bActive, hTask);
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
			str.Format(_T("%3.2f FPS"), static_cast<float>(nFPS) / 1000.0);
		else
			str.Format(_T("%5d FPS"), nFPS / 1000);

		// 8bpp サーフェスへ文字を書く
		m_pLayerFPS->FillRect(NULL, 0);
		m_pLayerFPS->DrawText(0, 0, NULL, str, static_cast<const NxBlt*>(NULL));
		m_pLayerFPS->SetUpdate();
	}
}

// FPS 表示用サーフェスとスプライトの初期化
// 8bpp サーフェスを作る
void CMainFrame::InitializeFPS()
{
	m_pLayerFPS = new CNxLayerSprite(m_pScreen, 8);
	m_pLayerFPS->Create(140, 24);				// 8bpp で作成(サイズ適当)
	m_pLayerFPS->SetZPos(INT_MAX);				// 最上位

	NxBlt nxb;
	nxb.dwFlags = NxBlt::colorFill|NxBlt::blendNormal|NxBlt::srcAlpha;
	nxb.nxbColor = CNxHLSColor(204, 200, 255);
	m_pLayerFPS->SetNxBlt(&nxb);
	
	m_fontFPS.SetFaceName(_T("Arial Bold"));
	m_fontFPS.SetHeight(-24);
	m_pLayerFPS->SetTextSmoothing(TRUE);
	m_pLayerFPS->SetFont(&m_fontFPS);
}

void CMainFrame::OnScreen() 
{
	m_pScreen->SetScreenMode(!m_pScreen->IsFullScreenMode());
}

void CMainFrame::OnViewMmx() 
{
	CNxDraw::GetInstance()->EnableMMX(!CNxDraw::GetInstance()->IsMMXEnabled());
}

void CMainFrame::OnUpdateViewMmx(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(CNxDraw::GetInstance()->IsMMXEnabled());
	pCmdUI->Enable(CNxDraw::GetInstance()->IsMMXSupported());
}
