// NxMouseSprite.h: CNxMouseSprite �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: CNxSprite �Ń}�E�X�J�[�\������������N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxSurfaceSprite.h"
#include "NxWindow.h"

class CNxMouseSprite : public CNxSurfaceSprite  
{
public:
	CNxMouseSprite(CNxWindow* pScreen, CNxSurface* pSurface = NULL, const RECT* lpRect = NULL);
	virtual ~CNxMouseSprite();

	int Show(BOOL bShow);
	virtual BOOL SetPos(int x, int y);

protected:
	virtual void PreUpdate();

private:
	CNxWindow* getWindow() const;

private:
	int  m_nShowCount;

	CNxMouseSprite(const CNxMouseSprite&);
	CNxMouseSprite& operator=(const CNxMouseSprite&);
};

inline CNxWindow* CNxMouseSprite::getWindow() const {
	return static_cast<CNxWindow*>(GetParent()); }