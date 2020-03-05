// NxMouseSprite.cpp: CNxMouseSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: CNxSprite でマウスカーソルを実現するクラス
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxMouseSprite.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxMouseSprite::CNxMouseSprite(CNxWindow* pWindow, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pWindow, pSurface, lpRect)
 , m_nShowCount(0)
{
	_ASSERTE(pWindow != NULL);

	// 初期位置を反映
	PreUpdate();
}

CNxMouseSprite::~CNxMouseSprite()
{

}

///////////////////////////////////////////////////////////////////////////
// public:
//	int CNxMouseSprite::Show(BOOL bShow)
// 概要: カーソルの表示カウンタを増減。正の数値であればカーソルを表示
// 引数: BOOL bShow ... カーソルを表示するなら TRUE
// 戻値: 現在の表示カウンタを返す
///////////////////////////////////////////////////////////////////////////

int CNxMouseSprite::Show(BOOL bShow)
{
	m_nShowCount += (bShow) ? 1 : -1;
	SetVisible(m_nShowCount >= 0);
	return m_nShowCount;
}

/////////////////////////////////////////////////////////////////////////
// public:
//	virtual void CNxMouseSprite::SetPos(int x, int y)
// 概要: スプライトの座標をマウスカーソルに反映させる
//       (CNxSprite::SetPos() のオーバーライド)
// 引数: int x ... 設定するX座標
//       int y ... 設定するY座標
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////

BOOL CNxMouseSprite::SetPos(int x, int y)
{
	POINT ptCursor;
	ptCursor.x = x;
	ptCursor.y = y;
	// トップレベルスプライト(CNxWindow) 上の座標へ変換
	SpriteToTop(&ptCursor);
	if (!getWindow()->SetCursorPos(ptCursor.x, ptCursor.y))
	{
		return FALSE;
	}
	else
	{
		return __super::SetPos(x, y);
	}
}

///////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxMouseSprite::PreUpdate()
// 概要: スプライトの更新状態が調べられる前に呼ばれる仮想関数
// 引数: なし
// 戻値: なし
// 備考: マウスカーソルの座標を取得してスプライト移動
///////////////////////////////////////////////////////////////////////////

void CNxMouseSprite::PreUpdate()
{
	POINT ptCursor;
	// 現在のトップレベルスプライト(CNxWindow) 上のマウスカーソル座標を取得
	if (!getWindow()->GetCursorPos(&ptCursor))
	{
		return;
	}
	// このスプライトの座標へ変換
	TopToSprite(&ptCursor);
	__super::SetPos(ptCursor.x, ptCursor.y);
}
