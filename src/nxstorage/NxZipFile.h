// NxZipFile.h: CNxZipFile �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxFile.h"
#include "NxZipArchive.h"

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
