// NxLayerSprite.cpp: CNxLayerSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: CNxSprite �� CNxSurfaceSprite �𑽏d�p�������N���X
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxLayerSprite.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
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
// �T�v: �T�[�t�F�X���쐬(�X�v���C�g�T�C�Y���ݒ�)
// ����: int nWidth     ... ��
//       int nHeight    ... ����
// �ߒl: �����Ȃ� TRUE
// ���l: CNxSurface::Create(int,int) �̃I�[�o�[���C�h
////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxLayerSprite::Create(int nWidth, int nHeight)
{
	if (CNxSurface::Create(nWidth, nHeight))
	{	// �����Ȃ�A(CNxSurfaceSprite �ɂ�����)�]�����T�[�t�F�X�Ƃ��Ď�����ݒ�
		CNxSurfaceSprite::SetSrcSurface(this);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxSurface* CNxLayerSprite::SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL)
// �T�v: �T�[�t�F�X���֘A�t����
// ����: CNxSurface* pSurface ... �֘A�t����T�[�t�F�X
//       const RECT* lpRect       ... �T�[�t�F�X�̋�`(NULL �Ȃ�΃T�[�t�F�X�S��)
// �ߒl: ���O�܂Ŋ֘A�Â����Ă����T�[�t�F�X(�����Ȃ�� NULL)
// ���l: CNxSurfaceSprite::SetSrcSurface() �̃I�[�o�[���C�h
//       �T�[�t�F�X�͎����������ׁA�ύX�ł��Ȃ�
///////////////////////////////////////////////////////////////////////////////////////////////

CNxSurface* CNxLayerSprite::SetSrcSurface(CNxSurface* /*pSurface*/, const RECT* lpRect)
{
	return CNxSurfaceSprite::SetSrcSurface(this, lpRect);
}
