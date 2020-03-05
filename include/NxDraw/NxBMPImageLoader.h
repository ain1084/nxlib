// NxBMPImageLoader.h: CNxBMPImageLoader クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: BMP 画像を読み込み、CNxDIBImage を返す
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxBMPImageLoader : public CNxDIBImageLoader  
{
public:
	CNxBMPImageLoader();
	virtual ~CNxBMPImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);

private:
	static BOOL decodeRLE4(LPBYTE lpbBits, CNxFile& nxfile, LONG lNextLine);
	static BOOL decodeRLE8(LPBYTE lpbBits, CNxFile& nxfile, LONG lNextLine);
};
