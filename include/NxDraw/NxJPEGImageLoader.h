// NxJPEGImageLoader.h: CNxJPEGImageLoader クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: JPEG 画像の読み込み CNxDIBImage を返す、CNxDIBImageLoader 派生クラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxJPEGImageLoader : public CNxDIBImageLoader  
{
public:
	CNxJPEGImageLoader();
	virtual ~CNxJPEGImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);
};
