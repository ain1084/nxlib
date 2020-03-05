// NxCustomDraw.cpp: CNxCustomDraw クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェスメモリへの直接描画(抽象クラス)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxCustomDraw.h"

using namespace NxDrawLocal;

/////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static BOOL CNxCustomDraw::GetValidOpacity(const NxBlt* pNxBlt, LPUINT lpuOpacity)
// 概要: NxBlt 構造体から、有効な uOpacity 値を得る
// 引数: const NxBltFx* pNxBlt ... NxBlt 構造体へのポインタ
//       LPUINT lpuOpacity     ... 不透明度を受け取る UINT 型変数へのポインタ
// 戻値: 返された不透明度(*lpuOpacity)が有効ならば TRUE
 ////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw::GetValidOpacity(const NxBlt* pNxBlt, LPUINT lpuOpacity)
{
	if (pNxBlt->dwFlags & NxBlt::opacity)
	{
		if (pNxBlt->uOpacity > 255)
			return FALSE;

		*lpuOpacity = pNxBlt->uOpacity;
	}
	else
		*lpuOpacity = 255;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static BOOL CNxCustomDraw::IsStretch(const RECT* lpDestRect, const RECT* lpSrcRect)
// 概要: 二つの矩形の大きさが異なっている(拡大縮小を行う)ならば TRUE を返す
// 引数: const RECT* lpDestRect ... 転送先矩形を示す RECT 構造体へのポインタ
//       const RECT* lpSrcRect  ... 転送元矩形を示す RECT 構造体へのポインタ
// 戻値: 大きさが異なるならば TRUE
////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw::IsStretch(const RECT* lpDestRect, const RECT* lpSrcRect)
{
	return (
		((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left)) |
		((lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top))
	) != 0;
}
