// NxTileSpriteDemo.cpp: CNxTileSpriteDemo クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxTileSpriteDemo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxTileSpriteDemo::CNxTileSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxTileSprite(pParent, pSurface, lpRect)
{
	m_ptDir.x = 0;
	m_ptDir.y = 0;
}

CNxTileSpriteDemo::~CNxTileSpriteDemo()
{

}

void CNxTileSpriteDemo::SetDirection(int x, int y)
{
	m_ptDir.x = x;
	m_ptDir.y = y;
}

void CNxTileSpriteDemo::PreUpdate()
{
	if (m_ptDir.x != 0 || m_ptDir.y != 0)
		OffsetSrcOrg(m_ptDir.x, m_ptDir.y);
}
