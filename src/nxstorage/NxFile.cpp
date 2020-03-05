// NxFile.cpp: CNxFile �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#include "NxStorage.h"
#include "NxFile.h"

#pragma comment(lib, "winmm.lib")

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxFile::CNxFile()
{
	m_hMMIO = NULL;
}

CNxFile::CNxFile(LPCTSTR lpszFileName, UINT fuMode)
{
	m_hMMIO = NULL;
	if (!Open(lpszFileName, fuMode))
	{
		_RPTF1(_CRT_ASSERT, "�t�@�C�� '%s' �̃I�[�v���Ɏ��s���܂���.\n", lpszFileName);
	}
}

CNxFile::~CNxFile()
{
	if (m_hMMIO != NULL)
		Close();
}

BOOL CNxFile::Attach(HMMIO hMMIO, LPCTSTR lpszFileName)
{
	if (m_hMMIO != NULL)
		return FALSE;

	m_hMMIO = hMMIO;
	m_strFileName = lpszFileName;
	return TRUE;
}

HMMIO CNxFile::Detach()
{
	_ASSERTE(m_hMMIO != NULL);
	HMMIO hResult = m_hMMIO;
	m_hMMIO = NULL;
	return hResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxFile::Open(LPCTSTR lpszFileName, UINT fuMode = CNxFile::modeRead)
// �T�v: �t�@�C�����J��
// ����: LPCTSTR lpszFileName ... �t�@�C�����ւ̃|�C���^
//       UINT    fuMode       ... �I�[�v�����[�h
// �ߒl: �����Ȃ�� TRUE
//
// memo: mmioOpen �ł��̂܂܃t�@�C������n���ĊJ���ƁA128byte ���������邽�߁A
//       CreateFile �ŊJ���Ă���n���h����n���Ă���
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxFile::Open(LPCTSTR lpszFileName, UINT fuMode)
{
	_ASSERTE(lpszFileName != NULL);

	DWORD fdwAccess;		// CreateFile �A�N�Z�X���[�h
	DWORD fdwShareMode;		// CreateFile ���L���[�h
	DWORD fdwCreate;		// CreateFile �쐬���[�h
	DWORD fdwMMOpen;		// mmioOpen �֓n�����[�h

	// �A�N�Z�X���[�h(fdwAccess) �̐ݒ�
	switch (fuMode & modeMask)
	{
	case modeRead:
		fdwAccess = GENERIC_READ;
		fdwMMOpen = MMIO_READ;
		break;
	case modeWrite:
		fdwAccess = GENERIC_WRITE;
		fdwMMOpen = MMIO_WRITE;
		break;
	case modeReadWrite:
		fdwAccess = GENERIC_READ|GENERIC_WRITE;
		fdwMMOpen = MMIO_READWRITE;
		break;
	default:
		_RPTF0(_CRT_ASSERT, "�A�N�Z�X���[�h���w�肳��Ă��܂���.\n");
		return FALSE;
	}

	// ���L���[�h(fdwShareMode) �̐ݒ�
	// �f�t�H���g�͓ǂݏ������L
	fdwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE;
	switch (fuMode & shareMask)
	{
	case shareDenyRead:
		fdwShareMode &= ~FILE_SHARE_READ;
	case shareDenyWrite:
		fdwShareMode &= ~FILE_SHARE_WRITE;
	}

	// �쐬���[�h (fdwCreate) �̐ݒ�
	// �f�t�H���g�͊����t�@�C���̃I�[�v��
	fdwCreate = OPEN_EXISTING;			
	if (fuMode & modeCreate)
		fdwCreate = (fuMode & modeNoTruncate) ? OPEN_ALWAYS : CREATE_ALWAYS;

	// �t�@�C�����J��
	HANDLE hFile = ::CreateFile(lpszFileName, fdwAccess, fdwShareMode, NULL,
								fdwCreate, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	MMIOINFO mi;
	memset(&mi, 0, sizeof(MMIOINFO));
	mi.fccIOProc = FOURCC_DOS;						// MS-DOS(Win32) �t�@�C�����w��
	mi.pchBuffer = NULL;							// �f�[�^�ւ̃|�C���^(NULL)
	mi.adwInfo[0] = reinterpret_cast<DWORD>(hFile);	// �t�@�C���n���h�����w��

	HMMIO hMMIO = ::mmioOpen(NULL, &mi, fdwMMOpen|MMIO_ALLOCBUF);
	if (hMMIO == NULL)
		return FALSE;

	return Attach(hMMIO, lpszFileName);
}

///////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFile::Close()
// �T�v: �t�@�C�������
// ����: �Ȃ�
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////

BOOL CNxFile::Close()
{
	_ASSERTE(m_hMMIO != NULL);

	if (::mmioClose(m_hMMIO, 0) == 0)
	{
		Detach();
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFile:GetSize()
// �T�v: �t�@�C���T�C�Y�𓾂�
// ����: �Ȃ�
// �ߒl: �����Ȃ�΁A�t�@�C���T�C�Y(�o�C�g�P��)�B���s�Ȃ�� -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::GetSize()
{
	_ASSERTE(m_hMMIO != NULL);

	LONG lCurrent;
	LONG lResult = -1;

	lCurrent = Seek(0, SEEK_CUR);			// ���݂̈ʒu��ۑ�
	if (lCurrent != -1)
	{
		lResult = Seek(0, SEEK_END);		// �Ō�ֈړ����āA���̈ʒu�𓾂�(=�T�C�Y)
		Seek(lCurrent, SEEK_SET);			// ���ɖ߂�
	}
	return lResult;
}

///////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFile::Flush()
// �T�v: �o�b�t�@���t���b�V������
// ����: �Ȃ�
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////

BOOL CNxFile::Flush()
{
	_ASSERTE(m_hMMIO != NULL);
	return ::mmioFlush(m_hMMIO, (UINT)0) == 0;
}

///////////////////////////////////////////////////////////////////
// public:
//	LONG CNxFile::Read(LPVOID lpBuffer, LONG lBytesToRead)
// �T�v: �t�@�C������f�[�^��ǂݍ���
// ����: LPVOID lpBuffer   ... �ǂݍ��ݐ�ւ̃|�C���^
//       LONG lBytesToRead ... �ǂݍ��ރo�C�g��
// �ߒl: �ǂݍ��܂ꂽ�o�C�g���B�G���[�Ȃ�� -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::Read(LPVOID lpBuffer, LONG lBytesToRead)
{
	_ASSERTE(m_hMMIO != NULL);
	return ::mmioRead(m_hMMIO, static_cast<LPSTR>(lpBuffer), lBytesToRead);
}

///////////////////////////////////////////////////////////////////
// public:
//	LONG CNxFile::Write(LPCVOID lpBuffer, LONG lBytesToWrite)
// �T�v: �t�@�C���փf�[�^����������
// ����: LPCVOID lpBuffer   ... �������܂��f�[�^�ւ̃|�C���^
//       LONG lBytesToWrite ... �������ރo�C�g��
// �ߒl: �������܂ꂽ�o�C�g���B�G���[�Ȃ�� -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::Write(LPCVOID lpBuffer, LONG lBytesToWrite)
{
	_ASSERTE(m_hMMIO != NULL);
	return ::mmioWrite(m_hMMIO, static_cast<LPSTR>(const_cast<LPVOID>(lpBuffer)), lBytesToWrite);
}

///////////////////////////////////////////////////////////////////
// public:
//	LONG CNxFile::Seek(LONG lOffset, int nOrigin)
// �T�v: �t�@�C���|�C���^���ړ�����
// ����: LONG lOffset ... �ړ������(�o�C�g�P��)
//       int nOrigin  .. �ړ����@(SEEK_SET/SEEK_CUR/SEEK_END)
// �ߒl: �ړ���̃t�@�C���|�C���^�B�G���[�Ȃ�� -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::Seek(LONG lOffset, int nOrigin)
{
	_ASSERTE(nOrigin == SEEK_SET || nOrigin == SEEK_CUR || nOrigin == SEEK_END);
	_ASSERTE(m_hMMIO != NULL);

	return ::mmioSeek(m_hMMIO, lOffset, nOrigin);
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxFile::GetFileName(LPTSTR lpString, int nMaxCount) const
// �T�v: �A�N�Z�X�Ώۃt�@�C�������̕������(�ǂ߂�`��)�Ԃ�
// ����: LPTSTR lpString ... �o�b�t�@�ւ̃|�C���^
//       int nMaxCount   ... �������P�ʂ̃o�b�t�@�T�C�Y(�k�������܂�)
// �ߒl: �k���������܂܂Ȃ��R�s�[���ꂽ�������B�G���[�Ȃ�� -1
////////////////////////////////////////////////////////////////////////////////

int CNxFile::GetFileName(LPTSTR lpString, int nMaxCount) const
{
	if (!IsOpen())
		return -1;

	int nLength = m_strFileName.length();

	if (lpString == NULL)
		return nLength;

	if (nLength < nMaxCount)
		nLength++;
	else
		nLength = nMaxCount;

	memcpy(lpString, m_strFileName.c_str(), nLength * sizeof(TCHAR));
	return nLength;
}

void CNxFile::GetFileName(std::basic_string<TCHAR>& rString) const
{
	rString = m_strFileName;
}
