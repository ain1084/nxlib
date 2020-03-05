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
	// ���� BYTE ��4���ׂ� DWORD �ϊ��e�[�u��
	for (v = 0x00000000, u = 0; u < 256; u++, v += 0x01010101)
		dwByteToDwordTable[u] = v;

	// byRangeLimitTable[RangeLimitCenter * 2 + 256]
	// �O�a���Z�p�e�[�u��(���Z�A���Z�u�����h���Ŏg�p)
	for (u = 0; u < RangeLimitCenter; u++)
		byRangeLimitTable[u] = 0;
	for (u = 0; u < 256; u++)
		byRangeLimitTable[u + RangeLimitCenter] = static_cast<BYTE>(u);
	for (u = 0; u < RangeLimitCenter; u++)
		byRangeLimitTable[u + RangeLimitCenter + 256] = 255;

	// dwlMMXAlphaMultiplierTable[256]
	// MMX �p�A���t�@�l��Z�l�e�[�u�� Alpha = 0
	// �ő�� 256 �{�Ƃ���� index = 1 ���� + 1 ���Ă���
	dwlMMXAlphaMultiplierTable[0] = 0;
	for (dwl = 0x0000000200020002, u = 1; u < 256; u++, dwl += 0x0000000100010001)
		dwlMMXAlphaMultiplierTable[u] = dwl;

	// dwlMMXNoRegardAlphaMultiplierTable[256]
	// MMX �p�A���t�@�l��Z�l�e�[�u�� Alpha = 0
	// dwlMMXAlphaMultiplierTable �Ǝ��Ă��邪�Aindex = 2 �ȉ��̓[��
	for (u = 0; u < 3; u++)
		dwlMMXNoRegardAlphaMultiplierTable[u] = 0;
	for (dwl = 0x0000000400040004, u = 3; u < 256; u++, dwl += 0x0000000100010001)
		dwlMMXNoRegardAlphaMultiplierTable[u] = dwl;
		
	// bySrcAlphaToOpacityTable[256][256]
	// �A���t�@�ϊ��e�[�u��
	// �]�����̃A���t�@�l(0 �` 255)�ƕs�����x(0 �` 255)����A
	// �ŏI�I�� Alpha (0 �` 255) �𓾂�
	// �܂��A8bit * 8bit / 255 �̉��Z���ʂƂ��Ă��g�p(�������A�؂�グ�L��)
	for (u = 0; u < 256; u++)
	{
		for (v = 0; v < 256; v++)
		{
			bySrcAlphaToOpacityTable[u][v] = static_cast<BYTE>((u * v + 127) / 255);
		}
	}

	// byDestAndSrcOpacityTable[256][256]
	// �]�����Ɠ]����̕s�����x����^�̓����x�����߂�ׂ̃e�[�u��
	// (�]����A���t�@���l�������d�ˍ��킹�Ɏg�p)
	//
	// �g���g�� source ver.0.80 by W.Dee �� (http://www.din.or.jp/~glit/TheOddStage/TVP/)
	// Projects\TVP32\OperatorsUnit.cpp ���Q�l�ɂ��܂���
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
	// �]����A���t�@���l�������u�����h���ɁA�]����A���t�@�`�����l����u��������l
	// [dest][src] �ŎQ��
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
// �T�v: �ėp��`�N���b�v�֐�
// ����: LPRECT lpDestRect ... �]�����`������ RECT �\���̂ւ̃|�C���^
//       LPRECT lpSrcRect  ... �]������`������ RECT �\���̂ւ̃|�C���^
//		 const RECT* lpDestClipRect ... �]����̃N���b�v��`(NULL = �N���b�v���Ȃ�)
//       const RECT* lpSrcClipRect  ... �]�����̃N���b�v��`(NULL = �N���b�v���Ȃ�)
// �ߒl: lpDestRect �Ŏ�������`����łȂ���� TRUE, ����ȊO�� FALSE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL ClipRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpDestClipRect, const RECT* lpSrcClipRect)
{
	// ���������֍��킹��
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