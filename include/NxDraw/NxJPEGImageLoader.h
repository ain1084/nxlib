// NxJPEGImageLoader.h: CNxJPEGImageLoader �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: JPEG �摜�̓ǂݍ��� CNxDIBImage ��Ԃ��ACNxDIBImageLoader �h���N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxJPEGImageLoader : public CNxDIBImageLoader  
{
public:
	CNxJPEGImageLoader();
	virtual ~CNxJPEGImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);
};
