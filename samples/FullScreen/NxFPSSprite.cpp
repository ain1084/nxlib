// NxFPSSprite.cpp: CNxFPSSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �Ȉ� FPS �\���X�v���C�g
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxFPSSprite.h"
#include <NxDraw/NxScreen.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxFPSSprite::CNxFPSSprite(CNxScreen* pParent) : CNxLayerSprite(pParent, 8)
{
	Create(140, 24);		// �T�C�Y�K���ɃT�[�t�F�X���쐬
	SetZPos(INT_MAX);		// �ŏ��

	m_font = CNxFont(_T("�l�r �o�S�V�b�N"), -24);
	SetFont(&m_font);

	SetTextSmoothing(TRUE);
	
	// 8bpp �T�[�t�F�X�֕`�悵�����ʂ���ʂ֓]������ׂ� NxBlt ��ݒ�
	NxBlt nxb;
	nxb.dwFlags = NxBlt::colorFill | NxBlt::srcAlpha;	// �]�����A���t�@�t���h��ׂ�
	nxb.nxbColor = CNxColor::white;						// �h��ׂ��F
	SetNxBlt(&nxb);
}

CNxFPSSprite::~CNxFPSSprite()
{
}

// �X�v���C�g�̍X�V��Ԃ����ׂ��钼�O�� CNxSprite �N���X��������Ăяo����鉼�z�֐�
void CNxFPSSprite::PreUpdate()
{
	CNxScreen* pScreen = static_cast<CNxScreen*>(GetParent());
	int nFPS = pScreen->GetFPS();
	if (nFPS == -1)
		return;		// FPS �ω�����

	// �ȑO�̓��e���N���A
	FillRect(NULL, 0);

	// FPS ��`��
	CString str;
	if (nFPS < 10000 * 1000)
		str.Format(_T("%3.2f FPS"), static_cast<float>(nFPS) / 1000);
	else
		str.Format(_T("%5d FPS"), nFPS / 1000);		// 10000 FPS �ȏ�Ȃ�Ώ����_�͕\�����Ȃ�
	DrawText(0, 0, NULL, str, static_cast<const NxBlt*>(NULL));
	
	// �X�v���C�g�X�V
	SetUpdate();
}
