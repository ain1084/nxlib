// NxRasterSprite.cpp: CNxRasterSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: ラスタースクロールスプライト
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxRasterSprite.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxRasterSprite::CNxRasterSprite(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pParent, pSurface, lpRect)
{
	m_nAngle = 0;			// 角度
	m_nMaxAmplitude = 1;	// 最大振幅
	m_nStep  = 1;			// 角度の増分
	MakeSinTable();
}

CNxRasterSprite::~CNxRasterSprite()
{
}

/////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxRasterSprite::MakeSinTable()
// 概要: m_nMaxAmplitude の値を最大振幅とする正弦波テーブルを生成
// 引数: なし
// 戻値: なし
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::MakeSinTable()
{
	// 65536 倍した正弦波テーブル 0 - 90度
	static const UINT uSinX65536[] =
	{
			 0,  1143,  2287,  3429,  4571,  5711,  6850,  7986,  9120,
		 10252, 11380, 12504, 13625, 14742, 15854, 16961, 18064, 19160,
		 20251, 21336, 22414, 23486, 24550, 25606, 26655, 27696, 28729,
		 29752, 30767, 31772, 32768, 33753, 34728, 35693, 36647, 37589,
		 38521, 39440, 40347, 41243, 42125, 42995, 43852, 44695, 45525,
		 46340, 47142, 47929, 48702, 49460, 50203, 50931, 51643, 52339,
		 53019, 53683, 54331, 54963, 55577, 56175, 56755, 57319, 57864,
		 58393, 58903, 59395, 59870, 60326, 60763, 61183, 61583, 61965,
		 62328, 62672, 62997, 63302, 63589, 63856, 64103, 64331, 64540,
		 64729, 64898, 65047, 65176, 65286, 65376, 65446, 65496, 65526,
		 65536
	};

	for (int i = 0; i < 91; i++)
		m_nSinTable[i] = (int)((uSinX65536[i] * m_nMaxAmplitude + 32767) / 65536);
}

/////////////////////////////////////////////////////////////////////////
// public:
//	void CNxRasterSprite::SetAngle(int nAngle)
// 概要: 開始角度を設定
// 引数: int nAngle ... 角度 (0 - 359)
// 戻値: なし
/////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::SetAngle(int nAngle)
{
	if (m_nAngle != nAngle % 360)
	{
		m_nAngle = nAngle % 360;
		SetUpdate();
	}
}

////////////////////////////////////////////////////////////////////////
// public:
//	void CNxRasterSprite::Rotate(int nVal)
// 概要: 角度を相対変化
// 引数: int val ... 変移
// 戻値: なし
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::Rotate(int nVal)
{
	if (nVal != 0)
		SetAngle(nVal + m_nAngle);
}

////////////////////////////////////////////////////////////////////////
// public:
//	void CNxRasterSrptie::RotateStep()
// 概要: 角度を SetStep() で指定された増分だけ進める
// 引数: なし
// 戻値: なし
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::RotateStep()
{
	SetAngle(m_nAngle + m_nStep);
}

////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::SetStep(int nStep)
// 概要: 角度の増分を設定
// 引数: int nStep ... 変移
// 戻値: なし
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::SetStep(int nStep)
{
	m_nStep = nStep;
	SetUpdate();
}

///////////////////////////////////////////////////////////////////////
// public:
//	void CNxRasterSprite::SetMaxAmplitude(int nAmp)
// 概要: 最大振幅を設定
// 引数: int nAmp ... 最大振幅
// 戻値: なし
//////////////////////////////////////////////////////////////////////

void CNxRasterSprite::SetMaxAmplitude(int nAmp)
{
	m_nMaxAmplitude = nAmp;
	MakeSinTable();
	SetUpdate();
}

//////////////////////////////////////////////////////////////////////
// protected:
//	int CNxRasterSprite::GetSin(int nAngle) const
// 概要: 正弦波の値を取得
// 引数: int nAngle ... 角度
// 戻値: m_nMaxAmplitude を最大振幅とする正弦波の値
//////////////////////////////////////////////////////////////////////

int inline CNxRasterSprite::GetSin(int nAngle) const
{
	nAngle %= 360;
	if (nAngle < 91)
	{	// 0 - 90
		return m_nSinTable[nAngle];
	} else if (nAngle < 181)
	{	// 91 - 180
		return m_nSinTable[90 - (nAngle - 90)];
	} else if (nAngle < 271)
	{	// 181 - 270
		return -m_nSinTable[nAngle - 180];
	} else
	{	 // 271 - 359
		return -m_nSinTable[90 - (nAngle - 270)];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxRasterSprite::Draw(CNxSurface* pSurface, const RECT* lpRect) const
// 概要: スプライトパターン描画 (CNxSprite::Draw() のオーバーライド)
// 戻値: なし
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxRasterSprite::Draw(CNxSurface* pSurface, const RECT* lpRect) const
{
	RECT rect;
	GetRect(&rect);

	NxBlt nxb;
	GetNxBlt(&nxb);
	
	// 途中から書換える時の補正
	int angle = m_nAngle + (lpRect->top - rect.top) * m_nStep;

	for (rect.top = lpRect->top; rect.top < lpRect->bottom; rect.top++)
	{
		rect.bottom = rect.top + 1;
		pSurface->Blt(GetSin(angle) + rect.left, rect.top, GetSrcSurface(), &rect, &nxb);
		angle += m_nStep;
	}
	return TRUE;
}

void CNxRasterSprite::PreUpdate()
{
	RotateStep();
}
