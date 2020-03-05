// NxDrawGlobal.cpp: namespace NxDraw �̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: NxDraw �S�̂Ɋւ�O���[�o���Ȓ萔�A�ϐ�
//       �N���X�錾�� NxDraw.h
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <algorithm>
#include <sstream>
#include "NxSurface.h"
#include "NxDrawLocal.h"		// namespace NxDrawLocal
#include "NxImageLoader.h"		// namespace NxDrawLocal

using namespace NxDrawLocal;

CNxDraw* CNxDraw::m_pInstance = NULL;

namespace
{

// CNxDraw �N���X�̔r���A�N�Z�X�p CriticalSection
CRITICAL_SECTION g_criticalSection;

inline void Lock() {
	::EnterCriticalSection(&g_criticalSection); }

inline void Unlock() {
	::LeaveCriticalSection(&g_criticalSection); }

/////////////////////////////////////////////////////////////////////////////////////////
// static BOOL GetProcessorSupport3DNow()
// �T�v: 3DNow! ���߂��g�p�\���ۂ��𒲂ׂ�
// ����: �Ȃ�
// �ߒl: �g�p�\�Ȃ�� TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL __declspec(naked) GetProcessorSupport3DNow()
{
	__asm
	{
		push	ebx
		pushfd
		pop		eax
		mov		edx, eax
		xor		eax, 0200000h
		push	eax
		popfd
		pushfd
		pop		eax
		xor		eax, edx
		jz		doesnot_support_CPUID

		mov		eax, 080000000h
		cpuid
		or		eax, eax
		jz		doesnot_support_FeatureFlag

		mov		eax, 080000001h
		cpuid
		test	edx, 080000000h
		jz		doesnot_support_3DNow
		mov		eax, TRUE
		jmp		check3DNow_exit
doesnot_support_CPUID:
doesnot_support_FeatureFlag:
doesnot_support_3DNow:
		mov		eax, FALSE
check3DNow_exit:
		pop		ebx
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// static BOOL GetProcessorSupportMMX()
// �T�v: MMX ���߂��g�p�\���ۂ��𒲂ׂ�
// ����: �Ȃ�
// �ߒl: �g�p�\�Ȃ�� TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL __declspec(naked) GetProcessorSupportMMX()
{
	__asm
	{
		push	ebx
		pushfd
		pop		eax
		mov		edx, eax
		xor		eax, 0200000h
		push	eax
		popfd
		pushfd
		pop		eax
		xor		eax, edx
		jz		doesnot_support_CPUID

		mov		eax, 0
		cpuid
		or		eax, eax
		jz		doesnot_support_FeatureFlag

		mov		eax, 1
		cpuid
		test	edx, 00800000h
		jz		doesnot_support_MMX

		mov		eax, TRUE
		jmp		checkMMX_exit
doesnot_support_CPUID:
doesnot_support_FeatureFlag:
doesnot_support_MMX:
		mov		eax, FALSE
checkMMX_exit:
		pop		ebx
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// static int StrCountCopy(LPTSTR lpDest, LPCTSTR lpszSrc, int nCount)
// �T�v: lpDest �֍ő� nCount ������ lpszSrc ����R�s�[����
// ����: LPTSTR  lpDest  ... ��������󂯎��o�b�t�@�ւ̃|�C���^
//       LPCTSTR lpszSrc ... �k�������ŏI���R�s�[��������ւ̃|�C���^
//       int nCount      ... lpDest �̕����P�ʂ̃T�C�Y
// �ߒl: �R�s�[���ꂽ������(�k������������)
//       nCount <= lpszSrc �������Ȃ�΁AlpDest �̍Ō�Ƀk�������͕t���Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////

static int StrCountCopy(LPTSTR lpDest, LPCTSTR lpszSrc, int nCount)
{
	if (nCount <= 0)
		return 0;
	
	int nLength;
	int nSrcLength = _tcslen(lpszSrc);
	if (nCount <= nSrcLength)
		nLength = nCount;
	else
		nLength = nSrcLength + 1;

	memcpy(lpDest, lpszSrc, nLength * sizeof(TCHAR));
	return nLength;
}

// CNxDraw �I�u�W�F�N�g�������I�ɐ���/�j������N���X
class CNxDrawAutoDestroyer
{
public:
	CNxDrawAutoDestroyer()
	{
		::InitializeCriticalSection(&g_criticalSection);
		CreateTableDynamic();
	}
	~CNxDrawAutoDestroyer()
	{
		CNxDraw::DestroyInstance();
		::DeleteCriticalSection(&g_criticalSection);
	}
} gNxDrawAutoDestroyer;

}	// namespace {


/////////////////////////////////////////////////////////////////////////////////////////
// private:
//	CNxDraw::CNxDraw()
// �T�v: CNxDraw �N���X�̃R���X�g���N�^
// ����: ---
// �ߒl: ---
/////////////////////////////////////////////////////////////////////////////////////////

CNxDraw::CNxDraw()
 : m_pImageLoader(new CNxImageLoader)
{
	// �N���f�B���N�g�����擾���� susie32 plug-in �����̃f�t�H���g�Ƃ���
#if !defined(NXDRAW_NO_SUSIE_SPI)
	TCHAR szModuleFileName[_MAX_FNAME];
	TCHAR szBuf[_MAX_FNAME];
	::GetModuleFileName(NULL, szModuleFileName, _MAX_FNAME);
	_tsplitpath_s(szModuleFileName, szBuf, _MAX_FNAME, NULL, 0, NULL, 0, NULL, 0);
	m_strSPIDirectory = szBuf;
	_tsplitpath_s(szModuleFileName, NULL, 0, szBuf, _MAX_FNAME, NULL, 0, NULL, 0);
	m_strSPIDirectory += szBuf;
	TCHAR chLast = m_strSPIDirectory[m_strSPIDirectory.length() - 1];
	if (chLast != '\\' && chLast != '/')
		m_strSPIDirectory += '\\';
#endif	// #if !defined(NXDRAW_NO_SUSIE_SPI)

	// MMX / 3DNow! �� CPU ���T�|�[�g���Ă��邩���ׂ�
	m_bSupportMMX = GetProcessorSupportMMX();
	if (m_bSupportMMX)	// ����͔O�̂��߁BMMX ���g�p�s�\�� 3DNow! ���g�p�ł��� CPU �͖���
	{
		m_bSupport3DNow = GetProcessorSupport3DNow();
	}
	else
	{
		m_bSupport3DNow = FALSE;
	}
	// �f�t�H���g�ł� MMX �� 3DNow! ���g�p����
	m_bEnable3DNow = m_bSupport3DNow;
#if !defined(NXDRAW_MMX_ONLY)
	m_bEnableMMX = m_bSupportMMX;
#endif	// #if !defined(NXDRAW_MMX_ONLY)
}

/////////////////////////////////////////////////////////////////////////////////////////
// private:
//	CNxDraw::~CNxDraw()
// �T�v: CNxDraw �N���X�̃f�X�g���N�^
// ����: ---
// �ߒl: ---
/////////////////////////////////////////////////////////////////////////////////////////

CNxDraw::~CNxDraw()
{
	Lock();

	// �e�L�X�g�`��p�T�[�t�F�X�̊J��
	CompactTextTemporarySurface();

	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDraw* CNxDraw::GetInstance()
// �T�v: CNxDraw �̃C���X�^���X���擾
// ����: �Ȃ�
// �ߒl: CNxDraw �ւ̃|�C���^
/////////////////////////////////////////////////////////////////////////////////////////

CNxDraw* CNxDraw::GetInstance()
{
	Lock();
	if (m_pInstance == NULL)
	{
		m_pInstance = new CNxDraw;
	}
	Unlock();
	return m_pInstance;
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDraw::DestroyInstance()
// �T�v: CNxDraw �̃C���X�^���X��j��
// ����: �Ȃ�
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::DestroyInstance()
{
	Lock();
	delete m_pInstance;
	m_pInstance = NULL;
	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL NxDraw::EnableMMX(BOOL bEnable)
// �T�v: MMX ���߂̎g�p/�s�g�p���w�肷��
// ����: BOOL bEnable ... MMX ���߂��g�p����Ȃ�� TRUE
// �ߒl: �ȑO�̏��
// ���l: �����I�� MMX ���g�p�����ł͂Ȃ��BbEnable �� TRUE �ɂ��Ă��A
//       �g�p�s�\�Ȃ�Ύg���邱�Ƃ͂Ȃ��B
/////////////////////////////////////////////////////////////////////////////////////////

#if !defined(NXDRAW_MMX_ONLY)
BOOL CNxDraw::EnableMMX(BOOL bEnable)
{
	Lock();
	bEnable &= m_bSupportMMX;
	std::swap(bEnable, m_bEnableMMX);
	Unlock();
	return bEnable;
}
#endif	// #if !defined(NXDRAW_MMX_ONLY)

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDraw::Enable3DNow(BOOL bEnable)
// �T�v: 3DNow! ���߂̎g�p/�s�g�p���w�肷��
// ����: BOOL bEnable ... 3DNow! ���߂��g�p����Ȃ�� TRUE
// �ߒl: �ȑO�̏��
// ���l: �����I�� 3DNow ���g�p�����ł͂Ȃ��BbEnable �� TRUE �ɂ��Ă��A
//       �g�p�s�\�Ȃ�Ύg���邱�Ƃ͂Ȃ��B
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDraw::Enable3DNow(BOOL bEnable)
{
	Lock();
	bEnable &= m_bSupport3DNow;
	std::swap(bEnable, m_bEnable3DNow);
	Unlock();
	return bEnable;
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDIBImage* CNxDraw::LoadImage(CNxFile& nxfile) const
// �T�v: CNxDIBImageLoader �h���N���X���g�p���ĉ摜��ǂݍ���
// ����: CNxFile& nxfile ... �ǂݍ��݌��t�@�C���ւ̎Q��
// �ߒl: CNxDIBImage �ւ̃|�C���^�B���s�Ȃ�� NULL
// ���l: �ȉ��� CNxDIBImageLoader �h���N���X���g�p�����
//       CNxBMPImageLoader
//       CNxPNGImageLoader (NXDRAW_IMAGELOADER_NO_PNG ����`����Ă��Ȃ��ꍇ)
//       CNxJPEGImageLoader (NXDRAW_IMAGELOADER_NO_JPEG ����`����Ă��Ȃ��ꍇ)
//		 CNxSPIImageLoader (NXDRAW_IMAGELOADER_NO_SUSIE_SPI ����`����Ă��Ȃ��ꍇ)
/////////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxDraw::LoadImage(CNxFile& nxfile) const
{
	return m_pImageLoader->CreateDIBImage(nxfile);
}

#if !defined(NXDRAW_LOADIMAGE_NO_SUSIE_SPI)

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxDraw::GetSPIDirectory(LPTSTR lpBuf, int nCount)
// �T�v: susie plug-in ����������f�B���N�g�����擾
// ����: LPTSTR lpBuf  ... �f�B���N�g�����󂯂Ƃ�o�b�t�@
//		 int    nCount ... �o�b�t�@�̒���
// �ߒl: �o�b�t�@�փR�s�[���ꂽ������(�k����������)
/////////////////////////////////////////////////////////////////////////////////////////

int CNxDraw::GetSPIDirectory(LPTSTR lpBuf, int nCount) const
{
	Lock();
	int nResult = StrCountCopy(lpBuf, m_strSPIDirectory.c_str(), nCount);
	Unlock();
	return nResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxDraw::GetSPIDirectory(std::basic_string<TCHAR>& rString)
// �T�v: susie plug-in ����������f�B���N�g�����擾
// ����: std::basic_string<TCHAR>& rString ... �f�B���N�g�����󂯎�� std::string �I�u�W�F�N�g�ւ̎Q��
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::GetSPIDirectory(std::basic_string<TCHAR>& rString) const
{
	Lock();
	rString = m_strSPIDirectory;
	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDraw::SetSPIDirectory(LPCTSTR lpszDirectory)
// �T�v: susie plug-in ����������f�B���N�g����ݒ�
// ����: LPCTSTR lpszDirectory ... �ݒ肷��f�B���N�g��(�h���C�u�����܂ސ�΃p�X)
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::SetSPIDirectory(LPCTSTR lpszDirectory)
{
	Lock();
	m_strSPIDirectory = lpszDirectory;
	TCHAR chLast = m_strSPIDirectory[m_strSPIDirectory.length() - 1];
	if (chLast != '\\' && chLast != '/')
		m_strSPIDirectory += '\\';

	// CNxImageLoader �Ō��ݓǂݍ��܂�Ă��� SPI ���J�����邽�߂ɍĐ���
	m_pImageLoader.reset(new CNxImageLoader);
	Unlock();
}
#endif	// #if !defined(NXDRAW_NO_SUSIE_SPI)

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDraw::CompactTextTemporaySurface()
// �T�v: �e�L�X�g�`��p�ꎞ�T�[�t�F�X��������(�J��)����
// ����: �Ȃ�
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////

void CNxDraw::CompactTextTemporarySurface()
{
	Lock();
	m_pTextTemporarySurface.reset();
	Unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	 HWND CNxDraw::SetFrameWnd(HWND hWndFrame)
// �T�v: ���b�Z�[�W�{�b�N�X�̕\�����Ɏg�p����I�[�i�[�̃E�B���h�E�n���h����ݒ�
// ����: HWND hWndFrame ... �I�[�i�[�E�B���h�E(�ʏ�̓t���[���E�B���h�E)
// �ߒl: �ȑO�̃n���h��(�����l�� NULL)
/////////////////////////////////////////////////////////////////////////////////////////

HWND CNxDraw::SetFrameWnd(HWND hWndFrame)
{
	std::swap(hWndFrame, m_hWndFrame);
	return hWndFrame;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CNxSurface* CNxDraw::GetTextTemporarySurface(UINT uWidth, UINT uHeight)
// �T�v: �e�L�X�g�`��p�ꎞ�T�[�t�F�X���擾(�w�肳�ꂽ���ƍ��������T�[�t�F�X��Ԃ�)
// ����: UINT uWidth  ... �T�[�t�F�X�ɗv�����镝
//       UINT uHeight ... �T�[�t�F�X�ɗv�����鍂��
// �ߒl: �T�[�t�F�X�ւ̃|�C���^�BNULL �Ȃ�Ύ��s
// ���l: �T�[�t�F�X�g�p�O�Ǝg�p��� Lock() / Unlock() ���s�Ȃ���
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CNxSurface* CNxDraw::GetTextTemporarySurface(UINT uWidth, UINT uHeight)
{
	if (m_pTextTemporarySurface.get() == 0
		|| m_pTextTemporarySurface->GetWidth() < uWidth
		|| m_pTextTemporarySurface->GetHeight() < uHeight)
	{	// ���쐬���͗v���T�C�Y��菬����
		m_pTextTemporarySurface.reset(new CNxSurface(8));
		if (!m_pTextTemporarySurface->Create(uWidth, uHeight))
		{
			m_pTextTemporarySurface.reset();
			_RPTF0(_CRT_ERROR, "�T�[�t�F�X�̍쐬�Ɏ��s���܂���.\n");
		}
	}
	return m_pTextTemporarySurface.get();
}
