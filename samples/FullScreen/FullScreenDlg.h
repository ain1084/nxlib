// FullScreenDlg.h : ヘッダー ファイル
//

#if !defined(AFX_FULLSCREENDLG_H__D05A0DA6_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_FULLSCREENDLG_H__D05A0DA6_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "NxFPSSprite.h"

/////////////////////////////////////////////////////////////////////////////
// CFullScreenDlg dialog

class CFullScreenDlg : public CDialog
{
// 構築
public:
	CFullScreenDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// Dialog Data
	//{{AFX_DATA(CFullScreenDlg)
	enum { IDD = IDD_FULLSCREEN_DIALOG };
		// メモ: この位置に ClassWizard によってデータ メンバが追加されます。
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CFullScreenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CFullScreenDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateSurface();
	CNxScreen* m_pScreen;
	CNxSurface* m_pSurface;
	CNxSurfaceSprite* m_pSprite;
	CNxFPSSprite* m_pSpriteFPS;
	int m_nHue;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FULLSCREENDLG_H__D05A0DA6_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
