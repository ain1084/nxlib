// NxStretchSprite.cpp: CNxStretchSprite クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxStretchSprite.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxStretchSprite::CNxStretchSprite(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pParent, pSurface, lpRect)
{
	SetSrcRect(lpRect);
}

CNxStretchSprite::~CNxStretchSprite()
{

}

void CNxStretchSprite::SetSrcRect(const RECT* lpSrcRect)
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

CNxSurface* CNxStretchSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect)
{
	CNxSurface* pResult = CNxSurfaceSprite::SetSrcSurface(pSurface, lpRect);
	SetSrcRect(lpRect);
	return pResult;
}

BOOL CNxStretchSprite::Draw(CNxSurface* pSurface, const RECT* /*lpRect*/) const
{
	RECT rect;
	GetRect(&rect);

	NxBlt nxb;
	GetNxBlt(&nxb);
	
	pSurface->Blt(&rect, GetSrcSurface(), &m_rcSrc, &nxb);
	return TRUE;
}
