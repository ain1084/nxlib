// NxBMPImageLoader.cpp: CNxBMPImageLoader �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000,2001 S.Ainoguchi
//
// �T�v: BMP �摜��ǂݍ��݁ACNxDIBImage ��Ԃ�
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <memory>
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxDIBImage.h"
#include "NxBMPImageLoader.h"

//////////////////////////////////////////////////////////////////////
// public:
//	CNxBMPImageLoader::CNxBMPImageLoader()
// �T�v: CNxBMPImageLoader �N���X�̃f�t�H���g�R���X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxBMPImageLoader::CNxBMPImageLoader()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxBMPImageLoader::~CNxBMPImageLoader()
// �T�v: CNxBMPImageLoader �N���X�̃f�X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxBMPImageLoader::~CNxBMPImageLoader()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxBMPImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// �T�v: �W�J�\�ȃf�[�^�`���ł��邩�𒲂ׂ�
// ����: LPCVOID lpvBuf ... �f�[�^�̍ŏ����� 2048byte ��ǂݍ��񂾃o�b�t�@�ւ̃|�C���^
//       LONG lLength   ... �f�[�^�̃T�C�Y(�ʏ�� 2048)
// �ߒl: �W�J�\�ł���� TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxBMPImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
{
	const BITMAPINFO* lpbmi;
	if (*static_cast<const WORD*>(lpvBuf) == MAKEWORD('B', 'M'))
	{	// �ŏ��� 'BM' �Ȃ�� BITMAPFILEHEADER  �� skip
		lLength -= sizeof(BITMAPFILEHEADER);
		lpbmi = reinterpret_cast<const BITMAPINFO*>(static_cast<const BITMAPFILEHEADER*>(lpvBuf) + 1);
	}
	else
	{	// �ŏ��� 'BM' �łȂ���� BITMAPINFO �ƌ��Ȃ�
		lpbmi = reinterpret_cast<const BITMAPINFO*>(lpvBuf);
	}

	if (lpbmi->bmiHeader.biSize < sizeof(BITMAPINFOHEADER))
		return FALSE;		// BITMAPINFOHEADER �̃T�C�Y���ُ�

	if (lpbmi->bmiHeader.biPlanes != 1)
		return FALSE;		// �v���[������ 1 �łȂ���΂Ȃ�Ȃ�

	// biBitCount �̃`�F�b�N
	switch (lpbmi->bmiHeader.biBitCount)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
		return TRUE;
	default:
		return FALSE;	// biBitCount ���ُ�
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxBMPImageLoader::CreateDIBImage(CNxFile& nxfile)
// �T�v: �摜��W�J���� CNxDIBImage �I�u�W�F�N�g��Ԃ�
// ����: CNxFile& nxfile ... CNxFile �I�u�W�F�N�g�ւ̎Q��
// �ߒl: �����Ȃ�΁A�쐬���� CNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^�B���s�Ȃ�� NULL
///////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxBMPImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	WORD wBFSig;		// BMP signature ('BM')
	LONG lFileSize;
	DWORD dwBmihSize;

	lFileSize = nxfile.GetSize();				// �T�C�Y

	// �ŏ��� 2byte �� wBFSig �֓ǂݍ���
	if (nxfile.Read(&wBFSig, sizeof(WORD)) == 0)
	{
		_RPTF0(_CRT_ERROR, _T("�t�@�C���̓ǂݍ��݂Ɏ��s.\n"));
		return NULL;
	}
	
	// signature �̒���
	if (wBFSig == MAKEWORD('B', 'M'))
	{
		// �ŏ��� 'BM' �Ȃ̂ŁABITMAP file
		// BITMAPFILEHEADER ���t���Ă���̂� skip ����
		lFileSize -= sizeof(BITMAPFILEHEADER);
		nxfile.Seek(sizeof(BITMAPFILEHEADER) - sizeof(WORD), SEEK_CUR);		// BITMAPPFILEHEADER �� skip

		// BITMAPINFOHEADER �̃T�C�Y��ǂݍ���
		if (nxfile.Read(&dwBmihSize, sizeof(DWORD)) != sizeof(DWORD))
			return FALSE;
	}
	else
	{	// �ŏ��� 'BM' �ȊO�ł���� BITMAPINFOHEADER �ƌ��Ȃ�
		// ������ wBFSig �� BITMAPINFOHEADER �̃T�C�Y�̉��� word �������Ă���
		// dwBmihSize �̏�ʂ́A�t�@�C������ word ��ǂݍ��݁A���ʂւ� wBFSig ��ݒ肵�A
		// BITMAPINFOHEADER �̃T�C�Y(biSize) �Ƃ���
		dwBmihSize = wBFSig;
		if (nxfile.Read(reinterpret_cast<LPWORD>(&dwBmihSize) + 1, sizeof(WORD)) != sizeof(WORD))
			return FALSE;
	}
	// �T�C�Y�� sizeof(BITMAPINFOHEADER) �ȉ��Ȃ�΃G���[
	if (dwBmihSize < sizeof(BITMAPINFOHEADER))
		return FALSE;

	// �t�@�C���T�C�Y���� BITMAPINFOHEADER.biSize ��(sizeof DWORD)������
	lFileSize -= sizeof(DWORD);

	// ������ nxfile �̃t�@�C���|�C���^�� BITMAPINFOHEADER �� biSize �̎�������
	// lFileSize �̓f�[�^(nxfile)�̃T�C�Y

	// BITMAPINFOHEADER �̑�����ǂݍ���
	BITMAPINFOHEADER bmihFile;
	bmihFile.biSize = dwBmihSize;
	if (nxfile.Read(&bmihFile.biWidth, sizeof(BITMAPINFOHEADER) - sizeof(DWORD)) != sizeof(BITMAPINFOHEADER) - sizeof(DWORD))
		return FALSE;
	lFileSize -= sizeof(BITMAPINFOHEADER) - sizeof(DWORD);

	// BITMAPINFOHEADER �ƃr�b�g�f�[�^�̊Ԃɓ���A
	// color mask �ƃp���b�g�f�[�^�̃T�C�Y���擾
	DWORD dwColorSize = CNxDIBImage::GetColorCount(reinterpret_cast<const BITMAPINFO*>(&bmihFile)) * sizeof(RGBQUAD);

	// �X�^�b�N��Ɋ��S�� BITMAPINFO ���\�z
	// BITMAPINFOHEADER ���R�s�[���āAdwColorSize �̃T�C�Y�����}�X�N�ƃp���b�g��ǂݍ���
	LPBITMAPINFO lpbmi = static_cast<LPBITMAPINFO>(_alloca(bmihFile.biSize + dwColorSize));
	memcpy(lpbmi, &bmihFile, bmihFile.biSize);
	if (nxfile.Read(reinterpret_cast<LPBYTE>(lpbmi) + lpbmi->bmiHeader.biSize, dwColorSize) != static_cast<int>(dwColorSize))
	{	// �G���[
		return NULL;
	}
	lFileSize -= dwColorSize;		// �t�@�C���T�C�Y����ǂݍ��񂾕�������

	// ���k�`���̃`�F�b�N�B�܂��A���k����Ă���r�b�g�}�b�v�Ȃ�΁A
	// CNxDIBImage �֓n�� BITMAPINFO ���A�����k�֕ύX(CNxDIBImage �N���X�������k DIB ��v�������)
	switch (bmihFile.biCompression)
	{
	case BI_RLE4:		// RLE4 (4bpp)
	case BI_RLE8:		// RLE8 (8bpp)
		lpbmi->bmiHeader.biCompression = BI_RGB;
		break;
	case BI_BITFIELDS:
	case BI_RGB:
		break;
	default:
		_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : �Ή����Ă��Ȃ����k�`���ł�.\n");
		return NULL;
	}
		
	// CNxDIBImage �I�u�W�F�N�g�̍쐬
	// �r�b�g�f�[�^�ւ̃��������m�ۂ����
	std::auto_ptr<CNxDIBImage> pDIBImage(new CNxDIBImage);
	if (!pDIBImage->Create(lpbmi))
	{
		_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : CNxDIBImage �̏������Ɏ��s���܂���.\n");
		return NULL;
	}
	
	if (bmihFile.biCompression == BI_BITFIELDS || bmihFile.biCompression == BI_RGB)
	{	// �����k (���̂܂܃������֓ǂݍ���)
		DWORD dwImageSize = pDIBImage->GetImageSize();
		if (nxfile.Read(pDIBImage->GetDIBits(), dwImageSize) != static_cast<LONG>(dwImageSize))
		{	// �ǂݍ��݃G���[
			_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : �t�@�C���̓ǂݍ��݂Ɏ��s���܂���.\n");
			return NULL;
		}
	}
	else
	{
		// RLE4/RLE8 ���k�f�[�^�̓W�J
		if (static_cast<LONG>(bmihFile.biSizeImage) > lFileSize)
		{	// �f�[�^�̃T�C�Y������Ȃ�
			_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : ���k���ꂽ Bitmap File �̃T�C�Y�����������܂�.\n");
			return NULL;
		}

		// RLE4 �Ȃ� DecodeRLE4() ��, RLE8 �Ȃ�� DecodeRLE8() ���Ăяo���ĉ摜��W�J
		if (!((bmihFile.biCompression == BI_RLE8) ? decodeRLE8 : decodeRLE4)
			(static_cast<LPBYTE>(pDIBImage->GetDIBits()), nxfile, -pDIBImage->GetPitch()))
		{	// �W�J�G���[
			_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : ���k���ꂽ Bitmap File �̓W�J���ɁA���Ή��̃R�[�h����������܂���.\n");
			return NULL;
		}
	}
	return pDIBImage.release();
}

///////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxBMPImageLoader::decodeRLE4(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
// �T�v: RLE4 ���k���ꂽ BMP ��W�J����
/////////////////////////////////////////////////////////////////////////////////////////// 

BOOL CNxBMPImageLoader::decodeRLE4(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
{
	BYTE byColor;
	BYTE byBuf[128];
	LPBYTE lpbLine = lpbBits;
	bool bEven = true;
	for (;;)
	{
		// ���[�h�ƁA����1�o�C�g�̓ǂݍ���
		if (nxfile.Read(byBuf, 2) != 2)
			return FALSE;

		if (byBuf[0] != 0)
		{	// encode mode
			byColor = byBuf[1];		// �ŏ���1�o�C�g��

			if (!bEven)
			{	// ��h�b�g
				*lpbLine++ = static_cast<BYTE>((*lpbLine & 0xf0) | (byColor >> 4));
				bEven = true;
				if (--byBuf[0] != 0)
				{
					for (BYTE byLoop = 0; byLoop < byBuf[0] / 2; byLoop++)
					{
						byColor = static_cast<BYTE>((byColor >> 4) | (byColor << 4));	// swap high and low
						*lpbLine++ = byColor;
					}
					if ((byBuf[0] % 2) != 0)
					{
						*lpbLine = static_cast<BYTE>(byColor << 4);
						bEven = false;
					}
				}
			}
			else
			{	// �����h�b�g
				memset(lpbLine, byColor, (byBuf[0] + 1) / 2);
				lpbLine += byBuf[0] / 2;
				bEven = ((byBuf[0] % 2) != 0) ? false : true;
			}
		}
		else
		{	// code mode
			switch (byBuf[1])	/* byBuf[1] = �R�[�h�ԍ� */
			{
			case 0:
				// end of line
				lpbBits += lPitch;
				lpbLine = lpbBits;
				bEven = true;
				break;
			case 1:
				// end of bitmap
				return TRUE;
			case 2:
				// move position (no supported)
				return FALSE;
			default:
				BYTE byCount = byBuf[1];		// �f�[�^�̌�
				nxfile.Read(byBuf, ((byCount + 1) / 2 + 1) / 2 * 2);
				if (!bEven)
				{	// ��h�b�g
					int nIndex = 0;
					byColor = byBuf[nIndex++];
					// �ŏ��� 1dot
					*lpbLine++ = static_cast<BYTE>((*lpbLine & 0xf0) | (byColor >> 4));
					bEven = true;
					if (--byCount == 0)
						break;

					// �o�C�g���E����
					for (BYTE byLoop = 0; byLoop < byCount / 2; byLoop++)
					{
						*lpbLine = static_cast<BYTE>(byColor << 4);
						byColor = byBuf[nIndex++];
						*lpbLine++ |= byColor >> 4;
					}

					// �Ō�� 1dot
					if ((byCount % 2) != 0)
					{
						*lpbLine = static_cast<BYTE>(byColor << 4);
						bEven = false;
					}
				}
				else
				{	// �����h�b�g
					memcpy(lpbLine, byBuf, (byCount + 1) / 2);
					lpbLine += byCount / 2;
					bEven = ((byCount % 2) != 0) ? false : true;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxBMPImageLoader::decodeRLE8(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
// �T�v: RLE8 ���k���ꂽ BMP ��W�J����
/////////////////////////////////////////////////////////////////////////////////////////// 

BOOL CNxBMPImageLoader::decodeRLE8(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
{
	BYTE byBuf[256];
	LPBYTE lpbLine = lpbBits;
	for (;;)
	{
		if (nxfile.Read(byBuf, 2) != 2)
			return FALSE;

		if (byBuf[0] != 0)
		{	// encode mode
			memset(lpbLine, byBuf[1], byBuf[0]);
			lpbLine += byBuf[0];
		}
		else
		{	// code mode
			switch (byBuf[1])
			{
			case 0:
				// end of line
				lpbBits += lPitch;
				lpbLine = lpbBits;
				break;
			case 1:
				// end of bitmap
				return TRUE;
			case 2:
				// move position (no supported)
				return FALSE;
			default:
				nxfile.Read(lpbLine, (byBuf[1] + 1) / 2 * 2);
				lpbLine += byBuf[1];
			}
		}
	}
}
