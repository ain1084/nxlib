// NxImageLoader.h: NxDrawLocal::CNxImageLoader クラスのインターフェイス
// Copyright(c) 2000,2001 S.Ainoguchi
// 2000/11/20 初版作成
//
// 概要: イメージ読み込みクラス。Create() によってイメージを読み込み、
//       GetHeader() と GetDIBits() で読み込んだイメージの
//       BITMAPINFO とビットデータが得られる。
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxFile;
class CNxDIBImageLoader;

#include "NxDIBImage.h"

namespace NxDrawLocal
{

class CNxImageLoader  
{
public:
	CNxImageLoader(void);
	virtual ~CNxImageLoader(void);
	BOOL IsSupported(CNxFile& nxfile) const;
	CNxDIBImage* CreateDIBImage(CNxFile& nxfile) const;

private:
	bool findLoader(CNxFile& nxfile) const;
	mutable std::auto_ptr<CNxDIBImageLoader> m_pLastLoader;

private:
	CNxImageLoader(const CNxImageLoader&);
	CNxImageLoader& operator=(const CNxImageLoader&);
};

}
