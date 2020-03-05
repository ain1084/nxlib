// NxBallSprite.h: CNxBallSprite クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXBALLSPRITE_H__6589FDF1_EAA3_11D3_87E9_0000E842F190__INCLUDED_)
#define AFX_NXBALLSPRITE_H__6589FDF1_EAA3_11D3_87E9_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxSurfaceSprite.h>

class CNxBallSprite : public CNxSurfaceSprite  
{
public:
	CNxBallSprite(CNxSprite* pParent, CNxSurface* pSurface);
	virtual ~CNxBallSprite();
	
private:
	int m_nDir;
	int m_nSpeedH;
	int m_nSpeedV;
	RECT m_rcBound;

protected:
	virtual void PreUpdate();
};

#endif // !defined(AFX_NXBALLSPRITE_H__6589FDF1_EAA3_11D3_87E9_0000E842F190__INCLUDED_)
