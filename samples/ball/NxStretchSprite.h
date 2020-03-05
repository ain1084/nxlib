// NxStretchSprite.h: CNxStretchSprite クラスのインターフェイス
// Copyright(c) 2000 S.Aingouchi
//
// 概要: 拡大縮小スプライト
//       CNxSurfaceSprite の派生クラスです。
//       SetSrcRect() メンバ関数で設定された転送元矩形から、
//       (CNxSurfaceSprite クラスメンバの) SetRect() 関数で指定された矩形へ
//       拡大又は縮小をおこないながら転送します。
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
	virtual void SetSrcRect(const RECT* lpSrcRect);
	void GetSrcRect(LPRECT lpSrcRect) const;

	// CNxSurfaceSprite override
	virtual CNxSurface* SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL);

protected:
	// CNxSprite override
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;

private:
	RECT m_rcSrc;		// 転送元矩形
};

inline void CNxStretchSprite::GetSrcRect(LPRECT lpSrcRect) const {
	*lpSrcRect = m_rcSrc; }

#endif // !defined(AFX_NXSTRETCHSPRITE_H__CF2E4702_1D9E_11D4_8815_0000E842F190__INCLUDED_)
