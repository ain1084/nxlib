// NxRasterSprite.h: CNxRasterSprite �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: ���X�^�[�X�N���[������X�v���C�g�BCNxSurfaceSprite ����h��
//       (�N���b�s���O�e�X�g�p)
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXRASTERSPRITE_H__9A9B1CF2_34D6_11D2_AE5E_0000E842F190__INCLUDED_)
#define AFX_NXRASTERSPRITE_H__9A9B1CF2_34D6_11D2_AE5E_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxSurfaceSprite.h>

class CNxRasterSprite : public CNxSurfaceSprite
{
public:
	CNxRasterSprite(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxRasterSprite();

	void SetAngle(int nAngle);
	void Rotate(int nVal);
	void RotateStep();
	void SetStep(int step);
	void SetMaxAmplitude(int nAmp);
	int  GetMaxAmplitude() const;
	int  GetStep() const;
	int  GetAngle() const;

protected:

	int inline GetSin(int nAngle) const;
	void MakeSinTable();
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;
	virtual void PreUpdate();

private:

	int m_nAngle;
	int m_nStep;
	int m_nMaxAmplitude;
	int m_nSinTable[90 + 1];
};

inline int CNxRasterSprite::GetMaxAmplitude() const {
	return m_nMaxAmplitude; }

inline int CNxRasterSprite::GetStep() const {
	return m_nStep; }

inline int CNxRasterSprite::GetAngle() const {
	return m_nAngle; }

#endif // !defined(AFX_NXRASTERSPRITE_H__9A9B1CF2_34D6_11D2_AE5E_0000E842F190__INCLUDED_)
