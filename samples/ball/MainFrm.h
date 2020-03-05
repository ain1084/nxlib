// MainFrm.h : CMainFrame クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__123605F9_E9FC_11D3_87E8_0000E842F190__INCLUDED_)
#define AFX_MAINFRM_H__123605F9_E9FC_11D3_87E8_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <vector>
#include "NxBallScreen.h"
#include <NxDraw/NxSurface.h>
#include <NxDraw/NxSurfaceSprite.h>
#include <NxDraw/NxLayerSprite.h>
#include <NxDraw/NxFont.h>

class CMainFrame : public CFrameWnd
{
protected: // シリアライズ機能のみから作成します。
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// アトリビュート
public:
	enum Background
	{
		Background_disable,
		Background_normal,
		Background_raster,
		Background_stretch,
		Background_stretch_linear_filter,
		Background_blur,
		Background_tile,
	};

	enum BallBlend
	{
		BallBlend_normal = NxBlt::blendNormal,
		BallBlend_add = NxBlt::blendAdd,
		BallBlend_sub = NxBlt::blendSub,
		BallBlend_multi = NxBlt::blendMulti,
		BallBlend_screen = NxBlt::blendScreen,
		BallBlend_darken = NxBlt::blendDarken,
		BallBlend_brighten = NxBlt::blendBrighten,
		BallBlend_none = -1,
	};

	enum { BallCount = 100 };

// オペレーション
public:
	BOOL OnIdle();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	typedef std::vector<CNxSurfaceSprite*> SpriteContainer;
	SpriteContainer m_spriteBall;

	CNxBallScreen* m_pScreen;
	CNxSurface* m_pSurfaceBall;
	CNxSurfaceSprite* m_pSpriteBG;
	CNxSurface* m_pSurfaceBG;
	CNxSurface* m_pSurfaceTile;
	CNxLayerSprite* m_pLayerFPS;
	BOOL m_bPause;
	BOOL m_bActive;
	CNxFont m_fontFPS;
	BOOL m_bTimer;
	Background m_background;
	BallBlend m_ballBlend;
	BOOL m_bTextSmooth;

	void InitializeFPS();
	void UpdateFPS();
	void SetBackground(Background nBackground);
	void SetBallSpriteBlend(BallBlend nBallBlend);
	void CreateBall();
	void DestroyBall();
	BallBlend CommandIdToBlend(UINT uID) const;

// 生成されたメッセージ マップ関数
protected:

	afx_msg void OnViewBlendRange(UINT nID);
	afx_msg void OnUpdateViewBlendRange(CCmdUI* pCmdUI);
	afx_msg void OnViewBackgroundRange(UINT nID);
	afx_msg void OnUpdateViewBackgroundRange(CCmdUI* pCmdUI);
	afx_msg void OnViewFilterRange(UINT nID);
	afx_msg void OnUpdateViewFilterRange(CCmdUI* pCmdUI);

	//{{AFX_MSG(CMainFrame)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnScreen();
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
	afx_msg void OnViewMmxEnable();
	afx_msg void OnViewPause();
	afx_msg void OnUpdateViewPause(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateViewMmxEnable(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnView3dnowEnable();
	afx_msg void OnUpdateView3dnowEnable(CCmdUI* pCmdUI);
	afx_msg void OnViewBall();
	afx_msg void OnViewTracking();
	afx_msg void OnUpdateViewTracking(CCmdUI* pCmdUI);
	afx_msg void OnViewBrightnessLight();
	afx_msg void OnViewBrightnessDark();
	afx_msg void OnUpdateViewBrightnessLight(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewBrightnessDark(CCmdUI* pCmdUI);
	afx_msg void OnViewBrightnessReset();
	afx_msg void OnViewFpsSmooth();
	afx_msg void OnUpdateViewFpsSmooth(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MAINFRM_H__123605F9_E9FC_11D3_87E8_0000E842F190__INCLUDED_)
