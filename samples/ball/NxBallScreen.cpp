// NxBallScreen.cpp: CNxBallScreen �N���X�̃C���v�������e�[�V����
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
// �\�z/����
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
		// �O���C�X�P�[����
		nxf.dwFlags = NxFilterBlt::grayscale;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		break;
	case Filter_sepia:
		// �Z�s�A��
		// �O���C�X�P�[�����ɕs�����x���w�肷�鎖�ŁA�����J���[�v�f���c��Z�s�A���ɂ��Ă��܂�
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
		// �F���ϊ�
		nxf.dwFlags = NxFilterBlt::hueTransform;
		nxf.nxfHueTransform.nHue = 180;
		nxf.nxfHueTransform.nSaturation = 0;
		nxf.nxfHueTransform.nLightness = 0;
		nxf.nxfHueTransform.bSingleness = FALSE;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		break;
	case Filter_negative:
		// �l�K���]
		nxf.dwFlags = NxFilterBlt::negative;
		pSurface->FilterBlt(lpRect, NULL, NULL, &nxf);
		break;
	}
	// ��{�N���X�̊֐����Ăяo��
	CNxScreen::DrawBehindChildren(pSurface, lpRect);
}
