// NxZipArchive.cpp: CNxZipArchive クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxZipArchive.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxZipArchive::CNxZipArchive(CNxFile* pFileArchive)
{
	m_pFileArchive = NULL;
	if (!pFileArchive->IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "アーカイブファイルがオープンされていません.\n");
	}
	m_pFileArchive = pFileArchive;
	m_lNextOffset = 0;				// 次に検索を開始するファイルオフセット
}

CNxZipArchive::~CNxZipArchive()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxZipArchive::FindFile(LPCTSTR lpszFileName, ZipFileInfo* pZipFileInfo) const
// 概要: アーカイブ内の指定されたファイルに関する情報を得る
// 引数: LPCTSTR lpszFileName      ... ファイル名
//       ZipFileInfo* pZipFileInfo ... ファイル情報を受けとる ZipFileInfo 構造体へのポインタ
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxZipArchive::FindFile(LPCTSTR lpszFileName, ZipFileInfo* pZipFileInfo) const
{
	if (m_pFileArchive == NULL)
		return FALSE;

	// まずキャッシュ内から検索
	ZipFileInfoContainer::const_iterator it = m_zipFileInfoList.find(lpszFileName);
	if (it != m_zipFileInfoList.end())
	{	// 発見
		*pZipFileInfo = it->second;
		return TRUE;
	}

	// 見つからないのでファイルから検索する
	// その過程で見つかったファイルはキャッシュへ追加
	// 目的のファイルが見つかれば終了

	for (;;)
	{
		std::basic_string<TCHAR> strFileName;
		ZipFileInfo zipFileInfo;

		// ヘッダの開始までシーク
		if (m_pFileArchive->Seek(m_lNextOffset, SEEK_SET) == -1)
			break;		// シーク失敗

		// ヘッダを読み込む
		if (m_pFileArchive->Read(&zipFileInfo, ZipFileInfo::nZipLocalFileHeaderSize) != ZipFileInfo::nZipLocalFileHeaderSize)
			break;		// ファイル終端?

		// ヘッダの Signature をチェック
		if (zipFileInfo.dwSignature != 0x04034b50)
			break;		// local header ではない

		// 追加する ZipFileInfo の準備
		int nFileNameLength = zipFileInfo.nFileNameLength;
		m_lNextOffset += ZipFileInfo::nZipLocalFileHeaderSize + zipFileInfo.nFileNameLength + zipFileInfo.nExtraFieldLength;
		zipFileInfo.lOffsetFileData = m_lNextOffset;

		// ファイル名の読み込み
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
		// map へ追加
		m_zipFileInfoList[strFileName] = zipFileInfo;

		// 次のファイルの準備
		m_lNextOffset += zipFileInfo.lCompressedSize;

		// 今回のファイル名と比較
		if (_tcsicmp(strFileName.c_str(), lpszFileName) == 0)
		{	// 発見
			*pZipFileInfo = zipFileInfo;
			return TRUE;
		}

	}
	return FALSE;
}
