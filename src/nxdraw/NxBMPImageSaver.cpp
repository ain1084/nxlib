// NxBMPImageSaver.cpp: CNxBMPImageSaver クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画像(CNxDIBImage) を BMP 形式で保存する、CNxDIBImageSaver 派生クラス
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <vector>
#include "NxBMPImageSaver.h"
#include "NxDIBImage.h"

//////////////////////////////////////////////////////////////////////
// public:
//	CNxBMPImageSaver::CNxBMPImageSaver(DWORD dwFlags = stripAlpha)
// 概要: CNxBMPImageSaver クラスのデフォルトコンストラクタ
// 引数: DWORD dwFlags ...フラグ
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxBMPImageSaver::CNxBMPImageSaver(DWORD dwFlags)
 : m_dwFlags(dwFlags)
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxBMPImageSaver::~CNxBMPImageSaver()
// 概要: CNxBMPImageSaver クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxBMPImageSaver::~CNxBMPImageSaver()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxBMPImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage,
//												const RECT* lpRect = NULL) const		  
// 概要: CNxDIBImage オブジェクトの内容をファイルへ保存
// 引数: CNxFile& nxfile                   ... 保存先 CNxFile オブジェクトへの参照
//       const CNxDIBImage& srcDIBImage    ... 保存される CNxDIBImage オブジェクトへの参照
//       const RECT* lpRect                ... 保存矩形(NULL ならば全体)
// 戻値: 成功ならば TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxBMPImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect) const
{
	if (!CNxDIBImageSaver::SaveDIBImage(nxfile, srcDIBImage, lpRect))
		return FALSE;

	RECT rect;
	if (lpRect == NULL)
	{
		srcDIBImage.GetRect(&rect);
	}
	else
	{
		RECT rcDIB;
		srcDIBImage.GetRect(&rcDIB);
		::IntersectRect(&rect, &rcDIB, lpRect);
		if (IsRectEmpty(&rect))
		{
			_RPTF0(_CRT_ERROR, "CNxBMPImageSaver : 保存矩形は空です.\n");
			return FALSE;
		}
	}
	// BITMAPINFOHEADER のコピーを作成
	DWORD dwInfoHeaderSize = srcDIBImage.GetInfoHeader()->biSize;
	LPBITMAPINFOHEADER lpbmiHeader = static_cast<LPBITMAPINFOHEADER>(_alloca(dwInfoHeaderSize));
	memcpy(lpbmiHeader, srcDIBImage.GetInfoHeader(), dwInfoHeaderSize);

	// 保存矩形の幅と高さ
	UINT uWidth = rect.right - rect.left;
	UINT uHeight = rect.bottom - rect.top;

	// stripAlpha がセットされているならば、32bpp を 24bpp として(アルファを除いて)保存する
	if ((m_dwFlags & stripAlpha) && lpbmiHeader->biBitCount == 32)
		lpbmiHeader->biBitCount = 24;

	// biSizeImage を設定
	LONG lRowSize = (((((lpbmiHeader->biBitCount * uWidth) + 7) / 8) + 3) / 4) * 4;
	lpbmiHeader->biSizeImage = lRowSize * uHeight;

	// BITMAPFILEHEADER を準備
	BITMAPFILEHEADER bmfh;
	bmfh.bfType = MAKEWORD('B', 'M');
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + dwInfoHeaderSize + srcDIBImage.GetColorCount() * sizeof(RGBQUAD);
	bmfh.bfSize = bmfh.bfOffBits + lpbmiHeader->biSizeImage;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;

	// BITMAPFILEHEADER を書き出す
	if (nxfile.Write(&bmfh, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER))
	{
		_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : BITMAPFILEHEADER の書き込みに失敗しました.\n");
		return FALSE;
	}
	
	// BITMAPINFOHEADER 構造体を書き出す
	if (nxfile.Write(lpbmiHeader, dwInfoHeaderSize) != static_cast<LONG>(dwInfoHeaderSize))
	{
		_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : BITMAPINFOHEADER の書き込みに失敗しました.\n");
		return FALSE;
	}

	// カラーテーブルの書き出し
	if (srcDIBImage.GetColorTable() != NULL)
	{
		UINT dwColorSize = srcDIBImage.GetColorCount() * sizeof(RGBQUAD);
		if (nxfile.Write(srcDIBImage.GetColorTable(), dwColorSize) != static_cast<LONG>(dwColorSize))
		{
			_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : カラーテーブルの書き込みに失敗しました.\n");
			return FALSE;
		}
	}

	// 1 line 分の行バッファを用意
	CNxDIBImage dibTemp;
	dibTemp.Create(rect.right - rect.left, 1, lpbmiHeader->biBitCount);
	
	// 一行ずつ書き出し
	int nLoop = rect.bottom - rect.top;
	rect.top = rect.bottom - 1;
	do
	{
		rect.bottom = rect.top + 1;
		dibTemp.Blt(0, 0, &srcDIBImage, &rect);
		LONG lWriteBytes = nxfile.Write(dibTemp.GetBits(), lRowSize);
		if (lWriteBytes == -1 || lRowSize != lWriteBytes)
		{
			_RPTF0(_CRT_ASSERT, "CNxBMPImageHandler::SaveDIBImage() : ファイルの書き込みに失敗しました.\n");
			return FALSE;
		}
		rect.top--;
	} while (--nLoop != 0);
	return TRUE;
}
