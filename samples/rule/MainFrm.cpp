// MainFrm.cpp : CMainFrame �N���X�̓���̒�`���s���܂��B
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
// CMainFrame �N���X�̍\�z/����

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
	m_pScreen->SetScreenMode(FALSE);			// �f�t�H���g�̓E�B���h�E���[�h
	m_pScreen->SetBkColor(CNxColor(CNxColor::black).SetAlpha(0));	// �w�i�����𖳌���
	m_pScreen->EnableAutoHideMenuBar(TRUE);						// ���j���[�o�[�������I�ɉB��

	// CNxLayerSprite(CNxSprite + CNxSurface �̑��d�p���N���X) �I�u�W�F�N�g���쐬
	m_pSpriteA = new CNxLayerSprite(m_pScreen);
	m_pSpriteA->Create(CNxFile(_T("backA.jpg")));
	m_pSpriteB = new CNxLayerSprite(m_pScreen);
	m_pSpriteB->Create(CNxFile(_T("backB.jpg")));
	
	// �J�n���� SpriteA ����O(����)�ASpriteB �͉�(�s����)�ɂȂ�
	m_pSpriteA->SetZPos(1);
	m_pSpriteB->SetZPos(0);

	// zip ���k�t�@�C�����J��
	CNxFile cf(_T("rules.zip"));
	// CNxZipArchive �֊֘A�t��
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
		CNxSurface* pRuleSurface = new CNxSurface(8);	// ���[���摜�� 8bpp �łȂ��Ă͂Ȃ�܂���
		pRuleSurface->Create(CNxZipFile(&zipArchive, ruleList[i].lpszFileName));

		nxb.dwFlags = NxBlt::blendNormal|NxBlt::rule;	// �ʏ�u�����h&���[���摜�g�p
		nxb.nxbRule.uLevel =  0;
		nxb.nxbRule.uVague = ruleList[i].uVague;
		nxb.nxbRule.ptOffset.x = 0;
		nxb.nxbRule.ptOffset.y = 0;
		nxb.nxbRule.pSurface = pRuleSurface;
		m_rule.push_back(nxb);
	}

	// �ŏ��̃��[���摜���g�p����
	SetRule(m_pSpriteA);

	// FPS �\���X�v���C�g������
	InitializeFPS();

	// �E�B���h�E�\��
	ShowWindow(SW_SHOW);
	return 0;
}


void CMainFrame::SetRule(CNxSurfaceSprite* pSprite)
{
	// NxBlt.nxbRule.pSurface == NULL �̏ꍇ�A
	// �]�����T�[�t�F�X�̃A���t�@�����[���Ƃ��Ďg�p���܂�

	NxBlt nxb;

	// ���[���摜��]�����T�[�t�F�X�̃A���t�@�����֓]��
	nxb.dwFlags = NxBlt::rgbaMask;
	nxb.nxbRGBAMask.byAlpha = 0xff;
	nxb.nxbRGBAMask.byRed = 0x00;
	nxb.nxbRGBAMask.byGreen = 0x00;
	nxb.nxbRGBAMask.byBlue = 0x00;
	pSprite->GetSrcSurface()->Blt(NULL, m_rule[m_nRuleIndex].nxbRule.pSurface, NULL, &nxb);
	
	// NxBlt.nxbRule.pSurface = NULL �ɂ����ANxBlt ��ݒ�
	memcpy(&nxb, &m_rule[m_nRuleIndex], sizeof(NxBlt));
	nxb.nxbRule.pSurface = NULL;
	pSprite->SetNxBlt(&nxb);

	// ���̃��[���摜��
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
	{	// �O�i(SpriteA) �����S�ɕ\��(256)����ď�����(512)��A���̃��[���摜�֐؂�ւ���
#else
	if (nxbf.nxbRule.uLevel >= 256)
	{	// �O�i(SpriteA) �����S�ɕ\�����ꂽ��A�w�i(SpriteB)�Ƃ���ւ���
		m_pSpriteA->SetNxBlt(NULL);			// �]�����@��������(���[���摜�K�p����)
		m_pSpriteA->SetZPos(0);				// SpriteA �� Z��������
		m_pSpriteB->SetZPos(1);				// SPriteB �� Z������O��
		std::swap(m_pSpriteA, m_pSpriteB);	// �|�C���^����ւ�
#endif
		SetRule(m_pSpriteA);				// ��O�ɂȂ����X�v���C�g�փ��[���摜���g�p����l�ɐݒ�
	}
	else
	{
		nxb.nxbRule.uLevel += 4;			// �]����i�s������
		m_pSpriteA->SetNxBlt(&nxb);
	}
	m_pScreen->Refresh();
	return TRUE;
}

BOOL CMainFrame::DestroyWindow() 
{
	// ���[���摜�̍폜
	RuleContainer::iterator it;
	for (it = m_rule.begin(); it != m_rule.end(); it++)
	{
		delete (*it).nxbRule.pSurface;
	}
	
	// m_pScreen ���폜����ƁA
	// ���̎q�X�v���C�g(m_pSpriteA, m_pSpriteB, m_pLayerFPS)���폜����܂�
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
			str.Format(_T("%3.2f FPS"), static_cast<float>(nFPS) / 1000.0);
		else
			str.Format(_T("%5d FPS"), nFPS / 1000);

		// 8bpp �T�[�t�F�X�֕���������
		m_pLayerFPS->FillRect(NULL, 0);
		m_pLayerFPS->DrawText(0, 0, NULL, str, static_cast<const NxBlt*>(NULL));
		m_pLayerFPS->SetUpdate();
	}
}

// FPS �\���p�T�[�t�F�X�ƃX�v���C�g�̏�����
// 8bpp �T�[�t�F�X�����
void CMainFrame::InitializeFPS()
{
	m_pLayerFPS = new CNxLayerSprite(m_pScreen, 8);
	m_pLayerFPS->Create(140, 24);				// 8bpp �ō쐬(�T�C�Y�K��)
	m_pLayerFPS->SetZPos(INT_MAX);				// �ŏ��

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
