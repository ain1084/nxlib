// FullScreenDlg.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"
#include "FullScreen.h"
#include "FullScreenDlg.h"
#include "NxStretchSprite.h"
#include <NxDraw/NxScreen.h>
#include <NxDraw/NxHLSColor.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// �A�v���P�[�V�����̃o�[�W�������Ŏg���Ă��� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂�
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �̃T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// ���b�Z�[�W �n���h��������܂���B
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFullScreenDlg �_�C�A���O

CFullScreenDlg::CFullScreenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFullScreenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFullScreenDlg)
		// ����: ���̈ʒu�� ClassWizard �ɂ���ă����o�̏��������ǉ�����܂��B
	//}}AFX_DATA_INIT
	// ����: LoadIcon �� Win32 �� DestroyIcon �̃T�u�V�[�P���X��v�����܂���B
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pScreen = NULL;
	m_pSprite = NULL;
	m_pSurface= NULL;
	m_pSpriteFPS = NULL;
	m_nHue = 0;
}

void CFullScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFullScreenDlg)
		// ����: ���̏ꏊ�ɂ� ClassWizard �ɂ���� DDX �� DDV �̌Ăяo�����ǉ�����܂��B
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFullScreenDlg, CDialog)
	//{{AFX_MSG_MAP(CFullScreenDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFullScreenDlg ���b�Z�[�W �n���h��

BOOL CFullScreenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "�o�[�W�������..." ���j���[���ڂ��V�X�e�� ���j���[�֒ǉ����܂��B

	// IDM_ABOUTBOX �̓R�}���h ���j���[�͈̔͂łȂ���΂Ȃ�܂���B
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���̃_�C�A���O�p�̃A�C�R����ݒ肵�܂��B�t���[�����[�N�̓A�v���P�[�V�����̃��C��
	// �E�B���h�E���_�C�A���O�łȂ����͎����I�ɐݒ肵�܂���B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R����ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R����ݒ�
	
	// TODO: ���ʂȏ��������s�����͂��̏ꏊ�ɒǉ����Ă��������B
	
	m_pScreen = new CNxScreen;
	m_pScreen->Attach(m_hWnd);
	m_pScreen->SetScreenMode(TRUE);

	// �T�[�t�F�X�̍쐬
	m_pSurface = new CNxSurface;
	m_pSurface->Create(2, 2);
	
	// �L�k�X�v���C�g�̍쐬
	m_pSprite = new CNxStretchSprite(m_pScreen, m_pSurface);

	// �]������`�̐ݒ�
	RECT rcSrc;
	::SetRect(&rcSrc, 0, 0, 2, 2);
	static_cast<CNxStretchSprite*>(m_pSprite)->SetSrcRect(&rcSrc);

	// �]�����`(���X�v���C�g�T�C�Y)�̐ݒ�
	m_pSprite->SetSize(m_pScreen->GetWidth(), m_pScreen->GetHeight());

	// �o�C���j�A�t�B���^���g�p
	NxBlt nxb;
	nxb.dwFlags = NxBlt::linearFilter;
	m_pSprite->SetNxBlt(&nxb);

	// FPS �\���p�X�v���C�g�̍쐬
	m_pSpriteFPS = new CNxFPSSprite(m_pScreen);
	
	UpdateSurface();

	return TRUE;  // TRUE ��Ԃ��ƃR���g���[���ɐݒ肵���t�H�[�J�X�͎����܂���B
}

void CFullScreenDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����_�C�A���O�{�b�N�X�ɍŏ����{�^����ǉ�����Ȃ�΁A�A�C�R����`�悷��
// �R�[�h���ȉ��ɋL�q����K�v������܂��BMFC �A�v���P�[�V������ document/view
// ���f�����g���Ă���̂ŁA���̏����̓t���[�����[�N�ɂ�莩���I�ɏ�������܂��B

void CFullScreenDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��p�̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// �N���C�A���g�̋�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R����`�悵�܂��B
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		m_pScreen->Refresh(dc.m_hDC);
		UpdateSurface();
		Invalidate(FALSE);
	}
}

// �V�X�e���́A���[�U�[���ŏ����E�B���h�E���h���b�O���Ă���ԁA
// �J�[�\����\�����邽�߂ɂ������Ăяo���܂��B
HCURSOR CFullScreenDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFullScreenDlg::OnDestroy() 
{
	delete m_pSurface;
	delete m_pScreen;
	CDialog::OnDestroy();
}

void CFullScreenDlg::UpdateSurface()
{
	// GetBits() �֐��ŃT�[�t�F�X���W(0,0) �ւ̃|�C���^�𓾂āA���ڕ`��
	// GetPitch() �֐����甽�����l�́A�T�[�t�F�X�̕����I�ȕ�������
	// �܂�A�T�[�t�F�X��� (x, y) �́Astatic_cast<LPBYTE>(GetBits()) + GetPitch() * y + x �Ōv�Z�ł���
	LPBYTE pbBit = static_cast<LPBYTE>(m_pSurface->GetBits());
	LONG lPitch = m_pSurface->GetPitch();
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 0 + 0) = CNxHLSColor((m_nHue +   0) % 360, 128, 255);	// ����
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 0 + 4) = CNxHLSColor((m_nHue +  90) % 360, 128, 255);	// �E��
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 1 + 4) = CNxHLSColor((m_nHue + 180) % 360, 128, 255);	// �E��
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 1 + 0) = CNxHLSColor((m_nHue + 270) % 360, 128, 255);	// ����
	if (--m_nHue < 0)
		m_nHue += 360;

	m_pSprite->SetUpdate();
}

BOOL CFullScreenDlg::OnEraseBkgnd(CDC* pDC) 
{
	// �w�i�������s��Ȃ�
	return TRUE;
}
