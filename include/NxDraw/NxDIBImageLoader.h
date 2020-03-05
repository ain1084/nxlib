// NxDIBImageLoader.h: CNxDIBImageLoader クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画像の読み込みを行う為のプロトコルクラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxFile;
class CNxDIBImage;

class __declspec(novtable) CNxDIBImageLoader
{
public:
	CNxDIBImageLoader();
	virtual ~CNxDIBImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const = 0;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile) = 0;
};
