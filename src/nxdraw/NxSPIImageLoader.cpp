// NxSPIImageLoader.cpp: CNxSPIImageLoader �N���X�̃C���v�������e�[�V����
// Copyright(C) 2000,2001 S.Ainoguchi
//
// �T�v: susie plug-in �o�R�ŉ摜��ǂݍ��݁ACNxDIBImage ��Ԃ�
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <vector>
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxDIBImage.h"
#include "NxSPIImageLoader.h"

namespace
{
	// �r�b�g�f�[�^�� HLOCAL �ŕێ����� CNxDIBImage �h���N���X
	class CNxLocalHandleDIBImage : public CNxDIBImage
	{
	public:
		CNxLocalHandleDIBImage(void);
		virtual ~CNxLocalHandleDIBImage(void);
		BOOL Create(HLOCAL hBMI, HLOCAL hBits);

	private:
		HLOCAL m_hBMI;
		HLOCAL m_hBits;

	private:
		CNxLocalHandleDIBImage(const CNxLocalHandleDIBImage&);
		CNxLocalHandleDIBImage& operator=(const CNxLocalHandleDIBImage&);
	};

	// �R���X�g���N�^
	CNxLocalHandleDIBImage::CNxLocalHandleDIBImage(void)
	 : m_hBMI(NULL)		// BITMAPINFO �� local memory handle
	 , m_hBits(NULL)		// �r�b�g�f�[�^�� local memory handle
	{

	}

	// �f�X�g���N�^
	CNxLocalHandleDIBImage::~CNxLocalHandleDIBImage(void)
	{
		// �������� unlcok

		if (m_hBMI != NULL)
		{
			::LocalUnlock(m_hBMI);
			::LocalFree(m_hBMI);
		}
		if (m_hBits != NULL)
		{
			::LocalUnlock(m_hBits);
			::LocalFree(m_hBits);
		}
	}

	BOOL CNxLocalHandleDIBImage::Create(HLOCAL hBMI, HLOCAL hBits)
	{
		m_hBits = hBits;
		m_hBMI = hBMI;

		// BITMAPINFO �\���̂̃������� lock
		LPBITMAPINFO lpbmi = static_cast<LPBITMAPINFO>(::LocalLock(m_hBMI));
		_ASSERTE(lpbmi != NULL);

		// �r�b�g�f�[�^�̃������� lock
		LPVOID lpvBits = ::LocalLock(m_hBits);
		_ASSERTE(lpvBits != NULL);
		
		return CNxDIBImage::Create(lpbmi, lpvBits);
	}
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxSPIImageLoader::CNxSPIImageLoader()
// �T�v: CNxSPIImageLoader �N���X�̃f�t�H���g�R���X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxSPIImageLoader::CNxSPIImageLoader(HINSTANCE hInstance)
 : m_hInstance(hInstance)
 , m_pfnIsSupported(NULL)
 , m_pfnGetPicture(NULL)
{
	// GetPluginInfo �̃A�h���X���擾
	int (PASCAL *pfnGetPluginInfo)(int infono, LPSTR buf, int buflen);
	reinterpret_cast<FARPROC&>(pfnGetPluginInfo) = ::GetProcAddress(m_hInstance, "GetPluginInfo");
	if (pfnGetPluginInfo == NULL)
	{
		return;
	}
	// API �o�[�W�����`�F�b�N
	char szAPIVersion[5];
	if ((pfnGetPluginInfo)(0, szAPIVersion, 5) == 0 || strcmp(szAPIVersion, "00IN") != 0)
	{
		return;
	}
	// IsSupported �� GetPicture �̃A�h���X���擾,�ۑ�
	reinterpret_cast<FARPROC&>(m_pfnIsSupported) = ::GetProcAddress(m_hInstance, "IsSupported");
	reinterpret_cast<FARPROC&>(m_pfnGetPicture) = ::GetProcAddress(m_hInstance, "GetPicture");
}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxSPIImageLoader::~CNxSPIImageLoader()
// �T�v: CNxSPIImageLoader �N���X�̃f�X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxSPIImageLoader::~CNxSPIImageLoader()
{
	::FreeLibrary(m_hInstance);
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSPIImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// �T�v: �W�J�\�ȃf�[�^�`���ł��邩�𒲂ׂ�
// ����: LPCVOID lpvBuf ... �f�[�^�̍ŏ����� 2048byte ��ǂݍ��񂾃o�b�t�@�ւ̃|�C���^
//       LONG lLength   ... �f�[�^�̃T�C�Y(�ʏ�� 2048)
// �ߒl: �W�J�\�ł���� TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSPIImageLoader::IsSupported(LPCVOID pvData, LONG /*lLength*/) const
{
	if (m_pfnIsSupported == NULL)
		return FALSE;		// IsSupported �֐������݂��Ȃ�
	
	return (m_pfnIsSupported)(NULL, reinterpret_cast<DWORD>(pvData)) != 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxSPIImageLoader::CreateDIBImage(CNxFile& nxfile)
// �T�v: �摜��W�J���� CNxDIBImage �I�u�W�F�N�g��Ԃ�
// ����: CNxFile& nxfile ... CNxFile �I�u�W�F�N�g�ւ̎Q��
// �ߒl: �����Ȃ�΁A�쐬���� CNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^�B���s�Ȃ�� NULL
///////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxSPIImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	if (m_pfnGetPicture == NULL)
		return NULL;
	
	// �S�̂��������֓ǂݍ���
	LONG lFileSize = nxfile.GetSize();
	std::vector<char> fileData(lFileSize);
	if (nxfile.Read(&fileData[0], lFileSize) != lFileSize)
		return NULL;

	// �T�|�[�g����Ă��邩?
	if (!IsSupported(&fileData[0], lFileSize))
		return NULL;
	
	// �摜�̓W�J
	HLOCAL hBMI, hBits;
	if ((m_pfnGetPicture)(&fileData[0], lFileSize, 1 /* 0:�t�@�C�� / 1:���������� */, &hBMI, &hBits, NULL, 0) != 0)
		return NULL;
	
	// �W�J����
	// CNxDIBImage �I�u�W�F�N�g���쐬���ĕԂ�
	std::auto_ptr<CNxLocalHandleDIBImage> pDIBImage(new CNxLocalHandleDIBImage);
	if (!pDIBImage->Create(hBMI, hBits))
	{
		return NULL;
	}
	return pDIBImage.release();
}
