// NxImageLoader.cpp: NxDrawLocal::CNxImageLoader �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000,2001 S.Ainoguchi
// 2000/11/20 ���ō쐬
// 2000/11/24 ��蒼��
//
// �T�v: �C���[�W�ǂݍ��݃N���X�BCreateDIBImage() �ɂ���āA
//       �ǂݍ��񂾃C���[�W�� CNxDIBImage �I�u�W�F�N�g��Ԃ�
//		 CNxDraw::LoadImage() ��p
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <io.h>
#include "NxImageLoader.h"

#include "NxBMPImageLoader.h"
#include "NxPNGImageLoader.h"
#include "NxSPIImageLoader.h"
#include "NxJPEGImageLoader.h"
#include "NxMAGImageLoader.h"

using namespace NxDrawLocal;

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxImageLoader::CNxImageLoader(void)
{
}

CNxImageLoader::~CNxImageLoader(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxImageLoader::IsSupported(CNxFile& nxfile) const
// �T�v: �T�|�[�g���Ă���t�H�[�}�b�g�ł��邩�𒲂ׂ�
// ����: CNxFile& nxfile ... �ǂݍ��݌��t�@�C��
// �ߒl: �W�J�\�ł���� TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxImageLoader::IsSupported(CNxFile& nxfile) const
{
	return findLoader(nxfile);
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDIBImage* CNxImageLoader::CreateDIBImage(CNxFile& nxfile) const
// �T�v: �摜��ǂݍ��݁ACNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^��Ԃ�
// ����: CNxFile& nxfile ... �ǂݍ��݌��t�@�C��
// �ߒl: �����Ȃ�� CNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^, ���s�Ȃ�� FALSE
//       CNxDIBImage �I�u�W�F�N�g���s�v�ɂȂ����� delete �ō폜���鎖
/////////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxImageLoader::CreateDIBImage(CNxFile& nxfile) const
{
	if (!findLoader(nxfile))
	{
		_RPTF0(_CRT_ASSERT, "CNxImageLoader::CreateDIBImage() : �Ή����Ă��Ȃ��t�H�[�}�b�g�ł�.\n");
		return NULL;
	}
	CNxDIBImage* pDIBImage = m_pLastLoader->CreateDIBImage(nxfile);
	if (pDIBImage == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxImageLoader::CreateDIBImage() : �W�J�Ɏ��s���܂���.\n");
		return NULL;
	}
	return pDIBImage;
}

//////////////////////////////////////////////////////////////////////////////////////
// private:
//	CNxImageHandler* CNxImageLoader::FindHandler(CNxFile& nxfile) const
// �T�v: �Ή��n���h�����������āACNxImageHandler �I�u�W�F�N�g�ւ̃|�C���^��Ԃ�
// ����: CNxFile& nxfle ... �ǂݍ��݌��t�@�C��
// �ߒl: �����Ȃ�� true
//////////////////////////////////////////////////////////////////////////////////////

bool CNxImageLoader::findLoader(CNxFile& nxfile) const
{
	// �`�F�b�N�p�f�[�^�Ƃ��āA�擪���� 2048byte ���o�b�t�@�֓ǂݍ���
	const LONG lCheckBufSize = 2048;
	char checkBuf[lCheckBufSize];
	
	LONG lOffset = nxfile.Seek(0, SEEK_CUR);
	LONG lLength = nxfile.Read(checkBuf, lCheckBufSize);
	nxfile.Seek(lOffset, SEEK_SET);		// file offset ��߂�
	if (lLength < 0)
	{
		return false;
	}
	
	// ����Ȃ����� 0 �œU��(for susie plug-in)
	memset(checkBuf + lLength, 0, max(lCheckBufSize - lLength, 0));

	// ���O�Ɏg�p���� ImageLoader ��D��I��...
	if (m_pLastLoader.get() != NULL)
	{
		if (m_pLastLoader->IsSupported(checkBuf, lCheckBufSize))
		{
			return true;
		}
	}

	// NxDraw ���T�|�[�g����W���� ImageLoader ���猟��
	enum
	{
#if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
		loaderJPEG,
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
#if !defined(NXDRAW_LOADIMAGE_NO_PNG)
		loaderPNG,
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_PNG)
#if	!defined(NXDRAW_LOADIMAGE_NO_MAG)
		loaderMAG,
#endif
		loaderBMP,
		loaderLast
	};
	for (int i = 0; i < loaderLast; i++)
	{
		switch (i)
		{
#if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
		case loaderJPEG:
			m_pLastLoader.reset(new CNxJPEGImageLoader);
			break;
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
#if !defined(NXDRAW_LOADIMAGE_NO_PNG)
		case loaderPNG:
			m_pLastLoader.reset(new CNxPNGImageLoader);
			break;
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_PNG)
#if	!defined(NXDRAW_LOADIMAGE_NO_MAG)
		case loaderMAG:
			m_pLastLoader.reset(new CNxMAGImageLoader);
			break;
#endif
		case loaderBMP:
			m_pLastLoader.reset(new CNxBMPImageLoader);
			break;
		}
		if (m_pLastLoader->IsSupported(checkBuf, lCheckBufSize))
		{
			return true;
		}
	}

#if !defined(NXDRAW_LOADIMAGE_NO_SUSIE_SPI)
	// �w�肳�ꂽ�f�B���N�g������ SPI ��T���ď��� try
	std::basic_string<TCHAR> strSPIDirectory;
	CNxDraw::GetInstance()->GetSPIDirectory(strSPIDirectory);
	long hFile;
	struct _tfinddata_t c_file;
	if ((hFile = _tfindfirst((strSPIDirectory + _T("*.spi")).c_str(), &c_file)) != -1L)
	{
		do
		{
			HINSTANCE hInstance = ::LoadLibrary((strSPIDirectory + c_file.name).c_str());
			if (hInstance != NULL)
			{
				m_pLastLoader.reset(new CNxSPIImageLoader(hInstance));
				if (m_pLastLoader->IsSupported(checkBuf, lCheckBufSize))
				{
					_findclose(hFile);
					return true;
				}
			}
		} while (_tfindnext(hFile, &c_file) == 0);
		_findclose(hFile);
	}
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_SUSIE_SPI)
	m_pLastLoader.reset();
	return false;
}
