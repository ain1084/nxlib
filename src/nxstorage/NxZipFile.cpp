// NxZipFile.cpp: CNxZipFile �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxZipFile.h"
#include "NxZipMMIOProc.h"
#include <sstream>

using namespace NxStorageLocal;

////////////////////////////////////////////////////////////////////////////
// public:
//	CNxZipFile::CNxZipFile(CNxZipArchive* pArchive)
// �T�v: CNxZipFile �N���X�̃R���X�g���N�^
// ����: CNxZipArchive* pZipArchive ... CNxZipArchive �N���X�ւ̃|�C���^
// �ߒl: �Ȃ�
// ���l: CNxZipArchive �ւ̃|�C���^��ݒ肷��̂݁B�t�@�C���͊J���Ȃ�
////////////////////////////////////////////////////////////////////////////

CNxZipFile::CNxZipFile(CNxZipArchive* pZipArchive)
{
	m_pZipArchive = pZipArchive;
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxZipFile::CNxZipFile(CNxZipArchive* pZipArchive, LPCTSTR lpszFileName)
// �T�v: CNxZipFile �N���X�̃R���X�g���N�^
// ����: CNxZipArchive* pZipArchive ... CNxZipArchive �N���X�ւ̃|�C���^
//       LPCTSTR lpszFileName       ... �t�@�C�����ւ̃|�C���^
// �ߒl: �Ȃ�
// ���l: .ZIP �A�[�J�C�u���̎w�肳�ꂽ�t�@�C�����J��
//       �G���[�̏ꍇ�ł���O�͑��o���Ȃ��B�t�@�C�����J���ꂽ���ۂ��𒲂ׂ�
//       �ɂ́ACNxFile::IsOpen() �֐����g�p����
///////////////////////////////////////////////////////////////////////////////

CNxZipFile::CNxZipFile(CNxZipArchive* pZipArchive, LPCTSTR lpszFileName)
{
	m_pZipArchive = pZipArchive;
	Open(lpszFileName);
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	CNxZipFile::~CNxZipFile()
// �T�v: CNxZipFile �N���X�̃f�X�g���N�^
///////////////////////////////////////////////////////////////////////////////

CNxZipFile::~CNxZipFile()
{

}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxZipFile::Open(LPCTSTR lpFileName)
// �T�v: .ZIP �A�[�J�C�u���̃t�@�C�����J��
// ����: LPCTSTR lpszFileName ... �t�@�C�����ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxZipFile::Open(LPCTSTR lpszFileName)
{
	if (m_pZipArchive->GetArchiveFile() == NULL || !m_pZipArchive->GetArchiveFile()->IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxZipArchive �I�u�W�F�N�g���s���ł�.\n");
		return FALSE;
	}

	CNxZipMMIOProc* pMMIOProc = new CNxZipMMIOProc;
	HMMIO hMMIO = pMMIOProc->Create(lpszFileName, MMIO_READ, reinterpret_cast<DWORD>(m_pZipArchive));
	if (hMMIO == NULL)
	{
		delete pMMIOProc;
		return FALSE;
	}

	std::basic_string<TCHAR> strArchiveFileName;
	m_pZipArchive->GetArchiveFile()->GetFileName(strArchiveFileName);

	std::basic_ostringstream<TCHAR> strFileName;
	strFileName << _T("ZipFile'") << lpszFileName << '\'' << std::endl;
	strFileName << _T("in archive:") << strArchiveFileName << std::ends;
	return Attach(hMMIO, strFileName.str().c_str());
}
