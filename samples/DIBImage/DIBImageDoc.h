// DIBImageDoc.h : CDIBImageDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBIMAGEDOC_H__2186E583_0C60_47D2_AD4E_36B5B7B80587__INCLUDED_)
#define AFX_DIBIMAGEDOC_H__2186E583_0C60_47D2_AD4E_36B5B7B80587__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CDIBImageDoc : public CDocument
{
protected: // シリアライズ機能のみから作成します。
	CDIBImageDoc();
	DECLARE_DYNCREATE(CDIBImageDoc)

// アトリビュート
public:

// オペレーション
public:
	CNxDIBImage* GetDIBImage() const;

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDIBImageDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CDIBImageDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CDIBImageDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CNxDIBImage* m_pDIBImage;
};

/////////////////////////////////////////////////////////////////////////////

inline CNxDIBImage* CDIBImageDoc::GetDIBImage() const {
	return m_pDIBImage; }

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_DIBIMAGEDOC_H__2186E583_0C60_47D2_AD4E_36B5B7B80587__INCLUDED_)
