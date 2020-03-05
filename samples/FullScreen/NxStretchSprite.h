// NxStretchSprite.h: CNxStretchSprite クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXSTRETCHSPRITE_H__CF2E4702_1D9E_11D4_8815_0000E842F190__INCLUDED_)
#define AFX_NXSTRETCHSPRITE_H__CF2E4702_1D9E_11D4_8815_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxSurfaceSprite.h>

class CNxStretchSprite : public CNxSurfaceSprite  
{
public:
	CNxStretchSprite(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxStretchSprite();

	virtual void SetSrcRect(const RECT* lpSrcRect = NULL);
	void GetSrcRect(LPRECT lpSrcRect) const;
	virtual CNxSurface* SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL);

protected:
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;

private:
	RECT m_rcSrc;
};

inline void CNxStretchSprite::GetSrcRect(LPRECT lpSrcRect) const {
	*lpSrcRect = m_rcSrc; }

#endif // !defined(AFX_NXSTRETCHSPRITE_H__CF2E4702_1D9E_11D4_8815_0000E842F190__INCLUDED_)
