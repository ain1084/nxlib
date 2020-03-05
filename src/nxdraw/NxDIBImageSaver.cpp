// NxDIBImageSaver.cpp: CNxDIBImageSaver クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 画像(CNxDIBImage) の保存を行う為のプロトコルクラス
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxDIBImageSaver.h"
#include "NxDIBImage.h"

//////////////////////////////////////////////////////////////////////
// public:
//	CNxDIBImageSaver::CNxDIBImageSaver()
// 概要: CNxDIBImageSaver クラスのデフォルトコンストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxDIBImageSaver::CNxDIBImageSaver()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImageSaver::~CNxDIBImageSaver()
// 概要: CNxDIBImageSaver クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxDIBImageSaver::~CNxDIBImageSaver()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxImageHandler::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& srcDIBImage,
//											   const RECT* lpRect = NULL) const = 0
// 概要: CNxDIBImage オブジェクトの内容をファイルへ保存
// 引数: CNxFile& nxfile                   ... 保存先 CNxFile オブジェクトへの参照
//       const CNxDIBImage& srcDIBImage    ... 保存される CNxDIBImage オブジェクトへの参照
//       const RECT* lpRect                ... 保存矩形(NULL ならば全体)
// 戻値: 成功ならば TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImageSaver::SaveDIBImage(CNxFile& nxfile, const CNxDIBImage& /*srcDIBImage*/, const RECT* /*lpRect*/) const
{
	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::SaveBitmapFile() : ファイルは開かれていません.\n");
		return FALSE;
	}
	return TRUE;
}
