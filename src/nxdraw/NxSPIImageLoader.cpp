// NxSPIImageLoader.cpp: CNxSPIImageLoader クラスのインプリメンテーション
// Copyright(C) 2000,2001 S.Ainoguchi
//
// 概要: susie plug-in 経由で画像を読み込み、CNxDIBImage を返す
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <vector>
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxDIBImage.h"
#include "NxSPIImageLoader.h"

namespace
{
	// ビットデータを HLOCAL で保持する CNxDIBImage 派生クラス
	class CNxLocalHandleDIBImage : public CNxDIBImage
	{
	public:
		CNxLocalHandleDIBImage(void);
		virtual ~CNxLocalHandleDIBImage(void);
		BOOL Create(HLOCAL hBMI, HLOCAL hBits);

	private:
		HLOCAL m_hBMI;
		HLOCAL m_hBits;

	private:
		CNxLocalHandleDIBImage(const CNxLocalHandleDIBImage&);
		CNxLocalHandleDIBImage& operator=(const CNxLocalHandleDIBImage&);
	};

	// コンストラクタ
	CNxLocalHandleDIBImage::CNxLocalHandleDIBImage(void)
	 : m_hBMI(NULL)		// BITMAPINFO の local memory handle
	 , m_hBits(NULL)		// ビットデータの local memory handle
	{

	}

	// デストラクタ
	CNxLocalHandleDIBImage::~CNxLocalHandleDIBImage(void)
	{
		// メモリの unlcok

		if (m_hBMI != NULL)
		{
			::LocalUnlock(m_hBMI);
			::LocalFree(m_hBMI);
		}
		if (m_hBits != NULL)
		{
			::LocalUnlock(m_hBits);
			::LocalFree(m_hBits);
		}
	}

	BOOL CNxLocalHandleDIBImage::Create(HLOCAL hBMI, HLOCAL hBits)
	{
		m_hBits = hBits;
		m_hBMI = hBMI;

		// BITMAPINFO 構造体のメモリを lock
		LPBITMAPINFO lpbmi = static_cast<LPBITMAPINFO>(::LocalLock(m_hBMI));
		_ASSERTE(lpbmi != NULL);

		// ビットデータのメモリを lock
		LPVOID lpvBits = ::LocalLock(m_hBits);
		_ASSERTE(lpvBits != NULL);
		
		return CNxDIBImage::Create(lpbmi, lpvBits);
	}
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxSPIImageLoader::CNxSPIImageLoader()
// 概要: CNxSPIImageLoader クラスのデフォルトコンストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxSPIImageLoader::CNxSPIImageLoader(HINSTANCE hInstance)
 : m_hInstance(hInstance)
 , m_pfnIsSupported(NULL)
 , m_pfnGetPicture(NULL)
{
	// GetPluginInfo のアドレスを取得
	int (PASCAL *pfnGetPluginInfo)(int infono, LPSTR buf, int buflen);
	reinterpret_cast<FARPROC&>(pfnGetPluginInfo) = ::GetProcAddress(m_hInstance, "GetPluginInfo");
	if (pfnGetPluginInfo == NULL)
	{
		return;
	}
	// API バージョンチェック
	char szAPIVersion[5];
	if ((pfnGetPluginInfo)(0, szAPIVersion, 5) == 0 || strcmp(szAPIVersion, "00IN") != 0)
	{
		return;
	}
	// IsSupported と GetPicture のアドレスを取得,保存
	reinterpret_cast<FARPROC&>(m_pfnIsSupported) = ::GetProcAddress(m_hInstance, "IsSupported");
	reinterpret_cast<FARPROC&>(m_pfnGetPicture) = ::GetProcAddress(m_hInstance, "GetPicture");
}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxSPIImageLoader::~CNxSPIImageLoader()
// 概要: CNxSPIImageLoader クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxSPIImageLoader::~CNxSPIImageLoader()
{
	::FreeLibrary(m_hInstance);
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSPIImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// 概要: 展開可能なデータ形式であるかを調べる
// 引数: LPCVOID lpvBuf ... データの最初から 2048byte を読み込んだバッファへのポインタ
//       LONG lLength   ... データのサイズ(通常は 2048)
// 戻値: 展開可能であれば TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSPIImageLoader::IsSupported(LPCVOID pvData, LONG /*lLength*/) const
{
	if (m_pfnIsSupported == NULL)
		return FALSE;		// IsSupported 関数が存在しない
	
	return (m_pfnIsSupported)(NULL, reinterpret_cast<DWORD>(pvData)) != 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxSPIImageLoader::CreateDIBImage(CNxFile& nxfile)
// 概要: 画像を展開して CNxDIBImage オブジェクトを返す
// 引数: CNxFile& nxfile ... CNxFile オブジェクトへの参照
// 戻値: 成功ならば、作成した CNxDIBImage オブジェクトへのポインタ。失敗ならば NULL
///////////////////////////////////////////////////////////////////////////////////////

CNxDIBImage* CNxSPIImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	if (m_pfnGetPicture == NULL)
		return NULL;
	
	// 全体をメモリへ読み込む
	LONG lFileSize = nxfile.GetSize();
	std::vector<char> fileData(lFileSize);
	if (nxfile.Read(&fileData[0], lFileSize) != lFileSize)
		return NULL;

	// サポートされているか?
	if (!IsSupported(&fileData[0], lFileSize))
		return NULL;
	
	// 画像の展開
	HLOCAL hBMI, hBits;
	if ((m_pfnGetPicture)(&fileData[0], lFileSize, 1 /* 0:ファイル / 1:メモリから */, &hBMI, &hBits, NULL, 0) != 0)
		return NULL;
	
	// 展開成功
	// CNxDIBImage オブジェクトを作成して返す
	std::auto_ptr<CNxLocalHandleDIBImage> pDIBImage(new CNxLocalHandleDIBImage);
	if (!pDIBImage->Create(hBMI, hBits))
	{
		return NULL;
	}
	return pDIBImage.release();
}
