// NxCustomDraw32.cpp: CNxCustomDraw32 �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �T�[�t�F�X�������ւ̒��ڕ`��(32bpp ��p)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxDrawLocal.h"
#include "NxCustomDraw32.h"
#include "NxAlphaBlend.h"
#include "NxHLSColor.h"

using namespace NxDrawLocal;

///////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static DWORD CNxCustomDraw32::BiLinearPixel(UINT uSrcOrgX, UINT uSrcOrgY,
//												DWORD dwTopLeft, DWORD dwTopRight,
//												DWORD dwBottomLeft, DWORD dwBottomRight)
// �ߒl: �t�B���^�K�p��̃s�N�Z��
// ���: BiLinearFilterWidthAlphaRegard() �� BiLnearFilter() �֐��p
//		 uSrcOrgX �� dwTopLeft �� dwTopRight (dwBottomLeft �� dwBottomRight) �Ԃ̈ʒu���A
//		 uSrcOrgY �� dwTop �� dwBottom �Ԃ̈ʒu�����肵�A����ꂽ�s�N�Z��(R)��Ԃ�
//
//	  dwTopLeft   dwTopRight
//		   +-------+
//		   |  |    |
//		   |--+    |
//		   |  R    |
//		   |	   |
//		   +-------+
// dwBottomLeft   dwBottomRight
///////////////////////////////////////////////////////////////////////////////////////////

DWORD CNxCustomDraw32::BiLinearPixel(UINT uSrcOrgX, UINT uSrcOrgY,
									 DWORD dwTopLeft, DWORD dwTopRight,
									 DWORD dwBottomLeft, DWORD dwBottomRight)
{
	UINT uSrcX = uSrcOrgX >> 24;	// ���� 8bit �݂̂��g�p
	UINT uSrcY = uSrcOrgY >> 24;

	DWORD dwLeft;
	DWORD dwRight;
	DWORD dwTop;
	DWORD dwBottom;

	dwLeft = dwTopLeft & 0x00ff00ff;
	dwRight = dwTopRight & 0x00ff00ff;
	dwTop = ((((dwRight - dwLeft) * uSrcX) >> 8) + dwLeft) & 0x00ff00ff;
	dwLeft = dwBottomLeft & 0x00ff00ff;
	dwRight = dwBottomRight & 0x00ff00ff;
	dwBottom = ((((dwRight - dwLeft) * uSrcX) >> 8) + dwLeft) & 0x00ff00ff;
	DWORD dwBlueRed = ((((dwBottom - dwTop) * uSrcY) >> 8) + dwTop) & 0x00ff00ff;

	dwLeft = (dwTopLeft >> 8) & 0x00ff00ff;
	dwRight = (dwTopRight >> 8) & 0x00ff00ff;
	dwTop = ((((dwRight - dwLeft) * uSrcX) >> 8) + dwLeft) & 0x00ff00ff;
	dwLeft = (dwBottomLeft >> 8) & 0x00ff00ff;
	dwRight = (dwBottomRight >> 8) & 0x00ff00ff;
	dwBottom = ((((dwRight - dwLeft) * uSrcX) >> 8) + dwLeft) & 0x00ff00ff;
	DWORD dwAlphaGreen = ((((dwBottom - dwTop) * uSrcY) >> 8) + dwTop) & 0x00ff00ff;
	return dwBlueRed | (dwAlphaGreen << 8);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static DWORD CNxCustomDraw32::BiLinearFilter32(UINT uSrcOrgX, UINT uSrcOrgY,
//												   const BYTE* lpSrcSurface, LONG lSrcPitch)
// �T�v: �]�����T�[�t�F�X�̎��� 4pixel �ɂ��āA�o�C���j�A�t�B���^��K�p�������ʂ�Ԃ�(32bpp �p)
//		 �A���t�@�`�����l���l0�̃s�N�Z���ɑ΂��ē��ꏈ�����s��(�������A�܂�������...)
// ����: UINT uSrcOrgX ... X ���W�̏�����(UINT_MIN �` UINT_MAX)
//		 UINT uSrcOrgY ... Y ���W�̏�����(UINT_MIN �` UINT_MAX)
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 LONG lSrcPitch ... �]�����T�[�t�F�X�̕�
// �ߒl: �t�B���^�K�p��̃s�N�Z��
//////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CNxCustomDraw32::BiLinearFilter32(UINT uSrcOrgX, UINT uSrcOrgY,
									    const BYTE* lpSrcSurface, LONG lSrcPitch)
{
	DWORD dwTopLeft = *reinterpret_cast<const DWORD*>(lpSrcSurface);
	DWORD dwTopRight = *reinterpret_cast<const DWORD*>(lpSrcSurface + 4);
	if ((dwTopLeft >> 24) == 0)
		dwTopLeft = dwTopRight & 0x00ffffff;
	else if ((dwTopRight >> 24) == 0)
		dwTopRight = dwTopLeft & 0x00ffffff;

	// ���� & �E��
	DWORD dwBottomLeft = *reinterpret_cast<const DWORD*>(lpSrcSurface + lSrcPitch);
	DWORD dwBottomRight = *reinterpret_cast<const DWORD*>(lpSrcSurface + lSrcPitch + 4);
	if ((dwBottomLeft >> 24) == 0)
		dwBottomLeft = dwBottomRight & 0x00ffffff;
	else if ((dwBottomRight >> 24) == 0)
		dwBottomRight = dwBottomLeft & 0x00ffffff;

	// �㉺
	if (((dwTopLeft | dwTopRight) >> 24) == 0)
	{
		dwTopLeft = dwBottomLeft & 0x00ffffff;
		dwTopRight = dwBottomRight & 0x00ffffff;
	}
	else if (((dwBottomLeft | dwBottomRight) >> 24) == 0)
	{
		dwBottomLeft = dwTopLeft & 0x00ffffff;
		dwBottomRight = dwTopRight & 0x00ffffff;
	}
	return BiLinearPixel(uSrcOrgX, uSrcOrgY, dwTopLeft, dwTopRight, dwBottomLeft, dwBottomRight);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static DWORD CNxCustomDraw32::BiLinearFilterNoRegardAlpha32(UINT uSrcOrgX, UINT uSrcOrgY,
//															    const BYTE* lpSrcSurface, LONG lSrcPitch)
// �T�v: �]�����T�[�t�F�X�̎��� 4pixel �ɂ��āA�o�C���j�A�t�B���^��K�p�������ʂ�Ԃ�(32bpp �p)
// ����: UINT uSrcOrgX ... X ���W�̏�����(UINT_MIN �` UINT_MAX)
//		 UINT uSrcOrgY ... Y ���W�̏�����(UINT_MIN �` UINT_MAX)
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 LONG lSrcPitch ... �]�����T�[�t�F�X�̕�
// �ߒl: �t�B���^�K�p��̃s�N�Z��
// ���l: �A���t�@�ɂ��Ă����Z�͓K�p�����B�������AA = 0 �ɑ΂��ĉ����s�Ȃ�Ȃ�
///////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CNxCustomDraw32::BiLinearFilterNoRegardAlpha32(UINT uSrcOrgX, UINT uSrcOrgY,
													 const BYTE* lpSrcSurface, LONG lSrcPitch)
{
	DWORD dwTopLeft = *reinterpret_cast<const DWORD*>(lpSrcSurface);
	DWORD dwTopRight = *reinterpret_cast<const DWORD*>(lpSrcSurface + 4);
	DWORD dwBottomLeft = *reinterpret_cast<const DWORD*>(lpSrcSurface + lSrcPitch);
	DWORD dwBottomRight = *reinterpret_cast<const DWORD*>(lpSrcSurface + lSrcPitch + 4);
	return BiLinearPixel(uSrcOrgX, uSrcOrgY, dwTopLeft, dwTopRight, dwBottomLeft, dwBottomRight);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static DWORD CNxCustomDraw32::BiLinearFilterNoRegardAlpha8(UINT uSrcOrgX, UINT uSrcOrgY,
//															   const BYTE* lpSrcSurface, LONG lSrcPitch)
// �T�v: �]�����T�[�t�F�X�̎��� 4pixel �ɂ��āA�o�C���j�A�t�B���^��K�p�������ʂ�Ԃ�(8bpp �p)
// ����: UINT uSrcOrgX ... X ���W�̏�����(UINT_MIN �` UINT_MAX)
//		 UINT uSrcOrgY ... Y ���W�̏�����(UINT_MIN �` UINT_MAX)
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 LONG lSrcPitch ... �]�����T�[�t�F�X�̕�
// �ߒl: �t�B���^�K�p��̃s�N�Z��
// ���l: �A���t�@�ɂ��Ă����Z�͓K�p�����B�������AA = 0 �ɑ΂��ĉ����s�Ȃ�Ȃ�
///////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CNxCustomDraw32::BiLinearFilterNoRegardAlpha8(UINT uSrcOrgX, UINT uSrcOrgY,
													const BYTE* lpSrcSurface, LONG lSrcPitch, const NxColor* pColorTable)
{
	DWORD dwTopLeft  = *(pColorTable + *(lpSrcSurface + 0));
	DWORD dwTopRight = *(pColorTable + *(lpSrcSurface + 1));
	DWORD dwBottomLeft  = *(pColorTable + *(lpSrcSurface + lSrcPitch + 0));
	DWORD dwBottomRight = *(pColorTable + *(lpSrcSurface + lSrcPitch + 1));
	return BiLinearPixel(uSrcOrgX, uSrcOrgY, dwTopLeft, dwTopRight, dwBottomLeft, dwBottomRight);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxCustomDraw32::PreprocessStretchBlt(CNxSurface::StretchBltInfo* pStretchBltInfo,
//											   LPRECT lpDestRect, const CNxSurface* pSrcSurface,
//											   LPRECT lpSrcRect, LPBOOL pbFilter)
// �T�v: �L�k Blt �̑O����
//       ���j�A�t�B���^�[���g�p�\���ۂ��𔻕ʂ��A�N���b�v���s��
// ����: CNxSurface::StretchBltInfo* pStretchBltInfo ... �L�k Blt �̒ǉ������󂯎��\����
//       LPRECT lpDestRect			   ... �]�����`
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X
//       LPRECT lpSrcRect			   ... �]�����N���b�v��`
//       LPBOOL bFilter				   ... ���j�A�t�B���^���g�p����Ȃ�� TRUE
// �ߒl: �N���b�v�̌��ʓ]����`����ɂȂ����Ȃ�� FALSE
//       *pbFilter �͎��ۂɃ��j�A�t�B���^���g�p�\�Ȃ�� TRUE�A�g�p�ł��Ȃ��Ȃ�� FALSE �֏�����������
///////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::PreprocessStretchBlt(const CNxSurface* pDestSurface, CNxSurface::StretchBltInfo* pStretchBltInfo,
										   LPRECT lpDestRect, const CNxSurface* pSrcSurface,
										   LPRECT lpSrcRect, LPBOOL pbFilter)
{
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	
	// �o�C���j�A�t�B���^���g�p����Ȃ�΁A�]�����̎Q�Ɣ͈͂���������
	// �������A�]�����̍������͕��� 1 dot �ł��邩�A
	// �g�嗦�� 0.5�{(�܂�)�ȉ��Ȃ�΁A�t�B���^�͎g�p���Ȃ�

	if (*pbFilter)
	{
		// �N���b�v�O�̓]�����Ɠ]����̃T�C�Y
		UINT uSrcWidth = abs(lpSrcRect->right - lpSrcRect->left);
		UINT uSrcHeight = abs(lpSrcRect->bottom - lpSrcRect->top);
		UINT uDestWidth = lpDestRect->right - lpDestRect->left;
		UINT uDestHeight = lpDestRect->bottom - lpDestRect->top;
		
		if (uSrcWidth > 1 && uSrcHeight > 1 && uDestWidth > (uSrcWidth / 2) && uDestHeight > (uSrcHeight / 2))
		{	// filter ��K�p����
			lpSrcRect->right--;
			lpSrcRect->bottom--;
			rcSrcClip.right--;
			rcSrcClip.bottom--;
			return pDestSurface->ClipStretchBltRect(*lpDestRect, *lpSrcRect, rcSrcClip, *pStretchBltInfo);
		}
	}

	// no filter
	*pbFilter = FALSE;
	return pDestSurface->ClipStretchBltRect(*lpDestRect, *lpSrcRect, rcSrcClip, *pStretchBltInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_RuleBlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
//														 LONG lDestDistance, LONG lSrcDistance, LONG lpRuleDistance,
//														 UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256])
// �T�v: �]�����A���t�@���g�p���Ȃ� rule �摜�g�p�ʏ�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpbRuleSurface ... rule �摜�T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lRuleDistance 		... 
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const BYTE byRuleToOpacityTable[256]
// ���l: rule �摜 (lpRuleSurface) ���� 1byte ��ǂݍ��݁A����� byRuleToOpacityTable �ŕϊ��������̂�
//		 �]�����s�N�Z���̕s�����x�Ƃ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_RuleBlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
											  LONG lDestDistance, LONG lSrcDistance, LONG lRuleDistance,
											  UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256])
{
	if (lpRuleSurface != NULL)
	{	// ���[���摜�w�肠��
		do
		{
			UINT uColumn = uWidth;
			do
			{
				*reinterpret_cast<LPDWORD>(lpDestSurface) =
					NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
									  *reinterpret_cast<const DWORD*>(lpSrcSurface), byRuleToOpacityTable[*lpRuleSurface]);
				lpDestSurface += 4;
				lpSrcSurface += 4;
				lpRuleSurface++;
			} while (--uColumn != 0);
			lpDestSurface += lDestDistance;
			lpSrcSurface += lSrcDistance;
			lpRuleSurface += lRuleDistance;
		} while (--uHeight != 0);
	}
	else
	{	// ���[���摜�w��Ȃ�
		do
		{
			UINT uColumn = uWidth;
			do
			{
				*reinterpret_cast<LPDWORD>(lpDestSurface) =
					NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
									  *reinterpret_cast<const DWORD*>(lpSrcSurface), byRuleToOpacityTable[*(lpSrcSurface + 3)]);
				lpDestSurface += 4;
				lpSrcSurface += 4;
			} while (--uColumn != 0);
			lpDestSurface += lDestDistance;
			lpSrcSurface += lSrcDistance;
		} while (--uHeight != 0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_RuleBlendNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
//														 LONG lDestDistance, LONG lSrcDistance, LONG lpRuleDistance,
//														 UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256])
// �T�v: �]�����A���t�@���g�p���Ȃ� rule �摜�g�p�ʏ�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpbRuleSurface ... rule �摜�T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lRuleDistance 		... 
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const BYTE byRuleToOpacityTable[256]
// ���l: rule �摜 (lpRuleSurface) ���� 1byte ��ǂݍ��݁A����� byRuleToOpacityTable �ŕϊ��������̂�
//		 �]�����s�N�Z���̕s�����x�Ƃ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_RuleBlendNormal_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/, const BYTE* /*lpRuleSurface*/,
																LONG /*lDestDistance*/, LONG /*lSrcDistance*/, LONG /*lRuleDistance*/,
																UINT /*uWidth*/, UINT /*uHeight*/, const BYTE /*byRuleToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		const BYTE* lpRuleSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		LONG		lRuleDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbRuleToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, [esp]StackFrame.lpRuleSurface
		mov			edx, [esp]StackFrame.lpbRuleToOpacityTable

		or			esi, esi
		jz			no_rule_surface

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

//has_rule_surface:
		align		8

loop_x_qword:
		
		movq		mm2, [ebp]
//		movzx		eax, byte ptr [esi + 0]		; for AMD K6
		_emit 0x0f
		_emit 0xb6
		_emit 0x46
		_emit 0x00
		movq		mm3, mm2
		movzx		ebx, byte ptr [esi + 1]
		movq		mm4, [edi]
		mov			al, byte ptr [edx + eax]
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		mov			bl, byte ptr [edx + ebx]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		add			esi, 2
		movq		mm6, [dwlMMXAlphaMultiplierTable + eax * 8]
		punpckhbw	mm5, mm0
		movq		mm7, [dwlMMXAlphaMultiplierTable + ebx * 8]
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm6
		psrlw		mm2, 8
		pmullw		mm3, mm7
		paddb		mm2, mm4
		psrlw		mm3, 8
		paddb		mm3, mm5
		add			ebp, 8
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
//		movzx		eax, byte ptr [esi + 0]
		_emit 0x0f
		_emit 0xb6
		_emit 0x46
		_emit 0x00
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm4, mm0
		inc			esi
		psubw		mm2, mm4
		add			ebp, 4
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			esi, [esp]StackFrame.lRuleDistance
		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret

no_rule_surface:
		// ���l��]�����T�[�t�F�X���Q��
		mov			esi, offset dwlMMXAlphaMultiplierTable

loop_y_n:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword_n
		
		align		8
loop_x_qword_n:
		movq		mm2, [ebp]
		movq		mm3, mm2
		movzx		eax, byte ptr [ebp + 3]
		movzx		ebx, byte ptr [ebp + 7]
		punpcklbw	mm2, mm0
		movq		mm4, [edi]
		movzx		eax, byte ptr [edx + eax]
		punpckhbw	mm3, mm0
		movzx		ebx, byte ptr [edx + ebx]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		movq		mm6, [esi + eax * 8]
		punpckhbw	mm5, mm0
		movq		mm7, [esi + ebx * 8]
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm6
		psrlw		mm2, 8
		pmullw		mm3, mm7
		paddb		mm2, mm4
		psrlw		mm3, 8
		paddb		mm3, mm5
		add			ebp, 8
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword_n

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end_n

skip_x_qword_n:

		movd		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm4, mm0
		psubw		mm2, mm4
		add			ebp, 4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4
	
loop_x_end_n:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y_n

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_RuleBlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
//																 LONG lDestDistance, LONG lSrcDistance, LONG lpRuleDistance,
//																 UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256])
// �T�v: �]�����A���t�@���g�p���� rule �摜�g�p�ʏ�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpbRuleSurface ... rule �T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lRuleDistance 		... 
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const BYTE byRuleToOpacityTable[256]
// ���l: rule �摜 (lpRuleSurface) ���� 1byte ��ǂݍ��݁A����� byRuleToOpacityTable �ŕϊ��������̂�
//		 �]�����s�N�Z���̕s�����x�Ƃ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_RuleBlendNormalSrcAlpha_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
													  LONG lDestDistance, LONG lSrcDistance, LONG lRuleDistance,
													  UINT uWidth, UINT uHeight, const BYTE byRuleToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface),
								  ConstTable::bySrcAlphaToOpacityTable[byRuleToOpacityTable[*lpRuleSurface]][*(lpSrcSurface + 3)]);
			lpDestSurface += 4;
			lpSrcSurface += 4;
			lpRuleSurface++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
		lpRuleSurface += lRuleDistance;
	} while (--uHeight != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_RuleBlendSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, const BYTE* lpRuleSurface,
//																 LONG lDestDistance, LONG lSrcDistance, LONG lpRuleDistance,
//																 UINT uWidth, UINT uHeight, const BYTE* lpbRuleToOpacityTable[256])
// �T�v: �]�����A���t�@���g�p����p�ʏ�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpbRuleSurface ... rule �摜�T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lRuleDistance 		... 
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const BYTE* lpbRuleToOpacityTable
// ���l: rule �摜 (lpRuleSurface) ���� 1byte ��ǂݍ��݁A����� lpbRuleToOpacityTable �ŕϊ��������̂�
//		 �]�����A���t�@�l���|���� 255 �ŏ��Z�����l���ŏI�I�ȕs�����x�Ƃ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_RuleBlendNormalSrcAlpha_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/, const BYTE* /*lpRuleSurface*/,
																		LONG /*lDestDistance*/, LONG /*lSrcDistance*/, LONG /*lRuleDistance*/,
																		UINT /*uWidth*/, UINT /*uHeight*/, const BYTE /*byRuleToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		const BYTE* lpRuleSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		LONG		lRuleDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbRuleToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, [esp]StackFrame.lpRuleSurface
		mov			edx, [esp]StackFrame.lpbRuleToOpacityTable

	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		
		align		8

loop_x_qword:
		
//		movzx		eax, byte ptr [esi + 0]
		_emit 0x0f
		_emit 0xb6
		_emit 0x46
		_emit 0x00
		movq		mm2, [ebp]
		movzx		ebx, byte ptr [esi + 1]
		movq		mm3, mm2
		mov			ah, byte ptr [edx + eax]
		movq		mm4, [edi]
		mov			bh, byte ptr [edx + ebx]
		movq		mm5, mm4
		mov			al, byte ptr [ebp + 3]
		punpcklbw	mm2, mm0
		mov			bl, byte ptr [ebp + 7]
		punpckhbw	mm3, mm0
		add			ebp, 8
		movzx		eax, byte ptr [bySrcAlphaToOpacityTable + eax]
		punpcklbw	mm4, mm0
		movzx		ebx, byte ptr [bySrcAlphaToOpacityTable + ebx]
		punpckhbw	mm5, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		movq		mm6, [dwlMMXAlphaMultiplierTable + eax * 8]
		add			esi, 2
		movq		mm7, [dwlMMXAlphaMultiplierTable + ebx * 8]
		pmullw		mm2, mm6
		pmullw		mm3, mm7
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

//		movzx		eax, byte ptr [esi + 0]
		_emit 0x0f
		_emit 0xb6
		_emit 0x46
		_emit 0x00
		movd		mm2, [ebp]
		mov			ah, byte ptr [edx + eax]
		movd		mm4, [edi]
		mov			al, byte ptr [ebp + 3]
		punpcklbw	mm2, mm0
		add			ebp, 4
		punpcklbw	mm4, mm0
		movzx		eax, byte ptr [bySrcAlphaToOpacityTable + eax]
		psubw		mm2, mm4
		inc			esi
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		add			esi, [esp]StackFrame.lRuleDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw32::Blt_RuleBlend(CNxSurface* pDestSurface,const RECT* lpDestRect,
//												const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												const NxBlt* pNxBlt) const
// �T�v: Rule �摜�g�p�u�����h Blt
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBtFx		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_RuleBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
									const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (pNxBlt->nxbRule.uLevel == 256)
	{	// uLevel �� 256 �̏ꍇ�̓��[���摜����
		if (pNxBlt->dwFlags & NxBlt::srcAlpha)
			return CNxCustomDraw32::Blt_BlendSrcAlpha(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
		else
			return CNxCustomDraw32::Blt_Blend(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
	}

	if (IsStretch(lpDestRect, lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "�g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	const NxBlt::NxRule& nxr = pNxBlt->nxbRule;

	if (nxr.uLevel > 512)
	{
		_RPTF0(_CRT_ASSERT, "nxbRule.uLevel �͈̔͂� 0 �` 512 �ł�.\n");
		return FALSE;
	}
	if (nxr.uVague > 511)
	{
		_RPTF0(_CRT_ASSERT, "nxbRule.nVague �͈̔͂� 0 �` 511 �ł�.\n");
		return FALSE;
	}
	if (pSrcSurface->GetBitCount() != 32)
	{
		_RPTF0(_CRT_ASSERT, "�]�����T�[�t�F�X�� 32bpp �ł͂���܂���.\n");
		return FALSE;
	}

	// uLevel �� 512(�̔{��)���A0 �Ȃ�Ή����\�����Ȃ�
	if (pNxBlt->nxbRule.uLevel % 512 == 0)
		return TRUE;

	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestSrcAlpha() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;
	
	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;

	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);

	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;
	
	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	POINT ptRule;
	ptRule.y = rcSrc.top + nxr.ptOffset.y;
	ptRule.x = rcSrc.left + nxr.ptOffset.x;
	
	if ((pNxBlt->dwFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
	{
		_RPTF0(_CRT_ASSERT, "�ʏ�u�����h�ȊO�̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	// rule �摜����̕s�����x�ϊ��e�[�u�����쐬(�]�����s�����x(uOpacity) �K�p��ς�)
	UINT u = 0;
	BYTE byRuleToOpacityTable[256];
	UINT uVague = nxr.uVague;

	UINT uLevel;
	BYTE byXOR;
	if (nxr.uLevel > 256)
	{	// 257 �ȏ�Ȃ�΁A�s�����x�𔽓]
		uLevel = nxr.uLevel - 256;
		byXOR = 0xff;
	}
	else
	{	// uLevel �� 0 �` 256
		uLevel = nxr.uLevel;
		byXOR = 0x00;
	}
	if (uLevel-- == 0)
		return TRUE;

	// ���[���摜����A�s�����x�֕ϊ�����ׂ̃e�[�u�����쐬
	memset(byRuleToOpacityTable, ConstTable::bySrcAlphaToOpacityTable[uOpacity][255 ^ byXOR], uLevel);		// uLevel �ȉ��͓]�����Ŋ��S�ɒu��
	for (u = uLevel;u < min(uLevel + uVague, 256); u++)
	{	// ���Ԃ̂����܂��ȕ���
		byRuleToOpacityTable[u] = ConstTable::bySrcAlphaToOpacityTable[uOpacity][static_cast<BYTE>(255 - ((u - uLevel) * 255 / uVague)) ^ byXOR];
	}
	memset(&byRuleToOpacityTable[u], ConstTable::bySrcAlphaToOpacityTable[uOpacity][byXOR], 256 - u);		// uLevel + uRange �ȏ�͓]���Ȃ�

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ���
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	// �]�����T�[�t�F�X�������ւ̃|�C���^�Ƌ���
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + pSrcSurface->GetPitch() * rcSrc.top + rcSrc.left * 4;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;

	// rule �T�[�t�F�X�������ւ̃|�C���^�Ƌ���
	// NxBlt.rule.pSurface == NULL �Ȃ��, �]�����T�[�t�F�X�̃A���t�@��p����
	const BYTE* lpRuleSurface = NULL;
	LONG lRuleDistance = 0;

	if (pNxBlt->dwFlags & NxBlt::srcAlpha)
	{
		if (nxr.pSurface == NULL || nxr.pSurface->GetBitCount() != 8)
		{
			_RPTF0(_CRT_ASSERT, "rule �摜�� 8bpp �łȂ��� NULL �ł�.\n");
			return FALSE;
		}
	}

	if (nxr.pSurface != NULL)
	{
		if ((static_cast<int>(nxr.pSurface->GetWidth()) - (nxr.ptOffset.x + rcSrc.right) |
			 static_cast<int>(nxr.pSurface->GetHeight()) - (nxr.ptOffset.y + rcSrc.bottom) |
			 ptRule.x | ptRule.y) < 0)
		{
			_RPTF0(_CRT_ASSERT, "rule �摜�̃T�C�Y������Ȃ����Arule �摜�̍��ォ��͂ݏo���Ă��܂�.\n");
			return FALSE;
		}
		lpRuleSurface = static_cast<const BYTE*>(nxr.pSurface->GetBits()) + nxr.pSurface->GetPitch() * ptRule.y + ptRule.x;
		lRuleDistance = nxr.pSurface->GetPitch() - uWidth;
	}

	if (pNxBlt->dwFlags & NxBlt::srcAlpha)
	{	// �]�����A���t�@�L��
		((CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_RuleBlendNormalSrcAlpha_MMX : Blt_RuleBlendNormalSrcAlpha_386)
			(lpDestSurface, lpSrcSurface, lpRuleSurface, lDestDistance, lSrcDistance, lRuleDistance, uWidth, uHeight, byRuleToOpacityTable);
	}
	else
	{	// �]�����A���t�@����
		((CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_RuleBlendNormal_MMX : Blt_RuleBlendNormal_386)
			(lpDestSurface, lpSrcSurface, lpRuleSurface, lDestDistance, lSrcDistance, lRuleDistance, uWidth, uHeight, byRuleToOpacityTable);
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendDestAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															  LONG lDestDistance, LONG lSrcDistance,
//															  UINT uWidth, UINT uHeight, UINT uOpacity)
// �T�v: �]����̃A���t�@���Z�𔺂��ʏ�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x (0 - 255)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendDestAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												   LONG lDestDistance, LONG lSrcDistance,
												   UINT uWidth, UINT uHeight, UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_DestAlpha(NxAlphaBlend::Normal, ConstTable::byDestAlphaResultTable,
											*reinterpret_cast<LPDWORD>(lpDestSurface), *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendDestAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															  LONG lDestDistance, LONG lSrcDistance,
//															  UINT uWidth, UINT uHeight, UINT uOpacity)
// �T�v: �]�����Ɠ]����̃A���t�@���Z�𔺂��ʏ�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendDestAlphaNormal_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																	 LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																	 UINT /*uWidth*/, UINT /*uHeight*/, UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EDI;
		LPVOID		ESI;
		LPVOID		EBP;
		LPVOID		EBX;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, offset byDestAndSrcOpacityTable
		mov			ebx, offset byDestAlphaResultTable
		movq		mm1, [dwlConst_00FF_FFFF_00FF_FFFF]

	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm2, [ebp]
		movq		mm3, mm2
		movq		mm4, [edi]
		mov			ah, byte ptr [edi + 3]
		punpcklbw	mm2, mm0
		movq		mm5, mm4
		movzx		esi, byte ptr [ebx + eax]
		punpcklbw	mm4, mm0
		punpckhbw	mm3, mm0
		movd		mm6, esi
		movzx		esi, byte ptr [edx + eax]
		punpckhbw	mm5, mm0
		psllq		mm6, 24
		psubw		mm2, mm4
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + esi * 8]
		mov			ah, byte ptr [edi + 7]
		psubw		mm3, mm5
		psrlw		mm2, 8
		movzx		esi, byte ptr [ebx + eax]
		paddb		mm2, mm4
		movd		mm7, esi
		movzx		esi, byte ptr [edx + eax]
		psllq		mm7, 56
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + esi * 8]
		psrlw		mm3, 8
		paddb		mm3, mm5
		packuswb	mm2, mm3
		por			mm6, mm7
		pand		mm2, mm1
		add			ebp, 8
		por			mm2, mm6
		movq		[edi] ,mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword

		mov			ecx, [esp]StackFrame.uWidth
		test		ecx, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		punpcklbw	mm2, mm0
		movd		mm4, [edi]
		mov			ah, byte ptr [edi + 3]
		punpcklbw	mm4, mm0
		movzx		esi, byte ptr [ebx + eax]
		psubw		mm2, mm4
		movd		mm6, esi
		movzx		esi, byte ptr [edx + eax]
		psllq		mm6, 24
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + esi * 8]
		psrlw		mm2, 8
		add			ebp, 4
		paddb		mm2, mm4
		packuswb	mm2, mm0
		pand		mm2, mm1
		por			mm2, mm6
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											 const CNxSurface *pSrcSurface, const RECT* lpSrcRect,
//											 const NxBlt *pNxBlt) const
// �T�v: �]����̃A���t�@���g�p�����u�����h Blt
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`(NULL = �T�[�t�F�X�S��)
//		 const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X
//		 const RECT* lpSrcRect		   ... �]������`(NULL = �T�[�t�F�X�S��)
//		 const NxBlt *pNxBlt		   ... �t���O�����w�肷�� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
										 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const
{
	if (pSrcSurface->GetBitCount() != 32)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestAlpha() : �]�����T�[�t�F�X�� 32bpp �ł͂���܂���.\n");
		return FALSE;
	}
	if ((pNxBlt->dwFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestAlpha() : �ʏ�u�����h�ȊO�̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	if (((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left) |
		(lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top)) != 0)
	{
		_RPTF0(_CRT_ASSERT, "�g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestAlpha() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);

	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;
	
	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	// �]�����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + pSrcSurface->GetPitch() * rcSrc.top + rcSrc.left * 4;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;

	(CNxDraw::GetInstance()->IsMMXEnabled() ?  Blt_BlendDestAlphaNormal_MMX : Blt_BlendDestAlphaNormal_386)
		(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, uOpacity);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendDestAlphaNormal_386(LPBYTE lpDestSurface, LONG lDestDistance,
//																		UINT uWidth, UINT uHeight,
//																		UINT uOpacity, DWORD dwColor)
// �T�v: �ʏ�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendDestAlphaNormal_386(LPBYTE lpDestSurface, LONG lDestDistance,
															 UINT uWidth, UINT uHeight,
															 UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_DestAlpha(NxAlphaBlend::Normal, ConstTable::byDestAlphaResultTable,
												  *reinterpret_cast<LPDWORD>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendDestAlphaNormal_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//																		UINT uWidth, UINT uHeight,
//																		UINT uOpacity, DWORD dwColor)
// �T�v: �]����A���t�@���g�p����ʏ�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendDestAlphaNormal_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																			   UINT /*uWidth*/, UINT /*uHeight*/,
																			   UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EDI;
		LPVOID		ESI;
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebp
		push		ebx
		push		esi
		push		edi

		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, offset byDestAndSrcOpacityTable
		mov			ebx, offset byDestAlphaResultTable

		punpcklbw	mm1, mm0

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword
			
		align		8

loop_x_qword:

		movq		mm2, mm1
		movq		mm4, [edi]
		movq		mm3, mm1
		mov			ah, byte ptr [edi + 3]
		movq		mm5, mm4
		movzx		ebp, byte ptr [edx + eax]
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		psubw		mm2, mm4
		movzx		esi, byte ptr [ebx + eax]
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + ebp * 8]
		movd		mm6, esi
		mov			ah, byte ptr [edi + 7]
		psllq		mm6, 24
		psubw		mm3, mm5
		psrlw		mm2, 8
		movzx		ebp, byte ptr [edx + eax]
		paddb		mm2, mm4
		movzx		esi, byte ptr [ebx + eax]
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + ebp * 8]
		movd		mm7, esi
		psrlw		mm3, 8
		psllq		mm7, 56
		paddb		mm3, mm5
		packuswb	mm2, mm3
		por			mm6, mm7
		pand		mm2, [dwlConst_00FF_FFFF_00FF_FFFF]
		add			ebp, 2
		por			mm2, mm6
		movq		[edi], mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword

		mov			ecx, [esp]StackFrame.uWidth
		test		ecx, 1
		jz			loop_x_end

skip_x_qword:

		movq		mm2, mm1
		movd		mm4, [edi]
		mov			ah, byte ptr [edi + 3]
		punpcklbw	mm4, mm0
		movzx		ebp, byte ptr [edx + eax]
		movzx		esi, byte ptr [ebx + eax]
		psubw		mm2, mm4
		movd		mm6, esi
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + ebp * 8]
		psllq		mm6, 24
		psrlw		mm2, 8
		inc			ebp
		paddb		mm2, mm4
		packuswb	mm2, mm0
		pand		mm2, [dwlConst_00FF_FFFF_00FF_FFFF]
		por			mm2, mm6
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebx
		pop			ebp
		emms
		ret
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_ColorFill_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													   const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													   const NxBlt* pNxBlt) const
// �T�v: �]����A���t�@���g�p����u�����h�h��ׂ�
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_ColorFill_BlendDestAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
												   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const
{
	if ((pNxBlt->dwFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_BlendDestAlpha() : �ʏ�u�����h�ȊO�̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	DWORD dwColor = pNxBlt->nxbColor;
	UINT uOpacity = (dwColor >> 24) & 0xff;

	if (uOpacity == 0)
		return TRUE;

	RECT rcDest = *lpDestRect;
	
	if (!pDestSurface->ClipBltRect(rcDest))
		return TRUE;

	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �������ݐ�T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	// �u�����h��� / MMX�̗L���ɂ���ĈقȂ�֐����Ăяo��
	(CNxDraw::GetInstance()->IsMMXEnabled() ? Blt_ColorFill_BlendDestAlphaNormal_MMX : Blt_ColorFill_BlendDestAlphaNormal_386)
		(lpDestSurface, lDestDistance, uWidth, uHeight, uOpacity, dwColor);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpAlphaSurface,
//																		   LONG lDestDistance, LONG lAlphaDistance,
//																		   UINT uWidth, UINT uHeight,
//																		   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �A���t�@�`�����l���T�[�t�F�X�t���ʏ�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpAlphaSurface ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpAlphaSurface,
																LONG lDestDistance, LONG lAlphaDistance,
																UINT uWidth, UINT uHeight,
																DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_DestSrcAlpha(NxAlphaBlend::Normal, ConstTable::byDestAlphaResultTable,
													 *reinterpret_cast<LPDWORD>(lpDestSurface), *lpAlphaSurface,
													 dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpAlphaSurface++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpAlphaSurface += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																		   LONG lDestDistance, LONG lAlphaDistance,
//																		   UINT uWidth, UINT uHeight,
//																		   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �A���t�@�`�����l���T�[�t�F�X�t���ʏ�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlphaNormal_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																				  LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																				  UINT /*uWidth*/, UINT /*uHeight*/,
																				  DWORD /*dwColor*/, const BYTE* /*lpbSrcAlphaToOpacityTable*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		punpcklbw	mm1, mm0

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:
		
		movq		mm2, mm1
		movzx		eax, byte ptr [ebp]
		movq		mm4, [edi]
		movq		mm3, mm1
		mov			al, byte ptr [ebx + eax]
		mov			ah, byte ptr [edi + 3]
		movq		mm5, mm4
		movzx		edx, byte ptr [byDestAndSrcOpacityTable + eax]
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		psubw		mm2, mm4
		movzx		esi, byte ptr [byDestAlphaResultTable + eax]
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + edx * 8]
		movzx		eax, byte ptr [ebp + 1]
		movd		mm6, esi
		mov			al, byte ptr [ebx + eax]
		mov			ah, byte ptr [edi + 7]
		psllq		mm6, 24
		psubw		mm3, mm5
		psrlw		mm2, 8
		movzx		edx, byte ptr [byDestAndSrcOpacityTable + eax]
		movzx		esi, byte ptr [byDestAlphaResultTable + eax]
		paddb		mm2, mm4
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + edx * 8]
		movd		mm7, esi
		psrlw		mm3, 8
		psllq		mm7, 56
		paddb		mm3, mm5
		packuswb	mm2, mm3
		por			mm6, mm7
		pand		mm2, [dwlConst_00FF_FFFF_00FF_FFFF]
		add			ebp, 2
		por			mm2, mm6
		movq		[edi], mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp]
		movq		mm2, mm1
		movd		mm4, [edi]
		mov			al, byte ptr [ebx + eax]
		mov			ah, byte ptr [edi + 3]
		punpcklbw	mm4, mm0
		movzx		edx, byte ptr [byDestAndSrcOpacityTable + eax]
		movzx		esi, byte ptr [byDestAlphaResultTable + eax]
		psubw		mm2, mm4
		movd		mm6, esi
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + edx * 8]
		psllq		mm6, 24
		psrlw		mm2, 8
		inc			ebp
		paddb		mm2, mm4
		packuswb	mm2, mm0
		pand		mm2, [dwlConst_00FF_FFFF_00FF_FFFF]
		por			mm2, mm6
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lAlphaDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//														  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//														  const NxBlt* pNxBlt) const
// �T�v: �]����Ɠ]�����̃A���t�@���g�p����u�����h�h��ׂ�
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����A���t�@�`�����l���T�[�t�F�X(8bpp) �ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]�����A���t�@�`�����l����`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
													  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (pSrcSurface->GetBitCount() != 8)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlpha() : �A���t�@�`�����l���T�[�t�F�X�� 8bpp �ł͂���܂���.\n");
		return FALSE;
	}

	if ((pNxBlt->dwFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_BlendDestSrcAlpha() : �ʏ�u�����h�ȊO�̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	if (((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left) |
		(lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top)) != 0)
	{
		_RPTF0(_CRT_ASSERT, "�g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	DWORD dwColor = pNxBlt->nxbColor;
	UINT uOpacity = (dwColor >> 24) & 0xff;

	if (uOpacity == 0)
		return TRUE;

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;

	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);

	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	// �]����(�A���t�@�`�����l��)�T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * pSrcSurface->GetPitch() + rcSrc.left;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth;

	(CNxDraw::GetInstance()->IsMMXEnabled() ? Blt_ColorFill_BlendDestSrcAlphaNormal_MMX : Blt_ColorFill_BlendDestSrcAlphaNormal_386)
		(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, dwColor, ConstTable::bySrcAlphaToOpacityTable[uOpacity]);

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendDestSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																 LONG lDestDistance, LONG lSrcDistance,
//																 UINT uWidth, UINT uHeight,
//																 const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����Ɠ]����̃A���t�@���Z�𔺂��ʏ�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendDestSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
													  LONG lDestDistance, LONG lSrcDistance,
													  UINT uWidth, UINT uHeight,
													  const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_DestSrcAlpha(NxAlphaBlend::Normal, ConstTable::byDestAndSrcOpacityTable, *reinterpret_cast<LPDWORD>(lpDestSurface),
											   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendDestSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																 LONG lDestDistance, LONG lSrcDistance,
//																 UINT uWidth, UINT uHeight,
//																 const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����Ɠ]����̃A���t�@���Z�𔺂��ʏ�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendDestSrcAlphaNormal_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																		LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																		UINT /*uWidth*/, UINT /*uHeight*/,
																		const BYTE /*bySrcAlphaToOpcityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		movq		mm1, [dwlConst_00FF_FFFF_00FF_FFFF]

	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movq		mm3, mm2
		movq		mm4, [edi]
		mov			al, byte ptr [edx + eax]
		mov			ah, byte ptr [edi + 3]
		punpcklbw	mm2, mm0
		movq		mm5, mm4
		movzx		ebx, byte ptr [byDestAndSrcOpacityTable + eax]
		punpcklbw	mm4, mm0
		punpckhbw	mm3, mm0
		punpckhbw	mm5, mm0
		psubw		mm2, mm4
		movzx		esi, byte ptr [byDestAlphaResultTable + eax]
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + ebx * 8]
		movzx		eax, byte ptr [ebp + 7]
		movd		mm6, esi
		mov			al, byte ptr [edx + eax]
		mov			ah, byte ptr [edi + 7]
		psllq		mm6, 24
		psubw		mm3, mm5
		psrlw		mm2, 8
		movzx		ebx, byte ptr [byDestAndSrcOpacityTable + eax]
		movzx		esi, byte ptr [byDestAlphaResultTable + eax] 
		paddb		mm2, mm4
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + ebx * 8]
		movd		mm7, esi
		psrlw		mm3, 8
		psllq		mm7, 56
		paddb		mm3, mm5
		packuswb	mm2, mm3
		por			mm6, mm7
		pand		mm2, mm1
		add			ebp, 8
		por			mm2, mm6
		movq		[edi] ,mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		punpcklbw	mm2, mm0
		movd		mm4, [edi]
		mov			al, byte ptr [edx + eax]
		mov			ah, byte ptr [edi + 3]
		punpcklbw	mm4, mm0
		movzx		ebx, byte ptr [byDestAndSrcOpacityTable + eax]
		movzx		esi, byte ptr [byDestAlphaResultTable + eax]
		psubw		mm2, mm4
		movd		mm6, esi
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + ebx * 8]
		psllq		mm6, 24
		psrlw		mm2, 8
		add			ebp, 4
		paddb		mm2, mm4
		packuswb	mm2, mm0
		pand		mm2, mm1
		por			mm2, mm6
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												const NxBlt* pNxBlt) const
// �T�v: �]�����Ɠ]����̃A���t�@���g�p�����u�����h Blt
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`(NULL = �T�[�t�F�X�S��)
//		 const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X
//		 const RECT* lpSrcRect		   ... �]������`(NULL = �T�[�t�F�X�S��)
//		 const NxBlt* pNxBlt		   ... �t���O�����w�肷�� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_BlendDestSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
											const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const
{
	if (pSrcSurface->GetBitCount() != 32)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestSrcAlpha() : �]�����T�[�t�F�X�� 32bpp �ł͂���܂���.\n");
		return FALSE;
	}
	if ((pNxBlt->dwFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestSrcAlpha() : �ʏ�u�����h�ȊO�̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	if (((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left) |
		(lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top)) != 0)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestSrcAlpha() : �g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendDestSrcAlpha() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;

	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);

	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	// �]�����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + pSrcSurface->GetPitch() * rcSrc.top + rcSrc.left * 4;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;

	(CNxDraw::GetInstance()->IsMMXEnabled() ?  Blt_BlendDestSrcAlphaNormal_MMX : Blt_BlendDestSrcAlphaNormal_386)
		(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, ConstTable::bySrcAlphaToOpacityTable[uOpacity]);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																 LONG lDestDistance, LONG lSrcDistance,
//																 UINT uWidth, UINT uHeight,
//																 const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h Blt (386��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												  LONG lDestDistance, LONG lSrcDistance,
												  UINT uWidth, UINT uHeight,
												  const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															LONG lDestDistance, LONG lSrcDistance,
//															UINT uWidth, UINT uHeight,
//															const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaNormal_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																	LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																	UINT /*uWidth*/, UINT /*uHeight*/,
																	const BYTE /*bySrcAlphaToOpcityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable

	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movq		mm3, mm2
		movzx		ebx, byte ptr [ebp + 7]
		punpcklbw	mm2, mm0
		movq		mm4, [edi]
		movzx		eax, byte ptr [edx + eax]
		punpckhbw	mm3, mm0
		movzx		ebx, byte ptr [edx + ebx]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		movq		mm7, [esi + ebx * 8]
		add			ebp, 8
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, [esi + eax * 8]
		pmullw		mm3, mm7
		paddw		mm2, mm4
		paddw		mm3, mm5
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm4, mm0
		psubw		mm2, mm4
		add			ebp, 4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																	LONG lDestDistance, LONG lSrcPitch,
//																	UINT uWidth, UINT uHeight,
//																	const BYTE bySrcAlphaToOpacityTable[256],
//																	cosnt CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h + �g��k�� Blt (386��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 		  ... �]�����̎��̍s�ւ̃|�C���^���Z�l(��)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
														 LONG lDestDistance, LONG lSrcPitch,
														 UINT uWidth, UINT uHeight,
														 const BYTE bySrcAlphaToOpacityTable[256],
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
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *(reinterpret_cast<const DWORD*>(lpSrcSurface) + ul64SrcOrgX.HighPart), bySrcAlphaToOpacityTable);

			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		uSrcOrgY = ul64SrcOrgY.LowPart;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
	} while (--uHeight != 0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretch_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																	LONG lDestDistance, LONG lSrcDistance,
//																	UINT uWidth, UINT uHeight,
//																	const BYTE bySrcAlphaToOpacityTable[256],
//																	const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h + �g��k��Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 		  ... �]�����̎��̍s�ւ̃|�C���^���Z�l(��)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretch_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																		   LONG /*lDestDistance*/, LONG /*lSrcPitch*/,
																		   UINT /*uWidth*/, UINT /*uHeight*/,
																		   const BYTE /*bySrcAlphaToOpacityTable*/[256],
																		   const CNxSurface::StretchBltInfo* /*pStretchBltInfo*/)
{
	using namespace ConstTable;

#pragma pack(push, 4)

	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		UINT		uSrcOrgY;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcPitch;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;
	
	__asm
	{
		sub			esp, 4
		push		ebp
		push		ebx
		push		esi
		push		edi
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			edx, [esp]StackFrame.lpSrcSurface
		pxor		mm0, mm0
		mov			eax, [ebx]StretchBltInfo.uSrcOrgY
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			[esp]StackFrame.uSrcOrgY, eax

loop_y:

		mov			ebp, 0
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		mov			ecx, [esp]StackFrame.uWidth

loop_x:

		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		movd		mm2, [edx + ebp * 4]
		punpcklbw	mm2, mm0
		movzx		eax, byte ptr [edx + ebp * 4 + 3]
		movd		mm4, [edi]
		punpcklbw	mm4, mm0
		movzx		eax, byte ptr [ebx + eax]
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		psubw		mm2, mm4
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		psrlw		mm2, 8
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		paddb		mm2, mm4
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4
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
		emms
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																				LONG lDestDistance, LONG lSrcDistance,
//																				UINT uWidth, UINT uHeight,
//																				const BYTE bySrcAlphaToOpacityTable[256],
//																				cosnt CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h + �o�C���j�A�t�B���^�g��k�� Blt (386��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 		  ... �]�����̎��̍s�ւ̃|�C���^���Z�l(��)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
																	 LONG lDestDistance, LONG lSrcPitch,
																	 UINT uWidth, UINT uHeight,
																	 const BYTE bySrcAlphaToOpacityTable[256],
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
			DWORD dwFilterResult = BiLinearFilter32(ul64SrcOrgX.LowPart, uSrcOrgY,
												    lpSrcSurface + ul64SrcOrgX.HighPart * 4, lSrcPitch);
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   dwFilterResult, bySrcAlphaToOpacityTable);

			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		uSrcOrgY = ul64SrcOrgY.LowPart;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
	} while (--uHeight != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilter_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																				LONG lDestDistance, LONG lSrcDistance,
//																				UINT uWidth, UINT uHeight,
//																				const BYTE bySrcAlphaToOpacityTable[256],
//																				cosnt CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h + �o�C���j�A�t�B���^�g��k��Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 		  ... �]�����̎��̍s�ւ̃|�C���^���Z�l(��)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// ���l: ���Z���x�� 8bit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilter_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																					   LONG /*lDestDistance*/, LONG /*lSrcPitch*/,
																					   UINT /*uWidth*/, UINT /*uHeight*/,
																					   const BYTE /*bySrcAlphaToOpacityTable*/[256],
																					   const CNxSurface::StretchBltInfo* /*pStretchBltInfo*/)
{
	using namespace ConstTable;

#pragma pack(push, 4)

	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		UINT		uSrcOrgY;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcPitch;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;

	__asm
	{
		sub			esp, 4
		push		ebp
		push		ebx
		push		esi
		push		edi
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			eax, [ebx]StretchBltInfo.uSrcOrgY
		mov			edx, [esp]StackFrame.lpSrcSurface
		mov			[esp]StackFrame.uSrcOrgY, eax
		mov			edi, [esp]StackFrame.lpDestSurface

//#define USE_NO_BRANCH_VERSION 
// ��`����ƁA�O���̏����ŕ��򖽗ߖ��g�p�̃o�[�W�������g�p�����
// �œK�����Â��̂��A���򖽗߂��g�p�������������ȈׁA���݂͖���`

#if defined(USE_NO_BRANCH_VERSION)
		// ���򖳂��R�[�h
loop_y:
		mov			ebp, 0
		mov			ecx, [esp]StackFrame.uWidth
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX

loop_x:
		lea			eax, [edx + ebp * 4]
		movq		mm2, [dwlConst_00FF_FFFF_00FF_FFFF]
		movq		mm0, [eax]							// load Y
		add			eax, [esp]StackFrame.lSrcPitch
		pcmpeqb		mm3, mm3
		movq		mm1, [eax]							// load (Y + 1)
		movq		mm6, mm0							// mm6 = AARRGGBB(left)/AARRGGBB(right)
		pcmpeqb		mm0, mm2							// if alpha == 0 then, mm0 = 0xff##_####
		movq		mm7, mm1
		pcmpeqb		mm1, mm2
		psrad		mm0, 31
		psrad		mm1, 31
		pxor		mm0, mm3
		pxor		mm1, mm3
		pand		mm6, mm0
		pand		mm7, mm1
		movq		mm4, mm6
		movq		mm5, mm7
		pand		mm6, mm2
		pand		mm7, mm2
		movq		mm2, mm6
		movq		mm3, mm7
		punpckldq	mm6, mm6
		punpckldq	mm7, mm7
		punpckhdq	mm2, mm6
		movq		mm6, [dwlConst_00FF_FFFF_00FF_FFFF]
		punpckhdq	mm3, mm7
		pcmpeqb		mm7, mm7
		pandn		mm0, mm2
		pandn		mm1, mm3
		por			mm0, mm4
		por			mm1, mm5
		movq		mm4, mm0
		movq		mm5, mm1
		movq		mm2, mm0
		movq		mm3, mm1
		punpckldq	mm4, mm4
		punpckldq	mm5, mm5
		por			mm4, mm0
		por			mm5, mm1
		pcmpeqb		mm4, mm6
		pcmpeqb		mm5, mm6
		punpckhdq	mm4, mm4
		punpckhdq	mm5, mm5
		psrad		mm4, 31
		mov			eax, [esp]StackFrame.uSrcOrgY
		psrad		mm5, 31
		pxor		mm4, mm7
		pxor		mm5, mm7
		pand		mm0, mm4
		pand		mm1, mm5
		shr			eax, 24
		movq		mm2, mm0
		movq		mm3, mm1
		pand		mm0, mm6
		pand		mm1, mm6
		movd		mm6, esi
		movd		mm7, eax
		pandn		mm4, mm1
		pandn		mm5, mm0
		por			mm2, mm4
		psrld		mm6, 24
		por			mm3, mm5
		pxor		mm4, mm4
		punpcklwd	mm6, mm6
		movq		mm0, mm2
		punpckldq	mm6, mm6
		movq		mm1, mm3
		punpcklbw	mm0, mm4
		punpcklbw	mm1, mm4
		punpckhbw	mm2, mm4
		punpckhbw	mm3, mm4
		psubw		mm2, mm0
		psubw		mm3, mm1
		pmullw		mm2, mm6
		punpcklwd	mm7, mm7
		pmullw		mm3, mm6
		psrlw		mm2, 8
		punpckldq	mm7, mm7
		psrlw		mm3, 8
		paddb		mm0, mm2
		paddb		mm1, mm3
		psubw		mm1, mm0
		pmullw		mm1, mm7
		psrlw		mm1, 8
		paddb		mm0, mm1
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		packuswb	mm0, mm4
		movd		mm6, [edi]
		movd		eax, mm0
		punpcklbw	mm0, mm4
		add			edi, 4
		shr			eax, 24
		punpcklbw	mm6, mm4
		movzx		eax, byte ptr [ebx + eax]
		psubw		mm0, mm6
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		pmullw		mm0, [dwlMMXAlphaMultiplierTable + eax * 8]
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		psrlw		mm0, 8
		paddb		mm0, mm6
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		dec			ecx
		packuswb	mm0, mm4
		movd		[edi - 4], mm0
		jnz			loop_x

#else
		// ���򍬂���̃R�[�h
loop_y:

		movd		mm7, [esp]StackFrame.uSrcOrgY
		mov			ebp, 0
		psrld		mm7, 24
		mov			ecx, [esp]StackFrame.uWidth
		punpcklwd	mm7, mm7
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		punpckldq	mm7, mm7

loop_x:

		movd		mm6, esi
		lea			esi, [edx + ebp * 4]
		movd		mm4, ecx
		mov			ecx, 0ff000000h
		movd		mm5, [dwlConst_00FF_FFFF_00FF_FFFF]
//		mov			eax, [esi + 0]		; avoid vector decode (for AMD K6)
		_emit 0x8b
		_emit 0x46
		_emit 0x00
		mov			ebx, [esi + 4]
		add			esi, [esp]StackFrame.lSrcPitch
		test		eax, ecx
		jz			top_copy_right_pixel
		test		ebx, ecx
		jnz			bottom_pixel
		mov			ebx, eax
		and			ebx, 00ffffffh
		jmp			short bottom_pixel
top_copy_right_pixel:
		mov			eax, ebx
		and			eax, 00ffffffh
bottom_pixel:
		movd		mm0, eax
//		mov			eax, [esi + 0]
		_emit 0x8b
		_emit 0x46
		_emit 0x00
		movd		mm2, ebx
		mov			ebx, [esi + 4]
		test		eax, ecx
		jz			bottom_copy_right_pixel
		test		ebx, ecx
		jnz			top_down_pixel
		mov			ebx, eax
		and			ebx, 00ffffffh
		jmp			short top_down_pixel
bottom_copy_right_pixel:
		mov			eax, ebx
		and			eax, 00ffffffh
top_down_pixel:
		movd		mm1, eax
		movd		eax, mm0
		movd		mm3, ebx
		movd		ebx, mm2
		movd		esi, mm6
		or			eax, ebx
		psrld		mm6, 24
		test		eax, ecx
		punpcklwd	mm6, mm6
		jnz			top_not_zero
		movq		mm0, mm1
		movq		mm2, mm3
		pand		mm0, mm5
		pand		mm2, mm5
		jmp			short filter
top_not_zero:
		movd		eax, mm1
		movd		ebx, mm3
		or			eax, ebx
		test		eax, ecx
		jnz			filter
		movq		mm1, mm0
		movq		mm3, mm2
		pand		mm1, mm5
		pand		mm3, mm5
filter:

		punpckldq	mm0, mm2
		punpckldq	mm1, mm3
		pxor		mm5, mm5
		punpckldq	mm6, mm6
		movq		mm2, mm0
		movq		mm3, mm1
		punpcklbw	mm0, mm5
		punpcklbw	mm1, mm5
		punpckhbw	mm2, mm5
		punpckhbw	mm3, mm5
		psubw		mm2, mm0
		psubw		mm3, mm1
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm0, mm2
		paddb		mm1, mm3
		mov			ecx, [esp]StackFrame.pStretchBltInfo
		psubw		mm1, mm0
		pmullw		mm1, mm7
		psrlw		mm1, 8
		paddb		mm0, mm1
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		packuswb	mm0, mm5
		movd		mm6, [edi]
		movd		eax, mm0
		punpcklbw	mm0, mm5
		add			edi, 4
		shr			eax, 24
		punpcklbw	mm6, mm5
		movzx		eax, byte ptr [ebx + eax]
		psubw		mm0, mm6
		add			esi, [ecx]StretchBltInfo.ul64SrcDeltaX.LowPart
		pmullw		mm0, [dwlMMXAlphaMultiplierTable + eax * 8]
		adc			ebp, [ecx]StretchBltInfo.ul64SrcDeltaX.HighPart
		psrlw		mm0, 8
		mov			ebx, ecx
		movd		ecx, mm4
		paddb		mm0, mm6
		packuswb	mm0, mm5
		dec			ecx
		movd		[edi - 4], mm0
		jnz			loop_x
#endif	// #if defined(USE_NO_BRANCH_VERSION)

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
		emms
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_386(LPBYTE lpDestSurface,
//																			    const BYTE* lpSrcSurface,
//																				LONG lDestDistance, LONG lSrcDistance,
//																				UINT uWidth, UINT uHeight,
//																				const BYTE bySrcAlphaToOpacityTable[256],
//																				cosnt CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h + �o�C���j�A�t�B���^�g��k�� Blt (386��)
//		 NxBlt::noRegard �ŁBalpha == 0 �̃s�N�Z������ꈵ�����Ȃ��B�]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 		  ... �]�����̎��̍s�ւ̃|�C���^���Z�l(��)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// ���l: alpha �l�� 2 �ȉ��̏ꍇ�́Aalpha = 0 �ɂȂ�e�[�u�����g�p����BNxBlt::noRegard �������A��������
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
																				  LONG lDestDistance, LONG lSrcPitch,
																				  UINT uWidth, UINT uHeight,
																				  const BYTE bySrcAlphaToOpacityTable[256],
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
			DWORD dwFilterResult = BiLinearFilterNoRegardAlpha32(ul64SrcOrgX.LowPart, uSrcOrgY,
															     lpSrcSurface + ul64SrcOrgX.HighPart * 4, lSrcPitch);
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   dwFilterResult, bySrcAlphaToOpacityTable,
										   ConstTable::dwlMMXNoRegardAlphaMultiplierTable);

			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		uSrcOrgY = ul64SrcOrgY.LowPart;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
	} while (--uHeight != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_MMX(LPBYTE lpDestSurface,
//																			    const BYTE* lpSrcSurface,
//																				LONG lDestDistance, LONG lSrcDistance,
//																				UINT uWidth, UINT uHeight,
//																				const BYTE bySrcAlphaToOpacityTable[256],
//																				cosnt CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���ʏ�u�����h + �o�C���j�A�t�B���^�g��k�� Blt (MMX ��)
//		 NxBlt::noRegard �ŁBalpha == 0 �̃s�N�Z������ꈵ�����Ȃ��B�]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 		  ... �]�����̎��̍s�ւ̃|�C���^���Z�l(��)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// ���l: alpha �l�� 2 �ȉ��̏ꍇ�́Aalpha = 0 �ɂȂ�e�[�u�����g�p����BNxBlt::noRegard �������A��������
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_MMX(LPBYTE /*lpDestSurface*/,
																					   const BYTE* /*lpSrcSurface*/,
																					   LONG /*lDestDistance*/, LONG /*lSrcPitch*/,
																					   UINT /*uWidth*/, UINT /*uHeight*/,
																					   const BYTE /*bySrcAlphaToOpacityTable*/[256],
																					   const CNxSurface::StretchBltInfo* /*pStretchBltInfo*/)
{
	using namespace ConstTable;

#pragma pack(push, 4)

	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		UINT		uSrcOrgY;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcPitch;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;

	__asm
	{
		sub			esp, 4
		push		ebp
		push		ebx
		push		esi
		push		edi
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			edx, [esp]StackFrame.lpSrcSurface
		pxor		mm0, mm0
		mov			eax, [ebx]StretchBltInfo.uSrcOrgY
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			[esp]StackFrame.uSrcOrgY, eax
		movd		mm1, ebx
loop_y:

		movd		mm7, [esp]StackFrame.uSrcOrgY
		mov			ebp, 0
		psrld		mm7, 24
		mov			ecx, [esp]StackFrame.uWidth
		punpcklwd	mm7, mm7
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		punpckldq	mm7, mm7

loop_x:

		lea			eax, [edx + ebp * 4]
		movd		mm6, esi
		movq		mm2, [eax]
		psrld		mm6, 24
		add			eax, [esp]StackFrame.lSrcPitch
		punpcklwd	mm6, mm6
		movq		mm3, [eax]
		punpckldq	mm6, mm6
		movq		mm4, mm2
		movq		mm5, mm3
		punpcklbw	mm2, mm0
		punpcklbw	mm3, mm0
		punpckhbw	mm4, mm0
		punpckhbw	mm5, mm0
		psubw		mm4, mm2
		psubw		mm5, mm3
		pmullw		mm4, mm6
		pmullw		mm5, mm6
		psrlw		mm4, 8
		psrlw		mm5, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		psubw		mm3, mm2
		pmullw		mm3, mm7
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		psrlw		mm3, 8
		paddb		mm2, mm3
		packuswb	mm2, mm0
		movd		mm4, [edi]
		movd		eax, mm2
		punpcklbw	mm2, mm0
		add			edi, 4
		shr			eax, 24
		punpcklbw	mm4, mm0
		movzx		eax, byte ptr [ebx + eax]
		movd		ebx, mm1
		psubw		mm2, mm4
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		pmullw		mm2, [dwlMMXNoRegardAlphaMultiplierTable + eax * 8]
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		dec			ecx
		movd		[edi - 4], mm2
		jnz			loop_x

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
		emms
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															  LONG lDestDistance, LONG lSrcDistance,
//															  UINT uWidth, UINT uHeight,
//															  const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ�����Z�u�����h Blt (386 ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											   LONG lDestDistance, LONG lSrcDistance,
											   UINT uWidth, UINT uHeight,
											   const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Add, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaAdd_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															  LONG lDestDistance, LONG lSrcDistance,
//															  UINT uWidth, UINT uHeight,
//															  const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ�����Z�u�����h Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaAdd_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																 LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																 UINT /*uWidth*/, UINT /*uHeight*/,
																 const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		
	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword
			
		align		8
loop_x_qword:
		
		movq		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movq		mm3, mm2
		movzx		ebx, byte ptr [ebp + 7]
		punpcklbw	mm2, mm0
		movzx		eax, byte ptr [edx + eax]
		punpckhbw	mm3, mm0
		movzx		ebx, byte ptr [edx + ebx]
		pmullw		mm2, [esi + eax * 8]
		pmullw		mm3, [esi + ebx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		packuswb	mm2, mm3
		paddusb		mm2, [edi]
		add			ebp, 8
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp + 3]
		movd		mm2, [ebp]
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm2, mm0
		pmullw		mm2, [esi + eax * 8]
		movd		mm4, [edi]
		psrlw		mm2, 8
		packuswb	mm2, mm0
		paddusb		mm2, mm4
		add			ebp, 4
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															  LONG lDestDistance, LONG lSrcDistance,
//															  UINT uWidth, UINT uHeight,
//															  const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ��錸�Z�u�����h Blt (386 ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												   LONG lDestDistance, LONG lSrcDistance,
											   UINT uWidth, UINT uHeight,
											   const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Sub, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaSub_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															  LONG lDestDistance, LONG lSrcDistance,
//															  UINT uWidth, UINT uHeight,
//															  const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ��錸�Z�u�����h Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaSub_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																 LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																 UINT /*uWidth*/, UINT /*uHeight*/,
																 const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable

	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword
			
		align		8

loop_x_qword:
		
		movq		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movq		mm3, mm2
		movzx		ebx, byte ptr [ebp + 7]
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm2, mm0
		movq		mm4, [edi]
		movzx		ebx, byte ptr [edx + ebx]
		punpckhbw	mm3, mm0
		pmullw		mm2, [esi + eax * 8]
		pmullw		mm3, [esi + ebx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		packuswb	mm2, mm3
		psubusb		mm4, mm2
		add			ebp, 8
		movq		[edi], mm4
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp + 3]
		movd		mm4, [edi]
		movd		mm2, [ebp]
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm2, mm0
		pmullw		mm2, [esi + eax * 8]
		add			ebp, 4
		psrlw		mm2, 8
		packuswb	mm2, mm0
		psubusb		mm4, mm2
		movd		[edi], mm4
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															LONG lDestDistance, LONG lSrcDistance,
//															UINT uWidth, UINT uHeight,
//															const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ����Z�u�����h Blt (386 ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												LONG lDestDistance, LONG lSrcDistance,
												UINT uWidth, UINT uHeight,
												const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Multi, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaMulti_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															LONG lDestDistance, LONG lSrcDistance,
//															UINT uWidth, UINT uHeight,
//															const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ����Z�u�����h Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaMulti_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																   LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																   UINT /*uWidth*/, UINT /*uHeight*/,
																   const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		pxor		mm0, mm0
		push		esi
		push		edi
		push		ebp
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		mov			edi, [esp]StackFrame.lpDestSurface
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm2, [ebp]
		movq		mm3, mm2
		movq		mm4, [edi]
		movq		mm5, mm4
		movzx		eax, byte ptr [ebp + 3]
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		movzx		ebx, byte ptr [ebp + 7]
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		movzx		eax, byte ptr [edx + eax]
		psrlw		mm2, 8
		psrlw		mm3, 8
		movzx		ebx, byte ptr [edx + ebx]
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pand		mm2, mm7
		pand		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, [esi + eax * 8]
		add			ebp, 8
		pmullw		mm3, [esi + ebx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		movd		mm4, [edi]
		movzx		eax, byte ptr [ebp + 3]
		add			ebp, 4
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		movzx		eax, byte ptr [edx + eax]
		pmullw		mm2, mm4
		psrlw		mm2, 8
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pand		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4
loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebp, [esp]StackFrame.lSrcDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															 LONG lDestDistance, LONG lSrcDistance,
//															 UINT uWidth, UINT uHeight,
//															 const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���X�N���[���u�����h Blt (386 ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												  LONG lDestDistance, LONG lSrcDistance,
												  UINT uWidth, UINT uHeight,
												  const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Screen, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaScreen_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															 LONG lDestDistance, LONG lSrcDistance,
//															 UINT uWidth, UINT uHeight,
//															 const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ���X�N���[���u�����h Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaScreen_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																	LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																	UINT /*uWidth*/, UINT /*uHeight*/,
																	const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		pxor		mm0, mm0
		push		esi
		push		edi
		push		ebp
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		mov			edi, [esp]StackFrame.lpDestSurface
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm2, [ebp]
		movq		mm3, mm2
		movq		mm4, [edi]
		movq		mm5, mm4
		movzx		eax, byte ptr [ebp + 3]
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		movzx		ebx, byte ptr [ebp + 7]
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		pxor		mm2, mm7
		pxor		mm3, mm7
		pxor		mm4, mm7
		pxor		mm5, mm7
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		pxor		mm4, mm7
		pxor		mm5, mm7
		movzx		eax, byte ptr [edx + eax]
		psrlw		mm2, 8
		psrlw		mm3, 8
		movzx		ebx, byte ptr [edx + ebx]
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pandn		mm2, mm7
		pandn		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, [esi + eax * 8]
		add			ebp, 8
		pmullw		mm3, [esi + ebx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		movd		mm4, [edi]
		movzx		eax, byte ptr [ebp + 3]
		add			ebp, 4
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		pxor		mm2, mm7
		pxor		mm4, mm7
		movzx		eax, byte ptr [edx + eax]
		pmullw		mm2, mm4
		pxor		mm4, mm7
		psrlw		mm2, 8
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pandn		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebp, [esp]StackFrame.lSrcDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																   LONG lDestDistance, LONG lSrcDistance,
//																   UINT uWidth, UINT uHeight,
//																   const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ��� Brighten �u�����h Blt (386 ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
													LONG lDestDistance, LONG lSrcDistance,
													UINT uWidth, UINT uHeight,
													const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Brighten, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaBrighten_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															   LONG lDestDistance, LONG lSrcDistance,
//															   UINT uWidth, UINT uHeight,
//															   const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ��� Brighten �u�����h Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaBrighten_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																	  LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																	  UINT /*uWidth*/, UINT /*uHeight*/,
																	  const BYTE /*lpbSrcAlphaToOpcityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable

	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword
			
		align		8

loop_x_qword:
		
		movq		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movq		mm3, mm2
		movzx		ebx, byte ptr [ebp + 7]
		punpcklbw	mm2, mm0
		movq		mm4, [edi]
		movzx		eax, byte ptr [edx + eax]
		punpckhbw	mm3, mm0
		movzx		ebx, byte ptr [edx + ebx]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		movq		mm6, mm2
		pcmpgtw		mm2, mm4
		movq		mm7, mm3
		pcmpgtw		mm3, mm5
		pand		mm6, mm2
		pandn		mm2, mm4
		pand		mm7, mm3
		pandn		mm3, mm5
		por			mm2, mm6
		movq		mm6, [esi + eax * 8]
		por			mm3, mm7
		movq		mm7, [esi + ebx * 8]
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm6
		pmullw		mm3, mm7
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebp, 8
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm4, mm0
		movq		mm6, mm2
		pcmpgtw		mm2, mm4
		pand		mm6, mm2
		pandn		mm2, mm4
		por			mm2, mm6
		psubw		mm2, mm4
		add			ebp, 4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															 LONG lDestDistance, LONG lSrcDistance,
//															 UINT uWidth, UINT uHeight,
//															 const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ��� Darken �u�����h Blt (386 ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSrcAlphaDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												  LONG lDestDistance, LONG lSrcDistance,
												  UINT uWidth, UINT uHeight,
												  const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Darken, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(lpSrcSurface), bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSrcAlphaDarken_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															 LONG lDestDistance, LONG lSrcDistance,
//															 UINT uWidth, UINT uHeight,
//															 const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�݂̂��Q�Ƃ��� Darken �u�����h Blt (MMX ��)
//		 �]����̃A���t�@�͕ω����Ȃ�
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 const BYTE bySrcAlphaToOpacityTable[256]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSrcAlphaDarken_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																	LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																	UINT /*uWidth*/, UINT /*uHeight*/,
																	const BYTE /*bySrcAlphaToOpcityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable

	loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword
			
		align		8

loop_x_qword:
		
		movq		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movq		mm3, mm2
		movzx		ebx, byte ptr [ebp + 7]
		punpcklbw	mm2, mm0
		movq		mm4, [edi]
		movzx		eax, byte ptr [edx + eax]
		punpckhbw	mm3, mm0
		movzx		ebx, byte ptr [edx + ebx]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		movq		mm6, mm4
		pcmpgtw		mm6, mm2
		movq		mm7, mm5
		pcmpgtw		mm7, mm3
		pand		mm2, mm6
		pand		mm3, mm7
		pandn		mm6, mm4
		pandn		mm7, mm5
		por			mm2, mm6
		movq		mm6, [esi + eax * 8]
		por			mm3, mm7
		psubw		mm2, mm4
		movq		mm7, [esi + ebx * 8]
		pmullw		mm2, mm6
		psubw		mm3, mm5
		psrlw		mm2, 8
		pmullw		mm3, mm7
		paddb		mm2, mm4
		psrlw		mm3, 8
		paddb		mm3, mm5
		add			ebp, 8
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		movzx		eax, byte ptr [ebp + 3]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		movzx		eax, byte ptr [edx + eax]
		punpcklbw	mm4, mm0
		movq		mm6, mm4
		pcmpgtw		mm6, mm2
		pand		mm2, mm6
		pandn		mm6, mm4
		por			mm2, mm6
		psubw		mm2, mm4
		add			ebp, 4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											const CNxSurface *pSrcSurface, const RECT* lpSrcRect,
//											const NxBlt* pNxBlt) const
// �T�v: �]�����A���t�@���g�p�����u�����h Blt
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`(NULL = �T�[�t�F�X�S��)
//		 const CNxSurface *pSrcSurface ... �]�����T�[�t�F�X
//		 const RECT* lpSrcRect		   ... �]������`(NULL = �T�[�t�F�X�S��)
//		 const NxBlt* pNxBlt		   ... �t���O�����w�肷�� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
										const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const
{
	if (pSrcSurface->GetBitCount() != 32)
	{
		_RPTF0(_CRT_WARN, "CNxCustomDraw32::Blt_BlendSrcAlpha() : �]�����T�[�t�F�X�� 32bpp �ł͂���܂���. srcAlpha �𖳎����܂�.\n");
		return Blt_Blend(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
	}

	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendSrcAlpha() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	int nBlendType = pNxBlt->dwFlags & NxBlt::blendTypeMask;

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	LONG lSrcPitch = pSrcSurface->GetPitch();

	if (!IsStretch(lpDestRect, lpSrcRect))
	{
		// �L�k���Ȃ�

		// �N���b�v
		RECT rcSrcClip;
		pSrcSurface->GetRect(&rcSrcClip);
		if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;

		// ���ƍ���
		UINT uWidth = rcDest.right - rcDest.left;
		UINT uHeight = rcDest.bottom - rcDest.top;

		// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
		LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
		LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;

		// �]����
		LONG lSrcDistance = lSrcPitch - uWidth * 4;
		const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left * 4;
			
		typedef void (__cdecl *BltSrcAlphaFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												LONG lDestDistance, LONG lSrcDistance, UINT uWidth, UINT uHeight,
												const BYTE bySrcAlphaToOpacityTable[256]);

		static const BltSrcAlphaFunc bltSrcAlphaFunc[][2] =
		{
#if defined(NXDRAW_MMX_ONLY)
			// 0 ... �ʏ�
			{ Blt_BlendSrcAlphaNormal_MMX, Blt_BlendSrcAlphaNormal_MMX },
			// 1 ... ���Z
			{ Blt_BlendSrcAlphaAdd_MMX, Blt_BlendSrcAlphaAdd_MMX },
			// 2 ... ���Z
			{ Blt_BlendSrcAlphaSub_MMX, Blt_BlendSrcAlphaSub_MMX, },
			// 3 ... ��Z
			{ Blt_BlendSrcAlphaMulti_MMX, Blt_BlendSrcAlphaMulti_MMX, },
			// 4 ... �X�N���[��
			{ Blt_BlendSrcAlphaScreen_MMX, Blt_BlendSrcAlphaScreen_MMX, },
			// 5 ... Brighten
			{ Blt_BlendSrcAlphaBrighten_MMX, Blt_BlendSrcAlphaBrighten_MMX, },
			// 6 ... Darken
			{ Blt_BlendSrcAlphaDarken_MMX, Blt_BlendSrcAlphaDarken_MMX, },
#else
			// 0 ... �ʏ�
			{ Blt_BlendSrcAlphaNormal_386, Blt_BlendSrcAlphaNormal_MMX },
			// 1 ... ���Z
			{ Blt_BlendSrcAlphaAdd_386, Blt_BlendSrcAlphaAdd_MMX },
			// 2 ... ���Z
			{ Blt_BlendSrcAlphaSub_386, Blt_BlendSrcAlphaSub_MMX, },
			// 3 ... ��Z
			{ Blt_BlendSrcAlphaMulti_386, Blt_BlendSrcAlphaMulti_MMX, },
			// 4 ... �X�N���[��
			{ Blt_BlendSrcAlphaScreen_386, Blt_BlendSrcAlphaScreen_MMX, },
			// 5 ... Brighten
			{ Blt_BlendSrcAlphaBrighten_386, Blt_BlendSrcAlphaBrighten_MMX, },
			// 6 ... Darken
			{ Blt_BlendSrcAlphaDarken_386, Blt_BlendSrcAlphaDarken_MMX, },
#endif	// #if defined(NXDRAW_MMX_ONLY)
		};
		// �u�����h��� / MMX�̗L���ɂ���Ċ֐����Ăяo��
		(bltSrcAlphaFunc[nBlendType - NxBlt::blendNormal][(CNxDraw::GetInstance()->IsMMXEnabled()) ? 1 : 0])
			(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, ConstTable::bySrcAlphaToOpacityTable[uOpacity]);
	}	// if (!IsStretch(lpDestRect, lpSrcRect))
	else
	{	// �g��k������
		if (nBlendType != NxBlt::blendNormal)
		{
			_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_BlendSrcAlpha() : �g��k���́A�ʏ�u�����h�ȊO�ɂ͑Ή����Ă��܂���.\n");
			return FALSE;
		}

		void (*pfnStretchBltFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcPitch,
								  UINT uWidth, UINT uHeight, const BYTE[256], const CNxSurface::StretchBltInfo* pStretchBltInfo);

		CNxSurface::StretchBltInfo stretchBltInfo;
		BOOL bFilter = (pNxBlt != NULL) && (pNxBlt->dwFlags & NxBlt::linearFilter);
		if (!PreprocessStretchBlt(pDestSurface, &stretchBltInfo, &rcDest, pSrcSurface, &rcSrc, &bFilter))
			return TRUE;

		if (bFilter)
		{
			if (pNxBlt->dwFlags & NxBlt::noRegardAlpha)
				pfnStretchBltFunc = CNxDraw::GetInstance()->IsMMXEnabled() ? Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_MMX
										: Blt_BlendSrcAlphaNormalStretchLinearFilterNoRegardAlpha_386;
			else
				pfnStretchBltFunc = CNxDraw::GetInstance()->IsMMXEnabled() ? Blt_BlendSrcAlphaNormalStretchLinearFilter_MMX
										: Blt_BlendSrcAlphaNormalStretchLinearFilter_386;
		}
		else
		{	// no filter
			pfnStretchBltFunc = CNxDraw::GetInstance()->IsMMXEnabled() ? Blt_BlendSrcAlphaNormalStretch_MMX : Blt_BlendSrcAlphaNormalStretch_386;
		}
			
		// ���ƍ���
		UINT uWidth = rcDest.right - rcDest.left;
		UINT uHeight = rcDest.bottom - rcDest.top;

		// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
		LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
		LPBYTE lpDestSurface = (LPBYTE)pDestSurface->GetBits() + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;

		// �]����
		const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left * 4;
			
		(pfnStretchBltFunc)(lpDestSurface, lpSrcSurface, lDestDistance, lSrcPitch, uWidth, uHeight, ConstTable::bySrcAlphaToOpacityTable[uOpacity], &stretchBltInfo);
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													 LONG lDestDistance, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight,
//													 UINT uOpacity)
// �T�v: �ʏ�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
										  LONG lDestDistance, LONG lSrcDistance,
										  UINT uWidth, UINT uHeight,
										  UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendNormalFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														  LONG lDestDistance, LONG lSrcDistance,
//														  UINT uWidth, UINT uHeight,
//														  UINT uOpacity)
// �T�v: 8bpp ����̒ʏ�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 const NxColor* pColorTable ... �J���[�e�[�u��(�p���b�g)�ւ̃|�C���^
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendNormalFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											   LONG lDestDistance, LONG lSrcDistance,
											   const NxColor* pColorTable, UINT uWidth, UINT uHeight,
											   UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *(pColorTable + *lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													 LONG lDestDistance, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight,
//													 UINT uOpacity)
// �T�v: �ʏ�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendNormal_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
															LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
															UINT /*uWidth*/, UINT /*uHeight*/,
															UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ebx;
		LPVOID		edi;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		edi
		pxor		mm0, mm0
		mov			eax, [esp]StackFrame.uOpacity
		mov			ebx, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm2, [ebx]
		movq		mm3, mm2
		punpcklbw	mm2, mm0
		movq		mm4, [edi]
		punpckhbw	mm3, mm0
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebx, 8
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebx]
		punpcklbw	mm2, mm0
		movd		mm4, [edi]
		punpcklbw	mm4, mm0
		psubw		mm2, mm4
		pmullw		mm2, mm1
		add			ebx, 4
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebx, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			edi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendNormalFrom8_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														  LONG lDestDistance, LONG lSrcDistance,
//														  const NxColor* pColorTable, UINT uWidth, UINT uHeight,
//														  UINT uOpacity)
// �T�v: 8bpp ����̒ʏ�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 const NxColor* pColorTable ... �J���[�e�[�u��(�p���b�g)�ւ̃|�C���^
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendNormalFrom8_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																 LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																 const NxColor* /*pColorTable*/, UINT /*uWidth*/, UINT /*uHeight*/,
																 UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		esi;
		LPVOID		edi;
		LPVOID		ebp;
		LPVOID		ebx;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		const NxColor* pColorTable;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		edi
		push		esi
		pxor		mm0, mm0
		mov			eax, [esp]StackFrame.uOpacity
		mov			ebx, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		mov			ebp, [esp]StackFrame.pColorTable

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movzx		eax, byte ptr [ebx]
		movzx		esi, byte ptr [ebx + 1]
		movq		mm4, [edi]
		movd		mm2, [ebp + eax * 4]
		movd		mm3, [ebp + esi * 4]
		movq		mm5, mm4
		punpcklbw	mm2, mm0
		punpcklbw	mm3, mm0
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebx, 2
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			ecx, [esp]StackFrame.uWidth
		test		ecx, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebx]
		movd		mm2, [ebp + eax * 4]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		psubw		mm2, mm4
		pmullw		mm2, mm1
		inc			ebx
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebx, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			esi
		pop			edi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
// void CNxCustomDraw32::Blt_BlendNormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													LONG lDestDistance, LONG lSrcPitch,
//													UINT uWidth, UINT uHeight, UINT uOpacity,
//													const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �ʏ�u�����h + �g��k�� Blt (386)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 UINT uOpacity				... �]�����s�����x(0 �` 255)
//		 ULARGE_INTEGER ul64SrcOrgY ... �]���� X ���_ * 4294967296
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendNormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
												 LONG lDestDistance, LONG lSrcPitch,
												 UINT uWidth, UINT uHeight, UINT uOpacity,
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
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *(reinterpret_cast<const DWORD*>(lpSrcSurface) + ul64SrcOrgX.HighPart), uOpacity);
			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		uSrcOrgY = ul64SrcOrgY.LowPart;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
	} while (--uHeight != 0);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
// void CNxCustomDraw32::Blt_BlendNormalStretchFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														 LONG lDestDistance, LONG lSrcPitch, const NxColor* pColorTable,
//														 UINT uWidth, UINT uHeight, UINT uOpacity,
//														 const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: 8bpp ����̒ʏ�u�����h + �g��k�� Blt (386)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 const NxColor* pColorTable ... �J���[�e�[�u���ւ̃|�C���^
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 UINT uOpacity				... �]�����s�����x(0 �` 255)
//		 ULARGE_INTEGER ul64SrcOrgY ... �]���� X ���_ * 4294967296
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendNormalStretchFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
													  LONG lDestDistance, LONG lSrcPitch, const NxColor* pColorTable,
													  UINT uWidth, UINT uHeight, UINT uOpacity,
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
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *(pColorTable + *(lpSrcSurface + ul64SrcOrgX.HighPart)), uOpacity);
			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		uSrcOrgY = ul64SrcOrgY.LowPart;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
	} while (--uHeight != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
// void CNxCustomDraw32::Blt_BlendNormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																LONG lDestDistance, LONG lSrcPitch,
//																UINT uWidth, UINT uHeight, UINT uOpacity,
//																const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �ʏ�u�����h + �o�C���j�A�t�B���^�g��k�� Blt (386)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 UINT uOpacity				... �]�����s�����x(0 �` 255)
//		 ULARGE_INTEGER ul64SrcOrgY ... �]���� X ���_ * 4294967296
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendNormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
															 LONG lDestDistance, LONG lSrcPitch,
															 UINT uWidth, UINT uHeight, UINT uOpacity,
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
			DWORD dwFilterResult = BiLinearFilter32(ul64SrcOrgX.LowPart, uSrcOrgY, lpSrcSurface + ul64SrcOrgX.HighPart * 4, lSrcPitch);
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  dwFilterResult, uOpacity);
			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		uSrcOrgY = ul64SrcOrgY.LowPart;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
	} while (--uHeight != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
// void CNxCustomDraw32::Blt_BlendNormalStretch_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													LONG lDestDistance, LONG lSrcPitch,
//													UINT uWidth, UINT uHeight, UINT uOpacity,
//													const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �ʏ�u�����h + �g��k�� Blt (MMX)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 UINT uOpacity				... �]�����s�����x(0 �` 255)
//		 ULARGE_INTEGER ul64SrcOrgY ... �]���� X ���_ * 4294967296
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendNormalStretch_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																   LONG /*lDestDistance*/, LONG /*lSrcPitch*/,
																   UINT /*uWidth*/, UINT /*uHeight*/, UINT /*uOpacity*/,
																   const CNxSurface::StretchBltInfo* /*pStretchBltInfo*/)
{
	using namespace ConstTable;

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
		UINT		   uOpacity;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;

	__asm
	{
		sub			esp, 4
		push		ebp
		push		ebx
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			eax, [esp]StackFrame.uOpacity
		mov			edx, [esp]StackFrame.lpSrcSurface
		mov			ecx, [ebx]StretchBltInfo.uSrcOrgY
		mov			edi, [esp]StackFrame.lpDestSurface
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			[esp]StackFrame.uSrcOrgY, ecx

loop_y:

		mov			eax, 0
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		mov			ecx, [esp]StackFrame.uWidth

loop_x:

		movd		mm2, [edx + eax * 4]
		punpcklbw	mm2, mm0
		movd		mm4, [edi]
		punpcklbw	mm4, mm0
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		adc			eax, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4
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
		emms
		ret
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
// void CNxCustomDraw32::Blt_BlendNormalStretchFrom8_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														 LONG lDestDistance, LONG lSrcPitch,
//														 UINT uWidth, UINT uHeight, UINT uOpacity,
//														 const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: 8bpp ����̒ʏ�u�����h + �g��k�� Blt (MMX)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 UINT uOpacity				... �]�����s�����x(0 �` 255)
//		 ULARGE_INTEGER ul64SrcOrgY ... �]���� X ���_ * 4294967296
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendNormalStretchFrom8_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																		LONG /*lDestDistance*/, LONG /*lSrcPitch*/, const NxColor* /*pColorTable*/,
																		UINT /*uWidth*/, UINT /*uHeight*/, UINT /*uOpacity*/,
																		const CNxSurface::StretchBltInfo* /*pStretchBltInfo*/)
{
	using namespace ConstTable;

#pragma pack(push, 4)

	struct StackFrame
	{
		LPVOID		   EDI;
		LPVOID		   ESI;
		LPVOID		   EBX;
		LPVOID		   EBP;
		UINT		   uSrcOrgY;
		ULARGE_INTEGER ul64SrcDeltaX;
		LPVOID		   EIP;
		LPBYTE		   lpDestSurface;
		const BYTE*	   lpSrcSurface;
		LONG		   lDestDistance;
		LONG		   lSrcPitch;
		const NxColor* pColorTable;
		UINT		   uWidth;
		UINT		   uHeight;
		UINT		   uOpacity;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;

	__asm
	{
		sub			esp, 12
		push		ebp
		push		ebx
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			eax, [esp]StackFrame.uOpacity
		mov			edx, [esp]StackFrame.lpSrcSurface
		mov			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		mov			edi, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		mov			ecx, [ebx]StretchBltInfo.uSrcOrgY
		mov			[esp]StackFrame.ul64SrcDeltaX.LowPart, esi
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			[esp]StackFrame.ul64SrcDeltaX.HighPart, edi
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			[esp]StackFrame.uSrcOrgY, ecx

loop_y:

		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		mov			ebx, [esp]StackFrame.pColorTable
		mov			ebp, 0
		mov			ecx, [esp]StackFrame.uWidth

loop_x:

		movzx		eax, byte ptr [edx + ebp]
		movd		mm4, [edi]
		add			esi, [esp]StackFrame.ul64SrcDeltaX.LowPart
		movd		mm2, [ebx + eax * 4]
		punpcklbw	mm4, mm0
		punpcklbw	mm2, mm0
		adc			ebp, [esp]StackFrame.ul64SrcDeltaX.HighPart
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4
		loop		loop_x

		mov			ebx, [esp]StackFrame.pStretchBltInfo
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
		add			esp, 12
		emms
		ret
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
// void CNxCustomDraw32::Blt_BlendNormalStretchLinearFilter_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																LONG lDestDistance, LONG lSrcPitch,
//																UINT uWidth, UINT uHeight, UINT uOpacity,
//																const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �ʏ�u�����h + �o�C���j�A�t�B���^ �g��k�� Blt (MMX)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance			... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 UINT uOpacity				... �]�����s�����x(0 �` 255)
//		 ULARGE_INTEGER ul64SrcOrgY ... �]���� X ���_ * 4294967296
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
// ���l: ���Z���x�� 8bit
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendNormalStretchLinearFilter_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																			   LONG /*lDestDistance*/, LONG /*lSrcPitch*/,
																			   UINT /*uWidth*/, UINT /*uHeight*/, UINT /*uOpacity*/,
																			   const CNxSurface::StretchBltInfo* /*pStretchBltInfo*/)
{
	using namespace ConstTable;

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
		UINT		   uOpacity;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;

	__asm
	{
		sub			esp, 4
		push		ebp
		push		ebx
		push		esi
		push		edi
		pxor		mm0, mm0
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			eax, [esp]StackFrame.uOpacity
		mov			edx, [esp]StackFrame.lpSrcSurface
		mov			ecx, [ebx]StretchBltInfo.uSrcOrgY
		mov			edi, [esp]StackFrame.lpDestSurface
		movq		mm5, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			[esp]StackFrame.uSrcOrgY, ecx

loop_y:

		movd		mm7, [esp]StackFrame.uSrcOrgY
		mov			ebp, 0
		psrld		mm7, 24
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		punpcklwd	mm7, mm7
		mov			ecx, [esp]StackFrame.uWidth
		punpckldq	mm7, mm7
loop_x:
		lea			eax, [edx + ebp * 4] 
		movd		mm6, esi
		movq		mm1, [eax]
		add			eax, [esp]StackFrame.lSrcPitch
		psrld		mm6, 24
		movq		mm2, mm1
		punpcklwd	mm6, mm6
		movq		mm3, [eax]
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0
		movq		mm4, mm3
		punpckldq	mm6, mm6
		punpcklbw	mm3, mm0
		punpckhbw	mm4, mm0
		psubw		mm2, mm1
		psubw		mm4, mm3
		pmullw		mm2, mm6
		pmullw		mm4, mm6
		movd		mm6, [edi]
		add			edi, 4
		psrlw		mm2, 8
		psrlw		mm4, 8
		paddb		mm2, mm1
		paddb		mm4, mm3
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		psubw		mm4, mm2
		punpcklbw	mm6, mm0
		pmullw		mm4, mm7
		psrlw		mm4, 8
		paddb		mm4, mm2
		psubw		mm4, mm6
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		pmullw		mm4, mm5
		psrlw		mm4, 8
		dec			ecx
		paddb		mm4, mm6
		packuswb	mm4, mm0
		movd		[edi - 4], mm4
		jnz			loop_x

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
		emms
		ret
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//												  LONG lDestDistance, LONG lSrcDistance,
//												  UINT uWidth, UINT uHeight,
//												  UINT uOpacity)
// �T�v: ���Z�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
									   LONG lDestDistance, LONG lSrcDistance,
									   UINT uWidth, UINT uHeight,
									   UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Sub, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendSub_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//												  LONG lDestDistance, LONG lSrcDistance,
//												  UINT uWidth, UINT uHeight,
//												  UINT uOpacity)
// �T�v: ���Z�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendSub_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
														 LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
														 UINT /*uWidth*/, UINT /*uHeight*/,
														 UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ebx;
		LPVOID		ebp;
		LPVOID		edi;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		nSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		edi
		mov			eax, [esp]StackFrame.uOpacity
		pxor		mm0, mm0
		mov			ebx, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		mov			edx, [esp]StackFrame.uHeight

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm2, [ebx]
		movq		mm3, mm2
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0

		movq		mm4, [edi]
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		packuswb	mm2, mm3
		psubusb		mm4, mm2
		add			ebx, 8
		movq		[edi], mm4
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebx]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		pmullw		mm2, mm1
		psrlw		mm2, 8
		packuswb	mm2, mm0
		psubusb		mm4, mm2
		add			ebx, 4
		movd		[edi], mm4
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebx, [esp]StackFrame.nSrcDistance
		dec			edx
		jnz			loop_y

		pop			edi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//												  LONG lDestDistance, LONG lSrcDistance,
//												  UINT uWidth, UINT uHeight,
//												  UINT uOpacity)
// �T�v: ���Z�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
									   LONG lDestDistance, LONG lSrcDistance,
									   UINT uWidth, UINT uHeight,
									   UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Add, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendAdd_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//												  LONG lDestDistance, LONG lSrcDistance,
//												  UINT uWidth, UINT uHeight,
//												  UINT uOpacity)
// �T�v: ���Z�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendAdd_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
														 LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
														 UINT /*uWidth*/, UINT /*uHeight*/,
														 UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ebx;
		LPVOID		ebp;
		LPVOID		edi;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		edi
		mov			eax, [esp]StackFrame.uOpacity
		pxor		mm0, mm0
		mov			ebx, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		mov			edx, [esp]StackFrame.uHeight

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm2, [ebx]
		movq		mm3, mm2
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		packuswb	mm2, mm3
		paddusb		mm2, [edi]
		add			ebx, 8
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebx]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		pmullw		mm2, mm1
		psrlw		mm2, 8
		packuswb	mm2, mm0
		paddusb		mm2, mm4
		add			ebx, 4
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebx, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			edi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													LONG lDestDistance, LONG lSrcDistance,
//													UINT uWidth, UINT uHeight,
//													UINT uOpacity)
// �T�v: ��Z�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
										 LONG lDestDistance, LONG lSrcDistance,
										 UINT uWidth, UINT uHeight,
										 UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Multi, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendMulti_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													LONG lDestDistance, LONG lSrcDistance,
//													UINT uWidth, UINT uHeight,
//													UINT uOpacity)
// �T�v: ��Z�u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendMulti_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
														   LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
														   UINT /*uWidth*/, UINT /*uHeight*/,
														   UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ebx;
		LPVOID		edi;
		LPVOID		ebp;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		edi
		push		ebp
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebp, [esp]StackFrame.lpSrcSurface
		pxor		mm0, mm0
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
		mov			eax, [esp]StackFrame.uWidth
		mov			edx, [esp]StackFrame.uHeight

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm2, [ebp]
		movq		mm3, mm2
		add			ebp, 8
		movq		mm4, [edi]
		movq		mm5, mm4
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		psrlw		mm2, 8
		psrlw		mm3, 8
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pand		mm2, mm7
		pand		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword
		
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		pmullw		mm2, mm4
		psrlw		mm2, 8
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pand		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		add			ebp, 4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebp, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													 LONG lDestDistance, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight,
//													 UINT uOpacity)
// �T�v: �X�N���[���u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
										  LONG lDestDistance, LONG lSrcDistance,
										  UINT uWidth, UINT uHeight,
										  UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Screen, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendScreen_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													 LONG lDestDistance, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight,
//													 UINT uOpacity)
// �T�v: �X�N���[���u�����h Blt (MMX ��)
// ����: LPBYTE lpDestSurface     ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance       ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       LONG lSrcDistance        ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       UINT uWidth              ... ��
//       UINT uHeight             ... ����
//       UINT uOpacity            ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendScreen_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
															LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
															UINT /*uWidth*/, UINT /*uHeight*/,
															UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ebx;
		LPVOID		edi;
		LPVOID		ebp;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		edi
		push		ebp
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebp, [esp]StackFrame.lpSrcSurface
		pxor		mm0, mm0
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		mov			edx, [esp]StackFrame.uHeight
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]

	loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

	loop_x_qword:

		movq		mm2, [ebp]
		add			ebp, 8
		movq		mm3, mm2
		movq		mm4, [edi]
		movq		mm5, mm4
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		pxor		mm2, mm7
		pxor		mm3, mm7
		pxor		mm4, mm7
		pxor		mm5, mm7
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		pxor		mm4, mm7
		pxor		mm5, mm7
		psrlw		mm2, 8
		psrlw		mm3, 8
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pandn		mm2, mm7
		pandn		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword
		
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebp]
		add			ebp, 4
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		pxor		mm2, mm7
		pxor		mm4, mm7
		pmullw		mm2, mm4
		pxor		mm4, mm7
		psrlw		mm2, 8
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pandn		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebp, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													   LONG lDestDistance, LONG lSrcDistance,
//													   UINT uWidth, UINT uHeight,
//													   UINT uOpacity)
// �T�v: ��r(��)�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											LONG lDestDistance, LONG lSrcDistance,
											UINT uWidth, UINT uHeight,
											UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Brighten, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendBirghten_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													   LONG lDestDistance, LONG lSrcDistance,
//													   UINT uWidth, UINT uHeight,
//													   UINT uOpacity)
// �T�v: ��r(��)�u�����h Blt (MMX ��)
//		 �s�N�Z���̔�r���s�Ȃ������Ō�͒ʏ�u�����h�ƂقƂ�Ǔ���
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendBrighten_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
															  LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
															  UINT /*uWidth*/, UINT /*uHeight*/,
															  UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ebx;
		LPVOID		ebp;
		LPVOID		edi;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		edi
		pxor		mm0, mm0
		mov			eax, [esp]StackFrame.uOpacity
		mov			ebx, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align 8

loop_x_qword:

		movq		mm2, [ebx]
		movq		mm3, mm2
		punpcklbw	mm2, mm0
		movq		mm4, [edi]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpckhbw	mm3, mm0
		punpckhbw	mm5, mm0
		movq		mm6, mm2
		pcmpgtw		mm2, mm4
		movq		mm7, mm3
		pcmpgtw		mm3, mm5
		pand		mm6, mm2
		pandn		mm2, mm4
		pand		mm7, mm3
		pandn		mm3, mm5
		por			mm2, mm6
		por			mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebx, 8
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebx]
		punpcklbw	mm2, mm0
		movd		mm4, [edi]
		punpcklbw	mm4, mm0
		movq		mm6, mm2
		pcmpgtw		mm2, mm4
		pand		mm6, mm2
		pandn		mm2, mm4
		por			mm2, mm6
		psubw		mm2, mm4
		pmullw		mm2, mm1
		add			ebx, 4
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebx, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			edi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													 LONG lDestDistance, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight,
//													 UINT uOpacity)
// �T�v: ��r(��)�u�����h Blt (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlendDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
										  LONG lDestDistance, LONG lSrcDistance,
										  UINT uWidth, UINT uHeight,
										  UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Darken, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  *reinterpret_cast<const DWORD*>(lpSrcSurface), uOpacity);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_BlendDarken_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													 LONG lDestDistance, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight,
//													 UINT uOpacity)
// �T�v: ��r(��)�u�����h Blt (MMX ��)
//		 �s�N�Z���̔�r���s�Ȃ������Ō�͒ʏ�u�����h�ƂقƂ�Ǔ���
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_BlendDarken_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
															LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
															UINT /*uWidth*/, UINT /*uHeight*/,
															UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ebx;
		LPVOID		ebp;
		LPVOID		edi;
		LPVOID		eip;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		edi
		pxor		mm0, mm0
		mov			eax, [esp]StackFrame.uOpacity
		mov			ebx, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm2, [ebx]
		movq		mm3, mm2
		movq		mm4, [edi]
		movq		mm5, mm4
		punpckhbw	mm3, mm0
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		movq		mm6, mm4
		pcmpgtw		mm6, mm2
		movq		mm7, mm5
		pcmpgtw		mm7, mm3
		pand		mm2, mm6
		pandn		mm6, mm4
		pand		mm3, mm7
		pandn		mm7, mm5
		por			mm2, mm6
		por			mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebx, 8
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, [ebx]
		punpcklbw	mm2, mm0
		movd		mm4, [edi]
		punpcklbw	mm4, mm0
		movq		mm6, mm4
		pcmpgtw		mm6, mm2
		pand		mm2, mm6
		pandn		mm6, mm4
		por			mm2, mm6
		psubw		mm2, mm4
		pmullw		mm2, mm1
		add			ebx, 4
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebx, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			edi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
//									const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//									const NxBlt* pNxBlt) const
// �T�v: �u�����h Blt
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �]����A���t�@�͕ۑ������
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
								const CNxSurface *pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const
{
	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Blend() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	int nBlendType = pNxBlt->dwFlags & NxBlt::blendTypeMask;

	if (!(pNxBlt->dwFlags & NxBlt::constDestAlpha))
	{	// constDestAlpha ���w�肳��ĂȂ��Ȃ��...
		if (uOpacity == 255 && nBlendType == NxBlt::blendNormal)
		{	// �s�����x�� 255 ���A�ʏ�u�����h�Ȃ�ΒP���]������
			return CNxCustomDraw32::Blt_Normal(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
		}
	}

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	LONG lSrcPitch = pSrcSurface->GetPitch();
	
	if (!IsStretch(lpDestRect, lpSrcRect))
	{	// �g��k������

		// �N���b�v
		RECT rcSrcClip;
		pSrcSurface->GetRect(&rcSrcClip);
		if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;	// �`���`�Ȃ�

		// ���ƍ���
		UINT uWidth = rcDest.right - rcDest.left;
		UINT uHeight = rcDest.bottom - rcDest.top;

		// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
		LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
		LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

		if (pSrcSurface->GetBitCount() == 32)
		{	// 32bpp �p

			// �]�����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
			LONG lSrcDistance = lSrcPitch - uWidth * 4;
			const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left * 4;


			typedef void (__cdecl *BltBlendFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurfae,
												 LONG lDestDistance, LONG lSrcDistance, UINT uWidth, UINT uHeight,
												 UINT uOpacity);

			static const BltBlendFunc bltBlendFunc[][2] =
			{
	#if defined(NXDRAW_MMX_ONLY)
				// 0 ... �ʏ� [ d = (d * (256 - uAlpha) / 256) + ((s * uAlpha) / 256) ]
				{ Blt_BlendNormal_MMX, Blt_BlendNormal_MMX },
				// 1 ... ���Z [ d = d + ((s * uAlpha) / 256) ]
				{ Blt_BlendAdd_MMX, Blt_BlendAdd_MMX },
				// 2 ... ���Z [ d = d - ((s * uAlpha) / 256) ]
				{ Blt_BlendSub_MMX, Blt_BlendSub_MMX },
				// 3 ... ��Z [ d = d - (((d - (s*d) / 255) * uAlpha) / 256) ]
				{ Blt_BlendMulti_MMX, Blt_BlendMulti_MMX },
				// 4 ... �X�N���[�� [ d = d + (((((255 -s) * (255 -d) / 255) - d) * uAlpha) / 256);
				{ Blt_BlendScreen_MMX, Blt_BlendScreen_MMX },
				// 5 ... ��r(��)
				{ Blt_BlendBrighten_MMX, Blt_BlendBrighten_MMX },
				// 5 ... ��r(��)
				{ Blt_BlendDarken_MMX, Blt_BlendDarken_MMX },
	#else
				// 0 ... �ʏ� [ d = (d * (256 - uAlpha) / 256) + ((s * uAlpha) / 256) ]
				{ Blt_BlendNormal_386, Blt_BlendNormal_MMX },
				// 1 ... ���Z [ d = d + ((s * uAlpha) / 256) ]
				{ Blt_BlendAdd_386, Blt_BlendAdd_MMX },
				// 2 ... ���Z [ d = d - ((s * uAlpha) / 256) ]
				{ Blt_BlendSub_386, Blt_BlendSub_MMX },
				// 3 ... ��Z [ d = d - (((d - (s*d) / 255) * uAlpha) / 256) ]
				{ Blt_BlendMulti_386, Blt_BlendMulti_MMX },
				// 4 ... �X�N���[�� [ d = d + (((((255 -s) * (255 -d) / 255) - d) * uAlpha) / 256);
				{ Blt_BlendScreen_386, Blt_BlendScreen_MMX },
				// 5 ... ��r(��)
				{ Blt_BlendBrighten_386, Blt_BlendBrighten_MMX },
				// 5 ... ��r(��)
				{ Blt_BlendDarken_386, Blt_BlendDarken_MMX },
	#endif	// #if defined(NXDRAW_MMX_ONY)
			};

			(bltBlendFunc[nBlendType - NxBlt::blendNormal][(CNxDraw::GetInstance()->IsMMXEnabled()) ? 1 : 0])
				(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, uOpacity);
		}	// if (pSrcSurface->GetBitCount() == 32)
		else
		{
			// 8bpp �p
			if (nBlendType != NxBlt::blendNormal)
			{
				_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Blend() : 8bpp ����̃u�����h�]���́A�ʏ�u�����h�������\�ł�.\n");
				return FALSE;
			}

			// �]�����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
			LONG lSrcDistance = lSrcPitch - uWidth;
			const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left;

			typedef void (__cdecl *BltBlendFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurfae,
												 LONG lDestDistance, LONG lSrcDistance, const NxColor* pColorTable,
												 UINT uWidth, UINT uHeight, UINT uOpacity);

			static const BltBlendFunc bltBlendFunc[][2] =
			{
	#if defined(NXDRAW_MMX_ONLY)
				// 0 ... �ʏ� [ d = (d * (256 - uAlpha) / 256) + ((s * uAlpha) / 256) ]
				{ Blt_BlendNormalFrom8_MMX, Blt_BlendNormalFrom8_MMX },
	#else
				// 0 ... �ʏ� [ d = (d * (256 - uAlpha) / 256) + ((s * uAlpha) / 256) ]
				{ Blt_BlendNormalFrom8_386, Blt_BlendNormalFrom8_MMX },
	#endif	// #if defined(NXDRAW_MMX_ONY)
			};

			(bltBlendFunc[nBlendType - NxBlt::blendNormal][(CNxDraw::GetInstance()->IsMMXEnabled()) ? 1 : 0])
				(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, pSrcSurface->GetColorTable(), uWidth, uHeight, uOpacity);
		}
	}
	else
	{	// �g��k��
		if (nBlendType != NxBlt::blendNormal)
		{
			_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Blend() : �g��k���́A�ʏ�u�����h�ȊO�ɂ͑Ή����Ă��܂���.\n");
			return FALSE;
		}

		if (pSrcSurface->GetBitCount() == 32)
		{	// 32bpp
			void (*pfnStretchBltFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance,
									  LONG lSrcPitch, UINT uWidth, UINT uHeight, UINT uOpacity, const CNxSurface::StretchBltInfo* pStretchBltInfo);

			// �N���b�v
			CNxSurface::StretchBltInfo stretchBltInfo;
			BOOL bFilter = (pNxBlt != NULL) && (pNxBlt->dwFlags & NxBlt::linearFilter);
			if (!PreprocessStretchBlt(pDestSurface, &stretchBltInfo, &rcDest, pSrcSurface, &rcSrc, &bFilter))
				return TRUE;

			if (bFilter)
			{
				pfnStretchBltFunc = (CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_BlendNormalStretchLinearFilter_MMX
															   : Blt_BlendNormalStretchLinearFilter_386;
			}
			else
			{	// �o�C���j�A�t�B���^�͎g�p���Ȃ�
				pfnStretchBltFunc = (CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_BlendNormalStretch_MMX
															   : Blt_BlendNormalStretch_386;
			}

			// ���ƍ���
			UINT uWidth = rcDest.right - rcDest.left;
			UINT uHeight = rcDest.bottom - rcDest.top;

			// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
			LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
			LPBYTE lpDestSurface = (LPBYTE)pDestSurface->GetBits() + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;

			// �]����
			const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left * 4;

			(pfnStretchBltFunc)(lpDestSurface, lpSrcSurface, lDestDistance, lSrcPitch, uWidth, uHeight, uOpacity, &stretchBltInfo);
		}
		else
		{
			// �N���b�v
			CNxSurface::StretchBltInfo stretchBltInfo;
			BOOL bFilter = FALSE;
			if (!PreprocessStretchBlt(pDestSurface, &stretchBltInfo, &rcDest, pSrcSurface, &rcSrc, &bFilter))
				return TRUE;

			// ���ƍ���
			UINT uWidth = rcDest.right - rcDest.left;
			UINT uHeight = rcDest.bottom - rcDest.top;

			// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
			LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
			LPBYTE lpDestSurface = (LPBYTE)pDestSurface->GetBits() + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;

			// �]����
			const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left;

			((CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_BlendNormalStretchFrom8_MMX
											   : Blt_BlendNormalStretchFrom8_386)
					(lpDestSurface, lpSrcSurface, lDestDistance, lSrcPitch, pSrcSurface->GetColorTable(), uWidth, uHeight, uOpacity, &stretchBltInfo);

		}
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendNormal_386(LPBYTE lpDestSurface, LONG lDestDistance,
//															   UINT uWidth, UINT uHeight,
//															   UINT uOpacity, DWORD dwColor)
// �T�v: �ʏ�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendNormal_386(LPBYTE lpDestSurface, LONG lDestDistance,
													UINT uWidth, UINT uHeight,
													UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill(NxAlphaBlend::Normal, *reinterpret_cast<const DWORD*>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendNormal_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//															   UINT uWidth, UINT uHeight,
//															   UINT uOpacity, DWORD dwColor)
// �T�v: �ʏ�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface     ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance       ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       UINT uWidth              ... ��
//       UINT uHeight             ... ����
//       UINT uOpacity            ... �s�����x(0 �` 255)
//       DWORD dwColor            ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendNormal_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																	  UINT /*uWidth*/, UINT /*uHeight*/,
																	  UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		pxor		mm0, mm0
		push		edi
		push		ebp
		movd		mm7, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		punpcklbw	mm7, mm0
loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm4, [edi]
		movq		mm2, mm7
		movq		mm3, mm7
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [edi]
		movq		mm2, mm7
		punpcklbw	mm4, mm0
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendAdd_386(LPBYTE lpDestSurface, LONG lDestDistance,
//															UINT uWidth, UINT uHeight,
//															UINT uOpacity, DWORD dwColor)
// �T�v: ���Z�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendAdd_386(LPBYTE lpDestSurface, LONG lDestDistance,
												 UINT uWidth, UINT uHeight,
												 UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill(NxAlphaBlend::Add, *reinterpret_cast<const DWORD*>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendAdd_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//															UINT uWidth, UINT uHeight,
//															UINT uOpacity, DWORD dwColor)
// �T�v: ���Z�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendAdd_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																   UINT /*uWidth*/, UINT /*uHeight*/,
																   UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		pxor		mm3, mm3
		push		edi
		push		ebp
		movd		mm0, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		punpcklbw	mm0, mm3
		mov			edi, [esp]StackFrame.lpDestSurface
		pmullw		mm0, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			ebx, [esp]StackFrame.uHeight
		psrlw		mm0, 8
		mov			eax, [esp]StackFrame.uWidth
		packuswb	mm0, mm0

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm1, [edi]
		paddusb		mm1, mm0
		movq		[edi], mm1
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm1, [edi]
		paddusb		mm1, mm0
		movd		[edi], mm1
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSub_386(LPBYTE lpDestSurface, LONG lDestDistance,
//															UINT uWidth, UINT uHeight,
//															UINT uOpacity, DWORD dwColor)
// �T�v: ���Z�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSub_386(LPBYTE lpDestSurface, LONG lDestDistance,
												 UINT uWidth, UINT uHeight,
												 UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill(NxAlphaBlend::Sub, *reinterpret_cast<const DWORD*>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSub_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//															UINT uWidth, UINT uHeight,
//															UINT uOpacity, DWORD dwColor)
// �T�v: ���Z�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface     ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//       LONG lDestDistance       ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//       UINT uWidth              ... ��
//       UINT uHeight             ... ����
//       UINT uOpacity            ... �]�����s�����x(0 �` 255)
//       DWORD dwColor            ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSub_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																   UINT /*uWidth*/, UINT /*uHeight*/,
																   UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		pxor		mm3, mm3
		push		edi
		push		ebp
		movd		mm0, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		punpcklbw	mm0, mm3
		mov			edi, [esp]StackFrame.lpDestSurface
		pmullw		mm0, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			ebx, [esp]StackFrame.uHeight
		psrlw		mm0, 8
		mov			eax, [esp]StackFrame.uWidth
		packuswb	mm0, mm0
		
loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm1, [edi]
		psubusb		mm1, mm0
		movq		[edi], mm1
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm1, [edi]
		psubusb		mm1, mm0
		movd		[edi], mm1
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebp
		pop			esi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendMulti_386(LPBYTE lpDestSurface, LONG lDestDistance,
//															  UINT uWidth, UINT uHeight,
//															  UINT uOpacity, DWORD dwColor)
// �T�v: ��Z�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendMulti_386(LPBYTE lpDestSurface, LONG lDestDistance,
												   UINT uWidth, UINT uHeight,
												   UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill(NxAlphaBlend::Multi, *reinterpret_cast<const DWORD*>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendMulti_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//															  UINT uWidth, UINT uHeight,
//															  UINT uOpacity, DWORD dwColor)
// �T�v: ��Z�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendMulti_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																	 UINT /*uWidth*/, UINT /*uHeight*/,
																	 UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		edi
		push		ebp
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		mov			edi, [esp]StackFrame.lpDestSurface
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
		mov			ebx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		
	loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm4, [edi]
		movd		mm2, ebp
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpcklbw	mm2, mm0
		punpckhbw	mm5, mm0
		movq		mm3, mm2
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		psrlw		mm2, 8
		psrlw		mm3, 8
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pand		mm2, mm7
		pand		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword
		
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [edi]
		movd		mm2, ebp
		punpcklbw	mm4, mm0
		punpcklbw	mm2, mm0
		pmullw		mm2, mm4
		psrlw		mm2, 8
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pand		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm3
		movd		[edi], mm2
		add			edi, 4

	loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendScreen_386(LPBYTE lpDestSurface, LONG lDestDistance,
//															   UINT uWidth, UINT uHeight,
//															   UINT uOpacity, DWORD dwColor)
// �T�v: �X�N���[���u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendScreen_386(LPBYTE lpDestSurface, LONG lDestDistance,
													UINT uWidth, UINT uHeight,
													UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill(NxAlphaBlend::Screen, *reinterpret_cast<const DWORD*>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendScreen_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//															   UINT uWidth, UINT uHeight,
//															   UINT uOpacity, DWORD dwColor)
// �T�v: �X�N���[���u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �]�����s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendScreen_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																	  UINT /*uWidth*/, UINT /*uHeight*/,
																	  UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		edi
		push		ebp
		pxor		mm0, mm0
		mov			ebp, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		mov			edi, [esp]StackFrame.lpDestSurface
		not			ebp
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
		mov			ebx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		
	loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm4, [edi]
		movd		mm2, ebp
		movq		mm5, mm4
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		movq		mm3, mm2
		punpckhbw	mm5, mm0
		pxor		mm4, mm7
		pxor		mm5, mm7
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		pxor		mm4, mm7
		pxor		mm5, mm7
		psrlw		mm2, 8
		psrlw		mm3, 8
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pandn		mm2, mm7
		pandn		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm2, ebp
		movd		mm4, [edi]
		punpcklbw	mm2, mm0
		punpcklbw	mm4, mm0
		pxor		mm4, mm7
		pmullw		mm2, mm4
		pxor		mm4, mm7
		psrlw		mm2, 8
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pandn		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm3
		movd		[edi], mm2
		add			edi, 4

	loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendBrighten_386(LPBYTE lpDestSurface, LONG lDestDistance,
//																 UINT uWidth, UINT uHeight,
//																 UINT uOpacity, DWORD dwColor)
// �T�v: Brighten �u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendBrighten_386(LPBYTE lpDestSurface, LONG lDestDistance,
													  UINT uWidth, UINT uHeight,
													  UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill(NxAlphaBlend::Brighten, *reinterpret_cast<const DWORD*>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendBrighten_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//																 UINT uWidth, UINT uHeight,
//																 UINT uOpacity, DWORD dwColor)
// �T�v: Brighten �u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendBrighten_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																		UINT /*uWidth*/, UINT /*uHeight*/,
																		UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		pxor		mm0, mm0
		push		edi
		push		ebp
		movd		mm7, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		punpcklbw	mm7, mm0

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm4, [edi]
		movq		mm2, mm7
		movq		mm3, mm7
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		movq		mm6, mm7
		pcmpgtw		mm2, mm4
		punpckhbw	mm5, mm0
		movq		mm0, mm7
		pcmpgtw		mm3, mm5
		pand		mm6, mm2
		pand		mm0, mm3
		pandn		mm2, mm4
		pandn		mm3, mm5
		por			mm2, mm6
		por			mm3, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		pxor		mm0, mm0
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [edi]
		movq		mm2, mm7
		punpcklbw	mm4, mm0
		movq		mm6, mm7
		pcmpgtw		mm2, mm4
		pand		mm6, mm2
		pandn		mm2, mm4
		por			mm2, mm6
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendDarken_386(LPBYTE lpDestSurface, LONG lDestDistance,
//															   UINT uWidth, UINT uHeight,
//															   UINT uOpacity, DWORD dwColor)
// �T�v: Darken �u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendDarken_386(LPBYTE lpDestSurface, LONG lDestDistance,
													UINT uWidth, UINT uHeight,
													UINT uOpacity, DWORD dwColor)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill(NxAlphaBlend::Darken, *reinterpret_cast<const DWORD*>(lpDestSurface), dwColor, uOpacity);
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendDarken_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//															   UINT uWidth, UINT uHeight,
//															   UINT uOpacity, DWORD dwColor)
// �T�v: Darken �u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 UINT uOpacity			  ... �s�����x(0 �` 255)
//		 DWORD dwColor			  ... �h��ׂ��F
// ���l: �]����A���t�@�͕ۑ������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendDarken_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																	  UINT /*uWidth*/, UINT /*uHeight*/,
																	  UINT /*uOpacity*/, DWORD /*dwColor*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		pxor		mm0, mm0
		push		edi
		push		ebp
		movd		mm7, [esp]StackFrame.dwColor
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		punpcklbw	mm7, mm0

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm4, [edi]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0

		movq		mm6, mm4
		movq		mm2, mm7
		movq		mm0, mm5
		movq		mm3, mm7
		pcmpgtw		mm6, mm2
		pcmpgtw		mm0, mm3
		pand		mm2, mm6
		pand		mm3, mm0
		pandn		mm6, mm4
		pandn		mm0, mm5
		por			mm2, mm6
		por			mm3, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		pxor		mm0, mm0
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [edi]
		punpcklbw	mm4, mm0
		movq		mm6, mm4
		movq		mm2, mm7
		pcmpgtw		mm6, mm2
		pand		mm2, mm6
		pandn		mm6, mm4
		por			mm2, mm6
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//											  const NxBlt* pNxBlt) const
// �T�v: �u�����h�h��ׂ�
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_ColorFill_Blend(CNxSurface* pDestSurface, const RECT* lpDestRect,
										  const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const
{
	DWORD dwColor  = pNxBlt->nxbColor;
	UINT uOpacity = (dwColor >> 24) & 0xff;

	if (uOpacity == 0)
		return TRUE;

	if (!(pNxBlt->dwFlags & NxBlt::constDestAlpha))
	{	// constDestAlpha ���w�肳��ĂȂ��Ȃ��...
		if (uOpacity == 255 && (pNxBlt->dwFlags & NxBlt::blendTypeMask) == NxBlt::blendNormal)
		{	// �s�����x�� 255 ���A�ʏ�u�����h�Ȃ�ΒP���ɓh��ׂ�
			return CNxCustomDraw32::Blt_ColorFill_Normal(pDestSurface, lpDestRect, NULL, NULL, pNxBlt);
		}
	}
	
	RECT rcDest = *lpDestRect;
	if (!pDestSurface->ClipBltRect(rcDest))
		return TRUE;

	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �������ݐ�T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	typedef void (__cdecl *FillAlphaFunc)(LPBYTE lpDestSurface, LONG lDestDistance, UINT uWidth, UINT uHeight, UINT uOpacity, DWORD dwColor);
	static const FillAlphaFunc fillAlphaFunc[][2] =
	{
#if defined(NXDRAW_MMX_ONLY)
		{ Blt_ColorFill_BlendNormal_MMX, Blt_ColorFill_BlendNormal_MMX },		// 0 ... �ʏ�
		{ Blt_ColorFill_BlendAdd_MMX, Blt_ColorFill_BlendAdd_MMX },				// 1 ... ���Z
		{ Blt_ColorFill_BlendSub_MMX, Blt_ColorFill_BlendSub_MMX },				// 2 ... ���Z
		{ Blt_ColorFill_BlendMulti_MMX, Blt_ColorFill_BlendMulti_MMX },			// 3 ... ��Z
		{ Blt_ColorFill_BlendScreen_MMX, Blt_ColorFill_BlendScreen_MMX },		// 4 ... �X�N���[��
		{ Blt_ColorFill_BlendBrighten_MMX, Blt_ColorFill_BlendBrighten_MMX },	// 5 ... Brighten
		{ Blt_ColorFill_BlendDarken_MMX, Blt_ColorFill_BlendDarken_MMX },		// 6 ... Darken
#else
		{ Blt_ColorFill_BlendNormal_386, Blt_ColorFill_BlendNormal_MMX },		// 0 ... �ʏ�
		{ Blt_ColorFill_BlendAdd_386, Blt_ColorFill_BlendAdd_MMX },				// 1 ... ���Z
		{ Blt_ColorFill_BlendSub_386, Blt_ColorFill_BlendSub_MMX },				// 2 ... ���Z
		{ Blt_ColorFill_BlendMulti_386, Blt_ColorFill_BlendMulti_MMX },			// 3 ... ��Z
		{ Blt_ColorFill_BlendScreen_386, Blt_ColorFill_BlendScreen_MMX },		// 4 ... �X�N���[��
		{ Blt_ColorFill_BlendBrighten_386, Blt_ColorFill_BlendBrighten_MMX },	// 5 ... Brighten
		{ Blt_ColorFill_BlendDarken_386, Blt_ColorFill_BlendDarken_MMX },		// 6 ... Darken
#endif	// #if defined(NXDRAW_MMX_ONLY)
	};

	// �u�����h��� / MMX�̗L���ɂ���ĈقȂ�֐����Ăяo��
	(fillAlphaFunc[(pNxBlt->dwFlags & NxBlt::blendTypeMask) - NxBlt::blendNormal][(CNxDraw::GetInstance()->IsMMXEnabled()) ? 1 : 0])
		(lpDestSurface, lDestDistance, uWidth, uHeight, uOpacity, dwColor);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	   LONG lDestDistance, LONG lAlphaDistance,
//																	   UINT uWidth, UINT uHeight,
//																	   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �A���t�@�`�����l���T�[�t�F�X�t���ʏ�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
															LONG lDestDistance, LONG lAlphaDistance,
															UINT uWidth, UINT uHeight,
															DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Normal, *reinterpret_cast<const DWORD*>(lpDestSurface),
												 *lpSrcAlpha, dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcAlpha++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcAlpha += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaNormal_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	   LONG lDestDistance, LONG lAlphaDistance,
//																	   UINT uWidth, UINT uHeight,
//																	   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t���ʏ�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
//
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaNormal_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																			  LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																			  UINT /*uWidth*/, UINT /*uHeight*/,
																			  DWORD /*dwColor*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		punpcklbw	mm1, mm0

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movzx		eax, byte ptr [ebp]
		movq		mm4, [edi]
		movq		mm2, mm1
		movzx		edx, byte ptr [ebp + 1]
		movq		mm3, mm1
		movzx		eax, byte ptr [ebx + eax]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		movzx		edx, byte ptr [ebx + edx]
		punpckhbw	mm5, mm0
		movq		mm6, [esi + eax * 8]
		psubw		mm2, mm4
		movq		mm7, [esi + edx * 8]
		psubw		mm3, mm5
		pmullw		mm2, mm6
		pmullw		mm3, mm7
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebp, 2
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp]
		movd		mm4, [edi]
		movq		mm2, mm1
		movzx		eax, byte ptr [ebx + eax]
		punpcklbw	mm4, mm0
		psubw		mm2, mm4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		inc			ebp
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lAlphaDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	LONG lDestDistance, LONG lAlphaDistance,
//																	UINT uWidth, UINT uHeight,
//																	DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�����Z�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaAdd_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
														 LONG lDestDistance, LONG lAlphaDistance,
														 UINT uWidth, UINT uHeight,
														 DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Add, *reinterpret_cast<const DWORD*>(lpDestSurface),
												 *lpSrcAlpha, dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcAlpha++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcAlpha += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaAdd_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	LONG lDestDistance, LONG lAlphaDistance,
//																	UINT uWidth, UINT uHeight,
//																	DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�����Z�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaAdd_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																		   LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																		   UINT /*uWidth*/, UINT /*uHeight*/,
																		   DWORD /*dwColor*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, [esp]StackFrame.uHeight
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		punpcklbw	mm1, mm0

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:
		
		movzx		eax, byte ptr [ebp]
		movzx		edx, byte ptr [ebp + 1]
		movq		mm2, mm1
		movq		mm3, mm1
		movzx		eax, byte ptr [ebx + eax]
		movzx		edx, byte ptr [ebx + edx]
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + edx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		packuswb	mm2, mm3
		paddusb		mm2, [edi]
		add			ebp, 2
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [edi]
		movzx		eax, byte ptr [ebp]
		movq		mm2, mm1
		movzx		eax, byte ptr [ebx + eax]
		inc			ebp
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		psrlw		mm2, 8
		packuswb	mm2, mm0
		paddusb		mm2, mm4
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lAlphaDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			esi
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	LONG lDestDistance, LONG lAlphaDistance,
//																	UINT uWidth, UINT uHeight,
//																	DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�����Z�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaSub_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
														 LONG lDestDistance, LONG lAlphaDistance,
														 UINT uWidth, UINT uHeight,
														 DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Sub, *reinterpret_cast<const DWORD*>(lpDestSurface),
												 *lpSrcAlpha, dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcAlpha++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcAlpha += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaSub_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	LONG lDestDistance, LONG lAlphaDistance,
//																	UINT uWidth, UINT uHeight,
//																	DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�����Z�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaSub_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																		   LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																		   UINT /*uWidth*/, UINT /*uHeight*/,
																		   DWORD /*dwColor*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, [esp]StackFrame.uHeight
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		punpcklbw	mm1, mm0

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword
			
		align		8

loop_x_qword:
		
		movq		mm4, [edi]
		movzx		eax, byte ptr [ebp]
		movq		mm2, mm1
		movzx		edx, byte ptr [ebp + 1]
		movq		mm3, mm1
		movzx		eax, byte ptr [ebx + eax]
		movzx		edx, byte ptr [ebx + edx]
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + edx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		packuswb	mm2, mm3
		psubusb		mm4, mm2
		add			ebp, 2
		movq		[edi], mm4
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [edi]
		movzx		eax, byte ptr [ebp]
		movq		mm2, mm1
		movzx		eax, byte ptr [ebx + eax]
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		psrlw		mm2, 8
		packuswb	mm2, mm0
		psubusb		mm4, mm2
		inc			ebp
		movd		[edi], mm4
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lAlphaDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			esi
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	  LONG lDestDistance, LONG lAlphaDistance,
//																	  UINT uWidth, UINT uHeight,
//																	  DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t����Z�u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 UINT uAlpha			   ... �s�����x(0 �` 255)
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaMulti_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
														   LONG lDestDistance, LONG lAlphaDistance,
														   UINT uWidth, UINT uHeight,
														   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Multi, *reinterpret_cast<const DWORD*>(lpDestSurface),
												 *lpSrcAlpha, dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcAlpha++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcAlpha += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaMulti_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	  LONG lDestDistance, LONG lAlphaDistance,
//																	  UINT uWidth, UINT uHeight,
//																	  DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t����Z�u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaMulti_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																		 	 LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																	 		 UINT /*uWidth*/, UINT /*uHeight*/,
																			 DWORD /*dwColor*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		esi
		push		edi
		push		ebp
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ecx, [esp]StackFrame.uHeight
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
		punpcklbw	mm1, mm0

loop_y:

		push		ecx
		mov			ecx, [esp]StackFrame.uWidth + 4
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm4, [edi]
		movq		mm2, mm1
		movq		mm3, mm1
		movq		mm5, mm4
		movzx		eax, byte ptr [ebp + 0]
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		movzx		ebx, byte ptr [ebp + 1]
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		movzx		eax, byte ptr [edx + eax]
		psrlw		mm2, 8
		psrlw		mm3, 8
		movzx		ebx, byte ptr [edx + ebx]
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pand		mm2, mm7
		pand		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		add			ebp, 2
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + ebx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth + 4
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp + 0]
		movd		mm4, [edi]
		movq		mm2, mm1
		punpcklbw	mm4, mm0
		pmullw		mm2, mm4
		psrlw		mm2, 8
		movzx		eax, byte ptr [edx + eax]
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pand		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		inc			ebp
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		pop			ecx
		add			edi, [esp]StackFrame.lDestDistance
		add			ebp, [esp]StackFrame.lAlphaDistance
		dec			ecx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	   LONG lDestDistance, LONG lAlphaDistance,
//																	   UINT uWidth, UINT uHeight,
//																	   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t���X�N���[���u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaScreen_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
														    LONG lDestDistance, LONG lAlphaDistance,
														    UINT uWidth, UINT uHeight,
														    DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Screen, *reinterpret_cast<const DWORD*>(lpDestSurface),
												 *lpSrcAlpha, dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcAlpha++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcAlpha += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaScreen_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	   LONG lDestDistance, LONG lAlphaDistance,
//																	   UINT uWidth, UINT uHeight,
//																	   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t���X�N���[���u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaScreen_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																			  LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																			  UINT /*uWidth*/, UINT /*uHeight*/,
																			  DWORD /*dwColor*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EBP;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		esi
		push		edi
		push		ebp
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ecx, [esp]StackFrame.uHeight
		mov			edx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		movq		mm7, [dwlConst_00FF_00FF_00FF_00FF]
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		punpcklbw	mm1, mm0
		movq		mm6, [dwlConst_8081_8081_8081_8081]
		pxor		mm1, mm7

loop_y:

		push		ecx
		mov			ecx, [esp]StackFrame.uWidth + 4
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

		movq		mm4, [edi]
		movq		mm2, mm1
		movq		mm3, mm1
		movq		mm5, mm4
		movzx		eax, byte ptr [ebp + 0]
		punpcklbw	mm4, mm0
		punpckhbw	mm5, mm0
		movzx		ebx, byte ptr [ebp + 1]
		pxor		mm4, mm7
		pxor		mm5, mm7
		pmullw		mm2, mm4
		pmullw		mm3, mm5
		pxor		mm4, mm7
		pxor		mm5, mm7
		movzx		eax, byte ptr [edx + eax]
		psrlw		mm2, 8
		psrlw		mm3, 8
		movzx		ebx, byte ptr [edx + ebx]
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		psrlw		mm2, 7
		psrlw		mm3, 7
		pandn		mm2, mm7
		pandn		mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		add			ebp, 2
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + ebx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		dec			ecx
		jnz			loop_x_qword

		mov			eax, [esp]StackFrame.uWidth + 4
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp + 0]
		movd		mm4, [edi]
		movq		mm2, mm1
		punpcklbw	mm4, mm0
		pxor		mm4, mm7
		pmullw		mm2, mm4
		pxor		mm4, mm7
		psrlw		mm2, 8
		movzx		eax, byte ptr [edx + eax]
		pmullw		mm2, mm6
		psrlw		mm2, 7
		pandn		mm2, mm7
		psubw		mm2, mm4
		pmullw		mm2, [dwlMMXAlphaMultiplierTable + eax * 8]
		inc			ebp
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		pop			ecx
		add			edi, [esp]StackFrame.lDestDistance
		add			ebp, [esp]StackFrame.lAlphaDistance
		dec			ecx
		jnz			loop_y

		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																		 LONG lDestDistance, LONG lAlphaDistance,
//																		 UINT uWidth, UINT uHeight,
//																		 DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�� Brighten �u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaBrighten_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
															  LONG lDestDistance, LONG lAlphaDistance,
															  UINT uWidth, UINT uHeight,
															  DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Brighten, *reinterpret_cast<const DWORD*>(lpDestSurface),
												 *lpSrcAlpha, dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcAlpha++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcAlpha += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaBrighten_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																		 LONG lDestDistance, LONG lAlphaDistance,
//																		 UINT uWidth, UINT uHeight,
//																		 DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�� Brighten �u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaBrighten_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																			    LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																			    UINT /*uWidth*/, UINT /*uHeight*/,
																			    DWORD /*dwColor*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		punpcklbw	mm1, mm0

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:
		
		movzx		eax, byte ptr [ebp]
		movq		mm4, [edi]
		movq		mm2, mm1
		movzx		edx, byte ptr [ebp + 1]
		movq		mm3, mm1
		movzx		eax, byte ptr [ebx + eax]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		movzx		edx, byte ptr [ebx + edx]
		punpckhbw	mm5, mm0
		movq		mm6, mm2
		pcmpgtw		mm2, mm4
		pand		mm6, mm2
		pandn		mm2, mm4
		por			mm2, mm6
		movq		mm7, mm3
		pcmpgtw		mm3, mm5
		pand		mm7, mm3
		pandn		mm3, mm5
		por			mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, [esi + eax * 8]
		pmullw		mm3, [esi + edx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebp, 2
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp]
		movd		mm4, [edi]
		movq		mm2, mm1
		movzx		eax, byte ptr [ebx + eax]
		punpcklbw	mm4, mm0
		movq		mm6, mm2
		pcmpgtw		mm2, mm4
		pand		mm6, mm2
		pandn		mm2, mm4
		por			mm2, mm6
		
		psubw		mm2, mm4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		inc			ebp
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lAlphaDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	   LONG lDestDistance, LONG lAlphaDistance,
//																	   UINT uWidth, UINT uHeight,
//																	   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�� Darken �u�����h�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaDarken_386(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
														    LONG lDestDistance, LONG lAlphaDistance,
														    UINT uWidth, UINT uHeight,
														    DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Darken, *reinterpret_cast<const DWORD*>(lpDestSurface),
												 *lpSrcAlpha, dwColor, bySrcAlphaToOpacityTable);
			lpDestSurface += 4;
			lpSrcAlpha++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcAlpha += lAlphaDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaDarken_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcAlpha,
//																	   LONG lDestDistance, LONG lAlphaDistance,
//																	   UINT uWidth, UINT uHeight,
//																	   DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t�� Darken �u�����h�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface	   ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BTE* lpSrcAlpha	   ... �A���t�@�`�����l���T�[�t�F�X�ւ̃|�C���^
//		 LONG lDestDistance 	   ... �]����̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lAlphaDistance	   ... �A���t�@�`���l���T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			   ... ��
//		 UINT uHeight			   ... ����
//		 DWORD dwColor			   ... �h��ׂ��F
//		 const BYTE bySrcAlphaToOpacityTable[256]
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_BlendSrcAlphaDarken_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcAlpha*/,
																			  LONG /*lDestDistance*/, LONG /*lAlphaDistance*/,
																			  UINT /*uWidth*/, UINT /*uHeight*/,
																			  DWORD /*dwColor*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EBP;
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcAlpha;
		LONG		lDestDistance;
		LONG		lAlphaDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		const BYTE* lpbSrcAlphaToOpacityTable;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		pxor		mm0, mm0
		movd		mm1, [esp]StackFrame.dwColor
		mov			ebp, [esp]StackFrame.lpSrcAlpha
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, offset dwlMMXAlphaMultiplierTable
		mov			ebx, [esp]StackFrame.lpbSrcAlphaToOpacityTable
		punpcklbw	mm1, mm0

loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword
			
		align		8

loop_x_qword:
		
		movzx		eax, byte ptr [ebp]
		movq		mm4, [edi]
		movq		mm2, mm1
		movzx		edx, byte ptr [ebp + 1]
		movq		mm3, mm1
		movzx		eax, byte ptr [ebx + eax]
		movq		mm5, mm4
		punpcklbw	mm4, mm0
		movzx		edx, byte ptr [ebx + edx]
		punpckhbw	mm5, mm0
		movq		mm6, mm4
		pcmpgtw		mm6, mm2
		pand		mm2, mm6
		pandn		mm6, mm4
		por			mm2, mm6
		movq		mm7, mm5
		pcmpgtw		mm7, mm3
		pand		mm3, mm7
		pandn		mm7, mm5
		por			mm3, mm7
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, [esi + eax * 8]
		pmullw		mm3, [esi + edx * 8]
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		add			ebp, 2
		packuswb	mm2, mm3
		movq		[edi], mm2
		add			edi, 8
		loop		loop_x_qword

		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movzx		eax, byte ptr [ebp]
		movd		mm4, [edi]
		movq		mm2, mm1
		movzx		eax, byte ptr [ebx + eax]
		punpcklbw	mm4, mm0
		movq		mm6, mm4
		pcmpgtw		mm6, mm2
		pand		mm2, mm6
		pandn		mm6, mm4
		por			mm2, mm6
		psubw		mm2, mm4
		pmullw		mm2, [esi + eax * 8]
		psrlw		mm2, 8
		paddb		mm2, mm4
		inc			ebp
		packuswb	mm2, mm0
		movd		[edi], mm2
		add			edi, 4

loop_x_end:

		add			ebp, [esp]StackFrame.lAlphaDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_ColorFill_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													  const NxBlt* pNxBlt) const
// �T�v: �]�����A���t�@�`�����l���T�[�t�F�X�t���u�����h�h��ׂ�(�]����A���t�@�ۑ�)
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �A���t�@�`�����l���T�[�t�F�X(8bpp) �ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �A���t�@�`�����l����`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �]����A���t�@�͕ۑ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_ColorFill_BlendSrcAlpha(CNxSurface* pDestSurface, const RECT* lpDestRect,
												  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left) |
		(lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top)) != 0)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_BlendSrcAlpha() : �g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	if (pSrcSurface->GetBitCount() != 8)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_BlendSrcAlpha() : �]�����T�[�t�F�X�� 8bpp �ł͂���܂���.\n");
		return FALSE;
	}

	DWORD dwColor = pNxBlt->nxbColor;
	UINT uOpacity = (dwColor >> 24) & 0xff;		// �A���t�@�͕s�����x������

	if (uOpacity == 0)
		return TRUE;

	RECT rcDest = *lpDestRect;
	RECT rcSrc = *lpSrcRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);

	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	// �A���t�@�`�����l���T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	const BYTE* lpSrcAlpha = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * pSrcSurface->GetPitch() + rcSrc.left;
	LONG lAlphaDistance = pSrcSurface->GetPitch() - uWidth;

	typedef void (__cdecl *FillSrcAlphaFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
											 UINT uWidth, UINT uHeight, DWORD dwColor, const BYTE bySrcAlphaToOpacityTable[256]);

	static const FillSrcAlphaFunc fillSrcAlphaFunc[][2] =
	{
#if defined(NXDRAW_MMX_ONLY)
		/////////////////// MMX
		// 0 ... �ʏ�
		{ Blt_ColorFill_BlendSrcAlphaNormal_MMX, Blt_ColorFill_BlendSrcAlphaNormal_MMX },
		// 1 ... ���Z
		{ Blt_ColorFill_BlendSrcAlphaAdd_MMX, Blt_ColorFill_BlendSrcAlphaAdd_MMX },
		// 2 ... ���Z
		{ Blt_ColorFill_BlendSrcAlphaSub_MMX, Blt_ColorFill_BlendSrcAlphaSub_MMX },
		// 3 ... ��Z
		{ Blt_ColorFill_BlendSrcAlphaMulti_MMX, Blt_ColorFill_BlendSrcAlphaMulti_MMX },
		// 4 ... �X�N���[��
		{ Blt_ColorFill_BlendSrcAlphaScreen_MMX, Blt_ColorFill_BlendSrcAlphaScreen_MMX },
		// 5 ... Brighten
		{ Blt_ColorFill_BlendSrcAlphaBrighten_MMX, Blt_ColorFill_BlendSrcAlphaBrighten_MMX },
		// 6 ... Darken
		{ Blt_ColorFill_BlendSrcAlphaDarken_MMX, Blt_ColorFill_BlendSrcAlphaDarken_MMX },
#else
		/////////////////// i386
		// 0 ... �ʏ�
		{ Blt_ColorFill_BlendSrcAlphaNormal_386, Blt_ColorFill_BlendSrcAlphaNormal_MMX },
		// 1 ... ���Z
		{ Blt_ColorFill_BlendSrcAlphaAdd_386, Blt_ColorFill_BlendSrcAlphaAdd_MMX },
		// 2 ... ���Z
		{ Blt_ColorFill_BlendSrcAlphaSub_386, Blt_ColorFill_BlendSrcAlphaSub_MMX },
		// 3 ... ��Z
		{ Blt_ColorFill_BlendSrcAlphaMulti_386, Blt_ColorFill_BlendSrcAlphaMulti_MMX },
		// 4 ... �X�N���[��
		{ Blt_ColorFill_BlendSrcAlphaScreen_386, Blt_ColorFill_BlendSrcAlphaScreen_MMX },
		// 5 ... Brighten
		{ Blt_ColorFill_BlendSrcAlphaBrighten_386, Blt_ColorFill_BlendSrcAlphaBrighten_MMX },
		// 6 ... Darken
		{ Blt_ColorFill_BlendSrcAlphaDarken_386, Blt_ColorFill_BlendSrcAlphaDarken_MMX },
#endif	// #if defined(NXDRAW_MMX_ONLY)
	};

	(fillSrcAlphaFunc[(pNxBlt->dwFlags & NxBlt::blendTypeMask) - NxBlt::blendNormal][(CNxDraw::GetInstance()->IsMMXEnabled()) ? 1 : 0])
		(lpDestSurface, lpSrcAlpha, lDestDistance, lAlphaDistance, uWidth, uHeight, dwColor, ConstTable::bySrcAlphaToOpacityTable[uOpacity]);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_Normal_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//													 UINT uWidth, UINT uHeight,
//													 DWORD dwColor)
// �T�v: �P���h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface ... �T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance   ... �T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth		  ... ��
//		 UINT uHeight		  ... ����
//		 DWORD dwColor		  ... �h��ׂ��F
// �ߒl: �Ȃ�
// ���l: �]����A���t�@�͏㏑�������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_Normal_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
															UINT /*uWidth*/, UINT /*uHeight*/,
															DWORD /*dwColor*/)
{
#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EDI;
		LPVOID		EBX;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		push		edi
		push		ebx
		movd		mm0, [esp]StackFrame.dwColor
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, [esp]StackFrame.uWidth
		mov			ebx, [esp]StackFrame.uHeight
		punpckldq	mm0, mm0

loop_y:

		mov			eax, edx
		mov			ecx, edx

		test		edi, 7
		jz			qword_aligned

		movd		[edi], mm0
		dec			eax
		add			edi, 4

		mov			ecx, eax

qword_aligned:

		test		eax, 0ffffffe0h
		jz			less_32
		shr			ecx, 5

		; 32 pixels (16 qwords / 128 bytes)

loop_x_qword_32:

		movq		[edi      ], mm0
		movq		[edi +   8], mm0
		movq		[edi +  16], mm0
		movq		[edi +  24], mm0
		movq		[edi +  32], mm0
		movq		[edi +  40], mm0
		movq		[edi +  48], mm0
		movq		[edi +  56], mm0
		add			edi, 128
		sub			eax, 32
		movq		[edi +  64 - 128], mm0
		movq		[edi +  72 - 128], mm0
		movq		[edi +  80 - 128], mm0
		movq		[edi +  88 - 128], mm0
		dec			ecx
		movq		[edi +  96 - 128], mm0
		movq		[edi + 104 - 128], mm0
		movq		[edi + 112 - 128], mm0
		movq		[edi + 120 - 128], mm0
		jnz			loop_x_qword_32
		mov			ecx, eax

less_32:
		
		test		eax, 0ffffffff8h
		jz			less_8
		shr			ecx, 3

		; 8 pixels (4 qwords / 32 bytes)

loop_x_qword_8:

		movq		[edi      ], mm0
		add			edi, 32
		sub			eax, 8
		movq		[edi -   8], mm0
		movq		[edi -  16], mm0
		dec			ecx
		movq		[edi -  24], mm0
		jnz			loop_x_qword_8
		mov			ecx, eax

less_8:

		movd		eax, mm0
		rep			stosd
		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebx
		pop			edi
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_ColorFill_Normal_386(LPBYTE lpDestSurface, LONG lDestDistance,
//													 UINT uWidth, UINT uHeight,
//													 DWORD dwColor)
// �T�v: �P���h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface ... �T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance   ... �T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth		  ... ��
//		 UINT uHeight		  ... ����
//		 DWORD dwColor		  ... �h��ׂ��F
// �ߒl: �Ȃ�
// ���l: �]����A���t�@�͏㏑�������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_Normal_386(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
															UINT /*uWidth*/, UINT /*uHeight*/,
															DWORD /*dwColor*/)
{
#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EIP;
		LPVOID		FLAG;
		LPVOID		EDI;
		LPVOID		EBX;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
	};
#pragma pack(pop)

	__asm
	{
		pushfd			; save direction flag
		push		edi
		push		ebx

		mov			edx, [esp]StackFrame.uWidth
		mov			ebx, [esp]StackFrame.uHeight
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			eax, [esp]StackFrame.dwColor
		cld

loop_y:

		mov			ecx, edx
		rep			stosd
		add			edi, [esp]StackFrame.lDestDistance
		dec			ebx
		jnz			loop_y

		pop			ebx
		pop			edi
		popfd			; restore direction flag
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
//											   const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//											   const NxBlt* pNxBlt) const
// �T�v: �P���h��ׂ�
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �h��ׂ���`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^(���g�p)
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^(���g�p)
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
// ���l: �]����A���t�@�͏㏑�������
////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_ColorFill_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
										   const CNxSurface* /*pSrcSurface*/, const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const
{
	RECT rcDest = *lpDestRect;

	if (!pDestSurface->ClipBltRect(rcDest))
		return TRUE;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �������ݐ�T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	((CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_ColorFill_Normal_MMX : Blt_ColorFill_Normal_386)
		(lpDestSurface, lDestDistance, uWidth, uHeight, pNxBlt->nxbColor);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_RGBAMaskFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													   LONG lDestDistance, LONG lSrcDistance,
//													   UINT uWidth, UINT uHeight, DWORD dwMask)
// �T�v: 8bpp ���� 32bpp �ւ� RGBA �}�X�N�]�� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 DWORD dwMask			  ... �]�����}�X�N
// �ߒl: �Ȃ�
// ���l: (�]���� AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
//		 �]������ 8bpp ����A�����f�[�^����ׂ� 32bpp �֕ϊ����Ă��珈��(��: 83 -> 83838383)
//		 �A���t�@���L��
////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_RGBAMaskFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											LONG lDestDistance, LONG lSrcDistance,
											UINT uWidth, UINT uHeight, DWORD dwMask)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			DWORD dwSrc = ConstTable::dwByteToDwordTable[*lpSrcSurface];		// 0xAB -> 0xABABABAB
			DWORD dwDest = *reinterpret_cast<LPDWORD>(lpDestSurface);
			*(LPDWORD)lpDestSurface = (dwDest & ~dwMask) | (dwSrc & dwMask);
			lpDestSurface += 4;
			lpSrcSurface++;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_RGBAMaskFrom32_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														LONG lDestDistance, LONG lSrcDistance,
//														UINT uWidth, UINT uHeight, DWORD dwMask)
// �T�v: 32bpp ���� 32bpp �ւ� RGBA �}�X�N�]�� (386 ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 DWORD dwMask			  ... �]�����}�X�N
// �ߒl: �Ȃ�
// ���l: (�]���� AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
//		 �A���t�@���L��
////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_RGBAMaskFrom32_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
											 LONG lDestDistance, LONG lSrcDistance,
											 UINT uWidth, UINT uHeight, DWORD dwMask)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
			DWORD dwDest = *reinterpret_cast<LPDWORD>(lpDestSurface);

			*(LPDWORD)lpDestSurface = (dwDest & ~dwMask) | (dwSrc & dwMask);
			lpDestSurface += 4;
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		lpSrcSurface += lSrcDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Blt_RGBAMaskFrom32_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														LONG lDestDistance, LONG lSrcDistance,
//														UINT uWidth, UINT uHeight, DWORD dwMask)
// �T�v: 32bpp ���� 32bpp �ւ� RGBA �}�X�N�]�� (MMX ��)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
//		 DWORD dwMask			  ... �]�����}�X�N
// �ߒl: �Ȃ�
// ���l: (�]���� AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
//		 �A���t�@���L��
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_RGBAMaskFrom32_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
															   LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
															   UINT /*uWidth*/, UINT /*uHeight*/, DWORD /*dwMask*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwMask;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		edi
		movd		mm0, [esp]StackFrame.dwMask
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			ebx, [esp]StackFrame.lpSrcSurface
		pcmpeqb		mm2, mm2		; load 0xffffffffffffffff
		punpckldq	mm0, mm0
		mov			eax, [esp]StackFrame.uWidth
		mov			edx, [esp]StackFrame.uHeight
		movq		mm1, mm0		; mm1 = mask
		pandn		mm0, mm2		; mm0 = ~mask

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm4, [edi]
		movq		mm5, [ebx]
		add			ebx, 8
		pand		mm4, mm0		; dest & ~mask
		pand		mm5, mm1		; src & mask
		por			mm4, mm5		; dest = (dest & ~mask) | (src & mask)
		movq		[edi], mm4
		add			edi, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [edi]
		movd		mm5, [ebx]
		add			ebx, 4
		pand		mm4, mm0		; dest & ~mask
		pand		mm5, mm1		; src & mask
		por			mm4, mm5		; dest = (dest & ~mask) | (src & mask)
		movd		[edi], mm4
		add			edi, 4

loop_x_end:

		add			edi, [esp]StackFrame.lDestDistance
		add			ebx, [esp]StackFrame.lSrcDistance
		dec			edx
		jnz			loop_y

		pop			edi
		pop			ebx
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
//									   const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//									   const NxBlt* pNxBlt) const
// �T�v: RGBA �}�X�N�]��
// ����: CNxSurface* pDestSurface ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: (�]���� AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
//		 32bpp -> 32bpp �� 8bpp -> 32bpp ���T�|�[�g
//		 �A���t�@���L��
////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
								   const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	if (((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left) |
		(lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top)) != 0)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_RGBAMask() : �g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	RECT rcDest = *lpDestRect;
	RECT rcSrc = *lpSrcRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;
	
	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	// �]�����T�[�t�F�X�������ւ̃|�C���^�Ƌ���
	LONG lSrcPitch = pSrcSurface->GetPitch();
	LONG lSrcDistance;
	const BYTE* lpSrcSurface;
	DWORD dwMask = *reinterpret_cast<const DWORD*>(&pNxBlt->nxbRGBAMask.byBlue);

	switch (pSrcSurface->GetBitCount())
	{
	case 32:		// 32bpp ����̓]��
		lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * pSrcSurface->GetPitch() + rcSrc.left * 4;
		lSrcDistance = lSrcPitch - uWidth * 4;
		((CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_RGBAMaskFrom32_MMX : Blt_RGBAMaskFrom32_386)
			(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, dwMask);
		break;
	case 8:			// 8bpp ����̓]��
		lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * pSrcSurface->GetPitch() + rcSrc.left;
		lSrcDistance = lSrcPitch - uWidth;
		Blt_RGBAMaskFrom8_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, dwMask);
		break;
	default:
		_RPTF2(_CRT_WARN, "CNxCustomDraw32::Blt_RGBAMask() : �T�|�[�g����Ă��Ȃ��r�b�g�[�x��(%d -> %d)�̓]���ł�.\n", pSrcSurface->GetBitCount(), pDestSurface->GetBitCount());
		return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_ColorFill_RGBAMask_386(LPBYTE lpDestSurface, LONG lDestDistance,
//													 UINT uWidth, UINT uHeight,
//													 DWORD dwMask)
// �T�v: RGBA �}�X�N�h��ׂ� (386 ��)
// ����: LPBYTE lpDestSurface  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance    ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidht		   ... ��
//		 UINT uHeight		   ... ����
//		 DWORD dwColor		   ... �h��ׂ��F
//		 DWORD dwMask		   ... �}�X�N
// �ߒl: �Ȃ�
// ���l: (dwColor AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
//		 �A���t�@���L��
////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_RGBAMask_386(LPBYTE lpDestSurface, LONG lDestDistance,
												 UINT uWidth, UINT uHeight,
												 DWORD dwColor, DWORD dwMask)
{
	DWORD dwSrc = dwColor & dwMask;
	DWORD dwDestMask = ~dwMask;

	do
	{
		UINT uColumn = uWidth;
		do
		{
			DWORD dwDest = *reinterpret_cast<LPDWORD>(lpDestSurface);
			*reinterpret_cast<LPDWORD>(lpDestSurface) = (dwDest & dwDestMask) | dwSrc;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_ColorFill_RGBAMask_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//													 UINT uWidth, UINT uHeight,
//													 DWORD dwMask)
// �T�v: RGBA �}�X�N�h��ׂ� (MMX ��)
// ����: LPBYTE lpDestSurface  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance    ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 UINT uWidht		   ... ��
//		 UINT uHeight		   ... ����
//		 DWORD dwColor		   ... �h��ׂ��F
//		 DWORD dwMask		   ... �}�X�N
// �ߒl: �Ȃ�
// ���l: (dwColor AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
//		 �A���t�@���L��
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_ColorFill_RGBAMask_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																   UINT /*uWidth*/, UINT /*uHeight*/, DWORD /*dwColor*/, DWORD /*dwMask*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EBX;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		DWORD		dwColor;
		DWORD		dwMask;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		movd		mm0, [esp]StackFrame.dwMask
		movd		mm2, [esp]StackFrame.dwColor
		mov			ebx, [esp]StackFrame.lpDestSurface
		pcmpeqb		mm3, mm3							// load 0xffff_ffff_ffff_ffff
		punpckldq	mm0, mm0							// (dwMask << 32) | dwMask
		punpckldq	mm2, mm2							// (dwColor << 32) | dwColor
		mov			eax, [esp]StackFrame.uWidth
		mov			edx, [esp]StackFrame.uHeight
		pand		mm2, mm0
		pandn		mm0, mm3

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:

		movq		mm4, [ebx]
		pand		mm4, mm0
		por			mm4, mm2
		movq		[ebx], mm4
		add			ebx, 8
		loop		loop_x_qword

		test		eax, 1
		jz			loop_x_end

skip_x_qword:

		movd		mm4, [ebx]
		pand		mm4, mm0
		por			mm4, mm2
		movd		[ebx], mm4
		add			ebx, 4

loop_x_end:

		add			ebx, [esp]StackFrame.lDestDistance
		dec			edx
		jnz			loop_y

		pop			ebx
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_ColorFill_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												 const NxBlt* pNxBlt) const
// �T�v: RGBA �}�X�N�h��ׂ�
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �h��ׂ���`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... ���g�p
//		 const RECT* lpSrcRect		   ... ���g�p
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: (color AND dwMask) OR (�]���� AND (NOT dwMask)) ���s�Ȃ�
//		 �A���t�@���L��
/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_ColorFill_RGBAMask(CNxSurface* pDestSurface, const RECT* lpDestRect,
											 const CNxSurface* /*pSrcSurface*/,
											 const RECT* /*lpSrcRect*/, const NxBlt* pNxBlt) const
{
	DWORD dwMask = *reinterpret_cast<const DWORD*>(&pNxBlt->nxbRGBAMask.byBlue);

	// mask �� 0xffffffff (color ���S�r�b�g�L��)�Ȃ�΁A�P���ɓh��ׂ�
	if (dwMask == 0xffffffff)
		return CNxCustomDraw32::Blt_ColorFill_Normal(pDestSurface, lpDestRect, NULL, NULL, pNxBlt);

	// �N���b�v
	RECT rcDest = *lpDestRect;
	if (!pDestSurface->ClipBltRect(rcDest))
		return TRUE;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �������ݐ�T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	((CNxDraw::GetInstance()->IsMMXEnabled()) ? Blt_ColorFill_RGBAMask_MMX : Blt_ColorFill_RGBAMask_386)
		(lpDestSurface, lDestDistance, uWidth, uHeight, pNxBlt->nxbColor, dwMask);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_Normal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//										 LONG lDestDistance, LONG lSrcDistance, LONG lSrcDelta,
//										 UINT uWidth, UINT uHeight)
// �T�v: �ʏ� Blt (���E���]�Ή�)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDelta			  ... �]�����̕ψ�(4 or -4)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_Normal_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
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
		mov			esi, [esp]StackFrame.lpSrcSurface
		test		edx, edx
		mov			edx, [esp]StackFrame.uWidth
		jns			normal

		// reverse copy

reverse_loop_y:

		mov			ecx, edx

reverse_loop_x:
		
//		mov			eax, [esi + 0]
		_emit 0x8b	; avoid vector decode (AMD K6)
		_emit 0x46
		_emit 0x00
		sub			esi, 4
		mov			[edi], eax
		add			edi, 4
		loop		reverse_loop_x
	
		add			esi, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			reverse_loop_y

		pop			edi
		pop			esi
		ret

		// normal copy

normal:

		mov			ecx, edx
		mov			eax, [esp]StackFrame.uHeight

normal_loop_y:

		rep			movsd

		mov			ecx, edx
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
//	void CNxCustomDraw32::Blt_NormalFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//											  LONG lDestDistance, LONG lSrcDistance, LONG lSrcDelta,
//											  const NxColor* pColorTable, UINT uWidth, UINT uHeight)
// �T�v: 8bpp ����� �ʏ� Blt (���E���]�Ή�)
// ����: LPBYTE lpDestSurface	  ... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 	  ... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDistance		  ... �]�����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcDelta			  ... �]�����̕ψ�(4 or -4)
//		 UINT uWidth			  ... ��
//		 UINT uHeight			  ... ����
// �ߒl: �Ȃ�
// ���l: pColorTable �̐F���g�p����B�A���t�@�� 0xff �ƌ��Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_NormalFrom8_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
															LONG /*lDestDistance*/, LONG /*lSrcDistance*/, LONG /*lSrcDelta*/,
															const NxColor* /*pColorTable*/, UINT /*uWidth*/, UINT /*uHeight*/)
{
#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		EDI;
		LPVOID		ESI;
		LPVOID		EBP;
		LPVOID		EBX;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		const BYTE*	lpSrcSurface;
		LONG		lDestDistance;
		LONG		lSrcDistance;
		LONG		lSrcDelta;
		const NxColor* pColorTable;
		UINT		uWidth;
		UINT		uHeight;
	};
#pragma pack(pop)

	__asm
	{
		push		ebx
		push		ebp
		push		esi
		push		edi
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			edx, [esp]StackFrame.lpSrcSurface
		mov			ebp, [esp]StackFrame.pColorTable

loop_y:

		mov			eax, [esp]StackFrame.lSrcDelta
		mov			ecx, [esp]StackFrame.uWidth
		or			eax, eax
		jns			normal_x

;reverse_x:

		shr			ecx, 2
		jz			reverse_x_byte

reverse_loop_x_dword:

		mov			ebx, [edx - 3]
		movzx		eax, bl
		shr			ebx, 8
		movzx		esi, bl
		shr			ebx, 8
		mov			eax, [ebp + eax * 4]
		mov			esi, [ebp + esi * 4]
		or			eax, 0ff000000h				; alpha = 255
		or			esi, 0ff000000h				; alpha = 255
		mov			[edi + 12] ,eax
		movzx		eax, bl
		shr			ebx, 8
		mov			[edi +  8], esi
		mov			eax, [ebp + eax * 4]
		mov			ebx, [ebp + ebx * 4]
		or			eax, 0ff000000h				; alpha = 255
		or			ebx, 0ff000000h				; alpha = 255
		sub			edx, 4
		mov			[edi +  4], eax
		mov			[edi +  0], ebx
		add			edi, 16
		loop		reverse_loop_x_dword

reverse_x_byte:

		mov			ecx, [esp]StackFrame.uWidth
		and			ecx, 3
		jz			x_end

reverse_loop_x_byte:

		movzx		eax, byte ptr [edx]
		dec			edx
		mov			eax, [ebp + eax * 4]
		mov			[edi], eax
		add			edi, 4
		loop		reverse_loop_x_byte
		jmp			short x_end

normal_x:

		shr			ecx, 2
		jz			normal_x_byte

normal_loop_x_dword:

		mov			ebx, [edx]
		add			edx, 4
		movzx		eax, bl
		shr			ebx, 8
		movzx		esi, bl
		shr			ebx, 8
		mov			eax, [ebp + eax * 4]
		mov			esi, [ebp + esi * 4]
		mov			[edi +  0] ,eax
		movzx		eax, bl
		shr			ebx, 8
		mov			[edi +  4], esi
		mov			eax, [ebp + eax * 4]
		mov			ebx, [ebp + ebx * 4]
		mov			[edi +  8], eax
		mov			[edi + 12], ebx
		add			edi, 16
		loop		normal_loop_x_dword

normal_x_byte:

		mov			ecx, [esp]StackFrame.uWidth
		and			ecx, 3
		jz			x_end

normal_loop_x_byte:

		movzx		eax, byte ptr [edx]
		inc			edx
		mov			eax, [ebp + eax * 4]
		mov			[edi], eax
		add			edi, 4
		loop		normal_loop_x_byte

x_end:

		add			edx, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebp
		pop			ebx
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_NormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															LONG lDestDistance, LONG lSrcPitch,
//															const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �o�C���j�A�t�B���^�g��k�� Blt (386)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 			... �]�����T�[�t�F�X�̎��̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_NormalStretchLinearFilter_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
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
			*reinterpret_cast<LPDWORD>(lpDestSurface)
				= BiLinearFilter32(ul64SrcOrgX.LowPart, uSrcOrgY, lpSrcSurface + ul64SrcOrgX.HighPart * 4, lSrcPitch);
			lpDestSurface += 4;
			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
		uSrcOrgY = ul64SrcOrgY.LowPart;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_NormalStretchLinearFilterFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																 LONG lDestDistance, LONG lSrcPitch, const NxColor* pColorTable,
//																 const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: 8bpp ����̃o�C���j�A�t�B���^�g��k�� Blt (386)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 			... �]�����T�[�t�F�X�̎��̍s�ւ̃|�C���^���Z�l
//		 const NxColor* pColorTable ... �J���[�e�[�u��
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_NormalStretchLinearFilterFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
															 LONG lDestDistance, LONG lSrcPitch, const NxColor* pColorTable,
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
			*reinterpret_cast<LPDWORD>(lpDestSurface)
				= BiLinearFilterNoRegardAlpha8(ul64SrcOrgX.LowPart, uSrcOrgY, lpSrcSurface + ul64SrcOrgX.HighPart, lSrcPitch, pColorTable);
			lpDestSurface += 4;
			ul64SrcOrgX.QuadPart += pStretchBltInfo->ul64SrcDeltaX.QuadPart;
		} while (--uColumn != 0);
		lpDestSurface += lDestDistance;
		ULARGE_INTEGER ul64SrcOrgY = pStretchBltInfo->ul64SrcDeltaY;
		ul64SrcOrgY.QuadPart += uSrcOrgY;
		lpSrcSurface += lSrcPitch * ul64SrcOrgY.HighPart;
		uSrcOrgY = ul64SrcOrgY.LowPart;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_NormalStretchLinearFilter_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															LONG lDestDistance, LONF lSrcPitch,
//															const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �o�C���j�A�t�B���^�g��k�� Blt (MMX)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 			... �]�����T�[�t�F�X�̎��̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_NormalStretchLinearFilter_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
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
		pxor		mm0, mm0

loop_y:

		movd		mm7, [esp]StackFrame.uSrcOrgY
		mov			ebp, 0
		mov			eax, edx
		psrld		mm7, 24
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		punpcklwd	mm7, mm7
		mov			ecx, [esp]StackFrame.uWidth
		punpckldq	mm7, mm7
		mov			ebx, [esp]StackFrame.pStretchBltInfo
loop_x:
		movd		mm6, esi
		movq		mm1, [eax]
		add			eax, [esp]StackFrame.lSrcPitch
		psrld		mm6, 24
		movq		mm2, mm1
		punpcklwd	mm6, mm6
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		movq		mm3, [eax]
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0
		movq		mm4, mm3
		punpckldq	mm6, mm6
		punpcklbw	mm3, mm0
		punpckhbw	mm4, mm0
		psubw		mm2, mm1
		psubw		mm4, mm3
		pmullw		mm2, mm6
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		pmullw		mm4, mm6
		psrlw		mm2, 8
		psrlw		mm4, 8
		paddb		mm2, mm1
		paddb		mm4, mm3
		lea			eax, [edx + ebp * 4]
		psubw		mm4, mm2
		pmullw		mm4, mm7
		psrlw		mm4, 8
		paddb		mm4, mm2
		packuswb	mm4, mm0
		movd		[edi], mm4
		add			edi, 4
		loop		loop_x

		mov			esi, [ebx]StretchBltInfo.ul64SrcDeltaY.LowPart
		mov			eax, [esp]StackFrame.lSrcPitch
		mov			ebp, [ebx]StretchBltInfo.ul64SrcDeltaY.HighPart
		add			esi, [esp]StackFrame.uSrcOrgY
		adc			ebp, 0
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
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_NormalStretchLinearFilterFrom8_MMX(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																 LONG lDestDistance, LONF lSrcPitch, const NxColor* pColorTable,
//																 const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: 8bpp ����̃o�C���j�A�t�B���^�g��k�� Blt (MMX)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 			... �]�����T�[�t�F�X�̎��̍s�ւ̃|�C���^���Z�l
//		 const NxColor* pColorTable ... �J���[�e�[�u��
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_NormalStretchLinearFilterFrom8_MMX(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																		       LONG /*lDestDistance*/, LONG /*lSrcPitch*/, const NxColor* /*pColorTable*/,
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
		const NxColor* pColorTable;
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
		pxor		mm0, mm0

loop_y:

		movd		mm7, [esp]StackFrame.uSrcOrgY
		mov			ebp, 0
		mov			eax, edx
		psrld		mm7, 24
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		punpcklwd	mm7, mm7
		mov			ecx, [esp]StackFrame.uWidth
		punpckldq	mm7, mm7
		mov			ebx, [esp]StackFrame.pStretchBltInfo
loop_x:
		movd		mm5, edx
		mov			ebx, [esp]StackFrame.pColorTable
		movzx		edx, byte ptr [eax]
		movd		mm6, esi
		movd		mm1, [ebx + edx * 4]
		movzx		edx, byte ptr [eax + 1]
		psrld		mm6, 24
		movd		mm2, [ebx + edx * 4]
		punpcklbw	mm1, mm0
		add			eax, [esp]StackFrame.lSrcPitch
		punpcklbw	mm2, mm0
		movzx		edx, byte ptr [eax]
		punpcklwd	mm6, mm6
		movd		mm3, [ebx + edx * 4]
		movzx		edx, byte ptr [eax + 1]
		punpcklbw	mm3, mm0
		movd		mm4, [ebx + edx * 4]
		mov			ebx, [esp]StackFrame.pStretchBltInfo
		punpcklbw	mm4, mm0
		punpckldq	mm6, mm6
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		psubw		mm2, mm1
		psubw		mm4, mm3
		pmullw		mm2, mm6
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		pmullw		mm4, mm6
		movd		edx, mm5
		psrlw		mm2, 8
		psrlw		mm4, 8
		paddb		mm2, mm1
		paddb		mm4, mm3
		lea			eax, [edx + ebp]
		psubw		mm4, mm2
		pmullw		mm4, mm7
		psrlw		mm4, 8
		paddb		mm4, mm2
		packuswb	mm4, mm0
		movd		[edi], mm4
		add			edi, 4
		dec			ecx
		jnz			loop_x

		mov			esi, [ebx]StretchBltInfo.ul64SrcDeltaY.LowPart
		mov			eax, [esp]StackFrame.lSrcPitch
		mov			ebp, [ebx]StretchBltInfo.ul64SrcDeltaY.HighPart
		add			esi, [esp]StackFrame.uSrcOrgY
		adc			ebp, 0
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
		emms
		ret
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_NormalStretch_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//												LONG lDestDistance, LONG lSrcPitch
//												const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: �g��k�� Blt (filter ����)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 			... �]�����T�[�t�F�X�̎��̍s�ւ̃|�C���^���Z�l
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_NormalStretch_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
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

		mov			eax, [edx + ebp * 4]
		add			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		mov			[edi], eax
		adc			ebp, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		add			edi, 4
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	void CNxCustomDraw32::Blt_NormalStretchFrom8_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													 LONG lDestDistance, LONG lSrcPitch, const NxColor* pColorTable,
//													 UINT uWidth, UINT uHeight,
//													 const CNxSurface::StretchBltInfo* pStretchBltInfo)
// �T�v: 8bpp ����� �g��k�� Blt (filter ����)
// ����: LPBYTE lpDestSurface		... �]����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const BYTE* lpSrcSurface	... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 LONG lDestDistance 		... �]����T�[�t�F�X�̍s�̒[���玟�̍s�ւ̃|�C���^���Z�l
//		 LONG lSrcPitch 			... �]�����T�[�t�F�X�̎��̍s�ւ̃|�C���^���Z�l
//		 const NxColor* pColorTable ... �J���[�e�[�u���ւ̃|�C���^
//		 UINT uWidth				... ��
//		 UINT uHeight				... ����
//		 const CNxSurface::StretchBltInfo* pStretchBltInfo
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Blt_NormalStretchFrom8_386(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																   LONG /*lDestDistance*/, LONG /*lSrcPitch*/,
																   const NxColor* /*pColorTable*/,
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
		ULARGE_INTEGER ul64SrcDeltaX;		// StretchBltInfo.ul64SrcDeltaX �̃R�s�[
		LPVOID		   EIP;
		LPBYTE		   lpDestSurface;
		const BYTE*	   lpSrcSurface;
		LONG		   lDestDistance;
		LONG		   lSrcPitch;
		const NxColor* pColorTable;
		UINT		   uWidth;
		UINT		   uHeight;
		const CNxSurface::StretchBltInfo* pStretchBltInfo;
	};
#pragma pack(pop)

	typedef CNxSurface::StretchBltInfo StretchBltInfo;

	__asm
	{
		sub			esp, 12
		push		ebp
		push		ebx
		push		esi
		push		edi

		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			edx, [esp]StackFrame.lpSrcSurface
		mov			eax, [ebx]StretchBltInfo.uSrcOrgY
		mov			ecx, [ebx]StretchBltInfo.ul64SrcDeltaX.LowPart
		mov			esi, [ebx]StretchBltInfo.ul64SrcDeltaX.HighPart
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			[esp]StackFrame.ul64SrcDeltaX.LowPart, ecx
		mov			[esp]StackFrame.ul64SrcDeltaX.HighPart, esi
		mov			[esp]StackFrame.uSrcOrgY, eax

loop_y:

		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			ebp, 0
		mov			ecx, [esp]StackFrame.uWidth
		mov			esi, [ebx]StretchBltInfo.uSrcOrgX
		mov			ebx, [esp]StackFrame.pColorTable

loop_x:

		movzx		eax, byte ptr [edx + ebp]
		add			esi, [esp]StackFrame.ul64SrcDeltaX.LowPart
		mov			eax, [ebx + eax * 4]
		adc			ebp, [esp]StackFrame.ul64SrcDeltaX.HighPart
		mov			[edi], eax
		add			edi, 4
		loop		loop_x

		mov			ebx, [esp]StackFrame.pStretchBltInfo
		mov			eax, [esp]StackFrame.lSrcPitch
		mov			esi, [ebx]StretchBltInfo.ul64SrcDeltaY.LowPart
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
		add			esp, 12
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
//									 const CNxSurace* pSrcSurface, const RECT* lpSrcRect,
//									 const NxBlt* pNxBlt) const
// �T�v: �ʏ� Blt (�L�k�Ή�)
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�̃r�b�g�f�[�^�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �]����A���t�@�͏㏑�������
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_Normal(CNxSurface* pDestSurface, const RECT* lpDestRect,
								 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt) const
{
	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	LONG lSrcPitch = pSrcSurface->GetPitch();

	if (((lpDestRect->right - lpDestRect->left) - abs(lpSrcRect->right - lpSrcRect->left) |
		(lpDestRect->bottom - lpDestRect->top) - abs(lpSrcRect->bottom - lpSrcRect->top)) == 0)
	{
		// �g��k������

		// �N���b�v
		RECT rcSrcClip;
		pSrcSurface->GetRect(&rcSrcClip);
		if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;

		UINT uSrcPixelSize = pSrcSurface->GetBitCount() / 8;
		
		// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
		UINT uWidth = rcDest.right - rcDest.left;
		UINT uHeight = rcDest.bottom - rcDest.top;

		LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
		LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;

		// ���]�̏���
		// ���E���]�̏ꍇ rcSrc.right �͉E�[ + 1, rcSrc.left �͍��[
		// �㉺���]�̏ꍇ rcSrc.bottom �͉��[ + 1, rcSrc.top �͏�[
		// �N���b�v��̋�`���甽�]�̔��ʂ͂ł��Ȃ����߁A�I���W�i���� lpSrcRect ���g�p����

		LONG lSrcDelta = uSrcPixelSize;		// X �R�s�[����
		if (lpSrcRect->right - lpSrcRect->left < 0)
		{	// ���E���]
			lSrcDelta = -lSrcDelta;
			rcSrc.left = rcSrc.right - 1;	// �J�n�� rcSrc.right - 1 (�E�[) �Ƃ���
		}
		if (lpSrcRect->bottom - lpSrcRect->top < 0)
		{	// �㉺���]
			lSrcPitch = -lSrcPitch;			// �]���� Pitch �������]
			rcSrc.top = -(rcSrc.bottom - 1);	// �J�n�� rcSrc.bottom - 1 �Ƃ��āA�����𔽓](�]���� Pitch �������]�ɍ��킹��)
		}

		// �]�����T�[�t�F�X�̃|�C���^���v�Z
		LONG lSrcDistance = lSrcPitch - uWidth * lSrcDelta;
		const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * lSrcPitch
															+ rcSrc.left * uSrcPixelSize;

		if (pSrcSurface->GetBitCount() == 32)
			Blt_Normal_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance,
						   lSrcDelta, uWidth, uHeight);
		else
		{
			Blt_NormalFrom8_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance,
						        lSrcDelta, pSrcSurface->GetColorTable(), uWidth, uHeight);
		}
	}
	else
	{
		if (pSrcSurface->GetBitCount() == 32)
		{
			// �]���� 32bpp
			void (*pfnStretchBltFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance,
									  LONG lSrcPitch, UINT uWidth, UINT uHeight,
									  const CNxSurface::StretchBltInfo* pStretchBltInfo);

			CNxSurface::StretchBltInfo stretchBltInfo;
			BOOL bFilter = (pNxBlt != NULL) && (pNxBlt->dwFlags & NxBlt::linearFilter);
			if (!PreprocessStretchBlt(pDestSurface, &stretchBltInfo, &rcDest, pSrcSurface, &rcSrc, &bFilter))
				return TRUE;

			if (bFilter)
			{	// filter
				pfnStretchBltFunc = CNxDraw::GetInstance()->IsMMXEnabled() ? Blt_NormalStretchLinearFilter_MMX : Blt_NormalStretchLinearFilter_386;
			}
			else
			{	// no filter
				pfnStretchBltFunc = Blt_NormalStretch_386;
			}

			// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
			UINT uWidth = rcDest.right - rcDest.left;
			UINT uHeight = rcDest.bottom - rcDest.top;

			LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
			LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;

			const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * lSrcPitch + rcSrc.left * 4;
			(pfnStretchBltFunc)(lpDestSurface, lpSrcSurface, lDestDistance, lSrcPitch, uWidth, uHeight, &stretchBltInfo);
	
		}
		else
		{
			// �]���� 8bpp
			void (*pfnStretchBltFunc)(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance,
									  LONG lSrcPitch, const NxColor* ColorTable, UINT uWidth, UINT uHeight,
									  const CNxSurface::StretchBltInfo* pStretchBltInfo);

			CNxSurface::StretchBltInfo stretchBltInfo;
			BOOL bFilter = (pNxBlt != NULL) && (pNxBlt->dwFlags & NxBlt::linearFilter);
			if (!PreprocessStretchBlt(pDestSurface, &stretchBltInfo, &rcDest, pSrcSurface, &rcSrc, &bFilter))
				return TRUE;

			if (bFilter)
			{	// filter
				pfnStretchBltFunc = CNxDraw::GetInstance()->IsMMXEnabled() ? Blt_NormalStretchLinearFilterFrom8_MMX : Blt_NormalStretchLinearFilterFrom8_386;
			}
			else
			{	// no filter
				pfnStretchBltFunc = Blt_NormalStretchFrom8_386;
			}

			// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
			UINT uWidth = rcDest.right - rcDest.left;
			UINT uHeight = rcDest.bottom - rcDest.top;

			LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
			LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + rcDest.top * pDestSurface->GetPitch() + rcDest.left * 4;

			const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + rcSrc.top * lSrcPitch + rcSrc.left;
			(pfnStretchBltFunc)(lpDestSurface, lpSrcSurface, lDestDistance, lSrcPitch, pSrcSurface->GetColorTable(), uWidth, uHeight, &stretchBltInfo);
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxCustomDraw32::Blt_BlurHorz_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//										   LONG lDestDistance, LONG lSrcDistance,
//										   UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin, UINT uRange)
// �T�v: �����ڂ����]�� 386
// ����: UINT uLeftMargin  ... �]�������ォ�獶(�̊O)���փA�N�Z�X�\�ȃh�b�g��
//		 UINT uRightMargin ... �]�������ォ��E���ւ̃A�N�Z�X�\�ȃh�b�g��(uWidth �Ɠ������A����ȏ�ɂȂ�)
//		 UINT uRange	   ... �ڂ����̋���(1 �` 127)
// ���l: ���Ƃ��� uRange = 63 �̏ꍇ�A����(63) + ���_(1) + �E��(63) = 127dot �̕��ς����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlurHorz_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
									   UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin, UINT uRange)
{
	UINT uMultiplier = 65536 / (uRange * 2 + 1);
	do
	{
		DWORD dwLeftRedBlue;		// rrrrbbbb
		DWORD dwLeftAlphaGreen;		// aaaagggg
		DWORD dwRedBlue;			// rrrrbbbb
		DWORD dwAlphaGreen;			// aaaagggg

		dwRedBlue = 0x00000000;
		dwAlphaGreen = 0x00000000;
		dwLeftRedBlue = 0x00000000;
		dwLeftAlphaGreen = 0x00000000;

		UINT uLeftMarginCount;
		if (uLeftMargin != 0)
		{
			UINT uColumn = min(uLeftMargin, uRange);
			lpSrcSurface -= uColumn * 4;
			do
			{
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
				lpSrcSurface += 4;
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			} while (--uColumn != 0);
		}
		if (uLeftMargin >= uRange)
		{
			uLeftMarginCount = 0;
		}
		else
		{
			DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface - uLeftMargin * 4);
			uLeftMarginCount = uRange - uLeftMargin;
			dwLeftRedBlue = dwSrc & 0x00ff00ff;
			dwLeftAlphaGreen = (dwSrc >> 8) & 0x00ff00ff;
			dwRedBlue += dwLeftRedBlue * uLeftMarginCount;
			dwAlphaGreen += dwLeftAlphaGreen * uLeftMarginCount;
		}
		if (uRange >= 2)
		{
			if (uRightMargin - 1 != 0)
			{
				UINT uColumn = min(uRightMargin - 1, uRange - 1);
				lpSrcSurface += uColumn * 4;
				do
				{
					DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
					dwRedBlue += dwSrc & 0x00ff00ff;
					dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
					lpSrcSurface -= 4;
				} while (--uColumn != 0);
			}
			if (uRightMargin < uRange - 1)
			{
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + (uRightMargin - 1) * 4);
				dwRedBlue += (dwSrc & 0x00ff00ff) * (uRange - uRightMargin - 1);
				dwAlphaGreen += ((dwSrc >> 8) & 0x00ff00ff) * (uRange - uRightMargin - 1);
			}
		}
		UINT uRightMarginCount = uRightMargin - uRange - 1;	// �A�N�Z�X�\�ȉE���̃s�N�Z��
		UINT uColumn = uWidth;

		// �������̃s�N�Z�������Z
		DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
		dwRedBlue += dwSrc & 0x00ff00ff;
		dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
		
		do
		{	// �E���̃s�N�Z���� 1dot ���Z
			if (static_cast<int>(uRightMarginCount--) > 0)
			{	// �A�N�Z�X�\�Ȕ͈͓�
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + uRange * 4);
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			}
			else
			{	// �͈͊O�B�E�[�̃s�N�Z���ŕ�U
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + (uRightMarginCount + uRange + 1) * 4);
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			}

			// ���Z���ʂ����Z���ď����o��
			*(lpDestSurface + 0) = static_cast<BYTE>((uMultiplier * (dwRedBlue & 0xffff) + 0x8000) >> 16);		// B
			*(lpDestSurface + 1) = static_cast<BYTE>((uMultiplier * (dwAlphaGreen & 0xffff) + 0x8000) >> 16);	// G
			*(lpDestSurface + 2) = static_cast<BYTE>((uMultiplier * (dwRedBlue >> 16) + 0x8000) >> 16);			// R
			*(lpDestSurface + 3) = static_cast<BYTE>((uMultiplier * (dwAlphaGreen >> 16) + 0x8000) >> 16);		// A
			lpDestSurface += 4;

			// ���Z���ʂ���A���[�̃s�N�Z���l�����Z
			if (uLeftMarginCount != 0)
			{	// �͈͊O�B�ۑ����Ă��������[�s�N�Z�������Z
				uLeftMarginCount--;
				dwRedBlue -= dwLeftRedBlue;
				dwAlphaGreen -= dwLeftAlphaGreen;
			}
			else
			{	// �͈͓�
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface - uRange * 4);
				dwRedBlue -= (dwSrc & 0x00ff00ff);
				dwAlphaGreen -= (dwSrc >> 8) & 0x00ff00ff;
			}
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while(--uHeight != 0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxCustomDraw32::Blt_BlurHorzBlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//													  LONG lDestDistance, LONG lSrcDistance,
//													  UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
//													  UINT uRange, UINT uOpacity)
// �T�v: �����ڂ����ʏ�u�����h�]�� 386
// ����: UINT uLeftMargin  ... �]�������ォ�獶(�̊O)���փA�N�Z�X�\�ȃh�b�g��
//       UINT uRightMargin ... �]�������ォ��E���ւ̃A�N�Z�X�\�ȃh�b�g��(uWidth �Ɠ������A����ȏ�ɂȂ�)
//       UINT uRange       ... �ڂ����̋���(1 �` 127)
// ���l: ���Ƃ��� uRange = 63 �̏ꍇ�A����(63) + ���_(1) + �E��(63) = 127dot �̕��ς����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning (push)
#pragma warning (disable : 4201)

void CNxCustomDraw32::Blt_BlurHorzBlendNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
												  UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
												  UINT uRange, UINT uOpacity)
{
	UINT uMultiplier = 65536 / (uRange * 2 + 1);
	do
	{
		DWORD dwLeftRedBlue;		// rrrrbbbb
		DWORD dwLeftAlphaGreen;		// aaaagggg
		DWORD dwRedBlue;			// rrrrbbbb
		DWORD dwAlphaGreen;			// aaaagggg

		dwRedBlue = 0x00000000;
		dwAlphaGreen = 0x00000000;
		dwLeftRedBlue = 0x00000000;
		dwLeftAlphaGreen = 0x00000000;

		UINT uLeftMarginCount;
		if (uLeftMargin != 0)
		{
			UINT uColumn = min(uLeftMargin, uRange);
			lpSrcSurface -= uColumn * 4;
			do
			{
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
				lpSrcSurface += 4;
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			} while (--uColumn != 0);
		}
		if (uLeftMargin >= uRange)
		{
			uLeftMarginCount = 0;
		}
		else
		{
			DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface - uLeftMargin * 4);
			uLeftMarginCount = uRange - uLeftMargin;
			dwLeftRedBlue = dwSrc & 0x00ff00ff;
			dwLeftAlphaGreen = (dwSrc >> 8) & 0x00ff00ff;
			dwRedBlue += dwLeftRedBlue * uLeftMarginCount;
			dwAlphaGreen += dwLeftAlphaGreen * uLeftMarginCount;
		}
		if (uRange >= 2)
		{
			if (uRightMargin - 1 != 0)
			{
				UINT uColumn = min(uRightMargin - 1, uRange - 1);
				lpSrcSurface += uColumn * 4;
				do
				{
					DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
					dwRedBlue += dwSrc & 0x00ff00ff;
					dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
					lpSrcSurface -= 4;
				} while (--uColumn != 0);
			}
			if (uRightMargin < uRange - 1)
			{
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + (uRightMargin - 1) * 4);
				dwRedBlue += (dwSrc & 0x00ff00ff) * (uRange - uRightMargin - 1);
				dwAlphaGreen += ((dwSrc >> 8) & 0x00ff00ff) * (uRange - uRightMargin - 1);
			}
		}	
		UINT uRightMarginCount = uRightMargin - uRange - 1;	// �A�N�Z�X�\�ȉE���̃s�N�Z��
		UINT uColumn = uWidth;

		// �������̃s�N�Z�������Z
		DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
		dwRedBlue += dwSrc & 0x00ff00ff;
		dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
		
		do
		{	// �E���̃s�N�Z���� 1dot ���Z
			if (static_cast<int>(uRightMarginCount--) > 0)
			{	// �A�N�Z�X�\�Ȕ͈͓�
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + uRange * 4);
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			}
			else
			{	// �͈͊O�B�E�[�̃s�N�Z���ŕ�U
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + (uRightMarginCount + uRange + 1) * 4);
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			}

			// ���Z���ʂ����Z���ď����o��
			union
			{
				DWORD dw;
				struct
				{
					BYTE byBlue;
					BYTE byGreen;
					BYTE byRed;
					BYTE byAlpha;
				};
			} src;
			
			src.byBlue = static_cast<BYTE>((uMultiplier * (dwRedBlue & 0xffff) + 0x8000) >> 16);		// B
			src.byGreen = static_cast<BYTE>((uMultiplier * (dwAlphaGreen & 0xffff) + 0x8000) >> 16);	// G
			src.byRed = static_cast<BYTE>((uMultiplier * (dwRedBlue >> 16) + 0x8000) >> 16);			// R
			src.byAlpha = static_cast<BYTE>((uMultiplier * (dwAlphaGreen >> 16) + 0x8000) >> 16);		// A

			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
								  src.dw, uOpacity);

			lpDestSurface += 4;

			// ���Z���ʂ���A���[�̃s�N�Z���l�����Z
			if (uLeftMarginCount != 0)
			{	// �͈͊O
				uLeftMarginCount--;
				dwRedBlue -= dwLeftRedBlue;
				dwAlphaGreen -= dwLeftAlphaGreen;
			}
			else
			{	// �͈͓�
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface - uRange * 4);
				dwRedBlue -= (dwSrc & 0x00ff00ff);
				dwAlphaGreen -= (dwSrc >> 8) & 0x00ff00ff;
			}
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while(--uHeight != 0);
}

#pragma warning (pop)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxCustomDraw32::Blt_BlurHorzBlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//															  LONG lDestDistance, LONG lSrcDistance,
//															  UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
//															  UINT uRange, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���Q�Ɛ����ڂ����ʏ�u�����h�]�� 386
// ����: UINT uLeftMargin  ... �]�������ォ�獶(�̊O)���փA�N�Z�X�\�ȃh�b�g��
//		 UINT uRightMargin ... �]�������ォ��E���ւ̃A�N�Z�X�\�ȃh�b�g��(uWidth �Ɠ������A����ȏ�ɂȂ�)
//		 UINT uRange	   ... �ڂ����̋���(1 �` 127)
// ���l: ���Ƃ��� uRange = 63 �̏ꍇ�A����(63) + ���_(1) + �E��(63) = 127dot �̕��ς����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_BlurHorzBlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
														  UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
														  UINT uRange, const BYTE bySrcAlphaToOpacityTable[256])
{
	UINT uMultiplier = 65536 / (uRange * 2 + 1);
	do
	{
		DWORD dwLeftRedBlue;		// rrrrbbbb
		DWORD dwLeftAlphaGreen;		// aaaagggg
		DWORD dwRedBlue;			// rrrrbbbb
		DWORD dwAlphaGreen;			// aaaagggg

		dwRedBlue = 0x00000000;
		dwAlphaGreen = 0x00000000;

		UINT uLeftMarginCount;
		if (uLeftMargin != 0)
		{
			UINT uColumn = min(uLeftMargin, uRange);
			lpSrcSurface -= uColumn * 4;
			do
			{
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
				lpSrcSurface += 4;
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			} while (--uColumn != 0);
		}
		if (uLeftMargin >= uRange)
		{
			uLeftMarginCount = 0;
			dwLeftRedBlue = 0x00000000;
			dwLeftAlphaGreen = 0x00000000;
		}
		else
		{
			DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface - uLeftMargin * 4);
			uLeftMarginCount = uRange - uLeftMargin;
			dwLeftRedBlue = dwSrc & 0x00ff00ff;
			dwLeftAlphaGreen = (dwSrc >> 8) & 0x00ff00ff;
			dwRedBlue += dwLeftRedBlue * uLeftMarginCount;
			dwAlphaGreen += dwLeftAlphaGreen * uLeftMarginCount;
		}
		if (uRange >= 2)
		{
			if (uRightMargin != 0)
			{
				UINT uColumn = min(uRightMargin - 1, uRange - 1);
				lpSrcSurface += uColumn * 4;
				do
				{
					DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
					dwRedBlue += dwSrc & 0x00ff00ff;
					dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
					lpSrcSurface -= 4;
				} while (--uColumn != 0);
			}
			if (uRightMargin < uRange - 1)
			{
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + (uRightMargin - 1) * 4);
				dwRedBlue += (dwSrc & 0x00ff00ff) * (uRange - uRightMargin - 1);
				dwAlphaGreen += ((dwSrc >> 8) & 0x00ff00ff) * (uRange - uRightMargin - 1);
			}
		}	
		UINT uRightMarginCount = uRightMargin - uRange - 1;	// �A�N�Z�X�\�ȉE���̃s�N�Z��
		UINT uColumn = uWidth;

		// �������̃s�N�Z�������Z
		DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface);
		dwRedBlue += dwSrc & 0x00ff00ff;
		dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
		
		do
		{	// �E���̃s�N�Z���� 1dot ���Z
			if (static_cast<int>(uRightMarginCount--) > 0)
			{	// �A�N�Z�X�\�Ȕ͈͓�
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + uRange * 4);
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			}
			else
			{	// �͈͊O�B�E�[�̃s�N�Z���ŕ�U
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface + (uRightMarginCount + uRange + 1) * 4);
				dwRedBlue += dwSrc & 0x00ff00ff;
				dwAlphaGreen += (dwSrc >> 8) & 0x00ff00ff;
			}

			// ���Z���ʂ����Z���ď����o��
			struct
			{
				BYTE byBlue;
				BYTE byGreen;
				BYTE byRed;
				BYTE byAlpha;
			} src;
			
			src.byBlue = static_cast<BYTE>((uMultiplier * (dwRedBlue & 0xffff) + 0x8000) >> 16);		// B
			src.byGreen = static_cast<BYTE>((uMultiplier * (dwAlphaGreen & 0xffff) + 0x8000) >> 16);	// G
			src.byRed = static_cast<BYTE>((uMultiplier * (dwRedBlue >> 16) + 0x8000) >> 16);			// R
			src.byAlpha = static_cast<BYTE>((uMultiplier * (dwAlphaGreen >> 16) + 0x8000) >> 16);		// A


			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt_SrcAlpha(NxAlphaBlend::Normal, *reinterpret_cast<LPDWORD>(lpDestSurface),
										   *reinterpret_cast<const DWORD*>(&src.byBlue), bySrcAlphaToOpacityTable);

			lpDestSurface += 4;

			// ���Z���ʂ���A���[�̃s�N�Z���l�����Z
			if (uLeftMarginCount != 0)
			{	// �͈͊O
				uLeftMarginCount--;
				dwRedBlue -= dwLeftRedBlue;
				dwAlphaGreen -= dwLeftAlphaGreen;
			}
			else
			{	// �͈͓�
				DWORD dwSrc = *reinterpret_cast<const DWORD*>(lpSrcSurface - uRange * 4);
				dwRedBlue -= (dwSrc & 0x00ff00ff);
				dwAlphaGreen -= (dwSrc >> 8) & 0x00ff00ff;
			}
			lpSrcSurface += 4;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while(--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxCustomDraw32::Blt_BlurHorz_3DNow(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//											 LONG lDestDistance, LONG lSrcDistance,
//											 UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin, UINT uRange)
// �T�v: �����ڂ����]�� 3DNow!
// ����: UINT uLeftMargin  ... �]�������ォ�獶(�̊O)���փA�N�Z�X�\�ȃh�b�g��
//       UINT uRightMargin ... �]�������ォ��E���ւ̃A�N�Z�X�\�ȃh�b�g��(uWidth �Ɠ������A����ȏ�ɂȂ�)
//       UINT uRange       ... �ڂ����̋���(1 �` 127)
// ���l: ���Ƃ��� uRange = 63 �̏ꍇ�A����(63) + ���_(1) + �E��(63) = 127dot �̕��ς����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4799)
void __declspec(naked) CNxCustomDraw32::Blt_BlurHorz_3DNow(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
														   LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
														   UINT /*uWidth*/, UINT /*uHeight*/,
														   UINT /*uLeftMargin*/, UINT /*uRightMargin*/, UINT /*uRange*/)
{
	using namespace ConstTable;

#pragma pack(push, 4)
	struct StackFrame
	{
		LPVOID		   EDI;
		LPVOID		   ESI;
		LPVOID		   EBX;
		LPVOID		   EBP;
		UINT		   uLeftOverCount;
		UINT		   uRightMarginCount;
		LPVOID		   EIP;
		LPBYTE		   lpDestSurface;
		const BYTE*	   lpSrcSurface;
		LONG		   lDestDistance;
		LONG		   lSrcDistance;
		UINT		   uWidth;
		UINT		   uHeight;
		UINT		   uLeftMargin;
		UINT		   uRightMargin;
		UINT		   uRange;
	};
#pragma pack(pop)
	__asm
	{
		sub			esp, 8
		push		ebp
		push		ebx
		push		esi
		push		edi
		mov			eax, [esp]StackFrame.uRange
		mov			edx, [esp]StackFrame.lpSrcSurface
		lea			eax, [eax * 2 + 1]
		mov			edi, [esp]StackFrame.lpDestSurface
		movd		mm4, eax
		pxor		mm0, mm0
		punpckldq	mm4, mm4
		_emit 0x0f	; pi2fd mm4, mm4
		_emit 0x0f
		_emit 0xe4
		_emit 0x0d
		_emit 0x0f	; pfrcp mm4, mm4
		_emit 0x0f
		_emit 0xe4
		_emit 0x96

loop_y:
		mov			eax, [esp]StackFrame.uLeftMargin
		mov			ebx, [esp]StackFrame.uRange
		pxor		mm1, mm1
		test		eax, eax
		jz			skip_add_left
		sub			eax, ebx
		sbb			ecx, ecx
		and			ecx, eax
		add			eax, ebx
		add			ecx, ebx
		lea			esi, [ecx * 4]
		sub			edx, esi
loop_add_left:
		movd		mm7, [edx]
		add			edx, 4
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		loop		loop_add_left
skip_add_left:
		cmp			eax, ebx
		jnc			not_over_left
		sub			ebx, eax
		neg			eax
		movd		mm7, ebx
		movd		mm2, [edx + eax * 4]
		punpcklwd	mm7, mm7
		punpcklbw	mm2, mm0
		punpckldq	mm7, mm7
		mov			ecx, ebx
		pmullw		mm7, mm2
		paddw		mm1, mm7
not_over_left:
		mov			eax, [esp]StackFrame.uRange
		mov			ebx, [esp]StackFrame.uRightMargin
		cmp			eax, 2
		mov			[esp]StackFrame.uLeftOverCount, ecx
		jb			no_margin_right
		dec			eax
		cmp			ebx, 2
		jb			not_add_right
		dec			ebx
		sub			eax, ebx
		sbb			ecx, ecx
		and			ecx, eax
		add			eax, ebx
		add			ecx, ebx
		lea			edx, [edx + ecx * 4]
		inc			ebx
loop_add_right:
		movd		mm7, [edx]
		sub			edx, 4
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		loop		loop_add_right
not_add_right:
		inc			eax
		cmp			ebx, eax
		jnc			not_over_right
		sub			eax, ebx
		movd		mm7, [edx + ebx * 4 - 4]
		lea			ebp, [eax - 1]
		punpcklbw	mm7, mm0
		movd		mm6, ebp
		punpcklwd	mm6, mm6
		punpckldq	mm6, mm6
		add			eax, ebx
		pmullw		mm7, mm6
		paddw		mm1, mm7
not_over_right:
no_margin_right:
		sub			ebx, eax
		movd		mm7, [edx]
		dec			ebx
		mov			ecx, [esp]StackFrame.uWidth
		mov			esi, ebx
		mov			ebx, [esp]StackFrame.uRange
		punpcklbw	mm7, mm0
		mov			ebp, [esp]StackFrame.uLeftOverCount
		paddw		mm1, mm7
loop_x:
		dec			esi
		mov			eax, ebx
		jns			column_add_right
		lea			eax, [esi + ebx + 1]
column_add_right:
		movd		mm7, [edx + eax * 4]
		dec			ebp
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		movq		mm3, mm1
		movq		mm7, mm1
		punpcklwd	mm3, mm0
		punpckhwd	mm7, mm0
		_emit 0x0f	; pi2fd mm3, mm3
		_emit 0x0f
		_emit 0xdb	; 11dd dsss (1101 1011 = mm3, mm3)
		_emit 0x0d
		_emit 0x0f	; pi2fd mm7, mm7
		_emit 0x0f
		_emit 0xff	; 11dd dsss
		_emit 0x0d		
		_emit 0x0f	; pfmul mm3, mm4
		_emit 0x0f
		_emit 0xdc	; 11dd dsss (1101 1100 = mm3, mm4)
		_emit 0xb4
		_emit 0x0f	; pfmul mm7, mm4
		_emit 0x0f
		_emit 0xfc	; 11dd dsss (1111 1100 = mm7, mm4)
		_emit 0xb4
		_emit 0x0f	; pf2id mm3, mm3
		_emit 0x0f
		_emit 0xdb	; 11dd dsss (1101 1011 = mm3, mm3)
		_emit 0x1d
		_emit 0x0f	; pf2id mm7, mm7
		_emit 0x0f
		_emit 0xff	; 11dd dsss (1111 1111 = mm7, mm7)
		_emit 0x1d
		packssdw	mm3, mm7
		packuswb	mm3, mm0
		movd		[edi], mm3
		js			column_sub_left_not_over
		psubw		mm1, mm2
		jmp			short column_sub_left_end
column_sub_left_not_over:
		mov			eax, ebx
		neg			eax
		movd		mm7, [edx + eax * 4]
		punpcklbw	mm7, mm0
		psubw		mm1, mm7
column_sub_left_end:
		add			edx, 4
		add			edi, 4
		loop		loop_x
		add			edx, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y

		pop			edi
		pop			esi
		pop			ebx
		pop			ebp
		add			esp, 8
		_emit 0x0f	; femms
		_emit 0x0e
		ret
	}
}

#pragma warning (default: 4799)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxCustomDraw32::Blt_BlurHorzBlendNormal_3DNow(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//														LONG lDestDistance, LONG lSrcDistance,
//														UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
//														UINT uRange, UINT uOpacity)
// �T�v: �����ڂ����ʏ�u�����h�]�� 3DNow! 
// ����: UINT uLeftMargin  ... �]�������ォ�獶(�̊O)���փA�N�Z�X�\�ȃh�b�g��
//		 UINT uRightMargin ... �]�������ォ��E���ւ̃A�N�Z�X�\�ȃh�b�g��(uWidth �Ɠ������A����ȏ�ɂȂ�)
//		 UINT uRange	   ... �ڂ����̋���(1 �` 127)
// ���l: ���Ƃ��� uRange = 63 �̏ꍇ�A����(63) + ���_(1) + �E��(63) = 127dot �̕��ς����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4799)
void __declspec(naked) CNxCustomDraw32::Blt_BlurHorzBlendNormal_3DNow(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																	  LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																	  UINT /*uWidth*/, UINT /*uHeight*/,
																	  UINT /*uLeftMargin*/, UINT /*uRightMargin*/,
																	  UINT /*uRange*/, UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push, 4)
	struct StackFrame
	{
		LPVOID		   EDI;
		LPVOID		   ESI;
		LPVOID		   EBX;
		LPVOID		   EBP;
		UINT		   uLeftOverCount;
		UINT		   uRightMarginCount;
		LPVOID		   EIP;
		LPBYTE		   lpDestSurface;
		const BYTE*	   lpSrcSurface;
		LONG		   lDestDistance;
		LONG		   lSrcDistance;
		UINT		   uWidth;
		UINT		   uHeight;
		UINT		   uLeftMargin;
		UINT		   uRightMargin;
		UINT		   uRange;
		UINT		   uOpacity;
	};
#pragma pack(pop)
	__asm
	{
		sub			esp, 8
		push		ebp
		push		ebx
		push		esi
		push		edi
		mov			ebx, [esp]StackFrame.uRange
		mov			eax, [esp]StackFrame.uOpacity
		mov			edx, [esp]StackFrame.lpSrcSurface
		lea			ebx, [ebx * 2 + 1]
		mov			edi, [esp]StackFrame.lpDestSurface
		movd		mm5, ebx
		movq		mm6, [dwlMMXAlphaMultiplierTable + eax * 8]
		punpckldq	mm5, mm5
		pxor		mm0, mm0
		_emit 0x0f	; pi2fd mm5, mm5
		_emit 0x0f
		_emit 0xed
		_emit 0x0d
		_emit 0x0f	; pfrcp mm5, mm5
		_emit 0x0f
		_emit 0xed
		_emit 0x96

loop_y:
		mov			eax, [esp]StackFrame.uLeftMargin
		mov			ebx, [esp]StackFrame.uRange
		pxor		mm1, mm1
		test		eax, eax
		jz			skip_add_left
		sub			eax, ebx
		sbb			ecx, ecx
		and			ecx, eax
		add			eax, ebx
		add			ecx, ebx
		lea			esi, [ecx * 4]
		sub			edx, esi
loop_add_left:
		movd		mm7, [edx]
		add			edx, 4
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		loop		loop_add_left
skip_add_left:
		cmp			eax, ebx
		mov			ecx, 0
		jnc			not_over_left
		sub			ebx, eax
		neg			eax
		movd		mm7, ebx
		movd		mm2, [edx + eax * 4]
		punpcklwd	mm7, mm7
		punpcklbw	mm2, mm0
		punpckldq	mm7, mm7
		mov			ecx, ebx
		pmullw		mm7, mm2
		paddw		mm1, mm7
not_over_left:
		mov			eax, [esp]StackFrame.uRange
		mov			ebx, [esp]StackFrame.uRightMargin
		cmp			eax, 2
		mov			[esp]StackFrame.uLeftOverCount, ecx
		jb			no_margin_right
		dec			eax
		cmp			ebx, 2
		jb			not_add_right
		dec			ebx
		sub			eax, ebx
		sbb			ecx, ecx
		and			ecx, eax
		add			eax, ebx
		add			ecx, ebx
		lea			edx, [edx + ecx * 4]
		inc			ebx
loop_add_right:
		movd		mm7, [edx]
		sub			edx, 4
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		loop		loop_add_right
not_add_right:
		inc			eax
		cmp			ebx, eax
		jnc			not_over_right
		sub			eax, ebx
		movd		mm7, [edx + ebx * 4 - 4]
		lea			ebp, [eax - 1]
		punpcklbw	mm7, mm0
		movd		mm4, ebp
		punpcklwd	mm4, mm4
		punpckldq	mm4, mm4
		add			eax, ebx
		pmullw		mm7, mm4
		paddw		mm1, mm7
not_over_right:
no_margin_right:
		sub			ebx, eax
		movd		mm7, [edx]
		dec			ebx
		mov			ecx, [esp]StackFrame.uWidth
		punpcklbw	mm7, mm0
		mov			ebp, [esp]StackFrame.uLeftOverCount
		mov			esi, ebx
		mov			ebx, [esp]StackFrame.uRange
		paddw		mm1, mm7
loop_x:
		movd		mm4, [edi]
		dec			esi
		mov			eax, ebx
		jns			column_add_right
		lea			eax, [esi + ebx + 1]
column_add_right:
		movd		mm7, [edx + eax * 4]
		punpcklbw	mm7, mm0
		punpcklbw	mm4, mm0
		paddw		mm1, mm7
		movq		mm3, mm1
		movq		mm7, mm1
		punpcklwd	mm3, mm0
		punpckhwd	mm7, mm0
		_emit 0x0f	; pi2fd mm3, mm3
		_emit 0x0f
		_emit 0xdb	; 11dd dsss (1101 1011 = mm3, mm3)
		_emit 0x0d
		_emit 0x0f	; pi2fd mm7, mm7
		_emit 0x0f
		_emit 0xff	; 11dd dsss
		_emit 0x0d		
		_emit 0x0f	; pfmul mm3, mm5
		_emit 0x0f
		_emit 0xdd	; 11dd dsss (1101 1101 = mm3, mm5)
		_emit 0xb4
		_emit 0x0f	; pfmul mm7, mm5
		_emit 0x0f
		_emit 0xfd	; 11dd dsss (1111 1101 = mm7, mm5)
		_emit 0xb4
		_emit 0x0f	; pf2id mm3, mm3
		_emit 0x0f
		_emit 0xdb	; 11dd dsss (1101 1011 = mm3, mm3)
		_emit 0x1d
		_emit 0x0f	; pf2id mm7, mm7
		_emit 0x0f
		_emit 0xff	; 11dd dsss
		_emit 0x1d
		dec			ebp
		packssdw	mm3, mm7
		psubw		mm3, mm4
		pmullw		mm3, mm6
		psrlw		mm3, 8
		paddb		mm3, mm4
		packuswb	mm3, mm0
		movd		[edi], mm3
		js			column_sub_left_not_over
		psubw		mm1, mm2
		jmp			short column_sub_left_end
column_sub_left_not_over:
		mov			eax, ebx
		neg			eax
		movd		mm7, [edx + eax * 4]
		punpcklbw	mm7, mm0
		psubw		mm1, mm7
column_sub_left_end:
		add			edx, 4
		add			edi, 4
		loop		loop_x
		add			edx, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y
		pop			edi
		pop			esi
		pop			ebx
		pop			ebp
		add			esp, 8
		_emit 0x0f	; femms
		_emit 0x0e
		ret
	}
}

#pragma warning (default: 4799)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxCustomDraw32::Blt_BlurHorzBlendSrcAlphaNormal_3DNow(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																LONG lDestDistance, LONG lSrcDistance,
//																UINT uWidth, UINT uHeight, UINT uLeftMargin, UINT uRightMargin,
//																UINT uRange, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���Q�Ɛ����ڂ����ʏ�u�����h�]�� 3DNow! 
// ����: UINT uLeftMargin  ... �]�������ォ�獶(�̊O)���փA�N�Z�X�\�ȃh�b�g��
//		 UINT uRightMargin ... �]�������ォ��E���ւ̃A�N�Z�X�\�ȃh�b�g��(uWidth �Ɠ������A����ȏ�ɂȂ�)
//		 UINT uRange	   ... �ڂ����̋���(1 �` 127)
// ���l: ���Ƃ��� uRange = 63 �̏ꍇ�A����(63) + ���_(1) + �E��(63) = 127dot �̕��ς����
//		 �A���t�@����ʈ������Ă��Ȃ��̂ŁA���ʂ����������Ȃ�ꍇ������...
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4799)
void __declspec(naked) CNxCustomDraw32::Blt_BlurHorzBlendSrcAlphaNormal_3DNow(LPBYTE /*lpDestSurface*/, const BYTE* /*lpSrcSurface*/,
																			  LONG /*lDestDistance*/, LONG /*lSrcDistance*/,
																			  UINT /*uWidth*/, UINT /*uHeight*/,
																			  UINT /*uLeftMargin*/, UINT /*uRightMargin*/,
																			  UINT /*uRange*/, const BYTE /*bySrcAlphaToOpacityTable*/[256])
{
	using namespace ConstTable;

#pragma pack(push, 4)
	struct StackFrame
	{
		LPVOID		   EDI;
		LPVOID		   ESI;
		LPVOID		   EBX;
		LPVOID		   EBP;
		UINT		   uLeftOverCount;
		UINT		   uRightMarginCount;
		LPVOID		   EIP;
		LPBYTE		   lpDestSurface;
		const BYTE*	   lpSrcSurface;
		LONG		   lDestDistance;
		LONG		   lSrcDistance;
		UINT		   uWidth;
		UINT		   uHeight;
		UINT		   uLeftMargin;
		UINT		   uRightMargin;
		UINT		   uRange;
		const BYTE*    pbSrcAlphaToOpacityTable;		// const BYTE bySrcAlphaToOpacityTable[256]
	};
#pragma pack(pop)
	__asm
	{
		sub			esp, 8
		push		ebp
		push		ebx
		push		esi
		push		edi
		mov			eax, [esp]StackFrame.uRange
		mov			edx, [esp]StackFrame.lpSrcSurface
		lea			eax, [eax * 2 + 1]
		mov			edi, [esp]StackFrame.lpDestSurface
		movd		mm5, eax
		pxor		mm0, mm0
		punpckldq	mm5, mm5
		_emit 0x0f	; pi2fd mm5, mm5
		_emit 0x0f
		_emit 0xed
		_emit 0x0d
		_emit 0x0f	; pfrcp mm5, mm5
		_emit 0x0f
		_emit 0xed
		_emit 0x96
		; mm5 = 1.0 / uRange

loop_y:
		mov			eax, [esp]StackFrame.uLeftMargin
		mov			ebx, [esp]StackFrame.uRange
		pxor		mm1, mm1
		test		eax, eax
		jz			skip_add_left
		sub			eax, ebx
		sbb			ecx, ecx
		and			ecx, eax
		add			eax, ebx
		add			ecx, ebx
		lea			esi, [ecx * 4]
		sub			edx, esi
loop_add_left:
		movd		mm7, [edx]
		add			edx, 4
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		loop		loop_add_left
skip_add_left:
		cmp			eax, ebx
		mov			ecx, 0
		jnc			not_over_left
		sub			ebx, eax
		neg			eax
		movd		mm2, [edx + eax * 4]
		movd		mm7, ebx
		punpcklbw	mm2, mm0
		punpcklwd	mm7, mm7
		mov			ecx, ebx
		punpckldq	mm7, mm7
		pmullw		mm7, mm2
		paddw		mm1, mm7
not_over_left:
		mov			eax, [esp]StackFrame.uRange
		mov			ebx, [esp]StackFrame.uRightMargin
		cmp			eax, 2
		mov			[esp]StackFrame.uLeftOverCount, ecx
		jb			no_margin_right
		dec			eax
		cmp			ebx, 2
		jb			not_add_right
		dec			ebx
		sub			eax, ebx
		sbb			ecx, ecx
		and			ecx, eax
		add			eax, ebx
		add			ecx, ebx
		lea			edx, [edx + ecx * 4]
		inc			ebx
loop_add_right:
		movd		mm7, [edx]
		sub			edx, 4
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		loop		loop_add_right

not_add_right:
		inc			eax
		cmp			ebx, eax
		jnc			not_over_right
		sub			eax, ebx
		movd		mm7, [edx + ebx * 4 - 4]
		lea			ebp, [eax - 1]
		punpcklbw	mm7, mm0
		movd		mm6, ebp
		punpcklwd	mm6, mm6
		punpckldq	mm6, mm6
		pmullw		mm7, mm6
		add			eax, ebx
		paddw		mm1, mm7
not_over_right:
no_margin_right:
		sub			ebx, eax
		movd		mm7, [edx]
		dec			ebx
		mov			ecx, [esp]StackFrame.uWidth
		mov			esi, ebx
		mov			ebx, [esp]StackFrame.uRange
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
loop_x:
		dec			esi
		mov			eax, ebx
		movd		mm4, [edi]
		jns			column_add_right
		lea			eax, [esi + ebx + 1]
column_add_right:
		movd		mm7, [edx + eax * 4]
		dec			ebp
		punpcklbw	mm7, mm0
		paddw		mm1, mm7
		movq		mm3, mm1
		movq		mm7, mm1
		punpcklwd	mm3, mm0
		punpckhwd	mm7, mm0
		_emit 0x0f	; pi2fd mm3, mm3
		_emit 0x0f
		_emit 0xdb	; 11dd dsss (1101 1011 = mm3, mm3)
		_emit 0x0d
		_emit 0x0f	; pi2fd mm7, mm7
		_emit 0x0f
		_emit 0xff	; 11dd dsss
		_emit 0x0d		
		_emit 0x0f	; pfmul mm3, mm5
		_emit 0x0f
		_emit 0xdd	; 11dd dsss (1101 1101 = mm3, mm5)
		_emit 0xb4
		_emit 0x0f	; pfmul mm7, mm5
		_emit 0x0f
		_emit 0xfd	; 11dd dsss (1111 1101 = mm7, mm5)
		_emit 0xb4
		mov			ebp, [esp]StackFrame.pbSrcAlphaToOpacityTable
		_emit 0x0f	; pf2id mm3, mm3
		_emit 0x0f
		_emit 0xdb	; 11dd dsss (1101 1011 = mm3, mm3)
		_emit 0x1d
		_emit 0x0f	; pf2id mm7, mm7
		_emit 0x0f
		_emit 0xff	; 11dd dsss
		_emit 0x1d
		packssdw	mm3, mm7
		movq		mm6, mm3
		punpcklbw	mm4, mm0
		psrlq		mm6, 48
		movd		eax, mm6
		movzx		eax, byte ptr [ebp + eax]
		psubw		mm3, mm4
		pmullw		mm3, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uLeftOverCount
		psrlw		mm3, 8
		paddb		mm3, mm4
		dec			eax
		packuswb	mm3, mm0
		mov			[esp]StackFrame.uLeftOverCount, eax
		movd		[edi], mm3
		js			column_sub_left_not_over
		psubw		mm1, mm2
		jmp			short column_sub_left_end
column_sub_left_not_over:
		mov			eax, ebx
		neg			eax
		movd		mm7, [edx + eax * 4]
		punpcklbw	mm7, mm0
		psubw		mm1, mm7
column_sub_left_end:
		add			edx, 4
		add			edi, 4
		dec			ecx
		jnz			loop_x
		add			edx, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			[esp]StackFrame.uHeight
		jnz			loop_y
		
		pop			edi
		pop			esi
		pop			ebx
		pop			ebp
		add			esp, 8
		_emit 0x0f	; femms
		_emit 0x0e
		ret
	}
}
#pragma warning (default: 4799)


///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_BlurHorz(CNxSurface* pDestSurface, const RECT* lpDestRect,
//									   const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//									   const NxBlt* pNxBlt) const
// �T�v: �����ڂ��� Blt
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... ������ʓ��̎w��Ɏg�p���� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �]����A���t�@�͔j�󂳂��
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_BlurHorz(CNxSurface* pDestSurface, const RECT* lpDestRect,
								   const CNxSurface *pSrcSurface, const RECT* lpSrcRect, const NxBlt *pNxBlt) const
{
	if (pNxBlt->uBlurRange == 0)
	{
		if (pNxBlt->dwFlags & NxBlt::srcAlpha)
			return Blt_BlendSrcAlpha(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
		else
			return Blt_Blend(pDestSurface, lpDestRect, pSrcSurface, lpSrcRect, pNxBlt);
	}

	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Blur() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	if (IsStretch(lpDestRect, lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Blur() : �g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	if (pSrcSurface->GetBitCount() != 32)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Blur() : �]�����T�[�t�F�X�� 32bpp �ł͂���܂���.\n");
		return FALSE;
	}

	if ((pNxBlt->dwFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_Blur() : �ʏ�u�����h�ȊO�ɂ͑Ή����Ă��܂���.\n");
		return FALSE;
	}
	
	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	LONG lSrcPitch = pSrcSurface->GetPitch();
	
	// �N���b�v
	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;
			
	UINT uLeftMargin = rcSrc.left - rcSrcClip.left;
	UINT uRightMargin = rcSrcClip.right - rcSrc.left - 1;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	LONG lSrcDistance = lSrcPitch - uWidth * 4;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left * 4;

	if (pNxBlt->dwFlags & NxBlt::srcAlpha)
	{	// �]�����A���t�@�t��
		(CNxDraw::GetInstance()->Is3DNowEnabled() ? Blt_BlurHorzBlendSrcAlphaNormal_3DNow : Blt_BlurHorzBlendSrcAlphaNormal_386)
			(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, uLeftMargin, uRightMargin,
			 pNxBlt->uBlurRange, ConstTable::bySrcAlphaToOpacityTable[uOpacity]);
	}
	else if (uOpacity != 255 || pNxBlt->dwFlags & NxBlt::constDestAlpha)
	{
		(CNxDraw::GetInstance()->Is3DNowEnabled() ? Blt_BlurHorzBlendNormal_3DNow : Blt_BlurHorzBlendNormal_386)
			(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight, uLeftMargin, uRightMargin,
			 pNxBlt->uBlurRange, uOpacity);
	}
	else
	{
		(CNxDraw::GetInstance()->Is3DNowEnabled() ? Blt_BlurHorz_3DNow : Blt_BlurHorz_386)
			(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance, uWidth, uHeight,
			 uLeftMargin, uRightMargin, pNxBlt->uBlurRange);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxCustomDraw32::Blt_ColorFill_BlurHorzBlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface,
//																		LONG lDestDistance, LONG lSrcDistance,
//																		UINT uWidth, UINT uHeight, DWORD dwColor,
//																		UINT uLeftMargin, UINT uRightMargin,
//																		UINT uRange, const BYTE bySrcAlphaToOpacityTable[256])
// �T�v: �]�����A���t�@�`�����l���Q�Ɛ����ڂ����ʏ�u�����h�h��Ԃ� 386
// ����: UINT uLeftMargin  ... �]�������ォ�獶(�̊O)���փA�N�Z�X�\�ȃh�b�g��
//		 UINT uRightMargin ... �]�������ォ��E���ւ̃A�N�Z�X�\�ȃh�b�g��(uWidth �Ɠ������A����ȏ�ɂȂ�)
//		 UINT uRange	   ... �ڂ����̋���(1 �` 127)
// ���l: ���Ƃ��� uRange = 63 �̏ꍇ�A����(63) + ���_(1) + �E��(63) = 127dot �̕��ς����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Blt_ColorFill_BlurHorzBlendSrcAlphaNormal_386(LPBYTE lpDestSurface, const BYTE* lpSrcSurface, LONG lDestDistance, LONG lSrcDistance,
																	UINT uWidth, UINT uHeight, DWORD dwColor,
																	UINT uLeftMargin, UINT uRightMargin,
																	UINT uRange, const BYTE bySrcAlphaToOpacityTable[256])
{
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
		{
			uLeftMarginCount = 0;
		}
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

			*reinterpret_cast<LPDWORD>(lpDestSurface) = NxAlphaBlend::ColorFill_SrcAlpha(NxAlphaBlend::Normal,
															*reinterpret_cast<const DWORD*>(lpDestSurface),
															 static_cast<BYTE>((uMultiplier * uSum + 0x8000) >> 16), dwColor, bySrcAlphaToOpacityTable);

			lpDestSurface += 4;

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
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxCustomDraw32::Blt_ColorFill_BlurHorzBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													  const NxBlt* pNxBlt) const
// �T�v: �����ڂ����u�����h�h��Ԃ�
// ����: CNxSurface* pDestSurface	   ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 	   ... �]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const NxBlt* pNxBlt		   ... ������ʓ��̎w��Ɏg�p���� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Blt_ColorFill_BlurHorzBlend(CNxSurface* pDestSurface, const RECT* lpDestRect,
												  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
												  const NxBlt* pNxBlt) const
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

	if (IsStretch(lpDestRect, lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "�g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}

	if (!(pNxBlt->dwFlags & NxBlt::srcAlpha))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_Blur() : NxBlt::srcAlpha ���K�v�ł�.\n");
		return FALSE;
	}
	
	if ((pNxBlt->dwFlags & NxBlt::blendTypeMask) != NxBlt::blendNormal)
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_Blur() : �ʏ�u�����h�ȊO�ɂ͑Ή����Ă��܂���.\n");
		return FALSE;
	}

	// �s�����x(uOpacity) �̎擾
	UINT uOpacity;
	if (!GetValidOpacity(pNxBlt, &uOpacity))
	{
		_RPTF0(_CRT_ASSERT, "CNxCustomDraw32::Blt_ColorFill_Blur() : �s�����x�̒l���ُ�ł�.");
		return FALSE;
	}
	if (uOpacity == 0)
		return TRUE;

	RECT rcSrc = *lpSrcRect;
	RECT rcDest = *lpDestRect;
	RECT rcSrcClip;
	pSrcSurface->GetRect(&rcSrcClip);
	LONG lSrcPitch = pSrcSurface->GetPitch();
	
	// �N���b�v
	if (!pDestSurface->ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;
			
	UINT uLeftMargin = rcSrc.left - rcSrcClip.left;
	UINT uRightMargin = rcSrcClip.right - rcSrc.left - 1;

	// ���ƍ���
	UINT uWidth = rcDest.right - rcDest.left;
	UINT uHeight = rcDest.bottom - rcDest.top;

	// �]����T�[�t�F�X�������ւ̃|�C���^�Ƌ������擾
	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + pDestSurface->GetPitch() * rcDest.top + rcDest.left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;

	LONG lSrcDistance = lSrcPitch - uWidth;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lSrcPitch * rcSrc.top + rcSrc.left;

	Blt_ColorFill_BlurHorzBlendSrcAlphaNormal_386(lpDestSurface, lpSrcSurface, lDestDistance, lSrcDistance,
												  uWidth, uHeight, pNxBlt->nxbColor,
												  uLeftMargin, uRightMargin, pNxBlt->uBlurRange,
												  ConstTable::bySrcAlphaToOpacityTable[uOpacity]);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Filter_Grayscale_386(LPBYTE lpDestSurface, LONG lDestDistance,
//													  const BYTE* lpSrcSurface, LONG lSrcDistance, 
//													  UINT uWidth, UINT uHeight)
// �T�v: �O���C�X�P�[�����t�B���^ (386 ��)
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Filter_Grayscale_386(LPBYTE lpDestSurface, LONG lDestDistance,
										   const BYTE* lpSrcSurface, LONG lSrcDistance,
										   UINT uWidth, UINT uHeight, UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			CNxColor color(*reinterpret_cast<const DWORD*>(lpSrcSurface));

			// R = G = B = grayscale �ɂ���(alpha �͑ΏۊO�Ȃ̂Ŗ���)
			BYTE byGrayscale = color.GetGrayscale();
			color.SetRed(byGrayscale);
			color.SetGreen(byGrayscale);
			color.SetBlue(byGrayscale);

			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<const DWORD*>(lpSrcSurface), color, uOpacity);
			lpSrcSurface += 4;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Filter_Grayscale_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//													  const BYTE* lpSrcSurface, LONG lSrcDistance, 
//													  UINT uWidth, UINT uHeight)
// �T�v: �O���C�X�P�[�����t�B���^ (MMX ��)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Filter_Grayscale_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
															 const BYTE* /*lpSrcSurface*/, LONG /*lSrcDistance*/,
															 UINT /*uWidth*/, UINT /*uHeight*/, UINT /*uOpacity*/)
{
	using namespace ConstTable;

	// Y = (0.298912*R + 0.586611*G + 0.114477*B)
	//    ->  Y = (77*R + 150*G + 29*B) / 256

	static const DWORDLONG dwlGrayMultiplier = (static_cast<DWORDLONG>(77) << 32) | (static_cast<DWORDLONG>(150) << 16) | 29;

#pragma pack(push, 4)
	struct StackFrame
	{
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpSrcSurface;
		LONG		lSrcDistance;
		const BYTE* lpDestDistance;
		LONG		lDestDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		esi
		push		edi
		mov			eax, [esp]StackFrame.uOpacity
		mov			esi, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestDistance
		mov			edx, [esp]StackFrame.uHeight
		pxor		mm0, mm0
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		movq		mm6, [dwlGrayMultiplier]
loop_y:

		mov			ecx, [esp]StackFrame.uWidth
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:
	
//		movq		mm2, [esi + 0]
		_emit 0x0f
		_emit 0x6f
		_emit 0x56
		_emit 0x00
		add			esi, 8
		movq		mm3, mm2
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		movq		mm4, mm2
		movq		mm5, mm3
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		add			edi, 8
		movq		mm0, mm2
		movq		mm7, mm3
		psrlq		mm2, 16
		psrlq		mm3, 16
		paddw		mm0, mm2
		paddw		mm7, mm3
		psrlq		mm2, 16
		psrlq		mm3, 16
		paddw		mm2, mm0
		paddw		mm3, mm7
		psrlq		mm2, 8
		psrlq		mm3, 8
		pxor		mm0, mm0
		punpcklbw	mm2, mm2
		punpcklbw	mm3, mm3
		punpcklwd	mm2, mm2
		punpcklwd	mm3, mm3
		punpcklbw	mm2, mm0
		punpcklbw	mm3, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		dec			ecx
		movq		[edi - 8], mm2
		jnz			loop_x_qword
skip_x_qword:
		mov			eax, [esp]StackFrame.uWidth
		test		eax, 1
		jz			skip_x

//		movd		mm2, [esi + 0]
		_emit 0x0f
		_emit 0x6e
		_emit 0x56
		_emit 0x00
		add			esi, 4
		punpcklbw	mm2, mm0
		movq		mm4, mm2
		pmullw		mm2, mm6
		add			edi, 4
		movq		mm7, mm2
		psrlq		mm2, 16
		paddw		mm7, mm2
		psrlq		mm2, 16
		paddw		mm2, mm7
		psrlq		mm2, 8
		punpcklbw	mm2, mm2
		punpcklwd	mm2, mm2
		punpcklbw	mm2, mm0
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi - 4], mm2

skip_x:
		add			esi, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			edx
		jnz			loop_y

		pop			edi
		pop			esi
		emms
		ret
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw32::Filter_Grayscale(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												   const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												   const NxFilterBlt* pNxFilterBlt) const
// �T�v: �O���C�X�P�[�����t�B���^
// ����: CNxSurface* pDestSurface		 ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 		 ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface	 ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect			 ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//		 const NxFilterBlt* pNxFilterBlt ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Filter_Grayscale(CNxSurface* pDestSurface, const RECT* lpDestRect,
									   const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
									   const NxFilterBlt* pNxFilterBlt) const
{
	// ���ƍ���
	UINT uWidth = lpDestRect->right - lpDestRect->left;
	UINT uHeight = lpDestRect->bottom - lpDestRect->top;

	UINT uOpacity = (pNxFilterBlt->dwFlags & NxFilterBlt::opacity) ? pNxFilterBlt->uOpacity : 255;

	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + lpDestRect->top * pDestSurface->GetPitch() + lpDestRect->left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left * 4;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;

	(CNxDraw::GetInstance()->IsMMXEnabled() ? Filter_Grayscale_MMX : Filter_Grayscale_386)
		(lpDestSurface, lDestDistance, lpSrcSurface, lSrcDistance, uWidth, uHeight, uOpacity);
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Filter_RGBColorBalance_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//															const BYTE* lpSrcSurface, LONG lSrcDistance,
//															UINT uWidth, UINT uHeight, UINT uOpacity,
//															DWORDLONG dwlMultiplier, DWORDLONG dwlAdder)
// �T�v: RGB �J���[�o�����X�t�B���^ (MMX)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Filter_RGBColorBalance_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
																   const BYTE* /*lpSrcSurface*/, LONG /*lSrcDistance*/,
																   UINT /*uWidth*/, UINT /*uHeight*/, UINT /*uOpacity*/,
																   DWORDLONG /*dwlMultiplier*/, DWORDLONG /*dwlAdder*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		const BYTE* lpSrcSurface;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
//		DWORDLONG	dwlMultiplier;
		DWORD		dwMultiplier[2];
//		DWORDLONG	dwlAdder;
		DWORD		dwAdder[2];
	};
#pragma pack(pop)

	__asm
	{
		push		edi
		push		esi
		mov			eax, [esp]StackFrame.uOpacity
		mov			esi, [esp]StackFrame.lpSrcSurface
		mov			edi, [esp]StackFrame.lpDestSurface
		pxor		mm0, mm0
		mov			edx, [esp]StackFrame.uHeight
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		movq		mm6, [esp]StackFrame.dwMultiplier
		movq		mm7, [esp]StackFrame.dwAdder

loop_y:

		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

		align		8

loop_x_qword:

//		movq		mm2, [esi + 0]
		_emit 0x0f
		_emit 0x6f
		_emit 0x56
		_emit 0x00
		add			esi, 8
		movq		mm3, mm2
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		movq		mm4, mm2
		movq		mm5, mm3
		pmullw		mm2, mm6
		pmullw		mm3, mm6
		add			edi, 8
		psrlw		mm2, 7
		psrlw		mm3, 7
		paddsw		mm2, mm7
		paddsw		mm3, mm7
		packuswb	mm2, mm0
		packuswb	mm3, mm0
		punpcklbw	mm2, mm0
		punpcklbw	mm3, mm0
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		dec			ecx
		movq		[edi - 8], mm2
		jnz			loop_x_qword
		
		test		eax, 1
		jz			loop_x_end

skip_x_qword:

//		movd		mm2, [esi + 0]
		_emit 0x0f
		_emit 0x6e
		_emit 0x56
		_emit 0x00
		add			esi, 4
		punpcklbw	mm2, mm0
		movq		mm4, mm2
		pmullw		mm2, mm6
		add			edi, 4
		psrlw		mm2, 7
		paddsw		mm2, mm7
		packuswb	mm2, mm0
		punpcklbw	mm2, mm0
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi - 4], mm2

loop_x_end:

		add			esi, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			edx
		jnz			loop_y

		pop			esi
		pop			edi
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Filter_RGBColorBalance_386(LPBYTE lpDestSurface, LONG lDestDistance,
//															const BYTE* lpSrcSurface, LONG lSrcDistance,
//															UINT uWidth, UINT uHeight, UINT uOpacity,
//															DWORDLONG dwlMultiplier, DWORDLONG dwlAdder)
// �T�v: RGB �J���[�o�����X�t�B���^ (386)
//		 DWORDLONG dwlMultipler	   ... ��Z�l(aa rr gg bb)
//		 DWORDLONG dwlAdder 	   ... ���Z�l(aa rr gg bb)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Filter_RGBColorBalance_386(LPBYTE lpDestSurface, LONG lDestDistance,
												 const BYTE* lpSrcSurface, LONG lSrcDistance,
												 UINT uWidth, UINT uHeight, UINT uOpacity,
												 DWORDLONG dwlMultiplier, DWORDLONG dwlAdder)
{
	UINT uMulRed = static_cast<UINT>(dwlMultiplier >> 32);
	UINT uMulGreen = static_cast<UINT>((dwlMultiplier >> 16) & 0xffff);
	UINT uMulBlue = static_cast<UINT>(dwlMultiplier & 0xffff);
	int nAddRed = static_cast<short>(dwlAdder >> 32);
	int nAddGreen = static_cast<short>((dwlAdder >> 16) & 0xfff);
	int nAddBlue = static_cast<short>(dwlAdder & 0xffff);

	do
	{
		UINT uColumn = uWidth;
		do
		{
			CNxColor color(*reinterpret_cast<const DWORD*>(lpSrcSurface));
			color.SetRed(ConstTable::byRangeLimitTable[((static_cast<UINT>(color.GetRed()) * uMulRed) >> 7)
														+ nAddRed + ConstTable::RangeLimitCenter]);
			color.SetGreen(ConstTable::byRangeLimitTable[((static_cast<UINT>(color.GetGreen()) * uMulGreen) >> 7)
														  + nAddGreen + ConstTable::RangeLimitCenter]);
			color.SetBlue(ConstTable::byRangeLimitTable[((static_cast<UINT>(color.GetBlue()) * uMulBlue) >> 7)
														 + nAddBlue + ConstTable::RangeLimitCenter]);
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<const DWORD*>(lpSrcSurface), color, uOpacity);
			lpSrcSurface += 4;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw32::Filter_RGBColorBalance(CNxSurface* pDestSurface, const RECT* lpDestRect,
//														 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//														 const NxFilterBlt* pNxFilterBlt) const
// �T�v: �O���C�X�P�[�����t�B���^
// ����: CNxSurface* pDestSurface ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect        ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Filter_RGBColorBalance(CNxSurface* pDestSurface, const RECT* lpDestRect,
											 const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const
{
	// ���ƍ���
	UINT uWidth = lpDestRect->right - lpDestRect->left;
	UINT uHeight = lpDestRect->bottom - lpDestRect->top;

	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + lpDestRect->top * pDestSurface->GetPitch() + lpDestRect->left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left * 4;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;

	UINT uOpacity = (pNxFilterBlt->dwFlags & NxFilterBlt::opacity) ? pNxFilterBlt->uOpacity : 255;

	DWORDLONG dwlMultiplier = *reinterpret_cast<const DWORDLONG*>(&pNxFilterBlt->nxfRGBColorBalance.Multiplier.wBlue) & 0x0000ffffffffffff;
	DWORDLONG dwlAdder = *reinterpret_cast<const DWORDLONG*>(&pNxFilterBlt->nxfRGBColorBalance.Adder.sBlue) & 0x0000ffffffffffff;

	if (CNxDraw::GetInstance()->IsMMXEnabled())
		Filter_RGBColorBalance_MMX(lpDestSurface, lDestDistance, lpSrcSurface, lSrcDistance, uWidth, uHeight, uOpacity,
								   dwlMultiplier, dwlAdder);
	else
		Filter_RGBColorBalance_386(lpDestSurface, lDestDistance, lpSrcSurface, lSrcDistance, uWidth, uHeight, uOpacity,
								   dwlMultiplier, dwlAdder);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw32::Filter_HueTransform(CNxSurface* pDestSurface, const RECT* lpDestRect,
//													  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//													  const NxFilterBlt* pNxFilterBlt) const
// �T�v: �F���ϊ��t�B���^
// ����: CNxSurface* pDestSurface			 ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect				 ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface		 ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect				 ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//		 const NxFilterBlt* pNxFilterBlt	 ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Filter_HueTransform(CNxSurface* pDestSurface, const RECT* lpDestRect,
										  const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt) const
{
	// ���ƍ���
	UINT uWidth = lpDestRect->right - lpDestRect->left;
	UINT uHeight = lpDestRect->bottom - lpDestRect->top;

	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + lpDestRect->top * pDestSurface->GetPitch() + lpDestRect->left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left * 4;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;

	UINT uOpacity = (pNxFilterBlt->dwFlags & NxFilterBlt::opacity) ? pNxFilterBlt->uOpacity : 255;
	
	int nHueAdder = pNxFilterBlt->nxfHueTransform.nHue;
	if (nHueAdder < 0)
		nHueAdder += 360;

	do
	{
		UINT uColumn = uWidth;
		do
		{
			// RGB �� HLS �֕ϊ�
			CNxHLSColor hls(*reinterpret_cast<const DWORD*>(lpSrcSurface));

			int nHue = hls.GetHue();
			int nSaturation = hls.GetSaturation();
			int nLightness = hls.GetLightness();

			// �p�����[�^�ɏ]���ĕϊ�
			if (pNxFilterBlt->nxfHueTransform.bSingleness)
				nHue = nHueAdder;
			else
			{
				nHue += nHueAdder;
				if (nHue >= 360)
					nHue -= 360;
			}

			nSaturation = ConstTable::byRangeLimitTable[nSaturation + pNxFilterBlt->nxfHueTransform.nSaturation
														 + ConstTable::RangeLimitCenter];

			nLightness = ConstTable::byRangeLimitTable[nLightness + pNxFilterBlt->nxfHueTransform.nLightness
														+ ConstTable::RangeLimitCenter];

			// HLS �� RGB �֕ϊ����ď����߂�
			hls.SetHLS(static_cast<WORD>(nHue), static_cast<BYTE>(nLightness), static_cast<BYTE>(nSaturation));
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<const DWORD*>(lpSrcSurface),
								  hls.GetNxColor(), uOpacity);
			lpSrcSurface += 4;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Filter_Negative_MMX(LPBYTE lpDestSurface, LONG lDestDistance,
//													 const BYTE* lpSrcSurface, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight, UINT uOpacity)
// �T�v: �l�K���]�t�B���^ (MMX)
// ����: const RECT* lpDestRect 		 ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface	 ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect			 ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//		 const NxFilterBlt* pNxFilterBlt ... NxFilterBlt �\���̂ւ̃|�C���^
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __declspec(naked) CNxCustomDraw32::Filter_Negative_MMX(LPBYTE /*lpDestSurface*/, LONG /*lDestDistance*/,
															const BYTE* /*lpSrcSurface*/, LONG /*lSrcDistance*/,
															UINT /*uWidth*/, UINT /*uHeight*/, UINT /*uOpacity*/)
{
	using namespace ConstTable;

#pragma pack(push ,4)
	struct StackFrame
	{
		LPVOID		ESI;
		LPVOID		EDI;
		LPVOID		EIP;
		LPBYTE		lpDestSurface;
		LONG		lDestDistance;
		const BYTE* lpSrcSurface;
		LONG		lSrcDistance;
		UINT		uWidth;
		UINT		uHeight;
		UINT		uOpacity;
	};
#pragma pack(pop)

	__asm
	{
		push		esi
		push		edi
		mov			eax, [esp]StackFrame.uOpacity
		mov			edi, [esp]StackFrame.lpDestSurface
		mov			esi, [esp]StackFrame.lpSrcSurface
		pxor		mm0, mm0
		movq		mm1, [dwlMMXAlphaMultiplierTable + eax * 8]
		mov			eax, [esp]StackFrame.uWidth
		mov			edx, [esp]StackFrame.uHeight
		movq		mm6, [dwlConst_0000_00FF_00FF_00FF]

loop_y:
		mov			ecx, eax
		shr			ecx, 1
		jz			skip_x_qword

loop_x_qword:
//		movq		mm2, [esi + 0]
		_emit 0x0f			; avoid vector decode
		_emit 0x6f
		_emit 0x56
		_emit 0x00
		add			esi, 8
		movq		mm3, mm2
		add			edi, 8
		punpcklbw	mm2, mm0
		punpckhbw	mm3, mm0
		movq		mm4, mm2
		movq		mm5, mm3
		pxor		mm2, mm6
		pxor		mm3, mm6
		psubw		mm2, mm4
		psubw		mm3, mm5
		pmullw		mm2, mm1
		pmullw		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		paddb		mm2, mm4
		paddb		mm3, mm5
		packuswb	mm2, mm3
		dec			ecx
		movq		[edi - 8], mm2
		jnz			loop_x_qword

skip_x_qword:
		test		eax, 1
		jz			skip_x

//		movd		mm2, [esi + 0]
		_emit 0x0f			; avoid vector decode
		_emit 0x6e
		_emit 0x56
		_emit 0x00
		add			esi, 4
		punpcklbw	mm2, mm0
		add			edi, 4
		movq		mm4, mm2
		pxor		mm2, mm6
		psubw		mm2, mm4
		pmullw		mm2, mm1
		psrlw		mm2, 8
		paddb		mm2, mm4
		packuswb	mm2, mm0
		movd		[edi - 4], mm2
skip_x:
		add			esi, [esp]StackFrame.lSrcDistance
		add			edi, [esp]StackFrame.lDestDistance
		dec			edx
		jnz			loop_y
		pop			edi
		pop			esi
		emms
		ret
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxCustomDraw32::Filter_Negative_386(LPBYTE lpDestSurface, LONG lDestDistance,
//													 const BYTE* lpSrcSurface, LONG lSrcDistance,
//													 UINT uWidth, UINT uHeight, UINT uOpacity)
// �T�v: �l�K���]�t�B���^ (386)
// ����: const RECT* lpDestRect 		 ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface	 ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect			 ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//		 const NxFilterBlt* pNxFilterBlt ... NxFilterBlt �\���̂ւ̃|�C���^
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxCustomDraw32::Filter_Negative_386(LPBYTE lpDestSurface, LONG lDestDistance,
										  const BYTE* lpSrcSurface, LONG lSrcDistance,
										  UINT uWidth, UINT uHeight, UINT uOpacity)
{
	do
	{
		UINT uColumn = uWidth;
		do
		{
			DWORD dwSrc = ~*reinterpret_cast<const DWORD*>(lpSrcSurface);
			*reinterpret_cast<LPDWORD>(lpDestSurface) =
				NxAlphaBlend::Blt(NxAlphaBlend::Normal, *reinterpret_cast<const DWORD*>(lpSrcSurface), dwSrc, uOpacity);
			lpSrcSurface += 4;
			lpDestSurface += 4;
		} while (--uColumn != 0);
		lpSrcSurface += lSrcDistance;
		lpDestSurface += lDestDistance;
	} while (--uHeight != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxCustomDraw32::Filter_Negative(CNxSurface* pDestSurface, const RECT* lpDestRect,
//												  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
//												  const NxFilterBlt* pNxFilterBlt) const
// �T�v: �l�K���]�t�B���^
// ����: CNxSurface* pDestSurface			 ... �]����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpDestRect 			 ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//		 const CNxSurface* pSrcSurface		 ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//		 const RECT* lpSrcRect				 ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//		 const NxFilterBlt* pNxFilterBlt	 ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
// ���l: �S�Ẵ|�C���^�� NULL �s�B��`�̓N���b�v�ς�
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxCustomDraw32::Filter_Negative(CNxSurface* pDestSurface, const RECT* lpDestRect,
									  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
									  const NxFilterBlt* pNxFilterBlt) const
{
	// ���ƍ���
	UINT uWidth = lpDestRect->right - lpDestRect->left;
	UINT uHeight = lpDestRect->bottom - lpDestRect->top;

	LPBYTE lpDestSurface = static_cast<LPBYTE>(pDestSurface->GetBits()) + lpDestRect->top * pDestSurface->GetPitch() + lpDestRect->left * 4;
	LONG lDestDistance = pDestSurface->GetPitch() - uWidth * 4;
	const BYTE* lpSrcSurface = static_cast<const BYTE*>(pSrcSurface->GetBits()) + lpSrcRect->top * pSrcSurface->GetPitch() + lpSrcRect->left * 4;
	LONG lSrcDistance = pSrcSurface->GetPitch() - uWidth * 4;

	UINT uOpacity = (pNxFilterBlt->dwFlags & NxFilterBlt::opacity) ? pNxFilterBlt->uOpacity : 255;

	(CNxDraw::GetInstance()->IsMMXEnabled() ? Filter_Negative_MMX : Filter_Negative_386)
		(lpDestSurface, lDestDistance, lpSrcSurface, lSrcDistance, uWidth, uHeight, uOpacity);
	return TRUE;
}
