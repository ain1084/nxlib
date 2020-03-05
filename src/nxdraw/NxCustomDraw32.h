// NxCustomDraw32.h: CNxCustomDraw32 クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェスメモリへの直接描画(32bpp 専用)
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxCustomDraw.h"

namespace NxDrawLocal
{

class CNxCustomDraw32 : public CNxCustomDraw  
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

	static void __cdecl Blt_Normal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
									   LONG lDestDistance, LONG lSrcDistance, LONG lSrcDelta,
									   UINT uWidth, UINT uHeight);

	static void __cdecl Blt_NormalFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											LONG lDestDistance, LONG lSrcDistance, LONG lSrcDelta,
											const NxColor* pColorTable, UINT uWidth, UINT uHeight);

	static void __cdecl Blt_NormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											  LONG lDestDistance, LONG lSrcPitch,
											  UINT uWidth, UINT uHeight,
											  const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_NormalStretchFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												   LONG lDestDistance, LONG lSrcPitch, const NxColor* pColorTable,
												   UINT uWidth, UINT uHeight,
												   const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_NormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
														  LONG lDestDistance, LONG lSrcPitch,
														  UINT uWidth, UINT uHeight,
														  const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_NormalStretchLinearFilterFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
															   LONG lDestDistance, LONG lSrcPitch, const NxColor* ColorTable,
															   UINT uWidth, UINT uHeight,
															   const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_NormalStretchLinearFilter_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
														  LONG lDestDistance, LONG lSrcPitch,
														  UINT uWidth, UINT uHeight,
														  const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_NormalStretchLinearFilterFrom8_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
															   LONG lDestDistance, LONG lSrcPitch, const NxColor* ColorTable,
															   UINT uWidth, UINT uHeight,
															   const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_RuleBlendNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
												LONG lDestDistance, LONG lSrcDistance, LONG lRuleDistance,
												UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256]);
	static void __cdecl Blt_RuleBlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
												LONG lDestDistance, LONG lSrcDistance, LONG lRuleDistance,
												UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256]);

	static void __cdecl Blt_RuleBlendNormalSrcAlpha_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
														LONG lDestDistance, LONG lSrcDistance, LONG lRuleDistance,
														UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256]);
	static void __cdecl Blt_RuleBlendNormalSrcAlpha_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
														LONG lDestDistance, LONG lSrcDistance, LONG lRuleDistance,
														UINT uWidth, UINT uHeight, const BYTE lpbRuleToOpacityTable[256]);

	static void __cdecl Blt_RGBAMaskFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											  UINT uWidth, UINT uHeight, DWORD dwMask);
	static void __cdecl Blt_RGBAMaskFrom32_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											   UINT uWidth, UINT uHeight, DWORD dwMask);
	static void __cdecl Blt_RGBAMaskFrom32_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,  LONG lDestDistance, LONG lSrcDistance,
											   UINT uWidth, UINT uHeight, DWORD dwMask);

	static void __cdecl Blt_RGBColorBalance_32to32_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													   UINT uWidth, UINT uHeight, DWORDLONG dwlMultiplier, DWORDLONG dwlAdder);
	static void __cdecl Blt_RGBColorBalance_32to32_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													   UINT uWidth, UINT uHeight, DWORDLONG dwlMultiplier, DWORDLONG dwlAdder);

	static void __cdecl Blt_BlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendNormalFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												 const NxColor* pColorTable, UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendNormalFrom8_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												 const NxColor* pColorTable, UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendNormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcPitch,
												   UINT uWidth, UINT uHeight, UINT uOpacity,
												   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendNormalStretchFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcPitch,
														const NxColor* pColorTable, UINT uWidth, UINT uHeight, UINT uOpacity,
														const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendNormalStretch_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcPitch,
												   UINT uWidth, UINT uHeight, UINT uOpacity,
												   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendNormalStretchFrom8_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcPitch,
														const NxColor* pColorTable, UINT uWidth, UINT uHeight, UINT uOpacity,
														const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendNormalStretchLinearFilter_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcPitch,
															   UINT uWidth, UINT uHeight, UINT uOpacity,
															   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendNormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcPitch,
															   UINT uWidth, UINT uHeight, UINT uOpacity,
															   const CNxSurface::StretchBltInfo* pStretchBltInfo);

	static void __cdecl Blt_BlendAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										 UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendAdd_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										 UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										 UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendSub_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										 UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										   UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendMulti_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										   UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										    UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendScreen_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											  UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendBrighten_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											  UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendDarken_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, UINT uOpacity);

	static void __cdecl Blt_BlendDestAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													 UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Blt_BlendDestAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													 UINT uWidth, UINT uHeight, UINT uOpacity);

	static void __cdecl Blt_BlendDestSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
														UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendDestSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
														UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);

	static void __cdecl Blt_BlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaNormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
														   LONG lDestDistance, LONG lSrcDistance,
														   UINT uWidth, UINT uHeight,
														   const BYTE bySrcAlphaToOpacityTable[256],
														   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendSrcAlphaNormalStretch_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
														   LONG lDestDistance, LONG lSrcDistance,
														   UINT uWidth, UINT uHeight,
														   const BYTE bySrcAlphaToOpacityTable[256],
														   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendSrcAlphaNormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
																	   LONG lDestDistance, LONG lSrcDistance,
																	   UINT uWidth, UINT uHeight,
																	   const BYTE bySrcAlphaToOpacityTable[256],
																	   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendSrcAlphaNormalStretchLinearFilter_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
																	   LONG lDestDistance, LONG lSrcDistance,
																	   UINT uWidth, UINT uHeight,
																	   const BYTE bySrcAlphaToOpacityTable[256],
																	   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
																			   LONG lDestDistance, LONG lSrcDistance,
																			   UINT uWidth, UINT uHeight,
																		       const BYTE bySrcAlphaToOpacityTable[256],
																			   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
																			   LONG lDestDistance, LONG lSrcDistance,
																			   UINT uWidth, UINT uHeight,
																		       const BYTE bySrcAlphaToOpacityTable[256],
																			   const CNxSurface::StretchBltInfo* pStretchBltInfo);
	static void __cdecl Blt_BlendSrcAlphaAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												 UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaAdd_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												 UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												 UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaSub_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												 UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												   UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaMulti_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												   UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaScreen_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													  UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaBrighten_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													  UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_BlendSrcAlphaDarken_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight, const BYTE bySrcAlphaToOpacityTable[256]);

	static void __cdecl Blt_ColorFill_RGBAMask_386(LPBYTE lpDestSurface, LONG lDestDistance,
												   UINT uWidth, UINT uHeight, DWORD dwColor, DWORD dwMask);
	static void __cdecl Blt_ColorFill_RGBAMask_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
												   UINT uWidth, UINT uHeight, DWORD dwColor, DWORD dwMask);

	static void __cdecl Blt_ColorFill_Normal_386(LPBYTE lpDestSurface, LONG lDestDistance,
												 UINT uWidth, UINT uHeight, DWORD dwColor);
	static void __cdecl Blt_ColorFill_Normal_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
												 UINT uWidth, UINT uHeight, DWORD dwColor);

	static void __cdecl Blt_ColorFill_BlendDestAlphaNormal_386(LPBYTE lpDestSurface, LONG lDestDistance,
														  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendDestAlphaNormal_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
														  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);

	static void __cdecl Blt_ColorFill_BlendNormal_386(LPBYTE lpDestSurface, LONG lDestDistance,
													  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendNormal_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
													  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendAdd_386(LPBYTE lpDestSurface, LONG lDestDistance,
												   UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendAdd_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
												   UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendSub_386(LPBYTE lpDestSurface, LONG lDestDistance,
												   UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendSub_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
												   UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendMulti_386(LPBYTE lpDestSurface, LONG lDestDistance,
													 UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendMulti_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
													 UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendScreen_386(LPBYTE lpDestSurface, LONG lDestDistance,
													  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendScreen_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
													  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendBrighten_386(LPBYTE lpDestSurface, LONG lDestDistance,
														UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendBrighten_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
														UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendDarken_386(LPBYTE lpDestSurface, LONG lDestDistance,
													  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static void __cdecl Blt_ColorFill_BlendDarken_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
													  UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);

	static void __cdecl Blt_ColorFill_BlendDestSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
																  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendDestSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
																  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);

	static void __cdecl Blt_ColorFill_BlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaAdd_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
														   UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaAdd_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
														   UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaSub_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
														   UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaSub_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
														   UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaMulti_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															 UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaMulti_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															 UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaScreen_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaScreen_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaBrighten_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
																UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaBrighten_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
																UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaDarken_386(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);
	static void __cdecl Blt_ColorFill_BlendSrcAlphaDarken_MMX(LPBYTE lpDestSurface, const BYTE* lpAlpha, LONG lDestDistance, LONG lAlphaDistance,
															  UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);

	static void __cdecl Blt_BlurHorz_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										 UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin, UINT uRange);

	static void __cdecl Blt_BlurHorz_3DNow(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
										   UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin, UINT uRange);

	static void __cdecl Blt_BlurHorzBlendNormal_3DNow(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													  UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
													  UINT uRange, UINT uOpacity);

	static void __cdecl Blt_BlurHorzBlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
													UINT uRange, UINT uOpacity);

	static void __cdecl Blt_BlurHorzBlendSrcAlphaNormal_3DNow(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
															  UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin, UINT uRange,
															  const BYTE bySrcAlphaToOpacityTable[256]);

	static void __cdecl Blt_BlurHorzBlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
															UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin, UINT uRange,
															const BYTE bySrcAlphaToOpacityTable[256]);

	static void __cdecl Blt_ColorFill_BlurHorzBlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
																	  UINT uWidth, UINT uHeight, DWORD dwColor,
																	  UINT uLeftMargin, UINT uRightMargin,
																	  UINT uRange, const BYTE bySrcAlphaToOpacityTable[256]);

	static void __cdecl Blt_ColorFill_BlurHorzBlendSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
																	  UINT uWidth, UINT uHeight, DWORD dwColor,
																	  UINT uLeftMargin, UINT uRightMargin,
																	  UINT uRange, const BYTE bySrcAlphaToOpacityTable[256]);
	
	static void __cdecl Filter_Grayscale_386(LPBYTE lpDestSurface, LONG lDestDistance, 
											 const BYTE* lpSrcSurface, LONG lSrcDistance,
											 UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Filter_Grayscale_MMX(LPBYTE lpDestSurface, LONG lDestDistance, 
											 const BYTE* lpSrcSurface, LONG lSrcDistance,
											 UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Filter_RGBColorBalance_386(LPBYTE lpDestSurface, LONG lDestDistance,
												   const BYTE* lpSrcSurface, LONG lSrcDistance,
												   UINT uWidth, UINT uHeight, UINT uOpacity,
												   DWORDLONG dwlMultiplier, DWORDLONG dwlAdder);
	static void __cdecl Filter_RGBColorBalance_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
												   const BYTE* lpSrcSurface, LONG lSrcDistance,
												   UINT uWidth, UINT uHeight, UINT uOpacity,
												   DWORDLONG dwlMultiplier, DWORDLONG dwlAdder);
	static void __cdecl Filter_Negative_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
											const BYTE* lpSrcSurface, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, UINT uOpacity);
	static void __cdecl Filter_Negative_386(LPBYTE lpDestSurface, LONG lDestDistance,
											const BYTE* lpSrcSurface, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, UINT uOpacity);


	static BOOL PreprocessStretchBlt(const CNxSurface* pDestSurface, CNxSurface::StretchBltInfo* pStretchBltInfo,
									 LPRECT lpDestRect, const CNxSurface* pSrcSurface, LPRECT lpSrcRect, LPBOOL pbFilter);

	static inline DWORD BiLinearFilter32(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch);
	static inline DWORD BiLinearFilterNoRegardAlpha32(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch);
	static inline DWORD BiLinearFilter8(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch, const NxColor* pColorTable);
	static inline DWORD BiLinearFilterNoRegardAlpha8(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch, const NxColor* pColorTable);
	static inline DWORD BiLinearPixel(UINT uSrcOrgX, UINT uSrgOrgY, DWORD dwTopLeft, DWORD dwTopRight,
									  DWORD dwBottomLeft, DWORD dwBottomRight);
};

}	// namespace NxDrawLocal
