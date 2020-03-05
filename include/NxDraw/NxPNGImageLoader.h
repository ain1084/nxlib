// NxPNGImageLoader.h: CNxPNGImageLoader クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: BMP 画像を読み込み、CNxDIBImage を返す
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxPNGImageLoader : public CNxDIBImageLoader  
{
public:
	CNxPNGImageLoader();
	virtual ~CNxPNGImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);
};
