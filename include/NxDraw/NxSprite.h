// NxSprite.h: CNxSprite クラスのインターフェイス
// Copyright(c) 2000, 2001 S.Ainoguchi
//
// 概要: スプライトの基本クラス
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDraw.h"
#include <vector>
#include <set>

namespace NxDrawLocal { class CNxUpdateRegion; }

class CNxSprite
{
public:
	// ZOrder 定数
	enum { Z_Lower = INT_MIN, Z_Highest = INT_MAX };

public:
	
	// 構築/消滅
	CNxSprite(CNxSprite* pParent);
	virtual ~CNxSprite();

	// 更新矩形
	void SetUpdate(const RECT* lpRect = NULL);

	// 表示状態
	virtual BOOL SetVisible(BOOL bVisible);
	BOOL IsVisible() const;

	// 子クリップ
	BOOL SetClipChildren(BOOL bClip);
	BOOL GetClipChildren() const;

	// 座標操作
	virtual BOOL SetPos(int x, int y);
	void GetPos(LPPOINT lpPoint) const;
	BOOL OffsetPos(int x, int y);
	BOOL MoveCenter(const CNxSprite* pParent = NULL);
	int GetXPos() const;
	int GetYPos() const;
	virtual int SetZPos(int z);
	int GetZPos() const;
	virtual BOOL SetDisplayOrg(int x, int y);
	void GetDisplayOrg(LPPOINT lpPoint) const;

	// スプライト矩形操作 / 取得
	virtual BOOL SetRect(const RECT* lpRect);	// 矩形の設定
	BOOL SetSize(int nWidth, int nHeight);		// 矩形のサイズ変更(左上の座標は維持)
	void GetRect(LPRECT lpRect) const;			// 矩形を得る
	int GetWidth() const;						// 幅を得る
	int GetHeight() const;						// 高さを得る

	// 親子関係操作等
	virtual CNxSprite* SetParent(CNxSprite* pNewParent);
	CNxSprite* GetParent() const;
	typedef BOOL (CNxSprite::*EnumChildrenProc)(CNxSprite* pSprite, LPVOID lpContext);
	BOOL EnumChildren(EnumChildrenProc pfnEnumProc, LPVOID lpContext);
	void SortChildren(BOOL bDirectOnly /* 直系の子のみソートするならば TRUE */);

	// 座標変換
	void SpriteToTop(LPPOINT lpPoint) const;
	void TopToSprite(LPPOINT lpPoint) const;

	// スプライト描画
	void DrawSurface(CNxSurface* pSurface, int dx, int dy, const RECT* lpSrcRect = NULL) const;

protected:

	// 描画関係の仮想関数
	virtual void PreUpdate();
	virtual void Update();
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;
	virtual void DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const;

	// 更新された領域の取得
	BOOL GetUpdateRegion(NxDrawLocal::CNxUpdateRegion* pRegion, BOOL bDoPreUpdate);

private:

	BOOL GetUpdateRegion(NxDrawLocal::CNxUpdateRegion* pRegion, POINT ptParent, const RECT* lpRectParent, BOOL bDoPreUpdate);
	void DrawSurface(CNxSurface* pSurface, const POINT* lpPointSurfaceDest, const POINT* lpPointParent, const RECT* lpRect) const;
	void UpdateForceChildren();
	BOOL UpdateWithChildren(BOOL bIgnoreVisible = FALSE, BOOL bIgnoreClip = FALSE);
	void AddChild(CNxSprite* pSprite);
	BOOL RemoveChild(CNxSprite* pSprite);
	inline static BOOL IntersectClipRect(LPPOINT lpPointDest, LPRECT lpSrcRect, const RECT* lpDestRect);

	// スプライト情報
	CNxSprite* m_pParent;		// 親スプライト
	RECT  m_rect;				// スプライトの矩形
	POINT m_ptPosition;			// 親スプライトからの相対座標
	BOOL  m_bVisible;			// 可視/不可視
	RECT  m_rcUpdate;			// 更新すべきスプライト内の矩形
	POINT m_ptDisplayOrg;		// 表示先オフセット
	int	  m_nZPosition;			// Z 位置
	BOOL  m_bClipChildren;		// 子スプライトをクリップするならば TRUE
	BOOL  m_bUpdateMyself;		// 自分自身で更新したならば TRUE (CNxSprite::Update() が呼び出される)
	RECT  m_rcForce;			// 強制的に更新する矩形
	RECT  m_rcPrevFullDraw;		// 以前にスプライト全域を描画した更新フラグ矩形
	BOOL  m_bDirtyZ;			// Z ソートの必要があれば TRUE

	enum
	{
		updateWithChildren				 = 0x00000001,	// Update する
		updateWithChildren_IgnoreClip    = 0x00000002,	// 子のクリップを無視
		updateWithChildren_IgnoreVisible = 0x00000004,	// 表示状態を無視(invisible -> visible の切り替えでは必要)
	};
	// 次回更新時に必要とする処理を(UpdateWithChildrenXXXXX の)論理和で示す
	// この内容によって、CNxSprite::SetDrawRect から UpdateWithChildren() 関数が一度だけ呼び出される
	DWORD m_fdwUpdateWithChildren;

	struct SpriteZGreat
	{
		bool operator()(const CNxSprite* x, const CNxSprite* y) const
		{
			return x->GetZPos() < y->GetZPos();
		}
	};
	// 子スプライトコンテナ
	typedef std::vector<CNxSprite*> SpriteContainer;
	SpriteContainer m_children;

	CNxSprite(const CNxSprite&);
	CNxSprite& operator=(const CNxSprite&);
};

//////////////////////////////////////////////////////////////////////////////////////////
// inline 関数

inline CNxSprite* CNxSprite::GetParent() const {
	return m_pParent; }

inline int CNxSprite::GetZPos() const {
	return m_nZPosition; }

inline BOOL CNxSprite::IsVisible() const {
	return m_bVisible; }

inline BOOL CNxSprite::OffsetPos(int x, int y) {
	return SetPos(m_ptPosition.x + x, m_ptPosition.y + y); }

inline int CNxSprite::GetXPos() const {
	return m_ptPosition.x; }

inline int CNxSprite::GetYPos() const {
	return m_ptPosition.y; }

inline void CNxSprite::GetPos(LPPOINT lpPoint) const {
	_ASSERTE(lpPoint != NULL);
	*lpPoint = m_ptPosition; }

inline int CNxSprite::GetWidth() const {
	return m_rect.right - m_rect.left; }

inline int CNxSprite::GetHeight() const {
	return m_rect.bottom - m_rect.top; }

inline void CNxSprite::GetRect(LPRECT lpRect) const {
	_ASSERTE(lpRect != NULL);
	*lpRect = m_rect; }

inline BOOL CNxSprite::GetClipChildren() const {
	return m_bClipChildren; }

inline void CNxSprite::GetDisplayOrg(LPPOINT lpPoint) const {
	_ASSERTE(lpPoint != NULL);
	*lpPoint = m_ptDisplayOrg; }
