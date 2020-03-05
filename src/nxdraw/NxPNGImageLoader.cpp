// NxPNGImageLoader.cpp: CNxPNGImageLoader クラスのインプリメンテーション
// Copyright(c) 2000,2001 S.Ainoguchi
//
// 概要: PNG 画像を読み込み、CNxDIBImage を返す
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <vector>
#include "NxPNGImageLoader.h"
#include "NxPNGErrorHandler.h"
#include "NxDIBImage.h"
#include "libpng/png.h"

using namespace NxDrawLocal::NxPNGErrorHandler;

namespace
{
	void ReadNxFile(png_structp png_ptr, png_bytep buf, png_size_t length)
	{
		CNxFile* pFile = static_cast<CNxFile*>(png_ptr->io_ptr);
		pFile->Read(buf, length);
	}
} // namespace

//////////////////////////////////////////////////////////////////////
// public:
//	CNxPNGImageLoader::CNxPNGImageLoader()
// 概要: CNxPNGImageLoader クラスのデフォルトコンストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxPNGImageLoader::CNxPNGImageLoader()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxPNGImageLoader::~CNxPNGImageLoader()
// 概要: CNxPNGImageLoader クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxPNGImageLoader::~CNxPNGImageLoader()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxPNGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// 概要: 展開可能なデータ形式であるかを調べる
// 引数: LPCVOID lpvBuf ... データの最初から 2048byte を読み込んだバッファへのポインタ
//       LONG lLength   ... データのサイズ(通常は 2048)
// 戻値: 展開可能であれば TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxPNGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
{
	if (png_check_sig(reinterpret_cast<LPBYTE>(const_cast<LPVOID>(lpvBuf)), lLength) != 0)
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxPNGImageLoader::CreateDIBImage(CNxFile& nxfile)
// 概要: 画像を展開して CNxDIBImage オブジェクトを返す
// 引数: CNxFile& nxfile ... CNxFile オブジェクトへの参照
// 戻値: 成功ならば、作成した CNxDIBImage オブジェクトへのポインタ。失敗ならば NULL
///////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4611)		// setjmp に関する警告を止める

CNxDIBImage* CNxPNGImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	png_colorp palette;
	LPBYTE *ppRow;
	int num_palette;
	int bit_depth;
	int color_type;
	int interlace_type;
	int compression_type;
	int filter_type;

	UINT uBitCount;
	LPBITMAPINFO lpbmi;
	ppRow = NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, &nxfile, UserError, UserWarning);
	if (png_ptr == NULL)
	{
		_RPTF0(_CRT_ERROR, "CNxPNGImageLoader::Load() : png_create_read_struct() が失敗しました.\n");
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		_RPTF0(_CRT_ERROR, "CNxPNGImageLoader::Load() : png_create_info_struct() が失敗しました.\n");
		return NULL;
	}

	png_set_read_fn(png_ptr, &nxfile, ReadNxFile);

	if (setjmp(png_jmpbuf(png_ptr)) != 0)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		delete []ppRow;
		return NULL;
	}
	
	png_read_info(png_ptr, info_ptr);
	if (!png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type))
	{
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		_RPTF0(_CRT_ERROR, "CNxPNGImageLoader::Load() : png_get_IHDR() が失敗しました.\n");
		return NULL;
	}

	if (bit_depth == 16)
	{	// ビット深度 16 は無意味なので 8 ビット深度へ
		png_set_strip_16(png_ptr);
		bit_depth = 8;
	}
	else if (bit_depth == 2)
	{	// 2bpp BITMAP は作れないので 8bpp へ変換して対応
		png_set_packing(png_ptr);
		bit_depth = 8;
	}

	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{	// index color
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{	// もし透過色キー付きならば RGBA へ変換
			png_set_palette_to_rgb(png_ptr);
			png_set_tRNS_to_alpha(png_ptr);
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		}
		else
		{	// パレットを取得
			png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
			uBitCount = bit_depth;
		}
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY)
	{	// グレイスケール(ビット深度 4, 8)
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{	// 透過色付きグレイスケールならば RGBA へ
			png_set_gray_to_rgb(png_ptr);
			png_set_tRNS_to_alpha(png_ptr);
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		}
		else
		{	// 透過色無しのグレイスケール
			uBitCount = bit_depth;
			num_palette = 1 << bit_depth;
		}
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{	// グレイスケール(ビット深度 8) + alpha
		// RGBA へ変換
		png_set_gray_to_rgb(png_ptr);
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	}
	if (color_type == PNG_COLOR_TYPE_RGB)
	{	// RGB カラー	
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{	// 透過色があれば、alpha 要素へ変換する
			png_set_tRNS_to_alpha(png_ptr);
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;	// RGBA として偽装する
		}
	}
	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{	// RGBA カラー
		uBitCount = 32;
		num_palette = 0;
		png_set_bgr(png_ptr);
	}
	else if (color_type == PNG_COLOR_TYPE_RGB)
	{	// RGB カラー
		uBitCount = 24;
		num_palette = 0;
		png_set_bgr(png_ptr);
	}

	lpbmi = static_cast<LPBITMAPINFO>(_alloca(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * num_palette));
	lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbmi->bmiHeader.biWidth = (LONG)width;
	lpbmi->bmiHeader.biHeight = (LONG)height;
	lpbmi->bmiHeader.biPlanes = 1;
	lpbmi->bmiHeader.biBitCount = (WORD)uBitCount;
	lpbmi->bmiHeader.biCompression = BI_RGB;
	lpbmi->bmiHeader.biSizeImage = 0;
	lpbmi->bmiHeader.biXPelsPerMeter = 0;
	lpbmi->bmiHeader.biYPelsPerMeter = 0;
	lpbmi->bmiHeader.biClrUsed = num_palette;
	lpbmi->bmiHeader.biClrImportant = 0;
	
	// RGBQUAD の設定
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		for (int i = 0; i < num_palette; i++)
		{
			lpbmi->bmiColors[i].rgbRed = palette[i].red;
			lpbmi->bmiColors[i].rgbGreen = palette[i].green;
			lpbmi->bmiColors[i].rgbBlue = palette[i].blue;
			lpbmi->bmiColors[i].rgbReserved = 0;
		}
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY)
	{
		for (int i = 0; i < num_palette; i++)
		{
			BYTE byValue = static_cast<BYTE>(255 * i / (num_palette - 1));		// 最後のパレット番号で RGB(255,255,255)
			lpbmi->bmiColors[i].rgbBlue = byValue;
			lpbmi->bmiColors[i].rgbGreen = byValue;
			lpbmi->bmiColors[i].rgbRed = byValue;
			lpbmi->bmiColors[i].rgbReserved = 0;
		}
	}

	// CNxDIBImage の作成
	std::auto_ptr<CNxDIBImage> pDIBImage(new CNxDIBImage);
	if (!pDIBImage->Create(lpbmi))
	{
		return NULL;
	}

	// ビットマップデータの取得
	ppRow = new LPBYTE[height];
	LPBYTE lpbBits = static_cast<LPBYTE>(pDIBImage->GetBits());
	for (unsigned int row = 0; row < height; row++, lpbBits += pDIBImage->GetPitch())
		ppRow[row] = lpbBits;

	png_read_image(png_ptr, ppRow);
	delete []ppRow;

	// png 後始末
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return pDIBImage.release();
}
#pragma warning (default : 4611)
