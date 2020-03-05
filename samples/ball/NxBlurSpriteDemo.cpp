// NxBlurSpriteDemo.cpp: CNxBlurSpriteDemo クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ball.h"
#include "NxBlurSpriteDemo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxBlurSpriteDemo::CNxBlurSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pParent, pSurface, lpRect)
{
	m_nRange = 0;
	m_nDirection = 0;
}

CNxBlurSpriteDemo::~CNxBlurSpriteDemo()
{

}

void CNxBlurSpriteDemo::SetRange(int nRange)
{
	NxBlt nxb;
	nxb.dwFlags = NxBlt::blurHorz;
	if (nRange < 0)
	{
		m_nDirection = -1;
		nxb.uBlurRange = abs(nRange);
		m_nRange = 0;
	}
	else
	{
		m_nDirection = 1;
		nxb.uBlurRange = 0;
		m_nRange = nRange;
	}
	SetNxBlt(&nxb);
}

void CNxBlurSpriteDemo::PreUpdate()
{
	NxBlt nxb;
	GetNxBlt(&nxb);

	if (abs(m_nRange) != nxb.uBlurRange)
	{
		nxb.uBlurRange += m_nDirection;
		SetNxBlt(&nxb);
	}
}
