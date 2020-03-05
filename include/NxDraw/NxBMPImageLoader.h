// NxBMPImageLoader.h: CNxBMPImageLoader �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: BMP �摜��ǂݍ��݁ACNxDIBImage ��Ԃ�
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxBMPImageLoader : public CNxDIBImageLoader  
{
public:
	CNxBMPImageLoader();
	virtual ~CNxBMPImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);

private:
	static BOOL decodeRLE4(LPBYTE lpbBits, CNxFile& nxfile, LONG lNextLine);
	static BOOL decodeRLE8(LPBYTE lpbBits, CNxFile& nxfile, LONG lNextLine);
};
