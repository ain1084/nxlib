// NxResourceFile.cpp: CNxResourceFile クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxResourceFile.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpResType = RT_RCDATA)
// 概要: CNxResourceFile クラスのコンストラクタ
// 引数: HINSTANCE hInstance ... 読み込むリソースのモジュール(instance)ハンドル
//       LPCTSTR lpResType   ... 読み込むリソースの種類
// 戻値: なし
// 備考: hInstance と lpResType の設定のみ。リソースは読み込まれない
////////////////////////////////////////////////////////////////////////////////////////////

CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpResType)
{
	m_hInstance = hInstance;
	m_strType   = lpResType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpName, LPCTSTR lpResType = RT_RCDATA)
// 概要: CNxResourceFile クラスのコンストラクタ
// 引数: HINSTANCE hInstance ... 読み込むリソースのモジュール(instance)ハンドル
//       LPCTSTR lpName      ... 読み込むリソースの名前
//       LPCTSTR lpResType   ... 読み込むリソースの種類
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////////////////////////////////

CNxResourceFile::CNxResourceFile(HINSTANCE hInstance, LPCTSTR lpName, LPCTSTR lpResType)
{
	m_hInstance = hInstance;
	m_strType   = lpResType;

	if (!Open(lpName))
	{
		_RPTF0(_CRT_ASSERT, _T("リソースの読み込みに失敗しました.\n"));
	}
}

CNxResourceFile::~CNxResourceFile()
{
}

//////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxResourceFile::Open(LPCTSTR lpName)
// 概要: リソースを読み込み、オブジェクトを使用できる状態にする
// 引数: LPCTSTR lpName ... リソースの名前
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////

BOOL CNxResourceFile::Open(LPCTSTR lpName)
{
	HRSRC hRSRC;
	HGLOBAL hGlobal;
	LPVOID lpvData;
	DWORD dwSize;

	// リソースを検索
	hRSRC = ::FindResource(m_hInstance, lpName, m_strType);
	if (hRSRC == NULL)
		return FALSE;

	// リソースのサイズを得る
	dwSize = ::SizeofResource(m_hInstance, hRSRC);
	if (dwSize == NULL)
		return FALSE;
	
	// リソースをロード
	hGlobal = ::LoadResource(m_hInstance, hRSRC);
	if (hGlobal == NULL)
		return FALSE;

	// リソースのロック
	lpvData = ::LockResource(hGlobal);
	if (lpvData == NULL)
		return FALSE;

	MMIOINFO mi;
	memset(&mi, 0, sizeof(MMIOINFO));
	mi.fccIOProc = FOURCC_MEM;					// メモリファイル
	mi.pchBuffer = static_cast<LPSTR>(lpvData);	// データの先頭
	mi.cchBuffer = dwSize;						// データのサイズ
	mi.adwInfo[0] = 0;							// 拡張可能な最小バイト数(0 = 拡張禁止)
	
	HMMIO hMMIO = ::mmioOpen(NULL, &mi, MMIO_READ);
	if (hMMIO == NULL)
		return NULL;

	std::basic_ostringstream<TCHAR> strTemp;
	strTemp.fill('0');
	strTemp.width(8);
	strTemp << _T("HINSTANCE = 0x") << std::hex << std::right << m_hInstance << std::endl;

	strTemp << _T("TYPE = ") << std::dec;
	if (m_strType.IsNumber())
		strTemp << LOWORD(static_cast<LPCTSTR>(m_strType)) << std::endl;
	else
		strTemp << '\'' << static_cast<LPCTSTR>(m_strType) << '\'' << std::endl;

	strTemp << _T("ID = ");
	if (HIWORD(lpName) == 0)
		strTemp << LOWORD(lpName);
	else
		strTemp << '\'' << lpName << '\'';

	return Attach(hMMIO, strTemp.str().c_str());
}

CNxResourceFile::CStringId::CStringId()
{
	m_lpString = NULL;
}

CNxResourceFile::CStringId::CStringId(LPCTSTR lpString)
{
	Set(lpString);
}

CNxResourceFile::CStringId::~CStringId()
{
	Remove();
}

CNxResourceFile::CStringId::CStringId(const CStringId& stringId)
{
	m_lpString = NULL;
	*this = stringId;
}

CNxResourceFile::CStringId& CNxResourceFile::CStringId::operator=(const CStringId& stringId)
{
	if (this != &stringId)
		Set(stringId.m_lpString);
	return *this;
}

void CNxResourceFile::CStringId::Set(LPCTSTR lpString)
{
	if (HIWORD(lpString) == 0)
		m_lpString = const_cast<LPTSTR>(lpString);
	else
		m_lpString = _tcsdup(lpString);
}

void CNxResourceFile::CStringId::Remove()
{
	if (HIWORD(m_lpString) == 0)
		return;

	free(m_lpString);
	m_lpString = NULL;
}
