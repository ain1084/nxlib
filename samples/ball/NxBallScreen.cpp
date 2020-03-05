// NxBallScreen.cpp: CNxBallScreen クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxBallScreen.h"
#include <NxStorage/NxStorage.h>
#include <NxDraw/NxSurface.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxBallScreen::CNxBallScreen()
{
	m_nFilter = Filter_none;
}

CNxBallScreen::~CNxBallScreen()
{

}

void CNxBallScreen::SetFilter(Filter nFilter)
{
	m_nFilter = nFilter;
	SetUpdate();
}

void CNxBallScreen::DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const
{
	NxFilterBlt nxf;
	switch (m_nFilter)
	{
	case Filter_grayscale:
		// グレイスケール化
		nxf.dwFlags = NxFilterBlt::grayscale;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		break;
	case Filter_sepia:
		// セピア調
		// グレイスケール時に不透明度を指定する事で、少しカラー要素が残るセピア調にしています
		nxf.dwFlags = NxFilterBlt::grayscale | NxFilterBlt::opacity;
		nxf.uOpacity = 240;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		nxf.dwFlags = NxFilterBlt::rgbColorBalance;
		nxf.nxfRGBColorBalance.Multiplier.wRed = 152;		// 152/128
		nxf.nxfRGBColorBalance.Multiplier.wGreen = 128;		// 128/128
		nxf.nxfRGBColorBalance.Multiplier.wBlue = 107;		// 107/128
		nxf.nxfRGBColorBalance.Adder.sRed = 24;
		nxf.nxfRGBColorBalance.Adder.sGreen = 0;
		nxf.nxfRGBColorBalance.Adder.sBlue = -4;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		break;
	case Filter_hueTransform:
		// 色相変換
		nxf.dwFlags = NxFilterBlt::hueTransform;
		nxf.nxfHueTransform.nHue = 180;
		nxf.nxfHueTransform.nSaturation = 0;
		nxf.nxfHueTransform.nLightness = 0;
		nxf.nxfHueTransform.bSingleness = FALSE;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		break;
	case Filter_negative:
		// ネガ反転
		nxf.dwFlags = NxFilterBlt::negative;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		break;
	}
	// 基本クラスの関数を呼び出す
	CNxScreen::DrawBehindChildren(pSurface, lpRect);
}
