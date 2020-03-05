// NxRasterSprite.cpp: CNxRasterSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: ���X�^�[�X�N���[���X�v���C�g
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxRasterSprite.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxRasterSprite::CNxRasterSprite(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxSurfaceSprite(pParent, pSurface, lpRect)
{
	m_nAngle = 0;			// �p�x
	m_nMaxAmplitude = 1;	// �ő�U��
	m_nStep  = 1;			// �p�x�̑���
	MakeSinTable();
}

CNxRasterSprite::~CNxRasterSprite()
{
}

/////////////////////////////////////////////////////////////////////////
// protected:
//	void CNxRasterSprite::MakeSinTable()
// �T�v: m_nMaxAmplitude �̒l���ő�U���Ƃ��鐳���g�e�[�u���𐶐�
// ����: �Ȃ�
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::MakeSinTable()
{
	// 65536 �{���������g�e�[�u�� 0 - 90�x
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
// �T�v: �J�n�p�x��ݒ�
// ����: int nAngle ... �p�x (0 - 359)
// �ߒl: �Ȃ�
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
// �T�v: �p�x�𑊑Εω�
// ����: int val ... �ψ�
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::Rotate(int nVal)
{
	if (nVal != 0)
		SetAngle(nVal + m_nAngle);
}

////////////////////////////////////////////////////////////////////////
// public:
//	void CNxRasterSrptie::RotateStep()
// �T�v: �p�x�� SetStep() �Ŏw�肳�ꂽ���������i�߂�
// ����: �Ȃ�
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::RotateStep()
{
	SetAngle(m_nAngle + m_nStep);
}

////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSprite::SetStep(int nStep)
// �T�v: �p�x�̑�����ݒ�
// ����: int nStep ... �ψ�
// �ߒl: �Ȃ�
////////////////////////////////////////////////////////////////////////

void CNxRasterSprite::SetStep(int nStep)
{
	m_nStep = nStep;
	SetUpdate();
}

///////////////////////////////////////////////////////////////////////
// public:
//	void CNxRasterSprite::SetMaxAmplitude(int nAmp)
// �T�v: �ő�U����ݒ�
// ����: int nAmp ... �ő�U��
// �ߒl: �Ȃ�
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
// �T�v: �����g�̒l���擾
// ����: int nAngle ... �p�x
// �ߒl: m_nMaxAmplitude ���ő�U���Ƃ��鐳���g�̒l
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
// �T�v: �X�v���C�g�p�^�[���`�� (CNxSprite::Draw() �̃I�[�o�[���C�h)
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxRasterSprite::Draw(CNxSurface* pSurface, const RECT* lpRect) const
{
	RECT rect;
	GetRect(&rect);

	NxBlt nxb;
	GetNxBlt(&nxb);
	
	// �r�����珑�����鎞�̕␳
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
