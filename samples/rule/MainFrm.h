// MainFrm.h : CMainFrame クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__0003F249_172A_11D4_880F_0000E842F190__INCLUDED_)
#define AFX_MAINFRM_H__0003F249_172A_11D4_880F_0000E842F190__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <vector>
#include <NxDraw/NxFont.h>

class CMainFrame : public CFrameWnd
{
protected: // シリアライズ機能のみから作成します。
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// アトリビュート
public:

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	void InitializeFPS();
	void UpdateFPS();
	BOOL OnIdle();
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CNxScreen* m_pScreen;
	CNxLayerSprite* m_pSpriteA;
	CNxLayerSprite* m_pSpriteB;
	CNxLayerSprite* m_pLayerFPS;
	BOOL m_bActive;
	CNxFont m_fontFPS;

	typedef std::vector<NxBlt> RuleContainer;
	RuleContainer m_rule;

// 生成されたメッセージ マップ関数
protected:
	int m_nRuleIndex;
	void SetRule(CNxSurfaceSprite* pSprite);
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnScreen();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewMmx();
	afx_msg void OnUpdateViewMmx(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MAINFRM_H__0003F249_172A_11D4_880F_0000E842F190__INCLUDED_)
