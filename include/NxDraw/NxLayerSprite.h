// NxLayerSprite.h: CNxLayerSprite クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: CNxSprite と CNxSurfaceSprite を多重継承したクラス
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxSurface.h"
#include "NxSurfaceSprite.h"

class CNxLayerSprite : public CNxSurface, public CNxSurfaceSprite  
{
public:
	CNxLayerSprite(CNxSprite* pParent, UINT uBitCount = 32);
	virtual ~CNxLayerSprite();

	using CNxSurface::Create;

	virtual BOOL Create(int nWidth, int nHeight);
	virtual CNxSurface* SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL);

	int GetWidth() const;
	int GetHeight() const;
	void GetRect(LPRECT lpRect) const;

private:
	CNxLayerSprite(const CNxLayerSprite&);
	CNxLayerSprite& operator=(const CNxLayerSprite&);
};

inline int CNxLayerSprite::GetWidth() const {
	return CNxSprite::GetWidth(); }

inline int CNxLayerSprite::GetHeight() const {
	return CNxSprite::GetHeight(); }

inline void CNxLayerSprite::GetRect(LPRECT lpRect) const {
	CNxSprite::GetRect(lpRect); }
