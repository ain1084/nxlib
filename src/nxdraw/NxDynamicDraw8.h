// NxDynamicDraw32.cpp: CNxDynamicDraw32 �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// �T�v: ���I�R�[�h�ɂ��T�[�t�F�X�������ւ̒��ڕ`��(8bpp ��p)
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDynamicDraw.h"

namespace NxDrawLocal
{

class CNxDynamicDraw8 : public CNxDynamicDraw
{
public:
	virtual BOOL Blt(CNxSurface* pDestSurface, const RECT* lpDestRect, 
					 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
					 const NxBlt* pNxBlt);
};

}	// namespace NxDrawLocal