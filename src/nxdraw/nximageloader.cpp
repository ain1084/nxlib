// NxImageLoader.cpp: NxDrawLocal::CNxImageLoader クラスのインプリメンテーション
// Copyright(c) 2000,2001 S.Ainoguchi
// 2000/11/20 初版作成
// 2000/11/24 作り直し
//
// 概要: イメージ読み込みクラス。CreateDIBImage() によって、
//       読み込んだイメージの CNxDIBImage オブジェクトを返す
//		 CNxDraw::LoadImage() 専用
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <io.h>
#include "NxImageLoader.h"

#include "NxBMPImageLoader.h"
#include "NxPNGImageLoader.h"
#include "NxSPIImageLoader.h"
#include "NxJPEGImageLoader.h"
#include "NxMAGImageLoader.h"

using namespace NxDrawLocal;

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxImageLoader::CNxImageLoader(void)
{
}

CNxImageLoader::~CNxImageLoader(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxImageLoader::IsSupported(CNxFile& nxfile) const
// 概要: サポートしているフォーマットであるかを調べる
// 引数: CNxFile& nxfile ... 読み込み元ファイル
// 戻値: 展開可能であれば TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxImageLoader::IsSupported(CNxFile& nxfile) const
{
	return findLoader(nxfile);
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDIBImage* CNxImageLoader::CreateDIBImage(CNxFile& nxfile) const
// 概要: 画像を読み込み、CNxDIBImage オブジェクトへのポインタを返す
// 引数: CNxFile& nxfile ... 読み込み元ファイル
// 戻値: 成功ならば CNxDIBImage オブジェクトへのポインタ, 失敗ならば FALSE
//       CNxDIBImage オブジェクトが不要になったら delete で削除する事
/////////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxImageLoader::CreateDIBImage(CNxFile& nxfile) const
{
	if (!findLoader(nxfile))
	{
		_RPTF0(_CRT_ASSERT, "CNxImageLoader::CreateDIBImage() : 対応していないフォーマットです.\n");
		return NULL;
	}
	CNxDIBImage* pDIBImage = m_pLastLoader->CreateDIBImage(nxfile);
	if (pDIBImage == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxImageLoader::CreateDIBImage() : 展開に失敗しました.\n");
		return NULL;
	}
	return pDIBImage;
}

//////////////////////////////////////////////////////////////////////////////////////
// private:
//	CNxImageHandler* CNxImageLoader::FindHandler(CNxFile& nxfile) const
// 概要: 対応ハンドラを検索して、CNxImageHandler オブジェクトへのポインタを返す
// 引数: CNxFile& nxfle ... 読み込み元ファイル
// 戻値: 成功ならば true
//////////////////////////////////////////////////////////////////////////////////////

bool CNxImageLoader::findLoader(CNxFile& nxfile) const
{
	// チェック用データとして、先頭から 2048byte をバッファへ読み込む
	const LONG lCheckBufSize = 2048;
	char checkBuf[lCheckBufSize];
	
	LONG lOffset = nxfile.Seek(0, SEEK_CUR);
	LONG lLength = nxfile.Read(checkBuf, lCheckBufSize);
	nxfile.Seek(lOffset, SEEK_SET);		// file offset を戻す
	if (lLength < 0)
	{
		return false;
	}
	
	// 足りない分を 0 で填る(for susie plug-in)
	memset(checkBuf + lLength, 0, max(lCheckBufSize - lLength, 0));

	// 直前に使用した ImageLoader を優先的に...
	if (m_pLastLoader.get() != NULL)
	{
		if (m_pLastLoader->IsSupported(checkBuf, lCheckBufSize))
		{
			return true;
		}
	}

	// NxDraw がサポートする標準の ImageLoader から検索
	enum
	{
#if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
		loaderJPEG,
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
#if !defined(NXDRAW_LOADIMAGE_NO_PNG)
		loaderPNG,
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_PNG)
#if	!defined(NXDRAW_LOADIMAGE_NO_MAG)
		loaderMAG,
#endif
		loaderBMP,
		loaderLast
	};
	for (int i = 0; i < loaderLast; i++)
	{
		switch (i)
		{
#if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
		case loaderJPEG:
			m_pLastLoader.reset(new CNxJPEGImageLoader);
			break;
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_JPEG)
#if !defined(NXDRAW_LOADIMAGE_NO_PNG)
		case loaderPNG:
			m_pLastLoader.reset(new CNxPNGImageLoader);
			break;
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_PNG)
#if	!defined(NXDRAW_LOADIMAGE_NO_MAG)
		case loaderMAG:
			m_pLastLoader.reset(new CNxMAGImageLoader);
			break;
#endif
		case loaderBMP:
			m_pLastLoader.reset(new CNxBMPImageLoader);
			break;
		}
		if (m_pLastLoader->IsSupported(checkBuf, lCheckBufSize))
		{
			return true;
		}
	}

#if !defined(NXDRAW_LOADIMAGE_NO_SUSIE_SPI)
	// 指定されたディレクトリから SPI を探して順次 try
	std::basic_string<TCHAR> strSPIDirectory;
	CNxDraw::GetInstance()->GetSPIDirectory(strSPIDirectory);
	long hFile;
	struct _tfinddata_t c_file;
	if ((hFile = _tfindfirst((strSPIDirectory + _T("*.spi")).c_str(), &c_file)) != -1L)
	{
		do
		{
			HINSTANCE hInstance = ::LoadLibrary((strSPIDirectory + c_file.name).c_str());
			if (hInstance != NULL)
			{
				m_pLastLoader.reset(new CNxSPIImageLoader(hInstance));
				if (m_pLastLoader->IsSupported(checkBuf, lCheckBufSize))
				{
					_findclose(hFile);
					return true;
				}
			}
		} while (_tfindnext(hFile, &c_file) == 0);
		_findclose(hFile);
	}
#endif	// #if !defined(NXDRAW_LOADIMAGE_NO_SUSIE_SPI)
	m_pLastLoader.reset();
	return false;
}
