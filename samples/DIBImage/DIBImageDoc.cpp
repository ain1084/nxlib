// DIBImageDoc.cpp : CDIBImageDoc クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "DIBImage.h"

#include "DIBImageDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDIBImageDoc

IMPLEMENT_DYNCREATE(CDIBImageDoc, CDocument)

BEGIN_MESSAGE_MAP(CDIBImageDoc, CDocument)
	//{{AFX_MSG_MAP(CDIBImageDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDIBImageDoc クラスの構築/消滅

CDIBImageDoc::CDIBImageDoc()
{
	// TODO: この位置に１度だけ呼ばれる構築用のコードを追加してください。
	m_pDIBImage = NULL;
}

CDIBImageDoc::~CDIBImageDoc()
{
	delete m_pDIBImage;
}

BOOL CDIBImageDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CWnd* pWndMain = AfxGetMainWnd();
	if (!pWndMain->OpenClipboard())
		return FALSE;

	CWaitCursor wc;

	// クリップボードからデータを取得
	HGLOBAL hGlobal = ::GetClipboardData(CF_DIB);

	// HGLOBAL から CNxDIBImage を作成。ビットデータのメモリは CNxDIBImage が確保。
	m_pDIBImage = new CNxDIBImage;
	if (!m_pDIBImage->Create(hGlobal))
	{	// エラー
		delete m_pDIBImage;
		::CloseClipboard();
		return FALSE;
	}

	::CloseClipboard();
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDIBImageDoc シリアライゼーション

void CDIBImageDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: この位置に保存用のコードを追加してください。
	}
	else
	{
		// TODO: この位置に読み込み用のコードを追加してください。
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageDoc クラスの診断

#ifdef _DEBUG
void CDIBImageDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDIBImageDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDIBImageDoc コマンド

BOOL CDIBImageDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// 画像の読み込み
	m_pDIBImage = CNxDraw::GetInstance()->LoadImage(CNxFile(lpszPathName));
	if (m_pDIBImage == NULL)
	{	// エラー発生
		CString strError;
		AfxFormatString1(strError, IDS_ERR_UNKNOW_FORMAT, lpszPathName);
		AfxMessageBox((LPCTSTR)strError, MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}

	return TRUE;
}
