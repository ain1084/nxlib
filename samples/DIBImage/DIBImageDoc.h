// DIBImageDoc.h : CDIBImageDoc �N���X�̐錾����уC���^�[�t�F�C�X�̒�`�����܂��B
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBIMAGEDOC_H__2186E583_0C60_47D2_AD4E_36B5B7B80587__INCLUDED_)
#define AFX_DIBIMAGEDOC_H__2186E583_0C60_47D2_AD4E_36B5B7B80587__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CDIBImageDoc : public CDocument
{
protected: // �V���A���C�Y�@�\�݂̂���쐬���܂��B
	CDIBImageDoc();
	DECLARE_DYNCREATE(CDIBImageDoc)

// �A�g���r���[�g
public:

// �I�y���[�V����
public:
	CNxDIBImage* GetDIBImage() const;

//�I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CDIBImageDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CDIBImageDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CDIBImageDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CNxDIBImage* m_pDIBImage;
};

/////////////////////////////////////////////////////////////////////////////

inline CNxDIBImage* CDIBImageDoc::GetDIBImage() const {
	return m_pDIBImage; }

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_DIBIMAGEDOC_H__2186E583_0C60_47D2_AD4E_36B5B7B80587__INCLUDED_)
