// FullScreenDlg.cpp : インプリメンテーション ファイル
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
// アプリケーションのバージョン情報で使われている CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
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
		// メッセージ ハンドラがありません。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFullScreenDlg ダイアログ

CFullScreenDlg::CFullScreenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFullScreenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFullScreenDlg)
		// メモ: この位置に ClassWizard によってメンバの初期化が追加されます。
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
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
		// メモ: この場所には ClassWizard によって DDX と DDV の呼び出しが追加されます。
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
// CFullScreenDlg メッセージ ハンドラ

BOOL CFullScreenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニュー項目をシステム メニューへ追加します。

	// IDM_ABOUTBOX はコマンド メニューの範囲でなければなりません。
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

	// このダイアログ用のアイコンを設定します。フレームワークはアプリケーションのメイン
	// ウィンドウがダイアログでない時は自動的に設定しません。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンを設定
	
	// TODO: 特別な初期化を行う時はこの場所に追加してください。
	
	m_pScreen = new CNxScreen;
	m_pScreen->Attach(m_hWnd);
	m_pScreen->SetScreenMode(TRUE);

	// サーフェスの作成
	m_pSurface = new CNxSurface;
	m_pSurface->Create(2, 2);
	
	// 伸縮スプライトの作成
	m_pSprite = new CNxStretchSprite(m_pScreen, m_pSurface);

	// 転送元矩形の設定
	RECT rcSrc;
	::SetRect(&rcSrc, 0, 0, 2, 2);
	static_cast<CNxStretchSprite*>(m_pSprite)->SetSrcRect(&rcSrc);

	// 転送先矩形(実スプライトサイズ)の設定
	m_pSprite->SetSize(m_pScreen->GetWidth(), m_pScreen->GetHeight());

	// バイリニアフィルタを使用
	NxBlt nxb;
	nxb.dwFlags = NxBlt::linearFilter;
	m_pSprite->SetNxBlt(&nxb);

	// FPS 表示用スプライトの作成
	m_pSpriteFPS = new CNxFPSSprite(m_pScreen);
	
	UpdateSurface();

	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
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

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void CFullScreenDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
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

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
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
	// GetBits() 関数でサーフェス座標(0,0) へのポインタを得て、直接描画
	// GetPitch() 関数から反される値は、サーフェスの物理的な幅を示す
	// つまり、サーフェス上の (x, y) は、static_cast<LPBYTE>(GetBits()) + GetPitch() * y + x で計算できる
	LPBYTE pbBit = static_cast<LPBYTE>(m_pSurface->GetBits());
	LONG lPitch = m_pSurface->GetPitch();
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 0 + 0) = CNxHLSColor((m_nHue +   0) % 360, 128, 255);	// 左上
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 0 + 4) = CNxHLSColor((m_nHue +  90) % 360, 128, 255);	// 右上
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 1 + 4) = CNxHLSColor((m_nHue + 180) % 360, 128, 255);	// 右下
	*reinterpret_cast<LPDWORD>(pbBit + lPitch * 1 + 0) = CNxHLSColor((m_nHue + 270) % 360, 128, 255);	// 左下
	if (--m_nHue < 0)
		m_nHue += 360;

	m_pSprite->SetUpdate();
}

BOOL CFullScreenDlg::OnEraseBkgnd(CDC* pDC) 
{
	// 背景消去を行わない
	return TRUE;
}
