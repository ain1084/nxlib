// NxBMPImageSaver.h: CNxBMPImageSaver クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画像(CNxDIBImage) を BMP 形式で保存する、CNxDIBImageSaver 派生クラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageSaver.h"

class CNxBMPImageSaver : public CNxDIBImageSaver  
{
public:
	enum Flags
	{
		stripAlpha = 0x00000001,	// α値を取り除いて保存
	};

	CNxBMPImageSaver(DWORD dwFlags = stripAlpha);
	virtual ~CNxBMPImageSaver();
	virtual BOOL SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect = 0) const;

private:
	DWORD m_dwFlags;
};
