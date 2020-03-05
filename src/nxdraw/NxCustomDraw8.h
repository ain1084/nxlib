// NxCustomDraw8.h: CNxCustomDraw8 クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェスメモリへの直接描画(8bpp 専用)
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxCustomDraw.h"

namespace NxDrawLocal
{

class CNxCustomDraw8 : public CNxCustomDraw  
{
public:
	virtual BOOL Blt_BlurHorz(CNxSurface* pDestSurface, const RECT* lpDestRect,
							  const CNxSurface *pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const;
	virtual BOOL Blt_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
							  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
							const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* /*pNxBlt*/) const;
	virtual BOOL Blt_RuleBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
							   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
						   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
								   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
									const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
									   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_ColorFill_BlurHorzBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
											 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_ColorFill_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
										const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
									  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
									 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_ColorFill_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
											 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_ColorFill_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
											  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const;
	virtual BOOL Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
												 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const;
	virtual BOOL Filter_Grayscale(CNxSurface* pDestSurface, const RECT* lpDestRect,
								  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const;
	virtual BOOL Filter_RGBColorBalance(CNxSurface* pDestSurface, const RECT* lpDestRect,
										const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const;
	virtual BOOL Filter_HueTransform(CNxSurface* pDestSurface, const RECT* lpDestRect,
									 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const;
	virtual BOOL Filter_Negative(CNxSurface* pDestSurface, const RECT* lpDestRect,
								 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const;
private:

	static inline BYTE BiLinearFilter(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch);


	static void __cdecl Blt_NormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
														  LONG lDestDistance, LONG lSrcPitch,
														  UINT uWidth, UINT uHeight,
														  const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_Normal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
									   LONG lDestDistance, LONG lSrcDistance, LONG lSrcDelta,
									   UINT uWidth, UINT uHeight);

	static void __cdecl Blt_NormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											  LONG lDestDistance, LONG lSrcPitch,
											  UINT uWidth, UINT uHeight,
											  const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_MaskFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										  UINT uWidth, UINT uHeight, DWORD dwMask);
	static void __cdecl Blt_MaskFrom32_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										   UINT uWidth, UINT uHeight, DWORD dwMask);
};

} // namespace NxDrawLocal