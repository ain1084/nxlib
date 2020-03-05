// NxSurface.cpp: CNxSurface クラスのインプリメンテーション
// Copyright(c) 2000,2001 S.Ainoguchi
//
// 概要: サーフェス管理クラス
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <math.h>
#include <algorithm>
#include "NxSurface.h"
#include "NxCustomDraw8.h"
#include "NxCustomDraw32.h"
#include "NxDynamicDraw8.h"
#include "NxDynamicDraw32.h"
#include "NxDrawLocal.h"
#include "NxFont.h"
#include "NxBMPImageSaver.h"

using namespace NxDrawLocal;

namespace
{	// 全ての CNxSurface クラスのインスタンスは、以下の何れかを使用
	const CNxCustomDraw8  g_customDraw8;	// for 8bpp
	const CNxCustomDraw32 g_customDraw32;	// for 32bpp
}


/////////////////////////////////////////////////////////////////////////
// public:
//	CNxSurface::CNxSurface(UINT uBitCount = 32)
// 概要: CNxSurface クラスのコンストラクタ
// 引数: UINT uBitCount ... ビット深度(32 又は 8)
// 戻値: ---
/////////////////////////////////////////////////////////////////////////

CNxSurface::CNxSurface(UINT uBitCount)
 : m_customDraw((uBitCount == 8) ? static_cast<const CNxCustomDraw&>(g_customDraw8) : static_cast<const CNxCustomDraw&>(g_customDraw32))
 , m_lpbmi(reinterpret_cast<LPBITMAPINFO>(uBitCount))	// Create() されるまでの間、ピクセルビット数を保持する
 , m_hBitmap(NULL)				// DIBSection のハンドル(HBITMAP 型)
 , m_hdcCurrent(NULL)			// 現在取得されている HDC
 , m_udcRefCount(0)				// 現在取得されている HDC の参照カウント
 , m_pFont(NULL)				// CNxFont へのポインタ
 , m_bTextSmoothing(FALSE)		// テキストのスムージング(off)
{
	switch (uBitCount)
	{
	case 8:
		m_pDynamicDraw = new CNxDynamicDraw8;
		break;
	default:
		if (uBitCount != 32)
		{
			_RPT1(_CRT_WARN, "CNxSurface::CNxSurface() : サポートしていないビット深度(%d)が指定されました.32bpp と見なされます.\n", uBitCount);
			uBitCount = 32;
		}
	case 32:
		m_pDynamicDraw = new CNxDynamicDraw32;
	}
	m_ptOrg.x = 0;				// 原点座標 X
	m_ptOrg.y = 0;				// 原点座標 Y
	::SetRectEmpty(&m_rcClip);	// クリップ矩形
}

/////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxSurface::~CNxSurface()
// 概要: CNxSurface クラスのデストラクタ
// 引数: ---
// 戻値: ---
/////////////////////////////////////////////////////////////////////////

CNxSurface::~CNxSurface()
{
	if (m_hdcCurrent != NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::~CNxSurface() : デバイスコンテキストが使用されています.\n");
		::DeleteDC(m_hdcCurrent);
	}

	// 動的描画コードクラス削除
	delete m_pDynamicDraw;
	
	// DIBSection 削除
	if (m_hBitmap != NULL)
	{
		if (!::DeleteObject(m_hBitmap))
		{
			_RPTF0(_CRT_ASSERT, "CNxSurface::~CNxSurface() : DIBSection の削除に失敗しました.\n");
		}
	}

	if (HIWORD(m_lpbmi) != 0)
	{	// m_lpbmi は Create() されるまでの間、下位word にビット深度を保持する
		// BITMAPINFO 構造体のメモリを開放
		free(m_lpbmi);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSurface::Create(int nWidth, int nHeight)
// 概要: サーフェスを作成
// 引数: int nWidth     ... 幅
//       int nHeight    ... 高さ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Create(int nWidth, int nHeight)
{
	_ASSERTE(m_hBitmap == NULL);
	_ASSERTE(nWidth >= 0);

	// BITMAPINFO 構造体のメモリを確保
	// コンストラクタによって m_lpbmi の下位 word へはビット深度が設定されている
	// 8bpp ならばカラーテーブル 256個分も確保
	UINT uBitCount = reinterpret_cast<UINT>(m_lpbmi);
	m_lpbmi = static_cast<LPBITMAPINFO>(calloc(sizeof(BITMAPINFOHEADER) + ((uBitCount == 32) ? 0 : sizeof(RGBQUAD) * 256), sizeof(BYTE)));

	// BITMAPINFOHEADER 設定
	m_lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_lpbmi->bmiHeader.biWidth = nWidth;
	m_lpbmi->bmiHeader.biHeight = nHeight;
	m_lpbmi->bmiHeader.biPlanes = 1;
	m_lpbmi->bmiHeader.biBitCount = static_cast<WORD>(uBitCount);
	m_lpbmi->bmiHeader.biXPelsPerMeter = 0;
	m_lpbmi->bmiHeader.biYPelsPerMeter = 0;
	m_lpbmi->bmiHeader.biClrImportant = 0;
	m_lpbmi->bmiHeader.biCompression = BI_RGB;
	m_lpbmi->bmiHeader.biClrUsed = 0;
	m_lpbmi->bmiHeader.biSizeImage = 0;

	if (uBitCount == 8)
	{	// 8bpp の場合はパレットを初期化
		for (UINT u = 0; u < 256; u++)
		{	// grayscale
			m_lpbmi->bmiColors[u].rgbBlue = static_cast<BYTE>(u);
			m_lpbmi->bmiColors[u].rgbGreen = static_cast<BYTE>(u);
			m_lpbmi->bmiColors[u].rgbRed = static_cast<BYTE>(u);
			m_lpbmi->bmiColors[u].rgbReserved = 0;
		}
	}

	// DIBSection 作成
	LPVOID lpvBits;
	m_hBitmap = ::CreateDIBSection(NULL, m_lpbmi, DIB_RGB_COLORS, &lpvBits, NULL, NULL);
	if (m_hBitmap == NULL)
	{
		free(m_lpbmi);
		m_lpbmi = NULL;
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : CreateDIBSection() 関数が失敗しました.\n");
		return FALSE;
	}

	// CNxDIBImage を作成
	m_dibImage.Create(m_lpbmi, lpvBits);

	// クリッピング矩形をサーフェス全体に設定
	SetClipRect(NULL);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Create(const BITMAPINFO* lpbmi, LPCVOID lpvBits = NULL)
// 概要: DIB と同じサイズのサーフェスを作り、内容をコピーする
// 引数: const BITMAPINFO* lpbmi ... DIB の情報を示す BITMAPINFO 構造体へのポインタ
//       LPCVOID lpvBits         ... DIB のビットデータへのポインタ
// 戻値: 成功なら TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Create(const BITMAPINFO* lpbmi, LPCVOID lpvBits)
{
	_ASSERTE(m_hBitmap == NULL);
	_ASSERTE(lpbmi != NULL);
	
	// DIB の幅と高さでサーフェスを作成
	if (!Create(lpbmi->bmiHeader.biWidth, lpbmi->bmiHeader.biHeight))
		return FALSE;

	// 作成したサーフェスへ DIB の内容をコピー
	return SetDIBits(0, 0, lpbmi, lpvBits);
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Create(const CNxDIBImage* pDIBImage)
// 概要: CNxDIBImage と同じサイズのサーフェスを作り、内容をコピーする
// 引数: const CNxDIBImage* pDIBImage ... CNxDIBImage オブジェクトへのポインタ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Create(const CNxDIBImage* pDIBImage)
{
	_ASSERT(m_hBitmap == NULL);
	
	// DIB の幅と高さでサーフェスを作成
	if (!Create(pDIBImage->GetInfoHeader()->biWidth, pDIBImage->GetInfoHeader()->biHeight))
		return FALSE;

	// 作成したサーフェスへ DIB の内容をコピー
	if (!SetDIBits(0, 0, pDIBImage))
	{
		_RPT0(_CRT_ASSERT, "CNxSurface::Create() : この形式の DIB には対応していません.\n");
		return FALSE;
	}
	// カラーテーブルの設定
	SetColorTable(pDIBImage);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Create(CNxFile& nxfile)
// 概要: イメージを読込み、イメージと同じサイズのサーフェスを作る
// 引数: CNxFile&  nxfile ... 読み込み元ファイル
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////
 
BOOL CNxSurface::Create(CNxFile& nxfile)
{
	_ASSERTE(m_hBitmap == NULL);

	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : オープン済みでない CNxFile オブジェクトが渡されました.\n");
		return FALSE;
	}
	
	// 画像の展開
	std::auto_ptr<CNxDIBImage> pDIBImage(CNxDraw::GetInstance()->LoadImage(nxfile));
	if (pDIBImage.get() == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : 画像の展開に失敗しました.\n");
		return FALSE;
	}

	// 画像を設定
	if (!Create(pDIBImage.get()))
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : サーフェスの作成又は画像の変換に失敗しました.\n");
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	HDC CNxSurface::GetDC()
// 概要: サーフェスへ描画する為のデバイスコンテキストを取得する
// 引数: なし
// 戻値: デバイスコンテキスト(NULL ならばエラー)
///////////////////////////////////////////////////////////////////////////////

HDC CNxSurface::GetDC()
{
	_ASSERTE(m_hBitmap != NULL);
	
	if (m_udcRefCount != 0)
	{	// 取得済みならば参照カウンタを増やす
		m_udcRefCount++;
		return m_hdcCurrent;
	}

	// メモリデバイスコンテキストを作成
	HDC hDC = ::CreateCompatibleDC(NULL);
	if (hDC == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::GetDC() : デバイスコンテキストの作成に失敗しました.\n");
		return NULL;
	}
	
	// hBitmap を選択
	if (::SelectObject(hDC, m_hBitmap) == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::GetDC() : ビットマップの SelectObject() に失敗しました.\n");
		::DeleteDC(hDC);
		return NULL;
	}

	m_udcRefCount = 1;		// 参照カウントは 1
	m_hdcCurrent = hDC;
	return hDC;
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::ReleaseDC()
// 概要: CNxSurface::GetDC() で得られたデバイスコンテキストを開放
// 引数: なし
// 戻値: デバイスコンテキストの参照カウント数
/////////////////////////////////////////////////////////////////////////////////

UINT CNxSurface::ReleaseDC()
{
	_ASSERTE(m_hBitmap != NULL);

	if (m_hdcCurrent == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::ReleaseDC() : デバイスコンテキストは取得されていません.\n");
		return 0;			// 取得していない
	}
	if (--m_udcRefCount == 0)
	{	// 参照カウンタが 0 ならば DC を削除
		::DeleteDC(m_hdcCurrent);
		m_hdcCurrent = NULL;
	}
	return m_udcRefCount;
}

//////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::TileBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//							 const RECT* lpSrcRect, int nSrcXOrg, int nSrcYOrg,
//							 NxBlt* pNxBlt = NULL)
// 概要: 転送元をタイル状に敷き詰める Blt
// 引数: const RECT* lpDestRect        ... 転送先矩形
//       const CNxSurface* pSrcSurface ... 転送元サーフェス
//       const RECT* lpSrcRect         ... 転送元矩形
//       int nSrcXOrg                  ... 転送元 X 座標原点
//       int nSrcYOrg                  ... 転送元 Y 座標原点
//       const NxBlt* pNxBlt       ... 転送方法を指定
// 戻値: 成功ならば TRUE
// 備考: 転送元反転には未対応
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::TileBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
						 const RECT* lpSrcRect, int nSrcXOrg, int nSrcYOrg, NxBlt* pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	if (pSrcSurface == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::TileBlt() : 転送元は省略できません.\n");
		return FALSE;
	}

	RECT rcDest;
	SetAbbreviateRect(&rcDest, lpDestRect);		// NULL 対策

	RECT rcSrc;
	pSrcSurface->SetAbbreviateRect(&rcSrc, lpSrcRect);	// NULL 対策

	// 転送元の左上が負の値ならば、原点へ加算
	if (rcSrc.left < 0)
	{
		nSrcXOrg += rcSrc.left;
		rcSrc.right -= rcSrc.left;
		rcSrc.left = 0;
	}
	if (rcSrc.top < 0)
	{
		nSrcYOrg += rcSrc.top;
		rcSrc.bottom -= rcSrc.top;
		rcSrc.top = 0;
	}

	// 転送元の幅と高さ
	int nSrcWidth = rcSrc.right - rcSrc.left;
	int nSrcHeight = rcSrc.bottom - rcSrc.top;

	if ((nSrcWidth - 1 | nSrcHeight - 1) + 1 == 0)
		return TRUE;		// 転送矩形なし。無限ループと divide by zero 回避の為 return する

	// 転送元と転送先矩形のサイズが同じ、原点座標が X,Y 共にゼロならば、普通に Blt
	if ((((rcDest.right - rcDest.left) - nSrcWidth | (rcDest.bottom - rcDest.top) - nSrcHeight) | nSrcXOrg | nSrcYOrg) == 0)
		return Blt(&rcDest, pSrcSurface, &rcSrc, pNxBlt);

	// 原点が転送元の範囲をはみ出さない様に修正
	nSrcXOrg %= nSrcWidth;
	nSrcYOrg %= nSrcHeight;

	// 原点を調整
	nSrcXOrg += (nSrcXOrg < 0) ? rcSrc.right : rcSrc.left;
	nSrcYOrg += (nSrcYOrg < 0) ? rcSrc.bottom : rcSrc.top;

	// 明らかに描画されない部分を取り除き、矩形を縮小
	if (rcDest.left + (rcSrc.right - nSrcXOrg) + m_ptOrg.x < m_rcClip.left)
	{
		rcDest.left += (rcSrc.right - nSrcXOrg);
		nSrcXOrg = rcSrc.left;
		int nDestLeftMargin = m_rcClip.left - (rcDest.left + m_ptOrg.x);
		rcDest.left += nDestLeftMargin - (nDestLeftMargin % nSrcWidth);
	}
	if (rcDest.right + m_ptOrg.x > m_rcClip.right)
		rcDest.right = m_rcClip.right - m_ptOrg.x;

	if (rcDest.top + (rcSrc.bottom - nSrcYOrg) + m_ptOrg.y < m_rcClip.top)
	{
		rcDest.top += (rcSrc.bottom - nSrcYOrg);
		nSrcYOrg = rcSrc.top;
		int nDestTopMargin = m_rcClip.top - (rcDest.top + m_ptOrg.y);
		rcDest.top += nDestTopMargin - (nDestTopMargin % nSrcWidth);
	}
	if (rcDest.bottom + m_ptOrg.y > m_rcClip.bottom)
		rcDest.bottom = m_rcClip.bottom - m_ptOrg.y;

	POINT ptSrc;
	POINT ptDest;

	for (ptDest.y = rcDest.top, ptSrc.y = nSrcYOrg; ptDest.y < rcDest.bottom;)
	{
		for (ptDest.x = rcDest.left, ptSrc.x = nSrcXOrg; ptDest.x < rcDest.right;)
		{
			RECT rcTileSrc;
			RECT rcTileDest;

			rcTileSrc.left = ptSrc.x;
			rcTileSrc.right = rcSrc.right;
			rcTileSrc.top = ptSrc.y;
			rcTileSrc.bottom = rcSrc.bottom;

			nSrcWidth = rcTileSrc.right - rcTileSrc.left;
			nSrcHeight = rcTileSrc.bottom - rcTileSrc.top;

			rcTileDest.left = ptDest.x;
			rcTileDest.right = rcTileDest.left + nSrcWidth;
			rcTileDest.top = ptDest.y;
			rcTileDest.bottom = rcTileDest.top + nSrcHeight;
			
			if (NxDrawLocal::ClipRect(&rcTileDest, &rcTileSrc, &rcDest, NULL))
			{
				if (!Blt(&rcTileDest, pSrcSurface, &rcTileSrc, pNxBlt))
					return FALSE;
			}

			// 次の X 座標へ
			ptDest.x += nSrcWidth;
			// 転送元 X 原点を reset
			ptSrc.x = rcSrc.left;
		}
		// 転送先を転送元の高さ分だけ下へ
		ptDest.y += nSrcHeight;
		// 転送元 Y 原点を reset
		ptSrc.y = rcSrc.top;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Blt(int dx, int dy, CNxSurface* pSurface, const RECT* lpSrcRect, NxBlt* pNxBlt = NULL)
// 概要: ビットブロック転送
// 引数: int dx                  ... 転送先 X 座標
//       int dy                  ... 転送先 Y 座標
//       CNxSurface* pSurface    ... 転送元サーフェス
//       const RECT* lpSrcRect   ... 転送元矩形
//       const NxBlt* pNxBlt ... 転送方法を指定する 
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Blt(int dx, int dy, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	if (lpSrcRect == NULL)
	{
		if (pSrcSurface == NULL)
		{
			_RPTF0(_CRT_ASSERT, "CNxSurface::Blt() : 転送元は省略できません.\n");
			return FALSE;
		}
		pSrcSurface->GetRect(&rcSrc);
		lpSrcRect = &rcSrc;
	}

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + abs(lpSrcRect->right - lpSrcRect->left);
	rcDest.bottom = dy + abs(lpSrcRect->bottom - lpSrcRect->top);
	return Blt(&rcDest, pSrcSurface, lpSrcRect, pNxBlt);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Blt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, NxBlt* pNxBlt = NULL)
// 概要: ビットブロック転送
// 引数: const RECT* lpDestRect        ... 転送先矩形
//       const CNxSurface* pSrcSurface ... 転送元サーフェス
//       const RECT* lpSrcRect         ... 転送元矩形
//       const NxBlt* pNxBlt       ... 転送方法を指定する(NULL ならばカラーキー無し単純転送)
// 戻値: 成功ならば TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Blt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	RECT rcDest;

	// 転送先が省略(NULL)されていれば全体を示す矩形を、そうでなければ lpDestRect を copy
	SetAbbreviateRect(&rcDest, lpDestRect);

	if (pSrcSurface == NULL)
	{	// 転送元が省略されていれば、転送先が指定されたと見なす
		pSrcSurface = this;
		rcSrc = rcDest;
	}
	else
	{	// lpSrcRect が NULL であれば全体
		pSrcSurface->SetAbbreviateRect(&rcSrc, lpSrcRect);
	}

	if (pNxBlt != NULL && pNxBlt->dwFlags != 0)
	{
		const DWORD& dwFlags = pNxBlt->dwFlags;

		// 左右反転
		if (dwFlags & NxBlt::mirrorLeftRight)
			std::swap(rcSrc.left, rcSrc.right);

		// 上下反転
		if (dwFlags & NxBlt::mirrorTopDown)
			std::swap(rcSrc.top, rcSrc.bottom);
		
		// 動的コード実行
		if (dwFlags & NxBlt::dynamic)
		{
			return m_pDynamicDraw->Blt(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
		}
		// colorFill (color | fill)
		else if ((dwFlags & (NxBlt::color | NxBlt::fill)) == (NxBlt::color | NxBlt::fill))
		{
			if ((dwFlags & (NxBlt::destAlpha | NxBlt::srcAlpha)) == (NxBlt::destAlpha | NxBlt::srcAlpha))
			{	// 転送先と転送元アルファを使用するブレンド
				return m_customDraw.Blt_ColorFill_BlendDestSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::blurHorz)
			{	// 水平ぼかしブレンド塗りつぶし
				return m_customDraw.Blt_ColorFill_BlurHorzBlend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::srcAlpha)
			{	// 転送元アルファのみ使用するブレンド
				return m_customDraw.Blt_ColorFill_BlendSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::destAlpha)
			{	// 転送先アルファのみ使用するブレンド
				return m_customDraw.Blt_ColorFill_BlendDestAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::rgbaMask)
			{	// マスク付き塗り潰し
				return m_customDraw.Blt_ColorFill_RGBAMask(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else
			{	// ブレンド塗り潰し
				return m_customDraw.Blt_ColorFill_Blend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
		}
		else
		{
			if (dwFlags & NxBlt::rule)
			{	// ルール画像使用ブレンド
				return m_customDraw.Blt_RuleBlend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::blurHorz)
			{	// 水平ぼかし
				return m_customDraw.Blt_BlurHorz(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if ((dwFlags & (NxBlt::destAlpha | NxBlt::srcAlpha)) == (NxBlt::destAlpha | NxBlt::srcAlpha))
			{	// 転送先と転送元アルファを使用するブレンド Blt
				return m_customDraw.Blt_BlendDestSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::srcAlpha)
			{	// 転送元アルファのみ使用するブレンド Blt
				return m_customDraw.Blt_BlendSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::destAlpha)
			{	// 転送先アルファのみ使用するブレンド Blt
				return m_customDraw.Blt_BlendDestAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::rgbaMask)
			{	// マスク転送
				return m_customDraw.Blt_RGBAMask(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else
			{	// アルファを使用しないブレンド転送
				return m_customDraw.Blt_Blend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
		}
	}
	return m_customDraw.Blt_Normal(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FilterBlt(int dx, int dy, const CNxSurface* pSrcSurface,
//							   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
// 概要: フィルタを適用して転送
// 引数: int dx                        ... フィルタ適用結果の転送先左上 X 座標
//       int dy                        ... フィルタ適用結果の転送先左上 Y 座標
//       const CNxSurface* pSrcSurface ... フィルタが適用されるサーフェスへのポインタ
//       const RECT* lpSrcRect         ... フィルタが適用される矩形を示す RECT 構造体へのポインタ
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FilterBlt(int dx, int dy, const CNxSurface* pSrcSurface,
						   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	if (lpSrcRect == NULL)
	{
		if (pSrcSurface == NULL)
		{
			_RPTF0(_CRT_ASSERT, "CNxSurface::FilterBlt() : 転送元は省略できません.\n");
			return FALSE;
		}
		pSrcSurface->GetRect(&rcSrc);
		lpSrcRect = &rcSrc;
	}

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + abs(lpSrcRect->right - lpSrcRect->left);
	rcDest.bottom = dy + abs(lpSrcRect->bottom - lpSrcRect->top);
	return FilterBlt(&rcDest, pSrcSurface, lpSrcRect, pNxFilterBlt);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FilterBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//							   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
// 概要: フィルタを適用して転送
// 引数: const RECT* lpDestRect        ... フィルタ適用結果の転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... フィルタが適用されるサーフェスへのポインタ
//       const RECT* lpSrcRect         ... フィルタが適用される矩形を示す RECT 構造体へのポインタ
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FilterBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
						   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcDest;
	RECT rcSrc;

	SetAbbreviateRect(&rcDest, lpDestRect);
	if (pSrcSurface == NULL)
	{	// pSrcSurface == NULL ならば、クリップを行った後の転送先矩形と同じにする
		if (!ClipBltRect(rcDest))
			return TRUE;		// 転送矩形なし

		pSrcSurface = this;		// 転送元と転送先サーフェスは同じ
		rcSrc = rcDest;
	}
	else
	{	// 転送元指定あり
		pSrcSurface->SetAbbreviateRect(&rcSrc, lpSrcRect);

		RECT rcSrcClip;
		pSrcSurface->GetRect(&rcSrcClip);
		if (!ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;		// 転送矩形なし
	}

	// 全てのフィルタは拡大縮小をサポートしない
	if (((rcDest.right - rcDest.left) - abs(rcSrc.right - rcSrc.left) |
		(rcDest.bottom - rcDest.top) - abs(rcSrc.bottom - rcSrc.top)) != 0)
	{
		_RPTF0(_CRT_ASSERT, "拡大縮小はサポートしていません.\n");
		return FALSE;
	}
	
	// NxFilterBlt::uOpacity の範囲をチェック
	if (pNxFilterBlt->dwFlags & NxFilterBlt::opacity)
	{	// uOpacity == 0 ならば、何もしない
		if (pNxFilterBlt->uOpacity == 0)
			return TRUE;
		else if (pNxFilterBlt->uOpacity > 255)
		{	// uOpacity は 0 ～ 255 でなければならない
			_RPTF0(_CRT_ASSERT, "NxFilterBlt.uOpacity の値は範囲外です.\n");
			return FALSE;
		}
	}

	typedef BOOL (CNxCustomDraw::*FilterProc)(CNxSurface* pDestSurface, const RECT* lpDestRect,
											  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
											  const NxFilterBlt* pNxFilterBlt) const;
	
	static const FilterProc pfnFilter[] =
	{
		&CNxCustomDraw::Filter_Grayscale,		// グレイスケール化
		&CNxCustomDraw::Filter_HueTransform,		// 色相変換
		&CNxCustomDraw::Filter_RGBColorBalance,	// RGB カラーバランス
		&CNxCustomDraw::Filter_Negative,			// ネガ反転
	};
	return (m_customDraw.*pfnFilter[pNxFilterBlt->dwFlags & NxFilterBlt::operationMask])
		(this, &rcDest, pSrcSurface, &rcSrc, pNxFilterBlt);
}

//////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::UpdateDIBColorTable()
// 概要: DIBSection のカラーテーブルを更新
// 引数: なし
// 戻値: 成功ならば TRUE
//////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::UpdateDIBColorTable()
{
	_ASSERTE(m_hBitmap != NULL);

	// HDC 取得
	HDC hDC = GetDC();
	if (hDC == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::UpdateDIBColorTable() : HDC の取得に失敗しました.\n");
		return FALSE;
	}
	BOOL bResult = ::SetDIBColorTable(hDC, 0, m_dibImage.GetColorCount(), m_dibImage.GetColorTable()) != 0;
	// HDC 開放
	ReleaseDC();
	return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::LoadImage(int dx, int dy, CNxFile& nxfile, const RECT* lpSrcRect = NULL,
//							   BOOL bUpdateColorTable = FALSE)
// 概要: イメージを指定矩形へ読み込む
// 引数: int dx                 ... 転送先 X 座標
//       int dy                 ... 転送先 Y 座標
//       CNxFile &nxfile        ... 読み込み元ファイル
//		 const RECT* lpSrcRect  ... 読み込まれたイメージから転送する矩形(NULL ならば全体)
//		 BOOL bUpdateColorTable ... TRUE ならばカラーテーブルを更新(8bpp 形式のみ)
// 戻値: 成功ならば TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::LoadImage(int dx, int dy, CNxFile& nxfile, const RECT* lpSrcRect, BOOL bUpdateColorTable)
{
	_ASSERTE(m_hBitmap != NULL);

	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::LoadImage() : オープンされていない CNxFile オブジェクトが渡されました.\n");
		return FALSE;
	}
	
	// 画像の読み込み
	std::auto_ptr<CNxDIBImage> dibImage(CNxDraw::GetInstance()->LoadImage(nxfile));
	if (dibImage.get() == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::LoadImage() : 画像の展開に失敗しました.\n");
		return FALSE;
	}

	// DIB を設定
	if (!SetDIBits(dx, dy, dibImage.get(), lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::LoadImage() : 対応していない DIB です.\n");
		return FALSE;
	}

	// カラーテーブルの設定
	if (bUpdateColorTable)
		SetColorTable(dibImage.get());

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FillRect(const RECT* lpRect, NxColor nxColor)
// 概要: サーフェスを塗りつぶす(ブレンド無しの純粋な上書き)
// 引数: const RECT* lpRect ... 塗りつぶす矩形 (NULL ならば全体)
//       NxColor nxColor    ... 塗りつぶす色
// 戻値: 成功ならば TRUE
// 備考: NxColor でそのままサーフェスを塗り潰す。8bpp の場合は A(α) 要素のみを使用
////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FillRect(const RECT* lpRect, NxColor nxColor)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rect;
	SetAbbreviateRect(&rect, lpRect);

	NxBlt nxb;
	nxb.nxbColor = nxColor;

	return m_customDraw.Blt_ColorFill_Normal(this, &rect, NULL, NULL, &nxb);
}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FillRect(int dx, int dy, int cx, int cy, NxColor nxcrColor)
// 概要: サーフェスを塗りつぶす
// 引数: int dx            ... 塗り潰し開始X座標
//       int dy            ... 塗り潰し開始Y座標
//       int cx            ... 塗り潰し領域の幅
//       int cy            ... 塗り潰し領域の高さ
//       NxColor nxcrColor ... 塗りつぶす色
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FillRect(int dx, int dy, int cx, int cy, NxColor nxcrColor)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + cx;
	rcDest.bottom = dy + cy;
	return FillRect(&rcDest, nxcrColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxFont* CNxSurface::SetFont(CNxFont* pFont)
// 概要: サーフェス描画へ用いるフォントを設定
// 引数: CNxFont* ... CNxFont オブジェクトへのポインタ
// 戻値: 成功ならば以前に選択されていた CNxFont
//////////////////////////////////////////////////////////////////////////////////////////////////////////

CNxFont* CNxSurface::SetFont(CNxFont* pFont)
{
	_ASSERTE(m_hBitmap != NULL);
	std::swap(m_pFont, pFont);
	return pFont;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SetTextSmoothing(BOOL bSmoothing)
// 概要: テキストスムージング描画の有無を設定
// 引数: BOOL bSmoothing ... TRUE ならばスムージング描画を行う
// 戻値: 以前の状態
// 備考: デフォルトはスムージング描画しない
///////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetTextSmoothing(BOOL bSmoothing)
{
	_ASSERTE(m_hBitmap != NULL);
	std::swap(m_bTextSmoothing, bSmoothing);
	return bSmoothing;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::GetTextExtent(LPCTSTR lpszString, LPRECT lpRect)
// 概要: テキストの描画矩形を取得
// 引数: LPCTSTR lpszString ... 文字列へのポインタ
//       LPRECT  lpRect     ... 矩形を得る RECT 構造体へのポインタ
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::GetTextExtent(LPCTSTR lpszString, LPRECT lpRect)
{
	_ASSERTE(m_hBitmap != NULL);

	_ASSERTE(lpszString != NULL);
	_ASSERTE(lpRect != NULL);
	
	if (m_pFont == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::GetTextExtent() : フォントが指定されていません.\n");
		return FALSE;
	}

	// DC 取得
	HDC hDC = GetDC();
	if (hDC == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::GetTextExtent() : DC の取得に失敗しました.\n");
		return FALSE;		// DC 取得失敗
	}

	// フォントを取得/選択
	HFONT hFont = m_pFont->GetHandleInternal((m_bTextSmoothing) ? CNxFont::FontType_Smooth : CNxFont::FontType_Normal);
	if (hFont == NULL)
	{
		_RPTF0(_CRT_ERROR, "CNxSurface::GetTextExtent() : CNxFont::GetHandle() が失敗しました.\n");
		ReleaseDC();
		return FALSE;
	}
	HFONT hFontOld = static_cast<HFONT>(::SelectObject(hDC, hFont));
	
	// テキストの矩形を得る
	// この関数で得られるサイズは、回転やイタリック、ボールドは考慮されない(らしい)
	SIZE size;
	if (!::GetTextExtentPoint32(hDC, lpszString, _tcslen(lpszString), &size))
	{
		_RPTF0(_CRT_ERROR, "CNxSurface::GetTextExtent() : GetTextExtentPoint32() 関数が失敗しました.\n");
		::SelectObject(hDC, hFontOld);
		ReleaseDC();
		return FALSE;
	}

	// TEXTMETRIC を取得
	TEXTMETRIC tm;
	::GetTextMetrics(hDC, &tm);

	// DC 開放
	::SelectObject(hDC, hFontOld);
	ReleaseDC();

	// イタリック体の時は、斜めになった分を幅へ加算
	if (tm.tmItalic)
	{
		size.cx += tm.tmAscent + tm.tmDescent;
	}

	// スムージングを行うなら実サイズは 1/CNxSurface::SmoothFontRatio にする
	if (m_bTextSmoothing)
	{
		// CNxSurface::SmoothFontRatio 単位へ切り上げ
		const UINT ratio = CNxSurface::SmoothFontRatio;
		// unsigned で計算
		UINT cx = size.cx;
		UINT cy = size.cy;
		cx = ((cx + ratio - 1) / ratio);
		cy = ((cy + ratio - 1) / ratio);
		size.cx = cx;
		size.cy = cy;
	}
	
	// 回転時の変換
	// 吉里吉里 source ver.0.80 by W.Dee 氏 (http://www.din.or.jp/~glit/TheOddStage/TVP)
	// Projects\TVP32\OperatorsUnit.cpp を参考にしました

	// LOGFONT から回転角度を取得
	LONG lEscapement = m_pFont->GetEscapement();

	// 角度を 0 ～ 3599 へ正規化
	lEscapement %= 360 * 10;
	if (lEscapement < 0)
		lEscapement += 3600;

	if (lEscapement == 0)
	{	// 横書き
		lpRect->left = 0;
		lpRect->top = 0;
		lpRect->right = size.cx;
		lpRect->bottom = size.cy;
	}
	else if (lEscapement == 270 * 10)
	{	// 縦書き
		lpRect->left = -size.cy;
		lpRect->top = 0;
		lpRect->right = 0;
		lpRect->bottom = size.cx;
	}
	else
	{
		const double fPI = 3.1415926535897932;
		double fDir = static_cast<double>(lEscapement) * fPI / 180.0f / 10.0f;
		double fSin = sin(fDir);
		double fCos = cos(fDir);

		if (lEscapement >= 0 && lEscapement < 90 * 10)
		{	// 0 - 90
			lpRect->left = 0;
			lpRect->top = static_cast<int>(-fSin * size.cx);
			lpRect->right = static_cast<int>(fSin * size.cy + fCos * size.cx);
			lpRect->bottom = static_cast<int>(fCos * size.cy);
		}
		else if (lEscapement >= 90 * 10 && lEscapement < 180 * 10)
		{	// 90 - 180
			lpRect->left = static_cast<int>(fCos * size.cx);
			lpRect->top = static_cast<int>(fCos * size.cy - fSin * size.cx);
			lpRect->right = static_cast<int>(fSin * size.cy);
			lpRect->bottom = 0;
		}
		else if (lEscapement >= 180 * 10 && lEscapement < 270 * 10)
		{	// 180 - 270
			lpRect->left = static_cast<int>(fCos * size.cx + fSin * size.cy);
			lpRect->top = static_cast<int>(fCos * size.cy);
			lpRect->right = 0;
			lpRect->bottom = static_cast<int>(-fSin * size.cx);
		}
		else
		{	// 270 - 360
			lpRect->left = static_cast<int>(fSin * size.cy);
			lpRect->top = 0;
			lpRect->right = static_cast<int>(fCos * size.cx);
			lpRect->bottom = static_cast<int>(fCos * size.cy - fSin * size.cx);
		}
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, const NxBlt *pNxBlt)
// 概要: テキストを描画
// 引数: int dx              ... 描画を開始する X 座標
//       iny dy              ... 描画を開始する Y 座標
//       const RECT* lpRect  ... クリッピング矩形(NULL ならばクリップしない)
//		 LPCTSTR lpszString  ... 文字列
//       const NxBlt *pNxBlt ... 内部バッファサーフェス(8bpp)からの転送方法を指定する NxBlt 構造体へのポインタ
// 戻値: 成功ならば TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, const NxBlt *pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	if (m_pFont == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::DrawText() : フォントが指定されていません.\n");
		return FALSE;
	}
	
	_ASSERTE(lpszString != NULL);
	
	// 描画したときのサイズを計算
	RECT rcText;
	if (!GetTextExtent(lpszString, &rcText))
	{	// エラー発生
		return FALSE;
	}

	UINT uTextWidth = static_cast<UINT>(rcText.right - rcText.left);
	UINT uTextHeight = static_cast<UINT>(rcText.bottom - rcText.top);

	if ((uTextWidth - 1 | uTextHeight - 1) + 1 == 0)
	{	// テキストの横幅又は高さがゼロ
		return TRUE;
	}

	RECT rcDest;
	rcDest.top = dy;
	rcDest.left = dx;
	rcDest.right = rcDest.left + uTextWidth;
	rcDest.bottom = rcDest.top + uTextHeight;

	RECT rcSrc;
	rcSrc.top = 0;
	rcSrc.left = 0;
	rcSrc.right = uTextWidth;
	rcSrc.bottom  = uTextHeight;

	// クリッピング矩形が指定されているならば、クリップしてみる
	if (lpRect != NULL)
	{
		if (!NxDrawLocal::ClipRect(&rcDest, &rcSrc, lpRect, NULL))
		{	// 描画すべき矩形がない
			return TRUE;
		}
	}

	// 一時サーフェスへ要求する幅と高さを準備
	UINT uSurfaceWidth = uTextWidth;
	UINT uSurfaceHeight = uTextHeight;
	if (m_bTextSmoothing)
	{	// スムージングする
		uSurfaceWidth *= 4;
		uSurfaceHeight *= 4;
	}

	// 一時サーフェス取得
	CNxSurface* pGrayscaleSurface = pGrayscaleSurface = CNxDraw::GetInstance()->GetTextTemporarySurface(uSurfaceWidth, uSurfaceHeight);
	if (pGrayscaleSurface == NULL)
		return FALSE;

	// 一時サーフェスの HDC を取得
	HDC hDC = pGrayscaleSurface->GetDC();
	if (hDC == NULL)
		return FALSE;

	// pGrayscaleSurface の DC は ReleaseDC() で必ず開放される
	// はずなので、状態の復帰は行なっていない

	if (::SelectObject(hDC, m_pFont->GetHandleInternal((m_bTextSmoothing) ? CNxFont::FontType_Smooth : CNxFont::FontType_Normal)) == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface:DrawText() : フォントの選択に失敗しました.\n");
		pGrayscaleSurface->ReleaseDC();
		return FALSE;
	}

	// 背景色の指定
	::SetBkColor(hDC, RGB(0, 0, 0));

	if (m_bTextSmoothing)
	{	// スムージングする場合...
		// 4倍角の文字を描画してから、補完縮小
		rcText.left *= CNxSurface::SmoothFontRatio;
		rcText.top *= CNxSurface::SmoothFontRatio;

		// テキストクリップ矩形の準備
		RECT rcTextClip;
		rcTextClip.left = rcSrc.left;
		rcTextClip.top = rcSrc.top;
		rcTextClip.right = rcSrc.left + uSurfaceWidth;
		rcTextClip.bottom = rcSrc.bottom + uSurfaceHeight;

		::SetTextColor(hDC, RGB(15, 15, 15));
		::ExtTextOut(hDC, -rcText.left, -rcText.top, ETO_OPAQUE, &rcTextClip, lpszString, _tcslen(lpszString), NULL);
		pGrayscaleSurface->ReleaseDC();
		::GdiFlush();
		FontSmoothing4x4(pGrayscaleSurface->GetBits(), pGrayscaleSurface->GetPitch(), uTextWidth, uTextHeight);
	}
	else
	{	// スムージングしない
		::SetTextColor(hDC, RGB(255, 255, 255));
		::ExtTextOut(hDC, -rcText.left, -rcText.top, ETO_OPAQUE, &rcSrc, lpszString, _tcslen(lpszString), NULL);
		pGrayscaleSurface->ReleaseDC();
		::GdiFlush();
	}

	// 出力
	BOOL bResult = Blt(&rcDest, pGrayscaleSurface, &rcSrc, pNxBlt);
	return bResult;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, NxColor nxColor)
// 概要: テキストを描画
// 引数: int dx             ... 描画を開始する X 座標
//       int dy             ... 描画を開始する Y 座標
//       const RECT* lpRect ... クリップ矩形(NULLL ならばサーフェス全体)
//		 LPCTSTR lpszString ... 文字列
//       NxColor nxColor    ... 描画色(アルファも有効)
// 戻値: 成功ならば TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, NxColor nxColor)
{
	_ASSERTE(m_hBitmap != NULL);
	_ASSERTE(lpszString != NULL);

	// 結果を通常ブレンドで転送
	NxBlt nxbf;
	nxbf.dwFlags = NxBlt::blendNormal | NxBlt::colorFill | NxBlt::destAlpha | NxBlt::srcAlpha;
	nxbf.nxbColor = nxColor;
	return DrawText(dx, dy, lpRect, lpszString, &nxbf);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SetDIBits(int dx, int dy, const CNxDIBImage* pDIBImage, LPCVOID lpvBits, const RECT* lpSrcRect = NULL)
// 概要: DIB を(必要ならば変換して) サーフェスへ転送
// 引数: int dx						  ... 転送先 X 座標
//       int dy						  ... 転送先 Y 座標
//       const CNxDIBImage* pDIBImage ... 転送元 CNxDIBImage オブジェクトへのポインタ
//		 const RECT* lpSrcRect		  ... 転送矩形(NULL ならば全体)
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetDIBits(int dx, int dy, const CNxDIBImage* pDIBImage, const RECT* lpSrcRect)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	if (lpSrcRect == NULL)
		pDIBImage->GetRect(&rcSrc);
	else
		rcSrc = *lpSrcRect;

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + (rcSrc.right - rcSrc.left);
	rcDest.bottom = dy + (rcSrc.bottom - rcSrc.top);

	// クリップ
	RECT rcSrcClip;
	pDIBImage->GetRect(&rcSrcClip);
	
	if (!ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;

	return m_dibImage.Blt(dx, dy, pDIBImage, lpSrcRect);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SetDIBits(int dx, int dy, const BITMAPINFO* lpbmi, LPCVOID lpvBits, const RECT* lpSrcRect = NULL)
// 概要: DIB を(必要ならば変換して) サーフェスへ転送
// 引数: int dx                  ... 転送先 X 座標
//       int dy                  ... 転送先 Y 座標
//       const BITMAPINFO *lpbmi ... 転送元 DIB
//       LPCVOID lpvBits         ... 転送元 DIB のビットデータへのポインタ(NULL ならば、lpbmi から計算)
//		 const RECT* lpSrcRect   ... 転送矩形(NULL ならば全体)
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetDIBits(int dx, int dy, const BITMAPINFO* lpbmi, LPCVOID lpvBits, const RECT* lpSrcRect)
{
	_ASSERTE(m_hBitmap != NULL);

	CNxDIBImage srcDIBImage;
	if (!srcDIBImage.Create(const_cast<LPBITMAPINFO>(lpbmi), const_cast<LPVOID>(lpvBits)))
		return FALSE;
	else
		return SetDIBits(dx, dy, &srcDIBImage, lpSrcRect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::GetDIBits(int dx, int dy, LPBITMAPINFO lpbmi, LPVOID lpvBits, const RECT* lpSrcRect = NULL) const
// 概要: サーフェスの内容を DIB として取得
// 引数: int dx                ... DIB 上の転送先 X 座標
//       int dy                ... DIB 上の転送先 Y 座標
//       LPBITMAPINFO lpbmi    ... 転送先 DIB の情報を示す BITMAPINFO 構造体へのポインタ
//       LPVOID lpvBits        ... ビットデータを取得するバッファへのポインタ(NULL 可)
//       const RECT* lpSrcRect ... DIB へ転送するサーフェス内矩形を示す RECT 構造体へのポインタ(NULL = 全体)
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::GetDIBits(int dx, int dy, LPBITMAPINFO lpbmi, LPVOID lpvBits, const RECT* lpSrcRect) const
{
	_ASSERTE(m_hBitmap != NULL);
	_ASSERTE(lpbmi != NULL);

	if (lpbmi->bmiHeader.biSize < sizeof(BITMAPINFOHEADER) ||
		lpbmi->bmiHeader.biPlanes != 1 ||
		lpbmi->bmiHeader.biCompression != BI_RGB)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::GetDIBits() : BITMAPINFO 構造体の内容が異常です.\n");
		return FALSE;
	}
		
	// 転送元クリップ矩形(このサーフェス全体を示す矩形)を取得
	RECT rcSrcClip;
	GetRect(&rcSrcClip);

	RECT rcSrc;

	// lpSrcRect == NULL ならば、サーフェス全体を示す
	if (lpSrcRect == NULL)
		GetRect(&rcSrc);
	else
		rcSrc = *lpSrcRect;


	// 転送先クリップ矩形を準備
	RECT rcDestClip;
	rcDestClip.left = 0;
	rcDestClip.top = 0;
	rcDestClip.right = lpbmi->bmiHeader.biWidth;
	rcDestClip.bottom = abs(lpbmi->bmiHeader.biHeight);

	RECT rcDest = rcDestClip;
	::OffsetRect(&rcDest, dx, dy);

	// BITMAPINFO 構造体の設定
	LONG lDIBPitch = (((((lpbmi->bmiHeader.biBitCount * lpbmi->bmiHeader.biWidth) + 7) / 8) + 3) / 4) * 4;
	lpbmi->bmiHeader.biSizeImage = lDIBPitch * abs(lpbmi->bmiHeader.biHeight);
	lpbmi->bmiHeader.biXPelsPerMeter = 0;
	lpbmi->bmiHeader.biYPelsPerMeter = 0;
	lpbmi->bmiHeader.biClrUsed = 0;
	lpbmi->bmiHeader.biClrImportant = 0;

	// クリップする
	if (!NxDrawLocal::ClipRect(&rcDest, &rcSrc, &rcDestClip, &rcSrcClip))
	{	// 転送矩形が空
		return TRUE;
	}
	if (lpvBits == NULL)
		return TRUE;	// ビットデータは取得しない

	CNxDIBImage destDIBImage;
	if (!destDIBImage.Create(lpbmi, lpvBits))
		return FALSE;

	return destDIBImage.Blt(rcDest.left, rcDest.top, &GetDIBImage(), &rcSrc);
}

//////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSurface::SetColorTable(const CNxDIBImage* pDIBImage)
// 概要: CNxDIBImage オブジェクトのパレットからカラーテーブルを設定
// 引数: const CNxDIBImage* pDIBImage ... 設定元 CNxDIBImage オブジェクトへのポインタ
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetColorTable(const CNxDIBImage* pDIBImage)
{
	if (GetBitCount() > 8)
		return FALSE;		// サーフェスが 8bpp ではない

	if (pDIBImage->GetBitCount() > 8)
		return FALSE;		// DIB がパレット付きではない

	memcpy(m_dibImage.GetColorTable(), pDIBImage->GetColorTable(), pDIBImage->GetColorCount() * sizeof(RGBQUAD));
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SaveBitmapFile(LPCTSTR lpszFileName, const RECT* lpRect = NULL) const
// 概要: サーフェスの内容をビットマップファイルとして保存する
// 引数: LPCTSTR lspzFileName ... 保存ファイル名
//       const RECT* lpRect   ... 保存する矩形(NULL ならば全体)
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SaveImage(LPCTSTR lpszFileName, const RECT* lpRect) const
{
	_ASSERTE(m_hBitmap != NULL);
	_ASSERTE(lpszFileName != NULL);

	CNxFile nxfile;
	if (!nxfile.Open(lpszFileName, CNxFile::modeCreate|CNxFile::modeWrite))
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::SaveBitmapFile() : ファイルの作成に失敗しました.\n");
		return FALSE;
	}
	return SaveImage(nxfile, lpRect);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SaveBitmapFile(CNxFile& nxfile, const RECT* lpRect = NULL) const
// 概要: サーフェスの内容をビットマップファイルとして保存する
// 引数: CNxFile& nxfile ... 書き込み可能なファイルへの参照
//       const RECT* lpRect ... 保存する矩形(NULL ならば全体)
// 戻値: 成功なら TRUE/ 失敗なら FALSE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SaveImage(CNxFile& nxfile, const RECT* lpRect) const
{
	_ASSERTE(m_hBitmap != NULL);

	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::SaveBitmapFile() : ファイルは開かれていません.\n");
		return FALSE;
	}

	// 24bpp 又は 8bpp の BMP として保存
	CNxBMPImageSaver saver(CNxBMPImageSaver::stripAlpha);
	return saver.SaveDIBImage(nxfile, GetDIBImage(), lpRect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxSurface::FontSmoothing4x4(LPBYTE lpSurface, LONG lPitch,
//											 UINT uWidth, UINT uHeight)
// 概要: スムージング描画用内部ルーチン (4x4 -> 1x1)
// 引数: LPVOID lpSurface     ... 元データ及び縮小後のデータが置かれるサーフェスへのポインタ
//       LONG   lPitch        ... サーフェスの送り幅(pSurface->GetPitch() の値)
//       LONG   lSrcDistance  ... 転送元(縮小前)の最後のピクセルから次の行への距離
//       UINT   uWidth        ... 縮小後の幅(元サイズの 1/4)
//       UINT   uHeight       ... 縮小後の高さ(元サイズの 1/4)
// 詳細:
//       元のピクセル値は 15(0x0f) で描画。注目している左上を基準として縦横 4 dot (合計 16 dot) を単純に加算。
//       合計が最大でも 15 * 16 = 0xf8 となってしまうため、5 - 3bit を 2 - 0bit へコピーして 255 に合わせる。
//       結果は、0 - 255 でピクセルの不透明度を示す。
//       元データは破壊される。
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  __declspec(naked) CNxSurface::FontSmoothing4x4(LPVOID /*lpSurface*/, LONG /*lPitch*/,
													 UINT /*uWidth*/, UINT /*uHeight*/)
{
#pragma pack(push, 4)
	struct StackFrame
	{
		DWORD	esi;
		DWORD	ebp;
		DWORD	ebx;
		DWORD	edi;
		DWORD	eip;
		LPVOID	lpSurface;
		LONG	lPitch;
		UINT	uWidth;
		UINT	uHeight;
	};
#pragma pack(pop)

	__asm
	{
		push	edi
		push	ebx
		push	ebp
		push	esi
		mov		eax, [esp]StackFrame.uWidth
		mov		esi, [esp]StackFrame.lPitch
		mov		ebx, [esp]StackFrame.uHeight
		shl		eax, 2
		mov		edi, [esp]StackFrame.lpSurface
		sub		esi, eax
		mov		ebp, edi

loop_y:
		
		mov		edx, [esp]StackFrame.uWidth
		mov		ecx, edx

		// y mod 4 = 0
loop_x_base:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		mov		[ebp], al
		inc		ebp
		loop	loop_x_base

		add		edi, esi
		sub		ebp, edx
		mov		ecx, edx

		// y mod 4 = 1
loop_x_add1:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		add		[ebp], al
		inc		ebp
		loop	loop_x_add1

		add		edi, esi
		sub		ebp, edx
		mov		ecx, edx

		// y mod 4 = 2
	loop_x_add2:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		add		[ebp], al
		inc		ebp
		loop	loop_x_add2

		add		edi, esi
		sub		ebp, edx
		mov		ecx, edx

		// y mod 4 = 3
	loop_x_add3:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		add		al, [ebp]
		mov		ah, al
		shr		al, 3
		and		al, 7
		or		al, ah
		mov		[ebp], al
		inc		ebp
		loop	loop_x_add3

		add		ebp, [esp]StackFrame.lPitch
		add		edi, esi
		sub		ebp, [esp]StackFrame.uWidth
		dec		ebx
		jnz		loop_y

		pop		esi
		pop		ebp
		pop		ebx
		pop		edi
		ret
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	 const RECT* CNxSurface::SetAbbreviateRect(LPRECT lpRect, const RECT* lpSrcRect) const
// 概要: this が NULL かつ、lpSrcRect が NULL であれば、空の矩形を設定する
//       lpSrcRect が NULL ならば、サーフェス全体を示す矩形を設定して返す
//       それ以外は、lpSrcRect をそのまま設定する
// 引数: LPRECT lpRect         ... 結果を受けとる RECT 構造体へのポインタ
//       const RECT *lpSrcRect ... 入力矩形
// 戻値: なし
//////////////////////////////////////////////////////////////////////////////////////////////

void CNxSurface::SetAbbreviateRect(LPRECT lpRect, const RECT* lpSrcRect) const
{
	if (lpSrcRect != NULL)
		*lpRect = *lpSrcRect;			// lpSrcRect != NULL
	else
	{
		if (this == NULL)
			::SetRectEmpty(lpRect);		// lpSrcRect == NULL && this == NULL
		else
			GetRect(lpRect);			// lpSrcRect == NULL && this != NULL
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSurface::SetClipRect(const RECT* lpClipRect)
// 概要: 描画クリッピング矩形を設定
// 引数: const RECT* lpClipRect ... 描画可能な矩形。NULL ならばサーフェス全体
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////////

void CNxSurface::SetClipRect(const RECT* lpClipRect)
{
	_ASSERTE(m_hBitmap != NULL);

	if (lpClipRect == NULL)
	{
		GetRect(&m_rcClip);
	}
	else
	{
		m_rcClip.left = max(lpClipRect->left, 0);
		m_rcClip.right = min(lpClipRect->right, static_cast<int>(GetWidth()));
		m_rcClip.top = max(lpClipRect->top, 0);
		m_rcClip.bottom = min(lpClipRect->bottom, static_cast<int>(GetHeight()));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSurface::ClipBltRect(LPRECT lpDestRect) const
//
// 概要: サーフェスのクリッピング情報に基づく、転送先のみのクリップ
// 引数: LPRECT lpDestRect ... 入力矩形
// 戻値: 転送矩形があれば TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::ClipBltRect(RECT& destRect) const
{
	destRect.top += m_ptOrg.y;
	if (destRect.top < m_rcClip.top)
		destRect.top = m_rcClip.top;

	destRect.bottom += m_ptOrg.y;
	if (destRect.bottom > m_rcClip.bottom)
		destRect.bottom = m_rcClip.bottom;

	if (destRect.top >= destRect.bottom)
		return FALSE;

	destRect.left += m_ptOrg.x;
	if (destRect.left < m_rcClip.left)
		destRect.left = m_rcClip.left;

	destRect.right += m_ptOrg.x;
	if (destRect.right > m_rcClip.right)
		destRect.right = m_rcClip.right;

	if (destRect.left >= destRect.right)
		return FALSE;
	else
		return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::ClipBltRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpSrcClipRect) const
// 概要: サーフェスのクリッピング情報に基づく、転送元と転送先矩形のクリップ (Blt 関数用)
// 引数: LPRECT lpDestRect         ... 転送先矩形
//       LPRECT lpSrcRect          ... 転送元矩形
//       const RECT* lpSrcClipRect ... 転送元クリッピング矩形
// 戻値: 転送矩形があれば TRUE
// 備考: 上下左右反転に対応
//       left > right の場合は左右反転。クリップ後は、
//		 left が左上を示す様に right > left に修正される(上下も同様)。
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::ClipBltRect(RECT& destRect, RECT& srcRect, const RECT& srcClipRect) const
{
	destRect.left += m_ptOrg.x;
	destRect.right += m_ptOrg.x;
	destRect.top += m_ptOrg.y;
	destRect.bottom += m_ptOrg.y;


	if (srcRect.top > srcRect.bottom)
	{	// 上下反転 (top > bottom)
		if (srcRect.bottom < srcClipRect.top)
		{
			destRect.top += srcClipRect.top - srcRect.bottom;
			srcRect.bottom = srcClipRect.top;
		}
		if (destRect.top < m_rcClip.top)
		{
			srcRect.top -= m_rcClip.top - destRect.top;
			destRect.top = m_rcClip.top;
		}
		if (srcClipRect.bottom - srcRect.top < 0)
		{
			destRect.bottom += srcClipRect.bottom - srcRect.top;
			srcRect.top = srcClipRect.bottom;
		}
		if (m_rcClip.bottom - destRect.bottom < 0)
		{
			srcRect.bottom -= m_rcClip.bottom - destRect.bottom;
			destRect.bottom = m_rcClip.bottom;
		}
		std::swap(srcRect.top, srcRect.bottom);		// 上下入れ替え(top < bottom にする)
	}
	else
	{	// 通常 (top < bottom)
		if (srcRect.top < srcClipRect.top)
		{
			destRect.top += srcClipRect.top - srcRect.top;
			srcRect.top = srcClipRect.top;
		}
		if (destRect.top < m_rcClip.top)
		{
			srcRect.top += m_rcClip.top - destRect.top;
			destRect.top = m_rcClip.top;
		}
		if (srcClipRect.bottom - srcRect.bottom < 0)
		{
			destRect.bottom += srcClipRect.bottom - srcRect.bottom;
			srcRect.bottom = srcClipRect.bottom;
		}
		if (m_rcClip.bottom - destRect.bottom < 0)
		{
			srcRect.bottom += m_rcClip.bottom - destRect.bottom;
			destRect.bottom = m_rcClip.bottom;
		}
	}
	if (destRect.top >= destRect.bottom)
		return FALSE;
		
	if (srcRect.left > srcRect.right)
	{	// 左右反転 (left > right)
		if (srcRect.right < srcClipRect.left)
		{
			destRect.left += srcClipRect.left - srcRect.right;
			srcRect.right = srcClipRect.left;
		}
		if (destRect.left < m_rcClip.left)
		{
			srcRect.left -= m_rcClip.left - destRect.left;
			destRect.left = m_rcClip.left;
		}
		if (srcClipRect.right - srcRect.left < 0)
		{
			destRect.right += srcClipRect.right - srcRect.left;
			srcRect.left = srcClipRect.right;
		}
		if (m_rcClip.right - destRect.right < 0)
		{
			srcRect.right -= m_rcClip.right - destRect.right;
			destRect.right = m_rcClip.right;
		}
		std::swap(srcRect.left, srcRect.right);		// 左右入れ替え
	}
	else
	{	// 通常 (left < right)
		if (srcRect.left < srcClipRect.left)
		{
			destRect.left += srcClipRect.left - srcRect.left;
			srcRect.left = srcClipRect.left;
		}
		if (destRect.left < m_rcClip.left)
		{
			srcRect.left += m_rcClip.left - destRect.left;
			destRect.left = m_rcClip.left;
		}
		if (srcClipRect.right - srcRect.right < 0)
		{
			destRect.right += srcClipRect.right - srcRect.right;
			srcRect.right = srcClipRect.right;
		}
		if (m_rcClip.right - destRect.right < 0)
		{
			srcRect.right += m_rcClip.right - destRect.right;
			destRect.right = m_rcClip.right;
		}
	}
	if (destRect.left >= destRect.right)
		return FALSE;
	else
		return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::ClipStretchBltRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpSrcClipRect,
//										StretchBltInfo* pStretchBltInfo) const
// 概要: サーフェスのクリッピング情報に基づく、転送元と転送先矩形のクリップ (拡大縮小 Blt 用)
// 引数: LPRECT lpDestRect				 ... 転送先矩形
//       LPRECT lpSrcRect				 ... 転送元矩形
//       const RECT* lpSrcClipRect		 ... 転送元クリッピング矩形
//		 StretchBltInfo* pStretchBltInfo ... 補正情報を受けとる StretchBltInfo 構造体へのポインタ
// 戻値: 転送矩形があれば TRUE
// 備考: 反転も対応
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::ClipStretchBltRect(RECT& destRect, RECT& srcRect, const RECT& srcClipRect,
									StretchBltInfo& stretchBltInfo) const
{
	UINT uSrcWidth = abs(srcRect.right - srcRect.left);
	UINT uSrcHeight = abs(srcRect.bottom - srcRect.top);

	UINT uDestWidth = (destRect.right - destRect.left);
	destRect.left += m_ptOrg.x;
	destRect.right += m_ptOrg.x;
	UINT uDestHeight = (destRect.bottom - destRect.top);
	destRect.top += m_ptOrg.y;
	destRect.bottom += m_ptOrg.y;

	// 転送元、転送先の高さ又は幅がゼロならば転送矩形無し(devide by zero 回避)
	if ((uDestWidth - 1 | uSrcWidth  - 1 | uDestHeight - 1 | uSrcHeight - 1) + 1 == 0)
	{
		// 一つでも 0 が混じっていれば uValue - 1 によって、結果は 0xffffffff
		// ただし、値の範囲は 0 ～ 0x7fffffff が前提
		return FALSE;
	}


	ULARGE_INTEGER ul64Adjust;		// 原点補正用 temporary 変数

	stretchBltInfo.uSrcOrgY = 0;
	stretchBltInfo.ul64SrcDeltaY.LowPart = 0;
	stretchBltInfo.ul64SrcDeltaY.HighPart = uSrcHeight;
	stretchBltInfo.ul64SrcDeltaY.QuadPart /= uDestHeight;	// 転送元原点変位

	// 垂直
	if (srcRect.top > srcRect.bottom)
	{	// 上下反転
		if (srcClipRect.bottom < srcRect.top)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.top - srcClipRect.bottom);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.top += ul64Adjust.HighPart;
			srcRect.top = srcClipRect.bottom;
		}
		if (m_rcClip.bottom < destRect.bottom)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.bottom - m_rcClip.bottom);
			srcRect.bottom += ul64Adjust.HighPart;
			destRect.bottom = m_rcClip.bottom;
		}
		if (srcClipRect.top > srcRect.bottom)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.top - srcRect.bottom);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.bottom -= ul64Adjust.HighPart + 1;
			srcRect.bottom = srcClipRect.top;
		}
		if (m_rcClip.top > destRect.top)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.top - destRect.top);
			srcRect.top -= ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgY = ul64Adjust.LowPart;
			destRect.top = m_rcClip.top;
		}
		std::swap(srcRect.top, srcRect.bottom);
	}
	else
	{	// 上下反転しない
		if (srcClipRect.top > srcRect.top)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.top - srcRect.top);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.top += ul64Adjust.HighPart;
			srcRect.top = srcClipRect.top;
		}
		if (m_rcClip.top > destRect.top)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.top - destRect.top);
			srcRect.top += ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgY = ul64Adjust.LowPart;
			destRect.top = m_rcClip.top;
		}
		if (srcClipRect.bottom < srcRect.bottom)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.bottom - srcClipRect.bottom);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.bottom -= ul64Adjust.HighPart + 1;
			srcRect.bottom = srcClipRect.bottom;
		}
		if (m_rcClip.bottom < destRect.bottom)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.bottom - m_rcClip.bottom);
			srcRect.bottom -= ul64Adjust.HighPart;
			destRect.bottom = m_rcClip.bottom;
		}
	}
	if (destRect.top >= destRect.bottom)
		return FALSE;

	stretchBltInfo.uSrcOrgX = 0;
	stretchBltInfo.ul64SrcDeltaX.LowPart = 0;
	stretchBltInfo.ul64SrcDeltaX.HighPart = uSrcWidth;
	stretchBltInfo.ul64SrcDeltaX.QuadPart /= uDestWidth;

	// 水平
	if (srcRect.right < srcRect.left)
	{	// 左右反転
		if (srcClipRect.right < srcRect.left)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.left - srcClipRect.right);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.left += ul64Adjust.HighPart;
			srcRect.left = srcClipRect.right;
		}
		if (m_rcClip.right < destRect.right)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.right - m_rcClip.right);
			srcRect.right += ul64Adjust.HighPart;
			destRect.right = m_rcClip.right;
		}
		if (srcClipRect.left > srcRect.right)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.left - srcRect.right);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.right -= ul64Adjust.HighPart + 1;
			srcRect.right = srcClipRect.left;
		}
		if (m_rcClip.left > destRect.left)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.left - destRect.left);
			srcRect.left -= ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgX = ul64Adjust.LowPart;
			destRect.left = m_rcClip.left;
		}
		std::swap(srcRect.left, srcRect.right);
	}
	else
	{	// 左右反転しない
		if (srcClipRect.left > srcRect.left)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.left - srcRect.left);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.left += ul64Adjust.HighPart;
			srcRect.left = srcClipRect.left;
		}
		if (m_rcClip.left > destRect.left)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.left - destRect.left);
			srcRect.left += ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgX = ul64Adjust.LowPart;
			destRect.left = m_rcClip.left;
		}
		if (srcClipRect.right < srcRect.right)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.right - srcClipRect.right);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.right -= ul64Adjust.HighPart + 1;
			srcRect.right = srcClipRect.right;
		}
		if (m_rcClip.right < destRect.right)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.right - m_rcClip.right);
			srcRect.right -= ul64Adjust.HighPart;
			destRect.right = m_rcClip.right;
		}
	}
	if (destRect.left >= destRect.right)
		return FALSE;
	else
		return TRUE;
}
