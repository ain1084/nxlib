// NxWindow.cpp: CNxWindow �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000,2001 S.Ainoguchi
//
// �T�v: CNxSprite �h���N���X
//       ���ѕt�����E�B���h�E�̃N���C�A���g�̈�֍����`��
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include <algorithm>
#include "NxWindow.h"
#include "NxSurface.h"

namespace
{	// �E�B���h�E�v���p�e�B��
	const TCHAR szWindowPropertyName[] = _T("CNXWINDOW_THIS");
};

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxWindow::CNxWindow()
 : m_hWnd(NULL)								// ���ѕt�����Ă��� HWND
 , m_uBrightness(255)						// ���邳(255 = default)
 , m_nxcrBkColor(CNxColor().SetColorRef(::GetSysColor(COLOR_WINDOW)))
 , m_nxcrBrightColor(CNxColor::white)		// ���邢�F(brightness 0 �` 254)
 , m_nxcrDarkColor(CNxColor::black)			// �Â��F(brightness 256 - 511)
{
	EnableTracking(TRUE);
}

CNxWindow::~CNxWindow()
{
	Detach();
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxWindow::SetTrackingUnit(int nXUnit, int nYUnit)
// �T�v: �����ǐՂ̒P�ʂ�ݒ�
// ����: int nXUnit ... �������̒P��
//       int nYUnit ... �c�����̒P��
// �ߒl: �����Ȃ� TRUE
// ���l: CNxTrackingSprite::SetTrackingUnit() �̃I�[�o�[���C�h
///////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::SetTrackingUnit(int nXUnit, int nYUnit)
{
	if (!CNxTrackingSprite::SetTrackingUnit(nXUnit, nYUnit))
		return FALSE;

	// nXUnit, nYUnit �ɉ����ăo�b�t�@���Đ���
	return createBufferSurface();
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxWindow::Attach(HWND hWnd)
// �T�v: �E�B���h�E�����ѕt����
// ����: HWND hWnd ... ���ѕt����E�B���h�E�n���h��
// �ߒl: �����Ȃ� TRUE
////////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::Attach(HWND hWnd)
{
	_ASSERT(::IsWindow(hWnd));
	
	if (m_hWnd != NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxWindow::Attach() : ���ɃE�B���h�E�����ѕt�����Ă��܂�.\n");
		return FALSE;
	}

	// this �|�C���^��n���ׂɃE�B���h�E�v���p�e�B��ݒ�
#if defined(_DEBUG)
	if (::GetProp(hWnd, szWindowPropertyName) != NULL)
		_RPT0(_CRT_WARN, "CNxWindow::Attach() : �w�肳�ꂽ�E�B���h�E�͊��ɑ��� CNxWindow �N���X�ƌ��ѕt�����Ă���\��������܂�.\n");
#endif
	if (!::SetProp(hWnd, szWindowPropertyName, reinterpret_cast<HANDLE>(this)))
	{
		_RPT0(_CRT_ASSERT, "CNxWindow::Attach() : �E�B���h�E�v���p�e�B�̐ݒ�Ɏ��s���܂���.\n");
		return FALSE;
	}

	// �o�b�t�@�p�T�[�t�F�X�쐬
	createBufferSurface();

	m_hWnd = hWnd;

	// �T�u�N���X��(�E�B���h�E�v���V�[�W�������ւ�)
	m_pfnWndProcPrev = reinterpret_cast<WNDPROC>(::SetWindowLong(m_hWnd, GWL_WNDPROC, reinterpret_cast<LONG>(wndProc)));

	// ���݂̃T�C�Y�ŏ����l�Ƃ��Đݒ�
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	return SetSize(rcClient.right, rcClient.bottom);
}

///////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxWindow::createBufferSurface()
// �T�v: �o�b�t�@�p�T�[�t�F�X���쐬
// ����: �Ȃ�
// �ߒl: �����Ȃ� TRUE
// ���l: �����͌Œ�
///////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::createBufferSurface()
{
	SIZE sizeTracking;
	GetTrackingUnit(&sizeTracking);		// �X�V�P�ʂ��擾

	if (m_pBufferSurface.get() != 0)
	{	// �쐬�ς�
		int nWidth = m_pBufferSurface->GetWidth();
		int nHeight = m_pBufferSurface->GetHeight();
		if (nHeight == sizeTracking.cy && (nWidth % sizeTracking.cx) == 0)
		{
			return TRUE;		// ���݂̃T�C�Y�Ɠ���
		}
	}

	// �o�b�t�@�p�T�[�t�F�X�̐���
	m_pBufferSurface.reset(new CNxSurface(32));
	if (!m_pBufferSurface->Create(640 - 640 % sizeTracking.cx, sizeTracking.cy))
	{
		_RPTF0(_CRT_ERROR, "CNxWindow : �o�b�t�@�p�T�[�t�F�X�̍쐬�Ɏ��s���܂���.\n");
		m_pBufferSurface.reset();
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual HWND CNxWindow::Detach()
// �T�v: �E�B���h�E��؂藣��
// ����: �Ȃ�
// �ߒl: �؂藣���ꂽ�E�B���h�E�n���h��
// ���l: �q�X�v���C�g�������Ȃ������ŁA�قƂ�ǂ̃��\�[�X���J�������
///////////////////////////////////////////////////////////////////////////////////

HWND CNxWindow::Detach()
{
	HWND hWndResult = m_hWnd;
	if (m_hWnd != NULL)
	{
		// �T�u�N���X������
		::SetWindowLong(m_hWnd, GWL_WNDPROC, reinterpret_cast<LONG>(m_pfnWndProcPrev));
		// �E�B���h�E�v���p�e�B�폜
		::RemoveProp(m_hWnd, szWindowPropertyName);
		// �o�b�t�@�p�T�[�t�F�X�폜
		m_pBufferSurface.reset();
		m_hWnd = NULL;
	}
	return hWndResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static LRESULT CALLBACK CNxWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// �T�v: �T�u�N���X����ɌĂ΂��E�B���h�E�v���V�[�W��
// ����: HWND hWnd     ... �E�B���h�E�n���h��
//       UINT uMsg     ... ���b�Z�[�W
//       WPARAM wParam ... ���b�Z�[�W�̒ǉ����1
//       LPARAM lParam ... ���b�Z�[�W�̒ǉ����2
// �ߒl: ���b�Z�[�W�̎�ނɂ��قȂ�
///////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �E�B���h�E�v���p�e�B���� CNxWindow �ւ̃|�C���^�����o��
	CNxWindow* pThis = reinterpret_cast<CNxWindow*>(::GetProp(hWnd, szWindowPropertyName));

	// �T�u�N���X���O�̃E�B���h�E�v���V�[�W���̃|�C���^�����o��
	WNDPROC pfnWndProcPrev = pThis->m_pfnWndProcPrev;

	// ���z�֐��Ăяo��
	pThis->OnWndMessage(uMsg, wParam, lParam);
	
	// �ȑO�̃E�B���h�E�v���V�[�W���փ��b�Z�[�W��n��
	return ::CallWindowProc(pfnWndProcPrev, hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindow::OnWndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
// �T�v: �E�B���h�E���b�Z�[�W������
// ����: UINT uMsg     ... ���b�Z�[�W
//       WPARAM wParam ... ���b�Z�[�W�̒ǉ����1
//       LPARAM lParam ... ���b�Z�[�W�̒ǉ����2
//////////////////////////////////////////////////////////////////////////////////////////////////

void CNxWindow::OnWndMessage(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		SetSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_NCDESTROY:
		Detach();
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	NxColor CNxWindow::SetBkColor(NxColor nxcrBkColor)
// �T�v: �w�i�F�̐ݒ�
// ����: NxColor nxcrBkColor ... �w�i�F
// �ߒl: �ȑO�̔w�i�F
/////////////////////////////////////////////////////////////////////////////////

NxColor CNxWindow::SetBkColor(NxColor nxcrBkColor)
{
	std::swap(nxcrBkColor, m_nxcrBkColor);
	SetUpdate();
	return nxcrBkColor;
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	NxColor CNxWindow::SetDarkColor(NxColor nxcrDarkColor)
// �T�v: SetBrightness() �ɂ���āA�Â����鎞�̖ړI�F��ݒ�
// ����: NxColor nxcrDarkColor ... �ݒ�F
// �ߒl: �ȑO�̐ݒ�F
/////////////////////////////////////////////////////////////////////////////////

NxColor CNxWindow::SetDarkColor(NxColor nxcrDarkColor)
{
	std::swap(nxcrDarkColor, m_nxcrDarkColor);
	SetUpdate();
	return nxcrDarkColor;
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	NxColor CNxWindow::SetBrightColor(NxColor nxcrBrightColor)
// �T�v: SetBrightness() �ɂ���āA���邭���鎞�̖ړI�F��ݒ�
// ����: NxColor nxcrBrightColor ... �ݒ�F
// �ߒl: �ȑO�̐ݒ�F
/////////////////////////////////////////////////////////////////////////////////

NxColor CNxWindow::SetBrightColor(NxColor nxcrBrightColor)
{
	std::swap(nxcrBrightColor, m_nxcrBrightColor);
	SetUpdate();
	return nxcrBrightColor;
}

//////////////////////////////////////////////////////////////////////////////////
// public:
//	UINT CNxWindow::SetBrightness(UINT uBrightness)
// �T�v: ��ʂ̖��邳��ݒ�
// ����: UINT uBrightness ... ��ʂ̖��邳(0 �` 254, (255), 256 �` 511)
// �ߒl: �ȑO�̖��邳
//////////////////////////////////////////////////////////////////////////////////

UINT CNxWindow::SetBrightness(UINT uBrightness)
{
	if (uBrightness > 511)
	{
		_RPTF1(_CRT_ASSERT, "CNxWindow::SetBrightness() : �͈͊O�̒l(%d) ���w�肳��܂���.\n", uBrightness);
		return uBrightness;
	}

	if (m_uBrightness == uBrightness)
		return uBrightness;

	std::swap(m_uBrightness, uBrightness);
	SetUpdate();
	return uBrightness;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxWindow::Refresh(HDC hDC = NULL, BOOL bForce = FALSE)
// �T�v: �t���[���̕`��
// ����: HDC  hDC           ... �`���f�o�C�X�R���e�L�X�g(NULL �Ȃ�Ό��݂̃E�B���h�E��)
//       BOOL bForce        ... TRUE �ɂ���Ƌ����I�ɍĕ`��(�S�̈悪�ĕ`��Ώ�)
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::Refresh(HDC hDC, BOOL bForce)
{
	RefreshContext rc;
	if (hDC == NULL)
	{
		if (m_hWnd == NULL)
		{
			_RPTF0(_CRT_WARN, "CNxWindow::Refresh() : �E�B���h�E�� Attach ����Ă��Ȃ��ꍇ�AhDC �� NULL ���w�肷�鎖�͂ł��܂���.\n");
			return FALSE;
		}
		rc.hdcClient = NULL;
	}
	else
	{
		rc.hdcClient = hDC;
	}

	return CNxTrackingSprite::Refresh(m_pBufferSurface->GetWidth(), m_pBufferSurface->GetHeight(), &rc, bForce);
}

//////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual BOOL CNxWindow::RefreshBegin(LPVOID lpContext) const
// �T�v: �X�V��`������ꍇ�A��`�̗񋓂���钼�O�ɌĂяo�����֐�
// ����: Refresh() �֐��֓n���ꂽ lpContext ����
// �ߒl: TRUE �Ȃ�΋�`�̗񋓂��J�n�AFALSE �Ȃ�Β��~
//////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::RefreshBegin(LPVOID lpContext) const
{
	RefreshContext* prc = static_cast<RefreshContext*>(lpContext);

	// �o�b�t�@�p�T�[�t�F�X�� HDC ���擾
	prc->hdcBitmap = m_pBufferSurface->GetDC();
	if (prc->hdcBitmap == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxWindow::RefreshBegin() : �f�o�C�X�R���e�X�g�̎擾�Ɏ��s���܂���.\n");
		return FALSE;
	}

	if (prc->hdcClient == NULL)
	{
		// hdcClient �� NULL (Refresh() �֐��� HDC ���ȗ����ꂽ�ꍇ) �Ȃ�΁A
		// GetDC() �ɂ���ăE�B���h�E�f�o�C�X�R���e�L�X�g���擾����
		prc->hdcClient = ::GetDC(m_hWnd);
		if (prc->hdcClient == NULL)
		{	// �擾���s
			m_pBufferSurface->ReleaseDC();
			prc->hdcBitmap = NULL;
			_RPTF0(_CRT_WARN, "CNxWindow::RefreshBegin() : �f�o�C�X�R���e�X�g�̎擾�Ɏ��s���܂���.\n");
			return FALSE;
		}
		prc->bReleaseDC = TRUE;		// �v ReleaseDC() �t���O
	}
	else
	{
		prc->bReleaseDC = FALSE;	// ReleaseDC() �͕s�v
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindow::RefreshEnd(LPVOID lpContext) const
// �T�v: �X�V��`������ꍇ�A�S�Ă̋�`�̗񋓂��I��������ɌĂяo�����֐�
// ����: Refresh() �֐��֓n���ꂽ lpContext ����
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////

void CNxWindow::RefreshEnd(LPVOID lpContext) const
{
	RefreshContext* prc = static_cast<RefreshContext*>(lpContext);

	// RefreshBegin() ���� GetDC() �ɂ���āA
	// �E�B���h�E�f�o�C�X�R���e�X�g���擾�����Ȃ�΁A�f�o�C�X�R���e�L�X�g���J��
	if (prc->bReleaseDC)
		::ReleaseDC(m_hWnd, prc->hdcClient);

	// �o�b�t�@�T�[�t�F�X�ւ� HDC ���J��
	m_pBufferSurface->ReleaseDC();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindow::RefreshRect(const RECT* lpRect, LPVOID lpContext) const
// �T�v: �X�V���ꂽ������ HDC �֓]��(CNxWindow::Refresh() ��������Ăяo�����)
// ����: const RECT* lpRect ... �X�V�����`
//       LPVOID lpContent   ... �R���e�L�X�g(EnumRectContext �\���̂ւ̃|�C���^)
// �ߒl: �Ȃ�
// ���l: lpContext �� RefreshContext �\���̂ւ̃|�C���^
////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxWindow::RefreshRect(const RECT* lpRect, LPVOID lpContext) const
{
	// �o�b�t�@�T�[�t�F�X�փX�v���C�g��`��
	DrawSurface(m_pBufferSurface.get(), 0, 0, lpRect);

	RefreshContext* prc = static_cast<RefreshContext*>(lpContext);
	// ���(HDC)�֓]��
	::BitBlt(prc->hdcClient,												// dest HDC
			 lpRect->left, lpRect->top,										// dx, dy
			 lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,	// nx, ny
			 prc->hdcBitmap,												// src HDC
			 0, 0,															// sx, sy
			 SRCCOPY);														// raster operation
}

////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindow::Draw(CNxSurface* pSurface, const RECT* lpRect) const
// �T�v: �X�v���C�g�`��
// ����: CNxSurface* pSurface ... �`���T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpRect   ... �X�v���C�g���̕`���`������ RECT �\���̂ւ̃|�C���^
// �ߒl: �q�X�v���C�g�̕`��𑱂���Ȃ�� TRUE
// ���l: �ŏ�ʃX�v���C�g�Ȃ̂ŁA��\���̏�Ԃł��Ăяo�����
////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::Draw(CNxSurface* pSurface, const RECT* lpRect) const
{
	if (IsVisible())
	{	// �\����
		if (CNxColor(m_nxcrBkColor).GetAlpha() != 0)
			pSurface->FillRect(lpRect, m_nxcrBkColor);
	}
	else
	{	// ��\��
		pSurface->FillRect(lpRect, m_nxcrDarkColor);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindows::DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const
// �T�v: �S�Ă̎q�X�v���C�g�̌�ɕ`��
// ����: CNxSurface* pSurface ... �`���T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpRect   ... �X�v���C�g���̕`���`������ RECT �\���̂ւ̃|�C���^
// �ߒl: �Ȃ�
// ���l: brightness �K�p����
/////////////////////////////////////////////////////////////////////////////////////////

void CNxWindow::DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const
{
	// brightness �K�p
	if (m_uBrightness != 255)
	{
		NxBlt nxb;
		if (m_uBrightness > 255)
		{	// ���邭���� (256 - 511)
			nxb.nxbColor = CNxColor(m_nxcrBrightColor).SetAlpha(static_cast<BYTE>(m_uBrightness - 256));
		}
		else
		{	// �Â����� (0 - 254)
			nxb.nxbColor = CNxColor(m_nxcrDarkColor).SetAlpha(static_cast<BYTE>(255 - m_uBrightness));
		}
		nxb.dwFlags = NxBlt::colorFill | NxBlt::blendNormal;
		pSurface->Blt(lpRect, NULL, NULL, &nxb);
	}
}

/////////////////////////////////////////////////////////////////////
// public:
//	void CNxWindow::GetCursorPos(LPPOINT lpPoint) const
// �T�v: �}�E�X�J�[�\���̍��W�𓾂�
// ����: LPPOINT lpPoint ... ���W���󂯂Ƃ� POINT �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
// ���l: �ߒl�̓E�B���h�E�̃N���C�A���g���W������
//////////////////////////////////////////////////////////////////////

BOOL CNxWindow::GetCursorPos(LPPOINT lpPoint) const
{
	_ASSERTE(lpPoint != NULL);
	if (m_hWnd == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxWindow::GetCursorPos() : �E�B���h�E�� Attach() ����Ă��܂���.\n");
		return FALSE;
	}	
	::GetCursorPos(lpPoint);
	::ScreenToClient(m_hWnd, lpPoint);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxWindow::SetCursorPos(int x, int y)
// �T�v: �}�E�X�J�[�\���̍��W��ݒ�
// ����: int x ... X ���W(�X�v���C�g�����W)
//       int y ... Y ���W(�X�v���C�g�����W)
// �ߒl: �����Ȃ�� TRUE
//////////////////////////////////////////////////////////////////////

BOOL CNxWindow::SetCursorPos(int x, int y)
{
	if (m_hWnd == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxWindow::GetCursorPos() : �E�B���h�E�� Attach() ����Ă��܂���.\n");
		return FALSE;
	}
	POINT ptPoint;
	ptPoint.x = x;
	ptPoint.y = y;
	::ClientToScreen(m_hWnd, &ptPoint);
	::SetCursorPos(ptPoint.x, ptPoint.y);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxSprite* CNxWindow::SetParent(CNxSprite* pNewParent)
// �T�v: �e��ύX����
// ����: CNxSprite* pNewParent ... �V�����e
// �ߒl: ���O�̐e�BNULL �Ȃ�Ύ��s
// ���l: CNxSprite::SetParent() �̃I�[�o�[���C�h�B��Ɏ��s����
//////////////////////////////////////////////////////////////////////////////

CNxSprite* CNxWindow::SetParent(CNxSprite* /*pNewParent*/)
{
	_RPTF0(_CRT_ASSERT, "CNxWindow::SetParent() : CNxWindow �̐e�͐ݒ�ł��܂���.\n");
	return NULL;
}
