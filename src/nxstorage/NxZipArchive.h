// NxZipArchive.h: CNxZipArchive クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxFile.h"
#include <map>
#include <string>

class CNxZipArchive
{
public:
	CNxZipArchive(CNxFile* pFileArchive);
	virtual ~CNxZipArchive();


#pragma pack(push, 1)
	struct ZipFileInfo
	{
		enum
		{
			nZipLocalFileHeaderSize = 30
		};
		DWORD dwSignature;			// 0
		WORD  wVersionExtract;		// 4
		WORD  wFlag;				// 6
		SHORT nCompressionMethod;	// 8
		WORD  wFileTime;			// 10
		WORD  wFileDate;			// 12
		DWORD dwCRC32;				// 14
		LONG  lCompressedSize;		// 18
		LONG  lUncompressedSize;	// 22
		SHORT nFileNameLength;		// 26
		SHORT nExtraFieldLength;	// 28
		WORD  wDummyForAlignment;	// 30
		LONG  lOffsetFileData;		// 32
	};
#pragma pack(pop)

	CNxFile* GetArchiveFile() const;
	BOOL FindFile(LPCTSTR lpszFileName, ZipFileInfo* pZipFileItem) const;
	int GetFileCount() const;

private:

	CNxFile* m_pFileArchive;

	struct FileNameCompareNoCase
	{
		inline bool operator()(const std::basic_string<TCHAR>& x, const std::basic_string<TCHAR>& y) const {
			return _tcsicmp(x.c_str(), y.c_str()) < 0; }
	};

	typedef std::map<std::basic_string<TCHAR>, ZipFileInfo, FileNameCompareNoCase> ZipFileInfoContainer;
	mutable ZipFileInfoContainer m_zipFileInfoList;
	mutable LONG m_lNextOffset;
};

inline CNxFile* CNxZipArchive::GetArchiveFile() const {
	return m_pFileArchive; }

inline int CNxZipArchive::GetFileCount() const {
	return m_zipFileInfoList.size(); }
