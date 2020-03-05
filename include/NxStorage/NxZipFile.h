// NxZipFile.h: CNxZipFile クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxFile.h"

class CNxZipArchive;

class CNxZipFile : public CNxFile
{
public:
	CNxZipFile(CNxZipArchive* pZipArchive);
	CNxZipFile(CNxZipArchive* pZipArchive, LPCTSTR lpszFileName);
	virtual ~CNxZipFile();
	virtual BOOL Open(LPCTSTR lpszFileName);

private:
	CNxZipArchive* m_pZipArchive;

	CNxZipFile(const CNxZipFile&);
	CNxZipFile& operator=(const CNxZipFile&);
};
