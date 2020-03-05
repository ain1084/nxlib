// NxTileSprite.cpp: CNxTileSprite クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxTileSprite.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxTileSprite::CNxTileSprite(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pParent, pSurface, lpRect)
{
	SetSrcRect(lpRect);
	m_ptOrg.x = 0;
	m_ptOrg.y = 0;
}

CNxTileSprite::~CNxTileSprite()
{

}

// SetSrcRect と SetSrcSurface は, CNxStretchSprite の同名の関数そのままです
void CNxTileSprite::SetSrcRect(const RECT* lpSrcRect)
{
	RECT rcRect;
	if (lpSrcRect == NULL)
	{
		GetRect(&rcRect);
		lpSrcRect = &rcRect;
	}
	m_rcSrc = *lpSrcRect;
	SetUpdate();
}

CNxSurface* CNxTileSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect)
{
	CNxSurface* pResult = CNxSurfaceSprite::SetSrcSurface(pSurface, lpRect);
	SetSrcRect(lpRect);
	return pResult;
}

void CNxTileSprite::SetSrcOrg(int x, int y)
{
	m_ptOrg.x = x;
	m_ptOrg.y = y;
	SetUpdate();
}

void CNxTileSprite::OffsetSrcOrg(int nXOffset, int nYOffset)
{
	SetSrcOrg(m_ptOrg.x + nXOffset, m_ptOrg.y + nYOffset);
}

BOOL CNxTileSprite::Draw(CNxSurface* pSurface, const RECT* /*lpRect*/) const
{
	RECT rect;
	GetRect(&rect);

	NxBlt nxb;
	GetNxBlt(&nxb);
	
	pSurface->TileBlt(&rect, GetSrcSurface(), &m_rcSrc, m_ptOrg.x, m_ptOrg.y, &nxb);
	return TRUE;
}
