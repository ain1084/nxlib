// NxMouseSprite.cpp: CNxMouseSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: CNxSprite �Ń}�E�X�J�[�\������������N���X
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxMouseSprite.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxMouseSprite::CNxMouseSprite(CNxWindow* pWindow, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pWindow, pSurface, lpRect)
 , m_nShowCount(0)
{
	_ASSERTE(pWindow != NULL);

	// �����ʒu�𔽉f
	PreUpdate();
}

CNxMouseSprite::~CNxMouseSprite()
{

}

///////////////////////////////////////////////////////////////////////////
// public:
//	int CNxMouseSprite::Show(BOOL bShow)
// �T�v: �J�[�\���̕\���J�E���^�𑝌��B���̐��l�ł���΃J�[�\����\��
// ����: BOOL bShow ... �J�[�\����\������Ȃ� TRUE
// �ߒl: ���݂̕\���J�E���^��Ԃ�
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
// �T�v: �X�v���C�g�̍��W���}�E�X�J�[�\���ɔ��f������
//       (CNxSprite::SetPos() �̃I�[�o�[���C�h)
// ����: int x ... �ݒ肷��X���W
//       int y ... �ݒ肷��Y���W
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////

BOOL CNxMouseSprite::SetPos(int x, int y)
{
	POINT ptCursor;
	ptCursor.x = x;
	ptCursor.y = y;
	// �g�b�v���x���X�v���C�g(CNxWindow) ��̍��W�֕ϊ�
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
// �T�v: �X�v���C�g�̍X�V��Ԃ����ׂ���O�ɌĂ΂�鉼�z�֐�
// ����: �Ȃ�
// �ߒl: �Ȃ�
// ���l: �}�E�X�J�[�\���̍��W���擾���ăX�v���C�g�ړ�
///////////////////////////////////////////////////////////////////////////

void CNxMouseSprite::PreUpdate()
{
	POINT ptCursor;
	// ���݂̃g�b�v���x���X�v���C�g(CNxWindow) ��̃}�E�X�J�[�\�����W���擾
	if (!getWindow()->GetCursorPos(&ptCursor))
	{
		return;
	}
	// ���̃X�v���C�g�̍��W�֕ϊ�
	TopToSprite(&ptCursor);
	__super::SetPos(ptCursor.x, ptCursor.y);
}
