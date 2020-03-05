// NxSprite.h: CNxSprite �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000, 2001 S.Ainoguchi
//
// �T�v: �X�v���C�g�̊�{�N���X
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDraw.h"
#include <vector>
#include <set>

namespace NxDrawLocal { class CNxUpdateRegion; }

class CNxSprite
{
public:
	// ZOrder �萔
	enum { Z_Lower = INT_MIN, Z_Highest = INT_MAX };

public:
	
	// �\�z/����
	CNxSprite(CNxSprite* pParent);
	virtual ~CNxSprite();

	// �X�V��`
	void SetUpdate(const RECT* lpRect = NULL);

	// �\�����
	virtual BOOL SetVisible(BOOL bVisible);
	BOOL IsVisible() const;

	// �q�N���b�v
	BOOL SetClipChildren(BOOL bClip);
	BOOL GetClipChildren() const;

	// ���W����
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

	// �X�v���C�g��`���� / �擾
	virtual BOOL SetRect(const RECT* lpRect);	// ��`�̐ݒ�
	BOOL SetSize(int nWidth, int nHeight);		// ��`�̃T�C�Y�ύX(����̍��W�͈ێ�)
	void GetRect(LPRECT lpRect) const;			// ��`�𓾂�
	int GetWidth() const;						// ���𓾂�
	int GetHeight() const;						// �����𓾂�

	// �e�q�֌W���쓙
	virtual CNxSprite* SetParent(CNxSprite* pNewParent);
	CNxSprite* GetParent() const;
	typedef BOOL (CNxSprite::*EnumChildrenProc)(CNxSprite* pSprite, LPVOID lpContext);
	BOOL EnumChildren(EnumChildrenProc pfnEnumProc, LPVOID lpContext);
	void SortChildren(BOOL bDirectOnly /* ���n�̎q�̂݃\�[�g����Ȃ�� TRUE */);

	// ���W�ϊ�
	void SpriteToTop(LPPOINT lpPoint) const;
	void TopToSprite(LPPOINT lpPoint) const;

	// �X�v���C�g�`��
	void DrawSurface(CNxSurface* pSurface, int dx, int dy, const RECT* lpSrcRect = NULL) const;

protected:

	// �`��֌W�̉��z�֐�
	virtual void PreUpdate();
	virtual void Update();
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;
	virtual void DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const;

	// �X�V���ꂽ�̈�̎擾
	BOOL GetUpdateRegion(NxDrawLocal::CNxUpdateRegion* pRegion, BOOL bDoPreUpdate);

private:

	BOOL GetUpdateRegion(NxDrawLocal::CNxUpdateRegion* pRegion, POINT ptParent, const RECT* lpRectParent, BOOL bDoPreUpdate);
	void DrawSurface(CNxSurface* pSurface, const POINT* lpPointSurfaceDest, const POINT* lpPointParent, const RECT* lpRect) const;
	void UpdateForceChildren();
	BOOL UpdateWithChildren(BOOL bIgnoreVisible = FALSE, BOOL bIgnoreClip = FALSE);
	void AddChild(CNxSprite* pSprite);
	BOOL RemoveChild(CNxSprite* pSprite);
	inline static BOOL IntersectClipRect(LPPOINT lpPointDest, LPRECT lpSrcRect, const RECT* lpDestRect);

	// �X�v���C�g���
	CNxSprite* m_pParent;		// �e�X�v���C�g
	RECT  m_rect;				// �X�v���C�g�̋�`
	POINT m_ptPosition;			// �e�X�v���C�g����̑��΍��W
	BOOL  m_bVisible;			// ��/�s��
	RECT  m_rcUpdate;			// �X�V���ׂ��X�v���C�g���̋�`
	POINT m_ptDisplayOrg;		// �\����I�t�Z�b�g
	int	  m_nZPosition;			// Z �ʒu
	BOOL  m_bClipChildren;		// �q�X�v���C�g���N���b�v����Ȃ�� TRUE
	BOOL  m_bUpdateMyself;		// �������g�ōX�V�����Ȃ�� TRUE (CNxSprite::Update() ���Ăяo�����)
	RECT  m_rcForce;			// �����I�ɍX�V�����`
	RECT  m_rcPrevFullDraw;		// �ȑO�ɃX�v���C�g�S���`�悵���X�V�t���O��`
	BOOL  m_bDirtyZ;			// Z �\�[�g�̕K�v������� TRUE

	enum
	{
		updateWithChildren				 = 0x00000001,	// Update ����
		updateWithChildren_IgnoreClip    = 0x00000002,	// �q�̃N���b�v�𖳎�
		updateWithChildren_IgnoreVisible = 0x00000004,	// �\����Ԃ𖳎�(invisible -> visible �̐؂�ւ��ł͕K�v)
	};
	// ����X�V���ɕK�v�Ƃ��鏈����(UpdateWithChildrenXXXXX ��)�_���a�Ŏ���
	// ���̓��e�ɂ���āACNxSprite::SetDrawRect ���� UpdateWithChildren() �֐�����x�����Ăяo�����
	DWORD m_fdwUpdateWithChildren;

	struct SpriteZGreat
	{
		bool operator()(const CNxSprite* x, const CNxSprite* y) const
		{
			return x->GetZPos() < y->GetZPos();
		}
	};
	// �q�X�v���C�g�R���e�i
	typedef std::vector<CNxSprite*> SpriteContainer;
	SpriteContainer m_children;

	CNxSprite(const CNxSprite&);
	CNxSprite& operator=(const CNxSprite&);
};

//////////////////////////////////////////////////////////////////////////////////////////
// inline �֐�

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
