// crossfade.cpp : アプリケーション用クラスの機能定義を行います。
//

#include "stdafx.h"
#include "crossfade.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCrossfadeApp

BEGIN_MESSAGE_MAP(CCrossfadeApp, CWinApp)
	//{{AFX_MSG_MAP(CCrossfadeApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//    この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG_MAP
	// 標準のファイル基本ドキュメント コマンド
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrossfadeApp クラスの構築

CCrossfadeApp::CCrossfadeApp()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CCrossfadeApp オブジェクト

CCrossfadeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCrossfadeApp クラスの初期化

BOOL CCrossfadeApp::InitInstance()
{
	m_pMainWnd = (CWnd*)RUNTIME_CLASS(CMainFrame)->CreateObject();
	return ((CFrameWnd*)m_pMainWnd)->LoadFrame(IDR_MAINFRAME);
}

/////////////////////////////////////////////////////////////////////////////
// アプリケーションのバージョン情報で使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard 仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // // DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	//{{AFX_MSG(CAboutDlg)
		// メッセージ ハンドラはありません。
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
		// メッセージ ハンドラはありません。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ダイアログを実行するためのアプリケーション コマンド
void CCrossfadeApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CCrossfadeApp コマンド

int CCrossfadeApp::ExitInstance() 
{
	return CWinApp::ExitInstance();
}

BOOL CCrossfadeApp::OnIdle(LONG lCount) 
{
	if (CWinApp::OnIdle(lCount))
		return TRUE;

	return ((CMainFrame*)AfxGetMainWnd())->OnIdle();
}
