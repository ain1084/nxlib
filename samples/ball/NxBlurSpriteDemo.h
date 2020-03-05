// NxBlurSpriteDemo.h: CNxBlurSpriteDemo クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXBLURSPRITEDEMO_H__F9E93542_4AEA_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_NXBLURSPRITEDEMO_H__F9E93542_4AEA_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxSurfaceSprite.h>

class CNxBlurSpriteDemo : public CNxSurfaceSprite  
{
public:
	CNxBlurSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxBlurSpriteDemo();
	
	void SetRange(int nRange);

protected:
	virtual void PreUpdate();

private:
	int m_nRange;
	int m_nDirection;
};

#endif // !defined(AFX_NXBLURSPRITEDEMO_H__F9E93542_4AEA_11D4_AAAB_0090CCA661BD__INCLUDED_)
