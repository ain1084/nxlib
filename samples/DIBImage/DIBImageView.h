// DIBImageView.h : CDIBImageView �N���X�̐錾����уC���^�[�t�F�C�X�̒�`�����܂��B
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBIMAGEVIEW_H__2B5140F2_DFBB_4105_82B6_45C262160F96__INCLUDED_)
#define AFX_DIBIMAGEVIEW_H__2B5140F2_DFBB_4105_82B6_45C262160F96__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDIBImageView : public CScrollView
{
protected: // �V���A���C�Y�@�\�݂̂���쐬���܂��B
	CDIBImageView();
	DECLARE_DYNCREATE(CDIBImageView)

// �A�g���r���[�g
public:
	CDIBImageDoc* GetDocument();

// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CDIBImageView)
	public:
	virtual void OnDraw(CDC* pDC);  // ���̃r���[��`�悷��ۂɃI�[�o�[���C�h����܂��B
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CDIBImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CDIBImageView)
	afx_msg void OnEditCopy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // DIBImageView.cpp �t�@�C�����f�o�b�O���̎��g�p����܂��B
inline CDIBImageDoc* CDIBImageView::GetDocument()
   { return (CDIBImageDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_DIBIMAGEVIEW_H__2B5140F2_DFBB_4105_82B6_45C262160F96__INCLUDED_)
