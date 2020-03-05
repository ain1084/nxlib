// MainFrm.h : CMainFrame �N���X�̐錾����уC���^�[�t�F�C�X�̒�`�����܂��B
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
protected: // �V���A���C�Y�@�\�݂̂���쐬���܂��B
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// �A�g���r���[�g
public:

// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
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

// �������ꂽ���b�Z�[�W �}�b�v�֐�
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
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_MAINFRM_H__0003F249_172A_11D4_880F_0000E842F190__INCLUDED_)
