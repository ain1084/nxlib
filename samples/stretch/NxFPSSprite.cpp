// NxFPSSprite.cpp: CNxFPSSprite �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �Ȉ� FPS �\���X�v���C�g
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <NxDraw/NxWindow.h>
#include <NxDraw/NxHLSColor.h>
#include "NxFPSSprite.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxFPSSprite::CNxFPSSprite(CNxWindow* pParent) : CNxLayerSprite(pParent, 32)
{
	Create(120, 20);		// �T�C�Y�K���ɃT�[�t�F�X���쐬
	SetZPos(INT_MAX);		// �ŏ��

	m_font = CNxFont(_T("�l�r �o�S�V�b�N"), -20);
	SetFont(&m_font);

	// �e�L�X�g�̃X���[�W���O���s��
	SetTextSmoothing(TRUE);

	// ���̃T�[�t�F�X����ʂ֓]������ׂ� NxBlt ��ݒ�
	NxBlt nxb;
	nxb.dwFlags = NxBlt::srcAlpha;
	SetNxBlt(&nxb);
}

CNxFPSSprite::~CNxFPSSprite()
{

}

// �X�v���C�g�̍X�V��Ԃ����ׂ��钼�O�� CNxSprite �N���X��������Ăяo����鉼�z�֐�
void CNxFPSSprite::PreUpdate()
{
	CNxWindow* pWindow = static_cast<CNxWindow*>(GetParent());
	int nFPS = pWindow->GetFPS();
	if (nFPS == -1)
		return;		// FPS �ω�����

	// �ȑO�̓��e���N���A
	FillRect(NULL, 0);

	// FPS ��`��
	TCHAR szBuf[32];
	if (nFPS < 10000 * 1000)
		_stprintf(szBuf, _T("%3.2f FPS"), static_cast<float>(nFPS) / 1000);
	else
		_stprintf(szBuf, _T("%5d FPS"), nFPS / 1000);

	DrawText(2, 2, NULL, szBuf, CNxColor(0, 0, 0, 128));		// �e
	DrawText(0, 0, NULL, szBuf, CNxHLSColor(35, 200, 250));

	// �X�v���C�g�X�V
	SetUpdate();
}
