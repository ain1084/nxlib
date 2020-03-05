// MainFrm.cpp : CMainFrame クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "ball.h"

#include "MainFrm.h"
#include "NxBallSprite.h"
#include "NxRasterSprite.h"
#include "NxStretchSpriteDemo.h"
#include "NxBlurSpriteDemo.h"
#include "NxTileSpriteDemo.h"
#include <NxDraw/NxHLSColor.h>
#include <NxStorage/NxFile.h>

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
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_COMMAND(ID_SCREEN, OnScreen)
	ON_WM_ACTIVATEAPP()
	ON_COMMAND(ID_VIEW_MMX_ENABLE, OnViewMmxEnable)
	ON_COMMAND(ID_VIEW_PAUSE, OnViewPause)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PAUSE, OnUpdateViewPause)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_MMX_ENABLE, OnUpdateViewMmxEnable)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_VIEW_3DNOW_ENABLE, OnView3dnowEnable)
	ON_UPDATE_COMMAND_UI(ID_VIEW_3DNOW_ENABLE, OnUpdateView3dnowEnable)
	ON_COMMAND(ID_VIEW_BALL, OnViewBall)
	ON_COMMAND(ID_VIEW_TRACKING, OnViewTracking)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TRACKING, OnUpdateViewTracking)
	ON_COMMAND(ID_VIEW_BRIGHTNESS_LIGHT, OnViewBrightnessLight)
	ON_COMMAND(ID_VIEW_BRIGHTNESS_DARK, OnViewBrightnessDark)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BRIGHTNESS_LIGHT, OnUpdateViewBrightnessLight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BRIGHTNESS_DARK, OnUpdateViewBrightnessDark)
	ON_COMMAND(ID_VIEW_BRIGHTNESS_RESET, OnViewBrightnessReset)
	ON_COMMAND(ID_VIEW_FPS_SMOOTH, OnViewFpsSmooth)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FPS_SMOOTH, OnUpdateViewFpsSmooth)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_BLEND_NONE, ID_VIEW_BLEND_DARKEN, OnUpdateViewBlendRange)
	ON_COMMAND_RANGE(ID_VIEW_BLEND_NONE, ID_VIEW_BLEND_DARKEN, OnViewBlendRange)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_BACKGROUND_DISABLE, ID_VIEW_BACKGROUND_TILE, OnUpdateViewBackgroundRange)
	ON_COMMAND_RANGE(ID_VIEW_BACKGROUND_DISABLE, ID_VIEW_BACKGROUND_TILE, OnViewBackgroundRange)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_FILTER_NONE, ID_VIEW_FILTER_NEGATIVE, OnUpdateViewFilterRange)
	ON_COMMAND_RANGE(ID_VIEW_FILTER_NONE, ID_VIEW_FILTER_NEGATIVE, OnViewFilterRange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの構築/消滅

CMainFrame::CMainFrame()
{
	m_pScreen = NULL;
	m_pSpriteBG = NULL;
	m_pSurfaceBG = NULL;
	m_pSurfaceTile = NULL;
	m_pLayerFPS = NULL;
	m_bPause = FALSE;
	m_bTimer = FALSE;
	m_bTextSmooth = TRUE;
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

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	// 背景消去をしない
	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// CNxScreen 作成
	m_pScreen = new CNxBallScreen;
	m_pScreen->EnableAutoHideMenuBar(TRUE);			// メニューバーが自動的に隠れる様にする
	if (!m_pScreen->Attach(m_hWnd))
	{
		delete m_pScreen;
		m_pScreen = NULL;
		return -1;
	}

	// 最初はウィンドウモード
	m_pScreen->SetScreenMode(1280, 960, FALSE);
//	m_pScreen->SetScreenMode(FALSE);

	// 背景サーフェスを作成、イメージをファイルから読み込む
	m_pSurfaceBG = new CNxSurface();
	m_pSurfaceBG->Create(CNxFile(_T("backA.jpg")));

	// タイル用(ただ小さいだけの)サーフェスをイメージから生成
	m_pSurfaceTile = new CNxSurface;
	m_pSurfaceTile->Create(CNxFile(_T("tile.png")));

	// Ball のサーフェスを作成
	// 6 個分のサイズのサーフェスを作成して、左上の一つをファイルから読み込む
	m_pSurfaceBall = new CNxSurface;
	m_pSurfaceBall->Create(32 * 6, 32);
	m_pSurfaceBall->LoadImage(0, 0, CNxFile(_T("ball.png")));

	// 色相変換によって色違いのボールを作成
	for (int i = 0; i < 5; i++)
	{
		NxFilterBlt nxf;
		nxf.dwFlags = NxFilterBlt::hueTransform;
		nxf.nxfHueTransform.nHue = (i + 1) * 60;	// 色相を 60°ずつ変化させる
		nxf.nxfHueTransform.nSaturation = 0;		// 彩度の変移なし
		nxf.nxfHueTransform.nLightness = 0;			// 明度の変移なし
		nxf.nxfHueTransform.bSingleness = FALSE;	// 単一色相にしない
		m_pSurfaceBall->FilterBlt((i + 1) * 32, 0, m_pSurfaceBall, CRect(0, 0, 32, 32), &nxf);
	}

	// 背景を設定
	SetBackground(Background_normal);

	// FPS 表示用サーフェスとスプライトの初期化
	InitializeFPS();

	// ボール作成
	CreateBall();
	
	// ウィンドウ表示
	ShowWindow(SW_SHOW);

	// 待機タイマ開始
	SetTimer(1, 800, NULL);
	return 0;
}

BOOL CMainFrame::DestroyWindow() 
{
	if (m_pScreen != NULL)
	{
		delete m_pScreen;
		delete m_pSurfaceTile;
		delete m_pSurfaceBG;
		delete m_pSurfaceBall;
		KillTimer(1);
	}
	return CFrameWnd::DestroyWindow();
}

// CBallApp::OnIdle() から呼び出される
BOOL CMainFrame::OnIdle()
{
	if (!m_bActive)
		return FALSE;		// アクティブでない
	
	if (!m_bTimer)
		return FALSE;		// 開始直後タイマーが発生してない
	
	if (m_bPause)
		return FALSE;		// 停止中

	UpdateFPS();

	m_pScreen->Refresh();
	return TRUE;
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // 描画用のデバイス コンテキスト
	m_pScreen->Refresh(dc.m_hDC, TRUE);
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD hTask) 
{
	m_bActive = bActive;
	CFrameWnd::OnActivateApp(bActive, hTask);
}

// Alt+Enter が押された時...
void CMainFrame::OnScreen() 
{
	m_pScreen->SetScreenMode(!m_pScreen->IsFullScreenMode());
}

// 全てのスプライトのブレンド方法を設定
void CMainFrame::SetBallSpriteBlend(BallBlend nBallBlend)
{
	m_ballBlend = nBallBlend;
	
	NxBlt nxbf;

	switch (m_ballBlend)
	{
	case BallBlend_normal:
	case BallBlend_add:
	case BallBlend_sub:
	case BallBlend_multi:
	case BallBlend_screen:
	case BallBlend_darken:
	case BallBlend_brighten:
		nxbf.dwFlags = m_ballBlend | NxBlt::srcAlpha | NxBlt::opacity;
		nxbf.uOpacity = 160;
		break;
	case BallBlend_none:
		nxbf.dwFlags = 0;
		break;
	}

	// 全ての ball に対して NxBlt を設定
	SpriteContainer::iterator it;
	for (it = m_spriteBall.begin(); it != m_spriteBall.end(); it++)
	{
		(*it)->SetNxBlt(&nxbf);
	}

	Invalidate(FALSE);
}

void CMainFrame::SetBackground(Background nBackground)
{
	RECT rcSrc;
	RECT rcScreen;
	m_pScreen->GetRect(&rcScreen);

	m_background = nBackground;
	
	
	delete m_pSpriteBG;
	m_pSpriteBG = NULL;

	switch (m_background)
	{
	case Background_blur:
		m_pSpriteBG = new CNxBlurSpriteDemo(m_pScreen, m_pSurfaceBG);
		static_cast<CNxBlurSpriteDemo*>(m_pSpriteBG)->SetRange(127);
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// 背景消去しない
		break;
	case Background_normal:
		m_pSpriteBG = new CNxSurfaceSprite(m_pScreen, m_pSurfaceBG);
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// 背景消去しない
		break;
	case Background_tile:
		m_pSpriteBG = new CNxTileSpriteDemo(m_pScreen, m_pSurfaceTile);
		m_pSpriteBG->SetRect(&rcScreen);
		m_pSurfaceTile->GetRect(&rcSrc);
		static_cast<CNxTileSpriteDemo*>(m_pSpriteBG)->SetSrcRect(&rcSrc);
		static_cast<CNxTileSpriteDemo*>(m_pSpriteBG)->SetDirection(-1, -1);
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// 背景消去しない
		break;
	case Background_raster:
		m_pSpriteBG = new CNxRasterSprite(m_pScreen, m_pSurfaceBG);
		static_cast<CNxRasterSprite*>(m_pSpriteBG)->SetMaxAmplitude(30);
		static_cast<CNxRasterSprite*>(m_pSpriteBG)->SetStep(4);
		m_pScreen->SetBkColor(CNxColor(CNxColor::black));			// 背景色は黒
		break;
	case Background_stretch:
	case Background_stretch_linear_filter:
		// 中央部分を 16x16 から、4x4 まで拡大
		m_pSpriteBG = new CNxStretchSpriteDemo(m_pScreen, m_pSurfaceBG);
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// 背景消去しない
		// 開始矩形
		rcSrc.left = (m_pSpriteBG->GetWidth() - m_pSpriteBG->GetWidth() / 16) / 2;
		rcSrc.top = (m_pSpriteBG->GetHeight() - m_pSpriteBG->GetHeight() / 16) / 2;
		rcSrc.right = rcSrc.left + m_pSpriteBG->GetWidth() / 16;
		rcSrc.bottom = rcSrc.top + m_pSpriteBG->GetHeight() / 16;
		static_cast<CNxStretchSpriteDemo*>(m_pSpriteBG)->SetSrcBeginRect(&rcSrc);
		// 終了矩形
		rcSrc.left = (m_pSpriteBG->GetWidth() - m_pSpriteBG->GetWidth() / 4) / 2;
		rcSrc.top = (m_pSpriteBG->GetHeight() - m_pSpriteBG->GetHeight() / 4) / 2;
		rcSrc.right = rcSrc.left + m_pSpriteBG->GetWidth() / 4;
		rcSrc.bottom = rcSrc.top + m_pSpriteBG->GetHeight() / 4;
		static_cast<CNxStretchSpriteDemo*>(m_pSpriteBG)->SetSrcEndRect(&rcSrc);
		// 補完付きならば、NxBlt::linearFilter を設定
		if (m_background == Background_stretch_linear_filter)
		{
			NxBlt nxbf;
			nxbf.dwFlags = NxBlt::linearFilter;
			m_pSpriteBG->SetNxBlt(&nxbf);
		}
		break;
	case Background_disable:
	default:
		m_pScreen->SetBkColor(CNxColor(CNxColor::black));
		return;
	}

	m_pSpriteBG->SetZPos(INT_MIN);		// Z は一番奥
	Invalidate(FALSE);
}

CMainFrame::BallBlend CMainFrame::CommandIdToBlend(UINT nID) const
{
	static const BallBlend nBlend[] =
	{
		BallBlend_none, BallBlend_normal, BallBlend_add, BallBlend_sub,
		BallBlend_multi, BallBlend_screen, BallBlend_brighten, BallBlend_darken,
	};
	return nBlend[nID - ID_VIEW_BLEND_NONE];
}

void CMainFrame::OnViewBlendRange(UINT nID) 
{
	SetBallSpriteBlend(CommandIdToBlend(nID));
}

void CMainFrame::OnUpdateViewBlendRange(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_spriteBall.empty());
	pCmdUI->SetRadio(m_ballBlend == CommandIdToBlend(pCmdUI->m_nID));
}

void CMainFrame::OnViewBackgroundRange(UINT nID) 
{
	SetBackground(static_cast<Background>(nID - ID_VIEW_BACKGROUND_DISABLE));
}

void CMainFrame::OnUpdateViewBackgroundRange(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_background == (pCmdUI->m_nID - ID_VIEW_BACKGROUND_DISABLE));
}

void CMainFrame::OnViewFilterRange(UINT nID)
{
	m_pScreen->SetFilter(static_cast<CNxBallScreen::Filter>(nID - ID_VIEW_FILTER_NONE));
	Invalidate(FALSE);
}

void CMainFrame::OnUpdateViewFilterRange(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_pScreen->GetFilter() == (pCmdUI->m_nID - ID_VIEW_FILTER_NONE));
}

// MMX on/off
void CMainFrame::OnViewMmxEnable() 
{
	CNxDraw* pNxDraw = CNxDraw::GetInstance();
	pNxDraw->EnableMMX(!pNxDraw->IsMMXEnabled());
}

void CMainFrame::OnUpdateViewMmxEnable(CCmdUI* pCmdUI) 
{
	CNxDraw* pNxDraw = CNxDraw::GetInstance();
	pCmdUI->SetCheck(pNxDraw->IsMMXEnabled());
#if defined(NXDRAW_MMX_ONLY)
	pCmdUI->Enable(FALSE);
#else
	pCmdUI->Enable(pNxDraw->IsMMXSupported());
#endif
}

// 3DNow! on/off
void CMainFrame::OnView3dnowEnable() 
{
	CNxDraw* pNxDraw = CNxDraw::GetInstance();
	pNxDraw->Enable3DNow(!pNxDraw->Is3DNowEnabled());
}

void CMainFrame::OnUpdateView3dnowEnable(CCmdUI* pCmdUI) 
{
	CNxDraw* pNxDraw = CNxDraw::GetInstance();
	pCmdUI->SetCheck(pNxDraw->Is3DNowEnabled());
	pCmdUI->Enable(pNxDraw->Is3DNowSupported());
}

// pause/resume
void CMainFrame::OnViewPause() 
{
	m_bPause = !m_bPause;
	Invalidate(FALSE);
}

void CMainFrame::OnUpdateViewPause(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bPause);
}

// 編集->コピー
void CMainFrame::OnEditCopy() 
{
	m_pScreen->SetClipChildren(FALSE);
	if (OpenClipboard())
	{
		CWaitCursor wc;

		CRect rcSrc;
		m_pScreen->GetRect(&rcSrc);
		CNxSurface surfaceTemp;
	    surfaceTemp.Create(m_pScreen->GetWidth(), m_pScreen->GetHeight());

		m_pScreen->DrawSurface(&surfaceTemp, 0, 0, &rcSrc);

		BITMAPINFO bmi;
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = rcSrc.Width();
		bmi.bmiHeader.biHeight = rcSrc.Height();
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biCompression = BI_RGB;
		if (surfaceTemp.GetDIBits(0, 0, &bmi, NULL, &rcSrc))
		{
			HGLOBAL hGlobal = ::GlobalAlloc(GHND | GMEM_DDESHARE, bmi.bmiHeader.biSizeImage + bmi.bmiHeader.biSize);
			if (hGlobal != NULL)
			{
				LPVOID lpvGlobal = ::GlobalLock(hGlobal);
				memcpy(lpvGlobal, &bmi, bmi.bmiHeader.biSize);
				if (surfaceTemp.GetDIBits(0, 0, &bmi, static_cast<LPBYTE>(lpvGlobal) + bmi.bmiHeader.biSize, &rcSrc))
				{
					::GlobalUnlock(hGlobal);
					::EmptyClipboard();
					::SetClipboardData(CF_DIB, hGlobal);
				}
				else
				{
					::GlobalUnlock(hGlobal);
					::GlobalFree(hGlobal);
				}
			}
		}
		::CloseClipboard();
	}
}

// 0.5秒経過後に呼び出される
void CMainFrame::OnTimer(UINT nIDEvent) 
{
	m_bTimer = TRUE;
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
			str.Format(_T("%d FPS"), (int)nFPS / 1000);
		m_pLayerFPS->SetTextSmoothing(m_bTextSmooth);
		m_pLayerFPS->FillRect(NULL, CNxColor(0, 0, 0, 0));
		m_pLayerFPS->DrawText(2, 2, NULL, (LPCTSTR)str, CNxColor(0, 0, 0, 128));
		m_pLayerFPS->DrawText(0, 0, NULL, (LPCTSTR)str, CNxHLSColor(35, 200, 250));
		m_pLayerFPS->SetUpdate();
	}
}

// FPS 表示用サーフェスとスプライトの初期化
void CMainFrame::InitializeFPS()
{
	m_pLayerFPS = new CNxLayerSprite(m_pScreen);
	m_pLayerFPS->Create(300, 60);				// 140 * 24 サイズは適当...
	m_pLayerFPS->SetZPos(50);//INT_MAX);				// 最上位

	// 転送方法を設定
	NxBlt nxbf;
	nxbf.dwFlags = NxBlt::srcAlpha;
	m_pLayerFPS->SetNxBlt(&nxbf);
	
	m_fontFPS = CNxFont(_T("Arial Bold"), -48);
	m_pLayerFPS->SetFont(&m_fontFPS);
}

void CMainFrame::CreateBall()
{
	int nParentWidth  = m_pScreen->GetWidth();		// 親スプライト(背景)の幅
	int nParentHeight = m_pScreen->GetHeight();		//					   高さ

	for (int i = 0; i < BallCount; i++)
	{
		CNxBallSprite* pSprite = new CNxBallSprite(m_pScreen, m_pSurfaceBall);
		// タイル状に敷き詰める (640x480 の場合、個数を 300 以上にするとハングするので注意)
//		pSprite->SetPos((i % (nParentWidth / 32)) * 32, (i / (nParentWidth / 32)) * 32);
		// ランダムで並べる
		pSprite->SetPos(rand() % (nParentWidth - 32), rand() % (nParentHeight - 32));
		pSprite->SetZPos(i);
		m_spriteBall.push_back(pSprite);
	}
	// ボールの表示方法を通常ブレンドとして設定
	SetBallSpriteBlend(BallBlend_normal);
}

void CMainFrame::DestroyBall()
{
	while (!m_spriteBall.empty())
	{
		delete m_spriteBall.back();
		m_spriteBall.pop_back();
	}
}

void CMainFrame::OnViewBall() 
{
	if (m_spriteBall.empty())
		CreateBall();
	else
		DestroyBall();
}

void CMainFrame::OnViewTracking() 
{
	m_pScreen->EnableTracking(!m_pScreen->IsTrackingEnabled());
}

void CMainFrame::OnUpdateViewTracking(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_pScreen->IsTrackingEnabled());
}

void CMainFrame::OnViewBrightnessLight() 
{
	int nBrightness = m_pScreen->GetBrightness() + 4;
	nBrightness = min(nBrightness, 511);
	m_pScreen->SetBrightness(nBrightness);
}

void CMainFrame::OnViewBrightnessDark() 
{
	int nBrightness = m_pScreen->GetBrightness() - 4;
	nBrightness = max(nBrightness, 0);
	m_pScreen->SetBrightness(nBrightness);
}

void CMainFrame::OnUpdateViewBrightnessLight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pScreen->GetBrightness() != 511);
}

void CMainFrame::OnUpdateViewBrightnessDark(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pScreen->GetBrightness() != 0);
}

void CMainFrame::OnViewBrightnessReset() 
{
	m_pScreen->SetBrightness(255);

#if 1
    NxBlt nxb;
//	nxb.dwFlags  = NxBlt::destAlpha;
    nxb.dwFlags  = NxBlt::srcAlpha | NxBlt::destAlpha;
//  nxb.dwFlags  = 0;

    CNxSprite*  pcSprite  = new CNxSprite(m_pScreen);
    {
        RECT  stRect  = {  0,  0,  640,  480  };
        pcSprite->SetRect( &stRect );
        pcSprite->SetPos( 64, 64 );
        pcSprite->SetZPos( 1 );
        pcSprite->SetClipChildren( FALSE );
    }
    CNxSurfaceSprite*  pcBG  = new CNxSurfaceSprite(pcSprite,m_pSurfaceBG);
    {
        RECT  stRect  = {  0,  0,  640,  480  };
        pcBG->SetRect( &stRect );
        pcBG->SetPos( 32, 32 );
        pcBG->SetZPos( 0 );
        pcBG->SetNxBlt( &nxb );
    }
 	CNxSurfaceSprite*  pcBall  = new CNxSurfaceSprite(pcSprite,m_pSurfaceBall);
    {
        RECT  stRect  = {  0,  0,  32,  32  };
        pcBall->SetRect( &stRect );
        pcBall->SetPos( 16, 16 );
        pcBall->SetZPos( 1 );
        pcBall->SetNxBlt( &nxb );
    }

#if 1
    CNxLayerSprite*  pcFamily  = new CNxLayerSprite(pcSprite->GetParent());
    pcFamily->Create( 640, 480 );
    pcFamily->FillRect( NULL, CNxColor(0,0,0,128) );
    pcSprite->DrawSurface( pcFamily, 0, 0, NULL );
    pcFamily->SetPos( 64, 64 );
    pcFamily->SetZPos(65536);// INT_MAX );
    nxb.dwFlags= NxBlt::srcAlpha;
    pcFamily->SetNxBlt( &nxb );
    pcSprite->SetVisible( FALSE );
//    m_pSpriteBG->SetVisible( FALSE );
#endif

#endif
}

void CMainFrame::OnViewFpsSmooth() 
{
	m_bTextSmooth = !m_bTextSmooth;
}

void CMainFrame::OnUpdateViewFpsSmooth(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bTextSmooth);
}
