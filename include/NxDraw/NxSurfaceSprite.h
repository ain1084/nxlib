// NxSurfaceSprite.h: CNxSurfaceSprite クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェスの一部分(或いは全体)を単純に表示する機能を
//       持つ、CNxSprite 派生クラス。
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxSprite.h"
#include "NxSurface.h"

class CNxSurfaceSprite : public CNxSprite  
{
public:
	CNxSurfaceSprite(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxSurfaceSprite();

	virtual BOOL SetRect(const RECT* lpRect);
	CNxSurface* GetSrcSurface() const;
	virtual CNxSurface* SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL);
	void SetNxBlt(const NxBlt* pNxBlt, BOOL bUpdate = TRUE);
	void GetNxBlt(NxBlt* pNxBlt) const;

protected:
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;

private:
	CNxSurface* m_pSrcSurface;
	NxBlt m_nxBlt;

	CNxSurfaceSprite(const CNxSurfaceSprite&);
	CNxSurfaceSprite& operator=(const CNxSurfaceSprite&);
};

inline CNxSurface* CNxSurfaceSprite::GetSrcSurface() const {
	return m_pSrcSurface; }

inline void CNxSurfaceSprite::GetNxBlt(NxBlt* pNxBlt) const {
	*pNxBlt = m_nxBlt; }
