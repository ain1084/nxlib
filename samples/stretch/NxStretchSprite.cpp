// NxStretchSprite.cpp: CNxStretchSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Aingouchi
//
// 概要: 拡大縮小スプライト
//       CNxSurfaceSprite の派生クラスです。
//       SetSrcRect() メンバ関数で設定された転送元矩形から、
//       (CNxSurfaceSprite クラスメンバの) SetRect() 関数で指定された矩形へ
//       拡大又は縮小をおこないながら転送します。
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxStretchSprite.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxStretchSprite::CNxStretchSprite(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pParent, pSurface, lpRect)
{
	SetSrcRect(lpRect);
}

CNxStretchSprite::~CNxStretchSprite()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual void CNxStretchSprite::SetSrcRect(const RECT* lpSrcRect)
// 概要: 転送元矩形を設定
// 引数: const RECT* lpSrcRect ... 設定する転送元矩形を示す RECT 構造体へのポインタ(NULL ならばサーフェス全体)
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxStretchSprite::SetSrcRect(const RECT* lpSrcRect)
{
	RECT rcRect;
	if (lpSrcRect == NULL)
	{
		GetRect(&rcRect);
		lpSrcRect = &rcRect;
	}
	m_rcSrc = *lpSrcRect;
	SetUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual void CNxStretchSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL)
// 概要: 転送元サーフェスと矩形を設定
// 引数: CNxSurface* pSurface ... サーフェスへのポインタ
//       const RECT *lpRect   ... 設定する矩形を示す RECT 構造体へのポインタ
// 備考: CNxSurfaceSprite::SetSrcSurface() のオーバーライド
//////////////////////////////////////////////////////////////////////////////////////////////////

CNxSurface* CNxStretchSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect)
{
	CNxSurface* pResult = CNxSurfaceSprite::SetSrcSurface(pSurface, lpRect);
	SetSrcRect(lpRect);
	return pResult;
}

BOOL CNxStretchSprite::Draw(CNxSurface* pSurface, const RECT* /*lpRect*/) const
{
	RECT rect;
	GetRect(&rect);

	NxBlt nxb;
	GetNxBlt(&nxb);
	
	pSurface->Blt(&rect, GetSrcSurface(), &m_rcSrc, &nxb);
	return TRUE;
}
