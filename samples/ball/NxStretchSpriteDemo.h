// NxStretchSpriteDemo.h: CNxStretchSpriteDemo クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXSTRETCHSPRITEDEMO_H__CF2E4703_1D9E_11D4_8815_0000E842F190__INCLUDED_)
#define AFX_NXSTRETCHSPRITEDEMO_H__CF2E4703_1D9E_11D4_8815_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "NxStretchSprite.h"

class CNxStretchSpriteDemo : public CNxStretchSprite  
{
public:
	CNxStretchSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxStretchSpriteDemo();

	void SetSrcBeginRect(const RECT* lpRect);
	void SetSrcEndRect(const RECT* lpRect);

protected:
	void PreUpdate();

private:
	RECT m_rcSrcBegin;
	RECT m_rcSrcEnd;
};

#endif // !defined(AFX_NXSTRETCHSPRITEDEMO_H__CF2E4703_1D9E_11D4_8815_0000E842F190__INCLUDED_)
