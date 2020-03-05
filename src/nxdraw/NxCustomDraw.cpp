// NxCustomDraw.cpp: CNxCustomDraw �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �T�[�t�F�X�������ւ̒��ڕ`��(���ۃN���X)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxCustomDraw.h"

using namespace NxDrawLocal;

/////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static BOOL CNxCustomDraw::GetValidOpacity(const NxBlt* pNxBlt, LPUINT lpuOpacity)
// �T�v: NxBlt �\���̂���A�L���� uOpacity �l�𓾂�
// ����: const NxBltFx* pNxBlt ... NxBlt �\���̂ւ̃|�C���^
//       LPUINT lpuOpacity     ... �s�����x���󂯎�� UINT �^�ϐ��ւ̃|�C���^
// �ߒl: �Ԃ��ꂽ�s�����x(*lpuOpacity)���L���Ȃ�� TRUE
 ////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw::GetValidOpacity(const NxBlt* pNxBlt, LPUINT lpuOpacity)
{
	if (pNxBlt->dwFlags & NxBlt::opacity)
	{
		if (pNxBlt->uOpacity > 255)
			return FALSE;

		*lpuOpacity = pNxBlt->uOpacity;
	}
	else
		*lpuOpacity = 255;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static BOOL CNxCustomDraw::IsStretch(const RECT* lpDestRect, const RECT* lpSrcRect)
// �T�v: ��̋�`�̑傫�����قȂ��Ă���(�g��k�����s��)�Ȃ�� TRUE ��Ԃ�
// ����: const RECT* lpDestRect ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const RECT* lpSrcRect  ... �]������`������ RECT �\���̂ւ̃|�C���^
// �ߒl: �傫�����قȂ�Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw::IsStretch(const RECT* lpDestRect, const RECT* lpSrcRect)
{
	return (
		((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left)) |
		((lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top))
	) != 0;
}
