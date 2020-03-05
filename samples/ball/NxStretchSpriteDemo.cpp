// NxStretchSpriteDemo.cpp: CNxStretchSpriteDemo �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ball.h"
#include "NxStretchSpriteDemo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxStretchSpriteDemo::CNxStretchSpriteDemo(CNxSprite* pParent, CNxSurface* pSurface, const RECT* lpRect)
 : CNxStretchSprite(pParent, pSurface, lpRect)
{
	GetSrcRect(&m_rcSrcBegin);
	GetSrcRect(&m_rcSrcEnd);
}

CNxStretchSpriteDemo::~CNxStretchSpriteDemo()
{

}

void CNxStretchSpriteDemo::SetSrcBeginRect(const RECT* lpRect)
{
	m_rcSrcBegin = *lpRect;
	SetSrcRect(&m_rcSrcBegin);
}

void CNxStretchSpriteDemo::SetSrcEndRect(const RECT* lpRect)
{
	m_rcSrcEnd = *lpRect;
}

void CNxStretchSpriteDemo::PreUpdate()
{
	// ���Ȃ�蔲���ł��B4:3 �̃T�C�Y�ȊO�ł͎~�܂�Ȃ��Ȃ�܂�
	RECT rcSrc;
	GetSrcRect(&rcSrc);

	if (::EqualRect(&rcSrc, &m_rcSrcEnd))
	{
		return;		// �ŏI�T�C�Y�Ɠ����Ȃ�Ή������Ȃ�
	}	

	int nDir = 1;
	if (m_rcSrcEnd.left < m_rcSrcEnd.left)
	{	// ����������
		nDir = -1;
	}

	::InflateRect(&rcSrc, 4 * nDir, 3 * nDir);
	SetSrcRect(&rcSrc);
}
