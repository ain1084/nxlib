// crossfade.h : CROSSFADE アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_CROSSFADE_H__62F36106_EC3A_11D3_87ED_0000E842F190__INCLUDED_)
#define AFX_CROSSFADE_H__62F36106_EC3A_11D3_87ED_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CCrossfadeApp:
// このクラスの動作の定義に関しては crossfade.cpp ファイルを参照してください。
//

class CCrossfadeApp : public CWinApp
{
public:
	CCrossfadeApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CCrossfadeApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CCrossfadeApp)
	afx_msg void OnAppAbout();
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_CROSSFADE_H__62F36106_EC3A_11D3_87ED_0000E842F190__INCLUDED_)
