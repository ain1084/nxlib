// DIBImage.h : DIBIMAGE �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C��
//

#if !defined(AFX_DIBIMAGE_H__9DB620B6_9617_4CFA_A7E5_98596CFE1D3D__INCLUDED_)
#define AFX_DIBIMAGE_H__9DB620B6_9617_4CFA_A7E5_98596CFE1D3D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // ���C�� �V���{��

/////////////////////////////////////////////////////////////////////////////
// CDIBImageApp:
// ���̃N���X�̓���̒�`�Ɋւ��Ă� DIBImage.cpp �t�@�C�����Q�Ƃ��Ă��������B
//

class CDIBImageApp : public CWinApp
{
public:
	CDIBImageApp();

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CDIBImageApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����

	//{{AFX_MSG(CDIBImageApp)
	afx_msg void OnAppAbout();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_DIBIMAGE_H__9DB620B6_9617_4CFA_A7E5_98596CFE1D3D__INCLUDED_)
