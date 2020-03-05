// NxCustomDraw8.cpp: CNxCustomDraw8 �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �T�[�t�F�X�������ւ̒��ڕ`��(8bpp ��p)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxCustomDraw8.h"
#include "NxDrawLocal.h"

using namespace NxDrawLocal;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static DWORD CNxCustomDraw8::BiLinearFilter(UINT uSrcOrgX, UINT uSrcOrgY, const BYTE* lpSrcSurface, LONG lSrcPitch)
// �T�v: �]�����T�[�t�F�X�̎��� 4pixel �ɂ��āA�o�C���j�A�t�B���^��K�p�������ʂ�Ԃ�
// ����: UINT uSrcOrgX ... X ���W�̏�����(UINT_MIN �` UINT_MAX)
//       UINT uSrcOrgY ... Y ���W�̏�����(UINT_MIN �` UINT_MAX)
//       const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//       LONG lSrcPitch ... �]�����T�[�t�F�X�̕�
// �ߒl: �t�B���^�K�p��̃s�N�Z��
// ���l: �A���t�@�ɂ��Ă����Z�͓K�p�����B
//
//       A �͍���(lpSrcSurface), B �͉E��(lpSrcSurface + 4),
//       C �͍���(lpSrcSurface + lSrcPitch)�AD �͉E��(lpSrcSurface + lSrcPitch + 4) �̃s�N�Z��
//		 uSrcOrgX �� AB(CD) �Ԃ̈ʒu���AuSrcOrgY �� AC(BD) �Ԃ̈ʒu�����肵�A����ꂽ�s�N�Z��(R)��Ԃ�
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
// �T�v: 32bpp ���� 8bpp �ւ̃}�X�N�]�� (386 ��)
// ����: LPBYTE lpDestSurface     ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance       ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       LONG lSrcDistance        ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       UINT uWidth              ... ����
//       UINT uHeight             ... �c��
//       DWORD dwMask             ... �]�����}�X�N
// �ߒl: �Ȃ�
// ���l: (�]���� AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
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
// �T�v: 8bpp ���� 8bpp �ւ̃}�X�N�]�� (386 ��)
// ����: LPBYTE lpDestSurface     ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance       ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       LONG lSrcDistance        ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       UINT uWidth              ... ����
//       UINT uHeight             ... �c��
//       DWORD dwMask             ... �]�����}�X�N
// �ߒl: �Ȃ�
// ���l: (�]���� AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
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
		shr		edx, 24									; alpha �̂ݗL��
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
// �T�v: RGBA �}�X�N�]�����s�Ȃ�
// ����: CNxSurface* pDestSurface ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect         ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface  ... �]�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect          ... �]������`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBlt            ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
								  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (IsStretch(lpDestRect, lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw8::Blt_RGBAMask() : �g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	UINT uWidth;
	UINT uHeight;
	LONG lDestDistance;
	LONG lSrcDistance;
	LPBYTE lpDestSurface;
	const BYTE* lpSrcSurface;
	
	// ���ƍ���
	uWidth = lpDestRect->right - lpDestRect->left;
	uHeight = lpDestRect->bottom - lpDestRect->top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + lpDestRect->top * pDestSurface->GetPitch() + lpDestRect->left;
	lDestDistance = pDestSurface->GetPitch() - uWidth;
	DWORD dwMask = *reinterpret_cast<const DWORD*>(&pNxBlt->nxbRGBAMask.byBlue);

	switch (pSrcSurface->GetBitCount())
	{
	case 32:	// 32bpp ���� 8bpp ��
		lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left * 4;
		lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;
		Blt_MaskFrom32_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, dwMask);
		break;
	case 8:		// 8bpp ���� 8bpp ��
		lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left;
		lSrcDistance = pSrcSurface->GetPitch() - uWidth;
		Blt_MaskFrom8_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, dwMask);
		break;
	default:
		_RPTF2(_CRT_WARN, "�T�[�|�[�g����Ă��Ȃ��r�b�g�[�x��(%d -> %d)�̓]���ł�.\n", pSrcSurface->GetBitCount(), pDestSurface->GetBitCount());
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													  const NxBlt* pNxBlt) const
// �T�v: �P���h��ׂ�
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �h��ׂ���`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X(�Q�Ƃ���Ȃ�)
//       const RECT* lpSrcRect         ... �]������`(�Q�Ƃ���Ȃ�)
//       const NxBlt* pNxBlt           ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
										  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/,
										  const NxBlt* pNxBlt) const
{
	// �N���b�v
	RECT rcDest = *lpDestRect;
	if (!pDestSurface->ClipBltRect(rcDest))
		return TRUE;
	
	// ���ƍ���
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
// �T�v: �ʏ� Blt (���]�Ή�)
// ����: LPBYTE lpDestSurface     ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance       ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       LONG lSrcDistance        ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDelta			  ... �]�����̕ψ�(4 or -4)
//       UINT uWidth              ... ��
//       UINT uHeight             ... ����
// �ߒl: �Ȃ�
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
// �T�v: �g��k�� Blt
// ����: LPBYTE lpDestSurface       ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       const BYTE* lpSrcSurface   ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance         ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       LONG lSrcDistance          ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       UINT uWidth                ... ��
//       UINT uHeight               ... ����
//       const CNxSurface::BltInfo* pBltInfo
// �ߒl: �Ȃ�
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

		; 32bpp �łƈႤ�̂͂�������

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
// �T�v: �o�C���j�A�t�B���^�g��k�� Blt (386)
// ����: LPBYTE lpDestSurface       ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       const BYTE* lpSrcSurface   ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance         ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       LONG lSrcPitch             ... �]�����T�[�t�F�X�̎��̍s�ւ̃|�C���^���Z�l
//       UINT uWidth                ... ��
//       UINT uHeight               ... ����
//       const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
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
// �T�v: �ʏ� Blt
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �]������`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBlt           ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
								const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (pSrcSurface->GetBitCount() != 8)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Normal() : �]�����T�[�t�F�X�� 8bpp �`���łȂ���΂Ȃ�܂���.\n");
		return FALSE;
	}

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	LONG lSrcPitch = pSrcSurface->GetPitch();

	if (!IsStretch(lpDestRect, lpSrcRect))
	{
		// �N���b�v
		if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;
	
		// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
		UINT uWidth = rcDest.right - rcDest.left;
		UINT uHeight = rcDest.bottom - rcDest.top;

		LONG lDestDistance = pDestSurface->GetPitch() - uWidth;
		LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left;


		// ���]�̏���
		// ���E���]�̏ꍇ rcSrc.right �͉E�[ + 1, rcSrc.left �͍��[
		// �㉺���]�̏ꍇ rcSrc.bottom �͉��[ + 1, rcSrc.top �͏�[
		// �N���b�v��̋�`���甽�]�̔��ʂ͂ł��Ȃ����߁A�I���W�i���� lpSrcRect ���g�p����
		LONG lSrcDeltaX = 1;				// X �R�s�[����

		if (lpSrcRect->right - lpSrcRect->left < 0)
		{	// ���E���]
			lSrcDeltaX = -1;
			rcSrc.left = rcSrc.right - 1;	// �J�n�� rcSrc.right - 1 (�E�[) �Ƃ���
		}
		if (lpSrcRect->bottom - lpSrcRect->top < 0)
		{	// �㉺���]
			lSrcPitch = -lSrcPitch;			// �]���� Pitch �������]
			rcSrc.top = -(rcSrc.bottom - 1);	// �J�n�� rcSrc.bottom - 1 �Ƃ��āA�����𔽓](�]���� Pitch �������]�ɍ��킹��)
		}

		// �]�����T�[�t�F�X�̃|�C���^���v�Z
		LONG lSrcDistance = lSrcPitch - uWidth * lSrcDeltaX;
		const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * lSrcPitch + rcSrc.left;

		Blt_Normal_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, lSrcDeltaX, uWidth, uHeight);
	}
	else
	{
		void (*pfnStretchBltFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance,
								  LONG lSrcPitch, UINT uWidth, UINT uHeight,
								  const CNxSurface::StretchBltInfo* pStretchBltInfo);

		// �N���b�v�O�̓]�����Ɠ]����̃T�C�Y
		UINT uSrcWidth = abs(rcSrc.right - rcSrc.left);
		UINT uSrcHeight = abs(rcSrc.bottom - rcSrc.top);
		UINT uDestWidth = rcDest.right - rcDest.left;
		UINT uDestHeight = rcDest.bottom - rcDest.top;

		// �o�C���j�A�t�B���^���g�p����Ȃ�΁A�]�����̎Q�Ɣ͈͂���������
		// �������A�]�����̍������͕��� 1 dot �ł��邩�A
		// �g�嗦�� 0.5�{(�܂�)�ȉ��Ȃ�΁A�t�B���^�͎g�p���Ȃ�
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
		
		// �N���b�v
		CNxSurface::StretchBltInfo StretchBltInfo;
		if (!pDestSurface->ClipStretchBltRect(rcDest, rcSrc, rcSrcClip, StretchBltInfo))
			return TRUE;
		
		// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
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
// �T�v: Ruel �摜���g�p����u�����h Blt
// ����: const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �]������`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBtFx        ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_RuleBlend(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
								   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_RuleBlend() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_Blend(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//										   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// �T�v: �u�����h Blt (�T�[�t�F�X���̃A���t�@���g�p���Ȃ��B�]����A���t�@�͕ۑ�)
// ����: const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �]������`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBlt       ... ������ʓ��̎w��Ɏg�p���� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
							   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw8::Blt_Blend() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	int nBlendType = pNxBlt->dwFlags & NxBlt::blendTypeMask;

	if (!(pNxBlt->dwFlags & NxBlt::constDestAlpha))
	{	// constDestAlpha ���w�肳��ĂȂ��Ȃ��...
		if (uOpacity == 255 && nBlendType == NxBlt::blendNormal)
		{	// �s�����x�� 255 ���A�ʏ�u�����h�Ȃ�ΒP���]������
			return CNxCustomDraw8::Blt_Normal(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
		}
	}

	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_Blend() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlendDest(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//											   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// �T�v: �]����A���t�@���g�p�����u�����h Blt
// ����: const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �]������`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBlt       ... ������ʓ��̎w��Ɏg�p���� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_BlendDestAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
										const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_BlendDestAlpha() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlendSrcAlpha(const RECT* lpDestRect, const CNxSurface *pSrcSurface,
//												   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// �T�v: �]�����A���t�@���g�p�����u�����h Blt
// ����: const RECT* lpDestRect        ... �]�����`(NULL = �T�[�t�F�X�S��)
//       const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X
//       const RECT* lpSrcRect         ... �]������`(NULL = �T�[�t�F�X�S��)
//       const NxBlt *pNxBlt       ... �t���O�����w�肷�� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_BlendSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
									   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_BlendSrcAlpha() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlendDestSrcAlpha(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//													   const RECT* lpSrcRect, const NxBlt *pNxBlt)
// �T�v: �]�����Ɠ]����̃A���t�@���g�p�����u�����h Blt
// ����: const RECT* lpDestRect        ... �]�����`(NULL = �T�[�t�F�X�S��)
//       const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X
//       const RECT* lpSrcRect         ... �]������`(NULL = �T�[�t�F�X�S��)
//       const NxBlt *pNxBlt       ... �t���O�����w�肷�� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_BlendDestSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
										   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_BlendDestSrcAlpha() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													 const NxBlt* pNxBlt) const
// �T�v: �u�����h�h��ׂ� (�T�[�t�F�X���A���t�@�Q�Ɩ���)
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X(�Q�Ƃ���Ȃ�)
//       const RECT* lpSrcRect         ... �]������`(�Q�Ƃ���Ȃ�)
//       const NxBlt* pNxBlt           ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect, const CNxSurface* /*pSrcSurface*/,
										 const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const
{
	// �A���t�@�l�� 255 ���AblendNormal �Ȃ�Βʏ�h��ׂ�
	if (pNxBlt->nxbColor >= 0xff000000 && (pNxBlt->dwFlags & NxBlt::blendTypeMask) == NxBlt::blendNormal)
		return Blt_ColorFill_Normal(pDestSurface, lpDestRect, NULL, NULL, pNxBlt);

	// �A���t�@�l = 0
	if (pNxBlt->nxbColor < 0x01000000)
		return TRUE;

	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_Blend() : �u�����h�h��ׂ��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//															  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/,
//															  const NxBlt* /*pNxBlt*/) const
// �T�v: �]����A���t�@���g�p����u�����h�h��ׂ�
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X(�Q�Ƃ���Ȃ�)
//       const RECT* lpSrcRect         ... �]������`(�Q�Ƃ���Ȃ�)
//       const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
												  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlendDestAlpha() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//															 const CNxSurface* pAlphaSurface, const RECT* lpAlphaRect,
//															 const NxBlt* pNxBlt) const
// �T�v: �]�����A���t�@���g�p����u�����h�h��ׂ�
// ����: CNxSurface* pDestSurface        ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect          ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface *pSrcSurface   ... �]����(8bpp)�T�[�t�F�X
//       const RECT* lpSrcRect           ... �]������`
//       const NxBlt* pNxBlt             ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlendSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
												 const CNxSurface* /*pAlphaChannel*/, const RECT* /*lpAlphaRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlendSrcAlpha() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//																 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//																 const NxBlt* pNxBlt) const
// �T�v: �]�����Ɠ]����̃A���t�@���g�p����u�����h�h��ׂ�
// ����: CNxSurface* pDestSurface        ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect          ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pAlphaSurface ... �]�����T�[�t�F�X(8bpp) �ւ̃|�C���^
//       const RECT* lpSrcRectt          ... �]�����T�[�t�F�X(8bpp)�̋�`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBlt             ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
													 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlendDestSrcAlpha() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_ColorFill_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
//														const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//														const NxBlt* pNxBlt) const
// �T�v: RGBA �}�X�N�h��ׂ�
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �h��ׂ���`������ RECT �\���̂ւ̃|�C���^
///      const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X(�Q�Ƃ���Ȃ�)
//       const RECT* lpSrcRect         ... �]������`(�Q�Ƃ���Ȃ�)
//       const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: (nxcrFillColor AND NxBlt::nxbfRGBAMask) OR (�]���� AND (NOT NxBlt::nxbfRGBAMask)) ���s�Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_RGBAMask(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
											const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_RGBAMask() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Blt_BlurHorz(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//											  const NxBlt *pNxBlt) const
// �T�v: �����ڂ��� Blt
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �]������`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBlt		   ... ������ʓ��̎w��Ɏg�p���� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �]����A���t�@�͕ۑ������
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
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_Blur() : �]�����T�[�t�F�X�� 8bpp �ł͂���܂���.\n");
		return FALSE;
	}

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	LONG lSrcPitch = pSrcSurface->GetPitch();
	
	if (IsStretch(lpDestRect, lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "�g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	// �N���b�v
	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;
			
	UINT uLeftMargin = rcSrc.left - rcSrcClip.left;
	UINT uRightMargin = rcSrcClip.right - rcSrc.left - 1;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
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
		UINT uRightMarginCount = uRightMargin - uRange - 1;	// �A�N�Z�X�\�ȉE���̃s�N�Z��
		UINT uColumn = uWidth;

		// �������̃s�N�Z�������Z
		uSum += *lpSrcSurface;
		
		do
		{	// �E���̃s�N�Z���� 1dot ���Z
			if (static_cast<int>(uRightMarginCount--) > 0)
				uSum += *(lpSrcSurface + uRange);							// �A�N�Z�X�\�Ȕ͈͓�
			else
				uSum += *(lpSrcSurface + uRightMarginCount + uRange + 1);	// �͈͊O�B�E�[�̃s�N�Z���ŕ�U

			*lpDestSurface++ = static_cast<BYTE>((uMultiplier * uSum + 0x8000) >> 16);

			// ���Z���ʂ���A���[�̃s�N�Z���l�����Z
			if (uLeftMarginCount != 0)
			{	// �͈͊O
				uLeftMarginCount--;
				uSum -= uLeft;
			}
			else
			{	// �͈͓�
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
// �T�v: �����ڂ����u�����h�h��Ԃ�
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �]������`������ RECT �\���̂ւ̃|�C���^
//       const NxBlt* pNxBlt		   ... ������ʓ��̎w��Ɏg�p���� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Blt_ColorFill_BlurHorzBlend(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
												 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* /*pNxBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Blt_ColorFill_BlurHorzBlend() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_Grayscale(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												  const NxFilterBlt* pNxFilterBlt) const
// �T�v: �O���C�X�P�[�����t�B���^
// ����: CNxSurface* pDestSurface			... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect				... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface		... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect				... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//       const NxFilterBlt* pNxFilterBlt	... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_Grayscale(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
									  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_Grayscale() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_RGBColorBalance(CNxSurface* pDestSurface, const RECT* lpDestRect,
//														const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//														const NxFilterBlt* pNxFilterBlt) const
// �T�v: RGB �J���[�o�����X�����t�B���^
// ����: CNxSurface* pDestSurface			 ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect				 ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface		 ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect				 ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//       const NxFilterBlt* pNxFilterBlt	 ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_RGBColorBalance(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
											const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_RGBColorBalance() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_HueTransform(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													 const NxFilterBlt* pNxFilterBlt) const
// �T�v: �F���ϊ��t�B���^
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_HueTransform(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
										 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_HueTransform() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw8::Filter_Negative(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												 const NxFilterBlt* pNxFilterBlt) const
// �T�v: �l�K���]�t�B���^
// ����: CNxSurface* pDestSurface      ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw8::Filter_Negative(CNxSurface* /*pDestSurface*/, const RECT* /*lpDestRect*/,
									 const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxFilterBlt* /*pNxFilterBlt*/) const
{
	_RPT0(_CRT_WARN, "CNxCustomDraw8::Filter_Negative() : ���̊֐��̓C���v�������g����Ă��܂���.\n");
	return FALSE;
}
