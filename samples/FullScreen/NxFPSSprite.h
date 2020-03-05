// NxFPSSprite.h: CNxFPSSprite �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �Ȉ� FPS �\���X�v���C�g
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXFPSSPRITE_H__8F9AE1C2_5594_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_NXFPSSPRITE_H__8F9AE1C2_5594_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxLayerSprite.h>
#include <NxDraw/NxFont.h>
class CNxScreen;

class CNxFPSSprite : public CNxLayerSprite  
{
public:
	CNxFPSSprite(CNxScreen* pParent);
	virtual ~CNxFPSSprite();

protected:
	virtual void PreUpdate();

private:
	CNxFont m_font;
};

#endif // !defined(AFX_NXFPSSPRITE_H__8F9AE1C2_5594_11D4_AAAB_0090CCA661BD__INCLUDED_)
