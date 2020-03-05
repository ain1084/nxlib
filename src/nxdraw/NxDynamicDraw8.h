// NxDynamicDraw32.cpp: CNxDynamicDraw32 クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// 概要: 動的コードによるサーフェスメモリへの直接描画(8bpp 専用)
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDynamicDraw.h"

namespace NxDrawLocal
{

class CNxDynamicDraw8 : public CNxDynamicDraw
{
public:
	virtual BOOL Blt(CNxSurface* pDestSurface, const RECT* lpDestRect, 
					 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
					 const NxBlt* pNxBlt);
};

}	// namespace NxDrawLocal