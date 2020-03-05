// NxBallScreen.h: CNxBallScreen クラスのインターフェイス
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
		Filter_none,			// フィルタ無し
		Filter_grayscale,		// グレイスケール化
		Filter_sepia,			// セピア調
		Filter_hueTransform,	// 色相変換
		Filter_negative,		// ネガ反転
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
