// NxCustomDraw.h: CNxCustomDraw クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェスメモリへの直接描画を行なう基本抽象クラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxSurface.h"

class CNxDIBImage;

namespace NxDrawLocal
{

class _declspec(novtable) CNxCustomDraw
{
public:
	virtual BOOL Blt_BlurHorz(CNxSurface* pDestSurface, const RECT* lpDestRect,
							  const CNxSurface *pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const = 0;
	virtual BOOL Blt_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
							  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
							const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* /*pNxBlt*/) const = 0;
	virtual BOOL Blt_RuleBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
							   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
						   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
								   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
									const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
									   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_ColorFill_BlurHorzBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
											 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_ColorFill_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
										const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
									  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
									 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_ColorFill_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
											 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_ColorFill_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
											  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
												 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const = 0;
	virtual BOOL Filter_Grayscale(CNxSurface* pDestSurface, const RECT* lpDestRect,
								  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const = 0;
	virtual BOOL Filter_RGBColorBalance(CNxSurface* pDestSurface, const RECT* lpDestRect,
										const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const = 0;
	virtual BOOL Filter_HueTransform(CNxSurface* pDestSurface, const RECT* lpDestRect,
									 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const = 0;
	virtual BOOL Filter_Negative(CNxSurface* pDestSurface, const RECT* lpDestRect,
								 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const = 0;
protected:
	static BOOL GetValidOpacity(const NxBlt* pNxBlt, LPUINT lpuOpacity);
	static BOOL IsStretch(const RECT* lpDestRect, const RECT* lpSrcRect);
};

} // namespace NxDrawLocal