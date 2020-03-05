// NxBMPImageSaver.h: CNxBMPImageSaver �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �摜(CNxDIBImage) �� BMP �`���ŕۑ�����ACNxDIBImageSaver �h���N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageSaver.h"

class CNxBMPImageSaver : public CNxDIBImageSaver  
{
public:
	enum Flags
	{
		stripAlpha = 0x00000001,	// ���l����菜���ĕۑ�
	};

	CNxBMPImageSaver(DWORD dwFlags = stripAlpha);
	virtual ~CNxBMPImageSaver();
	virtual BOOL SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect = 0) const;

private:
	DWORD m_dwFlags;
};
