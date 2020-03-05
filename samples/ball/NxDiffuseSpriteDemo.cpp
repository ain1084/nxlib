// NxDiffuseSpriteDemo.cpp: CNxDiffuseSpriteDemo クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ball.h"
#include "NxDiffuseSpriteDemo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxDiffuseSpriteDemo::CNxDiffuseSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pParent, pSurface, lpRect)
{
	m_nRange = 0;
	m_nDirection = 0;
}

CNxDiffuseSpriteDemo::~CNxDiffuseSpriteDemo()
{

}

void CNxDiffuseSpriteDemo::SetRange(int nRange)
{
	NxBltFx nxbf;
	nxbf.dwFlags = NxBltFx::diffuseHorz;
	if (nRange < 0)
	{
		m_nDirection = -1;
		nxbf.uDiffuseRange = abs(nRange);
		m_nRange = 0;
	}
	else
	{
		m_nDirection = 1;
		nxbf.uDiffuseRange = 0;
		m_nRange = nRange;
	}
	SetNxBltFx(&nxbf);
}

void CNxDiffuseSpriteDemo::PreUpdate()
{
	NxBltFx nxbf;
	GetNxBltFx(&nxbf);

	if (abs(m_nRange) != nxbf.uDiffuseRange)
	{
		nxbf.uDiffuseRange += m_nDirection;
		SetNxBltFx(&nxbf);
	}
}
