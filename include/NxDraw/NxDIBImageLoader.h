// NxDIBImageLoader.h: CNxDIBImageLoader �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �摜�̓ǂݍ��݂��s���ׂ̃v���g�R���N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxFile;
class CNxDIBImage;

class __declspec(novtable) CNxDIBImageLoader
{
public:
	CNxDIBImageLoader();
	virtual ~CNxDIBImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const = 0;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile) = 0;
};
