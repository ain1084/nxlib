#include <NxDraw.h>
#include <math.h>
#include "NxDrawLocal.h"

namespace NxDrawLocal
{

namespace ConstTable
{
	BYTE byRangeLimitTable[RangeLimitCenter * 2 + 256];
	DWORD dwByteToDwordTable[256];							// 1024 bytes
	BYTE bySrcAlphaToOpacityTable[256][256];				// 65536 bytes
	BYTE byDestAndSrcOpacityTable[256][256];				// 65536 bytes
	BYTE byDestAlphaResultTable[256][256];					// 65536 bytes
	DWORDLONG dwlMMXAlphaMultiplierTable[256];				// 2048 bytes
	DWORDLONG dwlMMXNoRegardAlphaMultiplierTable[256];		// 2048 bytes
}
	
void CreateTableDynamic()
{
	using namespace ConstTable;
	UINT u;
	UINT v;
	DWORDLONG dwl;

	// dwByteToDwordTable[256]
	// 同じ BYTE を4個並べた DWORD 変換テーブル
	for (v = 0x00000000, u = 0; u < 256; u++, v += 0x01010101)
		dwByteToDwordTable[u] = v;

	// byRangeLimitTable[RangeLimitCenter * 2 + 256]
	// 飽和演算用テーブル(加算、減算ブレンド等で使用)
	for (u = 0; u < RangeLimitCenter; u++)
		byRangeLimitTable[u] = 0;
	for (u = 0; u < 256; u++)
		byRangeLimitTable[u + RangeLimitCenter] = static_cast<BYTE>(u);
	for (u = 0; u < RangeLimitCenter; u++)
		byRangeLimitTable[u + RangeLimitCenter + 256] = 255;

	// dwlMMXAlphaMultiplierTable[256]
	// MMX 用アルファ値乗算値テーブル Alpha = 0
	// 最大を 256 倍とする為 index = 1 から + 1 している
	dwlMMXAlphaMultiplierTable[0] = 0;
	for (dwl = 0x0000000200020002, u = 1; u < 256; u++, dwl += 0x0000000100010001)
		dwlMMXAlphaMultiplierTable[u] = dwl;

	// dwlMMXNoRegardAlphaMultiplierTable[256]
	// MMX 用アルファ値乗算値テーブル Alpha = 0
	// dwlMMXAlphaMultiplierTable と似ているが、index = 2 以下はゼロ
	for (u = 0; u < 3; u++)
		dwlMMXNoRegardAlphaMultiplierTable[u] = 0;
	for (dwl = 0x0000000400040004, u = 3; u < 256; u++, dwl += 0x0000000100010001)
		dwlMMXNoRegardAlphaMultiplierTable[u] = dwl;
		
	// bySrcAlphaToOpacityTable[256][256]
	// アルファ変換テーブル
	// 転送元のアルファ値(0 ～ 255)と不透明度(0 ～ 255)から、
	// 最終的な Alpha (0 ～ 255) を得る
	// また、8bit * 8bit / 255 の演算結果としても使用(ただし、切り上げ有り)
	for (u = 0; u < 256; u++)
	{
		for (v = 0; v < 256; v++)
		{
			bySrcAlphaToOpacityTable[u][v] = static_cast<BYTE>((u * v + 127) / 255);
		}
	}

	// byDestAndSrcOpacityTable[256][256]
	// 転送元と転送先の不透明度から真の透明度を求める為のテーブル
	// (転送先アルファを考慮した重ね合わせに使用)
	//
	// 吉里吉里 source ver.0.80 by W.Dee 氏 (http://www.din.or.jp/~glit/TheOddStage/TVP/)
	// Projects\TVP32\OperatorsUnit.cpp を参考にしました
	for (v = 0; v < 256; v++)
		byDestAndSrcOpacityTable[0][v] = 255;
	
	for (u = 1; u < 256; u++)
	{
		for (v = 0; v < 256; v++)
		{
			double c = (static_cast<double>(v) / 255.0f) / (static_cast<double>(u) / 255.0f);
			c = c / (1.0 - (static_cast<double>(v) / 255.0f) + c);
			byDestAndSrcOpacityTable[u][v] = static_cast<BYTE>(min(static_cast<int>(c * 255.0f), 255));
		}
	}

	// byDestAlphaResultTable[256][256]
	// 転送先アルファを考慮したブレンド時に、転送先アルファチャンネルを置き換える値
	// [dest][src] で参照
	for (u = 0; u < 256; u++)
	{
		for (v = 0; v < 256; v++)
		{
			byDestAlphaResultTable[u][v] = static_cast<BYTE>(min(u + (255 - u) * v / 255, 255));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL ClipRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpDestClipRect, const RECT* lpSrcClipRect)
// 概要: 汎用矩形クリップ関数
// 引数: LPRECT lpDestRect ... 転送先矩形を示す RECT 構造体へのポインタ
//       LPRECT lpSrcRect  ... 転送元矩形を示す RECT 構造体へのポインタ
//		 const RECT* lpDestClipRect ... 転送先のクリップ矩形(NULL = クリップしない)
//       const RECT* lpSrcClipRect  ... 転送元のクリップ矩形(NULL = クリップしない)
// 戻値: lpDestRect で示される矩形が空でなければ TRUE, それ以外は FALSE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL ClipRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpDestClipRect, const RECT* lpSrcClipRect)
{
	// 小さい方へ合わせる
	int nHeight = min(lpSrcRect->bottom - lpSrcRect->top, lpDestRect->bottom - lpDestRect->top);
	lpSrcRect->bottom = lpSrcRect->top + nHeight;
	lpDestRect->bottom = lpDestRect->top + nHeight;

	if (lpDestClipRect != NULL)
	{
		if (lpDestRect->top < lpDestClipRect->top)
		{
			lpSrcRect->top += lpDestClipRect->top - lpDestRect->top;
			lpDestRect->top = lpDestClipRect->top;
		}
		if (lpDestClipRect->bottom - lpDestRect->bottom < 0)
		{
			lpSrcRect->bottom += lpDestClipRect->bottom - lpDestRect->bottom;
			lpDestRect->bottom = lpDestClipRect->bottom;
		}
	}
	if (lpSrcClipRect != NULL)
	{
		if (lpSrcRect->top < lpSrcClipRect->top)
		{
			lpDestRect->top += lpSrcClipRect->top - lpSrcRect->top;
			lpSrcRect->top = lpSrcClipRect->top;
		}
		if (lpSrcClipRect->bottom - lpSrcRect->bottom < 0)
		{
			lpDestRect->bottom += lpSrcClipRect->bottom - lpSrcRect->bottom;
			lpSrcRect->bottom = lpSrcClipRect->bottom;
		}
	}
	if (lpDestClipRect->top >= lpDestRect->bottom)
		return FALSE;


	int nWidth = min(lpSrcRect->right - lpSrcRect->left, lpDestRect->right - lpDestRect->left);
	lpSrcRect->right = lpSrcRect->left + nWidth;
	lpDestRect->right = lpDestRect->left + nWidth;

	if (lpDestClipRect != NULL)
	{
		if (lpDestRect->left < lpDestClipRect->left)
		{
			lpSrcRect->left += lpDestClipRect->left - lpDestRect->left;
			lpDestRect->left = lpDestClipRect->left;
		}
		if (lpDestClipRect->right - lpDestRect->right < 0)
		{
			lpSrcRect->right += lpDestClipRect->right - lpDestRect->right;
			lpDestRect->right = lpDestClipRect->right;
		}
	}
	if (lpSrcClipRect != NULL)
	{
		if (lpSrcRect->left < lpSrcClipRect->left)
		{
			lpDestRect->left += lpSrcClipRect->left - lpSrcRect->left;
			lpSrcRect->left = lpSrcClipRect->left;
		}
		if (lpSrcClipRect->right - lpSrcRect->right < 0)
		{
			lpDestRect->right += lpSrcClipRect->right - lpSrcRect->right;
			lpSrcRect->right = lpSrcClipRect->right;
		}
	}
	if (lpDestClipRect->left >= lpDestRect->right)
		return FALSE;
	else
		return TRUE;
}

}