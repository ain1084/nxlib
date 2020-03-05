// NxDynamicDraw32.cpp: CNxDynamicDraw32 �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// �T�v: ���I�R�[�h�ɂ��T�[�t�F�X�������ւ̒��ڕ`��(32bpp ��p)
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDynamicDraw.h"
#include "NxSurface.h"

namespace NxDrawLocal
{

class CNxDynamicDraw32 : public CNxDynamicDraw
{
public:
	CNxDynamicDraw32();
	virtual BOOL Blt(CNxSurface* pDestSurface, const RECT* lpDestRect, 
					 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
					 const NxBlt* pNxBlt);

private:
	static void GenerateFlagJump(LPBYTE& lpCurrent, LPBYTE lpJmpAddress,
								 BYTE byShortJmpCode, BYTE byNearJmpCode1, BYTE byNearJmpCode2);
	static void GenerateTransPixel(LPBYTE& Current, DWORD dwFlags);
	static void GenerateNormalBlt(LPBYTE Current, DWORD dwFlags, BOOL bMirrorLeftRight);
	static void GenerateStretchBlt(LPBYTE Current, DWORD dwFlags, BOOL bMirrorLeftRight);

	struct
	{
		DWORD dwFlags;
		BOOL  bStretch;
		BOOL  bMirrorLeftRight;
	} m_previous;

	#pragma pack(push, 4)
	struct BltStackFrame
	{
		LPVOID		edi;
		LPVOID		esi;
		LPVOID		ebp;
		LPVOID		ebx;
		LPVOID		eip;
		DWORD dwFlags;
		LPBYTE lpDestSurface;
		const BYTE* lpSrcSurface;
		LONG lDestDistance;
		LONG lSrcDistance;
		LONG lSrcPitch;
		UINT uWidth;
		UINT uHeight;
		UINT uOpacity;
		NxColor nxbColor;
		DWORD dwMask;
		const BYTE* lpbSrcAlphaToOpacityTable;
		CNxSurface::StretchBltInfo* pStretchBltInfo;
		UINT uSrcOrgY;
	};
	#pragma pack(pop)
};

} // namespace NxDrawLocal
