// NxMAGImageLoader.h: CNxMAGImageLoader クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: MAG 画像の読み込みを行う、CNxDIBImageLoader 派生クラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxMAGImageLoader : public CNxDIBImageLoader  
{
public:
	CNxMAGImageLoader();
	virtual ~CNxMAGImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);
};
