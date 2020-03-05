// NxZipMMIOProc.cpp: CNxZipMMIOProc �N���X�̃C���v�������e�[�V����
//
// Zip �t�@�C������̓ǂݍ��݂��s�Ȃ��A�J�X�^�����o�̓v���V�[�W���N���X
// CNxZipFile �N���X���g�p����
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxZipMMIOProc.h"

using namespace NxStorageLocal;

namespace
{
	const LONG lZipInputBufferSize = 4096;
	const LONG lZipSeekBufferSize  = 1024;
};

////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Open(LPMMIOINFO lpmmioinfo, LPCSTR lpszFileName)
// �T�v: MMIOM_OPEN ���b�Z�[�W�̉����֐�
// ����: LPMMIOINFO lpmmioinfo   ... MMIOINFO �\���̂ւ̃|�C���^
//       LPCTSTR    lpszFileName ... �t�@�C�����ւ̃|�C���^
// �ߒl: �����Ȃ� 0
////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Open(LPMMIOINFO lpmmioinfo, LPCSTR lpszFileName)
{
	// ���ɂ���t�@�C����{��
	CNxZipArchive* pZipArchive = (CNxZipArchive*)lpmmioinfo->adwInfo[1];

	LPCTSTR lptFileName;

#if defined(UNICODE)
	// �Ȃ��� lpszFileName �́AMBCS �֕ϊ�����Ă��܂��Ă���...
	std::wstring strFileName;
	int nFileNameLength = strlen(lpszFileName);
	strFileName.resize(::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszFileName, nFileNameLength, NULL, 0));
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszFileName, nFileNameLength, strFileName.begin(), strFileName.size());
	lptFileName = strFileName.c_str();
#else
	lptFileName = lpszFileName;
#endif
	
	if (!pZipArchive->FindFile(lptFileName, &m_zipInfo.fileInfo))
	{	// �t�@�C����������Ȃ�
		delete this;
		_RPTF1(_CRT_ASSERT, "�t�@�C��'%s' �͂���܂���.\n", lpszFileName);
		return MMIOERR_CANNOTOPEN;
	}
	m_zipInfo.pZipArchive = pZipArchive->GetArchiveFile();
	m_zipInfo.zStream.zalloc = Z_NULL;
	m_zipInfo.zStream.zfree = Z_NULL;
	m_zipInfo.zStream.opaque = Z_NULL;
	m_zipInfo.zStream.avail_in = 0;
	m_zipInfo.lOffsetZip = 0;				// ���k�f�[�^�擪����̃I�t�Z�b�g
	m_zipInfo.lPrevDiskOffset = 0;			// �O��̃f�B�X�N�I�t�Z�b�g
	m_zipInfo.dwCRC32 = 0;					// CRC32 checksum
	m_zipInfo.pbZipInputBuffer = NULL;		// Zip ���k�f�[�^�o�b�t�@
	m_zipInfo.pbZipSeekBuffer = NULL;		// Zip Seek �p�o�b�t�@
	if (m_zipInfo.fileInfo.nCompressionMethod != 0)
	{	// ���k����Ă���(�o�b�t�@���m��)
		m_zipInfo.pbZipInputBuffer = new BYTE[lZipInputBufferSize];		// ���k�f�[�^���͗p�o�b�t�@
		m_zipInfo.pbZipSeekBuffer  = new BYTE[lZipSeekBufferSize];		// �V�[�N�p�̃e���|����

		// inflate ����
		if (::inflateInit2(&m_zipInfo.zStream, -MAX_WBITS) != Z_OK)
		{
			delete[] m_zipInfo.pbZipInputBuffer;
			delete[] m_zipInfo.pbZipSeekBuffer;
			delete this;
			_RPTF0(_CRT_ERROR, "::inflateInit2() �����s���܂���.\n");
			return MMIOERR_CANNOTOPEN;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Read(LPMMIOINFO lpmmioinfo, LPVOID lpBuffer, LONG lBytesToRead)
// �T�v: MMIOM_READ ���b�Z�[�W�̉����֐�
// ����: LPMMIOINFO lpmmioinfo ... MMIOINFO �\���̂ւ̃|�C���^
//		 LPVOID lpBuffer       ... �ǂݍ��ݐ�o�b�t�@�ւ̃|�C���^
//       LONG lBytesToRead     ... �ǂݍ��݃o�C�g��
// �ߒl: �����Ȃ�ǂݍ��܂ꂽ�o�C�g���B���s�Ȃ�� -1
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Read(LPMMIOINFO lpmmioinfo, LPVOID lpBuffer, LONG lBytesToRead)
{
	LONG lTotalReadBytes = 0;	// �t�@�C������ǂݍ��܂ꂽ�o�C�g��
	if (m_zipInfo.fileInfo.nCompressionMethod == 0)
	{	// �����k(�P���ɓǂݍ��ނ���)
		LONG lReadBytes;
		m_zipInfo.pZipArchive->Seek(lpmmioinfo->lDiskOffset + m_zipInfo.fileInfo.lOffsetFileData, SEEK_SET);
		lReadBytes = m_zipInfo.pZipArchive->Read(lpBuffer, lBytesToRead);
		if (lReadBytes == -1 || lReadBytes == 0)
			return lReadBytes;

		lTotalReadBytes = lReadBytes;
		lpBuffer = static_cast<LPBYTE>(lpBuffer) + lReadBytes;
	}
	else
	{	// ���k�ς݃f�[�^
		if (lpmmioinfo->lDiskOffset < m_zipInfo.lPrevDiskOffset)
		{	// ��߂肷��̂ŁA�擪����� inflate ����
			::inflateReset(&m_zipInfo.zStream);
			m_zipInfo.zStream.avail_in = 0;
			m_zipInfo.lOffsetZip = 0;
			m_zipInfo.dwCRC32 = 0;
			m_zipInfo.lPrevDiskOffset = 0;
		}

		// �ړI�̈ʒu�܂� inflate ���J��Ԃ�
		while (lpmmioinfo->lDiskOffset > m_zipInfo.lPrevDiskOffset)
		{
			int nError;
			if (m_zipInfo.zStream.avail_in == 0)
			{
				LONG lReadBytes;
				m_zipInfo.pZipArchive->Seek(m_zipInfo.lOffsetZip + m_zipInfo.fileInfo.lOffsetFileData, SEEK_SET);
				lReadBytes = min(m_zipInfo.fileInfo.lCompressedSize - m_zipInfo.lOffsetZip, lZipInputBufferSize);
				lReadBytes = m_zipInfo.pZipArchive->Read(m_zipInfo.pbZipInputBuffer, lReadBytes);
				if (lReadBytes == -1 || lReadBytes == 0)
					break;

				m_zipInfo.lOffsetZip += lReadBytes;
					
				m_zipInfo.zStream.next_in = m_zipInfo.pbZipInputBuffer;
				m_zipInfo.zStream.avail_in = lReadBytes;
			}
			m_zipInfo.zStream.next_out = m_zipInfo.pbZipSeekBuffer;
			m_zipInfo.zStream.avail_out = min(lZipSeekBufferSize, lpmmioinfo->lDiskOffset - m_zipInfo.lPrevDiskOffset);
			nError = ::inflate(&m_zipInfo.zStream, Z_SYNC_FLUSH);
			LONG lSeekBytes = (m_zipInfo.zStream.next_out - m_zipInfo.pbZipSeekBuffer);
			m_zipInfo.lPrevDiskOffset += lSeekBytes;
			m_zipInfo.dwCRC32 = ::crc32(m_zipInfo.dwCRC32, m_zipInfo.pbZipSeekBuffer, lSeekBytes);
			if (nError == Z_STREAM_END)
				break;
		}
		while (lBytesToRead != 0)
		{
			int nError;
			// �X�g���[�����ɓ��̓f�[�^���Ȃ���Γǂݍ���
			if (m_zipInfo.zStream.avail_in == 0)
			{
				// ���k�ǂݍ��ރo�C�g�����o�b�t�@�T�C�Y�ɐ���
				LONG lReadBytes = min(m_zipInfo.fileInfo.lCompressedSize - m_zipInfo.lOffsetZip, lZipInputBufferSize);
				// ���k�f�[�^�̃t�@�C���|�C���^�����݂̈ʒu�֐ݒ�
				m_zipInfo.pZipArchive->Seek(m_zipInfo.lOffsetZip + m_zipInfo.fileInfo.lOffsetFileData, SEEK_SET);
				lReadBytes = m_zipInfo.pZipArchive->Read(m_zipInfo.pbZipInputBuffer, lReadBytes);
				if (lReadBytes == -1)
					break;
	
				m_zipInfo.lOffsetZip += lReadBytes;						// ���k�f�[�^���I�t�Z�b�g��ǂݍ��񂾃o�C�g�������i�߂�
				m_zipInfo.zStream.next_in = m_zipInfo.pbZipInputBuffer;	// ���k�f�[�^�擪�ւ̃|�C���^
				m_zipInfo.zStream.avail_in = lReadBytes;				// ���k�f�[�^�̃T�C�Y
			}
			m_zipInfo.zStream.next_out = static_cast<LPBYTE>(lpBuffer);	// �L���o�b�t�@�ւ̃|�C���^
			m_zipInfo.zStream.avail_out = lBytesToRead;					// �L���o�b�t�@�T�C�Y

			nError = ::inflate(&m_zipInfo.zStream, Z_SYNC_FLUSH);
			LONG lInflateBytes = (LONG)m_zipInfo.zStream.next_out - (LONG)lpBuffer;// �o��(�L��)���ꂽ�o�C�g��
			lpBuffer = static_cast<LPBYTE>(lpBuffer) + lInflateBytes;
			lBytesToRead -= lInflateBytes;
			lTotalReadBytes += lInflateBytes;
			if (nError == Z_STREAM_END)
				break;
		}
	}
	// CRC �̍X�V
	m_zipInfo.dwCRC32 = ::crc32(m_zipInfo.dwCRC32, static_cast<LPBYTE>(lpBuffer) - lTotalReadBytes, lTotalReadBytes);
	// lDiskOffset ���X�V
	lpmmioinfo->lDiskOffset += lTotalReadBytes;
	m_zipInfo.lPrevDiskOffset = lpmmioinfo->lDiskOffset;

	// �ߒl�͓ǂݍ��񂾃o�C�g��
	return lTotalReadBytes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Read(LPMMIOINFO lpmmioinfo, LONG lOffset, LONG nSeekOrigin)
// �T�v: MMIOM_SEEK ���b�Z�[�W�̉����֐�
// ����: LPMMIOINFO lpmmioinfo ... MMIOINFO �\���̂ւ̃|�C���^
//		 LONG lOffset          ... �t�@�C���̈ʒu
//		 LONG nSeekOrigin      ... �ړ����@(SEEK_SET, SEEK_CUR, SEEK_END)
// �ߒl: �����Ȃ�V�����t�@�C���̈ʒu�B���s -1
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Seek(LPMMIOINFO lpmmioinfo, LONG lOffset, LONG nSeekOrigin)
{
	switch (nSeekOrigin)
	{
	case SEEK_SET: 
		// lOffset ���̂܂�
		break;
	case SEEK_CUR:
		lOffset += lpmmioinfo->lDiskOffset;
		break;
	default:
		lOffset += m_zipInfo.fileInfo.lUncompressedSize;
		break;
	}
	if (lOffset < 0 || lOffset > m_zipInfo.fileInfo.lUncompressedSize)
		return -1;			// error (�t�@�C�����͂ݏo���Ă���)

	lpmmioinfo->lDiskOffset = lOffset;
	return lOffset;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Write(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite)
// �T�v: MMIOM_WRITE ���b�Z�[�W�̉����֐�
// ����: LPMMIOINFO lpmmioinfo ... MMIOINFO �\���̂ւ̃|�C���^
//		 LPCVOID lpBuffer      ... �������݃f�[�^�ւ̃|�C���^
//       LONG lBytesToWrite    ... �������݃o�C�g��
// �ߒl: �����Ȃ珑�����܂ꂽ�o�C�g���B���s�Ȃ�� -1
// ���l: Zip �ւ̏������݂̓T�|�[�g���Ȃ�
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Write(LPMMIOINFO /*lpmmioinfo*/, LPCVOID /*lpBuffer*/, LONG /*lBytesToWrite*/)
{
	return -1;	// falied
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::WriteFlush(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite)
// �T�v: MMIOM_WRITEFLUSH ���b�Z�[�W�̉����֐� (�����o�b�t�@�t���b�V����������)
// ����: LPMMIOINFO lpmmioinfo ... MMIOINFO �\���̂ւ̃|�C���^
//		 LPCVOID lpBuffer      ... �������݃f�[�^�ւ̃|�C���^
//       LONG lBytesToWrite    ... �������݃o�C�g��
// �ߒl: �����Ȃ珑�����܂ꂽ�o�C�g���B���s�Ȃ�� -1
// ���l: Zip �ւ̏������݂̓T�|�[�g���Ȃ�
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::WriteFlush(LPMMIOINFO /*lpmmioinfo*/, LPCVOID /*lpBuffer*/, LONG /*lBytesToWrite*/)
{
	return -1;	// falied
}


////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Close(LPMMIOINFO lpmmioinfo, UINT fuOption)
// �T�v: MMIOM_CLOSE ���b�Z�[�W�̉����֐�
// ����: LPMMIOINFO lpmmioinfo ... MMIOINFO �\���̂ւ̃|�C���^
//       UINT fuOption         ... �I�v�V����
// �ߒl: �����Ȃ� 0
////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Close(LPMMIOINFO /*lpmmioinfo*/, UINT /*fuOption*/)
{
	if (m_zipInfo.fileInfo.nCompressionMethod != 0)
	{	// ���k����Ă����Ȃ�΁Ainflate �̌�n���ƃo�b�t�@���J��
		::inflateEnd(&m_zipInfo.zStream);
		delete[] m_zipInfo.pbZipInputBuffer;
		delete[] m_zipInfo.pbZipSeekBuffer;
	}
	delete this;
	return 0;
}
