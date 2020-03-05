// NxPNGImageSaver.h: CNxPNGImageSaver クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画像(CNxDIBImage) を PNG 形式で保存する、CNxDIBImageSaver 派生クラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageSaver.h"

class CNxPNGImageSaver : public CNxDIBImageSaver  
{
public:

	enum Flags
	{
		noFilters   = 0x00000000,		// フィルタを一切使用しない
		filterNone  = 0x00000001,		// none filter
		filterSub   = 0x00000002,		// sub filter
		filterUp    = 0x00000004,		// up filter
		filterAvg   = 0x00000008,		// avg filter
		filterPaeth = 0x00000010,		// paeth filter
		allFilters  = filterNone | filterSub | filterUp | filterAvg | filterPaeth,	// 全てのフィルタを使用(default)
		stripAlpha  = 0x00000020,		// α値を保存しない
		interlace   = 0x00000040,		// インターレース形式で保存
	};

	CNxPNGImageSaver(DWORD dwFlags = allFilters);
	virtual ~CNxPNGImageSaver();
	virtual BOOL SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect = 0) const;

private:
	DWORD m_dwFlags;
	static BOOL isFullGrayscalePallete(const RGBQUAD* pRGBQUAD, UINT cEntries);
};
