// NxZipMMIOProc.h: CNxZipMMIOProc �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxMMIOProc.h"
#include "NxZipArchive.h"
#include "../NxShared/zlib/zlib.h"

namespace NxStorageLocal
{
	class CNxZipMMIOProc : public CNxMMIOProc  
	{
	protected:
		virtual LRESULT Open(LPMMIOINFO lpmmioinfo, LPCSTR lpszFileName);
		virtual LRESULT Close(LPMMIOINFO lpmmioinfo, UINT fuOption);
		virtual LRESULT Read(LPMMIOINFO lpmmioinfo, LPVOID lpBuffer, LONG lBytesToRead);
		virtual LRESULT Write(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite);
		virtual LRESULT WriteFlush(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite);
		virtual LRESULT Seek(LPMMIOINFO lpmmioinfo, LONG lOffset, LONG nSeekOrigin);

	private:
		struct
		{
			CNxZipArchive::ZipFileInfo fileInfo;	// Zip �t�@�C�����(�� local file header)
			z_stream zStream;						// zlib stream
			CNxFile* pZipArchive;					// Zip �A�[�J�C�u�t�@�C�������� CNxFile �N���X�ւ̃|�C���^
			LPBYTE pbZipInputBuffer;				// Zip ���k�f�[�^�o�b�t�@
			LPBYTE pbZipSeekBuffer;					// Zip Seek �p�W�J�o�b�t�@
			LONG lOffsetZip;						// ���k�f�[�^�{�̐擪����̃I�t�Z�b�g(start = 0)
			LONG lPrevDiskOffset;					// �V�[�N�O�̃I�t�Z�b�g
			DWORD dwCRC32;							// CRC32 (���݂̂Ƃ���A�X�V�̂݁B�`�F�b�N�͍s��Ȃ�)
		} m_zipInfo;
	};
}	// namespace NxStorageLocal
