// NxSPIImageLoader.h: CNxSPIImageLoader クラスのインターフェイス
// Copyright(C) 2000 S.Ainoguchi
//
// 概要: susie plug-in 経由で画像を読み込み、CNxDIBImage を返す
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDIBImageLoader.h"

class CNxSPIImageLoader : public CNxDIBImageLoader  
{
public:
	CNxSPIImageLoader(HINSTANCE hInstance);
	virtual ~CNxSPIImageLoader();

	virtual BOOL IsSupported(LPCVOID lpvBuf, LONG lLength) const;
	virtual CNxDIBImage* CreateDIBImage(CNxFile& nxfile);

private:

	int (PASCAL *m_pfnIsSupported)(LPSTR filename, DWORD dw);

	int (PASCAL *m_pfnGetPicture)(LPSTR buf, long len, unsigned int flag, HLOCAL* pHBm, HLOCAL* hBInfo,
								  FARPROC lpProgressCallback, long lData);

private:
	HINSTANCE m_hInstance;

	CNxSPIImageLoader(const CNxSPIImageLoader&);
	CNxSPIImageLoader& operator=(const CNxSPIImageLoader&);
};
