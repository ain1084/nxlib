// NxPNGImageSaver.cpp: CNxPNGImageSaver �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �摜(CNxDIBImage) �� BMP �`���ŕۑ�����ACNxDIBImageSaver �h���N���X
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <vector>
#include "NxPNGImageSaver.h"
#include "NxPNGErrorHandler.h"
#include "NxDIBImage.h"

using namespace NxDrawLocal::NxPNGErrorHandler;

namespace
{
	void WriteNxFile(png_structp png_ptr, png_bytep data, png_size_t length);
	void FlushNxFile(png_structp png_ptr);
} // namespace

//////////////////////////////////////////////////////////////////////
// public:
//	CNxPNGImageSaver::CNxPNGImageSaver(DWORD dwFlags)
// �T�v: CNxPNGImageSaver �N���X�̃f�t�H���g�R���X�g���N�^
// ����: DWORD dwFlags ... �t���O
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxPNGImageSaver::CNxPNGImageSaver(DWORD dwFlags)
{
	m_dwFlags = dwFlags;
}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxPNGImageSaver::~CNxPNGImageSaver()
// �T�v: CNxPNGImageSaver �N���X�̃f�X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxPNGImageSaver::~CNxPNGImageSaver()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxPNGImageSaver::isFullGrayscalePallete(const RGBQUAD* pRGBQUAD, const BYTE* pbAlphaTable, UINT cEntries)
// �T�v: �p���b�g���e�𒲍����Ċ��S�ȃO���C�X�P�[���Ȃ�΁ATRUE ��Ԃ�
// ����: const RGBQUAD* pRGBQUAD  ... �J���[�e�[�u���̍ŏ��̃G���g���ւ̃|�C���^
//       const BYTE* pbAlphaTable ... �A���t�@�l�e�[�u���̍ŏ��̃G���g���ւ̃|�C���^
//       UINT cEntries            ... �p���b�g��(1 �` 256)
// �ߒl: ���S�ȃO���C�X�P�[���p���b�g�Ȃ�� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxPNGImageSaver::isFullGrayscalePallete(const RGBQUAD* pRGBQUAD, UINT cEntries)
{
	for (UINT u = 0; u < cEntries; u++, pRGBQUAD++)
	{
		if (pRGBQUAD->rgbRed != pRGBQUAD->rgbGreen || pRGBQUAD->rgbGreen != pRGBQUAD->rgbBlue)
			return FALSE;		// R = G = B �ł͂Ȃ�

		if (pRGBQUAD->rgbRed != (255 * u / (cEntries - 1)))
			return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxPNGImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage,
//												const RECT* lpRect) const		  
// �T�v: CNxDIBImage �I�u�W�F�N�g�̓��e���t�@�C���֕ۑ�
// ����: CNxFile& nxfile                   ... �ۑ��� CNxFile �I�u�W�F�N�g�ւ̎Q��
//       const CNxDIBImage& srcDIBImage    ... �ۑ������ CNxDIBImage �I�u�W�F�N�g�ւ̎Q��
//       const RECT* lpRect                ... �ۑ���`
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxPNGImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect) const
{
	if (!CNxDIBImageSaver::SaveDIBImage(nxfile, srcDIBImage, lpRect))
		return FALSE;

	RECT rect;
	if (lpRect == NULL)
	{
		srcDIBImage.GetRect(&rect);
	}
	else
	{
		RECT rcDIB;
		srcDIBImage.GetRect(&rcDIB);
		::IntersectRect(&rect, &rcDIB, lpRect);
		if (IsRectEmpty(&rect))
		{
			_RPTF0(_CRT_ERROR, "CNxPNGImageSaver : �ۑ���`�͋�ł�.\n");
			return FALSE;
		}
	}

	// �ۑ������摜�`���̃s�N�Z���r�b�g��
	UINT uBitCount = srcDIBImage.GetBitCount();
	
	// libpng �̏�����
	png_structp png_ptr;
	png_infop info_ptr;

	// png_write_struct ���쐬
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, &nxfile, UserError, UserWarning);
	if (png_ptr == NULL)
		return FALSE;

	// PNG �f�[�^�������݊֐���ݒ�
	png_set_write_fn(png_ptr, &nxfile, WriteNxFile, FlushNxFile);

	// info_struct ���쐬
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{	// �G���[���� (png_write_struct ��j��)
		png_destroy_write_struct(&png_ptr, NULL);
		return FALSE;
	}

	// IHDR chunk �������ݏ���
	int color_type = PNG_COLOR_TYPE_RGB;
	int bit_depth = 8;
	switch (uBitCount)
	{
	case 32:
		png_set_bgr(png_ptr);
		if (m_dwFlags & stripAlpha)
		{	// �A���t�@����菜��
			color_type = PNG_COLOR_TYPE_RGB;
			uBitCount = 24;
		}
		else
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		break;
	case 24:
		color_type = PNG_COLOR_TYPE_RGB;
		png_set_bgr(png_ptr);
		break;
	case 16:
		// 16BPP �� 24BPP �֕ϊ����ĕۑ�
		uBitCount = 24;
		color_type = PNG_COLOR_TYPE_RGB;
		png_set_bgr(png_ptr);
		break;	
	case 8:
	case 4:
	case 1:
		color_type = PNG_COLOR_TYPE_PALETTE;
		bit_depth = uBitCount;
		break;
	}

	// �p���b�g�̐ݒ�

	std::vector<png_color> palette;
	std::vector<png_byte> trans;
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		int i;
		int num_palette = srcDIBImage.GetColorCount();
		std::vector<NxColor> nxcolors(num_palette);
		const RGBQUAD* pRGBQUAD = srcDIBImage.GetColorTable();

		if (isFullGrayscalePallete(pRGBQUAD, num_palette))
		{	// ���S�ȃO���C�X�P�[��
			color_type = PNG_COLOR_TYPE_GRAY;
		}
		else
		{
			palette.resize(num_palette);
			for (i = 0; i < num_palette; i++)
			{
				png_color pal;
				pal.red = pRGBQUAD->rgbRed;
				pal.green = pRGBQUAD->rgbGreen;
				pal.blue = pRGBQUAD->rgbBlue;
				palette[i] = pal;
				pRGBQUAD++;
			}
			png_set_PLTE(png_ptr, info_ptr, &palette[0], num_palette);
		}
	}

	// IHDR �`�����N��ݒ�
	png_set_IHDR(png_ptr, info_ptr, rect.right - rect.left, rect.bottom - rect.top, bit_depth, color_type,
				 (m_dwFlags & interlace) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// info �����o��
	png_write_info(png_ptr, info_ptr);


	// �t�B���^��ݒ�
	int filters = PNG_NO_FILTERS;
	if (m_dwFlags & filterNone)
		filters |= PNG_FILTER_NONE;
	if (m_dwFlags & filterSub)
		filters |= PNG_FILTER_SUB;
	if (m_dwFlags & filterUp)
		filters |= PNG_FILTER_UP;
	if (m_dwFlags & filterAvg)
		filters |= PNG_FILTER_AVG;
	if (m_dwFlags & filterPaeth)
		filters |= PNG_FILTER_PAETH;
	png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, filters); 
	
	// zlib ���k���x���ݒ�(���ʕs��)
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	// 1 line ���̍s�o�b�t�@���m��
	CNxDIBImage dibTemp;
	dibTemp.Create(rect.right - rect.left, 1, uBitCount);
	
	// ��s�������o��
	int nHeight = rect.bottom - rect.top;
	do
	{
		rect.bottom = rect.top + 1;
		dibTemp.Blt(0, 0, &srcDIBImage, &rect);
		png_write_row(png_ptr, static_cast<LPBYTE>(dibTemp.GetBits()));
		rect.top++;
	} while (--nHeight != 0);

	// ��n��
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return TRUE;
}

namespace
{

void WriteNxFile(png_structp png_ptr, png_bytep data, png_size_t length)
{
	CNxFile* pFile = static_cast<CNxFile*>(png_ptr->io_ptr);
	pFile->Write(data, length);
}

void FlushNxFile(png_structp png_ptr)
{
	CNxFile* pFile = static_cast<CNxFile*>(png_ptr->io_ptr);
	pFile->Flush();
}

} // namespace
