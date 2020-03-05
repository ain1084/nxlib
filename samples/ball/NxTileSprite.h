// NxTileSprite.h: CNxTileSprite クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXTILESPRITE_H__13072461_65E0_11D4_8835_0000E842F190__INCLUDED_)
#define AFX_NXTILESPRITE_H__13072461_65E0_11D4_8835_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxSurfaceSprite.h>

class CNxTileSprite : public CNxSurfaceSprite  
{
public:
	CNxTileSprite(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxTileSprite();

	void SetSrcOrg(int x, int y);
	void GetSrcOrg(LPPOINT lpPoint) const;
	void OffsetSrcOrg(int nXOffset, int nYOffset);

	virtual void SetSrcRect(const RECT* lpSrcRect = NULL);
	void GetSrcRect(LPRECT lpSrcRect) const;

	virtual CNxSurface* SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL);

protected:
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;

private:
	RECT m_rcSrc;
	POINT m_ptOrg;
};

inline void CNxTileSprite::GetSrcRect(LPRECT lpSrcRect) const {
	*lpSrcRect = m_rcSrc; }

inline void CNxTileSprite::GetSrcOrg(LPPOINT lpPoint) const {
	*lpPoint = m_ptOrg; }

#endif // !defined(AFX_NXTILESPRITE_H__13072461_65E0_11D4_8835_0000E842F190__INCLUDED_)
