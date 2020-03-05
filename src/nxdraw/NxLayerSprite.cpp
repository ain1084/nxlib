// NxLayerSprite.cpp: CNxLayerSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: CNxSprite と CNxSurfaceSprite を多重継承したクラス
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxLayerSprite.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxLayerSprite::CNxLayerSprite(CNxSprite* pParent, UINT uBitCount)
 : CNxSurfaceSprite(pParent), CNxSurface(uBitCount)
{

}

CNxLayerSprite::~CNxLayerSprite()
{

}

////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxLayerSprite::Create(int nWidth, int nHeight)
// 概要: サーフェスを作成(スプライトサイズも設定)
// 引数: int nWidth     ... 幅
//       int nHeight    ... 高さ
// 戻値: 成功なら TRUE
// 備考: CNxSurface::Create(int,int) のオーバーライド
////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxLayerSprite::Create(int nWidth, int nHeight)
{
	if (CNxSurface::Create(nWidth, nHeight))
	{	// 成功なら、(CNxSurfaceSprite における)転送元サーフェスとして自分を設定
		CNxSurfaceSprite::SetSrcSurface(this);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxSurface* CNxLayerSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL)
// 概要: サーフェスを関連付ける
// 引数: CNxSurface* pSurface ... 関連付けるサーフェス
//       const RECT* lpRect       ... サーフェスの矩形(NULL ならばサーフェス全体)
// 戻値: 直前まで関連づけられていたサーフェス(無しならば NULL)
// 備考: CNxSurfaceSprite::SetSrcSurface() のオーバーライド
//       サーフェスは自分を示す為、変更できない
///////////////////////////////////////////////////////////////////////////////////////////////

CNxSurface* CNxLayerSprite::SetSrcSurface(CNxSurface* /*pSurface*/, const RECT* lpRect)
{
	return CNxSurfaceSprite::SetSrcSurface(this, lpRect);
}
