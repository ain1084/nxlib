// NxStretchSprite.cpp: CNxStretchSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Aingouchi
//
// �T�v: �g��k���X�v���C�g
//       CNxSurfaceSprite �̔h���N���X�ł��B
//       SetSrcRect() �����o�֐��Őݒ肳�ꂽ�]������`����A
//       (CNxSurfaceSprite �N���X�����o��) SetRect() �֐��Ŏw�肳�ꂽ��`��
//       �g�喔�͏k���������Ȃ��Ȃ���]�����܂��B
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxStretchSprite.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
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
// �T�v: �]������`��ݒ�
// ����: const RECT* lpSrcRect ... �ݒ肷��]������`������ RECT �\���̂ւ̃|�C���^(NULL �Ȃ�΃T�[�t�F�X�S��)
// �ߒl: �Ȃ�
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
// �T�v: �]�����T�[�t�F�X�Ƌ�`��ݒ�
// ����: CNxSurface* pSurface ... �T�[�t�F�X�ւ̃|�C���^
//       const RECT *lpRect   ... �ݒ肷���`������ RECT �\���̂ւ̃|�C���^
// ���l: CNxSurfaceSprite::SetSrcSurface() �̃I�[�o�[���C�h
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
