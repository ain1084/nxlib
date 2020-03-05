// NxHLSColor.cpp: CNxHLSColor クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxHLSColor.h"

///////////////////////////////////////////////////////////////////////////
// private:
//	static NxColor CNxHLSColor::HLStoRGB(HLSColor hlsColor)
// 概要: HLS を RGB へ変換して返す
// 引数: HLSColor hlsColor ... 変換元 HLS (hhls)
// 戻値: 変換された RGB (最上位バイト(Alpha)は 0)
///////////////////////////////////////////////////////////////////////////

NxColor __declspec(naked) CNxHLSColor::HLStoRGB(HLSColor /*hlsColor*/)
{
#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID	EDI;
		LPVOID	EBP;
		LPVOID	EBX;
		LPVOID	EIP;
		HLSColor hlsColor;
	};
#pragma pack(pop)
	__asm
	{
		push	ebx
		push	ebp
		push	edi
		
		movzx	ecx, byte ptr [esp]StackFrame.hlsColor.byLightness
		movzx	edx, byte ptr [esp]StackFrame.hlsColor.bySaturation
		movzx	eax, word ptr [esp]StackFrame.hlsColor.wHue
		cmp		ecx, 128
		jl		lightness_less_128

		; ecx = lightness
		; edx = saturation

		; l + s - (s * l) / 255

		mov		ebx, ecx
		imul	ebx, edx		; l = s * l
		add		ebx, 127
		add		edx, ecx		; s + l
		imul	ebx, 8081h
		shr		ebx, 23
		sub		edx, ebx
		jmp		short main0

lightness_less_128:

		; ecx = lightness
		; edx = saturation

		; l * (255 + s) / 255	

		add		edx, 255
		imul	edx, ecx
		imul	edx, 8081h
		shr		edx, 23

main0:

		; ecx = lightness
		; edx = m2
		
		add		ecx, ecx
		lea		ebx, [eax + 120]	; hue + 120
		mov		edi, edx			; m2
		sub		ecx, edx			; m1

		/////////////////////////////////////
		; Red
		; ebx      = hue + 120
		; ecx      = m1
		; edx, edi = m2
		/////////////////////////////////////


		cmp		ebx, 360
		jl		R_getValue0

		sub		ebx, 360

R_getValue0:

		cmp		ebx, 60
		jl		R_less_60
		cmp		ebx, 180
		jl		R_less_180
		cmp		ebx, 240
		jl		R_less_240

		mov		ebx, ecx
		jmp		short R_getValueEnd

R_less_60:

		sub		edx, ecx
		imul	ebx, edx
		imul	ebx, 8889h
		shr		ebx, 21
		add		ebx, ecx
		jmp		short R_getValueEnd

R_less_180:

		mov		ebx, edx
		jmp		short R_getValueEnd

R_less_240:

		mov		ebp, 240
		sub		edx, ecx		
		sub		ebp, ebx
		mov		ebx, 8889h
		imul	edx, ebp
		imul	ebx, edx
		shr		ebx, 21
		add		ebx, ecx

R_getValueEnd:

		///////////////////////////////////
		; Green
		; hue + 0
		; ecx = m1
		; edi = m2
		///////////////////////////////////
			
		movzx	eax, word ptr [esp]StackFrame.hlsColor.wHue
		shl		ebx, 8

		cmp		eax, 60
		mov		edx, edi
		jl		G_less_60
		cmp		eax, 180
		jl		G_less_180
		cmp		eax, 240
		jl		G_less_240

		mov		eax, ecx
		jmp		short G_getValueEnd

G_less_60:

		sub		edx, ecx
		imul	eax, edx
		imul	eax, 8889h
		shr		eax, 21
		add		eax, ecx
		jmp		short G_getValueEnd

G_less_180:

		mov		eax, edx
		jmp		short G_getValueEnd

G_less_240:

		mov		ebp, 240
		sub		edx, ecx
		sub		ebp, eax
		mov		eax, 8889h
		imul	edx, ebp
		imul	eax, edx
		shr		eax, 21
		add		eax, ecx

G_getValueEnd:

		or		ebx, eax

		
		////////////////////////////////
		; Blue
		; hue
		; ecx = m1
		; edi = m2
		////////////////////////////////
		
		movzx	eax, word ptr [esp]StackFrame.hlsColor.wHue
		shl		ebx, 8
		sub		eax, 120

		jge		B_getValue0

		add		eax, 360


B_getValue0:

		cmp		eax, 60
		jl		B_less_60
		cmp		eax, 180
		jl		B_less_180
		cmp		eax, 240
		jl		B_less_240

		mov		eax, ecx
		jmp		short B_getValueEnd

B_less_60:

		sub		edi, ecx
		imul	eax, edi
		imul	eax, 8889h
		shr		eax, 21
		add		eax, ecx
		jmp		short B_getValueEnd

B_less_180:

		mov		eax, edi
		jmp		short B_getValueEnd

B_less_240:

		mov		ebp, 240
		sub		edi, ecx
		sub		ebp, eax
		mov		eax, 8889h
		imul	edi, ebp
		imul	eax, edi
		shr		eax, 21
		add		eax, ecx

B_getValueEnd:

		pop		edi
		pop		ebp
		or		eax, ebx
		pop		ebx
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////
// private:
//	static HLSCOlor CNxHLSColorSpace::RGBtoHLS(NxColor nxColor)
// 概要: RGB を HLS へ変換する
// 引数: NxColor dwColor ... 変換元 RGB (Alpha は無視される)
// 戻値: 変換された HLSColor 型
///////////////////////////////////////////////////////////////////////////////

#pragma warning (push)
#pragma warning (disable : 4201)
CNxHLSColor::HLSColor __declspec(naked) CNxHLSColor::RGBtoHLS(NxColor /*nxColor*/)
{
#pragma pack(push, 4)
	struct StackFrame
	{
		LPVOID	EBP;
		LPVOID	EDI;
		LPVOID	ESI;
		LPVOID	EBX;
		LPVOID	EIP;
		union
		{
			NxColor nxColor;
			struct
			{
				BYTE	byBlue;
				BYTE	byGreen;
				BYTE	byRed;
				BYTE	byDummy;
			};
		};
	};
#pragma pack(pop)

	__asm
	{
		push	ebx
		push	esi
		push	edi
		push	ebp

		movzx	eax, byte ptr [esp]StackFrame.byRed
		movzx	ebx, byte ptr [esp]StackFrame.byGreen
		cmp		eax, ebx
		movzx	esi, byte ptr [esp]StackFrame.byBlue
		ja		greenIsGreater		; green > red

		xchg	eax, ebx

greenIsGreater:

		; ecx = max(red, blue)
		; edx = min(green, blue)

		sub		eax, esi
		cmc
		sbb		ecx, ecx
		sub		ebx, esi
		sbb		edx, edx
		and		ecx, eax
		and		edx, ebx
		add		ecx, esi
		add		edx, esi

		mov		edi, ecx
		mov		ebp, edx
		lea		eax, [ecx + edx]
		shl		eax, 7
		and		eax, 0000ff00h		; eax = lightness * 256
		cmp		ecx, edx
		jnz		minMaxNotEqual

		; saturation = hue = 0

		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret

minMaxNotEqual:
		
		sub		ecx, edx			; ecx = delta
		add		ebp, edi
		cmp		eax, 00008000h
		mov		esi, 255
		push	eax					; save lightness
		mov		eax, ecx
		jb		calcSaturation		; saturation = delta * 255 / (max + min)
		
		; saturation = delta * 255 / (511 - max - min)

		xor		ebp, 511

calcSaturation:

		movzx	ebx, byte ptr [esp]StackFrame.byBlue + 4
		mul		esi
		movzx	esi, byte ptr [esp]StackFrame.byRed + 4
		div		ebp
		movzx	edx, byte ptr [esp]StackFrame.byGreen + 4

		; eax = saturation
		; ecx = delta
		; ebx = blue
		; edx = green
		; edi = max
		; esi = red

		cmp		esi, edi
		mov		ebp, 0 + 360
		push	eax					; save saturation
		mov		eax, edx
		jz		calcHue				; eax = green, ebx = blue
		cmp		edx, edi
		mov		eax, ebx
		mov		ebx, esi
		mov		ebp, 60 * 2 + 360
		jz		calcHue				; eax = blue = ebx = red
		mov		eax, esi
		mov		ebx, edx
		mov		ebp, 60 * 4 + 360

		; eax = red ebx = green

calcHue:

		mov		edx, 60
		sub		eax, ebx
		imul	edx
		idiv	ecx
		pop		ecx					; restore saturation
		add		eax, ebp

		cmp		eax, 360
		jl		noRoundHue
		sub		eax, 360

noRoundHue:

		pop		edx					; restore lightness
		shl		eax, 16
		or		ecx, edx
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		or		eax, ecx
		ret
	}
}

#pragma warning (pop)