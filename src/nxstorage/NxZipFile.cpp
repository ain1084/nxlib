// NxZipFile.cpp: CNxZipFile クラスのインプリメンテーション
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
// 概要: CNxZipFile クラスのコンストラクタ
// 引数: CNxZipArchive* pZipArchive ... CNxZipArchive クラスへのポインタ
// 戻値: なし
// 備考: CNxZipArchive へのポインタを設定するのみ。ファイルは開かない
////////////////////////////////////////////////////////////////////////////

CNxZipFile::CNxZipFile(CNxZipArchive* pZipArchive)
{
	m_pZipArchive = pZipArchive;
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxZipFile::CNxZipFile(CNxZipArchive* pZipArchive, LPCTSTR lpszFileName)
// 概要: CNxZipFile クラスのコンストラクタ
// 引数: CNxZipArchive* pZipArchive ... CNxZipArchive クラスへのポインタ
//       LPCTSTR lpszFileName       ... ファイル名へのポインタ
// 戻値: なし
// 備考: .ZIP アーカイブ内の指定されたファイルを開く
//       エラーの場合でも例外は送出しない。ファイルが開かれたか否かを調べる
//       には、CNxFile::IsOpen() 関数を使用する
///////////////////////////////////////////////////////////////////////////////

CNxZipFile::CNxZipFile(CNxZipArchive* pZipArchive, LPCTSTR lpszFileName)
{
	m_pZipArchive = pZipArchive;
	Open(lpszFileName);
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	CNxZipFile::~CNxZipFile()
// 概要: CNxZipFile クラスのデストラクタ
///////////////////////////////////////////////////////////////////////////////

CNxZipFile::~CNxZipFile()
{

}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxZipFile::Open(LPCTSTR lpFileName)
// 概要: .ZIP アーカイブ内のファイルを開く
// 引数: LPCTSTR lpszFileName ... ファイル名へのポインタ
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxZipFile::Open(LPCTSTR lpszFileName)
{
	if (m_pZipArchive->GetArchiveFile() == NULL || !m_pZipArchive->GetArchiveFile()->IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxZipArchive オブジェクトが不正です.\n");
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
