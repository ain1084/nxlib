// NxSprite.cpp: CNxSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000, 2001 S.Ainoguchi
//
// �T�v: �X�v���C�g�̊�{�N���X
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
// �T�v: CNxSprite �N���X�̃R���X�g���N�^
// ����: CNxSprite* pParent ... �e�X�v���C�g�ւ̃|�C���^(NULL��)
// �ߒl: ---
//////////////////////////////////////////////////////////////////////

CNxSprite::CNxSprite(CNxSprite* pParent)
 : m_pParent(pParent)				// �e�X�v���C�g�ւ̃|�C���^(null = �e����)
 , m_nZPosition(0)					// Z ���W
 , m_bVisible(TRUE)					// ���t���O
 , m_bClipChildren(TRUE)			// �q�N���b�v�s�Ȃ�
 , m_bUpdateMyself(FALSE)			// �����ł͍X�V���Ă��Ȃ�
 , m_bDirtyZ(FALSE)					// Z���W�ύX�t���O
 , m_fdwUpdateWithChildren(0)		// �����`��֘A�t���O
{
	m_ptPosition.x = 0;			// �X�v���C�g���W
	m_ptPosition.y = 0;
	m_ptDisplayOrg.x = 0;		// �\�����_
	m_ptDisplayOrg.y = 0;

	::SetRectEmpty(&m_rect);			// �X�v���C�g��`
	::SetRectEmpty(&m_rcForce);			// �����X�V�̋�`
	::SetRectEmpty(&m_rcUpdate);		// �X�V������`
	::SetRectEmpty(&m_rcPrevFullDraw);	// �ȑO�ɑS��`�悵����`

	// �w�肳�ꂽ�e�֎q�Ƃ��Ēǉ�
	if (m_pParent != NULL)
		m_pParent->AddChild(this);
}

/////////////////////////////////////////////////////////////////////
// public:
//	CNxSprite::~CNxSprite()
// �T�v: CNxSprite �N���X�̃f�X�g���N�^
// ����: ---
// �ߒl: ---
/////////////////////////////////////////////////////////////////////

CNxSprite::~CNxSprite()
{
	// �q�X�v���C�g��S�č폜
	while (!m_children.empty())
		delete m_children.back();

	// �e�X�v���C�g���X�g���玩�����폜
	if (m_pParent != NULL)
		m_pParent->RemoveChild(this);
}

////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxSprite::AddChild(CNxSprite* pSprite)
// �T�v: �q�X�v���C�g���X�g�փX�v���C�g��ǉ�����
// ����: CNxSprite* pSprite ... �ǉ�����X�v���C�g
// �ߒl: �Ȃ�
// ���l: �ǉ��Ɠ����� Z �\�[�g�����
////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::AddChild(CNxSprite* pSprite)
{
	// �e�q�֌W���l�X�g���Ă��܂��ƁA�F�X�ȂƂ���Ŗ������[�v��
	// �����N�����\�������邪�A���݂̂Ƃ���`�F�b�N�͍s�Ȃ��Ă��Ȃ�
	_ASSERTE(pSprite != NULL);
	
	// �Ō�ɑ}��
	m_children.push_back(pSprite);

	// �q�̋�`�X�V��v��
	m_fdwUpdateWithChildren |= updateWithChildren;

	// Z�\�[�g���s��
	m_bDirtyZ = TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSprite::RemoveChild(CNxSprite* pSprite)
// �T�v: �q�X�v���C�g���X�g����X�v���C�g���폜����
// ����: CNxSprite *pSprite ... �폜����X�v���C�g
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::RemoveChild(CNxSprite* pSprite)
{
	_ASSERTE(pSprite != NULL);
	
	SpriteContainer::iterator it;
	SpriteContainer::iterator itend = m_children.end();
	it = std::find(m_children.begin(), itend, pSprite);
	if (it == itend)
	{
		_RPTF0(_CRT_WARN, "CNxSprite : ���݂��Ȃ��X�v���C�g�̍폜���v������܂���.\n");
		return FALSE;
	}
	(*it)->UpdateWithChildren(TRUE, TRUE);						// �폜�����X�v���C�g(�ȉ�)�̋�`�����
	::UnionRect(&m_rcForce, &m_rcForce, &(*it)->m_rcForce);		// ���������`���������g�֒ǉ�
	m_children.erase(it);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSprite::UpdateForceChildren()
// �T�v: �������g�ƁA�S�Ă̎q�X�v���C�g�������I�ɍX�V
// ����: �Ȃ�
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::UpdateForceChildren()
{
	// �X�V��`��S�̂Ƃ��Đݒ�
	// �s���Ă��鎖�́ASetUpdate() �Ƃ��܂�ς�Ȃ����A������ SetUpdate() ���Ăяo���Ȃ��̂́A
	// m_bUpdateMyself �� true �ɂȂ��Ă��܂��A��� CNxSprite::Update() ������ɌĂяo�����
	// ���܂��̂�h����
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
// �T�v: �q���܂߂Đ������`�悳���l�ɁAm_rcForce (�����X�V��`) ��ݒ肷��
// ����: BOOL bIgnoreVisible ... �s���̃X�v���C�g���ΏۂƂ���Ȃ�� TRUE(invisible -> visible �ł͕K�{)
//       BOOL bIgnoreClip    ... �q�X�v���C�g�̃N���b�v�t���O�𖳎�����Ȃ�� TRUE
// �ߒl: ��`������Ȃ�� TRUE
// ���l: �������g�����ł͂Ȃ��A�q�X�v���C�g�։e����^���鑀���ɂ� m_fdwUpdateWithChildren ���K�v
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::UpdateWithChildren(BOOL bIgnoreVisible, BOOL bIgnoreClip)
{
	// ���̃X�v���C�g�ŗv������Ă���t���O���l��
	bIgnoreClip |= (m_fdwUpdateWithChildren & updateWithChildren_IgnoreClip);
	bIgnoreVisible |= (m_fdwUpdateWithChildren & updateWithChildren_IgnoreVisible);

	// �q�X�v���C�g�� CNxSprite::SetDrawRect ���ŉ��x�����ʂɏ�������Ȃ��l�Ƀt���O���N���A
	m_fdwUpdateWithChildren = 0;

	if (bIgnoreVisible | m_bVisible)
	{	// �\������Ă���(���͖���)
		if (bIgnoreClip | !m_bClipChildren)
		{	// �q�N���b�v���Ȃ�(���͖���)
			for (SpriteContainer::iterator it = m_children.begin(); it != m_children.end(); it++)
			{
				if ((*it)->UpdateWithChildren(bIgnoreVisible, bIgnoreClip))
				{	// �q�̋�`����������
					::UnionRect(&m_rcForce, &m_rcForce, &(*it)->m_rcForce);
					// �q�̋����X�V��`���N���A(�d�Ȃ�����`�𖳑ʂɍX�V����̂�h��)
					(*it)->m_rcForce.top = 0;
					(*it)->m_rcForce.left = 0;
					(*it)->m_rcForce.right = 0;
					(*it)->m_rcForce.bottom = 0;
				}
			}
		}
		// �����̋�`���ǉ�
		m_rcUpdate = m_rect;
		::UnionRect(&m_rcForce, &m_rcForce, &m_rcPrevFullDraw);
		return TRUE;
	}
	return FALSE;
}	


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::GetUpdateRegion(CNxUpdateRegion* pRegion, BOOL bDoPreUpdate)
// �T�v: �S�ẴX�v���C�g�̍X�V�̈���擾
// ����: CNxUpdateRegion* pRegion ... �X�V�̈���󂯎��ACNxUpdateRegion �h���N���X�ւ̃|�C���^
//       BOOL bDoPreUpdate        ... CNxSprite::PreUpdate() ���Ăяo���Ȃ�� TRUE
// �ߒl: �X�V�t���O����������(�X�V��������������)�Ȃ�� TRUE
// ���l: private �֐��̊ȈՔ�
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
// �T�v: �S�ẴX�v���C�g�̍X�V�̈���擾
// ����: CNxUpdateRegion* pRegion ... �X�V�̈���󂯎��ACNxUpdateRegion �h���N���X�ւ̃|�C���^
//       POINT  ptParent          ... �e�X�v���C�g�̍��W
//       const RECT* lpRectParent ... �e�X�v���C�g�̋�`
//       BOOL bDoPreUpdate        ... CNxSprite::PreUpdate() ���Ăяo���Ȃ�� TRUE
// �ߒl: �X�V�t���O����������(�X�V��������������)�Ȃ�� TRUE
// ���l: lpRectParent �̓X�v���C�g���������݉\�Ȕ͈͂������BCNxDrawRect �̎����͈͂𒴂��Ă͂Ȃ�Ȃ�
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::GetUpdateRegion(NxDrawLocal::CNxUpdateRegion* pRegion, POINT ptParent, const RECT* lpRectParent, BOOL bDoPreUpdate)
{
	// �X�V�O����
	if (bDoPreUpdate)
		PreUpdate();

	if (m_fdwUpdateWithChildren != 0)
	{	// �q�X�v���C�g�����A��X�V
		UpdateWithChildren();
		// ��̊֐����� m_fdwUpdateWithChildren �́A
		// �K���[���ɂȂ�̂ŁA�����ł̓N���A����K�v�͂Ȃ�
	}

	BOOL bUpdateFlag = FALSE;

	// �폜���ꂽ�q�X�v���C�g�̗̈擙�������I�ɍX�V
	//
	// if ((rect.right - rect.left | rect.bottom - rect.top) > 0) �ł́A��`����ł��邩�𒲂ׂĂ��邪
	// IsRectEmpty() �Ƃ͈قȂ�A�������[���łȂ���΁A��̋�`�Ƃ��Ă͔��肳��Ȃ��BIsRectEmpty() �ł́A
	// �ǂ��炩���[���ł���΋�Ƃ��Ĉ�����(�ǂ���ɂ����ŕʁX�ɔ��肳���̂Ŗ��͖���)�B

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
	{	// �e�X�v���C�g�L��(�ŏ�ʂ̐e�ł͂Ȃ�)

		// �X�v���C�g���\������Ă��Ȃ��Ȃ�΁A����Ȍ�̏����͕s�v
		if (!m_bVisible)
			return bUpdateFlag;
	
		// ���̃X�v���C�g�̍�����W
		ptParent.x += m_ptPosition.x + m_ptDisplayOrg.x;
		ptParent.y += m_ptPosition.y + m_ptDisplayOrg.y;
	}

	// �X�V�t���O�𗧂Ă�
	if ((m_rcUpdate.right - m_rcUpdate.left | m_rcUpdate.bottom - m_rcUpdate.top) > 0)
	{
		RECT rcSrc;
		POINT ptDest;
		int nRectDiff;

		// rcSrc = m_rcUpdate;
		// m_rcUpdate == m_rect �Ȃ�΁AnRectDiff == 0 (�S��X�V�̔��ʂɗp����)
		// ptDest �́AptParent + (m_rcUpdate - m_rect) ���ݒ肳���

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
		{	// ��`�͗L��
			rcSrc.right = rcSrc.right - rcSrc.left + ptDest.x;
			rcSrc.bottom = rcSrc.bottom - rcSrc.top + ptDest.y;
			rcSrc.left = ptDest.x;
			rcSrc.top = ptDest.y;

			// �S��X�V��`(m_rcUpdate == m_rect) �Ȃ�Εۑ�����
			if (nRectDiff == 0)
				m_rcPrevFullDraw = rcSrc;

			// ����̕`��͈͂̃t���O�𗧂Ă�
			if (pRegion != NULL)
				pRegion->AddRect(&rcSrc);

			// �������g�ɂ��X�V�Ȃ�΍X�V�֐����Ăяo��
			if (m_bUpdateMyself)
			{
				m_bUpdateMyself = FALSE;
				Update();
			}
			bUpdateFlag = TRUE;				// (�X�V�ς�)
		}
		else
		{	// ��`������
			// ���񂪑S��X�V(m_rcUpdate == m_rect)�������Ȃ��(�O���)��`���N���A
			if (nRectDiff == 0)
			{
				m_rcPrevFullDraw.top = 0;
				m_rcPrevFullDraw.left = 0;
				m_rcPrevFullDraw.right = 0;
				m_rcPrevFullDraw.bottom = 0;
			}
		}
		// �`��v���N���A
		m_rcUpdate.top = 0;
		m_rcUpdate.left = 0;
		m_rcUpdate.right = 0;
		m_rcUpdate.bottom = 0;
	}

	if (!m_bClipChildren || (m_rcPrevFullDraw.right - m_rcPrevFullDraw.left | m_rcPrevFullDraw.bottom - m_rcPrevFullDraw.top) > 0)
	{
		if (m_pParent == NULL)
		{	// �ŏ�ʂ̐e�Ȃ�΍��W�������ňړ�
			ptParent.x += m_ptPosition.x + m_ptDisplayOrg.x;
			ptParent.y += m_ptPosition.y + m_ptDisplayOrg.y;
		}
		else
		{	// �q�N���b�v����Ȃ玩����(���񂪖����Ȃ�Β��O��)��`�A���Ȃ��Ȃ�Ύ����̐e�̋�`(lpRectParent)
			if (m_bClipChildren)
				lpRectParent = &m_rcPrevFullDraw;
		}	

		// �q�X�v���C�g�� Z �\�[�g(�����̎q�̂�)
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
// �T�v: �X�v���C�g��`��(�����p)
// ����: CNxSurface* pSruface ... �`���̃T�[�t�F�X
//       const POINT* lpPointSurfaceDest  ... �T�[�t�F�X��ւ̕\�����W������ POINT �\���̂ւ̃|�C���^
//       const POINT* lpPointParent       ... �e�X�v���C�g�̍��W������ POINT �\���̂ւ̃|�C���^
//       const RECT* lpRect               ... �`�悷��X�v���C�g��`
// �ߒl: �Ȃ�
// ���l: �������ŏ�ʂ̐e�Ȃ�΁A�s���ł� Draw() �͌Ăяo��
///////////////////////////////////////////////////////////////////////////////////////////////////////  

void CNxSprite::DrawSurface(CNxSurface* pSurface, const POINT* lpPointSurfaceDest,
							const POINT *lpPointParent, const RECT* lpRect) const
{
	POINT ptParent;

	// �������s���Ȃ�� return (�������A�������ŏ�̐e�ł���ꍇ������)
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
	POINT ptDest = ptParent;		// �X�v���C�g�̕`�����W(���̎��_�ł͎b��)
	RECT rcSrcClip = m_rect;		// IntersectClipRect ��̓X�v���C�g�̓]����`
	SpriteContainer::const_iterator it;
		
	if (!IntersectClipRect(&ptDest, &rcSrcClip, lpRect))	// (inline �֐�)
	{	// �`���`�Ȃ�
		if (m_pParent == NULL)
		{	// [�ŏ�ʃX�v���C�g�̏ꍇ]
			// �s���Ȃ�Ζ߂�(�ŏ�ʃX�v���C�g�Ȃ�΁A�s���ł������܂œ��B�����)
			if (!m_bVisible)
				return;

			// �����ō��W���ړ�
			ptParent.x += (m_ptPosition.x + m_ptDisplayOrg.x);
			ptParent.y += (m_ptPosition.y + m_ptDisplayOrg.y);
		}
		else
		{	// [�q�X�v���C�g�̏ꍇ]
			// �`���`�������A�q���N���b�v����Ȃ�΁A�ȉ��̎q�͉����`��ł��Ȃ��B
			if (m_bClipChildren)
				return;
		}
		// �q�X�v���C�g���ŉ��ʂ���\������
		for (it = m_children.begin(); it != m_children.end(); it++)
			(*it)->DrawSurface(pSurface, lpPointSurfaceDest, &ptParent, lpRect);
	}
	else
	{
		// �]�����ׂ��X�v���C�g����`(rcSrcClip)����A�]���挴�_���Z�o
		POINT ptDestOrg;
		ptDestOrg.x = lpPointSurfaceDest->x + (ptParent.x - m_rect.left - lpRect->left);
		ptDestOrg.y = lpPointSurfaceDest->y + (ptParent.y - m_rect.top - lpRect->top);

		// �]����N���b�s���O��`���Z�o
		RECT rcDestClip;
		rcDestClip.top = rcSrcClip.top + ptDestOrg.y;
		rcDestClip.left = rcSrcClip.left + ptDestOrg.x;
		rcDestClip.right = rcSrcClip.right + ptDestOrg.x;
		rcDestClip.bottom = rcSrcClip.bottom + ptDestOrg.y;

		pSurface->SetOrg(ptDestOrg.x, ptDestOrg.y);	// ���_
		pSurface->SetClipRect(&rcDestClip);			// �N���b�s���O��`
		
		// �`��
		BOOL bDrawChildren = Draw(pSurface, &rcSrcClip);

		if (m_pParent == NULL)
		{
			// �s���ł� m_pParent == NULL �Ȃ�΂����܂œ��B����B
			// �������g���s���ł���Ύq�̕\���͕s�v
			if (!m_bVisible)
				return;

			ptParent.x += (m_ptPosition.x + m_ptDisplayOrg.x);
			ptParent.y += (m_ptPosition.y + m_ptDisplayOrg.y);
		}
		else
		{
			// �q�X�v���C�g���N���b�v����Ȃ�Ύ����̋�`�A
			// �����łȂ��Ȃ�ΐe�̋�`�����̂܂܎q�X�v���C�g�֓n��

			if (m_bClipChildren)
			{	// �e��`���ɏk��������`&���W���v�Z
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
		// �q�X�v���C�g���ŉ��ʂ���\������
		if (bDrawChildren)
		{
			for (it = m_children.begin(); it != m_children.end(); it++)
				(*it)->DrawSurface(pSurface, lpPointSurfaceDest, &ptParent, lpRect);
		}
		// �q�X�v���C�g�\����� DrawBehindChildren() �֐����Ăяo��
		pSurface->SetOrg(ptDestOrg.x, ptDestOrg.y);
		pSurface->SetClipRect(&rcDestClip);
		DrawBehindChildren(pSurface, &rcSrcClip);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::DrawSurface(CNxSurface* pDestSurface, int dx, int dy, const RECT* lpSrcRect = NULL) const
// �T�v: �X�v���C�g��`�� (public)
// ����: CNxSurface* pDestSurface .... �`���T�[�t�F�X�ւ̃|�C���^
//       int dx                   .... �T�[�t�F�X��̕`��� X ���W
//       int dy                   .... �T�[�t�F�X��̕`��� Y ���W
//       const RECT* lpSrcRect    .... �`�悳���X�v���C�g��`
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::DrawSurface(CNxSurface* pDestSurface, int dx, int dy, const RECT* lpSrcRect) const
{
	// ���O�̃N���b�v
	RECT rcSrc;
	if (lpSrcRect == NULL)
	{	// lpSrcRect ���ȗ�����Ă�����A�X�v���C�g�̋�`���g�p
		GetRect(&rcSrc);
	}
	else
		rcSrc = *lpSrcRect;

	RECT rcClip;
	pDestSurface->GetClipRect(&rcClip);
	POINT ptSurfaceDestOrg;
	pDestSurface->GetOrg(&ptSurfaceDestOrg);

	POINT ptSurfaceDest;	// �`������W

	// �N���b�v(��x (0,0) �֍��킹�Ă����r)
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

	// �e�̍��W(��� (0, 0))
	POINT ptParent;
	ptParent.x = 0;
	ptParent.y = 0;
	
	// �`��
	DrawSurface(pDestSurface, &ptSurfaceDest, &ptParent, &rcSrc);

	// ���_�ƃN���b�v��`�̕��A
	pDestSurface->SetOrg(ptSurfaceDestOrg.x, ptSurfaceDestOrg.y);
	pDestSurface->SetClipRect(&rcClip);
}

/////////////////////////////////////////////////////////////////////////////
// public:
//	virtual int CNxSprite::SetZPos(int z)
// �T�v: Z ���W��ݒ�
// ����: int z ... �ݒ肳��� Z ���W
// �ߒl: �ȑO�� Z ���W
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
// �T�v: �q�� Z ���ɕ��ёւ���
// ����: BOOL bDirectOnly ... TRUE �Ȃ�Β����̎q�݂̂��\�[�g�BFALSE �Ȃ�ΑS��
// �ߒl: �Ȃ�
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
// �T�v: ���W��ݒ�
// ����: int x ... �ݒ肷��X���W
//       int y ... �ݒ肷��Y���W
// �ߒl: �����Ȃ� TRUE
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
// �T�v: �X�v���C�g�̕\������W�̌��_��ݒ�
// ����: int x ... �ݒ肷�錴�_�� X ���W
//       int y ... �ݒ肷�錴�_�� Y ���W
// �ߒl: �����Ȃ� TRUE
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
// �T�v: �X�v���C�g�̋�`��ݒ�
// ����: const RECT* lpRect ... �ݒ肷���`(NULL �Ȃ�΋�ɂȂ�)
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::SetRect(const RECT* lpRect)
{
	// lpRect == NULL ���́A��`�����K������Ă��Ȃ�(�㉺���E���������Ă���)�̂ł���΋�Ƃ��Đݒ�
	if (lpRect == NULL || (lpRect->right - lpRect->left | lpRect->bottom - lpRect->top) < 0)
		::SetRectEmpty(&m_rect);
	else
		m_rect = *lpRect;

	m_fdwUpdateWithChildren |= updateWithChildren;

	// top-level �Ȃ�΁A�S�ċ����ĕ`��
	if (m_pParent == NULL)
		UpdateForceChildren();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::SetSize(int nWidth, int nHeight)
// �T�v: �X�v���C�g�̃T�C�Y��ύX
// ����: int nWidth  ... ��
//       int nHeight ... ����
// �ߒl: �����Ȃ� TRUE
// ���l: ����͕ς炸�A�E���݂̂��ύX�����
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
// �T�v: ���t���O��ύX
// ����: BOOL bVisible ... ���t���O(TRUE = ��/FALSE = �s��)
// �ߒl: ���O�̏��
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
// �T�v: �X�V�t���O�𗧂Ă�
// ����: const RECT* lpRect ... �X�V�����̈�(SetRect() ��)  NULL �Ȃ�ΑS��
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////////////

void CNxSprite::SetUpdate(const RECT* lpRect)
{
	if (lpRect != NULL)
	{
		// �����X�V: ���܂ł̋�`���܂ޗl��
		::UnionRect(&m_rcUpdate, &m_rcUpdate, lpRect);
		// �������g�̋�`���͂ݏo���Ȃ��l��...
		::IntersectRect(&m_rcUpdate, &m_rcUpdate, &m_rect);

		// �ȏ�́A�ȉ��Ɠ���
/*
		m_rcUpdate.left = max(m_rect.left, min(m_rcUpdate.left, lpRect->left));
		m_rcUpdate.right = min(m_rect.right, max(m_rcUpdate.right, lpRect->right));
		m_rcUpdate.top = max(m_rect.top, min(m_rcUpdate.top, lpRect->top));
		m_rcUpdate.bottom = min(m_rect.bottom, max(m_rcUpdate.bottom, lpRect->bottom));
*/
	}
	else
	{	// �S��X�V: �]������`���̂܂�
		m_rcUpdate = m_rect;
	}
	// �������g�ōX�V
	m_bUpdateMyself = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSprite::SetClipChildren(BOOL bClip)
// �T�v: �q�X�v���C�g�N���b�v�̗L��/��������ݒ�
// ����: BOOL bClip ... �q�X�v���C�g���������g�̋�`�փN���b�v����Ȃ�� TRUE
// �ߒl: �ȑO�̏��
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
// �T�v: �e��ύX����
// ����: CNxSprite* pNewParent ... �V�����e
// �ߒl: ���O�̐e�BNULL �Ȃ�Ύ��s
//////////////////////////////////////////////////////////////////////////////

CNxSprite* CNxSprite::SetParent(CNxSprite* pNewParent)
{
	CNxSprite* pPrevParent = GetParent();

	if (pNewParent == pPrevParent)
		return pNewParent;

	if (pNewParent == this)
	{
		_RPTF0(_CRT_ASSERT, "CNxSprite::SetParent() : �������g��e�ɂ͂ł��܂���.\n");
		return NULL;
	}

	// ���܂ł̐e���玩������菜��
	pPrevParent->RemoveChild(this);

	// �V�����e�֐ڑ�
	pNewParent->AddChild(this);

	m_pParent = pNewParent;
	return pPrevParent;
}

/////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::SpriteToTop(LPPOINT lpPoint) const
// �T�v: �X�v���C�g���W���ŏ�ʂ̐e�̍��W�֕ϊ�
// ����: LPPOINT lpPoint ... �ϊ����� POINT �\���̂ւ̃|�C���^
// �ߒl: �Ȃ�
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
// �T�v: �q�X�v���C�g���
// ����: pfnEnumProc      ... �Ăяo�����֐�
//       LPVOID lpContext ... �֐��֓n�����p�����[�^�[
// �ߒl: �񋓂����f���ꂽ�Ȃ�� FALSE
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
// �T�v: �ŏ�ʂ̐e�̍��W���X�v���C�g���W�֕ϊ�
// ����: LPPOINT lpPoint ... �ϊ����W(POINT �\����)�ւ̃|�C���^
// �ߒl: �Ȃ�
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
// �T�v: �X�v���C�g�𑼂̃X�v���C�g�̒��S�ֈړ�����
// ����: const CNxSprite *pParent ... ��Ƃ���X�v���C�g(NULL = �e�X�v���C�g)
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::MoveCenter(const CNxSprite* pParent)
{
	if (pParent == NULL)
		pParent = GetParent();

	if (pParent == NULL || this == pParent)
		return FALSE;	// �����Ɠ������A�e�������Ȃ�

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
// �T�v: �]���͈͂ƍ��W�̌v�Z
// ����: LPPOINT     lpPointDest  ... �]������W���󂯎�� POINT (���͂̓X�v���C�g�̓]������W)
//       LPRECT      lpSrcRect    ... �]������`���󂯎�� RECT  (���͂̓X�v���C�g�̓]������`)
//       const RECT* lpDestRect   ... �v���]����` (lpPointDest �Ɠ������W�n)
// �ߒl: TRUE �Ȃ�̈�L��AFALSE �Ȃ�̈斳��
// lpSrcRect    �́A�Z�o�����]������`(�ʏ�]�����T�[�t�F�X��ł̗̈�ƂȂ�)
// lpPointDest  �́A�]������W
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::IntersectClipRect(LPPOINT lpPointDest, LPRECT lpSrcRect, const RECT* lpDestRect)
{
	int nOutside;
	// ��������
	if ((nOutside = lpDestRect->top - lpPointDest->y) > 0)
	{
		lpSrcRect->top += nOutside;
		lpPointDest->y = lpDestRect->top;
	}
	if ((nOutside = lpDestRect->bottom - (lpPointDest->y + (lpSrcRect->bottom - lpSrcRect->top))) < 0)
		lpSrcRect->bottom += nOutside;

	if (lpSrcRect->top >= lpSrcRect->bottom)
		return FALSE;

	// ��������
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
// �T�v: �X�V�t���O�����ׂ��钼�O�ɌĂяo����鉼�z�֐�
// ����: �Ȃ�
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::PreUpdate()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxSprite::Update()
// �T�v: �X�V��`������ΌĂяo����鉼�z�֐�
// ����: �Ȃ�
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::Update()
{

}

////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxSprite::Draw(CNxSurface* pSurface, const RECT* lpRect) const
// �T�v: �X�v���C�g�`��
// ����: CNxSurface* pSurface ... �`���T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpRect   ... �X�v���C�g���̕`���`������ RECT �\���̂ւ̃|�C���^
// �ߒl: �q�X�v���C�g�̕`��𑱂���Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSprite::Draw(CNxSurface* /*pSurface*/, const RECT* /*lpRect*/) const
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxSprite::DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const
// �T�v: �X�v���C�g�`��(�q�X�v���C�g�̌�)
// ����: CNxSurface* pSurface ... �`���T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpRect   ... �X�v���C�g���̕`���`������ RECT �\���̂ւ̃|�C���^
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////

void CNxSprite::DrawBehindChildren(CNxSurface* /*pSurface*/, const RECT* /*lpRect*/) const
{

}
