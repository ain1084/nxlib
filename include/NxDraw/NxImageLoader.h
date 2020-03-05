// NxImageLoader.h: NxDrawLocal::CNxImageLoader �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000,2001 S.Ainoguchi
// 2000/11/20 ���ō쐬
//
// �T�v: �C���[�W�ǂݍ��݃N���X�BCreate() �ɂ���ăC���[�W��ǂݍ��݁A
//       GetHeader() �� GetDIBits() �œǂݍ��񂾃C���[�W��
//       BITMAPINFO �ƃr�b�g�f�[�^��������B
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxFile;
class CNxDIBImageLoader;

#include "NxDIBImage.h"

namespace NxDrawLocal
{

class CNxImageLoader  
{
public:
	CNxImageLoader(void);
	virtual ~CNxImageLoader(void);
	BOOL IsSupported(CNxFile& nxfile) const;
	CNxDIBImage* CreateDIBImage(CNxFile& nxfile) const;

private:
	bool findLoader(CNxFile& nxfile) const;
	mutable std::auto_ptr<CNxDIBImageLoader> m_pLastLoader;

private:
	CNxImageLoader(const CNxImageLoader&);
	CNxImageLoader& operator=(const CNxImageLoader&);
};

}
