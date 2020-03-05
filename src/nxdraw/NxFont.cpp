// NxFont.cpp: CNxFont クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//////////////////////////////////////////////////////////////////////

#include <nxdraw.h>
#include "NxFont.h"
#include "NxSurface.h"


//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont()
// 概要: CNxFont クラスのデフォルトコンストラクタ
// 引数: なし
// 備考: フォント名 "Arial", サイズを 10 で初期化
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont()
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
{
	memset(&m_lf, 0, sizeof(LOGFONT));
	m_lf.lfHeight = 10;
	m_lf.lfCharSet = DEFAULT_CHARSET;
	::lstrcpyn(m_lf.lfFaceName, _T("Arial"), LF_FACESIZE);
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont(LPCTSTR lpszFaceName, LONG lSize)
// 概要: CNxFont クラスのコンストラクタ
//       フォント名とサイズを指定して初期化
// 引数: LPCTSTR lpszFaceName ... フォントの名前
//       LONG lSize           ... フォントのサイズ
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont(LPCTSTR lpszFaceName, LONG lSize)
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
{
	memset(&m_lf, 0, sizeof(LOGFONT));
	m_lf.lfHeight = lSize;
	m_lf.lfCharSet = DEFAULT_CHARSET;
	::lstrcpyn(m_lf.lfFaceName, lpszFaceName, LF_FACESIZE);
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont(const LONGFONT* lpLogFont)
// 概要: CNxFont クラスのコンストラクタ
//       LOGFONT 構造体をコピーして初期化
// 引数: const LOGFONT* lpLogFont ... コピー元 LOGFONT 構造体
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont(const LOGFONT* lpLogFont)
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
 , m_lf(*lpLogFont)
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::CNxFont(const CNxFont& font)
// 概要: CNxFont クラスのコピーコンストラクタ
//////////////////////////////////////////////////////////////////////

CNxFont::CNxFont(const CNxFont& font)
 : m_hFont(NULL)
 , m_hFontSmooth(NULL)
 , m_bDirty(FALSE)
 , m_lf(font.m_lf)
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont& CNxFont::operator=(const CNxFont& font)
// 概要: CNxFont クラスの代入演算子
//////////////////////////////////////////////////////////////////////

CNxFont& CNxFont::operator=(const CNxFont& font)
{
	if (this != &font)
	{
		m_lf = font.m_lf;
		m_bDirty = TRUE;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxFont::~CNxFont()
// 概要: CNxFont クラスのデストラクタ
// 備考: 作成したフォントは削除される
//////////////////////////////////////////////////////////////////////

CNxFont::~CNxFont()
{
	deleteFont();
}

//////////////////////////////////////////////////////////////////////
// private:
//	void CNxFont::deleteFont()
// 概要: 作成したフォントを削除
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

void CNxFont::deleteFont()
{
#if _MSC_VER >= 1200
#pragma warning (disable : 4390)
#endif
	if (m_hFont != NULL)
	{
		if (!::DeleteObject(m_hFont))
		{
			_RPTF0(_CRT_WARN, "CNxFont : フォントの削除に失敗しました。フォントがデバイスコンテキストへ選択されている可能性があります.\n");
		}
		m_hFont = NULL;
	}
	if (m_hFontSmooth != NULL)
	{
		if (!::DeleteObject(m_hFontSmooth))
		{
			_RPTF0(_CRT_WARN, "CNxFont : フォントの削除に失敗しました。フォントがデバイスコンテキストへ選択されている可能性があります.\n");
		}
		m_hFontSmooth = NULL;
	}
#if _MSC_VER >= 1200
#pragma warning (default : 4390)
#endif
	m_bDirty = FALSE;
}

//////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFont::Attach(HFONT hFont)
// 概要: HFONT を結び付ける
// 引数: HFONT hFont ... フォントへのハンドル
// 戻値: 成功なら TRUE
// 備考: 結び付けられたフォントの所有権は CNxFont へ移ります。
//////////////////////////////////////////////////////////////////////

BOOL CNxFont::Attach(HFONT hFont)
{
	// 現在のフォントを削除
	deleteFont();

	// HFONT から LOGFONT を取得
	if (::GetObject(hFont, sizeof(LOGFONT), &m_lf) == 0)
	{
		_RPTF0(_CRT_ASSERT, "CNxFont::Attach() : ::GetObject() に失敗しました.\n");
		return FALSE;
	}
	m_hFont = hFont;
	m_bDirty = FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// public:
//	HFONT CNxFont::Detach()
// 概要: CNxFont から HFONT を切り離して返す
// 引数: なし
// 戻値: 成功ならフォントのハンドル
// 備考: フォントが作成されていなければ、作成して返す。
//       返されたハンドルが不要になったら ::DeleteObject() で削除して下さい
//////////////////////////////////////////////////////////////////////

HFONT CNxFont::Detach()
{
	HFONT hFont = GetHandle();
	m_hFont = NULL;					// HFONT 切り離し
	return hFont;
}

//////////////////////////////////////////////////////////////////////
// public:
//	HFONT CNxFont::GetHandleInternal(BOOL bForSmooth)
// 概要: HFONT を作成して返す
// 引数: ---
// 戻値: 成功ならフォントのハンドル
// 備考: 内部用関数です。直接使用しないでください
//////////////////////////////////////////////////////////////////////

HFONT CNxFont::GetHandleInternal(FontType fontType)
{
	if (m_bDirty)
	{	// LOGFONT が変更されたならば、現在のフォントを削除
		deleteFont();
	}

	if (fontType == FontType_Smooth)
	{	// スムージング用フォントの作成
		// height, width を CNxSurface::SmoothFontRatio(4) 倍にして作成
		if (m_hFontSmooth == NULL)
		{
			LOGFONT lf = m_lf;
			lf.lfHeight *= CNxSurface::SmoothFontRatio;
			lf.lfWidth *= CNxSurface::SmoothFontRatio;
			m_hFontSmooth = ::CreateFontIndirect(&lf);
			if (m_hFontSmooth == NULL)
			{
				_RPTF0(_CRT_ASSERT, "CNxFont::GetFont() スムージング用フォントの作成に失敗しました.\n");
				return NULL;
			}
		}
		return m_hFontSmooth;
	}
	else
	{	// 通常フォントの作成
		if (m_hFont == NULL)
		{
			m_hFont = ::CreateFontIndirect(&m_lf);
			if (m_hFont == NULL)
			{
				_RPTF0(_CRT_ASSERT, "CNxFont::GetFont() フォントの作成に失敗しました.\n");
				return NULL;
			}
		}
		return m_hFont;
	}
}
