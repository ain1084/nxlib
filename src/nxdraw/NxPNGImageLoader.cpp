// NxPNGImageLoader.cpp: CNxPNGImageLoader �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000,2001 S.Ainoguchi
//
// �T�v: PNG �摜��ǂݍ��݁ACNxDIBImage ��Ԃ�
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <vector>
#include "NxPNGImageLoader.h"
#include "NxPNGErrorHandler.h"
#include "NxDIBImage.h"
#include "libpng/png.h"

using namespace NxDrawLocal::NxPNGErrorHandler;

namespace
{
	void ReadNxFile(png_structp png_ptr, png_bytep buf, png_size_t length)
	{
		CNxFile* pFile = static_cast<CNxFile*>(png_ptr->io_ptr);
		pFile->Read(buf, length);
	}
} // namespace

//////////////////////////////////////////////////////////////////////
// public:
//	CNxPNGImageLoader::CNxPNGImageLoader()
// �T�v: CNxPNGImageLoader �N���X�̃f�t�H���g�R���X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxPNGImageLoader::CNxPNGImageLoader()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxPNGImageLoader::~CNxPNGImageLoader()
// �T�v: CNxPNGImageLoader �N���X�̃f�X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxPNGImageLoader::~CNxPNGImageLoader()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxPNGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// �T�v: �W�J�\�ȃf�[�^�`���ł��邩�𒲂ׂ�
// ����: LPCVOID lpvBuf ... �f�[�^�̍ŏ����� 2048byte ��ǂݍ��񂾃o�b�t�@�ւ̃|�C���^
//       LONG lLength   ... �f�[�^�̃T�C�Y(�ʏ�� 2048)
// �ߒl: �W�J�\�ł���� TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxPNGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
{
	if (png_check_sig(reinterpret_cast<LPBYTE>(const_cast<LPVOID>(lpvBuf)), lLength) != 0)
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxPNGImageLoader::CreateDIBImage(CNxFile& nxfile)
// �T�v: �摜��W�J���� CNxDIBImage �I�u�W�F�N�g��Ԃ�
// ����: CNxFile& nxfile ... CNxFile �I�u�W�F�N�g�ւ̎Q��
// �ߒl: �����Ȃ�΁A�쐬���� CNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^�B���s�Ȃ�� NULL
///////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4611)		// setjmp �Ɋւ���x�����~�߂�

CNxDIBImage* CNxPNGImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	png_colorp palette;
	LPBYTE *ppRow;
	int num_palette;
	int bit_depth;
	int color_type;
	int interlace_type;
	int compression_type;
	int filter_type;

	UINT uBitCount;
	LPBITMAPINFO lpbmi;
	ppRow = NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, &nxfile, UserError, UserWarning);
	if (png_ptr == NULL)
	{
		_RPTF0(_CRT_ERROR, "CNxPNGImageLoader::Load() : png_create_read_struct() �����s���܂���.\n");
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		_RPTF0(_CRT_ERROR, "CNxPNGImageLoader::Load() : png_create_info_struct() �����s���܂���.\n");
		return NULL;
	}

	png_set_read_fn(png_ptr, &nxfile, ReadNxFile);

	if (setjmp(png_jmpbuf(png_ptr)) != 0)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		delete []ppRow;
		return NULL;
	}
	
	png_read_info(png_ptr, info_ptr);
	if (!png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type))
	{
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		_RPTF0(_CRT_ERROR, "CNxPNGImageLoader::Load() : png_get_IHDR() �����s���܂���.\n");
		return NULL;
	}

	if (bit_depth == 16)
	{	// �r�b�g�[�x 16 �͖��Ӗ��Ȃ̂� 8 �r�b�g�[�x��
		png_set_strip_16(png_ptr);
		bit_depth = 8;
	}
	else if (bit_depth == 2)
	{	// 2bpp BITMAP �͍��Ȃ��̂� 8bpp �֕ϊ����đΉ�
		png_set_packing(png_ptr);
		bit_depth = 8;
	}

	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{	// index color
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{	// �������ߐF�L�[�t���Ȃ�� RGBA �֕ϊ�
			png_set_palette_to_rgb(png_ptr);
			png_set_tRNS_to_alpha(png_ptr);
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		}
		else
		{	// �p���b�g���擾
			png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
			uBitCount = bit_depth;
		}
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY)
	{	// �O���C�X�P�[��(�r�b�g�[�x 4, 8)
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{	// ���ߐF�t���O���C�X�P�[���Ȃ�� RGBA ��
			png_set_gray_to_rgb(png_ptr);
			png_set_tRNS_to_alpha(png_ptr);
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		}
		else
		{	// ���ߐF�����̃O���C�X�P�[��
			uBitCount = bit_depth;
			num_palette = 1 << bit_depth;
		}
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{	// �O���C�X�P�[��(�r�b�g�[�x 8) + alpha
		// RGBA �֕ϊ�
		png_set_gray_to_rgb(png_ptr);
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	}
	if (color_type == PNG_COLOR_TYPE_RGB)
	{	// RGB �J���[	
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{	// ���ߐF������΁Aalpha �v�f�֕ϊ�����
			png_set_tRNS_to_alpha(png_ptr);
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;	// RGBA �Ƃ��ċU������
		}
	}
	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{	// RGBA �J���[
		uBitCount = 32;
		num_palette = 0;
		png_set_bgr(png_ptr);
	}
	else if (color_type == PNG_COLOR_TYPE_RGB)
	{	// RGB �J���[
		uBitCount = 24;
		num_palette = 0;
		png_set_bgr(png_ptr);
	}

	lpbmi = static_cast<LPBITMAPINFO>(_alloca(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * num_palette));
	lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbmi->bmiHeader.biWidth = (LONG)width;
	lpbmi->bmiHeader.biHeight = (LONG)height;
	lpbmi->bmiHeader.biPlanes = 1;
	lpbmi->bmiHeader.biBitCount = (WORD)uBitCount;
	lpbmi->bmiHeader.biCompression = BI_RGB;
	lpbmi->bmiHeader.biSizeImage = 0;
	lpbmi->bmiHeader.biXPelsPerMeter = 0;
	lpbmi->bmiHeader.biYPelsPerMeter = 0;
	lpbmi->bmiHeader.biClrUsed = num_palette;
	lpbmi->bmiHeader.biClrImportant = 0;
	
	// RGBQUAD �̐ݒ�
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		for (int i = 0; i < num_palette; i++)
		{
			lpbmi->bmiColors[i].rgbRed = palette[i].red;
			lpbmi->bmiColors[i].rgbGreen = palette[i].green;
			lpbmi->bmiColors[i].rgbBlue = palette[i].blue;
			lpbmi->bmiColors[i].rgbReserved = 0;
		}
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY)
	{
		for (int i = 0; i < num_palette; i++)
		{
			BYTE byValue = static_cast<BYTE>(255 * i / (num_palette - 1));		// �Ō�̃p���b�g�ԍ��� RGB(255,255,255)
			lpbmi->bmiColors[i].rgbBlue = byValue;
			lpbmi->bmiColors[i].rgbGreen = byValue;
			lpbmi->bmiColors[i].rgbRed = byValue;
			lpbmi->bmiColors[i].rgbReserved = 0;
		}
	}

	// CNxDIBImage �̍쐬
	std::auto_ptr<CNxDIBImage> pDIBImage(new CNxDIBImage);
	if (!pDIBImage->Create(lpbmi))
	{
		return NULL;
	}

	// �r�b�g�}�b�v�f�[�^�̎擾
	ppRow = new LPBYTE[height];
	LPBYTE lpbBits = static_cast<LPBYTE>(pDIBImage->GetBits());
	for (unsigned int row = 0; row < height; row++, lpbBits += pDIBImage->GetPitch())
		ppRow[row] = lpbBits;

	png_read_image(png_ptr, ppRow);
	delete []ppRow;

	// png ��n��
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return pDIBImage.release();
}
#pragma warning (default : 4611)
