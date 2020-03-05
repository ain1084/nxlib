// MainFrm.cpp : CMainFrame �N���X�̓���̒�`���s���܂��B
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
// CMainFrame �N���X�̍\�z/����

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
	// TODO: ���̈ʒu�� CREATESTRUCT cs ���C�����āAWindow �N���X��X�^�C����
	//       �C�����Ă��������B
	cs.style &= ~(WS_THICKFRAME|WS_MAXIMIZEBOX);
	cs.dwExStyle = WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE;
	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame �N���X�̐f�f

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
// CMainFrame ���b�Z�[�W �n���h��

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	// �w�i���������Ȃ�
	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// CNxScreen �쐬
	m_pScreen = new CNxBallScreen;
	m_pScreen->EnableAutoHideMenuBar(TRUE);			// ���j���[�o�[�������I�ɉB���l�ɂ���
	if (!m_pScreen->Attach(m_hWnd))
	{
		delete m_pScreen;
		m_pScreen = NULL;
		return -1;
	}

	// �ŏ��̓E�B���h�E���[�h
	m_pScreen->SetScreenMode(1280, 960, FALSE);
//	m_pScreen->SetScreenMode(FALSE);

	// �w�i�T�[�t�F�X���쐬�A�C���[�W���t�@�C������ǂݍ���
	m_pSurfaceBG = new CNxSurface();
	m_pSurfaceBG->Create(CNxFile(_T("backA.jpg")));

	// �^�C���p(����������������)�T�[�t�F�X���C���[�W���琶��
	m_pSurfaceTile = new CNxSurface;
	m_pSurfaceTile->Create(CNxFile(_T("tile.png")));

	// Ball �̃T�[�t�F�X���쐬
	// 6 ���̃T�C�Y�̃T�[�t�F�X���쐬���āA����̈���t�@�C������ǂݍ���
	m_pSurfaceBall = new CNxSurface;
	m_pSurfaceBall->Create(32 * 6, 32);
	m_pSurfaceBall->LoadImage(0, 0, CNxFile(_T("ball.png")));

	// �F���ϊ��ɂ���ĐF�Ⴂ�̃{�[�����쐬
	for (int i = 0; i < 5; i++)
	{
		NxFilterBlt nxf;
		nxf.dwFlags = NxFilterBlt::hueTransform;
		nxf.nxfHueTransform.nHue = (i + 1) * 60;	// �F���� 60�����ω�������
		nxf.nxfHueTransform.nSaturation = 0;		// �ʓx�̕ψڂȂ�
		nxf.nxfHueTransform.nLightness = 0;			// ���x�̕ψڂȂ�
		nxf.nxfHueTransform.bSingleness = FALSE;	// �P��F���ɂ��Ȃ�
		m_pSurfaceBall->FilterBlt((i + 1) * 32, 0, m_pSurfaceBall, CRect(0, 0, 32, 32), &nxf);
	}

	// �w�i��ݒ�
	SetBackground(Background_normal);

	// FPS �\���p�T�[�t�F�X�ƃX�v���C�g�̏�����
	InitializeFPS();

	// �{�[���쐬
	CreateBall();
	
	// �E�B���h�E�\��
	ShowWindow(SW_SHOW);

	// �ҋ@�^�C�}�J�n
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

// CBallApp::OnIdle() ����Ăяo�����
BOOL CMainFrame::OnIdle()
{
	if (!m_bActive)
		return FALSE;		// �A�N�e�B�u�łȂ�
	
	if (!m_bTimer)
		return FALSE;		// �J�n����^�C�}�[���������ĂȂ�
	
	if (m_bPause)
		return FALSE;		// ��~��

	UpdateFPS();

	m_pScreen->Refresh();
	return TRUE;
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // �`��p�̃f�o�C�X �R���e�L�X�g
	m_pScreen->Refresh(dc.m_hDC, TRUE);
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD hTask) 
{
	m_bActive = bActive;
	CFrameWnd::OnActivateApp(bActive, hTask);
}

// Alt+Enter �������ꂽ��...
void CMainFrame::OnScreen() 
{
	m_pScreen->SetScreenMode(!m_pScreen->IsFullScreenMode());
}

// �S�ẴX�v���C�g�̃u�����h���@��ݒ�
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

	// �S�Ă� ball �ɑ΂��� NxBlt ��ݒ�
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
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// �w�i�������Ȃ�
		break;
	case Background_normal:
		m_pSpriteBG = new CNxSurfaceSprite(m_pScreen, m_pSurfaceBG);
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// �w�i�������Ȃ�
		break;
	case Background_tile:
		m_pSpriteBG = new CNxTileSpriteDemo(m_pScreen, m_pSurfaceTile);
		m_pSpriteBG->SetRect(&rcScreen);
		m_pSurfaceTile->GetRect(&rcSrc);
		static_cast<CNxTileSpriteDemo*>(m_pSpriteBG)->SetSrcRect(&rcSrc);
		static_cast<CNxTileSpriteDemo*>(m_pSpriteBG)->SetDirection(-1, -1);
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// �w�i�������Ȃ�
		break;
	case Background_raster:
		m_pSpriteBG = new CNxRasterSprite(m_pScreen, m_pSurfaceBG);
		static_cast<CNxRasterSprite*>(m_pSpriteBG)->SetMaxAmplitude(30);
		static_cast<CNxRasterSprite*>(m_pSpriteBG)->SetStep(4);
		m_pScreen->SetBkColor(CNxColor(CNxColor::black));			// �w�i�F�͍�
		break;
	case Background_stretch:
	case Background_stretch_linear_filter:
		// ���������� 16x16 ����A4x4 �܂Ŋg��
		m_pSpriteBG = new CNxStretchSpriteDemo(m_pScreen, m_pSurfaceBG);
		m_pScreen->SetBkColor(CNxColor().SetAlpha(0));				// �w�i�������Ȃ�
		// �J�n��`
		rcSrc.left = (m_pSpriteBG->GetWidth() - m_pSpriteBG->GetWidth() / 16) / 2;
		rcSrc.top = (m_pSpriteBG->GetHeight() - m_pSpriteBG->GetHeight() / 16) / 2;
		rcSrc.right = rcSrc.left + m_pSpriteBG->GetWidth() / 16;
		rcSrc.bottom = rcSrc.top + m_pSpriteBG->GetHeight() / 16;
		static_cast<CNxStretchSpriteDemo*>(m_pSpriteBG)->SetSrcBeginRect(&rcSrc);
		// �I����`
		rcSrc.left = (m_pSpriteBG->GetWidth() - m_pSpriteBG->GetWidth() / 4) / 2;
		rcSrc.top = (m_pSpriteBG->GetHeight() - m_pSpriteBG->GetHeight() / 4) / 2;
		rcSrc.right = rcSrc.left + m_pSpriteBG->GetWidth() / 4;
		rcSrc.bottom = rcSrc.top + m_pSpriteBG->GetHeight() / 4;
		static_cast<CNxStretchSpriteDemo*>(m_pSpriteBG)->SetSrcEndRect(&rcSrc);
		// �⊮�t���Ȃ�΁ANxBlt::linearFilter ��ݒ�
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

	m_pSpriteBG->SetZPos(INT_MIN);		// Z �͈�ԉ�
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

// �ҏW->�R�s�[
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

// 0.5�b�o�ߌ�ɌĂяo�����
void CMainFrame::OnTimer(UINT nIDEvent) 
{
	m_bTimer = TRUE;
}

// FPS �X�V
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

// FPS �\���p�T�[�t�F�X�ƃX�v���C�g�̏�����
void CMainFrame::InitializeFPS()
{
	m_pLayerFPS = new CNxLayerSprite(m_pScreen);
	m_pLayerFPS->Create(300, 60);				// 140 * 24 �T�C�Y�͓K��...
	m_pLayerFPS->SetZPos(50);//INT_MAX);				// �ŏ��

	// �]�����@��ݒ�
	NxBlt nxbf;
	nxbf.dwFlags = NxBlt::srcAlpha;
	m_pLayerFPS->SetNxBlt(&nxbf);
	
	m_fontFPS = CNxFont(_T("Arial Bold"), -48);
	m_pLayerFPS->SetFont(&m_fontFPS);
}

void CMainFrame::CreateBall()
{
	int nParentWidth  = m_pScreen->GetWidth();		// �e�X�v���C�g(�w�i)�̕�
	int nParentHeight = m_pScreen->GetHeight();		//					   ����

	for (int i = 0; i < BallCount; i++)
	{
		CNxBallSprite* pSprite = new CNxBallSprite(m_pScreen, m_pSurfaceBall);
		// �^�C����ɕ~���l�߂� (640x480 �̏ꍇ�A���� 300 �ȏ�ɂ���ƃn���O����̂Œ���)
//		pSprite->SetPos((i % (nParentWidth / 32)) * 32, (i / (nParentWidth / 32)) * 32);
		// �����_���ŕ��ׂ�
		pSprite->SetPos(rand() % (nParentWidth - 32), rand() % (nParentHeight - 32));
		pSprite->SetZPos(i);
		m_spriteBall.push_back(pSprite);
	}
	// �{�[���̕\�����@��ʏ�u�����h�Ƃ��Đݒ�
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
