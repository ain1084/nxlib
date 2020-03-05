// MainFrm.cpp : CMainFrame �N���X�̓���̒�`���s���܂��B
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
// CMainFrame �N���X�̍\�z/����

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

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// CNxScreen �쐬
	m_pScreen = new CNxScreen;
	if (!m_pScreen->Attach(m_hWnd))
	{
		delete m_pScreen;
		m_pScreen = NULL;
		return -1;
	}
	m_pScreen->SetScreenMode(FALSE);	// �E�B���h�E���[�h
	m_pScreen->SetBkColor(CNxColor(CNxColor::black).SetAlpha(0));	// �w�i�����𖳌���
	m_pScreen->EnableAutoHideMenuBar(TRUE);		// ���j���[�o�[�������I�ɉB��

	// CNxLayerSprite(CNxSprite + CNxSurface �̑��d�p���N���X) �I�u�W�F�N�g���쐬
	m_pSpriteA = new CNxLayerSprite(m_pScreen);
	m_pSpriteA->Create(CNxFile(_T("backA.jpg")));
	m_pSpriteB = new CNxLayerSprite(m_pScreen);
	m_pSpriteB->Create(CNxFile(_T("backB.jpg")));
	
	// �J�n���� SpriteA ����O(����)�ASpriteB �͉�(�s����)�ɂȂ�
	m_pSpriteA->SetZPos(1);
	m_pSpriteB->SetZPos(0);

	NxBlt nxb;
	// ����: blendNormal �ȊO�ł͐������\���ł��܂���
	nxb.dwFlags = NxBlt::blendNormal|NxBlt::opacity;

	// SpriteA �͓���
	nxb.uOpacity = 0;
	m_pSpriteA->SetNxBlt(&nxb);

	// SpriteB �͕s����
	nxb.uOpacity = 255;
	m_pSpriteB->SetNxBlt(&nxb);

	// FPS �\���X�v���C�g������
	InitializeFPS();

	// �E�B���h�E�\��
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
	{	// �O�i(SpriteA) �����S�s�����ɂȂ�����A�w�i(SpriteB)�Ƃ���ւ���
		m_pSpriteA->SetZPos(0);		// SpriteA �� Z��������
		m_pSpriteB->SetZPos(1);		// SPriteB �� Z������O��
		std::swap(m_pSpriteA, m_pSpriteB);	// �|�C���^����ւ�
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
	// m_pScreen ���폜����ƁA
	// ���̎q�X�v���C�g(m_pSpriteA, m_pSpriteB, m_pLayerFPS)���폜����܂�
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
	CPaintDC dc(this); // �`��p�̃f�o�C�X �R���e�L�X�g
	m_pScreen->Refresh(dc.m_hDC, TRUE);
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
			str.Format(_T("%5d FPS"), (int)nFPS / 1000);

		m_pLayerFPS->FillRect(NULL, 0);
		m_pLayerFPS->DrawText(0, 0, NULL, (LPCTSTR)str, (const NxBlt*)NULL);
		m_pLayerFPS->SetUpdate();
	}
}

// FPS �\���p�T�[�t�F�X�ƃX�v���C�g�̏�����
// 8bpp �T�[�t�F�X�����
void CMainFrame::InitializeFPS()
{
	m_pLayerFPS = new CNxLayerSprite(m_pScreen, 8);	// 8bpp �ō쐬
	m_pLayerFPS->Create(220, 24);			// �T�C�Y�K��
	m_pLayerFPS->SetZPos(INT_MAX);			// �ŏ��

	NxBlt nxb;
	nxb.dwFlags = NxBlt::colorFill | NxBlt::blendNormal | NxBlt::srcAlpha;
	nxb.nxbColor = CNxColor(0, 255, 255);
	m_pLayerFPS->SetNxBlt(&nxb);
	
	// �X���[�W���O���s��
	m_pLayerFPS->SetTextSmoothing(TRUE);

	// �t�H���g���쐬���Đݒ�
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
	
