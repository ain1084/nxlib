// NxStretchSpriteDemo.cpp: CNxStretchSpriteDemo クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ball.h"
#include "NxStretchSpriteDemo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxStretchSpriteDemo::CNxStretchSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxStretchSprite(pParent, pSurface, lpRect)
{
	GetSrcRect(&m_rcSrcBegin);
	GetSrcRect(&m_rcSrcEnd);
}

CNxStretchSpriteDemo::~CNxStretchSpriteDemo()
{

}

void CNxStretchSpriteDemo::SetSrcBeginRect(const RECT* lpRect)
{
	m_rcSrcBegin = *lpRect;
	SetSrcRect(&m_rcSrcBegin);
}

void CNxStretchSpriteDemo::SetSrcEndRect(const RECT* lpRect)
{
	m_rcSrcEnd = *lpRect;
}

void CNxStretchSpriteDemo::PreUpdate()
{
	// かなり手抜きです。4:3 のサイズ以外では止まらなくなります
	RECT rcSrc;
	GetSrcRect(&rcSrc);

	if (::EqualRect(&rcSrc, &m_rcSrcEnd))
	{
		return;		// 最終サイズと同じならば何もしない
	}	

	int nDir = 1;
	if (m_rcSrcEnd.left < m_rcSrcEnd.left)
	{	// 小さくする
		nDir = -1;
	}

	::InflateRect(&rcSrc, 4 * nDir, 3 * nDir);
	SetSrcRect(&rcSrc);
}
