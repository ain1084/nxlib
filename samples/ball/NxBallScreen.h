// NxBallScreen.h: CNxBallScreen �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXBALLSCREEN_H__961C5D02_6884_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_NXBALLSCREEN_H__961C5D02_6884_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxDraw/NxScreen.h>

class CNxBallScreen : public CNxScreen  
{
public:
	enum Filter
	{
		Filter_none,			// �t�B���^����
		Filter_grayscale,		// �O���C�X�P�[����
		Filter_sepia,			// �Z�s�A��
		Filter_hueTransform,	// �F���ϊ�
		Filter_negative,		// �l�K���]
	};
	
	CNxBallScreen();
	virtual ~CNxBallScreen();

	Filter GetFilter() const;
	void SetFilter(Filter nFilter);

protected:
	virtual void DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const;

private:
	Filter m_nFilter;
};

inline CNxBallScreen::Filter CNxBallScreen::GetFilter() const {
	return m_nFilter; }

#endif // !defined(AFX_NXBALLSCREEN_H__961C5D02_6884_11D4_AAAB_0090CCA661BD__INCLUDED_)
