// NxDynamicDraw32.cpp: CNxDynamicDraw32 クラスのインプリメンテーション
// Copyright(c) 2000, 2001 S.Ainoguchi / Y.Ojima
//
// 概要: 動的コードによるサーフェスメモリへの直接描画(32bpp 専用)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxDynamicDraw32.h"
#include "NxDrawLocal.h"
#include "NxSurface.h"

using namespace NxDrawLocal;

CNxDynamicDraw32::CNxDynamicDraw32()
{
	// 実行コード未生成としてマーク
	// (NxBlt.dwFlags == 0 で呼び出される事はないため)
	m_previous.dwFlags = 0;
}

//////////////////////////////////////////////////////////////////////
// コード埋め込み用マクロ
//////////////////////////////////////////////////////////////////////

#define BLOCK_START(n)									\
	__asm { jmp CodeEnd##n }							\
	CodeStart##n:										\
	__asm {

#define BLOCK_END(n)									\
	}													\
	CodeEnd##n:											\
	__asm { mov CodeStart, offset CodeStart##n }		\
	__asm { mov CodeEnd, offset CodeEnd##n }			\
	memcpy(Current, CodeStart, CodeEnd - CodeStart);	\
	Current += CodeEnd - CodeStart;

#define CODE_JZ(c,a)	GenerateFlagJump(c, a, 0x74, 0x0F, 0x84);
#define CODE_JNZ(c,a)	GenerateFlagJump(c, a, 0x75, 0x0F, 0x85);


//////////////////////////////////////////////////////////////////////
// 条件ジャンプコードの生成
//////////////////////////////////////////////////////////////////////

void CNxDynamicDraw32::GenerateFlagJump
(LPBYTE& lpCurrent,		// 現在のコードへのポインタへの参照
 LPBYTE lpJmpAddress,	// ジャンプ先へのポインタ
 BYTE byShortJmpCode,	// Short Jmp のバイトコード
 BYTE byNearJmpCode1,	// Near Jmp のバイトコード 1
 BYTE byNearJmpCode2)	// Near Jmp のバイトコード 2
{
	// ジャンプ先までの距離を求める
	DWORD dwDistance = lpJmpAddress - lpCurrent - 2;

	// 127バイト以内の場合	
	if (dwDistance >= -127 && dwDistance <= 127)
	{
		// Short Jmp を生成
		*lpCurrent++ = byShortJmpCode;
		*lpCurrent++ = (BYTE)dwDistance;
	}
	// 128バイト以上の場合	
	else
	{
		// Near Jmp を生成
		*lpCurrent++ = byNearJmpCode1;
		*lpCurrent++ = byNearJmpCode2;
		*((LPDWORD&)lpCurrent)++ = dwDistance - 4;
	}
}

//////////////////////////////////////////////////////////////////////
// 1ピクセル転送ルーチンの生成
//////////////////////////////////////////////////////////////////////

// 「emms 命令が無い」警告
#pragma warning (disable : 4799)
// 「インライン asm はグローバルな最適化を抑制します」警告
#pragma warning (disable : 4740)

void CNxDynamicDraw32::GenerateTransPixel(LPBYTE& Current, DWORD dwFlags)
{
	using namespace ConstTable;
//	using CNxDynamicDraw32::BltStackFrame;

	LPBYTE CodeStart;
	LPBYTE CodeEnd;

	// in : eax = src opacity (0 - 255)
	//      mm2 = src pixel (00xx00rr 00gg00bb)

	// Grayscale
	if (dwFlags & NxBlt::grayscale)
	{
		static const DWORDLONG dwlGrayMultiplier = (static_cast<DWORDLONG>(77) << 32) | (static_cast<DWORDLONG>(150) << 16) | 29;

		BLOCK_START(Pixel_Grayscale)
			pmullw		mm2, [dwlGrayMultiplier]
			movq		mm7, mm2
			psrlq		mm2, 16
			paddw		mm7, mm2
			psrlq		mm2, 16
			paddw		mm2, mm7
			psrlq		mm2, 8
			punpcklbw	mm2, mm2
			punpcklwd	mm2, mm2
			punpckldq	mm2, mm2
		BLOCK_END(Pixel_Grayscale)
	}
	// Color あり
	if (dwFlags & NxBlt::color)
	{
		BLOCK_START(Pixel_Color)
			movd		mm6, eax
			movzx		eax, byte ptr [esp]BltStackFrame.nxbColor + 3
			movd		mm4, [esp]BltStackFrame.nxbColor
			punpcklbw	mm4, mm0
			psubw		mm4, mm2
			pmullw		mm4, [dwlMMXAlphaMultiplierTable + eax * 8]
			movd		eax, mm6
			psrlw		mm4, 8
			paddb		mm4, mm2
			packuswb	mm4, mm0
			movq		mm2, mm4
			punpcklbw	mm2, mm0
		BLOCK_END(Pixel_Color)
	}
	switch (dwFlags & NxBlt::blendTypeMask)
	{
	case NxBlt::blendNormal:
		// 通常ブレンド
		// DestAlphaあり
		if (dwFlags & NxBlt::destAlpha)
		{
			BLOCK_START(Pixel_Normal_DestAlpha)
				movd		mm4, [edi]
				movd		mm1, ebx
				mov			ebx, [esp]BltStackFrame.lpbSrcAlphaToOpacityTable
				mov			al, byte ptr [ebx + eax]
				punpcklbw	mm4, mm0
				mov			ah, byte ptr [edi + 3]
				movzx		ebx, byte ptr [byDestAndSrcOpacityTable + eax]
				psubw		mm2, mm4
				movzx		eax, byte ptr [byDestAlphaResultTable + eax]
				pmullw		mm2, [dwlMMXAlphaMultiplierTable + ebx * 8]
				movd		ebx, mm1
				psrlw		mm2, 8
				shl			eax, 24
				paddb		mm4, mm2
				movd		mm6, eax
				packuswb	mm4, mm0
				pand		mm4, [dwlConst_00FF_FFFF_00FF_FFFF]
				por			mm4, mm6
			BLOCK_END(Pixel_Normal_DestAlpha)
		}
		// DestAlphaなし
		else
		{
			BLOCK_START(Pixel_Normal_NoDestAlpha)
				movd		mm4, [edi]
				punpcklbw	mm4, mm0
				psubw		mm2, mm4
				pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
				psrlw		mm2, 8
				paddb		mm4, mm2
				packuswb	mm4, mm0
			BLOCK_END(Pixel_Normal_NoDestAlpha)
		}
		break;
	case NxBlt::blendAdd:
		// 加算ブレンド
		BLOCK_START(Pixel_Add)
			pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
			movd		mm4, [edi]
			psrlw		mm2, 8
			packuswb	mm2, mm0
			paddusb		mm4, mm2
		BLOCK_END(Pixel_Add)
		break;
	case NxBlt::blendSub:
		// 減算ブレンド
		BLOCK_START(Pixel_Sub)
			pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
			movd		mm4, [edi]
			psrlw		mm2, 8
			packuswb	mm2, mm0
			psubusb		mm4, mm2
		BLOCK_END(Pixel_Sub)
		break;
	case NxBlt::blendMulti:
		// 乗算ブレンド
		BLOCK_START(Pixel_Multi)
			movd		mm4, [edi]
			punpcklbw	mm4, mm0
			pmullw		mm2, mm4
			psrlw		mm2, 8
			pmullw		mm2, [dwlConst_8081_8081_8081_8081]
			psrlw		mm2, 7
			pand		mm2, [dwlConst_00FF_00FF_00FF_00FF]
			psubw		mm2, mm4
			pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
			psrlw		mm2, 8
			paddb		mm4, mm2
			packuswb	mm4, mm0
		BLOCK_END(Pixel_Multi)
		break;
	case NxBlt::blendScreen:
		// スクリーンブレンド
		BLOCK_START(Pixel_Screen)
			movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
			movd		mm4, [edi]
			punpcklbw	mm4, mm0
			pxor		mm2, mm7
			pxor		mm4, mm7
			pmullw		mm2, mm4
			pxor		mm4, mm7
			psrlw		mm2, 8
			pmullw		mm2, [dwlConst_8081_8081_8081_8081]
			psrlw		mm2, 7
			pandn		mm2, mm7
			psubw		mm2, mm4
			pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
			psrlw		mm2, 8
			paddb		mm4, mm2
			packuswb	mm4, mm0
		BLOCK_END(Pixel_Screen)
		break;
	case NxBlt::blendBrighten:
		// 明度比較(明)ブレンド
		BLOCK_START(Pixel_Brighten)
			movd		mm4, [edi]
			punpcklbw	mm4, mm0
			movq		mm6, mm2
			pcmpgtw		mm2, mm4
			pand		mm6, mm2
			pandn		mm2, mm4
			por			mm2, mm6
			psubw		mm2, mm4
			pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
			psrlw		mm2, 8
			paddb		mm4, mm2
			packuswb	mm4, mm0
		BLOCK_END(Pixel_Brighten)
		break;
	case NxBlt::blendDarken:
		// 明度比較(暗)ブレンド
		BLOCK_START(Pixel_Darken)
			movd		mm4, [edi]
			punpcklbw	mm4, mm0
			movq		mm6, mm4
			pcmpgtw		mm6, mm2
			pand		mm2, mm6
			pandn		mm6, mm4
			por			mm2, mm6
			psubw		mm2, mm4
			pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
			psrlw		mm2, 8
			paddb		mm4, mm2
			packuswb	mm4, mm0
		BLOCK_END(Pixel_Darken)
		break;
	}

	// RGBAMaskあり
	if (dwFlags & NxBlt::rgbaMask)
	{
		BLOCK_START(Pixel_Put_RGBAMask)
			movd		mm3, [esp]BltStackFrame.dwMask
			movd		mm6, [edi]
			pand		mm4, mm3
			pandn		mm3, mm6
			por			mm4, mm3
			movd		[edi], mm4
		BLOCK_END(Pixel_Put_RGBAMask)
	}
	// RGBAMaskなし
	else
	{
		BLOCK_START(Pixel_Put_NoRGBAMask)
			movd		[edi], mm4
		BLOCK_END(Pixel_Put_NoRGBAMask)
	}
}
#pragma warning (default : 4799)

//////////////////////////////////////////////////////////////////////
// ビットブロック転送 (拡大縮小なし)
//////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4731)

// 「インライン asm はグローバルな最適化を抑制します」警告
#pragma warning (disable : 4740)
void CNxDynamicDraw32::GenerateNormalBlt(LPBYTE Current, DWORD dwFlags, BOOL bMirrorLeftRight)
{
	using namespace ConstTable;

	LPBYTE CodeStart;
	LPBYTE CodeEnd;

	// 初期化コード
	BLOCK_START(Initialize)
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			esi, [esp]BltStackFrame.lpSrcSurface
		mov			edi, [esp]BltStackFrame.lpDestSurface
		mov			edx, [esp]BltStackFrame.lpbSrcAlphaToOpacityTable
	BLOCK_END(Initialize)

	// Y軸ループ開始
	LPBYTE LoopY = Current;
	{
		// X軸ループ前処理
		BLOCK_START(BeforeLoopX)
			mov			ecx, [esp]BltStackFrame.uWidth
		BLOCK_END(BeforeLoopX)

		// X軸ループ開始
		LPBYTE LoopX = Current;
		{
			// SrcAlphaあり
			if (dwFlags & NxBlt::srcAlpha)
			{
				BLOCK_START(BeforeTransPixel_SrcAlpha)
					movd		mm2, [esi]
					movzx		eax, byte ptr [esi + 3]
					punpcklbw	mm2, mm0
				BLOCK_END(BeforeTransPixel_SrcAlpha)

				// DestAlphaなし
				if (!(dwFlags & NxBlt::destAlpha))
				{
					BLOCK_START(BeforeTransPixel_SrcAlphaNoDestAlpha)
						movzx		eax, byte ptr [edx + eax]
					BLOCK_END(BeforeTransPixel_SrcAlphaNoDestAlpha)
				}
			}
			// SrcAlphaなし
			else
			{
				BLOCK_START(BeforeTransPixel_NoSrcAlpha)
					movd		mm2, [esi]
					movzx		eax, [esp]BltStackFrame.uOpacity
					punpcklbw	mm2, mm0
				BLOCK_END(BeforeTransPixel_NoSrcAlpha)
			}

			// 1ピクセル転送
			GenerateTransPixel(Current, dwFlags);
		}

		// 左右反転あり
		if (bMirrorLeftRight)
		{
			// X軸ループ終了
			BLOCK_START(EndLoopX_Mirror)
				sub			esi, 4
				add			edi, 4
				dec			ecx
			BLOCK_END(EndLoopX_Mirror)
		}
		// 左右反転なし
		else
		{
			// X軸ループ終了
			BLOCK_START(EndLoopX_NoMirror)
				add			esi, 4
				add			edi, 4
				dec			ecx
			BLOCK_END(EndLoopX_NoMirror)
		}
		CODE_JNZ(Current, LoopX)
	}
	// Y軸ループ終了
	BLOCK_START(EndLoopY)
		add			esi, [esp]BltStackFrame.lSrcDistance
		add			edi, [esp]BltStackFrame.lDestDistance
		dec			[esp]BltStackFrame.uHeight
	BLOCK_END(EndLoopY)
	CODE_JNZ(Current, LoopY)

	// 終了コード
	BLOCK_START(Finalize)
		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	BLOCK_END(Finalize)
}

#pragma warning (default : 4731)

//////////////////////////////////////////////////////////////////////
// ビットブロック転送 (拡大縮小あり)
//////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4731)

// 「インライン asm はグローバルな最適化を抑制します」警告
#pragma warning (disable : 4740)

void CNxDynamicDraw32::GenerateStretchBlt(LPBYTE Current, DWORD dwFlags, BOOL bMirrorLeftRight)
{
	using namespace ConstTable;
	typedef CNxSurface::StretchBltInfo StretchBltInfo;
	typedef CNxDynamicDraw32::BltStackFrame BltStackFrame;

	LPBYTE CodeStart;
	LPBYTE CodeEnd;

	// 初期化コード
	BLOCK_START(Initialize)
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			esi, [esp]BltStackFrame.lpSrcSurface
		mov			edi, [esp]BltStackFrame.lpDestSurface
		mov			ebx, [esp]BltStackFrame.pStretchBltInfo
	BLOCK_END(Initialize)

	// Y軸ループ開始
	LPBYTE LoopY = Current;
	{
		// X軸ループ前処理
		BLOCK_START(BeforeLoopX)
			mov			ebp, 0
			mov			edx, [ebx]StretchBltInfo.uSrcOrgX
			mov			ecx, [esp]BltStackFrame.uWidth
		BLOCK_END(BeforeLoopX)

		// X軸ループ開始
		LPBYTE LoopX = Current;
		{
			// 補完あり
			if (dwFlags & NxBlt::linearFilter)
			{
				// 左右反転
				if (bMirrorLeftRight)
				{
					BLOCK_START(BeforeTransPixel_Filter_1_Mirror)
						lea			eax, [esi + ebp * 4 - 4]
						movq		mm2, [eax]
						add			eax, [esp]BltStackFrame.lSrcPitch
						movq		mm1, [eax]
						movq		mm6, mm2
						movq		mm5, mm1
						punpckhdq	mm2, mm2
						punpckhdq	mm1, mm1
						punpckldq	mm2, mm6
						punpckldq	mm1, mm5
					BLOCK_END(BeforeTransPixel_Filter_1_Mirror)
				}
				else
				{
					BLOCK_START(BeforeTransPixel_Filter_1)
						lea			eax, [esi + ebp * 4]
						movq		mm2, [eax]
						add			eax, [esp]BltStackFrame.lSrcPitch
						movq		mm1, [eax]
					BLOCK_END(BeforeTransPixel_Filter_1)
				}

				if (dwFlags & NxBlt::srcAlpha)
				{	// 転送元アルファ参照
					BLOCK_START(BeforeTransPixel_Filter_3)
						movq		mm4, [dwlConst_00FF_FFFF_00FF_FFFF]
						pcmpeqb		mm3, mm3
						movq		mm6, mm2
						pcmpeqb		mm2, mm4
						movq		mm7, mm1
						pcmpeqb		mm1, mm4
						psrad		mm2, 31
						psrad		mm1, 31
						pxor		mm2, mm3
						pxor		mm1, mm3
						pand		mm6, mm2
						pand		mm7, mm1
						movq		mm0, mm6
						movq		mm5, mm7
						pand		mm6, mm4
						pand		mm7, mm4
						movq		mm4, mm6
						movq		mm3, mm7
						punpckldq	mm6, mm6
						punpckldq	mm7, mm7
						punpckhdq	mm4, mm6
						movq		mm6, [dwlConst_00FF_FFFF_00FF_FFFF]
						punpckhdq	mm3, mm7
						pcmpeqb		mm7, mm7
						pandn		mm2, mm4
						pandn		mm1, mm3
						por			mm2, mm0
						por			mm1, mm5
						movq		mm0, mm2
						movq		mm5, mm1
						movq		mm4, mm2
						movq		mm3, mm1
						punpckldq	mm0, mm0
						punpckldq	mm5, mm5
						por			mm0, mm2
						por			mm5, mm1
						pcmpeqb		mm0, mm6
						pcmpeqb		mm5, mm6
						punpckhdq	mm0, mm0
						punpckhdq	mm5, mm5
						psrad		mm0, 31
						mov			eax, [esp]BltStackFrame.uSrcOrgY
						psrad		mm5, 31
						pxor		mm0, mm7
						pxor		mm5, mm7
						pand		mm2, mm0
						pand		mm1, mm5
						shr			eax, 24
						movq		mm4, mm2
						movq		mm3, mm1
						pand		mm2, mm6
						pand		mm1, mm6
						movd		mm6, edx
						movd		mm7, eax
						pandn		mm0, mm1
						pandn		mm5, mm2
						por			mm4, mm0
						psrld		mm6, 24
						por			mm3, mm5
						pxor		mm0, mm0
						punpcklwd	mm6, mm6
						movq		mm2, mm4
						punpckldq	mm6, mm6
						movq		mm1, mm3
						punpcklbw	mm2, mm0
						punpcklbw	mm1, mm0
						punpckhbw	mm4, mm0
						punpckhbw	mm3, mm0
						psubw		mm4, mm2
						psubw		mm3, mm1
						pmullw		mm4, mm6
						punpcklwd	mm7, mm7
						pmullw		mm3, mm6
						psrlw		mm4, 8
						punpckldq	mm7, mm7
						psrlw		mm3, 8
						paddb		mm2, mm4
						paddb		mm1, mm3
						psubw		mm1, mm2
						pmullw		mm1, mm7
						psrlw		mm1, 8
						paddb		mm2, mm1
						packuswb	mm2, mm0
						movd		eax, mm2
						punpcklbw	mm2, mm0
						shr			eax, 24
					BLOCK_END(BeforeTransPixel_Filter_3)
				}
				else
				{	// 転送元アルファ参照せず
					BLOCK_START(BeforeTransPixel_Filter_4)
						mov			eax, [esp]BltStackFrame.uSrcOrgY
						movq		mm4, mm2
						movd		mm6, edx
						movd		mm7, eax
						movq		mm3, mm1
						psrld		mm7, 24
						punpcklbw	mm1, mm0
						punpcklwd	mm7, mm7
						psrld		mm6, 24
						punpckhbw	mm3, mm0
						punpckldq	mm7, mm7
						punpcklbw	mm2, mm0
						punpckhbw	mm4, mm0
						punpcklwd	mm6, mm6
						psubw		mm1, mm2
						psubw		mm3, mm4
						pmullw		mm1, mm7
						punpckldq	mm6, mm6
						pmullw		mm3, mm7
						psrlw		mm1, 8
						psrlw		mm3, 8
						paddb		mm2, mm1
						paddb		mm4, mm3
						psubw		mm4, mm2
						pmullw		mm4, mm6
						psrlw		mm4, 8
						paddb		mm2, mm4
					BLOCK_END(BeforeTransPixel_Filter_4)
				}
			}
			// 補完なし
			else
			{
				BLOCK_START(BeforeTransPixel_NoFilter)
					movd		mm2, [esi + ebp * 4]
					movzx		eax, byte ptr [esi + ebp * 4 + 3]
					punpcklbw	mm2, mm0
				BLOCK_END(BeforeTransPixel_NoFilter)
			}

			// SrcAlphaあり
			if (dwFlags & NxBlt::srcAlpha)
			{
				// DestAlphaなし
				if (!(dwFlags & NxBlt::destAlpha))
				{
					BLOCK_START(BeforeTransPixel_SrcAlphaNoDestAlpha)
						mov			ebx, [esp]BltStackFrame.lpbSrcAlphaToOpacityTable
						movzx		eax, byte ptr [ebx + eax]
						mov			ebx, [esp]BltStackFrame.pStretchBltInfo
					BLOCK_END(BeforeTransPixel_SrcAlphaNoDestAlpha)
				}
			}
			// SrcAlphaなし
			else
			{
				BLOCK_START(BeforeTransPixel_NoSrcAlpha)
					movzx		eax, [esp]BltStackFrame.uOpacity
				BLOCK_END(BeforeTransPixel_NoSrcAlpha)
			}

			// 1ピクセル転送
			GenerateTransPixel(Current, dwFlags);
		}
		// 左右反転あり
		if (bMirrorLeftRight)
		{
			// X軸ループ終了
			BLOCK_START(EndLoopX_Mirror)
				add			edx, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
				sbb			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
				add			edi, 4
				dec			ecx
			BLOCK_END(EndLoopX_Mirror)
		}
		// 左右反転なし
		else
		{
			// X軸ループ終了
			BLOCK_START(EndLoopX_NoMirror)
				add			edx, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
				adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
				add			edi, 4
				dec			ecx
			BLOCK_END(EndLoopX_NoMirror)
		}
		CODE_JNZ(Current, LoopX)
	}
	// Y軸ループ終了
	BLOCK_START(EndLoopY)
		mov			edx, [ebx]StretchBltInfo.ul64SrcDeltaY.LowPart
		mov			eax, [esp]BltStackFrame.lSrcPitch
		mov			ebp, [ebx]StretchBltInfo.ul64SrcDeltaY.HighPart
		add			edx, [esp]BltStackFrame.uSrcOrgY
		adc			ebp, ecx
		add			edi, [esp]BltStackFrame.lDestDistance
		imul		eax, ebp
		mov			[esp]BltStackFrame.uSrcOrgY, edx
		add			esi, eax
		dec			[esp]BltStackFrame.uHeight
	BLOCK_END(EndLoopY)
	CODE_JNZ(Current, LoopY)

	// 終了コード
	BLOCK_START(Finalize)
		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	BLOCK_END(Finalize)
}

#pragma warning (default : 4731)

//////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDynamicDraw32::Blt()
// 概要: ビットブロック転送
// 戻値: 成功ならば TRUE
//////////////////////////////////////////////////////////////////////

BOOL CNxDynamicDraw32::Blt
(CNxSurface* pDestSurface,			// 転送先サーフェス
 const RECT* lpDestRect,			// 転送先矩形
 const CNxSurface* pSrcSurface,		// 転送元サーフェス
 const RECT* lpSrcRect,				// 転送元矩形
 const NxBlt* pNxBlt)				// 転送方法
{
	if (!CNxDraw::GetInstance()->IsMMXEnabled())
	{
		_RPTF0(_CRT_ASSERT, "CNxDrawDynamic32::Blt() : この関数は MMX 命令をサポートしない CPU では実行できません.\n");
		return FALSE;
	}
	
	if (pSrcSurface->GetBitCount() != 32)
	{
		_RPTF0(_CRT_ASSERT, "CNxDynamicDraw32::Blt() : "
			"転送元サーフェスが 32bpp ではありません.\n");
		return FALSE;
	}

	DWORD dwBltFlags = pNxBlt->dwFlags;
	
	UINT uOpacity;
	if (dwBltFlags & NxBlt::opacity)
	{
		if (pNxBlt->uOpacity == 0)
			return TRUE;
		else if (pNxBlt->uOpacity > 255)
		{
			_RPTF0(_CRT_ASSERT, "CNxDynamicDraw32::Blt() : 不透明度の値が異常です.");
			return FALSE;
		}
		uOpacity = pNxBlt->uOpacity;
	}
	else
	{
		uOpacity = 255;
	}

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	LONG lSrcPitch = pSrcSurface->GetPitch();
	CNxSurface::StretchBltInfo StretchBltInfo;
	BOOL bStretch;

	// 拡大縮小なし
	if (((lpDestRect->right - lpDestRect->left) -
		abs(lpSrcRect->right - lpSrcRect->left) |
		(lpDestRect->bottom - lpDestRect->top) -
		abs(lpSrcRect->bottom - lpSrcRect->top)) == 0)
	{
		// クリッピング
		if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;
		bStretch = FALSE;
		memset(&StretchBltInfo, 0, sizeof(StretchBltInfo));
	}
	// 拡大縮小あり
	else
	{
		// クリップ前の転送元と転送先のサイズ
		UINT uSrcWidth = abs(rcSrc.right - rcSrc.left);
		UINT uSrcHeight = abs(rcSrc.bottom - rcSrc.top);
		UINT uDestWidth = rcDest.right - rcDest.left;
		UINT uDestHeight = rcDest.bottom - rcDest.top;

		if (dwBltFlags & NxBlt::linearFilter)
		{
			// 転送元の高さ又は幅が 1 dot であるか、拡大率が 0.5倍(含む)以下ならば、フィルタは使用しない
			if (uSrcWidth > 1 && uSrcHeight > 1	&& uDestWidth > (uSrcWidth / 2) && uDestHeight > (uSrcHeight / 2))
			{	// バイリニアフィルタ使用時の座標補正
				// フィルタ使用時は余分に 1dot 分大きい矩形をアクセスするので、
				// クリップ前に矩形を縮小する
				if (lpSrcRect->right > lpSrcRect->left)
				{	// 正方向 (右端から縮小)
					rcSrc.right--;
					rcSrcClip.right--;
				}
				else
				{	// 左右反転 (左端から縮小)
					rcSrc.left--;
					rcSrcClip.left++;
				}
				if (lpSrcRect->bottom > lpSrcRect->top)
				{
					rcSrc.bottom--;
					rcSrcClip.bottom--;
				}
				else
				{	// 上下反転
					rcSrc.top++;
					rcSrcClip.top++;
				}
			}
			else
			{	// フィルタは使用しない
				dwBltFlags &= ~NxBlt::linearFilter;
			}
		}
		// クリッピング
		if (!pDestSurface->ClipStretchBltRect(rcDest, rcSrc, rcSrcClip, StretchBltInfo))
			return TRUE;
		bStretch = TRUE;
	}

	// 幅と高さ
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// 転送先サーフェスメモリへのポインタと距離を取得
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits())
		+ pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;

	LONG lSrcDelta = 4;		// X コピー方向
	if (lpSrcRect->right - lpSrcRect->left < 0)
	{	// 左右反転
		lSrcDelta = -lSrcDelta;
		rcSrc.left = rcSrc.right - 1;
	}
	if (lpSrcRect->bottom - lpSrcRect->top < 0)
	{	// 上下反転
		lSrcPitch = -lSrcPitch;
		rcSrc.top = -(rcSrc.bottom - 1);
	}

	BOOL bMirrorLeftRight = (lSrcDelta < 0);		// 左右反転するならば TRUE

	// 転送元
	LONG lSrcDistance = lSrcPitch - uWidth * lSrcDelta;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>
		(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top +
		rcSrc.left * 4;

	// blendNormal 以外では destAlpha は使用不可
	if ((dwBltFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
		dwBltFlags &= ~NxBlt::destAlpha;
	
	// 実行バッファをロック
	LPVOID lpExecuteBuffer = LockExecuteBuffer();

	if (bStretch != m_previous.bStretch || dwBltFlags != m_previous.dwFlags ||	bMirrorLeftRight != m_previous.bMirrorLeftRight)
	{	// 以前の条件と異なるならば、コードを生成
		((bStretch) ? GenerateStretchBlt : GenerateNormalBlt)(static_cast<LPBYTE>(lpExecuteBuffer), dwBltFlags, bMirrorLeftRight);

		m_previous.bStretch = bStretch;
		m_previous.dwFlags = dwBltFlags;
		m_previous.bMirrorLeftRight = bMirrorLeftRight;
	}

	// 実行バッファの関数プロトタイプ
	typedef void (__cdecl *BltProc)
		(DWORD dwBltFlags, LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
		 LONG lSrcPitch, UINT uWidth, UINT uHeight, UINT uOpacity, NxColor color, DWORD dwMask,
		 const BYTE* lpbSrcAlphaToOpacityTable, CNxSurface::StretchBltInfo* pBltInfo, UINT uSrcOrgY);

	// 生成コードの実行
	reinterpret_cast<BltProc>(lpExecuteBuffer)
	(
		dwBltFlags,
		lpDestSurface,
		lpSrcSurface,
		lDestDistance,
		lSrcDistance,
		lSrcPitch,
		uWidth,
		uHeight,
		uOpacity,
		pNxBlt->nxbColor,
		*reinterpret_cast<const DWORD*>(&pNxBlt->nxbRGBAMask),
		ConstTable::bySrcAlphaToOpacityTable[uOpacity],
		&StretchBltInfo,
		StretchBltInfo.uSrcOrgY
	);

	// 実行バッファのロックを解除
	UnlockExecuteBuffer();
	
	return TRUE;
}
