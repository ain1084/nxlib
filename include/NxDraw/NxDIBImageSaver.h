// NxDIBImageSaver.h: CNxDIBImageSaver クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画像(CNxDIBImage) の保存を行う為のプロトコルクラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxFile;
class CNxDIBImage;

class __declspec(novtable) CNxDIBImageSaver
{
public:
	CNxDIBImageSaver();
	virtual ~CNxDIBImageSaver();
	virtual BOOL SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage, const RECT* lpRect = NULL) const = 0;
};
