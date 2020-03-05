// NxTileSpriteDemo.h: CNxTileSpriteDemo クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXTILESPRITEDEMO_H__DBEAA420_74D9_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_NXTILESPRITEDEMO_H__DBEAA420_74D9_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "NxTileSprite.h"

class CNxTileSpriteDemo : public CNxTileSprite  
{
public:
	CNxTileSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxTileSpriteDemo();
	void SetDirection(int x, int y);

protected:
	virtual void PreUpdate();

private:
	POINT m_ptDir;
};

#endif // !defined(AFX_NXTILESPRITEDEMO_H__DBEAA420_74D9_11D4_AAAB_0090CCA661BD__INCLUDED_)
