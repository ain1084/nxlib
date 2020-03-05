#pragma once

namespace NxDrawLocal
{
	// ���s���e�[�u���쐬
	void CreateTableDynamic();

	// �ėp�N���b�v�֐�(��`������� TRUE ��Ԃ�)
	BOOL ClipRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpDestClipRect, const RECT* lpSrcClipRect);

	namespace ConstTable
	{
		const DWORDLONG dwlConst_8081_8081_8081_8081 = 0x8081808180818081;
		const DWORDLONG dwlConst_00FF_00FF_00FF_00FF = 0x00ff00ff00ff00ff;
		const DWORDLONG dwlConst_FF00_0000_FF00_0000 = 0xff000000ff000000;
		const DWORDLONG dwlConst_00FF_FFFF_00FF_FFFF = 0x00ffffff00ffffff;
		const DWORDLONG dwlConst_0000_00FF_00FF_00FF = 0x000000ff00ff00ff;

		enum { RangeLimitCenter = 768 };	// byRangeLimitTable �̍ŏ�/�ő�l�̗]�T

		extern BYTE byRangeLimitTable[RangeLimitCenter * 2 + 256];
		extern DWORD dwByteToDwordTable[256];
		extern BYTE bySrcAlphaToOpacityTable[256][256];
		extern BYTE byDestAndSrcOpacityTable[256][256];
		extern BYTE byDestAlphaResultTable[256][256];
		extern DWORDLONG dwlMMXAlphaMultiplierTable[256];
		extern DWORDLONG dwlMMXNoRegardAlphaMultiplierTable[256];
	}	// namespace ConstTable
}
