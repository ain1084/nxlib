// NxZipMMIOProc.cpp: CNxZipMMIOProc クラスのインプリメンテーション
//
// Zip ファイルからの読み込みを行なう、カスタム入出力プロシージャクラス
// CNxZipFile クラスが使用する
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
// 概要: MMIOM_OPEN メッセージの応答関数
// 引数: LPMMIOINFO lpmmioinfo   ... MMIOINFO 構造体へのポインタ
//       LPCTSTR    lpszFileName ... ファイル名へのポインタ
// 戻値: 成功なら 0
////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Open(LPMMIOINFO lpmmioinfo, LPCSTR lpszFileName)
{
	// 書庫からファイルを捜す
	CNxZipArchive* pZipArchive = (CNxZipArchive*)lpmmioinfo->adwInfo[1];

	LPCTSTR lptFileName;

#if defined(UNICODE)
	// なぜか lpszFileName は、MBCS へ変換されてしまっている...
	std::wstring strFileName;
	int nFileNameLength = strlen(lpszFileName);
	strFileName.resize(::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszFileName, nFileNameLength, NULL, 0));
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszFileName, nFileNameLength, strFileName.begin(), strFileName.size());
	lptFileName = strFileName.c_str();
#else
	lptFileName = lpszFileName;
#endif
	
	if (!pZipArchive->FindFile(lptFileName, &m_zipInfo.fileInfo))
	{	// ファイルが見つからない
		delete this;
		_RPTF1(_CRT_ASSERT, "ファイル'%s' はありません.\n", lpszFileName);
		return MMIOERR_CANNOTOPEN;
	}
	m_zipInfo.pZipArchive = pZipArchive->GetArchiveFile();
	m_zipInfo.zStream.zalloc = Z_NULL;
	m_zipInfo.zStream.zfree = Z_NULL;
	m_zipInfo.zStream.opaque = Z_NULL;
	m_zipInfo.zStream.avail_in = 0;
	m_zipInfo.lOffsetZip = 0;				// 圧縮データ先頭からのオフセット
	m_zipInfo.lPrevDiskOffset = 0;			// 前回のディスクオフセット
	m_zipInfo.dwCRC32 = 0;					// CRC32 checksum
	m_zipInfo.pbZipInputBuffer = NULL;		// Zip 圧縮データバッファ
	m_zipInfo.pbZipSeekBuffer = NULL;		// Zip Seek 用バッファ
	if (m_zipInfo.fileInfo.nCompressionMethod != 0)
	{	// 圧縮されている(バッファを確保)
		m_zipInfo.pbZipInputBuffer = new BYTE[lZipInputBufferSize];		// 圧縮データ入力用バッファ
		m_zipInfo.pbZipSeekBuffer  = new BYTE[lZipSeekBufferSize];		// シーク用のテンポラリ

		// inflate 準備
		if (::inflateInit2(&m_zipInfo.zStream, -MAX_WBITS) != Z_OK)
		{
			delete[] m_zipInfo.pbZipInputBuffer;
			delete[] m_zipInfo.pbZipSeekBuffer;
			delete this;
			_RPTF0(_CRT_ERROR, "::inflateInit2() が失敗しました.\n");
			return MMIOERR_CANNOTOPEN;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Read(LPMMIOINFO lpmmioinfo, LPVOID lpBuffer, LONG lBytesToRead)
// 概要: MMIOM_READ メッセージの応答関数
// 引数: LPMMIOINFO lpmmioinfo ... MMIOINFO 構造体へのポインタ
//		 LPVOID lpBuffer       ... 読み込み先バッファへのポインタ
//       LONG lBytesToRead     ... 読み込みバイト数
// 戻値: 成功なら読み込まれたバイト数。失敗ならば -1
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Read(LPMMIOINFO lpmmioinfo, LPVOID lpBuffer, LONG lBytesToRead)
{
	LONG lTotalReadBytes = 0;	// ファイルから読み込まれたバイト数
	if (m_zipInfo.fileInfo.nCompressionMethod == 0)
	{	// 無圧縮(単純に読み込むだけ)
		LONG lReadBytes;
		m_zipInfo.pZipArchive->Seek(lpmmioinfo->lDiskOffset + m_zipInfo.fileInfo.lOffsetFileData, SEEK_SET);
		lReadBytes = m_zipInfo.pZipArchive->Read(lpBuffer, lBytesToRead);
		if (lReadBytes == -1 || lReadBytes == 0)
			return lReadBytes;

		lTotalReadBytes = lReadBytes;
		lpBuffer = static_cast<LPBYTE>(lpBuffer) + lReadBytes;
	}
	else
	{	// 圧縮済みデータ
		if (lpmmioinfo->lDiskOffset < m_zipInfo.lPrevDiskOffset)
		{	// 後戻りするので、先頭から再 inflate する
			::inflateReset(&m_zipInfo.zStream);
			m_zipInfo.zStream.avail_in = 0;
			m_zipInfo.lOffsetZip = 0;
			m_zipInfo.dwCRC32 = 0;
			m_zipInfo.lPrevDiskOffset = 0;
		}

		// 目的の位置まで inflate を繰り返す
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
			// ストリーム内に入力データがなければ読み込む
			if (m_zipInfo.zStream.avail_in == 0)
			{
				// 圧縮読み込むバイト数をバッファサイズに制限
				LONG lReadBytes = min(m_zipInfo.fileInfo.lCompressedSize - m_zipInfo.lOffsetZip, lZipInputBufferSize);
				// 圧縮データのファイルポインタを現在の位置へ設定
				m_zipInfo.pZipArchive->Seek(m_zipInfo.lOffsetZip + m_zipInfo.fileInfo.lOffsetFileData, SEEK_SET);
				lReadBytes = m_zipInfo.pZipArchive->Read(m_zipInfo.pbZipInputBuffer, lReadBytes);
				if (lReadBytes == -1)
					break;
	
				m_zipInfo.lOffsetZip += lReadBytes;						// 圧縮データをオフセットを読み込んだバイト数だけ進める
				m_zipInfo.zStream.next_in = m_zipInfo.pbZipInputBuffer;	// 圧縮データ先頭へのポインタ
				m_zipInfo.zStream.avail_in = lReadBytes;				// 圧縮データのサイズ
			}
			m_zipInfo.zStream.next_out = static_cast<LPBYTE>(lpBuffer);	// 伸長バッファへのポインタ
			m_zipInfo.zStream.avail_out = lBytesToRead;					// 伸長バッファサイズ

			nError = ::inflate(&m_zipInfo.zStream, Z_SYNC_FLUSH);
			LONG lInflateBytes = (LONG)m_zipInfo.zStream.next_out - (LONG)lpBuffer;// 出力(伸長)されたバイト数
			lpBuffer = static_cast<LPBYTE>(lpBuffer) + lInflateBytes;
			lBytesToRead -= lInflateBytes;
			lTotalReadBytes += lInflateBytes;
			if (nError == Z_STREAM_END)
				break;
		}
	}
	// CRC の更新
	m_zipInfo.dwCRC32 = ::crc32(m_zipInfo.dwCRC32, static_cast<LPBYTE>(lpBuffer) - lTotalReadBytes, lTotalReadBytes);
	// lDiskOffset を更新
	lpmmioinfo->lDiskOffset += lTotalReadBytes;
	m_zipInfo.lPrevDiskOffset = lpmmioinfo->lDiskOffset;

	// 戻値は読み込んだバイト数
	return lTotalReadBytes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Read(LPMMIOINFO lpmmioinfo, LONG lOffset, LONG nSeekOrigin)
// 概要: MMIOM_SEEK メッセージの応答関数
// 引数: LPMMIOINFO lpmmioinfo ... MMIOINFO 構造体へのポインタ
//		 LONG lOffset          ... ファイルの位置
//		 LONG nSeekOrigin      ... 移動方法(SEEK_SET, SEEK_CUR, SEEK_END)
// 戻値: 成功なら新しいファイルの位置。失敗 -1
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Seek(LPMMIOINFO lpmmioinfo, LONG lOffset, LONG nSeekOrigin)
{
	switch (nSeekOrigin)
	{
	case SEEK_SET: 
		// lOffset そのまま
		break;
	case SEEK_CUR:
		lOffset += lpmmioinfo->lDiskOffset;
		break;
	default:
		lOffset += m_zipInfo.fileInfo.lUncompressedSize;
		break;
	}
	if (lOffset < 0 || lOffset > m_zipInfo.fileInfo.lUncompressedSize)
		return -1;			// error (ファイルをはみ出している)

	lpmmioinfo->lDiskOffset = lOffset;
	return lOffset;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Write(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite)
// 概要: MMIOM_WRITE メッセージの応答関数
// 引数: LPMMIOINFO lpmmioinfo ... MMIOINFO 構造体へのポインタ
//		 LPCVOID lpBuffer      ... 書き込みデータへのポインタ
//       LONG lBytesToWrite    ... 書き込みバイト数
// 戻値: 成功なら書き込まれたバイト数。失敗ならば -1
// 備考: Zip への書き込みはサポートしない
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Write(LPMMIOINFO /*lpmmioinfo*/, LPCVOID /*lpBuffer*/, LONG /*lBytesToWrite*/)
{
	return -1;	// falied
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::WriteFlush(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite)
// 概要: MMIOM_WRITEFLUSH メッセージの応答関数 (内部バッファフラッシュ書き込み)
// 引数: LPMMIOINFO lpmmioinfo ... MMIOINFO 構造体へのポインタ
//		 LPCVOID lpBuffer      ... 書き込みデータへのポインタ
//       LONG lBytesToWrite    ... 書き込みバイト数
// 戻値: 成功なら書き込まれたバイト数。失敗ならば -1
// 備考: Zip への書き込みはサポートしない
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::WriteFlush(LPMMIOINFO /*lpmmioinfo*/, LPCVOID /*lpBuffer*/, LONG /*lBytesToWrite*/)
{
	return -1;	// falied
}


////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	LRESULT CNxZipMMIOProc::Close(LPMMIOINFO lpmmioinfo, UINT fuOption)
// 概要: MMIOM_CLOSE メッセージの応答関数
// 引数: LPMMIOINFO lpmmioinfo ... MMIOINFO 構造体へのポインタ
//       UINT fuOption         ... オプション
// 戻値: 成功なら 0
////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxZipMMIOProc::Close(LPMMIOINFO /*lpmmioinfo*/, UINT /*fuOption*/)
{
	if (m_zipInfo.fileInfo.nCompressionMethod != 0)
	{	// 圧縮されていたならば、inflate の後始末とバッファを開放
		::inflateEnd(&m_zipInfo.zStream);
		delete[] m_zipInfo.pbZipInputBuffer;
		delete[] m_zipInfo.pbZipSeekBuffer;
	}
	delete this;
	return 0;
}
