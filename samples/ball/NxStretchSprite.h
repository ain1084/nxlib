// NxStretchSprite.h: CNxStretchSprite �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Aingouchi
//
// �T�v: �g��k���X�v���C�g
//       CNxSurfaceSprite �̔h���N���X�ł��B
//       SetSrcRect() �����o�֐��Őݒ肳�ꂽ�]������`����A
//       (CNxSurfaceSprite �N���X�����o��) SetRect() �֐��Ŏw�肳�ꂽ��`��
//       �g�喔�͏k���������Ȃ��Ȃ���]�����܂��B
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXSTRETCHSPRITE_H__CF2E4702_1D9E_11D4_8815_0000E842F190__INCLUDED_)
#define AFX_NXSTRETCHSPRITE_H__CF2E4702_1D9E_11D4_8815_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxSurfaceSprite.h>

class CNxStretchSprite : public CNxSurfaceSprite  
{
public:
	CNxStretchSprite(CNxSprite* pParent, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxStretchSprite();
	virtual void SetSrcRect(const RECT* lpSrcRect);
	void GetSrcRect(LPRECT lpSrcRect) const;

	// CNxSurfaceSprite override
	virtual CNxSurface* SetSrcSurface(CNxSurface* pSurface, const RECT* lpRect = NULL);

protected:
	// CNxSprite override
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;

private:
	RECT m_rcSrc;		// �]������`
};

inline void CNxStretchSprite::GetSrcRect(LPRECT lpSrcRect) const {
	*lpSrcRect = m_rcSrc; }

#endif // !defined(AFX_NXSTRETCHSPRITE_H__CF2E4702_1D9E_11D4_8815_0000E842F190__INCLUDED_)
