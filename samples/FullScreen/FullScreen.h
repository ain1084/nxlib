// FullScreen.h : FULLSCREEN アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_FULLSCREEN_H__D05A0DA4_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_FULLSCREEN_H__D05A0DA4_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CFullScreenApp:
// このクラスの動作の定義に関しては FullScreen.cpp ファイルを参照してください。
//

class CFullScreenApp : public CWinApp
{
public:
	CFullScreenApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CFullScreenApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CFullScreenApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FULLSCREEN_H__D05A0DA4_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
