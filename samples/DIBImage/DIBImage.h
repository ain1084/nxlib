// DIBImage.h : DIBIMAGE アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_DIBIMAGE_H__9DB620B6_9617_4CFA_A7E5_98596CFE1D3D__INCLUDED_)
#define AFX_DIBIMAGE_H__9DB620B6_9617_4CFA_A7E5_98596CFE1D3D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CDIBImageApp:
// このクラスの動作の定義に関しては DIBImage.cpp ファイルを参照してください。
//

class CDIBImageApp : public CWinApp
{
public:
	CDIBImageApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDIBImageApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CDIBImageApp)
	afx_msg void OnAppAbout();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_DIBIMAGE_H__9DB620B6_9617_4CFA_A7E5_98596CFE1D3D__INCLUDED_)
