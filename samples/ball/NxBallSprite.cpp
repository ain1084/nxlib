// NxBallSprite.cpp: CNxBallSprite クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxBallSprite.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxBallSprite::CNxBallSprite(CNxSprite* pParent, CNxSurface* pSurface)
 : CNxSurfaceSprite(pParent, pSurface)
{
	m_nDir = rand() % 4;			// 進行方向
	m_nSpeedH = rand() % 3 + 1;		// 横速度
	m_nSpeedV = rand() % 3 + 1;		// 縦速度

	int nColor = rand() % 6;
	RECT rect;
	rect.left = nColor * 32;
	rect.top = 0;
	rect.right = rect.left + 32;
	rect.bottom = 32;
	SetRect(&rect);

	// 転げ回る領域
	GetParent()->GetRect(&m_rcBound);
	::InflateRect(&m_rcBound, 20, 20);	// 少し拡張
}

CNxBallSprite::~CNxBallSprite()
{

}

// スプライトの更新状態が調べられる直前に呼び出される仮想関数
void CNxBallSprite::PreUpdate()
{
	static const POINT ptBallMove[] =
	{
		{  1, -1 },
		{  1,  1 },
		{ -1,  1 },
		{ -1, -1 },
	};

	POINT point;
	GetPos(&point);

	for (;;)
	{
		point.x += ptBallMove[m_nDir].x * m_nSpeedH;
		point.y += ptBallMove[m_nDir].y * m_nSpeedV;

		// はみ出していれば方向転換
		if (point.x < m_rcBound.left || point.y < m_rcBound.top ||
			point.x + GetWidth() > m_rcBound.right ||
			point.y + GetHeight() > m_rcBound.bottom)
		{
			m_nDir = (m_nDir + 1) % 4;
		}
		else
			break;
	}
	SetPos(point.x, point.y);
}
