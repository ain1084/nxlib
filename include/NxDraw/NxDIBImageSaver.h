// NxDIBImageSaver.h: CNxDIBImageSaver �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �摜(CNxDIBImage) �̕ۑ����s���ׂ̃v���g�R���N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxFile;
class CNxDIBImage;

class __declspec(novtable) CNxDIBImageSaver
{
public:
	CNxDIBImageSaver();
	virtual ~CNxDIBImageSaver();
	virtual BOOL SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect = NULL) const = 0;
};
