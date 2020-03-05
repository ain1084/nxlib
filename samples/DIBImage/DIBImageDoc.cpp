// DIBImageDoc.cpp : CDIBImageDoc �N���X�̓���̒�`���s���܂��B
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
// CDIBImageDoc �N���X�̍\�z/����

CDIBImageDoc::CDIBImageDoc()
{
	// TODO: ���̈ʒu�ɂP�x�����Ă΂��\�z�p�̃R�[�h��ǉ����Ă��������B
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

	// �N���b�v�{�[�h����f�[�^���擾
	HGLOBAL hGlobal = ::GetClipboardData(CF_DIB);

	// HGLOBAL ���� CNxDIBImage ���쐬�B�r�b�g�f�[�^�̃������� CNxDIBImage ���m�ہB
	m_pDIBImage = new CNxDIBImage;
	if (!m_pDIBImage->Create(hGlobal))
	{	// �G���[
		delete m_pDIBImage;
		::CloseClipboard();
		return FALSE;
	}

	::CloseClipboard();
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDIBImageDoc �V���A���C�[�[�V����

void CDIBImageDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: ���̈ʒu�ɕۑ��p�̃R�[�h��ǉ����Ă��������B
	}
	else
	{
		// TODO: ���̈ʒu�ɓǂݍ��ݗp�̃R�[�h��ǉ����Ă��������B
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageDoc �N���X�̐f�f

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
// CDIBImageDoc �R�}���h

BOOL CDIBImageDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// �摜�̓ǂݍ���
	m_pDIBImage = CNxDraw::GetInstance()->LoadImage(CNxFile(lpszPathName));
	if (m_pDIBImage == NULL)
	{	// �G���[����
		CString strError;
		AfxFormatString1(strError, IDS_ERR_UNKNOW_FORMAT, lpszPathName);
		AfxMessageBox((LPCTSTR)strError, MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}

	return TRUE;
}
