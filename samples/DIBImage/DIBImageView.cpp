// DIBImageView.cpp : CDIBImageView クラスの動作の定義を行います。
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
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView クラスの構築/消滅

CDIBImageView::CDIBImageView()
{
	// TODO: この場所に構築用のコードを追加してください。

}

CDIBImageView::~CDIBImageView()
{
}

BOOL CDIBImageView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView クラスの描画

void CDIBImageView::OnDraw(CDC* pDC)
{
	CDIBImageDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CNxDIBImage* pDIBImage = pDoc->GetDIBImage();
	
	// DIB の幅と高さを取得
	int nWidth = pDIBImage->GetInfoHeader()->biWidth;
	int nHeight = pDIBImage->GetInfoHeader()->biHeight;

	::SetDIBitsToDevice(pDC->m_hDC,					// 描画先 HDC
						0, 0, nWidth, nHeight,		// destX, destY, dwWidth, dwHeight
						0, 0, 0, nHeight,			// x, y, startscan, scanlines
						pDIBImage->GetDIBits(),		// ビットデータへのポインタ(※)
						pDIBImage->GetInfo(),		// BITMAPINFO 構造体へのポインタ
						DIB_RGB_COLORS);

	// ※ GetBits() ではない点に注意して下さい。
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView クラスの印刷

BOOL CDIBImageView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// デフォルトの印刷準備
	return DoPreparePrinting(pInfo);
}

void CDIBImageView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CDIBImageView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView クラスの診断

#ifdef _DEBUG
void CDIBImageView::AssertValid() const
{
	CView::AssertValid();
}

void CDIBImageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDIBImageDoc* CDIBImageView::GetDocument() // 非デバッグ バージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDIBImageDoc)));
	return (CDIBImageDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDIBImageView クラスのメッセージ ハンドラ

void CDIBImageView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

	CDIBImageDoc* pDoc = GetDocument();
	CNxDIBImage* pDIBImage = pDoc->GetDIBImage();

	// DIB の幅と高さを取得
	int nWidth = pDIBImage->GetInfoHeader()->biWidth;
	int nHeight = pDIBImage->GetInfoHeader()->biHeight;
	
	// ウィンドウと画像のサイズを合わせる
	// まず、自分(CView = クライアントウィンドウ)のサイズ計算してから、
	// 更にそれを囲むフレーム(CChildFrame)のサイズを計算して設定
	CRect rcRect(0, 0, nWidth, nHeight);
	::AdjustWindowRectEx(rcRect, GetStyle(), NULL, GetExStyle());
	::AdjustWindowRectEx(rcRect, GetParent()->GetStyle(), NULL, GetParent()->GetExStyle());
	GetParent()->SetWindowPos(NULL, 0, 0, rcRect.Width(), rcRect.Height(), SWP_NOZORDER|SWP_NOMOVE);

	// スクロールサイズの設定
	SetScrollSizes(MM_TEXT, CSize(nWidth, nHeight));
}

void CDIBImageView::OnEditCopy() 
{
	CDIBImageDoc* pDoc = GetDocument();
	CNxDIBImage* pDIBImage = pDoc->GetDIBImage();

	if (OpenClipboard())
	{
		CWaitCursor wc;
		
		// BITMAPINFOHEADER 構造体とカラーテーブルのサイズを取得
		DWORD dwInfoSize = pDIBImage->GetInfoSize();

		// ビットデータのサイズを取得
		DWORD dwImageSize = pDIBImage->GetImageSize();

		// メモリを確保(エラー処理は省略しています...)
		HGLOBAL hGlobal = ::GlobalAlloc(GHND | GMEM_DDESHARE, dwInfoSize + dwImageSize);
		LPVOID lpvGlobal = ::GlobalLock(hGlobal);

		// BITMAPINFO 構造体とカラーテーブルをコピー
		memcpy(lpvGlobal, pDIBImage->GetInfo(), dwInfoSize);

		// ビットデータをコピー
		LPVOID lpvBits = static_cast<LPBYTE>(lpvGlobal) + dwInfoSize;
		memcpy(lpvBits, pDIBImage->GetDIBits(), dwImageSize);

		// クリップボードへデータを設定
		::EmptyClipboard();
		::GlobalUnlock(hGlobal);
		::SetClipboardData(CF_DIB, hGlobal);
		::CloseClipboard();
	}
}
