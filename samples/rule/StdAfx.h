// stdafx.h : 標準のシステム インクルード ファイル、
//            または参照回数が多く、かつあまり変更されない
//            プロジェクト専用のインクルード ファイルを記述します。
//

#if !defined(AFX_STDAFX_H__0003F247_172A_11D4_880F_0000E842F190__INCLUDED_)
#define AFX_STDAFX_H__0003F247_172A_11D4_880F_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Windows ヘッダーから殆ど使用されないスタッフを除外します。

#include <afxwin.h>         // MFC のコアおよび標準コンポーネント
#include <afxext.h>         // MFC の拡張部分
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC の Windows コモン コントロール サポート
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <NxDraw/NxScreen.h>
#include <NxDraw/NxLayerSprite.h>
#include <NxDraw/NxHLSColor.h>
#include <NxStorage/NxFile.h>
#include <NxStorage/NxZipArchive.h>
#include <NxStorage/NxZipFile.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_STDAFX_H__0003F247_172A_11D4_880F_0000E842F190__INCLUDED_)
