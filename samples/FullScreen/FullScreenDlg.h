// FullScreenDlg.h : �w�b�_�[ �t�@�C��
//

#if !defined(AFX_FULLSCREENDLG_H__D05A0DA6_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_FULLSCREENDLG_H__D05A0DA6_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "NxFPSSprite.h"

/////////////////////////////////////////////////////////////////////////////
// CFullScreenDlg dialog

class CFullScreenDlg : public CDialog
{
// �\�z
public:
	CFullScreenDlg(CWnd* pParent = NULL);	// �W���̃R���X�g���N�^

// Dialog Data
	//{{AFX_DATA(CFullScreenDlg)
	enum { IDD = IDD_FULLSCREEN_DIALOG };
		// ����: ���̈ʒu�� ClassWizard �ɂ���ăf�[�^ �����o���ǉ�����܂��B
	//}}AFX_DATA

	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CFullScreenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �̃T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	HICON m_hIcon;

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CFullScreenDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateSurface();
	CNxScreen* m_pScreen;
	CNxSurface* m_pSurface;
	CNxSurfaceSprite* m_pSprite;
	CNxFPSSprite* m_pSpriteFPS;
	int m_nHue;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_FULLSCREENDLG_H__D05A0DA6_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
