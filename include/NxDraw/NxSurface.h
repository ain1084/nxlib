// NxSurface.h: CNxSurface クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェス管理クラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDraw.h"
#include "NxColor.h"
#include "NxDIBImage.h"
#include <vector>

// CNxSurface::FilterBlt() 関数で使用する構造体
struct NxFilterBlt
{
	static const DWORD grayscale		= 0x00000000;
	static const DWORD hueTransform		= 0x00000001;
	static const DWORD rgbColorBalance  = 0x00000002;
	static const DWORD negative			= 0x00000003;
	static const DWORD operationMask    = 0x00000003;
	static const DWORD opacity          = 0x80000000;

	DWORD dwFlags;
	UINT  uOpacity;		// フィルタ適用結果の不透明度(0 ～ 255)

	union
	{
		// nxfRGBColorBalance
		struct RGBColorBalance
		{
			struct
			{
				WORD wBlue;
				WORD wGreen;
				WORD wRed;
				WORD wDummy;
			} Multiplier;
			struct
			{
				short sBlue;
				short sGreen;
				short sRed;
				short wDummy;
			} Adder;
		} nxfRGBColorBalance;
		// nxfHueTransform
		struct HueTransform
		{
			int  nHue;
			int  nSaturation;
			int  nLightness;
			BOOL bSingleness;
		} nxfHueTransform;
	};
};

// Blt で使用するエフェクト等を指定する構造体
struct NxBlt
{
	static const DWORD blendNormal     = 0x00000000;	// 通常ブレンド
	static const DWORD blendAdd        = 0x00000001;	// 加算ブレンド
	static const DWORD blendSub        = 0x00000002;	// 減算ブレンド
	static const DWORD blendMulti      = 0x00000003;	// 乗算ブレンド
	static const DWORD blendScreen     = 0x00000004;	// スクリーンブレンド
	static const DWORD blendBrighten   = 0x00000005;	// 明度(明るく)ブレンド
	static const DWORD blendDarken     = 0x00000006;	// 明度(暗く)ブレンド
	static const DWORD blendTypeMask   = 0x00000007;

	static const DWORD opacity			= 0x00000010;	// uOpacity 有効
	static const DWORD destAlpha		= 0x00000020;	// 転送先サーフェスのアルファ(も)使用
	static const DWORD color			= 0x00000040;	// nxbColor メンバ有効
	static const DWORD fill				= 0x00000080;	// 塗り潰し
	static const DWORD colorFill		= color | fill;	// 単色塗り潰し
	static const DWORD rgbaMask			= 0x00000200;	// nxbfRGBAMask 有効。単純 Blt, Fill において、マスクを使用
	static const DWORD srcAlpha			= 0x00000800;	// 転送元サーフェスのアルファを使用
	static const DWORD rule				= 0x00001000;	// rule 画像に従って転送
	static const DWORD linearFilter		= 0x00002000;	// 拡大時に linear filter を適用
	static const DWORD mirrorLeftRight	= 0x00004000;	// 左右反転
	static const DWORD mirrorTopDown	= 0x00008000;	// 上下反転
	static const DWORD constDestAlpha	= 0x00010000;	// 転送先アルファを固定(より高速な方法へ切り替えない)
	static const DWORD blurHorz			= 0x00020000;	// 水平ぼかし
	static const DWORD noRegardAlpha	= 0x00040000;	// 転送元 alpha channel 値がゼロの pixel を特殊扱いしない(srcAlpha|linearFilter 時のみ)
	static const DWORD grayscale		= 0x00080000;	// grayscale 化
	static const DWORD dynamic			= 0x80000000;	// 動的コード使用

	DWORD dwFlags;

	union
	{
		DWORD dwRGBAMask;
		struct RGBAMask
		{
			BYTE byBlue;
			BYTE byGreen;
			BYTE byRed;
			BYTE byAlpha;
		} nxbRGBAMask;
	};

	struct NxRule
	{
		UINT  uLevel;
		UINT  uVague;
		POINT ptOffset;
		const CNxSurface* pSurface;
	};

	union
	{
		NxColor	nxbColor;
		NxRule	nxbRule;
	};
	UINT uBlurRange;
	UINT uOpacity;
};




class CNxFont;
class CNxFile;
namespace NxDrawLocal
{
	class CNxCustomDraw;
	class CNxDynamicDraw;
}

#include "NxDIBImage.h"
#include "NxSprite.h"


class CNxSurface
{
public:
	CNxSurface(UINT uBitCount = 32);
	virtual ~CNxSurface();

	// 初期化
	BOOL Create(CNxFile& nxfile);
	BOOL Create(const BITMAPINFO* lpbmi, LPCVOID lpvBits = NULL);
	BOOL Create(const CNxDIBImage* pDIBImage);
	virtual BOOL Create(int nWidth, int nHeight);

	// 描画
	BOOL Blt(int dx, int dy, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt = NULL);
	BOOL Blt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt = NULL);
	BOOL TileBlt(const RECT* lpDestRect, const CNxSurface* pSurface, const RECT* lpSrcRect, int nSrcXOrg, int nSrcYOrg, NxBlt* pNxBlt = NULL);
	BOOL FilterBlt(int dx, int dy, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt);
	BOOL FilterBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt);
	BOOL FillRect(const RECT* lpRect, NxColor nxColor);
	BOOL FillRect(int dx, int dy, int cx, int cy, NxColor nxColor);
	BOOL LoadImage(int dx, int dy, CNxFile& nxfile, const RECT* lpSrcRect = NULL, BOOL bUpdateColorTable = FALSE);
	CNxFont* SetFont(CNxFont* pFont);
	CNxFont* GetFont() const;
	BOOL DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, NxColor nxColor);
	BOOL DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, const NxBlt* pNxBlt);
	BOOL GetTextExtent(LPCTSTR lpString, LPRECT lpRect);
	HDC GetDC();
	UINT ReleaseDC();
	HBITMAP GetHandle() const;
	BOOL SetTextSmoothing(BOOL bSmoothing);
	BOOL GetTextSmoothing() const;

	// 幅と高さ
	UINT GetWidth() const;
	UINT GetHeight() const;
	void GetRect(LPRECT lpRect) const;

	// サーフェスメモリ操作
	UINT GetBitCount() const;
	LONG GetPitch() const;
	LPVOID GetBits();
	LPCVOID GetBits() const;
	BOOL SetDIBits(int dx, int dy, const BITMAPINFO* lpbmi, LPCVOID lpvBits = NULL, const RECT* lpSrcRect = NULL);
	BOOL SetDIBits(int dx, int dy, const CNxDIBImage* pDIBImage, const RECT* lpSrcRect = NULL);
	BOOL GetDIBits(int dx, int dy, LPBITMAPINFO lpbmi, LPVOID lpvBits = NULL, const RECT* lpSrcRect = NULL) const;

	// カラーテーブル操作
	NxColor* GetColorTable();
	const NxColor* GetColorTable() const;
	BOOL UpdateDIBColorTable();

	CNxDIBImage& GetDIBImage() const;

	// 保存(できるだけ CNxImageSaver クラスを使用して下さい)
	BOOL SaveImage(CNxFile& nxfile, const RECT* lpRect = NULL) const;
	BOOL SaveImage(LPCTSTR lpszFileName, const RECT* lpRect = NULL) const;

	// クリッピング矩形の設定/取得
	void SetClipRect(const RECT* lpClipRect);
	void GetClipRect(LPRECT lpClipRect) const;

	// 転送先原点の設定/取得
	void SetOrg(int x, int y);
	void GetOrg(LPPOINT ptPoint) const;

	// 拡大縮小転送用情報(内部用 : 使用しないで下さい)
	struct StretchBltInfo
	{
		UINT uSrcOrgX;					// 転送元参照開始原点の小数部
		UINT uSrcOrgY;
		ULARGE_INTEGER ul64SrcDeltaX;	// 転送元参照原点変位(HighPart = 整数部, LowPart = 小数部)
		ULARGE_INTEGER ul64SrcDeltaY;
	};

	// Blt() 用クリップ (内部用です。使用しないでください)
	BOOL ClipBltRect(RECT& destRect) const;
	BOOL ClipBltRect(RECT& destRect, RECT& srcRect, const RECT& rcClipRect) const;
	BOOL ClipStretchBltRect(RECT& destRect, RECT& srcRect, const RECT& srcClipRect, StretchBltInfo& stretchBltInfo) const;

	enum { SmoothFontRatio = 4 };	// スムージング用フォントの倍率

private:

	void SetAbbreviateRect(LPRECT lpRect, const RECT* lpSrcRect) const;
	static void __cdecl FontSmoothing4x4(LPVOID lpSurface, LONG lPitch, UINT uWidth, UINT uHeight);	// 補完縮小
	BOOL SetColorTable(const CNxDIBImage* pDIBImage);

private:
	CNxDIBImage m_dibImage;
	LPBITMAPINFO m_lpbmi;
	const NxDrawLocal::CNxCustomDraw &m_customDraw;		// CNxCustomDraw クラス(const)への参照
	NxDrawLocal::CNxDynamicDraw* m_pDynamicDraw;		// CNxDynamicDraw クラスへのポインタ
	HBITMAP		 m_hBitmap;								// DIBSection のハンドル
	HDC			 m_hdcCurrent;							// 取得した HDC
	UINT		 m_udcRefCount;							// HDC の参照カウント(GetDC() で増加, ReleaseDC() で減少)
	POINT		 m_ptOrg;
	RECT		 m_rcClip;
	CNxFont*	 m_pFont;
	BOOL		 m_bTextSmoothing;

private:
	// 代入とコピー禁止
	CNxSurface(const CNxSurface& surface);
	CNxSurface& operator=(const CNxSurface&);
};

inline HBITMAP CNxSurface::GetHandle() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_hBitmap; }

inline UINT CNxSurface::GetWidth() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetWidth(); }

inline UINT CNxSurface::GetHeight() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetHeight(); }

inline UINT CNxSurface::GetBitCount() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetBitCount(); }

inline void CNxSurface::GetRect(LPRECT lpRect) const {
	_ASSERTE(lpRect != NULL);
	_ASSERTE(m_hBitmap != NULL);
	lpRect->left = 0; lpRect->top = 0; lpRect->right = GetWidth();	lpRect->bottom = GetHeight(); }

inline LONG CNxSurface::GetPitch() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetPitch(); }

inline LPVOID CNxSurface::GetBits() {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetBits(); }

inline LPCVOID CNxSurface::GetBits() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetBits(); }

inline void CNxSurface::GetClipRect(LPRECT lpClipRect) const {
	_ASSERTE(lpClipRect != NULL);
	_ASSERTE(m_hBitmap != NULL);
	*lpClipRect = m_rcClip; }

inline void CNxSurface::SetOrg(int x, int y) {
	m_ptOrg.x = x; m_ptOrg.y = y; }

inline void CNxSurface::GetOrg(LPPOINT ptPoint) const {
	*ptPoint = m_ptOrg; }

inline BOOL CNxSurface::GetTextSmoothing() const {
	return m_bTextSmoothing; }

inline CNxFont* CNxSurface::GetFont() const {
	return m_pFont; }

inline NxColor* CNxSurface::GetColorTable() {
	return reinterpret_cast<NxColor*>(m_dibImage.GetColorTable()); }

inline const NxColor* CNxSurface::GetColorTable() const {
	return reinterpret_cast<const NxColor*>(m_dibImage.GetColorTable()); }

inline CNxDIBImage& CNxSurface::GetDIBImage() const {
	return (CNxDIBImage&)m_dibImage; }
