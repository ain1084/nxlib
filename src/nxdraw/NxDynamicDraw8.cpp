// NxDynamicDraw8.cpp: CNxDynamicDraw8 クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// 概要: 動的コードによるサーフェスメモリへの直接描画(8bpp 専用)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxDynamicDraw8.h"
#include "NxDrawLocal.h"
#include "NxSurface.h"

using namespace NxDrawLocal;

//////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDynamicDraw8::Blt()
// 概要: ビットブロック転送
// 戻値: 成功ならば TRUE
//////////////////////////////////////////////////////////////////////

BOOL CNxDynamicDraw8::Blt
(CNxSurface* /*pDestSurface*/,			// 転送先サーフェス
 const RECT* /*lpDestRect*/,			// 転送先矩形
 const CNxSurface* /*pSrcSurface*/,		// 転送元サーフェス
 const RECT* /*lpSrcRect*/,				// 転送元矩形
 const NxBlt* /*pNxBlt*/)				// 転送方法
{
	_RPTF0(_CRT_ASSERT, "CNxDyanmicDraw8::Blt() : この関数はインプリメントされていません.\n");
	return FALSE;
}
