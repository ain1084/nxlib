// NxCustomDraw8.cpp: CNxCustomDraw8 クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: サーフェスメモリへの直接描画(8bpp 専用)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxCustomDraw8.h"
#include "NxDrawLocal.h"

using namespace NxDrawLocal;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static DWORD CNxCustomDraw8::BiLinearFilter(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch)
// 概要: 転送元サーフェスの周囲 4pixel について、バイリニアフィルタを適用した結果を返す
// 引数: UINT uSrcOrgX ... X 座標の小数部(UINT_MIN ～ UINT_MAX)
//       UINT uSrcOrgY ... Y 座標の小数部(UINT_MIN ～ UINT_MAX)
//       const BYTE* lpSrcSurface ... 転送元サーフェスへのポインタ
//       LONG lSrcPitch ... 転送元サーフェスの幅
// 戻値: フィルタ適用後のピクセル
// 備考: アルファについても演算は適用される。
//
//       A は左上(lpSrcSurface), B は右上(lpSrcSurface + 4),
//       C は左下(lpSrcSurface + lSrcPitch)、D は右下(lpSrcSurface + lSrcPitch + 4) のピクセル
//		 uSrcOrgX で AB(CD) 間の位置を、uSrcOrgY で AC(BD) 間の位置を決定し、得られたピクセル(R)を返す
//         A       B
//         +-------+
//         |  |    |
//         |--+    |
//         |  R    |
//         |       |
//         +-------+
//         C       D
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BYTE CNxCustomDraw8::BiLinearFilter(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch)
{
	UINT uSrcX = uSrcOrgX >> 24;
	UINT uSrcY = uSrcOrgY >> 24;
	
	DWORD dwLeft;
	DWORD dwRight;
	DWORD dwTop;
	DWORD dwBottom;

	dwLeft = *(lpSrcSurface);
	dwRight = *(lpSrcSurface + 1);
	dwTop = ((((dwRight - dwLeft) * uSrcX) >> 8) + dwLeft) & 0xff;
	dwLeft = *(lpSrcSurface + lSrcPitch);
	dwRight = *(lpSrcSurface + lSrcPitch + 1);
	dwBottom = ((((dwRight - dwLeft) * uSrcX) >> 8) + dwLeft) & 0xff;
	return static_cast<BYTE>(((((dwBottom - dwTop) * uSrcY) >> 8) + dwTop) & 0xff);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw8::Blt_MaskFrom32_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//												   LONG lDestDistance, LONG lSrcDistance,
//												   UINT uWidth, UINT uHeight, DWORD dwMask)
// 概要: 32bpp から 8bpp へのマスク転送 (386 版)
// 引数: LPBYTE lpDestSurface     ... 転送先サーフェスのビットデータへのポインタ
//       const BYTE* lpSrcSurface ... 転送元サーフェスのビットデータへのポインタ
//       LONG lDestDistance       ... 転送先サーフェスの行の端から次の行へのポインタ加算値
//       LONG lSrcDistance        ... 転送元サーフェスの行の端から次の行へのポインタ加算値
//       UINT uWidth              ... 横幅
//       UINT uHeight             ... 縦幅
//       DWORD dwMask             ... 転送元マスク
// 戻値: なし
// 備考: (転送元 AND dwMask) OR (転送先 AND (NOT dwMask)) を行なう
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw8::Blt_MaskFrom32_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
														  LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
														  UINT /*uWidth*/, UINT /*uHeight*/, DWORD /*dwMask*/)
{

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		eip;
		LPVOID		edi;
		LPVOID		esi;
		LPVOID		ebp;
		LPBYTE		lpDestSurface;
		const BYTE* lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwMask;
	};
#pragma pack(pop)

	__asm
	{
		push		edi
		push		esi
		push		ebp
		mov			eax, [esp]StackFrame.dwMask
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebx, eax
		test		eax, 0ff000000h
		mov			cl, 24
		jz			found_bit
		test		eax, 000ff0000h
		mov			cl, 16
		jz			found_bit
		test		eax, 00000ff00h
		mov			cl, 8
		jz			found_bit
		mov			cl, 0

found_bit:

		shr			eax, cl
		mov			ebp, [esp]StackFrame.lpSrcSurface
		not			al
		mov			ah, al

loop_y:

		mov			esi, [esp]StackFrame.uWidth

loop_x:
			
		mov			edx, [ebp]
		add			ebp, 4
		and			edx, ebx
		mov			al, [edi]
		shr			edx, cl
		and			al, ah
		or			al, dl
		mov			[edi], al
		inc			edi
		dec			esi
		jnz			loop_x

		mov			eax, [esp]StackFrame.uHeight
		add			edi, [esp]StackFrame.lDestDistance
		add			ebp, [esp]StackFrame.lSrcDistance
		dec			eax
		mov			[esp]StackFrame.uHeight, eax
		jnz			loop_y

		pop			ebp
		pop			esi
		pop			edi
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw8::Blt_MaskFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//												  LONG lDestDistance, LONG lSrcDistance,
//												  UINT uWidth, UINT uHeight, DWORD dwMask)
// 概要: 8bpp から 8bpp へのマスク転送 (386 版)
// 引数: LPBYTE lpDestSurface     ... 転送先サーフェスのビットデータへのポインタ
//       const BYTE* lpSrcSurface ... 転送元サーフェスのビットデータへのポインタ
//       LONG lDestDistance       ... 転送先サーフェスの行の端から次の行へのポインタ加算値
//       LONG lSrcDistance        ... 転送元サーフェスの行の端から次の行へのポインタ加算値
//       UINT uWidth              ... 横幅
//       UINT uHeight             ... 縦幅
//       DWORD dwMask             ... 転送元マスク
// 戻値: なし
// 備考: (転送元 AND dwMask) OR (転送先 AND (NOT dwMask)) を行なう
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw8::Blt_MaskFrom8_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
														 LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
														 UINT /*uWidth*/, UINT /*uHeight*/, DWORD /*dwMask*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		eip;
		LPVOID		ebx;
		LPVOID		edi;
		LPVOID		esi;
		LPVOID		ebp;
		LPBYTE		lpDestSurface;
		const BYTE* lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwMask;
	};
#pragma pack(pop)

	__asm
	{
		push	ebx
		push	edi
		push	esi
		push	ebp
		mov		edx, [esp]StackFrame.dwMask
		mov		edi, [esp]StackFrame.lpDestSurface
		shr		edx, 24									; alpha のみ有効
		mov		ebp, [esp]StackFrame.lpSrcSurface
		mov		edx, [dwByteToDwordTable + edx * 4]
		mov		esi, edx
		not		edx

loop_y:

		mov		ecx, [esp]StackFrame.uWidth
		test	ecx, 0fffffffch
		jz		skip_x_qword
		shr		ecx, 2

loop_x_dword:

		mov		eax, [ebp]
		add		ebp, 4
		mov		ebx, [edi]
		and		eax, esi
		and		ebx, edx
		or		eax, ebx
		mov		[edi], eax
		add		edi, 4
		loop	loop_x_dword

		mov		ecx, [esp]StackFrame.uWidth
		and		ecx, 3
		jz		loop_x_end

skip_x_qword:

loop_x_byte:

		movzx	eax, byte ptr [ebp]
		inc		ebp
		movzx	ebx, byte ptr [edi]
		and		eax, esi
		and		ebx, edx
		or		eax, ebx
		mov		[edi], al
		inc		edi
		loop	loop_x_byte

loop_x_end:

		mov		eax, [esp]StackFrame.uHeight
		add		edi, [esp]StackFrame.lDestDistance
		add		ebp, [esp]StackFrame.lSrcDistance
		dec		eax
		mov		[esp]StackFrame.uHeight, eax
		jnz		loop_y

		pop		ebp
		pop		esi
		pop		edi
		pop		ebx
		ret
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, cosnt NxBlt* pNxBlt) const
// 概要: RGBA マスク転送を行なう
// 引数: CNxSurface* pDestSurface ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect         ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface  ... 転送元サーフェスへのポインタ
//       const RECT* lpSrcRect          ... 転送元矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBlt            ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
								  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (IsStretch(lpDestRect, lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw8::Blt_RGBAMask() : 拡大縮小はサポートしていません.\n");
		return FALSE;
	}

	UINT uWidth;
	UINT uHeight;
	LONG lDestDistance;
	LONG lSrcDistance;
	LPBYTE lpDestSurface;
	const BYTE* lpSrcSurface;
	
	// 幅と高さ
	uWidth = lpDestRect->right - lpDestRect->left;
	uHeight = lpDestRect->bottom - lpDestRect->top;

	// 転送先サーフェスメモリへのポインタと距離を取得
	lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + lpDestRect->top * pDestSurface->GetPitch() + lpDestRect->left;
	lDestDistance = pDestSurface->GetPitch() - uWidth;
	DWORD dwMask = *reinterpret_cast<const DWORD*>(&pNxBlt->nxbRGBAMask.byBlue);

	switch (pSrcSurface->GetBitCount())
	{
	case 32:	// 32bpp から 8bpp へ
		lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left * 4;
		lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;
		Blt_MaskFrom32_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, dwMask);
		break;
	case 8:		// 8bpp から 8bpp へ
		lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left;
		lSrcDistance = pSrcSurface->GetPitch() - uWidth;
		Blt_MaskFrom8_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, dwMask);
		break;
	default:
		_RPTF2(_CRT_WARN, "サーポートされていないビット深度間(%d -> %d)の転送です.\n", pSrcSurface->GetBitCount(), pDestSurface->GetBitCount());
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													  const NxBlt* pNxBlt) const
// 概要: 単純塗り潰し
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... 塗り潰す矩形を示す RECT 構造体へのポインタ
//       const CNxSurface *pSrcSurface ... 転送元サーフェス(参照されない)
//       const RECT* lpSrcRect         ... 転送元矩形(参照されない)
//       const NxBlt* pNxBlt           ... NxBlt 構造体へのポインタ
// 戻値: 成功ならば TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
										  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/,
										  const NxBlt* pNxBlt) const
{
	// クリップ
	RECT rcDest = *lpDestRect;
	if (!pDestSurface->ClipBltRect(rcDest))
		return TRUE;
	
	// 幅と高さ
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	LONG lDestPitch = pDestSurface->GetPitch();
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * lDestPitch + rcDest.left;

	DWORD dwColor = ConstTable::dwByteToDwordTable[pNxBlt->nxbColor & 0xff];
	do
	{
		memset(lpDestSurface, dwColor, uWidth);
		lpDestSurface += lDestPitch;
	} while (--uHeight != 0);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw8::Blt_Normal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//										LONG lDestDistance, LONG lSrcDelta, LONG lSrcDistance)
// 概要: 通常 Blt (反転対応)
// 引数: LPBYTE lpDestSurface     ... 転送先サーフェスのビットデータへのポインタ
//       const BYTE* lpSrcSurface ... 転送元サーフェスのビットデータへのポインタ
//       LONG lDestDistance       ... 転送先サーフェスの行の端から次の行へのポインタ加算値
//       LONG lSrcDistance        ... 転送元サーフェスの行の端から次の行へのポインタ加算値
//		 LONG lSrcDelta			  ... 転送元の変位(4 or -4)
//       UINT uWidth              ... 幅
//       UINT uHeight             ... 高さ
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw8::Blt_Normal_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
													  LONG /*lDestDistance*/, LONG /*lSrcDistance*/, LONG /*lSrcDelta*/,
													  UINT /*uWidth*/, UINT /*uHeight*/)
{
#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EDI;
		LPVOID		ESI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		LONG		lSrcDelta;
		UINT		uWidth;
		UINT		uHeight;
	};
#pragma pack(pop)

	__asm
	{
		push		esi
		push		edi
		mov			edx, [esp]StackFrame.lSrcDelta
		mov			edi, [esp]StackFrame.lpDestSurface
		test		edx, edx
		mov			esi, [esp]StackFrame.lpSrcSurface
		mov			edx, [esp]StackFrame.uWidth
		jns			normal

		// reverse

reverse_loop_y:

		mov			ecx, edx

reverse_loop_x:

		mov			al, [esi]
		dec			esi
		mov			[edi], al
		inc			edi
		loop		reverse_loop_x

		mov			eax, [esp]StackFrame.uHeight
		add			esi, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			eax
		mov			[esp]StackFrame.uHeight, eax
		jnz			reverse_loop_y

		pop			edi
		pop			esi
		ret

normal:
		
		mov			eax, [esp]StackFrame.uHeight

normal_loop_y:

		mov			ecx, edx
		shr			ecx, 2
		rep			movsd
		mov			ecx, edx
		and			ecx, 3
		rep			movsb
		add			esi, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			eax
		jnz			normal_loop_y

		pop			edi
		pop			esi
		ret


	
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw8::Blt_NormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//											   LONG lDestDistance, LONG lSrcDistance,
//											   const CNxSurface::StretchBltInfo* pStretchBltInfo)
// 概要: 拡大縮小 Blt
// 引数: LPBYTE lpDestSurface       ... 転送先サーフェスのビットデータへのポインタ
//       const BYTE* lpSrcSurface   ... 転送元サーフェスのビットデータへのポインタ
//       LONG lDestDistance         ... 転送先サーフェスの行の端から次の行へのポインタ加算値
//       LONG lSrcDistance          ... 転送元サーフェスの行の端から次の行へのポインタ加算値
//       UINT uWidth                ... 幅
//       UINT uHeight               ... 高さ
//       const CNxSurface::BltInfo* pBltInfo
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw8::Blt_NormalStretch_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
															 LONG /*lDestDistance*/, LONG /*lSrcPitch*/,
															 UINT /*uWidth*/, UINT /*uHeight*/,
															 const CNxSurface::StretchBltInfo* /*pStretchBltInfo*/)
{
#pragma pack(push, 4)
	struct StackFrame
	{
		LPVOID		   EDI;
		LPVOID		   ESI;
		LPVOID		   EBX;
		LPVOID		   EBP;
		UINT		   uSrcOrgY;
		LPVOID		   EIP;
		LPBYTE		   lpDestSurface;
		const BYTE*	   lpSrcSurface;
		LONG		   lDestDistance;
		LONG		   lSrcPitch;
		UINT		   uWidth;
		UINT		   uHeight;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;

	__asm
	{
		sub			esp, 4		; allocate local variable (uSrcOrgY)
		push		ebp
		push		ebx
		push		esi
		push		edi

		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			edx, [esp]StackFrame.lpSrcSurface
		mov			eax, [ebx]StretchBltInfo.uSrcOrgY
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			[esp]StackFrame.uSrcOrgY, eax

loop_y:

		mov			ebp, 0
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		mov			ecx, [esp]StackFrame.uWidth

loop_x:

		; 32bpp 版と違うのはここだけ

		mov			al, [edx + ebp]		
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		mov			[edi], al
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		inc			edi
		loop		loop_x

		mov			esi, [ebx]StretchBltInfo.ul64SrcDeltaY.LowPart
		mov			eax, [esp]StackFrame.lSrcPitch
		mov			ebp, [ebx]StretchBltInfo.ul64SrcDeltaY.HighPart
		add			esi, [esp]StackFrame.uSrcOrgY
		adc			ebp, ecx
		add			edi, [esp]StackFrame.lDestDistance
		imul		eax, ebp
		mov			[esp]StackFrame.uSrcOrgY, esi
		add			edx, eax
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebx
		pop			ebp
		add			esp, 4
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw8::Blt_NormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														   LONG lDestDistance, LONG lSrcPitch,
//														   const CNxSurface::StretchBltInfo* pStretchBltInfo)
// 概要: バイリニアフィルタ拡大縮小 Blt (386)
// 引数: LPBYTE lpDestSurface       ... 転送先サーフェスのビットデータへのポインタ
//       const BYTE* lpSrcSurface   ... 転送元サーフェスのビットデータへのポインタ
//       LONG lDestDistance         ... 転送先サーフェスの行の端から次の行へのポインタ加算値
//       LONG lSrcPitch             ... 転送元サーフェスの次の行へのポインタ加算値
//       UINT uWidth                ... 幅
//       UINT uHeight               ... 高さ
//       const CNxSurface::StretchBltInfo* pStretchBltInfo
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw8::Blt_NormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
													   LONG lDestDistance, LONG lSrcPitch,
													   UINT uWidth, UINT uHeight,
													   const CNxSurface::StretchBltInfo* pStretchBltInfo)
{
	UINT uSrcOrgY = pStretchBltInfo->uSrcOrgY;
	do
	{
		UINT uColumn = uWidth;
		ULARGE_INTEGER ul64SrcOrgX;
		ul64SrcOrgX.QuadPart = pStretchBltInfo->uSrcOrgX;
		do
		{
			*lpDestSurface++ = BiLinearFilter(ul64SrcOrgX.LowPart, uSrcOrgY, lpSrcSurface + ul64SrcOrgX.HighPart, lSrcPitch);
			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
		uSrcOrgY = ul64SrcOrgY.LowPart;
	} while (--uHeight != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											const CNxSurace* pSrcSurface, const RECT* lpSrcRect,
//											const NxBlt* pNxBlt) const
// 概要: 通常 Blt
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... 転送元サーフェスのビットデータへのポインタ
//       const RECT* lpSrcRect         ... 転送元矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBlt           ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
								const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (pSrcSurface->GetBitCount() != 8)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Normal() : 転送元サーフェスは 8bpp 形式でなければなりません.\n");
		return FALSE;
	}

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	LONG lSrcPitch = pSrcSurface->GetPitch();

	if (!IsStretch(lpDestRect, lpSrcRect))
	{
		// クリップ
		if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;
	
		// 転送先サーフェスメモリへのポインタと距離を取得
		UINT uWidth = rcDest.right - rcDest.left;
		UINT uHeight = rcDest.bottom - rcDest.top;

		LONG lDestDistance = pDestSurface->GetPitch() - uWidth;
		LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left;


		// 反転の処理
		// 左右反転の場合 rcSrc.right は右端 + 1, rcSrc.left は左端
		// 上下反転の場合 rcSrc.bottom は下端 + 1, rcSrc.top は上端
		// クリップ後の矩形から反転の判別はできないため、オリジナルの lpSrcRect を使用する
		LONG lSrcDeltaX = 1;				// X コピー方向

		if (lpSrcRect->right - lpSrcRect->left < 0)
		{	// 左右反転
			lSrcDeltaX = -1;
			rcSrc.left = rcSrc.right - 1;	// 開始を rcSrc.right - 1 (右端) とする
		}
		if (lpSrcRect->bottom - lpSrcRect->top < 0)
		{	// 上下反転
			lSrcPitch = -lSrcPitch;			// 転送元 Pitch 符号反転
			rcSrc.top = -(rcSrc.bottom - 1);	// 開始を rcSrc.bottom - 1 として、符号を反転(転送元 Pitch 符号反転に合わせる)
		}

		// 転送元サーフェスのポインタを計算
		LONG lSrcDistance = lSrcPitch - uWidth * lSrcDeltaX;
		const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * lSrcPitch + rcSrc.left;

		Blt_Normal_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, lSrcDeltaX, uWidth, uHeight);
	}
	else
	{
		void (*pfnStretchBltFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance,
								  LONG lSrcPitch, UINT uWidth, UINT uHeight,
								  const CNxSurface::StretchBltInfo* pStretchBltInfo);

		// クリップ前の転送元と転送先のサイズ
		UINT uSrcWidth = abs(rcSrc.right - rcSrc.left);
		UINT uSrcHeight = abs(rcSrc.bottom - rcSrc.top);
		UINT uDestWidth = rcDest.right - rcDest.left;
		UINT uDestHeight = rcDest.bottom - rcDest.top;

		// バイリニアフィルタを使用するならば、転送元の参照範囲を狭くする
		// ただし、転送元の高さ又は幅が 1 dot であるか、
		// 拡大率が 0.5倍(含む)以下ならば、フィルタは使用しない
		if ((pNxBlt != NULL) && (pNxBlt->dwFlags & NxBlt::linearFilter)
			&& uSrcWidth > 1 && uSrcHeight > 1
			&& uDestWidth > (uSrcWidth / 2) && uDestHeight > (uSrcHeight / 2))
		{	// filter
			rcSrc.right--;
			rcSrc.bottom--;
			rcSrcClip.right--;
			rcSrcClip.bottom--;
			pfnStretchBltFunc = Blt_NormalStretchLinearFilter_386;
		}
		else
		{
			pfnStretchBltFunc = Blt_NormalStretch_386;
		}
		
		// クリップ
		CNxSurface::StretchBltInfo StretchBltInfo;
		if (!pDestSurface->ClipStretchBltRect(rcDest, rcSrc, rcSrcClip, StretchBltInfo))
			return TRUE;
		
		// 転送先サーフェスメモリへのポインタと距離を取得
		UINT uWidth = rcDest.right - rcDest.left;
		UINT uHeight = rcDest.bottom - rcDest.top;

		LONG lDestDistance = pDestSurface->GetPitch() - uWidth;
		LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left;

		const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * lSrcPitch + rcSrc.left;
		(pfnStretchBltFunc)(lpDestSurface, lpSrcSurface, lDestDistance, lSrcPitch, uWidth, uHeight, &StretchBltInfo);
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDra8w::Blt_RuleBlend(const RECT* lpDestRect, const NxSurface* pSrcSurface,
//											   const RECT* lpSrcRect, const NxBlt* pNxBlt)
// 概要: Ruel 画像を使用するブレンド Blt
// 引数: const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... 転送元サーフェスへのポインタ
//       const RECT* lpSrcRect         ... 転送元矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBtFx        ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_RuleBlend(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
								   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_RuleBlend() : この関数はインプリメントされていません.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_Blend(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//										   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// 概要: ブレンド Blt (サーフェス内のアルファを使用しない。転送先アルファは保存)
// 引数: const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... 転送元サーフェスへのポインタ
//       const RECT* lpSrcRect         ... 転送元矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBlt       ... 特殊効果等の指定に使用する NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
							   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	// 不透明度(uOpacity) の取得
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw8::Blt_Blend() : 不透明度の値が異常です.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	int nBlendType = pNxBlt->dwFlags & NxBlt::blendTypeMask;

	if (!(pNxBlt->dwFlags & NxBlt::constDestAlpha))
	{	// constDestAlpha が指定されてないならば...
		if (uOpacity == 255 && nBlendType == NxBlt::blendNormal)
		{	// 不透明度が 255 かつ、通常ブレンドならば単純転送する
			return CNxCustomDraw8::Blt_Normal(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
		}
	}

	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_Blend() : この関数はインプリメントされていません.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlendDest(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//											   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// 概要: 転送先アルファを使用したブレンド Blt
// 引数: const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... 転送元サーフェスへのポインタ
//       const RECT* lpSrcRect         ... 転送元矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBlt       ... 特殊効果等の指定に使用する NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_BlendDestAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
										const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_BlendDestAlpha() : この関数はインプリメントされていません.\n");
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlendSrcAlpha(const RECT* lpDestRect, const CNxSurface *pSrcSurface,
//												   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// 概要: 転送元アルファを使用したブレンド Blt
// 引数: const RECT* lpDestRect        ... 転送先矩形(NULL = サーフェス全体)
//       const CNxSurface *pSrcSurface ... 転送元サーフェス
//       const RECT* lpSrcRect         ... 転送元矩形(NULL = サーフェス全体)
//       const NxBlt *pNxBlt       ... フラグ等を指定する NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_BlendSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
									   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_BlendSrcAlpha() : この関数はインプリメントされていません.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlendDestSrcAlpha(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//													   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// 概要: 転送元と転送先のアルファを使用したブレンド Blt
// 引数: const RECT* lpDestRect        ... 転送先矩形(NULL = サーフェス全体)
//       const CNxSurface *pSrcSurface ... 転送元サーフェス
//       const RECT* lpSrcRect         ... 転送元矩形(NULL = サーフェス全体)
//       const NxBlt *pNxBlt       ... フラグ等を指定する NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_BlendDestSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
										   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_BlendDestSrcAlpha() : この関数はインプリメントされていません.\n");
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													 const NxBlt* pNxBlt) const
// 概要: ブレンド塗り潰し (サーフェス内アルファ参照無し)
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface *pSrcSurface ... 転送元サーフェス(参照されない)
//       const RECT* lpSrcRect         ... 転送元矩形(参照されない)
//       const NxBlt* pNxBlt           ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect, const CNxSurface* /*pSrcSurface*/,
										 const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const
{
	// アルファ値が 255 かつ、blendNormal ならば通常塗り潰し
	if (pNxBlt->nxbColor >= 0xff000000 && (pNxBlt->dwFlags & NxBlt::blendTypeMask) == NxBlt::blendNormal)
		return Blt_ColorFill_Normal(pDestSurface, lpDestRect, NULL, NULL, pNxBlt);

	// アルファ値 = 0
	if (pNxBlt->nxbColor < 0x01000000)
		return TRUE;

	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_Blend() : ブレンド塗り潰しはインプリメントされていません.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//															  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/,
//															  const NxBlt* /*pNxBlt*/) const
// 概要: 転送先アルファを使用するブレンド塗り潰し
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface *pSrcSurface ... 転送元サーフェス(参照されない)
//       const RECT* lpSrcRect         ... 転送元矩形(参照されない)
//       const NxBlt* pNxBlt		   ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
												  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlendDestAlpha() : この関数はインプリメントされていません.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//															 const CNxSurface* pAlphaSurface, const RECT* lpAlphaRect,
//															 const NxBlt* pNxBlt) const
// 概要: 転送元アルファを使用するブレンド塗り潰し
// 引数: CNxSurface* pDestSurface        ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect          ... 転送先矩形を示す RECT 構造体へのポインタ
//		 const CNxSurface *pSrcSurface   ... 転送元(8bpp)サーフェス
//       const RECT* lpSrcRect           ... 転送元矩形
//       const NxBlt* pNxBlt             ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlendSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
												 const CNxSurface* /*pAlphaChannel*/, const RECT* /*lpAlphaRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlendSrcAlpha() : この関数はインプリメントされていません.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//																 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//																 const NxBlt* pNxBlt) const
// 概要: 転送元と転送先のアルファを使用するブレンド塗り潰し
// 引数: CNxSurface* pDestSurface        ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect          ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pAlphaSurface ... 転送元サーフェス(8bpp) へのポインタ
//       const RECT* lpSrcRectt          ... 転送元サーフェス(8bpp)の矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBlt             ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
													 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlendDestSrcAlpha() : この関数はインプリメントされていません.\n");
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
//														const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//														const NxBlt* pNxBlt) const
// 概要: RGBA マスク塗り潰し
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... 塗り潰す矩形を示す RECT 構造体へのポインタ
///      const CNxSurface *pSrcSurface ... 転送元サーフェス(参照されない)
//       const RECT* lpSrcRect         ... 転送元矩形(参照されない)
//       const NxBlt* pNxBlt		   ... NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
// 備考: (nxcrFillColor AND NxBlt::nxbfRGBAMask) OR (転送先 AND (NOT NxBlt::nxbfRGBAMask)) を行なう
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_RGBAMask(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
											const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_RGBAMask() : この関数はインプリメントされていません.\n");
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlurHorz(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//											  const NxBlt *pNxBlt) const
// 概要: 水平ぼかし Blt
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... 転送元サーフェスへのポインタ
//       const RECT* lpSrcRect         ... 転送元矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBlt		   ... 特殊効果等の指定に使用する NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
// 備考: 転送先アルファは保存される
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_BlurHorz(CNxSurface* pDestSurface, const RECT* lpDestRect,
								  const CNxSurface *pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const
{
	if (pNxBlt->uBlurRange == 0)
	{
		if (pNxBlt->dwFlags & NxBlt::srcAlpha)
			return Blt_ColorFill_BlendSrcAlpha(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
		else
			return Blt_ColorFill_Blend(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
	}

	if (pSrcSurface->GetBitCount() != 8)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_Blur() : 転送元サーフェスが 8bpp ではありません.\n");
		return FALSE;
	}

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	LONG lSrcPitch = pSrcSurface->GetPitch();
	
	if (IsStretch(lpDestRect, lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "拡大縮小はサポートしていません.\n");
		return FALSE;
	}

	// クリップ
	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;
			
	UINT uLeftMargin = rcSrc.left - rcSrcClip.left;
	UINT uRightMargin = rcSrcClip.right - rcSrc.left - 1;

	// 幅と高さ
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// 転送先サーフェスメモリへのポインタと距離を取得
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth;

	LONG lSrcDistance = lSrcPitch - uWidth;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left;

	UINT uRange = pNxBlt->uBlurRange;
	
	UINT uMultiplier = 65536 / (uRange * 2 + 1);
	do
	{
		UINT uLeft;
		UINT uSum;

		uSum = 0;
		uLeft = 0;

		UINT uLeftMarginCount;
		if (uLeftMargin != 0)
		{
			UINT uColumn = min(uLeftMargin, uRange);
			lpSrcSurface -= uColumn;
			do
			{
				uSum += *lpSrcSurface++;
			} while (--uColumn != 0);
		}
		if (uLeftMargin >= uRange)
			uLeftMarginCount = 0;
		else
		{
			uLeft = *(lpSrcSurface - uLeftMargin);
			uLeftMarginCount = uRange - uLeftMargin;
			uSum += uLeft * uLeftMarginCount;
		}
		if (uRange >= 2)
		{
			if (uRightMargin - 1 != 0)
			{
				UINT uColumn = min(uRightMargin - 1, uRange - 1);
				lpSrcSurface += uColumn;
				do
				{
					uSum += *lpSrcSurface--;
				} while (--uColumn != 0);
			}
			if (uRightMargin < uRange - 1)
				uSum += *(lpSrcSurface + uRightMargin - 1) * (uRange - uRightMargin - 1);
		}	
		UINT uRightMarginCount = uRightMargin - uRange - 1;	// アクセス可能な右側のピクセル
		UINT uColumn = uWidth;

		// 処理中のピクセルを加算
		uSum += *lpSrcSurface;
		
		do
		{	// 右側のピクセルを 1dot 加算
			if (static_cast<int>(uRightMarginCount--) > 0)
				uSum += *(lpSrcSurface + uRange);							// アクセス可能な範囲内
			else
				uSum += *(lpSrcSurface + uRightMarginCount + uRange + 1);	// 範囲外。右端のピクセルで補填

			*lpDestSurface++ = static_cast<BYTE>((uMultiplier * uSum + 0x8000) >> 16);

			// 加算結果から、左端のピクセル値を減算
			if (uLeftMarginCount != 0)
			{	// 範囲外
				uLeftMarginCount--;
				uSum -= uLeft;
			}
			else
			{	// 範囲内
				uSum -= *(lpSrcSurface - uRange);
			}
			lpSrcSurface++;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while(--uHeight != 0);
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_BlurHorzBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
//															 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//															 const NxBlt* pNxBlt) const
// 概要: 水平ぼかしブレンド塗りつぶし
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... 転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... 転送元サーフェスへのポインタ
//       const RECT* lpSrcRect         ... 転送元矩形を示す RECT 構造体へのポインタ
//       const NxBlt* pNxBlt		   ... 特殊効果等の指定に使用する NxBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlurHorzBlend(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
												 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlurHorzBlend() : この関数はインプリメントされていません.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_Grayscale(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												  const NxFilterBlt* pNxFilterBlt) const
// 概要: グレイスケール化フィルタ
// 引数: CNxSurface* pDestSurface			... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect				... フィルタ適用結果の転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface		... フィルタが適用されるサーフェスへのポインタ
//       const RECT* lpSrcRect				... フィルタが適用される矩形を示す RECT 構造体へのポインタ
//       const NxFilterBlt* pNxFilterBlt	... NxFilterBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
// 備考: 全てのポインタは NULL 不可。矩形はクリップ済み
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_Grayscale(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
									  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_Grayscale() : この関数はインプリメントされていません.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_RGBColorBalance(CNxSurface* pDestSurface, const RECT* lpDestRect,
//														const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//														const NxFilterBlt* pNxFilterBlt) const
// 概要: RGB カラーバランス調整フィルタ
// 引数: CNxSurface* pDestSurface			 ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect				 ... フィルタ適用結果の転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface		 ... フィルタが適用されるサーフェスへのポインタ
//       const RECT* lpSrcRect				 ... フィルタが適用される矩形を示す RECT 構造体へのポインタ
//       const NxFilterBlt* pNxFilterBlt	 ... NxFilterBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
// 備考: 全てのポインタは NULL 不可。矩形はクリップ済み
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_RGBColorBalance(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
											const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_RGBColorBalance() : この関数はインプリメントされていません.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_HueTransform(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													 const NxFilterBlt* pNxFilterBlt) const
// 概要: 色相変換フィルタ
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... フィルタ適用結果の転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... フィルタが適用されるサーフェスへのポインタ
//       const RECT* lpSrcRect         ... フィルタが適用される矩形を示す RECT 構造体へのポインタ
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
// 備考: 全てのポインタは NULL 不可。矩形はクリップ済み
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_HueTransform(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
										 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_HueTransform() : この関数はインプリメントされていません.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_Negative(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												 const NxFilterBlt* pNxFilterBlt) const
// 概要: ネガ反転フィルタ
// 引数: CNxSurface* pDestSurface      ... 転送先サーフェスへのポインタ
//		 const RECT* lpDestRect        ... フィルタ適用結果の転送先矩形を示す RECT 構造体へのポインタ
//       const CNxSurface* pSrcSurface ... フィルタが適用されるサーフェスへのポインタ
//       const RECT* lpSrcRect         ... フィルタが適用される矩形を示す RECT 構造体へのポインタ
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt 構造体へのポインタ
// 戻値: 成功なら TRUE
// 備考: 全てのポインタは NULL 不可。矩形はクリップ済み
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_Negative(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
									 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_Negative() : この関数はインプリメントされていません.\n");
	return FALSE;
}
