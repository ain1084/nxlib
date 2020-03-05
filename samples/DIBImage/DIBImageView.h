// DIBImageView.h : CDIBImageView クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBIMAGEVIEW_H__2B5140F2_DFBB_4105_82B6_45C262160F96__INCLUDED_)
#define AFX_DIBIMAGEVIEW_H__2B5140F2_DFBB_4105_82B6_45C262160F96__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDIBImageView : public CScrollView
{
protected: // シリアライズ機能のみから作成します。
	CDIBImageView();
	DECLARE_DYNCREATE(CDIBImageView)

// アトリビュート
public:
	CDIBImageDoc* GetDocument();

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDIBImageView)
	public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画する際にオーバーライドされます。
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CDIBImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CDIBImageView)
	afx_msg void OnEditCopy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // DIBImageView.cpp ファイルがデバッグ環境の時使用されます。
inline CDIBImageDoc* CDIBImageView::GetDocument()
   { return (CDIBImageDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_DIBIMAGEVIEW_H__2B5140F2_DFBB_4105_82B6_45C262160F96__INCLUDED_)
