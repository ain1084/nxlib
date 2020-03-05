// NxSurfaceSprite.cpp: CNxSurfaceSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェスの一部分(或いは全体)を単純に表示する機能を
//       持つ、CNxSprite 派生クラス。
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxSurfaceSprite.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxSurfaceSprite::CNxSurfaceSprite(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSprite(pParent)
{
	memset(&m_nxBlt, 0, sizeof(NxBlt));

	SetSrcSurface(pSurface, lpRect);
}

CNxSurfaceSprite::~CNxSurfaceSprite()
{

}

//////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSurfaceSprite::SetRect(const RECT* lpRect = NULL)
// 概要: スプライトの矩形を設定
// 引数: const RECT* lpRect ... 設定する矩形
// 戻値: 成功ならば TRUE。それ以外は FALSE (矩形がサーフェス範囲外等)
// 備考: lpRect が NULL の場合、接続されているサーフェス全体となる
//////////////////////////////////////////////////////////////////////////////

BOOL CNxSurfaceSprite::SetRect(const RECT* lpRect)
{
	RECT rect;

	if (m_pSrcSurface != NULL && lpRect == NULL)
	{
		m_pSrcSurface->GetRect(&rect);
		lpRect = &rect;
	}

	return CNxSprite::SetRect(lpRect);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxSurface* CNxSurfaceSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL)
// 概要: サーフェスを関連付ける
// 引数: CNxSurface* pSurface ... 関連付けるサーフェス
//       const RECT* lpRect       ... サーフェスの矩形(NULL ならばサーフェス全体)
// 戻値: 直前まで関連づけられていたサーフェス(無しならば NULL)
///////////////////////////////////////////////////////////////////////////////////////////////

CNxSurface* CNxSurfaceSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect)
{
	std::swap(m_pSrcSurface, pSurface);
	SetRect(lpRect);
	return pSurface;
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSurfaceSprite::Draw(CNxSurface* pSurface, const RECT* lpRect) const
// 概要: スプライト描画
// 引数: CNxSurface* pSurface ... 描画先サーフェスへのポインタ
//       const RECT* lpRect   ... スプライト内の描画矩形を示す RECT 構造体へのポインタ
// 戻値: 子スプライトの描画を続けるならば TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurfaceSprite::Draw(CNxSurface* pSurface, const RECT* /*lpRect*/) const
{
	RECT rect;
	GetRect(&rect);
	pSurface->Blt(&rect, GetSrcSurface(), &rect, &m_nxBlt);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSurfaceSprite::SetNxBltFx(const NxBlt* pNxBlt, BOOL bUpdate = TRUE)
// 概要: 転送時に使用する NxBlt を設定
// 引数: const NxBltFx* pNxBlt ... 設定する NxBlt 構造体へのポインタ
//       BOOL bUpdate          ... TRUE ならば、SetUpdate() を呼び出す
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////////

void CNxSurfaceSprite::SetNxBlt(const NxBlt* pNxBlt, BOOL bUpdate)
{
	if (pNxBlt == NULL)
		memset(&m_nxBlt, 0, sizeof(NxBlt));		// pNxBlt == NULL ならば構造体を初期化
	else
		m_nxBlt = *pNxBlt;

	if (bUpdate)
		SetUpdate();
}
