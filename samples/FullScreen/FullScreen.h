// FullScreen.h : FULLSCREEN �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C���ł��B
//

#if !defined(AFX_FULLSCREEN_H__D05A0DA4_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
#define AFX_FULLSCREEN_H__D05A0DA4_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// ���C�� �V���{��

/////////////////////////////////////////////////////////////////////////////
// CFullScreenApp:
// ���̃N���X�̓���̒�`�Ɋւ��Ă� FullScreen.cpp �t�@�C�����Q�Ƃ��Ă��������B
//

class CFullScreenApp : public CWinApp
{
public:
	CFullScreenApp();

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CFullScreenApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����

	//{{AFX_MSG(CFullScreenApp)
		// ���� - ClassWizard �͂��̈ʒu�Ƀ����o�֐���ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_FULLSCREEN_H__D05A0DA4_4F8E_11D4_AAAB_0090CCA661BD__INCLUDED_)
