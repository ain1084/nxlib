// NxBallSprite.cpp: CNxBallSprite �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxBallSprite.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxBallSprite::CNxBallSprite(CNxSprite* pParent, CNxSurface* pSurface)
 : CNxSurfaceSprite(pParent, pSurface)
{
	m_nDir = rand() % 4;			// �i�s����
	m_nSpeedH = rand() % 3 + 1;		// �����x
	m_nSpeedV = rand() % 3 + 1;		// �c���x

	int nColor = rand() % 6;
	RECT rect;
	rect.left = nColor * 32;
	rect.top = 0;
	rect.right = rect.left + 32;
	rect.bottom = 32;
	SetRect(&rect);

	// �]�����̈�
	GetParent()->GetRect(&m_rcBound);
	::InflateRect(&m_rcBound, 20, 20);	// �����g��
}

CNxBallSprite::~CNxBallSprite()
{

}

// �X�v���C�g�̍X�V��Ԃ����ׂ��钼�O�ɌĂяo����鉼�z�֐�
void CNxBallSprite::PreUpdate()
{
	static const POINT ptBallMove[] =
	{
		{  1, -1 },
		{  1,  1 },
		{ -1,  1 },
		{ -1, -1 },
	};

	POINT point;
	GetPos(&point);

	for (;;)
	{
		point.x += ptBallMove[m_nDir].x * m_nSpeedH;
		point.y += ptBallMove[m_nDir].y * m_nSpeedV;

		// �͂ݏo���Ă���Ε����]��
		if (point.x < m_rcBound.left || point.y < m_rcBound.top ||
			point.x + GetWidth() > m_rcBound.right ||
			point.y + GetHeight() > m_rcBound.bottom)
		{
			m_nDir = (m_nDir + 1) % 4;
		}
		else
			break;
	}
	SetPos(point.x, point.y);
}
