// NxZipArchive.cpp: CNxZipArchive �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxZipArchive.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxZipArchive::CNxZipArchive(CNxFile* pFileArchive)
{
	m_pFileArchive = NULL;
	if (!pFileArchive->IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "�A�[�J�C�u�t�@�C�����I�[�v������Ă��܂���.\n");
	}
	m_pFileArchive = pFileArchive;
	m_lNextOffset = 0;				// ���Ɍ������J�n����t�@�C���I�t�Z�b�g
}

CNxZipArchive::~CNxZipArchive()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxZipArchive::FindFile(LPCTSTR lpszFileName, ZipFileInfo* pZipFileInfo) const
// �T�v: �A�[�J�C�u���̎w�肳�ꂽ�t�@�C���Ɋւ�����𓾂�
// ����: LPCTSTR lpszFileName      ... �t�@�C����
//       ZipFileInfo* pZipFileInfo ... �t�@�C�������󂯂Ƃ� ZipFileInfo �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxZipArchive::FindFile(LPCTSTR lpszFileName, ZipFileInfo* pZipFileInfo) const
{
	if (m_pFileArchive == NULL)
		return FALSE;

	// �܂��L���b�V�������猟��
	ZipFileInfoContainer::const_iterator it = m_zipFileInfoList.find(lpszFileName);
	if (it != m_zipFileInfoList.end())
	{	// ����
		*pZipFileInfo = it->second;
		return TRUE;
	}

	// ������Ȃ��̂Ńt�@�C�����猟������
	// ���̉ߒ��Ō��������t�@�C���̓L���b�V���֒ǉ�
	// �ړI�̃t�@�C����������ΏI��

	for (;;)
	{
		std::basic_string<TCHAR> strFileName;
		ZipFileInfo zipFileInfo;

		// �w�b�_�̊J�n�܂ŃV�[�N
		if (m_pFileArchive->Seek(m_lNextOffset, SEEK_SET) == -1)
			break;		// �V�[�N���s

		// �w�b�_��ǂݍ���
		if (m_pFileArchive->Read(&zipFileInfo, ZipFileInfo::nZipLocalFileHeaderSize) != ZipFileInfo::nZipLocalFileHeaderSize)
			break;		// �t�@�C���I�[?

		// �w�b�_�� Signature ���`�F�b�N
		if (zipFileInfo.dwSignature != 0x04034b50)
			break;		// local header �ł͂Ȃ�

		// �ǉ����� ZipFileInfo �̏���
		int nFileNameLength = zipFileInfo.nFileNameLength;
		m_lNextOffset += ZipFileInfo::nZipLocalFileHeaderSize + zipFileInfo.nFileNameLength + zipFileInfo.nExtraFieldLength;
		zipFileInfo.lOffsetFileData = m_lNextOffset;

		// �t�@�C�����̓ǂݍ���
#if defined(UNICODE)
		std::string str;
		str.resize(nFileNameLength);
		m_pFileArchive->Read(str.begin(), nFileNameLength);
		strFileName.resize(::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), str.length(), NULL, 0));
		::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), str.length(), strFileName.begin(), strFileName.size());
#else
		strFileName.resize(nFileNameLength);
		m_pFileArchive->Read(&strFileName[0], nFileNameLength);
#endif
		// map �֒ǉ�
		m_zipFileInfoList[strFileName] = zipFileInfo;

		// ���̃t�@�C���̏���
		m_lNextOffset += zipFileInfo.lCompressedSize;

		// ����̃t�@�C�����Ɣ�r
		if (_tcsicmp(strFileName.c_str(), lpszFileName) == 0)
		{	// ����
			*pZipFileInfo = zipFileInfo;
			return TRUE;
		}

	}
	return FALSE;
}
