// NxMAGImageLoader.cpp: CNxMAGImageLoader クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: MAG 画像の読み込みを行う、CNxDIBImageLoader 派生クラス
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <vector>
#include "NxDIBImage.h"
#include "NxMAGImageLoader.h"

//////////////////////////////////////////////////////////////////////
// public:
//	CNxMAGImageLoader::CNxMAGImageLoader()
// 概要: CNxMAGImageLoader クラスのデフォルトコンストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxMAGImageLoader::CNxMAGImageLoader()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxMAGImageLoader::~CNxMAGImageLoader()
// 概要: CNxMAGImageLoader クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxMAGImageLoader::~CNxMAGImageLoader()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxMAGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// 概要: 展開可能なデータ形式であるかを調べる
// 引数: LPCVOID lpvBuf ... データの最初から 2048byte を読み込んだバッファへのポインタ
//       LONG lLength   ... データ全体のサイズ
// 戻値: 展開可能であれば TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxMAGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
{
	static const char cMaki02[] =
	{
		'M', 'A', 'K', 'I', '0', '2', ' ', ' '
	};
	if (lLength < sizeof(cMaki02))
		return FALSE;

	return memcmp(lpvBuf, cMaki02, sizeof(cMaki02)) == 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxMAGImageLoader::CreateDIBImage(CNxFile& nxfile)
// 概要: 画像を展開して CNxDIBImage オブジェクトを返す
// 引数: CNxFile& nxfile ... CNxFile オブジェクトへの参照
// 戻値: 成功ならば、作成した CNxDIBImage オブジェクトへのポインタ。失敗ならば NULL
///////////////////////////////////////////////////////////////////////////////////////

#pragma warning (push)
#pragma warning (disable : 4731)

CNxDIBImage* CNxMAGImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	struct MagHeader
	{
		BYTE	byHeadTop;			// zero
		BYTE	byMachine;
		BYTE	byMachineNote;
		BYTE	byScreenMode;
		WORD	wStartX;
		WORD	wStartY;
		WORD	wEndX;
		WORD	wEndY;
		LONG	lOffsetFlagA;
		LONG	lOffsetFlagB;
		LONG	lSizeFlagB;
		LONG	lOffsetPixel;
		LONG	lSizePixel;
	};

	typedef const BYTE* LPCBYTE;
	const BYTE MAGHD_SCRMODE_COLOR256 = 0x80;
	const POINT ptPixelCopyTable[16] =
	{
		{  0,   0 },		// 0 (dummy)
		{ -2,   0 },		// 1
		{ -4,   0 },		// 2
		{ -8,   0 },		// 3
		{  0,  -1 },		// 4
		{ -2,  -1 },		// 5
		{  0,  -2 },		// 6
		{ -2,  -2 },		// 7
		{ -4,  -2 },		// 8
		{  0,  -4 },		// 9
		{ -2,  -4 },		// 10
		{ -4,  -4 },		// 11
		{  0,  -8 },		// 12
		{ -2,  -8 },		// 13
		{ -4,  -8 },		// 14
		{  0, -16 },		// 15
	};

	void *pvbm;
	int i, cLines, cLineBytes, cPaletteEntries, cLineDots, cFlagPerLine;
	LONG lDistance;
	LONG lSize;

	lSize = nxfile.GetSize();
	std::vector<BYTE> vectorData(lSize);
	LPBYTE pbData = &vectorData[0];
	nxfile.Read((char*)pbData, lSize);
	
	pbData = (LPBYTE)memchr(pbData, '\0', lSize);				// コメントを skip
	MagHeader* lpMagHeader = (MagHeader*)pbData;				// ヘッダ
	cLines = lpMagHeader->wEndY - lpMagHeader->wStartY + 1;		// ライン数
	cLineDots = lpMagHeader->wEndX - lpMagHeader->wStartX + 1;	// 横方向ドット数

/* 1999/9/30 削除
	if (lpptStartResult != NULL)
	{
		lpptStartResult->x = lpMagHeader->wStartX;
		lpptStartResult->y = lpMagHeader->wStartY;
	}
*/
	if (lpMagHeader->byScreenMode & MAGHD_SCRMODE_COLOR256)
	{	///////////////////////// 256 color
		// パレットは 256 entries
		// 1byte で 1dot を示す
		// 4dot 単位
		cPaletteEntries = 256;
		cLineDots = cLineDots + 3 & ~0x3;
		cLineBytes = cLineDots;
	}
	else
	{	///////////////////////// 16color
		// パレットは 16 entries
		// 1byte で 2dot を示す
		// 8dot 単位
		cPaletteEntries = 16;
		cLineDots = cLineDots + 7 & ~0x7;
		cLineBytes = cLineDots >> 1;
	}
	lDistance = cLineBytes;
	cFlagPerLine = cLineBytes >> 2;			// line 当たりのフラグサイズ

	std::auto_ptr<CNxDIBImage> pDIBImage(new CNxDIBImage);
	if (!pDIBImage->Create(cLineDots, cLines, (cPaletteEntries == 256) ? 8 : 4))
	{
		return NULL;
	}

	// パレット初期化
	RGBQUAD *rgbq = pDIBImage->GetColorTable();
	LPCBYTE pbPalette = (LPCBYTE)(lpMagHeader + 1);
	for (i = 0; i < cPaletteEntries; i++)
	{
		rgbq->rgbGreen = *pbPalette++;
		rgbq->rgbRed = *pbPalette++;
		rgbq->rgbBlue = *pbPalette++;
		rgbq->rgbReserved = 0;
		rgbq++;
	}

	// Pixel コピー時の座標テーブルを初期化
	LONG lPixelCopyFrom[16];
	for (i = 0; i < 16; i++)
		lPixelCopyFrom[i] = ptPixelCopyTable[i].x + ptPixelCopyTable[i].y * pDIBImage->GetPitch();

	// 展開開始
	LPCBYTE pbFlagA = (LPCBYTE)(lpMagHeader->lOffsetFlagA + (LPCBYTE)lpMagHeader);
	LPCBYTE pbFlagB = (LPCBYTE)(lpMagHeader->lOffsetFlagB + (LPCBYTE)lpMagHeader);
	LPCBYTE pbPixel = (LPCBYTE)(lpMagHeader->lOffsetPixel + (LPCBYTE)lpMagHeader);
	LPBYTE pbFlagBuf = (LPBYTE)_alloca(cFlagPerLine);
	memset(pbFlagBuf, 0, cFlagPerLine);
	BYTE cFlagABitCount = 1;
	BYTE byFlagABitData;
	pvbm = static_cast<LPBYTE>(pDIBImage->GetBits());
	lDistance = pDIBImage->GetPitch() - lDistance;

	_asm
	{
		mov		ebx, pbPixel
		mov		ecx,cLines
		mov		edi, pvbm

yloop:
		mov		esi, pbFlagA
		push	ecx
		mov		dl, byFlagABitData
		push	edi
		mov		edi, pbFlagBuf
		mov		dh, cFlagABitCount
		push	ebx
		mov		ebx, pbFlagB
		mov		ecx, cFlagPerLine

loop_l:

		dec		dh
		jnz		short skip_read_flagA
		mov		dl, [esi]
		inc		esi
		mov		dh, 8

skip_read_flagA:

		shl		dl, 1
		jnc		short skip_get_flagB
		mov		al, [ebx]
		inc		ebx
		xor		[edi], al

skip_get_flagB:

		inc		edi
		loop	loop_l
		mov		pbFlagA, esi
		mov		pbFlagB, ebx
		mov		byFlagABitData, dl
		mov		cFlagABitCount, dh
	// pixel copy
		pop		ebx
		mov		edx, pbFlagBuf
		pop		edi
		mov		ecx, cFlagPerLine
		push	ebp
		xor		eax, eax
		lea		ebp, lPixelCopyFrom
		// high

copy_pixel_loop:

		movzx	eax, byte ptr [edx]
		mov		esi, edi
		and		eax, 0f0h
		jnz		short copy_pixel_from_flag_high
		mov		esi,ebx
		add		ebx,2

copy_pixel_from_flag_high:

		shr		eax, 2
		add		esi, [ebp + eax]
		mov		ax, [esi]
		mov		[edi], ax
		add		esi, 2
		add		edi, 2
		movzx	eax, byte ptr [edx]
		mov		esi, edi
		and		eax, 0fh
		jnz		short copy_pixel_from_flag_low
		mov		esi,ebx
		add		ebx,2

copy_pixel_from_flag_low:

		add		esi, [ebp + eax * 4]
		mov		ax, [esi]
		inc		edx
		mov		[edi], ax
		add		esi, 2
		add		edi, 2
		dec		ecx
		jnz		copy_pixel_loop
		pop		ebp
		pop		ecx
		add		edi, lDistance
		dec		ecx
		jnz		yloop
	}
	return pDIBImage.release();
}

#pragma warning (pop)