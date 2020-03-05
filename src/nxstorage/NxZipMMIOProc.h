// NxZipMMIOProc.h: CNxZipMMIOProc クラスのインターフェイス
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
			CNxZipArchive::ZipFileInfo fileInfo;	// Zip ファイル情報(含 local file header)
			z_stream zStream;						// zlib stream
			CNxFile* pZipArchive;					// Zip アーカイブファイルを示す CNxFile クラスへのポインタ
			LPBYTE pbZipInputBuffer;				// Zip 圧縮データバッファ
			LPBYTE pbZipSeekBuffer;					// Zip Seek 用展開バッファ
			LONG lOffsetZip;						// 圧縮データ本体先頭からのオフセット(start = 0)
			LONG lPrevDiskOffset;					// シーク前のオフセット
			DWORD dwCRC32;							// CRC32 (現在のところ、更新のみ。チェックは行わない)
		} m_zipInfo;
	};
}	// namespace NxStorageLocal
