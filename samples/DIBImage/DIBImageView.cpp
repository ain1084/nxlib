// DIBImageView.cpp : CDIBImageView �N���X�̓���̒�`���s���܂��B
//

#include "stdafx.h"
#include "DIBImage.h"

#include "DIBImageDoc.h"
#include "DIBImageView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView

IMPLEMENT_DYNCREATE(CDIBImageView, CScrollView)

BEGIN_MESSAGE_MAP(CDIBImageView, CScrollView)
	//{{AFX_MSG_MAP(CDIBImageView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	//}}AFX_MSG_MAP
	// �W������R�}���h
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView �N���X�̍\�z/����

CDIBImageView::CDIBImageView()
{
	// TODO: ���̏ꏊ�ɍ\�z�p�̃R�[�h��ǉ����Ă��������B

}

CDIBImageView::~CDIBImageView()
{
}

BOOL CDIBImageView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: ���̈ʒu�� CREATESTRUCT cs ���C������ Window �N���X�܂��̓X�^�C����
	//  �C�����Ă��������B

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView �N���X�̕`��

void CDIBImageView::OnDraw(CDC* pDC)
{
	CDIBImageDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CNxDIBImage* pDIBImage = pDoc->GetDIBImage();
	
	// DIB �̕��ƍ������擾
	int nWidth = pDIBImage->GetInfoHeader()->biWidth;
	int nHeight = pDIBImage->GetInfoHeader()->biHeight;

	::SetDIBitsToDevice(pDC->m_hDC,					// �`��� HDC
						0, 0, nWidth, nHeight,		// destX, destY, dwWidth, dwHeight
						0, 0, 0, nHeight,			// x, y, startscan, scanlines
						pDIBImage->GetDIBits(),		// �r�b�g�f�[�^�ւ̃|�C���^(��)
						pDIBImage->GetInfo(),		// BITMAPINFO �\���̂ւ̃|�C���^
						DIB_RGB_COLORS);

	// �� GetBits() �ł͂Ȃ��_�ɒ��ӂ��ĉ������B
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView �N���X�̈��

BOOL CDIBImageView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// �f�t�H���g�̈������
	return DoPreparePrinting(pInfo);
}

void CDIBImageView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ����O�̓��ʂȏ�����������ǉ����Ă��������B
}

void CDIBImageView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �����̌㏈����ǉ����Ă��������B
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView �N���X�̐f�f

#ifdef _DEBUG
void CDIBImageView::AssertValid() const
{
	CView::AssertValid();
}

void CDIBImageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDIBImageDoc* CDIBImageView::GetDocument() // ��f�o�b�O �o�[�W�����̓C�����C���ł��B
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDIBImageDoc)));
	return (CDIBImageDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView �N���X�̃��b�Z�[�W �n���h��

void CDIBImageView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

	CDIBImageDoc* pDoc = GetDocument();
	CNxDIBImage* pDIBImage = pDoc->GetDIBImage();

	// DIB �̕��ƍ������擾
	int nWidth = pDIBImage->GetInfoHeader()->biWidth;
	int nHeight = pDIBImage->GetInfoHeader()->biHeight;
	
	// �E�B���h�E�Ɖ摜�̃T�C�Y�����킹��
	// �܂��A����(CView = �N���C�A���g�E�B���h�E)�̃T�C�Y�v�Z���Ă���A
	// �X�ɂ�����͂ރt���[��(CChildFrame)�̃T�C�Y���v�Z���Đݒ�
	CRect rcRect(0, 0, nWidth, nHeight);
	::AdjustWindowRectEx(rcRect, GetStyle(), NULL, GetExStyle());
	::AdjustWindowRectEx(rcRect, GetParent()->GetStyle(), NULL, GetParent()->GetExStyle());
	GetParent()->SetWindowPos(NULL, 0, 0, rcRect.Width(), rcRect.Height(), SWP_NOZORDER|SWP_NOMOVE);

	// �X�N���[���T�C�Y�̐ݒ�
	SetScrollSizes(MM_TEXT, CSize(nWidth, nHeight));
}

void CDIBImageView::OnEditCopy() 
{
	CDIBImageDoc* pDoc = GetDocument();
	CNxDIBImage* pDIBImage = pDoc->GetDIBImage();

	if (OpenClipboard())
	{
		CWaitCursor wc;
		
		// BITMAPINFOHEADER �\���̂ƃJ���[�e�[�u���̃T�C�Y���擾
		DWORD dwInfoSize = pDIBImage->GetInfoSize();

		// �r�b�g�f�[�^�̃T�C�Y���擾
		DWORD dwImageSize = pDIBImage->GetImageSize();

		// ���������m��(�G���[�����͏ȗ����Ă��܂�...)
		HGLOBAL hGlobal = ::GlobalAlloc(GHND | GMEM_DDESHARE, dwInfoSize + dwImageSize);
		LPVOID lpvGlobal = ::GlobalLock(hGlobal);

		// BITMAPINFO �\���̂ƃJ���[�e�[�u�����R�s�[
		memcpy(lpvGlobal, pDIBImage->GetInfo(), dwInfoSize);

		// �r�b�g�f�[�^���R�s�[
		LPVOID lpvBits = static_cast<LPBYTE>(lpvGlobal) + dwInfoSize;
		memcpy(lpvBits, pDIBImage->GetDIBits(), dwImageSize);

		// �N���b�v�{�[�h�փf�[�^��ݒ�
		::EmptyClipboard();
		::GlobalUnlock(hGlobal);
		::SetClipboardData(CF_DIB, hGlobal);
		::CloseClipboard();
	}
}
