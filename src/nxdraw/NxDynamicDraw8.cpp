// NxDynamicDraw8.cpp: CNxDynamicDraw8 �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// �T�v: ���I�R�[�h�ɂ��T�[�t�F�X�������ւ̒��ڕ`��(8bpp ��p)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxDynamicDraw8.h"
#include "NxDrawLocal.h"
#include "NxSurface.h"

using namespace NxDrawLocal;

//////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDynamicDraw8::Blt()
// �T�v: �r�b�g�u���b�N�]��
// �ߒl: �����Ȃ�� TRUE
//////////////////////////////////////////////////////////////////////

BOOL CNxDynamicDraw8::Blt
(CNxSurface* /*pDestSurface*/,			// �]����T�[�t�F�X
 const RECT* /*lpDestRect*/,			// �]�����`
 const CNxSurface* /*pSrcSurface*/,		// �]�����T�[�t�F�X
 const RECT* /*lpSrcRect*/,				// �]������`
 const NxBlt* /*pNxBlt*/)				// �]�����@
{
	_RPTF0(_CRT_ASSERT, "CNxDyanmicDraw8::Blt() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}
