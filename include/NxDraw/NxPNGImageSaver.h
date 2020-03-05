// NxPNGImageSaver.h: CNxPNGImageSaver �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �摜(CNxDIBImage) �� PNG �`���ŕۑ�����ACNxDIBImageSaver �h���N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageSaver.h"

class CNxPNGImageSaver : public CNxDIBImageSaver  
{
public:

	enum Flags
	{
		noFilters   = 0x00000000,		// �t�B���^����؎g�p���Ȃ�
		filterNone  = 0x00000001,		// none filter
		filterSub   = 0x00000002,		// sub filter
		filterUp    = 0x00000004,		// up filter
		filterAvg   = 0x00000008,		// avg filter
		filterPaeth = 0x00000010,		// paeth filter
		allFilters  = filterNone | filterSub | filterUp | filterAvg | filterPaeth,	// �S�Ẵt�B���^���g�p(default)
		stripAlpha  = 0x00000020,		// ���l��ۑ����Ȃ�
		interlace   = 0x00000040,		// �C���^�[���[�X�`���ŕۑ�
	};

	CNxPNGImageSaver(DWORD dwFlags = allFilters);
	virtual ~CNxPNGImageSaver();
	virtual BOOL SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect = 0) const;

private:
	DWORD m_dwFlags;
	static BOOL isFullGrayscalePallete(const RGBQUAD* pRGBQUAD, UINT cEntries);
};
