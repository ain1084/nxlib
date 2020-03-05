// NxPNGImageSaver.cpp: CNxPNGImageSaver クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画像(CNxDIBImage) を BMP 形式で保存する、CNxDIBImageSaver 派生クラス
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <vector>
#include "NxPNGImageSaver.h"
#include "NxPNGErrorHandler.h"
#include "NxDIBImage.h"

using namespace NxDrawLocal::NxPNGErrorHandler;

namespace
{
	void WriteNxFile(png_structp png_ptr, png_bytep data, png_size_t length);
	void FlushNxFile(png_structp png_ptr);
} // namespace

//////////////////////////////////////////////////////////////////////
// public:
//	CNxPNGImageSaver::CNxPNGImageSaver(DWORD dwFlags)
// 概要: CNxPNGImageSaver クラスのデフォルトコンストラクタ
// 引数: DWORD dwFlags ... フラグ
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxPNGImageSaver::CNxPNGImageSaver(DWORD dwFlags)
{
	m_dwFlags = dwFlags;
}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxPNGImageSaver::~CNxPNGImageSaver()
// 概要: CNxPNGImageSaver クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxPNGImageSaver::~CNxPNGImageSaver()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxPNGImageSaver::isFullGrayscalePallete(const RGBQUAD* pRGBQUAD, const BYTE* pbAlphaTable, UINT cEntries)
// 概要: パレット内容を調査して完全なグレイスケールならば、TRUE を返す
// 引数: const RGBQUAD* pRGBQUAD  ... カラーテーブルの最初のエントリへのポインタ
//       const BYTE* pbAlphaTable ... アルファ値テーブルの最初のエントリへのポインタ
//       UINT cEntries            ... パレット数(1 ～ 256)
// 戻値: 完全なグレイスケールパレットならば TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxPNGImageSaver::isFullGrayscalePallete(const RGBQUAD* pRGBQUAD, UINT cEntries)
{
	for (UINT u = 0; u < cEntries; u++, pRGBQUAD++)
	{
		if (pRGBQUAD->rgbRed != pRGBQUAD->rgbGreen || pRGBQUAD->rgbGreen != pRGBQUAD->rgbBlue)
			return FALSE;		// R = G = B ではない

		if (pRGBQUAD->rgbRed != (255 * u / (cEntries - 1)))
			return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxPNGImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage,
//												const RECT* lpRect) const		  
// 概要: CNxDIBImage オブジェクトの内容をファイルへ保存
// 引数: CNxFile& nxfile                   ... 保存先 CNxFile オブジェクトへの参照
//       const CNxDIBImage& srcDIBImage    ... 保存される CNxDIBImage オブジェクトへの参照
//       const RECT* lpRect                ... 保存矩形
// 戻値: 成功ならば TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxPNGImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect) const
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
			_RPTF0(_CRT_ERROR, "CNxPNGImageSaver : 保存矩形は空です.\n");
			return FALSE;
		}
	}

	// 保存される画像形式のピクセルビット数
	UINT uBitCount = srcDIBImage.GetBitCount();
	
	// libpng の初期化
	png_structp png_ptr;
	png_infop info_ptr;

	// png_write_struct を作成
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, &nxfile, UserError, UserWarning);
	if (png_ptr == NULL)
		return FALSE;

	// PNG データ書き込み関数を設定
	png_set_write_fn(png_ptr, &nxfile, WriteNxFile, FlushNxFile);

	// info_struct を作成
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{	// エラー発生 (png_write_struct を破棄)
		png_destroy_write_struct(&png_ptr, NULL);
		return FALSE;
	}

	// IHDR chunk 書き込み準備
	int color_type = PNG_COLOR_TYPE_RGB;
	int bit_depth = 8;
	switch (uBitCount)
	{
	case 32:
		png_set_bgr(png_ptr);
		if (m_dwFlags & stripAlpha)
		{	// アルファを取り除く
			color_type = PNG_COLOR_TYPE_RGB;
			uBitCount = 24;
		}
		else
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		break;
	case 24:
		color_type = PNG_COLOR_TYPE_RGB;
		png_set_bgr(png_ptr);
		break;
	case 16:
		// 16BPP は 24BPP へ変換して保存
		uBitCount = 24;
		color_type = PNG_COLOR_TYPE_RGB;
		png_set_bgr(png_ptr);
		break;	
	case 8:
	case 4:
	case 1:
		color_type = PNG_COLOR_TYPE_PALETTE;
		bit_depth = uBitCount;
		break;
	}

	// パレットの設定

	std::vector<png_color> palette;
	std::vector<png_byte> trans;
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		int i;
		int num_palette = srcDIBImage.GetColorCount();
		std::vector<NxColor> nxcolors(num_palette);
		const RGBQUAD* pRGBQUAD = srcDIBImage.GetColorTable();

		if (isFullGrayscalePallete(pRGBQUAD, num_palette))
		{	// 完全なグレイスケール
			color_type = PNG_COLOR_TYPE_GRAY;
		}
		else
		{
			palette.resize(num_palette);
			for (i = 0; i < num_palette; i++)
			{
				png_color pal;
				pal.red = pRGBQUAD->rgbRed;
				pal.green = pRGBQUAD->rgbGreen;
				pal.blue = pRGBQUAD->rgbBlue;
				palette[i] = pal;
				pRGBQUAD++;
			}
			png_set_PLTE(png_ptr, info_ptr, &palette[0], num_palette);
		}
	}

	// IHDR チャンクを設定
	png_set_IHDR(png_ptr, info_ptr, rect.right - rect.left, rect.bottom - rect.top, bit_depth, color_type,
				 (m_dwFlags & interlace) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// info 書き出し
	png_write_info(png_ptr, info_ptr);


	// フィルタを設定
	int filters = PNG_NO_FILTERS;
	if (m_dwFlags & filterNone)
		filters |= PNG_FILTER_NONE;
	if (m_dwFlags & filterSub)
		filters |= PNG_FILTER_SUB;
	if (m_dwFlags & filterUp)
		filters |= PNG_FILTER_UP;
	if (m_dwFlags & filterAvg)
		filters |= PNG_FILTER_AVG;
	if (m_dwFlags & filterPaeth)
		filters |= PNG_FILTER_PAETH;
	png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, filters); 
	
	// zlib 圧縮レベル設定(効果不明)
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	// 1 line 分の行バッファを確保
	CNxDIBImage dibTemp;
	dibTemp.Create(rect.right - rect.left, 1, uBitCount);
	
	// 一行ずつ書き出し
	int nHeight = rect.bottom - rect.top;
	do
	{
		rect.bottom = rect.top + 1;
		dibTemp.Blt(0, 0, &srcDIBImage, &rect);
		png_write_row(png_ptr, static_cast<LPBYTE>(dibTemp.GetBits()));
		rect.top++;
	} while (--nHeight != 0);

	// 後始末
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return TRUE;
}

namespace
{

void WriteNxFile(png_structp png_ptr, png_bytep data, png_size_t length)
{
	CNxFile* pFile = static_cast<CNxFile*>(png_ptr->io_ptr);
	pFile->Write(data, length);
}

void FlushNxFile(png_structp png_ptr)
{
	CNxFile* pFile = static_cast<CNxFile*>(png_ptr->io_ptr);
	pFile->Flush();
}

} // namespace
