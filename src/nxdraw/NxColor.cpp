// NxColor.cpp: CNxHLSColor クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxColor.h"

///////////////////////////////////////////////////////////////////////////
// public:
//	BYTE CNxColor::GetGrayscale()
// 概要: グレイスケール値を取得
// 引数: なし
// 戻値: 変換結果(BYTE)
///////////////////////////////////////////////////////////////////////////

BYTE __declspec(naked) CNxColor::GetGrayscale() const
{
	__asm
	{	; ecx = this
		movzx	eax, byte ptr [ecx]CNxColor.m_byBlue
		movzx	edx, byte ptr [ecx]CNxColor.m_byGreen
		movzx	ecx, byte ptr [ecx]CNxColor.m_byRed
		imul	eax, 1920606
		imul	edx, 9841699
		imul	ecx, 5014911
		add		eax, edx
		add		eax, ecx
		shr		eax, 24
		ret
	}
}
