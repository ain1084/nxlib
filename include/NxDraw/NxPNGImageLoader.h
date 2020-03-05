// NxPNGImageLoader.h: CNxPNGImageLoader �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: BMP �摜��ǂݍ��݁ACNxDIBImage ��Ԃ�
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxPNGImageLoader : public CNxDIBImageLoader  
{
public:
	CNxPNGImageLoader();
	virtual ~CNxPNGImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);
};
