// NxAlphaBlend.h
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: ブレンド用のインライン関数をまとめたクラス
//
// 内部用なのでコメントは省略
//
#pragma once
#include "NxDrawLocal.h"

using namespace NxDrawLocal;

namespace NxDrawLocal
{

namespace NxAlphaBlend
{
	// ブレンド関数のプロトタイプ
	typedef DWORD (*BlendFunc)(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[]);

	// 直書き用
	inline DWORD Direct(DWORD /*dwDest*/, DWORD dwSrc, UINT /*uOpacity*/, const DWORDLONG /*dwlAlphaTransformTable*/[])
	{
		return dwSrc;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// DWORD CNxAlphaBlend::Normal(DWORD dwDest, DWORD dwSrc, UINT uOpacity)
	// 概要: 通常ブレンド
	// 引数: DWORD dwDest   ... 転送先ピクセル
	//       DWORD dwSrc    ... 転送元ピクセル
    //       UINT  uOpacity ... 転送元不透明度(0 ～ 255)
	// 戻値: 演算結果 (alpha = 0)
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Special thanks to H.Niisaka.

	inline DWORD Normal(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[])
	{
		// 0 ～ 255 を、0, 2 ～ 256 へ変換
		uOpacity = static_cast<UINT>(dwlAlphaTransformTable[uOpacity] & 0xffff);
		
		DWORD dwSrcBR, dwSrcG;
		DWORD dwDestBR, dwDestG;

		dwSrcBR = dwSrc & 0x00ff00ff;
		dwSrcG = dwSrc & 0x0000ff00;
		dwDestBR = dwDest & 0x00ff00ff;
		dwDestG = dwDest & 0x0000ff00;
		dwDestBR = ((((dwSrcBR - dwDestBR) * uOpacity) >> 8) + dwDestBR) & 0x00ff00ff;
		dwDestG = ((((dwSrcG - dwDestG) * uOpacity) >> 8) + dwDestG) & 0x0000ff00;

		return dwDestBR | dwDestG;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// DWORD CNxAlphaBlend::Add(DWORD dwDest, DWORD dwSrc, UINT uOpacity)
	// 概要: 加算ブレンド
	// 引数: DWORD dwDest   ... 転送先ピクセル
	//       DWORD dwSrc    ... 転送元ピクセル
    //       UINT  uOpacity ... 転送元不透明度(0 ～ 255)
	// 戻値: 演算結果 (alpha = 0)
	///////////////////////////////////////////////////////////////////////////////////////////////////

	inline DWORD Add(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[])
	{
		// 0 ～ 255 を、0, 2 ～ 256 へ変換
		uOpacity = static_cast<UINT>(dwlAlphaTransformTable[uOpacity] & 0xffff);
	
		DWORD dwSrcBR, dwSrcG;
		DWORD dwDestBR, dwDestG;
		
		dwSrcBR = ((dwSrc & 0x00ff00ff) * uOpacity) >> 8;
		dwSrcG = ((dwSrc & 0x0000ff00) * uOpacity) >> 16;
		dwDestBR = dwDest & 0x00ff00ff;
		dwDestG = (dwDest & 0x0000ff00) >> 8;
		dwDestBR = ConstTable::byRangeLimitTable[(dwDestBR & 0xff) + (dwSrcBR & 0xff) + ConstTable::RangeLimitCenter]
			| (ConstTable::byRangeLimitTable[(dwDestBR >> 16) + (dwSrcBR >> 16) + ConstTable::RangeLimitCenter] << 16);
		dwDestG = ConstTable::byRangeLimitTable[dwSrcG + dwDestG + ConstTable::RangeLimitCenter] << 8;

		return dwDestBR | dwDestG;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// DWORD CNxAlphaBlend::Sub(DWORD dwDest, DWORD dwSrc, UINT uOpacity)
	// 概要: 減算ブレンド (dest = dest - ((src * uOpacity) / 256)
	// 引数: DWORD dwDest   ... 転送先ピクセル
	//       DWORD dwSrc    ... 転送元ピクセル
    //       UINT  uOpacity ... 転送元不透明度(0 ～ 255)
	// 戻値: 演算結果 (alpha = 0)
	///////////////////////////////////////////////////////////////////////////////////////////////////

	inline DWORD Sub(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[])
	{
		// 0 ～ 255 を、0, 2 ～ 256 へ変換
		uOpacity = static_cast<UINT>(dwlAlphaTransformTable[uOpacity] & 0xffff);

		DWORD dwSrcBR, dwSrcG;
		DWORD dwDestBR, dwDestG;
		
		dwSrcBR = ((dwSrc & 0x00ff00ff) * uOpacity) >> 8;
		dwSrcG = ((dwSrc & 0x0000ff00) * uOpacity) >> 16;
		dwDestBR = (dwDest & 0x00ff00ff);
		dwDestG = (dwDest & 0x0000ff00) >> 8;
		dwDestBR = ConstTable::byRangeLimitTable[(dwDestBR & 0xff) - (dwSrcBR & 0xff) + ConstTable::RangeLimitCenter]
			| (ConstTable::byRangeLimitTable[(dwDestBR >> 16) - (dwSrcBR >> 16) + ConstTable::RangeLimitCenter] << 16);
		dwDestG = ConstTable::byRangeLimitTable[dwDestG - dwSrcG + ConstTable::RangeLimitCenter] << 8;

		return dwDestBR | dwDestG;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// DWORD CNxAlphaBlend::Multi(DWORD dwDest, DWORD dwSrc, UINT uOpacity)
	// 概要: 乗算ブレンド
	// 引数: DWORD dwDest   ... 転送先ピクセル
	//       DWORD dwSrc    ... 転送元ピクセル
    //       UINT  uOpacity ... 転送元不透明度(0 ～ 255)
	// 戻値: 演算結果 (alpha = 0)
	///////////////////////////////////////////////////////////////////////////////////////////////////

	inline DWORD Multi(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[])
	{
		// 0 ～ 255 を、0, 2 ～ 256 へ変換
		uOpacity = static_cast<UINT>(dwlAlphaTransformTable[uOpacity] & 0xffff);

		DWORD dwSrcBR, dwSrcG;
		DWORD dwDestBR, dwDestG;

		dwDestG = dwDest & 0x0000ff00;
		dwDestBR = dwDest & 0x00ff00ff;
		dwSrcG = static_cast<DWORD>(ConstTable::bySrcAlphaToOpacityTable[(dwSrc >> 8) & 0xff][dwDestG >> 8]) << 8;
		dwSrcBR = static_cast<DWORD>(ConstTable::bySrcAlphaToOpacityTable[(dwSrc >> 16) & 0xff][dwDestBR >> 16]) << 16;
		dwSrcBR |= static_cast<DWORD>(ConstTable::bySrcAlphaToOpacityTable[dwSrc & 0xff][dwDestBR & 0xff]);

		dwDestBR = ((((dwSrcBR - dwDestBR) * uOpacity) >> 8) + dwDestBR) & 0x00ff00ff;
		dwDestG = ((((dwSrcG - dwDestG) * uOpacity) >> 8) + dwDestG) & 0x0000ff00;

		return dwDestBR | dwDestG;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// DWORD CNxAlphaBlend::Screen(DWORD dwDest, DWORD dwSrc, UINT uOpacity)
	// 概要: スクリーンブレンド
	// 引数: DWORD dwDest   ... 転送先ピクセル
	//       DWORD dwSrc    ... 転送元ピクセル
    //       UINT  uOpacity ... 転送元不透明度(0 ～ 255)
	// 戻値: 演算結果 (alpha = 0)
	///////////////////////////////////////////////////////////////////////////////////////////////////

	inline DWORD Screen(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[])
	{
		// 0 ～ 255 を、0, 2 ～ 256 へ変換
		uOpacity = static_cast<UINT>(dwlAlphaTransformTable[uOpacity] & 0xffff);

		DWORD dwSrcBR, dwSrcG;
		DWORD dwDestBR, dwDestG;

		dwDestBR = (dwDest & 0x00ff00ff) ^ 0x00ff00ff;
		dwDestG = (dwDest & 0x0000ff00) ^ 0x0000ff00;
		
		dwSrcBR = static_cast<DWORD>(ConstTable::bySrcAlphaToOpacityTable[((dwSrc >> 16) & 0xff) ^ 0xff][dwDestBR >> 16]) << 16;
		dwSrcBR |= static_cast<DWORD>(ConstTable::bySrcAlphaToOpacityTable[(dwSrc & 0xff) ^ 0xff][dwDestBR & 0xff]);
		dwSrcBR = dwSrcBR ^ 0x00ff00ff;
		dwSrcG = (static_cast<DWORD>(ConstTable::bySrcAlphaToOpacityTable[((dwSrc >> 8) & 0xff) ^ 0xff][dwDestG >> 8]) ^ 0xff) << 8;

		dwDestBR = dwDestBR ^ 0x00ff00ff;
		dwDestG = dwDestG ^ 0x0000ff00;

		dwDestBR = ((((dwSrcBR - dwDestBR) * uOpacity) >> 8) + dwDestBR) & 0x00ff00ff;
		dwDestG = ((((dwSrcG - dwDestG) * uOpacity) >> 8) + dwDestG) & 0x0000ff00;

		return dwDestBR | dwDestG;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// DWORD CNxAlphaBlend::Brighten(DWORD dwDest, DWORD dwSrc, UINT uOpacity)
	// 概要: 明度(明るく)ブレンド
	// 引数: DWORD dwDest   ... 転送先ピクセル
	//       DWORD dwSrc    ... 転送元ピクセル
    //       UINT  uOpacity ... 転送元不透明度(0 ～ 255)
	// 戻値: 演算結果 (alpha = 0)
	///////////////////////////////////////////////////////////////////////////////////////////////////

	inline DWORD Brighten(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[])
	{
		// 0 ～ 255 を、0, 2 ～ 256 へ変換
		uOpacity = static_cast<UINT>(dwlAlphaTransformTable[uOpacity] & 0xffff);

		DWORD dwSrcBR, dwSrcG;
		DWORD dwDestBR, dwDestG;

		dwDestG = dwDest & 0x0000ff00;
		dwDestBR = dwDest & 0x00ff00ff;
		
		dwSrcG =   ((dwSrc & 0x0000ff00) < dwDestG) ? dwDestG : (dwSrc & 0x0000ff00);
		dwSrcBR =  ((dwSrc & 0x000000ff) < (dwDestBR & 0x000000ff)) ? (dwDestBR & 0x000000ff) : (dwSrc & 0x000000ff);
		dwSrcBR |= ((dwSrc & 0x00ff0000) < (dwDestBR & 0x00ff0000)) ? (dwDestBR & 0x00ff0000) : (dwSrc & 0x00ff0000);

		dwDestBR = ((((dwSrcBR - dwDestBR) * uOpacity) >> 8) + dwDestBR) & 0x00ff00ff;
		dwDestG = ((((dwSrcG - dwDestG) * uOpacity) >> 8) + dwDestG) & 0x0000ff00;

		return dwDestBR | dwDestG;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// DWORD CNxAlphaBlend::Darkgen(DWORD dwDest, DWORD dwSrc, UINT uOpacity)
	// 概要: 明度(暗く)ブレンド
	// 引数: DWORD dwDest   ... 転送先ピクセル
	//       DWORD dwSrc    ... 転送元ピクセル
    //       UINT  uOpacity ... 転送元不透明度(0 ～ 255)
	// 戻値: 演算結果 (alpha = 0)
	///////////////////////////////////////////////////////////////////////////////////////////////////

	inline DWORD Darken(DWORD dwDest, DWORD dwSrc, UINT uOpacity, const DWORDLONG dwlAlphaTransformTable[])
	{
		// 0 ～ 255 を、0, 2 ～ 256 へ変換
		uOpacity = static_cast<UINT>(dwlAlphaTransformTable[uOpacity] & 0xffff);

		DWORD dwSrcBR, dwSrcG;
		DWORD dwDestBR, dwDestG;

		dwDestG = dwDest & 0x0000ff00;
		dwDestBR = dwDest & 0x00ff00ff;
		
		dwSrcG =   ((dwSrc & 0x0000ff00) > dwDestG) ? dwDestG : (dwSrc & 0x0000ff00);
		dwSrcBR =  ((dwSrc & 0x000000ff) > (dwDestBR & 0x000000ff)) ? (dwDestBR & 0x000000ff) : (dwSrc & 0x000000ff);
		dwSrcBR |= ((dwSrc & 0x00ff0000) > (dwDestBR & 0x00ff0000)) ? (dwDestBR & 0x00ff0000) : (dwSrc & 0x00ff0000);

		dwDestBR = ((((dwSrcBR - dwDestBR) * uOpacity) >> 8) + dwDestBR) & 0x00ff00ff;
		dwDestG = ((((dwSrcG - dwDestG) * uOpacity) >> 8) + dwDestG) & 0x0000ff00;

		return dwDestBR | dwDestG;
	}

	//
	// Blt 用
	// 

	inline DWORD Blt(BlendFunc pfnBlend, DWORD dwDest, DWORD dwSrc, UINT uOpacity,
					 const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		return (pfnBlend)(dwDest, dwSrc, uOpacity, dwlAlphaTransformTable) | (dwDest & 0xff000000);
	}

	inline DWORD Blt_SrcAlpha(BlendFunc pfnBlend, DWORD dwDest, DWORD dwSrc, const BYTE bySrcAlphaToOpacityTable[256],
							  const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		UINT uOpacity = bySrcAlphaToOpacityTable[(dwSrc >> 24)];
		return (pfnBlend)(dwDest, dwSrc, uOpacity, dwlAlphaTransformTable) | (dwDest & 0xff000000);
	}

	inline DWORD Blt_DestAlpha(BlendFunc pfnBlend, const BYTE /*byDestAndSrcAlphaTable*/[256][256],
							   DWORD dwDest, DWORD dwSrc, UINT uSrcOpacity,
							   const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		BYTE byDestOpacity = static_cast<BYTE>(dwDest >> 24);
		dwDest = (pfnBlend)(dwDest, dwSrc, ConstTable::byDestAndSrcOpacityTable[byDestOpacity][uSrcOpacity & 0xff],
							dwlAlphaTransformTable);
		return dwDest | (static_cast<DWORD>(ConstTable::byDestAlphaResultTable[byDestOpacity][uSrcOpacity]) << 24);
	}

	inline DWORD Blt_DestSrcAlpha(BlendFunc pfnBlend, const BYTE /*byDestAndSrcAlphaTable*/[256][256], DWORD dwDest,
								 DWORD dwSrc, const BYTE bySrcAlphaToOpacityTable[256],
								 const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		BYTE bySrcOpacity = bySrcAlphaToOpacityTable[dwSrc >> 24];
		BYTE byDestOpacity = static_cast<BYTE>(dwDest >> 24);
		dwDest = (pfnBlend)(dwDest, dwSrc, ConstTable::byDestAndSrcOpacityTable[byDestOpacity][bySrcOpacity],
							dwlAlphaTransformTable) & 0x00ffffff;
		return dwDest | (static_cast<DWORD>(ConstTable::byDestAlphaResultTable[byDestOpacity][bySrcOpacity]) << 24);
	}

	//
	// 塗り潰し用
	//

	inline DWORD ColorFill(BlendFunc pfnBlend, DWORD dwDest, DWORD dwColor, UINT uOpacity,
						   const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		return (pfnBlend)(dwDest, dwColor, uOpacity, dwlAlphaTransformTable) | (dwDest & 0xff000000);
	}

	inline DWORD ColorFill_SrcAlpha(BlendFunc pfnBlend, DWORD dwDest, BYTE byAlpha,
								   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256],
								   const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		UINT uOpacity = bySrcAlphaToOpacityTable[byAlpha];
		return (pfnBlend)(dwDest, dwColor, uOpacity, dwlAlphaTransformTable) | (dwDest & 0xff000000);
	}

	inline DWORD ColorFill_DestAlpha(BlendFunc pfnBlend, const BYTE /*byDestAndSrcAlphaTable*/[256][256],
									 DWORD dwDest, DWORD dwColor, UINT uSrcOpacity,
									 const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		BYTE byDestOpacity = static_cast<BYTE>(dwDest >> 24);
		dwDest = (pfnBlend)(dwDest, dwColor, ConstTable::byDestAndSrcOpacityTable[byDestOpacity][uSrcOpacity & 0xff],
							dwlAlphaTransformTable) & 0x00ffffff;
		return dwDest | (static_cast<DWORD>(ConstTable::byDestAlphaResultTable[byDestOpacity][uSrcOpacity & 0xff]) << 24);
	}

	inline DWORD ColorFill_DestSrcAlpha(BlendFunc pfnBlend, const BYTE /*byDestAndSrcAlphaTable*/[256][256], DWORD dwDest,
									    BYTE byAlpha, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256],
										const DWORDLONG dwlAlphaTransformTable[] = ConstTable::dwlMMXAlphaMultiplierTable)
	{
		BYTE bySrcOpacity = bySrcAlphaToOpacityTable[byAlpha];
		BYTE byDestOpacity = static_cast<BYTE>(dwDest >> 24);
		dwDest = (pfnBlend)(dwDest, dwColor, ConstTable::byDestAndSrcOpacityTable[byDestOpacity][bySrcOpacity],
							dwlAlphaTransformTable) & 0x00ffffff;
		return dwDest | (static_cast<DWORD>(ConstTable::byDestAlphaResultTable[byDestOpacity][bySrcOpacity]) << 24);
	}
}

}