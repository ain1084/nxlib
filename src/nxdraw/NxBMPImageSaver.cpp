// NxBMPImageSaver.cpp: CNxBMPImageSaver �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �摜(CNxDIBImage) �� BMP �`���ŕۑ�����ACNxDIBImageSaver �h���N���X
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <vector>
#include "NxBMPImageSaver.h"
#include "NxDIBImage.h"

//////////////////////////////////////////////////////////////////////
// public:
//	CNxBMPImageSaver::CNxBMPImageSaver(DWORD dwFlags = stripAlpha)
// �T�v: CNxBMPImageSaver �N���X�̃f�t�H���g�R���X�g���N�^
// ����: DWORD dwFlags ...�t���O
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxBMPImageSaver::CNxBMPImageSaver(DWORD dwFlags)
 : m_dwFlags(dwFlags)
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxBMPImageSaver::~CNxBMPImageSaver()
// �T�v: CNxBMPImageSaver �N���X�̃f�X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxBMPImageSaver::~CNxBMPImageSaver()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxBMPImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage,
//												const RECT* lpRect = NULL) const		  
// �T�v: CNxDIBImage �I�u�W�F�N�g�̓��e���t�@�C���֕ۑ�
// ����: CNxFile& nxfile                   ... �ۑ��� CNxFile �I�u�W�F�N�g�ւ̎Q��
//       const CNxDIBImage& srcDIBImage    ... �ۑ������ CNxDIBImage �I�u�W�F�N�g�ւ̎Q��
//       const RECT* lpRect                ... �ۑ���`(NULL �Ȃ�ΑS��)
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxBMPImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect) const
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
			_RPTF0(_CRT_ERROR, "CNxBMPImageSaver : �ۑ���`�͋�ł�.\n");
			return FALSE;
		}
	}
	// BITMAPINFOHEADER �̃R�s�[���쐬
	DWORD dwInfoHeaderSize = srcDIBImage.GetInfoHeader()->biSize;
	LPBITMAPINFOHEADER lpbmiHeader = static_cast<LPBITMAPINFOHEADER>(_alloca(dwInfoHeaderSize));
	memcpy(lpbmiHeader, srcDIBImage.GetInfoHeader(), dwInfoHeaderSize);

	// �ۑ���`�̕��ƍ���
	UINT uWidth = rect.right - rect.left;
	UINT uHeight = rect.bottom - rect.top;

	// stripAlpha ���Z�b�g����Ă���Ȃ�΁A32bpp �� 24bpp �Ƃ���(�A���t�@��������)�ۑ�����
	if ((m_dwFlags & stripAlpha) && lpbmiHeader->biBitCount == 32)
		lpbmiHeader->biBitCount = 24;

	// biSizeImage ��ݒ�
	LONG lRowSize = (((((lpbmiHeader->biBitCount * uWidth) + 7) / 8) + 3) / 4) * 4;
	lpbmiHeader->biSizeImage = lRowSize * uHeight;

	// BITMAPFILEHEADER ������
	BITMAPFILEHEADER bmfh;
	bmfh.bfType = MAKEWORD('B', 'M');
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + dwInfoHeaderSize + srcDIBImage.GetColorCount() * sizeof(RGBQUAD);
	bmfh.bfSize = bmfh.bfOffBits + lpbmiHeader->biSizeImage;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;

	// BITMAPFILEHEADER �������o��
	if (nxfile.Write(&bmfh, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER))
	{
		_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : BITMAPFILEHEADER �̏������݂Ɏ��s���܂���.\n");
		return FALSE;
	}
	
	// BITMAPINFOHEADER �\���̂������o��
	if (nxfile.Write(lpbmiHeader, dwInfoHeaderSize) != static_cast<LONG>(dwInfoHeaderSize))
	{
		_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : BITMAPINFOHEADER �̏������݂Ɏ��s���܂���.\n");
		return FALSE;
	}

	// �J���[�e�[�u���̏����o��
	if (srcDIBImage.GetColorTable() != NULL)
	{
		UINT dwColorSize = srcDIBImage.GetColorCount() * sizeof(RGBQUAD);
		if (nxfile.Write(srcDIBImage.GetColorTable(), dwColorSize) != static_cast<LONG>(dwColorSize))
		{
			_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : �J���[�e�[�u���̏������݂Ɏ��s���܂���.\n");
			return FALSE;
		}
	}

	// 1 line ���̍s�o�b�t�@��p��
	CNxDIBImage dibTemp;
	dibTemp.Create(rect.right - rect.left, 1, lpbmiHeader->biBitCount);
	
	// ��s�������o��
	int nLoop = rect.bottom - rect.top;
	rect.top = rect.bottom - 1;
	do
	{
		rect.bottom = rect.top + 1;
		dibTemp.Blt(0, 0, &srcDIBImage, &rect);
		LONG lWriteBytes = nxfile.Write(dibTemp.GetBits(), lRowSize);
		if (lWriteBytes == -1 || lRowSize != lWriteBytes)
		{
			_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : �t�@�C���̏������݂Ɏ��s���܂���.\n");
			return FALSE;
		}
		rect.top--;
	} while (--nLoop != 0);
	return TRUE;
}
