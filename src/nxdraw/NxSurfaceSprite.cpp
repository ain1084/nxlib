// NxSurfaceSprite.cpp: CNxSurfaceSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �T�[�t�F�X�̈ꕔ��(�����͑S��)��P���ɕ\������@�\��
//       ���ACNxSprite �h���N���X�B
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxSurfaceSprite.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
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
// �T�v: �X�v���C�g�̋�`��ݒ�
// ����: const RECT* lpRect ... �ݒ肷���`
// �ߒl: �����Ȃ�� TRUE�B����ȊO�� FALSE (��`���T�[�t�F�X�͈͊O��)
// ���l: lpRect �� NULL �̏ꍇ�A�ڑ�����Ă���T�[�t�F�X�S�̂ƂȂ�
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
// �T�v: �T�[�t�F�X���֘A�t����
// ����: CNxSurface* pSurface ... �֘A�t����T�[�t�F�X
//       const RECT* lpRect       ... �T�[�t�F�X�̋�`(NULL �Ȃ�΃T�[�t�F�X�S��)
// �ߒl: ���O�܂Ŋ֘A�Â����Ă����T�[�t�F�X(�����Ȃ�� NULL)
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
// �T�v: �X�v���C�g�`��
// ����: CNxSurface* pSurface ... �`���T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpRect   ... �X�v���C�g���̕`���`������ RECT �\���̂ւ̃|�C���^
// �ߒl: �q�X�v���C�g�̕`��𑱂���Ȃ�� TRUE
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
// �T�v: �]�����Ɏg�p���� NxBlt ��ݒ�
// ����: const NxBltFx* pNxBlt ... �ݒ肷�� NxBlt �\���̂ւ̃|�C���^
//       BOOL bUpdate          ... TRUE �Ȃ�΁ASetUpdate() ���Ăяo��
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////

void CNxSurfaceSprite::SetNxBlt(const NxBlt* pNxBlt, BOOL bUpdate)
{
	if (pNxBlt == NULL)
		memset(&m_nxBlt, 0, sizeof(NxBlt));		// pNxBlt == NULL �Ȃ�΍\���̂�������
	else
		m_nxBlt = *pNxBlt;

	if (bUpdate)
		SetUpdate();
}
