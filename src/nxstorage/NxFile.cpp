// NxFile.cpp: CNxFile クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#include "NxStorage.h"
#include "NxFile.h"

#pragma comment(lib, "winmm.lib")

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxFile::CNxFile()
{
	m_hMMIO = NULL;
}

CNxFile::CNxFile(LPCTSTR lpszFileName, UINT fuMode)
{
	m_hMMIO = NULL;
	if (!Open(lpszFileName, fuMode))
	{
		_RPTF1(_CRT_ASSERT, "ファイル '%s' のオープンに失敗しました.\n", lpszFileName);
	}
}

CNxFile::~CNxFile()
{
	if (m_hMMIO != NULL)
		Close();
}

BOOL CNxFile::Attach(HMMIO hMMIO, LPCTSTR lpszFileName)
{
	if (m_hMMIO != NULL)
		return FALSE;

	m_hMMIO = hMMIO;
	m_strFileName = lpszFileName;
	return TRUE;
}

HMMIO CNxFile::Detach()
{
	_ASSERTE(m_hMMIO != NULL);
	HMMIO hResult = m_hMMIO;
	m_hMMIO = NULL;
	return hResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxFile::Open(LPCTSTR lpszFileName, UINT fuMode = CNxFile::modeRead)
// 概要: ファイルを開く
// 引数: LPCTSTR lpszFileName ... ファイル名へのポインタ
//       UINT    fuMode       ... オープンモード
// 戻値: 成功ならば TRUE
//
// memo: mmioOpen でそのままファイル名を渡して開くと、128byte 制限があるため、
//       CreateFile で開いてからハンドルを渡している
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxFile::Open(LPCTSTR lpszFileName, UINT fuMode)
{
	_ASSERTE(lpszFileName != NULL);

	DWORD fdwAccess;		// CreateFile アクセスモード
	DWORD fdwShareMode;		// CreateFile 共有モード
	DWORD fdwCreate;		// CreateFile 作成モード
	DWORD fdwMMOpen;		// mmioOpen へ渡すモード

	// アクセスモード(fdwAccess) の設定
	switch (fuMode & modeMask)
	{
	case modeRead:
		fdwAccess = GENERIC_READ;
		fdwMMOpen = MMIO_READ;
		break;
	case modeWrite:
		fdwAccess = GENERIC_WRITE;
		fdwMMOpen = MMIO_WRITE;
		break;
	case modeReadWrite:
		fdwAccess = GENERIC_READ|GENERIC_WRITE;
		fdwMMOpen = MMIO_READWRITE;
		break;
	default:
		_RPTF0(_CRT_ASSERT, "アクセスモードが指定されていません.\n");
		return FALSE;
	}

	// 共有モード(fdwShareMode) の設定
	// デフォルトは読み書き共有
	fdwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE;
	switch (fuMode & shareMask)
	{
	case shareDenyRead:
		fdwShareMode &= ~FILE_SHARE_READ;
	case shareDenyWrite:
		fdwShareMode &= ~FILE_SHARE_WRITE;
	}

	// 作成モード (fdwCreate) の設定
	// デフォルトは既存ファイルのオープン
	fdwCreate = OPEN_EXISTING;			
	if (fuMode & modeCreate)
		fdwCreate = (fuMode & modeNoTruncate) ? OPEN_ALWAYS : CREATE_ALWAYS;

	// ファイルを開く
	HANDLE hFile = ::CreateFile(lpszFileName, fdwAccess, fdwShareMode, NULL,
								fdwCreate, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	MMIOINFO mi;
	memset(&mi, 0, sizeof(MMIOINFO));
	mi.fccIOProc = FOURCC_DOS;						// MS-DOS(Win32) ファイルを指定
	mi.pchBuffer = NULL;							// データへのポインタ(NULL)
	mi.adwInfo[0] = reinterpret_cast<DWORD>(hFile);	// ファイルハンドルを指定

	HMMIO hMMIO = ::mmioOpen(NULL, &mi, fdwMMOpen|MMIO_ALLOCBUF);
	if (hMMIO == NULL)
		return FALSE;

	return Attach(hMMIO, lpszFileName);
}

///////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFile::Close()
// 概要: ファイルを閉じる
// 引数: なし
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////

BOOL CNxFile::Close()
{
	_ASSERTE(m_hMMIO != NULL);

	if (::mmioClose(m_hMMIO, 0) == 0)
	{
		Detach();
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFile:GetSize()
// 概要: ファイルサイズを得る
// 引数: なし
// 戻値: 成功ならば、ファイルサイズ(バイト単位)。失敗ならば -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::GetSize()
{
	_ASSERTE(m_hMMIO != NULL);

	LONG lCurrent;
	LONG lResult = -1;

	lCurrent = Seek(0, SEEK_CUR);			// 現在の位置を保存
	if (lCurrent != -1)
	{
		lResult = Seek(0, SEEK_END);		// 最後へ移動して、その位置を得る(=サイズ)
		Seek(lCurrent, SEEK_SET);			// 元に戻す
	}
	return lResult;
}

///////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxFile::Flush()
// 概要: バッファをフラッシュする
// 引数: なし
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////

BOOL CNxFile::Flush()
{
	_ASSERTE(m_hMMIO != NULL);
	return ::mmioFlush(m_hMMIO, (UINT)0) == 0;
}

///////////////////////////////////////////////////////////////////
// public:
//	LONG CNxFile::Read(LPVOID lpBuffer, LONG lBytesToRead)
// 概要: ファイルからデータを読み込む
// 引数: LPVOID lpBuffer   ... 読み込み先へのポインタ
//       LONG lBytesToRead ... 読み込むバイト数
// 戻値: 読み込まれたバイト数。エラーならば -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::Read(LPVOID lpBuffer, LONG lBytesToRead)
{
	_ASSERTE(m_hMMIO != NULL);
	return ::mmioRead(m_hMMIO, static_cast<LPSTR>(lpBuffer), lBytesToRead);
}

///////////////////////////////////////////////////////////////////
// public:
//	LONG CNxFile::Write(LPCVOID lpBuffer, LONG lBytesToWrite)
// 概要: ファイルへデータを書き込む
// 引数: LPCVOID lpBuffer   ... 書き込まれるデータへのポインタ
//       LONG lBytesToWrite ... 書き込むバイト数
// 戻値: 書き込まれたバイト数。エラーならば -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::Write(LPCVOID lpBuffer, LONG lBytesToWrite)
{
	_ASSERTE(m_hMMIO != NULL);
	return ::mmioWrite(m_hMMIO, static_cast<LPSTR>(const_cast<LPVOID>(lpBuffer)), lBytesToWrite);
}

///////////////////////////////////////////////////////////////////
// public:
//	LONG CNxFile::Seek(LONG lOffset, int nOrigin)
// 概要: ファイルポインタを移動する
// 引数: LONG lOffset ... 移動する量(バイト単位)
//       int nOrigin  .. 移動方法(SEEK_SET/SEEK_CUR/SEEK_END)
// 戻値: 移動後のファイルポインタ。エラーならば -1
///////////////////////////////////////////////////////////////////

LONG CNxFile::Seek(LONG lOffset, int nOrigin)
{
	_ASSERTE(nOrigin == SEEK_SET || nOrigin == SEEK_CUR || nOrigin == SEEK_END);
	_ASSERTE(m_hMMIO != NULL);

	return ::mmioSeek(m_hMMIO, lOffset, nOrigin);
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxFile::GetFileName(LPTSTR lpString, int nMaxCount) const
// 概要: アクセス対象ファイル名等の文字列を(読める形で)返す
// 引数: LPTSTR lpString ... バッファへのポインタ
//       int nMaxCount   ... 文字数単位のバッファサイズ(ヌル文字含む)
// 戻値: ヌル文字を含まないコピーされた文字数。エラーならば -1
////////////////////////////////////////////////////////////////////////////////

int CNxFile::GetFileName(LPTSTR lpString, int nMaxCount) const
{
	if (!IsOpen())
		return -1;

	int nLength = m_strFileName.length();

	if (lpString == NULL)
		return nLength;

	if (nLength < nMaxCount)
		nLength++;
	else
		nLength = nMaxCount;

	memcpy(lpString, m_strFileName.c_str(), nLength * sizeof(TCHAR));
	return nLength;
}

void CNxFile::GetFileName(std::basic_string<TCHAR>& rString) const
{
	rString = m_strFileName;
}
