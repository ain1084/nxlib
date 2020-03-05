// NxBMPImageLoader.cpp: CNxBMPImageLoader クラスのインプリメンテーション
// Copyright(c) 2000,2001 S.Ainoguchi
//
// 概要: BMP 画像を読み込み、CNxDIBImage を返す
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <memory>
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxDIBImage.h"
#include "NxBMPImageLoader.h"

//////////////////////////////////////////////////////////////////////
// public:
//	CNxBMPImageLoader::CNxBMPImageLoader()
// 概要: CNxBMPImageLoader クラスのデフォルトコンストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxBMPImageLoader::CNxBMPImageLoader()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxBMPImageLoader::~CNxBMPImageLoader()
// 概要: CNxBMPImageLoader クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxBMPImageLoader::~CNxBMPImageLoader()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxBMPImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// 概要: 展開可能なデータ形式であるかを調べる
// 引数: LPCVOID lpvBuf ... データの最初から 2048byte を読み込んだバッファへのポインタ
//       LONG lLength   ... データのサイズ(通常は 2048)
// 戻値: 展開可能であれば TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxBMPImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
{
	const BITMAPINFO* lpbmi;
	if (*static_cast<const WORD*>(lpvBuf) == MAKEWORD('B', 'M'))
	{	// 最初が 'BM' ならば BITMAPFILEHEADER  を skip
		lLength -= sizeof(BITMAPFILEHEADER);
		lpbmi = reinterpret_cast<const BITMAPINFO*>(static_cast<const BITMAPFILEHEADER*>(lpvBuf) + 1);
	}
	else
	{	// 最初が 'BM' でなければ BITMAPINFO と見なす
		lpbmi = reinterpret_cast<const BITMAPINFO*>(lpvBuf);
	}

	if (lpbmi->bmiHeader.biSize < sizeof(BITMAPINFOHEADER))
		return FALSE;		// BITMAPINFOHEADER のサイズが異常

	if (lpbmi->bmiHeader.biPlanes != 1)
		return FALSE;		// プレーン数は 1 でなければならない

	// biBitCount のチェック
	switch (lpbmi->bmiHeader.biBitCount)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
		return TRUE;
	default:
		return FALSE;	// biBitCount が異常
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxBMPImageLoader::CreateDIBImage(CNxFile& nxfile)
// 概要: 画像を展開して CNxDIBImage オブジェクトを返す
// 引数: CNxFile& nxfile ... CNxFile オブジェクトへの参照
// 戻値: 成功ならば、作成した CNxDIBImage オブジェクトへのポインタ。失敗ならば NULL
///////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxBMPImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	WORD wBFSig;		// BMP signature ('BM')
	LONG lFileSize;
	DWORD dwBmihSize;

	lFileSize = nxfile.GetSize();				// サイズ

	// 最初の 2byte を wBFSig へ読み込む
	if (nxfile.Read(&wBFSig, sizeof(WORD)) == 0)
	{
		_RPTF0(_CRT_ERROR, _T("ファイルの読み込みに失敗.\n"));
		return NULL;
	}
	
	// signature の調査
	if (wBFSig == MAKEWORD('B', 'M'))
	{
		// 最初が 'BM' なので、BITMAP file
		// BITMAPFILEHEADER が付いているので skip する
		lFileSize -= sizeof(BITMAPFILEHEADER);
		nxfile.Seek(sizeof(BITMAPFILEHEADER) - sizeof(WORD), SEEK_CUR);		// BITMAPPFILEHEADER を skip

		// BITMAPINFOHEADER のサイズを読み込む
		if (nxfile.Read(&dwBmihSize, sizeof(DWORD)) != sizeof(DWORD))
			return FALSE;
	}
	else
	{	// 最初が 'BM' 以外であれば BITMAPINFOHEADER と見なす
		// ここで wBFSig は BITMAPINFOHEADER のサイズの下位 word が入っている
		// dwBmihSize の上位は、ファイルから word を読み込み、下位へは wBFSig を設定し、
		// BITMAPINFOHEADER のサイズ(biSize) とする
		dwBmihSize = wBFSig;
		if (nxfile.Read(reinterpret_cast<LPWORD>(&dwBmihSize) + 1, sizeof(WORD)) != sizeof(WORD))
			return FALSE;
	}
	// サイズが sizeof(BITMAPINFOHEADER) 以下ならばエラー
	if (dwBmihSize < sizeof(BITMAPINFOHEADER))
		return FALSE;

	// ファイルサイズから BITMAPINFOHEADER.biSize 分(sizeof DWORD)を引く
	lFileSize -= sizeof(DWORD);

	// ここで nxfile のファイルポインタは BITMAPINFOHEADER の biSize の次を示す
	// lFileSize はデータ(nxfile)のサイズ

	// BITMAPINFOHEADER の続きを読み込む
	BITMAPINFOHEADER bmihFile;
	bmihFile.biSize = dwBmihSize;
	if (nxfile.Read(&bmihFile.biWidth, sizeof(BITMAPINFOHEADER) - sizeof(DWORD)) != sizeof(BITMAPINFOHEADER) - sizeof(DWORD))
		return FALSE;
	lFileSize -= sizeof(BITMAPINFOHEADER) - sizeof(DWORD);

	// BITMAPINFOHEADER とビットデータの間に入る、
	// color mask とパレットデータのサイズを取得
	DWORD dwColorSize = CNxDIBImage::GetColorCount(reinterpret_cast<const BITMAPINFO*>(&bmihFile)) * sizeof(RGBQUAD);

	// スタック上に完全な BITMAPINFO を構築
	// BITMAPINFOHEADER をコピーして、dwColorSize のサイズだけマスクとパレットを読み込み
	LPBITMAPINFO lpbmi = static_cast<LPBITMAPINFO>(_alloca(bmihFile.biSize + dwColorSize));
	memcpy(lpbmi, &bmihFile, bmihFile.biSize);
	if (nxfile.Read(reinterpret_cast<LPBYTE>(lpbmi) + lpbmi->bmiHeader.biSize, dwColorSize) != static_cast<int>(dwColorSize))
	{	// エラー
		return NULL;
	}
	lFileSize -= dwColorSize;		// ファイルサイズから読み込んだ分を引く

	// 圧縮形式のチェック。また、圧縮されているビットマップならば、
	// CNxDIBImage へ渡す BITMAPINFO を、無圧縮へ変更(CNxDIBImage クラスが無圧縮 DIB を要求する為)
	switch (bmihFile.biCompression)
	{
	case BI_RLE4:		// RLE4 (4bpp)
	case BI_RLE8:		// RLE8 (8bpp)
		lpbmi->bmiHeader.biCompression = BI_RGB;
		break;
	case BI_BITFIELDS:
	case BI_RGB:
		break;
	default:
		_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : 対応していない圧縮形式です.\n");
		return NULL;
	}
		
	// CNxDIBImage オブジェクトの作成
	// ビットデータへのメモリも確保される
	std::auto_ptr<CNxDIBImage> pDIBImage(new CNxDIBImage);
	if (!pDIBImage->Create(lpbmi))
	{
		_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : CNxDIBImage の初期化に失敗しました.\n");
		return NULL;
	}
	
	if (bmihFile.biCompression == BI_BITFIELDS || bmihFile.biCompression == BI_RGB)
	{	// 無圧縮 (そのままメモリへ読み込み)
		DWORD dwImageSize = pDIBImage->GetImageSize();
		if (nxfile.Read(pDIBImage->GetDIBits(), dwImageSize) != static_cast<LONG>(dwImageSize))
		{	// 読み込みエラー
			_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : ファイルの読み込みに失敗しました.\n");
			return NULL;
		}
	}
	else
	{
		// RLE4/RLE8 圧縮データの展開
		if (static_cast<LONG>(bmihFile.biSizeImage) > lFileSize)
		{	// データのサイズが足りない
			_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : 圧縮された Bitmap File のサイズが小さすぎます.\n");
			return NULL;
		}

		// RLE4 なら DecodeRLE4() を, RLE8 ならば DecodeRLE8() を呼び出して画像を展開
		if (!((bmihFile.biCompression == BI_RLE8) ? decodeRLE8 : decodeRLE4)
			(static_cast<LPBYTE>(pDIBImage->GetDIBits()), nxfile, -pDIBImage->GetPitch()))
		{	// 展開エラー
			_RPTF0(_CRT_ASSERT, "CNxBMPImageLoader : 圧縮された Bitmap File の展開中に、未対応のコードが発見されました.\n");
			return NULL;
		}
	}
	return pDIBImage.release();
}

///////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxBMPImageLoader::decodeRLE4(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
// 概要: RLE4 圧縮された BMP を展開する
/////////////////////////////////////////////////////////////////////////////////////////// 

BOOL CNxBMPImageLoader::decodeRLE4(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
{
	BYTE byColor;
	BYTE byBuf[128];
	LPBYTE lpbLine = lpbBits;
	bool bEven = true;
	for (;;)
	{
		// モードと、続く1バイトの読み込み
		if (nxfile.Read(byBuf, 2) != 2)
			return FALSE;

		if (byBuf[0] != 0)
		{	// encode mode
			byColor = byBuf[1];		// 最初の1バイト目

			if (!bEven)
			{	// 奇数ドット
				*lpbLine++ = static_cast<BYTE>((*lpbLine & 0xf0) | (byColor >> 4));
				bEven = true;
				if (--byBuf[0] != 0)
				{
					for (BYTE byLoop = 0; byLoop < byBuf[0] / 2; byLoop++)
					{
						byColor = static_cast<BYTE>((byColor >> 4) | (byColor << 4));	// swap high and low
						*lpbLine++ = byColor;
					}
					if ((byBuf[0] % 2) != 0)
					{
						*lpbLine = static_cast<BYTE>(byColor << 4);
						bEven = false;
					}
				}
			}
			else
			{	// 偶数ドット
				memset(lpbLine, byColor, (byBuf[0] + 1) / 2);
				lpbLine += byBuf[0] / 2;
				bEven = ((byBuf[0] % 2) != 0) ? false : true;
			}
		}
		else
		{	// code mode
			switch (byBuf[1])	/* byBuf[1] = コード番号 */
			{
			case 0:
				// end of line
				lpbBits += lPitch;
				lpbLine = lpbBits;
				bEven = true;
				break;
			case 1:
				// end of bitmap
				return TRUE;
			case 2:
				// move position (no supported)
				return FALSE;
			default:
				BYTE byCount = byBuf[1];		// データの個数
				nxfile.Read(byBuf, ((byCount + 1) / 2 + 1) / 2 * 2);
				if (!bEven)
				{	// 奇数ドット
					int nIndex = 0;
					byColor = byBuf[nIndex++];
					// 最初の 1dot
					*lpbLine++ = static_cast<BYTE>((*lpbLine & 0xf0) | (byColor >> 4));
					bEven = true;
					if (--byCount == 0)
						break;

					// バイト境界部分
					for (BYTE byLoop = 0; byLoop < byCount / 2; byLoop++)
					{
						*lpbLine = static_cast<BYTE>(byColor << 4);
						byColor = byBuf[nIndex++];
						*lpbLine++ |= byColor >> 4;
					}

					// 最後の 1dot
					if ((byCount % 2) != 0)
					{
						*lpbLine = static_cast<BYTE>(byColor << 4);
						bEven = false;
					}
				}
				else
				{	// 偶数ドット
					memcpy(lpbLine, byBuf, (byCount + 1) / 2);
					lpbLine += byCount / 2;
					bEven = ((byCount % 2) != 0) ? false : true;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxBMPImageLoader::decodeRLE8(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
// 概要: RLE8 圧縮された BMP を展開する
/////////////////////////////////////////////////////////////////////////////////////////// 

BOOL CNxBMPImageLoader::decodeRLE8(LPBYTE lpbBits, CNxFile& nxfile, LONG lPitch)
{
	BYTE byBuf[256];
	LPBYTE lpbLine = lpbBits;
	for (;;)
	{
		if (nxfile.Read(byBuf, 2) != 2)
			return FALSE;

		if (byBuf[0] != 0)
		{	// encode mode
			memset(lpbLine, byBuf[1], byBuf[0]);
			lpbLine += byBuf[0];
		}
		else
		{	// code mode
			switch (byBuf[1])
			{
			case 0:
				// end of line
				lpbBits += lPitch;
				lpbLine = lpbBits;
				break;
			case 1:
				// end of bitmap
				return TRUE;
			case 2:
				// move position (no supported)
				return FALSE;
			default:
				nxfile.Read(lpbLine, (byBuf[1] + 1) / 2 * 2);
				lpbLine += byBuf[1];
			}
		}
	}
}
