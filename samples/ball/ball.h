// ball.h : BALL アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_BALL_H__123605F5_E9FC_11D3_87E8_0000E842F190__INCLUDED_)
#define AFX_BALL_H__123605F5_E9FC_11D3_87E8_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CBallApp:
// このクラスの動作の定義に関しては ball.cpp ファイルを参照してください。
//

class CBallApp : public CWinApp
{
public:
	CBallApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CBallApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CBallApp)
	afx_msg void OnAppAbout();
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_BALL_H__123605F5_E9FC_11D3_87E8_0000E842F190__INCLUDED_)
