// NxMMIOProc.cpp: CNxMMIOProc クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: mmio のカスタムプロシージャ用抽象クラス
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxMMIOProc.h"

using namespace NxStorageLocal;

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxMMIOProc::CNxMMIOProc()
{

}

CNxMMIOProc::~CNxMMIOProc()
{

}


////////////////////////////////////////////////////////////////////////////
// protected:
//	CNxMMIOProc::Create(LPCTSTR lpszFileName, DWORD dwFlags)
// 概要: カスタム入出力プロシージャを使用する様に mmio をオープン
// 引数: LPCTSTR lpszFileName ... ファイル名へのポインタ
//       DWORD dwFlags        ... mmioOpen へ渡すフラグ(MMIO_READ 等)
// 戻値: 成功なら mmio へのハンドル。それ以外は NULL
////////////////////////////////////////////////////////////////////////////

HMMIO CNxMMIOProc::Create(LPCTSTR lpszFileName, DWORD dwFlags, DWORD dwParam1, DWORD dwParam2)
{
	MMIOINFO mmioinfo;
	memset(&mmioinfo, 0, sizeof(mmioinfo));
	mmioinfo.pIOProc = MMIOProc;
	mmioinfo.adwInfo[0] = reinterpret_cast<DWORD>(this);
	mmioinfo.adwInfo[1] = dwParam1;
	mmioinfo.adwInfo[2] = dwParam2;

	// なぜか UNICODE だとアクセス違反になるので...
	if (lpszFileName == NULL)
		lpszFileName = _T("");
	
	return ::mmioOpen(const_cast<LPTSTR>(lpszFileName), &mmioinfo, dwFlags);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static LRESULT CNxMMIOProc::MMIOProc(LPSTR lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2)
// 概要: mmio のカスタム入出力プロシージャ
// 引数: LPSTR lpmmioinfo ... MMIOINFO 構造体へのポインタ(cast が必要)
//       UINT  uMsg       ... メッセージ
//       LONG lParam1     ... 引数1(意味はメッセージにより異なる)
//       LONG lParam2     ... 引数2(意味はメッセードにより異なる)
// 戻値: 失敗ならば - 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxMMIOProc::MMIOProc(LPSTR lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2)
{
	LPMMIOINFO lpmi = reinterpret_cast<LPMMIOINFO>(lpmmioinfo);
	
	CNxMMIOProc* This = reinterpret_cast<CNxMMIOProc*>(lpmi->adwInfo[0]);
	LRESULT lResult = 0;
	switch (uMsg)
	{
	case MMIOM_OPEN:
		lResult = This->Open(lpmi, reinterpret_cast<LPCSTR>(lParam1));// なぜ LPCTSTR でないのか...?
		break;
	case MMIOM_CLOSE:
		lResult = This->Close(lpmi, static_cast<UINT>(lParam1));
		break;
	case MMIOM_READ:
		lResult = This->Read(lpmi, reinterpret_cast<LPVOID>(lParam1), lParam2);
		break;
	case MMIOM_WRITE:
		lResult = This->Write(lpmi, reinterpret_cast<const VOID*>(lParam1), lParam2);
		break;
	case MMIOM_WRITEFLUSH:
		lResult = This->WriteFlush(lpmi, reinterpret_cast<const VOID*>(lParam1), lParam2);
		break;
	case MMIOM_SEEK:
		lResult = This->Seek(lpmi, lParam1, lParam2);
		break;
	default:
		lResult = This->User(lpmi, uMsg, lParam1, lParam2);
	}
	return lResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual LRESULT CNxMMIOProc::User(LPMMIOINFO lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2)
// 概要: mmioSendMessage 関数によって送られる、ユーザ定義メッセージの応答関数
// 引数: LPMMIOINFO lpmmioinfo ... MMIOINFO 構造体へのポインタ
//       UINT uMsg             ... メッセージ
//       LONG lParam1          ... パラメータ1
//       LONG lParam2          ... パラメータ2
// 戻値: 認識しないメッセージならば 0
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxMMIOProc::User(LPMMIOINFO /*lpmmioinfo*/, UINT /*uMsg*/, LONG /*lParam1*/, LONG /*lParam2*/)
{
	return 0;
}
