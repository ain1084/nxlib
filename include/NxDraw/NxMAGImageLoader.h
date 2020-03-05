// NxMAGImageLoader.h: CNxMAGImageLoader �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: MAG �摜�̓ǂݍ��݂��s���ACNxDIBImageLoader �h���N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxMAGImageLoader : public CNxDIBImageLoader  
{
public:
	CNxMAGImageLoader();
	virtual ~CNxMAGImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);
};
