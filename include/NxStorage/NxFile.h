// NxFile.h: CNxFile クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxStorage.h"
#include <string>

class CNxFile
{
public:

	enum
	{
		modeRead       = 0x00000001,
		modeWrite      = 0x00000002,
		modeReadWrite  = modeRead|modeWrite,
		modeMask       = modeRead|modeWrite,
		modeCreate     = 0x00000004,
		modeNoTruncate = 0x00000008,
		shareDenyNone  = 0x00000000,
		shareDenyRead  = 0x00000010,
		shareDenyWrite = 0x00000020,
		shareExclusive = 0x00000030,
		shareMask      = 0x00000030,
	};

	CNxFile();
	CNxFile(LPCTSTR lpszFileName, UINT fuMode = CNxFile::modeRead);
	virtual ~CNxFile();
	virtual BOOL Open(LPCTSTR lpszFileName, UINT fuMode = CNxFile::modeRead);

	BOOL Close();
	BOOL Flush();
	LONG Read(LPVOID lpBuffer, LONG lBytesToRead);
	LONG Write(LPCVOID lpBuffer, LONG lBytesToWrite);
	LONG Seek(LONG lOffset, int nOrigin);
	LONG GetSize();
	int GetFileName(LPTSTR lpString, int nMaxCount) const;
	void GetFileName(std::basic_string<TCHAR>& rString) const;

	BOOL IsOpen() const;

	BOOL Attach(HMMIO hMMIO, LPCTSTR lpszFileName);
	HMMIO Detach();

private:
	HMMIO m_hMMIO;
	std::basic_string<TCHAR> m_strFileName;

	CNxFile(const CNxFile&);
	CNxFile& operator=(const CNxFile&);
};

inline BOOL CNxFile::IsOpen() const {
	return m_hMMIO != NULL; }
