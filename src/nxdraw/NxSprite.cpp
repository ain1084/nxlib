// NxSprite.cpp: CNxSprite クラスのインプリメンテーション
// Copyright(c) 2000, 2001 S.Ainoguchi
//
// 概要: スプライトの基本クラス
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include <algorithm>
#include "NxSprite.h"
#include "NxUpdateRegion.h"
#include "NxSurface.h"

using namespace NxDrawLocal;


//////////////////////////////////////////////////////////////////////
// public:
//	CNxSprite::CNxSprite(CNxSprite* pParent)
// 概要: CNxSprite クラスのコンストラクタ
// 引数: CNxSprite* pParent ... 親スプライトへのポインタ(NULL可)
// 戻値: ---
//////////////////////////////////////////////////////////////////////

CNxSprite::CNxSprite(CNxSprite* pParent)
 : m_pParent(pParent)				// 親スプライトへのポインタ(null = 親無し)
 , m_nZPosition(0)					// Z 座標
 , m_bVisible(TRUE)					// 可視フラグ
 , m_bClipChildren(TRUE)			// 子クリップ行なう
 , m_bUpdateMyself(FALSE)			// 自分では更新していない
 , m_bDirtyZ(FALSE)					// Z座標変更フラグ
 , m_fdwUpdateWithChildren(0)		// 差分描画関連フラグ
{
	m_ptPosition.x = 0;			// スプライト座標
	m_ptPosition.y = 0;
	m_ptDisplayOrg.x = 0;		// 表示原点
	m_ptDisplayOrg.y = 0;

	::SetRectEmpty(&m_rect);			// スプライト矩形
	::SetRectEmpty(&m_rcForce);			// 強制更新の矩形
	::SetRectEmpty(&m_rcUpdate);		// 更新される矩形
	::SetRectEmpty(&m_rcPrevFullDraw);	// 以前に全域描画した矩形

	// 指定された親へ子として追加
	if (m_pParent != NULL)
		m_pParent->AddChild(this);
}

/////////////////////////////////////////////////////////////////////
// public:
//	CNxSprite::~CNxSprite()
// 概要: CNxSprite クラスのデストラクタ
// 引数: ---
// 戻値: ---
/////////////////////////////////////////////////////////////////////

CNxSprite::~CNxSprite()
{
	// 子スプライトを全て削除
	while (!m_children.empty())
		delete m_children.back();

	// 親スプライトリストから自分を削除
	if (m_pParent != NULL)
		m_pParent->RemoveChild(this);
}

////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxSprite::AddChild(CNxSprite* pSprite)
// 概要: 子スプライトリストへスプライトを追加する
// 引数: CNxSprite* pSprite ... 追加するスプライト
// 戻値: なし
// 備考: 追加と同時に Z ソートされる
////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::AddChild(CNxSprite* pSprite)
{
	// 親子関係がネストしてしまうと、色々なところで無限ループを
	// 引き起こす可能性があるが、現在のところチェックは行なっていない
	_ASSERTE(pSprite != NULL);
	
	// 最後に挿入
	m_children.push_back(pSprite);

	// 子の矩形更新を要求
	m_fdwUpdateWithChildren |= updateWithChildren;

	// Zソートを行う
	m_bDirtyZ = TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSprite::RemoveChild(CNxSprite* pSprite)
// 概要: 子スプライトリストからスプライトを削除する
// 引数: CNxSprite *pSprite ... 削除するスプライト
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::RemoveChild(CNxSprite* pSprite)
{
	_ASSERTE(pSprite != NULL);
	
	SpriteContainer::iterator it;
	SpriteContainer::iterator itend = m_children.end();
	it = std::find(m_children.begin(), itend, pSprite);
	if (it == itend)
	{
		_RPTF0(_CRT_WARN, "CNxSprite : 存在しないスプライトの削除が要求されました.\n");
		return FALSE;
	}
	(*it)->UpdateWithChildren(TRUE, TRUE);						// 削除されるスプライト(以下)の矩形を回収
	::UnionRect(&m_rcForce, &m_rcForce, &(*it)->m_rcForce);		// 回収した矩形を自分自身へ追加
	m_children.erase(it);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSprite::UpdateForceChildren()
// 概要: 自分自身と、全ての子スプライトを強制的に更新
// 引数: なし
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::UpdateForceChildren()
{
	// 更新矩形を全体として設定
	// 行っている事は、SetUpdate() とあまり変らないが、敢えて SetUpdate() を呼び出さないのは、
	// m_bUpdateMyself が true になってしまい、後で CNxSprite::Update() が勝手に呼び出されて
	// しまうのを防ぐ為
	m_rcUpdate = m_rect;
	m_fdwUpdateWithChildren = 0;
	::SetRectEmpty(&m_rcPrevFullDraw);
	::SetRectEmpty(&m_rcForce);

	for (SpriteContainer::iterator it = m_children.begin(); it != m_children.end(); it++)
		(*it)->UpdateForceChildren();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSprite::UpdateWithChildren(BOOL bIgnoreVisible BOOL bIgnoreClip)
// 概要: 子を含めて正しく描画される様に、m_rcForce (強制更新矩形) を設定する
// 引数: BOOL bIgnoreVisible ... 不可視のスプライトも対象とするならば TRUE(invisible -> visible では必須)
//       BOOL bIgnoreClip    ... 子スプライトのクリップフラグを無視するならば TRUE
// 戻値: 矩形があるならば TRUE
// 備考: 自分自身だけではなく、子スプライトへ影響を与える操作後には m_fdwUpdateWithChildren が必要
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::UpdateWithChildren(BOOL bIgnoreVisible, BOOL bIgnoreClip)
{
	// このスプライトで要求されているフラグを考慮
	bIgnoreClip |= (m_fdwUpdateWithChildren & updateWithChildren_IgnoreClip);
	bIgnoreVisible |= (m_fdwUpdateWithChildren & updateWithChildren_IgnoreVisible);

	// 子スプライトの CNxSprite::SetDrawRect 内で何度も無駄に処理されない様にフラグをクリア
	m_fdwUpdateWithChildren = 0;

	if (bIgnoreVisible | m_bVisible)
	{	// 表示されている(又は無視)
		if (bIgnoreClip | !m_bClipChildren)
		{	// 子クリップしない(又は無視)
			for (SpriteContainer::iterator it = m_children.begin(); it != m_children.end(); it++)
			{
				if ((*it)->UpdateWithChildren(bIgnoreVisible, bIgnoreClip))
				{	// 子の矩形を結合する
					::UnionRect(&m_rcForce, &m_rcForce, &(*it)->m_rcForce);
					// 子の強制更新矩形をクリア(重なった矩形を無駄に更新するのを防ぐ)
					(*it)->m_rcForce.top = 0;
					(*it)->m_rcForce.left = 0;
					(*it)->m_rcForce.right = 0;
					(*it)->m_rcForce.bottom = 0;
				}
			}
		}
		// 自分の矩形も追加
		m_rcUpdate = m_rect;
		::UnionRect(&m_rcForce, &m_rcForce, &m_rcPrevFullDraw);
		return TRUE;
	}
	return FALSE;
}	


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::GetUpdateRegion(CNxUpdateRegion* pRegion, BOOL bDoPreUpdate)
// 概要: 全てのスプライトの更新領域を取得
// 引数: CNxUpdateRegion* pRegion ... 更新領域を受け取る、CNxUpdateRegion 派生クラスへのポインタ
//       BOOL bDoPreUpdate        ... CNxSprite::PreUpdate() を呼び出すならば TRUE
// 戻値: 更新フラグを書き込んだ(更新部分が発生した)ならば TRUE
// 備考: private 関数の簡易版
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::GetUpdateRegion(NxDrawLocal::CNxUpdateRegion* pRegion, BOOL bDoPreUpdate)
{
	POINT ptParent;
	ptParent.x = 0;
	ptParent.y = 0;

	return GetUpdateRegion(pRegion, ptParent, &m_rect, bDoPreUpdate);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSprite::GetUpdateRegion(CNxUpdateRegion* pRegion, POINT ptParent, const RECT* lpRectParent, BOOL bDoPreUpdate)
// 概要: 全てのスプライトの更新領域を取得
// 引数: CNxUpdateRegion* pRegion ... 更新領域を受け取る、CNxUpdateRegion 派生クラスへのポインタ
//       POINT  ptParent          ... 親スプライトの座標
//       const RECT* lpRectParent ... 親スプライトの矩形
//       BOOL bDoPreUpdate        ... CNxSprite::PreUpdate() を呼び出すならば TRUE
// 戻値: 更新フラグを書き込んだ(更新部分が発生した)ならば TRUE
// 備考: lpRectParent はスプライトが書き込み可能な範囲を示す。CNxDrawRect の示す範囲を超えてはならない
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::GetUpdateRegion(NxDrawLocal::CNxUpdateRegion* pRegion, POINT ptParent, const RECT* lpRectParent, BOOL bDoPreUpdate)
{
	// 更新前処理
	if (bDoPreUpdate)
		PreUpdate();

	if (m_fdwUpdateWithChildren != 0)
	{	// 子スプライトも道連れ更新
		UpdateWithChildren();
		// 上の関数中で m_fdwUpdateWithChildren は、
		// 必ずゼロになるので、ここではクリアする必要はない
	}

	BOOL bUpdateFlag = FALSE;

	// 削除された子スプライトの領域等を強制的に更新
	//
	// if ((rect.right - rect.left | rect.bottom - rect.top) > 0) では、矩形が空であるかを調べているが
	// IsRectEmpty() とは異なり、両方共ゼロでなければ、空の矩形としては判定されない。IsRectEmpty() では、
	// どちらかがゼロであれば空として扱われる(どちらにせよ後で別々に判定されるので問題は無い)。

	if ((m_rcForce.right - m_rcForce.left | m_rcForce.bottom - m_rcForce.top) > 0)
	{
		if (pRegion != NULL)
			pRegion->AddRect(&m_rcForce);

		m_rcForce.left = 0;
		m_rcForce.top = 0;
		m_rcForce.right = 0;
		m_rcForce.bottom = 0;
		bUpdateFlag = TRUE;
	}

	if (m_pParent != NULL)
	{	// 親スプライト有り(最上位の親ではない)

		// スプライトが表示されていないならば、これ以後の処理は不要
		if (!m_bVisible)
			return bUpdateFlag;
	
		// このスプライトの左上座標
		ptParent.x += m_ptPosition.x + m_ptDisplayOrg.x;
		ptParent.y += m_ptPosition.y + m_ptDisplayOrg.y;
	}

	// 更新フラグを立てる
	if ((m_rcUpdate.right - m_rcUpdate.left | m_rcUpdate.bottom - m_rcUpdate.top) > 0)
	{
		RECT rcSrc;
		POINT ptDest;
		int nRectDiff;

		// rcSrc = m_rcUpdate;
		// m_rcUpdate == m_rect ならば、nRectDiff == 0 (全域更新の判別に用いる)
		// ptDest は、ptParent + (m_rcUpdate - m_rect) が設定される

		nRectDiff = m_rcUpdate.top - m_rect.top;
		rcSrc.top = m_rcUpdate.top;
		ptDest.y = ptParent.y + m_rcUpdate.top - m_rect.top;
		nRectDiff |= m_rcUpdate.left - m_rect.left;
		rcSrc.left = m_rcUpdate.left;
		ptDest.x = ptParent.x + m_rcUpdate.left - m_rect.left;
		nRectDiff |= m_rcUpdate.right - m_rect.right;
		rcSrc.right = m_rcUpdate.right;
		nRectDiff |= m_rcUpdate.bottom - m_rect.bottom;
		rcSrc.bottom = m_rcUpdate.bottom;

		if (IntersectClipRect(&ptDest, &rcSrc, lpRectParent))
		{	// 矩形は有効
			rcSrc.right = rcSrc.right - rcSrc.left + ptDest.x;
			rcSrc.bottom = rcSrc.bottom - rcSrc.top + ptDest.y;
			rcSrc.left = ptDest.x;
			rcSrc.top = ptDest.y;

			// 全域更新矩形(m_rcUpdate == m_rect) ならば保存する
			if (nRectDiff == 0)
				m_rcPrevFullDraw = rcSrc;

			// 今回の描画範囲のフラグを立てる
			if (pRegion != NULL)
				pRegion->AddRect(&rcSrc);

			// 自分自身による更新ならば更新関数を呼び出す
			if (m_bUpdateMyself)
			{
				m_bUpdateMyself = FALSE;
				Update();
			}
			bUpdateFlag = TRUE;				// (更新済み)
		}
		else
		{	// 矩形が無効
			// 今回が全域更新(m_rcUpdate == m_rect)だったならば(前回の)矩形をクリア
			if (nRectDiff == 0)
			{
				m_rcPrevFullDraw.top = 0;
				m_rcPrevFullDraw.left = 0;
				m_rcPrevFullDraw.right = 0;
				m_rcPrevFullDraw.bottom = 0;
			}
		}
		// 描画要求クリア
		m_rcUpdate.top = 0;
		m_rcUpdate.left = 0;
		m_rcUpdate.right = 0;
		m_rcUpdate.bottom = 0;
	}

	if (!m_bClipChildren || (m_rcPrevFullDraw.right - m_rcPrevFullDraw.left | m_rcPrevFullDraw.bottom - m_rcPrevFullDraw.top) > 0)
	{
		if (m_pParent == NULL)
		{	// 最上位の親ならば座標をここで移動
			ptParent.x += m_ptPosition.x + m_ptDisplayOrg.x;
			ptParent.y += m_ptPosition.y + m_ptDisplayOrg.y;
		}
		else
		{	// 子クリップするなら自分の(今回が無効ならば直前の)矩形、しないならば自分の親の矩形(lpRectParent)
			if (m_bClipChildren)
				lpRectParent = &m_rcPrevFullDraw;
		}	

		// 子スプライトの Z ソート(直属の子のみ)
		SortChildren(TRUE);
		
		for (SpriteContainer::iterator it = m_children.begin(); it != m_children.end(); it++)
		{
			bUpdateFlag |= (*it)->GetUpdateRegion(pRegion, ptParent, lpRectParent, bDoPreUpdate);
		}
	}
	return bUpdateFlag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxSprite::DrawSurface(CNxSurface* pSurface, const POINT* lpPointSurfaceDest,
//								const POINT* lpPointParent, const RECT* lpRect) const
// 概要: スプライトを描画(内部用)
// 引数: CNxSurface* pSruface ... 描画先のサーフェス
//       const POINT* lpPointSurfaceDest  ... サーフェス上への表示座標を示す POINT 構造体へのポインタ
//       const POINT* lpPointParent       ... 親スプライトの座標を示す POINT 構造体へのポインタ
//       const RECT* lpRect               ... 描画するスプライト矩形
// 戻値: なし
// 備考: 自分が最上位の親ならば、不可視でも Draw() は呼び出す
///////////////////////////////////////////////////////////////////////////////////////////////////////  

void CNxSprite::DrawSurface(CNxSurface* pSurface, const POINT* lpPointSurfaceDest,
							const POINT *lpPointParent, const RECT* lpRect) const
{
	POINT ptParent;

	// 自分が不可視ならば return (ただし、自分が最上の親である場合を除く)
	if (m_pParent == NULL)
	{
		ptParent.x = lpPointParent->x;
		ptParent.y = lpPointParent->y;
	}
	else
	{
		if (!m_bVisible)
			return;

		ptParent.x = lpPointParent->x + (m_ptPosition.x + m_ptDisplayOrg.x);
		ptParent.y = lpPointParent->y + (m_ptPosition.y + m_ptDisplayOrg.y);
	}

	RECT rcChildClip;
	POINT ptSurfaceDest;
	POINT ptDest = ptParent;		// スプライトの描画先座標(この時点では暫定)
	RECT rcSrcClip = m_rect;		// IntersectClipRect 後はスプライトの転送矩形
	SpriteContainer::const_iterator it;
		
	if (!IntersectClipRect(&ptDest, &rcSrcClip, lpRect))	// (inline 関数)
	{	// 描画矩形なし
		if (m_pParent == NULL)
		{	// [最上位スプライトの場合]
			// 不可視ならば戻る(最上位スプライトならば、不可視でもここまで到達する為)
			if (!m_bVisible)
				return;

			// ここで座標を移動
			ptParent.x += (m_ptPosition.x + m_ptDisplayOrg.x);
			ptParent.y += (m_ptPosition.y + m_ptDisplayOrg.y);
		}
		else
		{	// [子スプライトの場合]
			// 描画矩形が無く、子をクリップするならば、以下の子は何も描画できない。
			if (m_bClipChildren)
				return;
		}
		// 子スプライトを最下面から表示する
		for (it = m_children.begin(); it != m_children.end(); it++)
			(*it)->DrawSurface(pSurface, lpPointSurfaceDest, &ptParent, lpRect);
	}
	else
	{
		// 転送すべきスプライト内矩形(rcSrcClip)から、転送先原点を算出
		POINT ptDestOrg;
		ptDestOrg.x = lpPointSurfaceDest->x + (ptParent.x - m_rect.left - lpRect->left);
		ptDestOrg.y = lpPointSurfaceDest->y + (ptParent.y - m_rect.top - lpRect->top);

		// 転送先クリッピング矩形を算出
		RECT rcDestClip;
		rcDestClip.top = rcSrcClip.top + ptDestOrg.y;
		rcDestClip.left = rcSrcClip.left + ptDestOrg.x;
		rcDestClip.right = rcSrcClip.right + ptDestOrg.x;
		rcDestClip.bottom = rcSrcClip.bottom + ptDestOrg.y;

		pSurface->SetOrg(ptDestOrg.x, ptDestOrg.y);	// 原点
		pSurface->SetClipRect(&rcDestClip);			// クリッピング矩形
		
		// 描画
		BOOL bDrawChildren = Draw(pSurface, &rcSrcClip);

		if (m_pParent == NULL)
		{
			// 不可視でも m_pParent == NULL ならばここまで到達する。
			// 自分自身が不可視であれば子の表示は不要
			if (!m_bVisible)
				return;

			ptParent.x += (m_ptPosition.x + m_ptDisplayOrg.x);
			ptParent.y += (m_ptPosition.y + m_ptDisplayOrg.y);
		}
		else
		{
			// 子スプライトをクリップするならば自分の矩形、
			// そうでないならば親の矩形をそのまま子スプライトへ渡す

			if (m_bClipChildren)
			{	// 親矩形内に縮小した矩形&座標を計算
				ptSurfaceDest.x = lpPointSurfaceDest->x + (ptDest.x - lpRect->left);
				ptSurfaceDest.y = lpPointSurfaceDest->y + (ptDest.y - lpRect->top);
				lpPointSurfaceDest = &ptSurfaceDest;
				rcChildClip.left = ptDest.x;
				rcChildClip.top = ptDest.y;
				rcChildClip.right = (rcSrcClip.right - rcSrcClip.left) + ptDest.x;
				rcChildClip.bottom = (rcSrcClip.bottom - rcSrcClip.top) + ptDest.y;
				lpRect = &rcChildClip;
			}
		}
		// 子スプライトを最下面から表示する
		if (bDrawChildren)
		{
			for (it = m_children.begin(); it != m_children.end(); it++)
				(*it)->DrawSurface(pSurface, lpPointSurfaceDest, &ptParent, lpRect);
		}
		// 子スプライト表示後に DrawBehindChildren() 関数を呼び出す
		pSurface->SetOrg(ptDestOrg.x, ptDestOrg.y);
		pSurface->SetClipRect(&rcDestClip);
		DrawBehindChildren(pSurface, &rcSrcClip);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::DrawSurface(CNxSurface* pDestSurface, int dx, int dy, const RECT* lpSrcRect = NULL) const
// 概要: スプライトを描画 (public)
// 引数: CNxSurface* pDestSurface .... 描画先サーフェスへのポインタ
//       int dx                   .... サーフェス上の描画先 X 座標
//       int dy                   .... サーフェス上の描画先 Y 座標
//       const RECT* lpSrcRect    .... 描画されるスプライト矩形
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::DrawSurface(CNxSurface* pDestSurface, int dx, int dy, const RECT* lpSrcRect) const
{
	// 自前のクリップ
	RECT rcSrc;
	if (lpSrcRect == NULL)
	{	// lpSrcRect が省略されていたら、スプライトの矩形を使用
		GetRect(&rcSrc);
	}
	else
		rcSrc = *lpSrcRect;

	RECT rcClip;
	pDestSurface->GetClipRect(&rcClip);
	POINT ptSurfaceDestOrg;
	pDestSurface->GetOrg(&ptSurfaceDestOrg);

	POINT ptSurfaceDest;	// 描画先先座標

	// クリップ(一度 (0,0) へ合わせてから比較)
	int nXOffset = -rcSrc.left + dx + ptSurfaceDestOrg.x;
	rcSrc.left = max(rcSrc.left + nXOffset, rcClip.left);
	ptSurfaceDest.x = rcSrc.left;
	rcSrc.left -= nXOffset;
	rcSrc.right = min(rcSrc.right + nXOffset, rcClip.right);
	rcSrc.right -= nXOffset;
	int nYOffset = -rcSrc.top + dy + ptSurfaceDestOrg.y;
	rcSrc.top = max(rcSrc.top + nYOffset, rcClip.top);
	ptSurfaceDest.y = rcSrc.top;
	rcSrc.top -= nYOffset;
	rcSrc.bottom = min(rcSrc.bottom + nYOffset, rcClip.bottom);
	rcSrc.bottom -= nYOffset;

	// 親の座標(常に (0, 0))
	POINT ptParent;
	ptParent.x = 0;
	ptParent.y = 0;
	
	// 描画
	DrawSurface(pDestSurface, &ptSurfaceDest, &ptParent, &rcSrc);

	// 原点とクリップ矩形の復帰
	pDestSurface->SetOrg(ptSurfaceDestOrg.x, ptSurfaceDestOrg.y);
	pDestSurface->SetClipRect(&rcClip);
}

/////////////////////////////////////////////////////////////////////////////
// public:
//	virtual int CNxSprite::SetZPos(int z)
// 概要: Z 座標を設定
// 引数: int z ... 設定される Z 座標
// 戻値: 以前の Z 座標
////////////////////////////////////////////////////////////////////////////

int CNxSprite::SetZPos(int z)
{
	if (GetParent() == NULL || GetZPos() == z)
		return GetZPos();

	std::swap(z, m_nZPosition);
	GetParent()->m_bDirtyZ = TRUE;
	m_fdwUpdateWithChildren |= updateWithChildren;
	return z;
}


////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::SortChildren(BOOL bDirectOnly)
// 概要: 子を Z 順に並び替える
// 引数: BOOL bDirectOnly ... TRUE ならば直属の子のみをソート。FALSE ならば全て
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::SortChildren(BOOL bDirectOnly)
{
	if (m_bDirtyZ)
	{
		m_bDirtyZ = FALSE;
		std::sort(m_children.begin(), m_children.end(), SpriteZGreat());
	}
	if (bDirectOnly)
		return;

	for (SpriteContainer::iterator it = m_children.begin(); it != m_children.end(); it++)
		(*it)->SortChildren(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSprite::SetPos(int x, int y)
// 概要: 座標を設定
// 引数: int x ... 設定するX座標
//       int y ... 設定するY座標
// 戻値: 成功なら TRUE
////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::SetPos(int x, int y)
{
	if (m_ptPosition.x != x || m_ptPosition.y != y)
	{
		m_ptPosition.x = x;
		m_ptPosition.y = y;
		m_fdwUpdateWithChildren |= updateWithChildren|updateWithChildren_IgnoreClip;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSprite::SetDisplayOrg(int x, int y)
// 概要: スプライトの表示先座標の原点を設定
// 引数: int x ... 設定する原点の X 座標
//       int y ... 設定する原点の Y 座標
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::SetDisplayOrg(int x, int y)
{
	if (m_ptDisplayOrg.x != x || m_ptDisplayOrg.y != y)
	{
		m_ptDisplayOrg.x = x;
		m_ptDisplayOrg.y = y;
		m_fdwUpdateWithChildren |= updateWithChildren;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSprite::SetRect(const RECT* lpRect)
// 概要: スプライトの矩形を設定
// 引数: const RECT* lpRect ... 設定する矩形(NULL ならば空になる)
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::SetRect(const RECT* lpRect)
{
	// lpRect == NULL 又は、矩形が正規化されていない(上下左右が交差している)のであれば空として設定
	if (lpRect == NULL || (lpRect->right - lpRect->left | lpRect->bottom - lpRect->top) < 0)
		::SetRectEmpty(&m_rect);
	else
		m_rect = *lpRect;

	m_fdwUpdateWithChildren |= updateWithChildren;

	// top-level ならば、全て強制再描画
	if (m_pParent == NULL)
		UpdateForceChildren();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::SetSize(int nWidth, int nHeight)
// 概要: スプライトのサイズを変更
// 引数: int nWidth  ... 幅
//       int nHeight ... 高さ
// 戻値: 成功なら TRUE
// 備考: 左上は変らず、右下のみが変更される
//////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::SetSize(int nWidth, int nHeight)
{
	RECT rcTemp;
	rcTemp.top = m_rect.top;
	rcTemp.left = m_rect.left;
	rcTemp.right = rcTemp.left + nWidth;
	rcTemp.bottom = rcTemp.top + nHeight;
	return SetRect(&rcTemp);
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	virtual void CNxSprite::SetVisible(BOOL bVisible)
// 概要: 可視フラグを変更
// 引数: BOOL bVisible ... 可視フラグ(TRUE = 可視/FALSE = 不可視)
// 戻値: 直前の状態
//////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::SetVisible(BOOL bVisible)
{
	if (bVisible != m_bVisible)
	{
		m_fdwUpdateWithChildren |= updateWithChildren|updateWithChildren_IgnoreVisible;
		std::swap(m_bVisible, bVisible);
	}
	return bVisible;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::SetUpdate(const RECT* lpRect = NULL)
// 概要: 更新フラグを立てる
// 引数: const RECT* lpRect ... 更新される領域(SetRect() 内)  NULL ならば全体
// 戻値: なし
//////////////////////////////////////////////////////////////////////////////

void CNxSprite::SetUpdate(const RECT* lpRect)
{
	if (lpRect != NULL)
	{
		// 部分更新: 今までの矩形を含む様に
		::UnionRect(&m_rcUpdate, &m_rcUpdate, lpRect);
		// 自分自身の矩形をはみ出さない様に...
		::IntersectRect(&m_rcUpdate, &m_rcUpdate, &m_rect);

		// 以上は、以下と等価
/*
		m_rcUpdate.left = max(m_rect.left, min(m_rcUpdate.left, lpRect->left));
		m_rcUpdate.right = min(m_rect.right, max(m_rcUpdate.right, lpRect->right));
		m_rcUpdate.top = max(m_rect.top, min(m_rcUpdate.top, lpRect->top));
		m_rcUpdate.bottom = min(m_rect.bottom, max(m_rcUpdate.bottom, lpRect->bottom));
*/
	}
	else
	{	// 全域更新: 転送元矩形そのまま
		m_rcUpdate = m_rect;
	}
	// 自分自身で更新
	m_bUpdateMyself = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::SetClipChildren(BOOL bClip)
// 概要: 子スプライトクリップの有効/無効化を設定
// 引数: BOOL bClip ... 子スプライトを自分自身の矩形へクリップするならば TRUE
// 戻値: 以前の状態
///////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::SetClipChildren(BOOL bClip)
{
	std::swap(bClip, m_bClipChildren);
	m_fdwUpdateWithChildren |= updateWithChildren;
	return bClip;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxSprite* CNxSprite::SetParent(CNxSprite* pNewParent)
// 概要: 親を変更する
// 引数: CNxSprite* pNewParent ... 新しい親
// 戻値: 直前の親。NULL ならば失敗
//////////////////////////////////////////////////////////////////////////////

CNxSprite* CNxSprite::SetParent(CNxSprite* pNewParent)
{
	CNxSprite* pPrevParent = GetParent();

	if (pNewParent == pPrevParent)
		return pNewParent;

	if (pNewParent == this)
	{
		_RPTF0(_CRT_ASSERT, "CNxSprite::SetParent() : 自分自身を親にはできません.\n");
		return NULL;
	}

	// 今までの親から自分を取り除く
	pPrevParent->RemoveChild(this);

	// 新しい親へ接続
	pNewParent->AddChild(this);

	m_pParent = pNewParent;
	return pPrevParent;
}

/////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::SpriteToTop(LPPOINT lpPoint) const
// 概要: スプライト座標を最上位の親の座標へ変換
// 引数: LPPOINT lpPoint ... 変換する POINT 構造体へのポインタ
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////

void CNxSprite::SpriteToTop(LPPOINT lpPoint) const
{
	_ASSERTE(lpPoint != NULL);

	CNxSprite* pParent = GetParent();
	if (pParent != NULL)
	{
		lpPoint->x += pParent->GetXPos();
		lpPoint->y += pParent->GetYPos();
		pParent->SpriteToTop(lpPoint);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::EnumChildren(EnumChildrenProc pfnEnumProc, LPVOID lpContext)
// 概要: 子スプライトを列挙
// 引数: pfnEnumProc      ... 呼び出される関数
//       LPVOID lpContext ... 関数へ渡されるパラメーター
// 戻値: 列挙が中断されたならば FALSE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::EnumChildren(EnumChildrenProc pfnEnumProc, LPVOID lpContext)
{
	for (SpriteContainer::const_iterator it = m_children.begin(); it != m_children.end(); it++)
	{
		if (!(this->*pfnEnumProc)(*it, lpContext))
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::TopToSprite(LPPOINT lpPoint) const
// 概要: 最上位の親の座標をスプライト座標へ変換
// 引数: LPPOINT lpPoint ... 変換座標(POINT 構造体)へのポインタ
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::TopToSprite(LPPOINT lpPoint) const
{
	_ASSERTE(lpPoint != NULL);
	
	CNxSprite* pParent = GetParent();
	if (pParent != NULL)
	{
		lpPoint->x -= pParent->GetXPos();
		lpPoint->y -= pParent->GetYPos();
		pParent->TopToSprite(lpPoint);
	}
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::MoveCenter(const CNxSprite* pParent = NULL)
// 概要: スプライトを他のスプライトの中心へ移動する
// 引数: const CNxSprite *pParent ... 基準とするスプライト(NULL = 親スプライト)
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::MoveCenter(const CNxSprite* pParent)
{
	if (pParent == NULL)
		pParent = GetParent();

	if (pParent == NULL || this == pParent)
		return FALSE;	// 自分と同じか、親を持たない

	int pcx = pParent->GetWidth();
	int pcy = pParent->GetHeight();
	POINT ptParent;
	pParent->GetPos(&ptParent);
	pParent->SpriteToTop(&ptParent);

	int cx = GetWidth();
	int cy = GetHeight();
	POINT ptMyself;
	GetPos(&ptMyself);
	SpriteToTop(&ptMyself);

	if (pcx >= cx)
		ptMyself.x = ptParent.x + (pcx - cx) / 2;
	else
		ptMyself.x = cx - (cx - pcx) / 2;
	if (pcy >= cy)
		ptMyself.y = ptParent.y + (pcy - cy) / 2;
	else
		ptMyself.y = cy - (cy - pcy) / 2;

	TopToSprite(&ptMyself);
	return SetPos(ptMyself.x, ptMyself.y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static BOOL CNxSprite::IntersectClipRect(LPPOINT lpPointDest, LPRECT lpSrcRect, const RECT* lpDestRect)
// 概要: 転送範囲と座標の計算
// 引数: LPPOINT     lpPointDest  ... 転送先座標を受け取る POINT (入力はスプライトの転送先座標)
//       LPRECT      lpSrcRect    ... 転送元矩形を受け取る RECT  (入力はスプライトの転送元矩形)
//       const RECT* lpDestRect   ... 要求転送矩形 (lpPointDest と同じ座標系)
// 戻値: TRUE なら領域有り、FALSE なら領域無し
// lpSrcRect    は、算出した転送元矩形(通常転送元サーフェス上での領域となる)
// lpPointDest  は、転送先座標
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::IntersectClipRect(LPPOINT lpPointDest, LPRECT lpSrcRect, const RECT* lpDestRect)
{
	int nOutside;
	// 垂直方向
	if ((nOutside = lpDestRect->top - lpPointDest->y) > 0)
	{
		lpSrcRect->top += nOutside;
		lpPointDest->y = lpDestRect->top;
	}
	if ((nOutside = lpDestRect->bottom - (lpPointDest->y + (lpSrcRect->bottom - lpSrcRect->top))) < 0)
		lpSrcRect->bottom += nOutside;

	if (lpSrcRect->top >= lpSrcRect->bottom)
		return FALSE;

	// 水平方向
	if ((nOutside = lpDestRect->left - lpPointDest->x) > 0)
	{
		lpSrcRect->left += nOutside;
		lpPointDest->x = lpDestRect->left;
	}
	if ((nOutside = lpDestRect->right - (lpPointDest->x + (lpSrcRect->right - lpSrcRect->left))) < 0) 
		lpSrcRect->right += nOutside;

	if (lpSrcRect->left >= lpSrcRect->right)
		return FALSE;
	else
		return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxSprite::PreUpdate()
// 概要: 更新フラグが調べられる直前に呼び出される仮想関数
// 引数: なし
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::PreUpdate()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxSprite::Update()
// 概要: 更新矩形があれば呼び出される仮想関数
// 引数: なし
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::Update()
{

}

////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxSprite::Draw(CNxSurface* pSurface, const RECT* lpRect) const
// 概要: スプライト描画
// 引数: CNxSurface* pSurface ... 描画先サーフェスへのポインタ
//       const RECT* lpRect   ... スプライト内の描画矩形を示す RECT 構造体へのポインタ
// 戻値: 子スプライトの描画を続けるならば TRUE
////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::Draw(CNxSurface* /*pSurface*/, const RECT* /*lpRect*/) const
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxSprite::DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const
// 概要: スプライト描画(子スプライトの後)
// 引数: CNxSurface* pSurface ... 描画先サーフェスへのポインタ
//       const RECT* lpRect   ... スプライト内の描画矩形を示す RECT 構造体へのポインタ
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::DrawBehindChildren(CNxSurface* /*pSurface*/, const RECT* /*lpRect*/) const
{

}
