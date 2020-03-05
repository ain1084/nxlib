// NxDiffuseSpriteDemo.h: CNxDiffuseSpriteDemo クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXDIFFUSESPRITEDEMO_H__F9E93542_4AEA_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_NXDIFFUSESPRITEDEMO_H__F9E93542_4AEA_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <nxdraw/nxdraw.h>

class CNxDiffuseSpriteDemo : public CNxSurfaceSprite  
{
public:
	CNxDiffuseSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxDiffuseSpriteDemo();
	
	void SetRange(int nRange);

protected:
	virtual void PreUpdate();

private:
	int m_nRange;
	int m_nDirection;
};

#endif // !defined(AFX_NXDIFFUSESPRITEDEMO_H__F9E93542_4AEA_11D4_AAAB_0090CCA661BD__INCLUDED_)
