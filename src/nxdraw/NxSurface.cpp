// NxSurface.cpp: CNxSurface �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000,2001 S.Ainoguchi
//
// �T�v: �T�[�t�F�X�Ǘ��N���X
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include <math.h>
#include <algorithm>
#include "NxSurface.h"
#include "NxCustomDraw8.h"
#include "NxCustomDraw32.h"
#include "NxDynamicDraw8.h"
#include "NxDynamicDraw32.h"
#include "NxDrawLocal.h"
#include "NxFont.h"
#include "NxBMPImageSaver.h"

using namespace NxDrawLocal;

namespace
{	// �S�Ă� CNxSurface �N���X�̃C���X�^���X�́A�ȉ��̉��ꂩ���g�p
	const CNxCustomDraw8  g_customDraw8;	// for 8bpp
	const CNxCustomDraw32 g_customDraw32;	// for 32bpp
}


/////////////////////////////////////////////////////////////////////////
// public:
//	CNxSurface::CNxSurface(UINT uBitCount = 32)
// �T�v: CNxSurface �N���X�̃R���X�g���N�^
// ����: UINT uBitCount ... �r�b�g�[�x(32 ���� 8)
// �ߒl: ---
/////////////////////////////////////////////////////////////////////////

CNxSurface::CNxSurface(UINT uBitCount)
 : m_customDraw((uBitCount == 8) ? static_cast<const CNxCustomDraw&>(g_customDraw8) : static_cast<const CNxCustomDraw&>(g_customDraw32))
 , m_lpbmi(reinterpret_cast<LPBITMAPINFO>(uBitCount))	// Create() �����܂ł̊ԁA�s�N�Z���r�b�g����ێ�����
 , m_hBitmap(NULL)				// DIBSection �̃n���h��(HBITMAP �^)
 , m_hdcCurrent(NULL)			// ���ݎ擾����Ă��� HDC
 , m_udcRefCount(0)				// ���ݎ擾����Ă��� HDC �̎Q�ƃJ�E���g
 , m_pFont(NULL)				// CNxFont �ւ̃|�C���^
 , m_bTextSmoothing(FALSE)		// �e�L�X�g�̃X���[�W���O(off)
{
	switch (uBitCount)
	{
	case 8:
		m_pDynamicDraw = new CNxDynamicDraw8;
		break;
	default:
		if (uBitCount != 32)
		{
			_RPT1(_CRT_WARN, "CNxSurface::CNxSurface() : �T�|�[�g���Ă��Ȃ��r�b�g�[�x(%d)���w�肳��܂���.32bpp �ƌ��Ȃ���܂�.\n", uBitCount);
			uBitCount = 32;
		}
	case 32:
		m_pDynamicDraw = new CNxDynamicDraw32;
	}
	m_ptOrg.x = 0;				// ���_���W X
	m_ptOrg.y = 0;				// ���_���W Y
	::SetRectEmpty(&m_rcClip);	// �N���b�v��`
}

/////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxSurface::~CNxSurface()
// �T�v: CNxSurface �N���X�̃f�X�g���N�^
// ����: ---
// �ߒl: ---
/////////////////////////////////////////////////////////////////////////

CNxSurface::~CNxSurface()
{
	if (m_hdcCurrent != NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::~CNxSurface() : �f�o�C�X�R���e�L�X�g���g�p����Ă��܂�.\n");
		::DeleteDC(m_hdcCurrent);
	}

	// ���I�`��R�[�h�N���X�폜
	delete m_pDynamicDraw;
	
	// DIBSection �폜
	if (m_hBitmap != NULL)
	{
		if (!::DeleteObject(m_hBitmap))
		{
			_RPTF0(_CRT_ASSERT, "CNxSurface::~CNxSurface() : DIBSection �̍폜�Ɏ��s���܂���.\n");
		}
	}

	if (HIWORD(m_lpbmi) != 0)
	{	// m_lpbmi �� Create() �����܂ł̊ԁA����word �Ƀr�b�g�[�x��ێ�����
		// BITMAPINFO �\���̂̃��������J��
		free(m_lpbmi);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxSurface::Create(int nWidth, int nHeight)
// �T�v: �T�[�t�F�X���쐬
// ����: int nWidth     ... ��
//       int nHeight    ... ����
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Create(int nWidth, int nHeight)
{
	_ASSERTE(m_hBitmap == NULL);
	_ASSERTE(nWidth >= 0);

	// BITMAPINFO �\���̂̃��������m��
	// �R���X�g���N�^�ɂ���� m_lpbmi �̉��� word �ւ̓r�b�g�[�x���ݒ肳��Ă���
	// 8bpp �Ȃ�΃J���[�e�[�u�� 256�����m��
	UINT uBitCount = reinterpret_cast<UINT>(m_lpbmi);
	m_lpbmi = static_cast<LPBITMAPINFO>(calloc(sizeof(BITMAPINFOHEADER) + ((uBitCount == 32) ? 0 : sizeof(RGBQUAD) * 256), sizeof(BYTE)));

	// BITMAPINFOHEADER �ݒ�
	m_lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_lpbmi->bmiHeader.biWidth = nWidth;
	m_lpbmi->bmiHeader.biHeight = nHeight;
	m_lpbmi->bmiHeader.biPlanes = 1;
	m_lpbmi->bmiHeader.biBitCount = static_cast<WORD>(uBitCount);
	m_lpbmi->bmiHeader.biXPelsPerMeter = 0;
	m_lpbmi->bmiHeader.biYPelsPerMeter = 0;
	m_lpbmi->bmiHeader.biClrImportant = 0;
	m_lpbmi->bmiHeader.biCompression = BI_RGB;
	m_lpbmi->bmiHeader.biClrUsed = 0;
	m_lpbmi->bmiHeader.biSizeImage = 0;

	if (uBitCount == 8)
	{	// 8bpp �̏ꍇ�̓p���b�g��������
		for (UINT u = 0; u < 256; u++)
		{	// grayscale
			m_lpbmi->bmiColors[u].rgbBlue = static_cast<BYTE>(u);
			m_lpbmi->bmiColors[u].rgbGreen = static_cast<BYTE>(u);
			m_lpbmi->bmiColors[u].rgbRed = static_cast<BYTE>(u);
			m_lpbmi->bmiColors[u].rgbReserved = 0;
		}
	}

	// DIBSection �쐬
	LPVOID lpvBits;
	m_hBitmap = ::CreateDIBSection(NULL, m_lpbmi, DIB_RGB_COLORS, &lpvBits, NULL, NULL);
	if (m_hBitmap == NULL)
	{
		free(m_lpbmi);
		m_lpbmi = NULL;
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : CreateDIBSection() �֐������s���܂���.\n");
		return FALSE;
	}

	// CNxDIBImage ���쐬
	m_dibImage.Create(m_lpbmi, lpvBits);

	// �N���b�s���O��`���T�[�t�F�X�S�̂ɐݒ�
	SetClipRect(NULL);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Create(const BITMAPINFO* lpbmi, LPCVOID lpvBits = NULL)
// �T�v: DIB �Ɠ����T�C�Y�̃T�[�t�F�X�����A���e���R�s�[����
// ����: const BITMAPINFO* lpbmi ... DIB �̏������� BITMAPINFO �\���̂ւ̃|�C���^
//       LPCVOID lpvBits         ... DIB �̃r�b�g�f�[�^�ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Create(const BITMAPINFO* lpbmi, LPCVOID lpvBits)
{
	_ASSERTE(m_hBitmap == NULL);
	_ASSERTE(lpbmi != NULL);
	
	// DIB �̕��ƍ����ŃT�[�t�F�X���쐬
	if (!Create(lpbmi->bmiHeader.biWidth, lpbmi->bmiHeader.biHeight))
		return FALSE;

	// �쐬�����T�[�t�F�X�� DIB �̓��e���R�s�[
	return SetDIBits(0, 0, lpbmi, lpvBits);
}

/////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Create(const CNxDIBImage* pDIBImage)
// �T�v: CNxDIBImage �Ɠ����T�C�Y�̃T�[�t�F�X�����A���e���R�s�[����
// ����: const CNxDIBImage* pDIBImage ... CNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Create(const CNxDIBImage* pDIBImage)
{
	_ASSERT(m_hBitmap == NULL);
	
	// DIB �̕��ƍ����ŃT�[�t�F�X���쐬
	if (!Create(pDIBImage->GetInfoHeader()->biWidth, pDIBImage->GetInfoHeader()->biHeight))
		return FALSE;

	// �쐬�����T�[�t�F�X�� DIB �̓��e���R�s�[
	if (!SetDIBits(0, 0, pDIBImage))
	{
		_RPT0(_CRT_ASSERT, "CNxSurface::Create() : ���̌`���� DIB �ɂ͑Ή����Ă��܂���.\n");
		return FALSE;
	}
	// �J���[�e�[�u���̐ݒ�
	SetColorTable(pDIBImage);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Create(CNxFile& nxfile)
// �T�v: �C���[�W��Ǎ��݁A�C���[�W�Ɠ����T�C�Y�̃T�[�t�F�X�����
// ����: CNxFile&  nxfile ... �ǂݍ��݌��t�@�C��
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////
 
BOOL CNxSurface::Create(CNxFile& nxfile)
{
	_ASSERTE(m_hBitmap == NULL);

	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : �I�[�v���ς݂łȂ� CNxFile �I�u�W�F�N�g���n����܂���.\n");
		return FALSE;
	}
	
	// �摜�̓W�J
	std::auto_ptr<CNxDIBImage> pDIBImage(CNxDraw::GetInstance()->LoadImage(nxfile));
	if (pDIBImage.get() == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : �摜�̓W�J�Ɏ��s���܂���.\n");
		return FALSE;
	}

	// �摜��ݒ�
	if (!Create(pDIBImage.get()))
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::Create() : �T�[�t�F�X�̍쐬���͉摜�̕ϊ��Ɏ��s���܂���.\n");
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	HDC CNxSurface::GetDC()
// �T�v: �T�[�t�F�X�֕`�悷��ׂ̃f�o�C�X�R���e�L�X�g���擾����
// ����: �Ȃ�
// �ߒl: �f�o�C�X�R���e�L�X�g(NULL �Ȃ�΃G���[)
///////////////////////////////////////////////////////////////////////////////

HDC CNxSurface::GetDC()
{
	_ASSERTE(m_hBitmap != NULL);
	
	if (m_udcRefCount != 0)
	{	// �擾�ς݂Ȃ�ΎQ�ƃJ�E���^�𑝂₷
		m_udcRefCount++;
		return m_hdcCurrent;
	}

	// �������f�o�C�X�R���e�L�X�g���쐬
	HDC hDC = ::CreateCompatibleDC(NULL);
	if (hDC == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::GetDC() : �f�o�C�X�R���e�L�X�g�̍쐬�Ɏ��s���܂���.\n");
		return NULL;
	}
	
	// hBitmap ��I��
	if (::SelectObject(hDC, m_hBitmap) == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::GetDC() : �r�b�g�}�b�v�� SelectObject() �Ɏ��s���܂���.\n");
		::DeleteDC(hDC);
		return NULL;
	}

	m_udcRefCount = 1;		// �Q�ƃJ�E���g�� 1
	m_hdcCurrent = hDC;
	return hDC;
}

/////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::ReleaseDC()
// �T�v: CNxSurface::GetDC() �œ���ꂽ�f�o�C�X�R���e�L�X�g���J��
// ����: �Ȃ�
// �ߒl: �f�o�C�X�R���e�L�X�g�̎Q�ƃJ�E���g��
/////////////////////////////////////////////////////////////////////////////////

UINT CNxSurface::ReleaseDC()
{
	_ASSERTE(m_hBitmap != NULL);

	if (m_hdcCurrent == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::ReleaseDC() : �f�o�C�X�R���e�L�X�g�͎擾����Ă��܂���.\n");
		return 0;			// �擾���Ă��Ȃ�
	}
	if (--m_udcRefCount == 0)
	{	// �Q�ƃJ�E���^�� 0 �Ȃ�� DC ���폜
		::DeleteDC(m_hdcCurrent);
		m_hdcCurrent = NULL;
	}
	return m_udcRefCount;
}

//////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::TileBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//							 const RECT* lpSrcRect, int nSrcXOrg, int nSrcYOrg,
//							 NxBlt* pNxBlt = NULL)
// �T�v: �]�������^�C����ɕ~���l�߂� Blt
// ����: const RECT* lpDestRect        ... �]�����`
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X
//       const RECT* lpSrcRect         ... �]������`
//       int nSrcXOrg                  ... �]���� X ���W���_
//       int nSrcYOrg                  ... �]���� Y ���W���_
//       const NxBlt* pNxBlt       ... �]�����@���w��
// �ߒl: �����Ȃ�� TRUE
// ���l: �]�������]�ɂ͖��Ή�
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::TileBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
						 const RECT* lpSrcRect, int nSrcXOrg, int nSrcYOrg, NxBlt* pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	if (pSrcSurface == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::TileBlt() : �]�����͏ȗ��ł��܂���.\n");
		return FALSE;
	}

	RECT rcDest;
	SetAbbreviateRect(&rcDest, lpDestRect);		// NULL �΍�

	RECT rcSrc;
	pSrcSurface->SetAbbreviateRect(&rcSrc, lpSrcRect);	// NULL �΍�

	// �]�����̍��オ���̒l�Ȃ�΁A���_�։��Z
	if (rcSrc.left < 0)
	{
		nSrcXOrg += rcSrc.left;
		rcSrc.right -= rcSrc.left;
		rcSrc.left = 0;
	}
	if (rcSrc.top < 0)
	{
		nSrcYOrg += rcSrc.top;
		rcSrc.bottom -= rcSrc.top;
		rcSrc.top = 0;
	}

	// �]�����̕��ƍ���
	int nSrcWidth = rcSrc.right - rcSrc.left;
	int nSrcHeight = rcSrc.bottom - rcSrc.top;

	if ((nSrcWidth - 1 | nSrcHeight - 1) + 1 == 0)
		return TRUE;		// �]����`�Ȃ��B�������[�v�� divide by zero ����̈� return ����

	// �]�����Ɠ]�����`�̃T�C�Y�������A���_���W�� X,Y ���Ƀ[���Ȃ�΁A���ʂ� Blt
	if ((((rcDest.right - rcDest.left) - nSrcWidth | (rcDest.bottom - rcDest.top) - nSrcHeight) | nSrcXOrg | nSrcYOrg) == 0)
		return Blt(&rcDest, pSrcSurface, &rcSrc, pNxBlt);

	// ���_���]�����͈̔͂��͂ݏo���Ȃ��l�ɏC��
	nSrcXOrg %= nSrcWidth;
	nSrcYOrg %= nSrcHeight;

	// ���_�𒲐�
	nSrcXOrg += (nSrcXOrg < 0) ? rcSrc.right : rcSrc.left;
	nSrcYOrg += (nSrcYOrg < 0) ? rcSrc.bottom : rcSrc.top;

	// ���炩�ɕ`�悳��Ȃ���������菜���A��`���k��
	if (rcDest.left + (rcSrc.right - nSrcXOrg) + m_ptOrg.x < m_rcClip.left)
	{
		rcDest.left += (rcSrc.right - nSrcXOrg);
		nSrcXOrg = rcSrc.left;
		int nDestLeftMargin = m_rcClip.left - (rcDest.left + m_ptOrg.x);
		rcDest.left += nDestLeftMargin - (nDestLeftMargin % nSrcWidth);
	}
	if (rcDest.right + m_ptOrg.x > m_rcClip.right)
		rcDest.right = m_rcClip.right - m_ptOrg.x;

	if (rcDest.top + (rcSrc.bottom - nSrcYOrg) + m_ptOrg.y < m_rcClip.top)
	{
		rcDest.top += (rcSrc.bottom - nSrcYOrg);
		nSrcYOrg = rcSrc.top;
		int nDestTopMargin = m_rcClip.top - (rcDest.top + m_ptOrg.y);
		rcDest.top += nDestTopMargin - (nDestTopMargin % nSrcWidth);
	}
	if (rcDest.bottom + m_ptOrg.y > m_rcClip.bottom)
		rcDest.bottom = m_rcClip.bottom - m_ptOrg.y;

	POINT ptSrc;
	POINT ptDest;

	for (ptDest.y = rcDest.top, ptSrc.y = nSrcYOrg; ptDest.y < rcDest.bottom;)
	{
		for (ptDest.x = rcDest.left, ptSrc.x = nSrcXOrg; ptDest.x < rcDest.right;)
		{
			RECT rcTileSrc;
			RECT rcTileDest;

			rcTileSrc.left = ptSrc.x;
			rcTileSrc.right = rcSrc.right;
			rcTileSrc.top = ptSrc.y;
			rcTileSrc.bottom = rcSrc.bottom;

			nSrcWidth = rcTileSrc.right - rcTileSrc.left;
			nSrcHeight = rcTileSrc.bottom - rcTileSrc.top;

			rcTileDest.left = ptDest.x;
			rcTileDest.right = rcTileDest.left + nSrcWidth;
			rcTileDest.top = ptDest.y;
			rcTileDest.bottom = rcTileDest.top + nSrcHeight;
			
			if (NxDrawLocal::ClipRect(&rcTileDest, &rcTileSrc, &rcDest, NULL))
			{
				if (!Blt(&rcTileDest, pSrcSurface, &rcTileSrc, pNxBlt))
					return FALSE;
			}

			// ���� X ���W��
			ptDest.x += nSrcWidth;
			// �]���� X ���_�� reset
			ptSrc.x = rcSrc.left;
		}
		// �]�����]�����̍�������������
		ptDest.y += nSrcHeight;
		// �]���� Y ���_�� reset
		ptSrc.y = rcSrc.top;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Blt(int dx, int dy, CNxSurface* pSurface, const RECT* lpSrcRect, NxBlt* pNxBlt = NULL)
// �T�v: �r�b�g�u���b�N�]��
// ����: int dx                  ... �]���� X ���W
//       int dy                  ... �]���� Y ���W
//       CNxSurface* pSurface    ... �]�����T�[�t�F�X
//       const RECT* lpSrcRect   ... �]������`
//       const NxBlt* pNxBlt ... �]�����@���w�肷�� 
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Blt(int dx, int dy, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	if (lpSrcRect == NULL)
	{
		if (pSrcSurface == NULL)
		{
			_RPTF0(_CRT_ASSERT, "CNxSurface::Blt() : �]�����͏ȗ��ł��܂���.\n");
			return FALSE;
		}
		pSrcSurface->GetRect(&rcSrc);
		lpSrcRect = &rcSrc;
	}

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + abs(lpSrcRect->right - lpSrcRect->left);
	rcDest.bottom = dy + abs(lpSrcRect->bottom - lpSrcRect->top);
	return Blt(&rcDest, pSrcSurface, lpSrcRect, pNxBlt);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::Blt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, NxBlt* pNxBlt = NULL)
// �T�v: �r�b�g�u���b�N�]��
// ����: const RECT* lpDestRect        ... �]�����`
//       const CNxSurface* pSrcSurface ... �]�����T�[�t�F�X
//       const RECT* lpSrcRect         ... �]������`
//       const NxBlt* pNxBlt       ... �]�����@���w�肷��(NULL �Ȃ�΃J���[�L�[�����P���]��)
// �ߒl: �����Ȃ�� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::Blt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	RECT rcDest;

	// �]���悪�ȗ�(NULL)����Ă���ΑS�̂�������`���A�����łȂ���� lpDestRect �� copy
	SetAbbreviateRect(&rcDest, lpDestRect);

	if (pSrcSurface == NULL)
	{	// �]�������ȗ�����Ă���΁A�]���悪�w�肳�ꂽ�ƌ��Ȃ�
		pSrcSurface = this;
		rcSrc = rcDest;
	}
	else
	{	// lpSrcRect �� NULL �ł���ΑS��
		pSrcSurface->SetAbbreviateRect(&rcSrc, lpSrcRect);
	}

	if (pNxBlt != NULL && pNxBlt->dwFlags != 0)
	{
		const DWORD& dwFlags = pNxBlt->dwFlags;

		// ���E���]
		if (dwFlags & NxBlt::mirrorLeftRight)
			std::swap(rcSrc.left, rcSrc.right);

		// �㉺���]
		if (dwFlags & NxBlt::mirrorTopDown)
			std::swap(rcSrc.top, rcSrc.bottom);
		
		// ���I�R�[�h���s
		if (dwFlags & NxBlt::dynamic)
		{
			return m_pDynamicDraw->Blt(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
		}
		// colorFill (color | fill)
		else if ((dwFlags & (NxBlt::color | NxBlt::fill)) == (NxBlt::color | NxBlt::fill))
		{
			if ((dwFlags & (NxBlt::destAlpha | NxBlt::srcAlpha)) == (NxBlt::destAlpha | NxBlt::srcAlpha))
			{	// �]����Ɠ]�����A���t�@���g�p����u�����h
				return m_customDraw.Blt_ColorFill_BlendDestSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::blurHorz)
			{	// �����ڂ����u�����h�h��Ԃ�
				return m_customDraw.Blt_ColorFill_BlurHorzBlend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::srcAlpha)
			{	// �]�����A���t�@�̂ݎg�p����u�����h
				return m_customDraw.Blt_ColorFill_BlendSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::destAlpha)
			{	// �]����A���t�@�̂ݎg�p����u�����h
				return m_customDraw.Blt_ColorFill_BlendDestAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::rgbaMask)
			{	// �}�X�N�t���h��ׂ�
				return m_customDraw.Blt_ColorFill_RGBAMask(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else
			{	// �u�����h�h��ׂ�
				return m_customDraw.Blt_ColorFill_Blend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
		}
		else
		{
			if (dwFlags & NxBlt::rule)
			{	// ���[���摜�g�p�u�����h
				return m_customDraw.Blt_RuleBlend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::blurHorz)
			{	// �����ڂ���
				return m_customDraw.Blt_BlurHorz(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if ((dwFlags & (NxBlt::destAlpha | NxBlt::srcAlpha)) == (NxBlt::destAlpha | NxBlt::srcAlpha))
			{	// �]����Ɠ]�����A���t�@���g�p����u�����h Blt
				return m_customDraw.Blt_BlendDestSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::srcAlpha)
			{	// �]�����A���t�@�̂ݎg�p����u�����h Blt
				return m_customDraw.Blt_BlendSrcAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::destAlpha)
			{	// �]����A���t�@�̂ݎg�p����u�����h Blt
				return m_customDraw.Blt_BlendDestAlpha(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else if (dwFlags & NxBlt::rgbaMask)
			{	// �}�X�N�]��
				return m_customDraw.Blt_RGBAMask(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
			else
			{	// �A���t�@���g�p���Ȃ��u�����h�]��
				return m_customDraw.Blt_Blend(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
			}
		}
	}
	return m_customDraw.Blt_Normal(this, &rcDest, pSrcSurface, &rcSrc, pNxBlt);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FilterBlt(int dx, int dy, const CNxSurface* pSrcSurface,
//							   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
// �T�v: �t�B���^��K�p���ē]��
// ����: int dx                        ... �t�B���^�K�p���ʂ̓]���捶�� X ���W
//       int dy                        ... �t�B���^�K�p���ʂ̓]���捶�� Y ���W
//       const CNxSurface* pSrcSurface ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FilterBlt(int dx, int dy, const CNxSurface* pSrcSurface,
						   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	if (lpSrcRect == NULL)
	{
		if (pSrcSurface == NULL)
		{
			_RPTF0(_CRT_ASSERT, "CNxSurface::FilterBlt() : �]�����͏ȗ��ł��܂���.\n");
			return FALSE;
		}
		pSrcSurface->GetRect(&rcSrc);
		lpSrcRect = &rcSrc;
	}

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + abs(lpSrcRect->right - lpSrcRect->left);
	rcDest.bottom = dy + abs(lpSrcRect->bottom - lpSrcRect->top);
	return FilterBlt(&rcDest, pSrcSurface, lpSrcRect, pNxFilterBlt);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FilterBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
//							   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
// �T�v: �t�B���^��K�p���ē]��
// ����: const RECT* lpDestRect        ... �t�B���^�K�p���ʂ̓]�����`������ RECT �\���̂ւ̃|�C���^
//       const CNxSurface* pSrcSurface ... �t�B���^���K�p�����T�[�t�F�X�ւ̃|�C���^
//       const RECT* lpSrcRect         ... �t�B���^���K�p������`������ RECT �\���̂ւ̃|�C���^
//       const NxFilterBlt* pNxFilterBlt     ... NxFilterBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FilterBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface,
						   const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcDest;
	RECT rcSrc;

	SetAbbreviateRect(&rcDest, lpDestRect);
	if (pSrcSurface == NULL)
	{	// pSrcSurface == NULL �Ȃ�΁A�N���b�v���s������̓]�����`�Ɠ����ɂ���
		if (!ClipBltRect(rcDest))
			return TRUE;		// �]����`�Ȃ�

		pSrcSurface = this;		// �]�����Ɠ]����T�[�t�F�X�͓���
		rcSrc = rcDest;
	}
	else
	{	// �]�����w�肠��
		pSrcSurface->SetAbbreviateRect(&rcSrc, lpSrcRect);

		RECT rcSrcClip;
		pSrcSurface->GetRect(&rcSrcClip);
		if (!ClipBltRect(rcDest, rcSrc, rcSrcClip))
			return TRUE;		// �]����`�Ȃ�
	}

	// �S�Ẵt�B���^�͊g��k�����T�|�[�g���Ȃ�
	if (((rcDest.right - rcDest.left) - abs(rcSrc.right - rcSrc.left) |
		(rcDest.bottom - rcDest.top) - abs(rcSrc.bottom - rcSrc.top)) != 0)
	{
		_RPTF0(_CRT_ASSERT, "�g��k���̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}
	
	// NxFilterBlt::uOpacity �͈̔͂��`�F�b�N
	if (pNxFilterBlt->dwFlags & NxFilterBlt::opacity)
	{	// uOpacity == 0 �Ȃ�΁A�������Ȃ�
		if (pNxFilterBlt->uOpacity == 0)
			return TRUE;
		else if (pNxFilterBlt->uOpacity > 255)
		{	// uOpacity �� 0 �` 255 �łȂ���΂Ȃ�Ȃ�
			_RPTF0(_CRT_ASSERT, "NxFilterBlt.uOpacity �̒l�͔͈͊O�ł�.\n");
			return FALSE;
		}
	}

	typedef BOOL (CNxCustomDraw::*FilterProc)(CNxSurface* pDestSurface, const RECT* lpDestRect,
											  const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
											  const NxFilterBlt* pNxFilterBlt) const;
	
	static const FilterProc pfnFilter[] =
	{
		&CNxCustomDraw::Filter_Grayscale,		// �O���C�X�P�[����
		&CNxCustomDraw::Filter_HueTransform,		// �F���ϊ�
		&CNxCustomDraw::Filter_RGBColorBalance,	// RGB �J���[�o�����X
		&CNxCustomDraw::Filter_Negative,			// �l�K���]
	};
	return (m_customDraw.*pfnFilter[pNxFilterBlt->dwFlags & NxFilterBlt::operationMask])
		(this, &rcDest, pSrcSurface, &rcSrc, pNxFilterBlt);
}

//////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::UpdateDIBColorTable()
// �T�v: DIBSection �̃J���[�e�[�u�����X�V
// ����: �Ȃ�
// �ߒl: �����Ȃ�� TRUE
//////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::UpdateDIBColorTable()
{
	_ASSERTE(m_hBitmap != NULL);

	// HDC �擾
	HDC hDC = GetDC();
	if (hDC == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::UpdateDIBColorTable() : HDC �̎擾�Ɏ��s���܂���.\n");
		return FALSE;
	}
	BOOL bResult = ::SetDIBColorTable(hDC, 0, m_dibImage.GetColorCount(), m_dibImage.GetColorTable()) != 0;
	// HDC �J��
	ReleaseDC();
	return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::LoadImage(int dx, int dy, CNxFile& nxfile, const RECT* lpSrcRect = NULL,
//							   BOOL bUpdateColorTable = FALSE)
// �T�v: �C���[�W���w���`�֓ǂݍ���
// ����: int dx                 ... �]���� X ���W
//       int dy                 ... �]���� Y ���W
//       CNxFile &nxfile        ... �ǂݍ��݌��t�@�C��
//		 const RECT* lpSrcRect  ... �ǂݍ��܂ꂽ�C���[�W����]�������`(NULL �Ȃ�ΑS��)
//		 BOOL bUpdateColorTable ... TRUE �Ȃ�΃J���[�e�[�u�����X�V(8bpp �`���̂�)
// �ߒl: �����Ȃ�� TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::LoadImage(int dx, int dy, CNxFile& nxfile, const RECT* lpSrcRect, BOOL bUpdateColorTable)
{
	_ASSERTE(m_hBitmap != NULL);

	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::LoadImage() : �I�[�v������Ă��Ȃ� CNxFile �I�u�W�F�N�g���n����܂���.\n");
		return FALSE;
	}
	
	// �摜�̓ǂݍ���
	std::auto_ptr<CNxDIBImage> dibImage(CNxDraw::GetInstance()->LoadImage(nxfile));
	if (dibImage.get() == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::LoadImage() : �摜�̓W�J�Ɏ��s���܂���.\n");
		return FALSE;
	}

	// DIB ��ݒ�
	if (!SetDIBits(dx, dy, dibImage.get(), lpSrcRect))
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::LoadImage() : �Ή����Ă��Ȃ� DIB �ł�.\n");
		return FALSE;
	}

	// �J���[�e�[�u���̐ݒ�
	if (bUpdateColorTable)
		SetColorTable(dibImage.get());

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FillRect(const RECT* lpRect, NxColor nxColor)
// �T�v: �T�[�t�F�X��h��Ԃ�(�u�����h�����̏����ȏ㏑��)
// ����: const RECT* lpRect ... �h��Ԃ���` (NULL �Ȃ�ΑS��)
//       NxColor nxColor    ... �h��Ԃ��F
// �ߒl: �����Ȃ�� TRUE
// ���l: NxColor �ł��̂܂܃T�[�t�F�X��h��ׂ��B8bpp �̏ꍇ�� A(��) �v�f�݂̂��g�p
////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FillRect(const RECT* lpRect, NxColor nxColor)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rect;
	SetAbbreviateRect(&rect, lpRect);

	NxBlt nxb;
	nxb.nxbColor = nxColor;

	return m_customDraw.Blt_ColorFill_Normal(this, &rect, NULL, NULL, &nxb);
}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::FillRect(int dx, int dy, int cx, int cy, NxColor nxcrColor)
// �T�v: �T�[�t�F�X��h��Ԃ�
// ����: int dx            ... �h��ׂ��J�nX���W
//       int dy            ... �h��ׂ��J�nY���W
//       int cx            ... �h��ׂ��̈�̕�
//       int cy            ... �h��ׂ��̈�̍���
//       NxColor nxcrColor ... �h��Ԃ��F
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::FillRect(int dx, int dy, int cx, int cy, NxColor nxcrColor)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + cx;
	rcDest.bottom = dy + cy;
	return FillRect(&rcDest, nxcrColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	CNxFont* CNxSurface::SetFont(CNxFont* pFont)
// �T�v: �T�[�t�F�X�`��֗p����t�H���g��ݒ�
// ����: CNxFont* ... CNxFont �I�u�W�F�N�g�ւ̃|�C���^
// �ߒl: �����Ȃ�ΈȑO�ɑI������Ă��� CNxFont
//////////////////////////////////////////////////////////////////////////////////////////////////////////

CNxFont* CNxSurface::SetFont(CNxFont* pFont)
{
	_ASSERTE(m_hBitmap != NULL);
	std::swap(m_pFont, pFont);
	return pFont;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SetTextSmoothing(BOOL bSmoothing)
// �T�v: �e�L�X�g�X���[�W���O�`��̗L����ݒ�
// ����: BOOL bSmoothing ... TRUE �Ȃ�΃X���[�W���O�`����s��
// �ߒl: �ȑO�̏��
// ���l: �f�t�H���g�̓X���[�W���O�`�悵�Ȃ�
///////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetTextSmoothing(BOOL bSmoothing)
{
	_ASSERTE(m_hBitmap != NULL);
	std::swap(m_bTextSmoothing, bSmoothing);
	return bSmoothing;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::GetTextExtent(LPCTSTR lpszString, LPRECT lpRect)
// �T�v: �e�L�X�g�̕`���`���擾
// ����: LPCTSTR lpszString ... ������ւ̃|�C���^
//       LPRECT  lpRect     ... ��`�𓾂� RECT �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::GetTextExtent(LPCTSTR lpszString, LPRECT lpRect)
{
	_ASSERTE(m_hBitmap != NULL);

	_ASSERTE(lpszString != NULL);
	_ASSERTE(lpRect != NULL);
	
	if (m_pFont == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::GetTextExtent() : �t�H���g���w�肳��Ă��܂���.\n");
		return FALSE;
	}

	// DC �擾
	HDC hDC = GetDC();
	if (hDC == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::GetTextExtent() : DC �̎擾�Ɏ��s���܂���.\n");
		return FALSE;		// DC �擾���s
	}

	// �t�H���g���擾/�I��
	HFONT hFont = m_pFont->GetHandleInternal((m_bTextSmoothing) ? CNxFont::FontType_Smooth : CNxFont::FontType_Normal);
	if (hFont == NULL)
	{
		_RPTF0(_CRT_ERROR, "CNxSurface::GetTextExtent() : CNxFont::GetHandle() �����s���܂���.\n");
		ReleaseDC();
		return FALSE;
	}
	HFONT hFontOld = static_cast<HFONT>(::SelectObject(hDC, hFont));
	
	// �e�L�X�g�̋�`�𓾂�
	// ���̊֐��œ�����T�C�Y�́A��]��C�^���b�N�A�{�[���h�͍l������Ȃ�(�炵��)
	SIZE size;
	if (!::GetTextExtentPoint32(hDC, lpszString, _tcslen(lpszString), &size))
	{
		_RPTF0(_CRT_ERROR, "CNxSurface::GetTextExtent() : GetTextExtentPoint32() �֐������s���܂���.\n");
		::SelectObject(hDC, hFontOld);
		ReleaseDC();
		return FALSE;
	}

	// TEXTMETRIC ���擾
	TEXTMETRIC tm;
	::GetTextMetrics(hDC, &tm);

	// DC �J��
	::SelectObject(hDC, hFontOld);
	ReleaseDC();

	// �C�^���b�N�̂̎��́A�΂߂ɂȂ������𕝂։��Z
	if (tm.tmItalic)
	{
		size.cx += tm.tmAscent + tm.tmDescent;
	}

	// �X���[�W���O���s���Ȃ���T�C�Y�� 1/CNxSurface::SmoothFontRatio �ɂ���
	if (m_bTextSmoothing)
	{
		// CNxSurface::SmoothFontRatio �P�ʂ֐؂�グ
		const UINT ratio = CNxSurface::SmoothFontRatio;
		// unsigned �Ōv�Z
		UINT cx = size.cx;
		UINT cy = size.cy;
		cx = ((cx + ratio - 1) / ratio);
		cy = ((cy + ratio - 1) / ratio);
		size.cx = cx;
		size.cy = cy;
	}
	
	// ��]���̕ϊ�
	// �g���g�� source ver.0.80 by W.Dee �� (http://www.din.or.jp/~glit/TheOddStage/TVP)
	// Projects\TVP32\OperatorsUnit.cpp ���Q�l�ɂ��܂���

	// LOGFONT �����]�p�x���擾
	LONG lEscapement = m_pFont->GetEscapement();

	// �p�x�� 0 �` 3599 �֐��K��
	lEscapement %= 360 * 10;
	if (lEscapement < 0)
		lEscapement += 3600;

	if (lEscapement == 0)
	{	// ������
		lpRect->left = 0;
		lpRect->top = 0;
		lpRect->right = size.cx;
		lpRect->bottom = size.cy;
	}
	else if (lEscapement == 270 * 10)
	{	// �c����
		lpRect->left = -size.cy;
		lpRect->top = 0;
		lpRect->right = 0;
		lpRect->bottom = size.cx;
	}
	else
	{
		const double fPI = 3.1415926535897932;
		double fDir = static_cast<double>(lEscapement) * fPI / 180.0f / 10.0f;
		double fSin = sin(fDir);
		double fCos = cos(fDir);

		if (lEscapement >= 0 && lEscapement < 90 * 10)
		{	// 0 - 90
			lpRect->left = 0;
			lpRect->top = static_cast<int>(-fSin * size.cx);
			lpRect->right = static_cast<int>(fSin * size.cy + fCos * size.cx);
			lpRect->bottom = static_cast<int>(fCos * size.cy);
		}
		else if (lEscapement >= 90 * 10 && lEscapement < 180 * 10)
		{	// 90 - 180
			lpRect->left = static_cast<int>(fCos * size.cx);
			lpRect->top = static_cast<int>(fCos * size.cy - fSin * size.cx);
			lpRect->right = static_cast<int>(fSin * size.cy);
			lpRect->bottom = 0;
		}
		else if (lEscapement >= 180 * 10 && lEscapement < 270 * 10)
		{	// 180 - 270
			lpRect->left = static_cast<int>(fCos * size.cx + fSin * size.cy);
			lpRect->top = static_cast<int>(fCos * size.cy);
			lpRect->right = 0;
			lpRect->bottom = static_cast<int>(-fSin * size.cx);
		}
		else
		{	// 270 - 360
			lpRect->left = static_cast<int>(fSin * size.cy);
			lpRect->top = 0;
			lpRect->right = static_cast<int>(fCos * size.cx);
			lpRect->bottom = static_cast<int>(fCos * size.cy - fSin * size.cx);
		}
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, const NxBlt *pNxBlt)
// �T�v: �e�L�X�g��`��
// ����: int dx              ... �`����J�n���� X ���W
//       iny dy              ... �`����J�n���� Y ���W
//       const RECT* lpRect  ... �N���b�s���O��`(NULL �Ȃ�΃N���b�v���Ȃ�)
//		 LPCTSTR lpszString  ... ������
//       const NxBlt *pNxBlt ... �����o�b�t�@�T�[�t�F�X(8bpp)����̓]�����@���w�肷�� NxBlt �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, const NxBlt *pNxBlt)
{
	_ASSERTE(m_hBitmap != NULL);

	if (m_pFont == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxSurface::DrawText() : �t�H���g���w�肳��Ă��܂���.\n");
		return FALSE;
	}
	
	_ASSERTE(lpszString != NULL);
	
	// �`�悵���Ƃ��̃T�C�Y���v�Z
	RECT rcText;
	if (!GetTextExtent(lpszString, &rcText))
	{	// �G���[����
		return FALSE;
	}

	UINT uTextWidth = static_cast<UINT>(rcText.right - rcText.left);
	UINT uTextHeight = static_cast<UINT>(rcText.bottom - rcText.top);

	if ((uTextWidth - 1 | uTextHeight - 1) + 1 == 0)
	{	// �e�L�X�g�̉������͍������[��
		return TRUE;
	}

	RECT rcDest;
	rcDest.top = dy;
	rcDest.left = dx;
	rcDest.right = rcDest.left + uTextWidth;
	rcDest.bottom = rcDest.top + uTextHeight;

	RECT rcSrc;
	rcSrc.top = 0;
	rcSrc.left = 0;
	rcSrc.right = uTextWidth;
	rcSrc.bottom  = uTextHeight;

	// �N���b�s���O��`���w�肳��Ă���Ȃ�΁A�N���b�v���Ă݂�
	if (lpRect != NULL)
	{
		if (!NxDrawLocal::ClipRect(&rcDest, &rcSrc, lpRect, NULL))
		{	// �`�悷�ׂ���`���Ȃ�
			return TRUE;
		}
	}

	// �ꎞ�T�[�t�F�X�֗v�����镝�ƍ���������
	UINT uSurfaceWidth = uTextWidth;
	UINT uSurfaceHeight = uTextHeight;
	if (m_bTextSmoothing)
	{	// �X���[�W���O����
		uSurfaceWidth *= 4;
		uSurfaceHeight *= 4;
	}

	// �ꎞ�T�[�t�F�X�擾
	CNxSurface* pGrayscaleSurface = pGrayscaleSurface = CNxDraw::GetInstance()->GetTextTemporarySurface(uSurfaceWidth, uSurfaceHeight);
	if (pGrayscaleSurface == NULL)
		return FALSE;

	// �ꎞ�T�[�t�F�X�� HDC ���擾
	HDC hDC = pGrayscaleSurface->GetDC();
	if (hDC == NULL)
		return FALSE;

	// pGrayscaleSurface �� DC �� ReleaseDC() �ŕK���J�������
	// �͂��Ȃ̂ŁA��Ԃ̕��A�͍s�Ȃ��Ă��Ȃ�

	if (::SelectObject(hDC, m_pFont->GetHandleInternal((m_bTextSmoothing) ? CNxFont::FontType_Smooth : CNxFont::FontType_Normal)) == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface:DrawText() : �t�H���g�̑I���Ɏ��s���܂���.\n");
		pGrayscaleSurface->ReleaseDC();
		return FALSE;
	}

	// �w�i�F�̎w��
	::SetBkColor(hDC, RGB(0, 0, 0));

	if (m_bTextSmoothing)
	{	// �X���[�W���O����ꍇ...
		// 4�{�p�̕�����`�悵�Ă���A�⊮�k��
		rcText.left *= CNxSurface::SmoothFontRatio;
		rcText.top *= CNxSurface::SmoothFontRatio;

		// �e�L�X�g�N���b�v��`�̏���
		RECT rcTextClip;
		rcTextClip.left = rcSrc.left;
		rcTextClip.top = rcSrc.top;
		rcTextClip.right = rcSrc.left + uSurfaceWidth;
		rcTextClip.bottom = rcSrc.bottom + uSurfaceHeight;

		::SetTextColor(hDC, RGB(15, 15, 15));
		::ExtTextOut(hDC, -rcText.left, -rcText.top, ETO_OPAQUE, &rcTextClip, lpszString, _tcslen(lpszString), NULL);
		pGrayscaleSurface->ReleaseDC();
		::GdiFlush();
		FontSmoothing4x4(pGrayscaleSurface->GetBits(), pGrayscaleSurface->GetPitch(), uTextWidth, uTextHeight);
	}
	else
	{	// �X���[�W���O���Ȃ�
		::SetTextColor(hDC, RGB(255, 255, 255));
		::ExtTextOut(hDC, -rcText.left, -rcText.top, ETO_OPAQUE, &rcSrc, lpszString, _tcslen(lpszString), NULL);
		pGrayscaleSurface->ReleaseDC();
		::GdiFlush();
	}

	// �o��
	BOOL bResult = Blt(&rcDest, pGrayscaleSurface, &rcSrc, pNxBlt);
	return bResult;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, NxColor nxColor)
// �T�v: �e�L�X�g��`��
// ����: int dx             ... �`����J�n���� X ���W
//       int dy             ... �`����J�n���� Y ���W
//       const RECT* lpRect ... �N���b�v��`(NULLL �Ȃ�΃T�[�t�F�X�S��)
//		 LPCTSTR lpszString ... ������
//       NxColor nxColor    ... �`��F(�A���t�@���L��)
// �ߒl: �����Ȃ�� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, NxColor nxColor)
{
	_ASSERTE(m_hBitmap != NULL);
	_ASSERTE(lpszString != NULL);

	// ���ʂ�ʏ�u�����h�œ]��
	NxBlt nxbf;
	nxbf.dwFlags = NxBlt::blendNormal | NxBlt::colorFill | NxBlt::destAlpha | NxBlt::srcAlpha;
	nxbf.nxbColor = nxColor;
	return DrawText(dx, dy, lpRect, lpszString, &nxbf);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SetDIBits(int dx, int dy, const CNxDIBImage* pDIBImage, LPCVOID lpvBits, const RECT* lpSrcRect = NULL)
// �T�v: DIB ��(�K�v�Ȃ�Εϊ�����) �T�[�t�F�X�֓]��
// ����: int dx						  ... �]���� X ���W
//       int dy						  ... �]���� Y ���W
//       const CNxDIBImage* pDIBImage ... �]���� CNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^
//		 const RECT* lpSrcRect		  ... �]����`(NULL �Ȃ�ΑS��)
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetDIBits(int dx, int dy, const CNxDIBImage* pDIBImage, const RECT* lpSrcRect)
{
	_ASSERTE(m_hBitmap != NULL);

	RECT rcSrc;
	if (lpSrcRect == NULL)
		pDIBImage->GetRect(&rcSrc);
	else
		rcSrc = *lpSrcRect;

	RECT rcDest;
	rcDest.left = dx;
	rcDest.top = dy;
	rcDest.right = dx + (rcSrc.right - rcSrc.left);
	rcDest.bottom = dy + (rcSrc.bottom - rcSrc.top);

	// �N���b�v
	RECT rcSrcClip;
	pDIBImage->GetRect(&rcSrcClip);
	
	if (!ClipBltRect(rcDest, rcSrc, rcSrcClip))
		return TRUE;

	return m_dibImage.Blt(dx, dy, pDIBImage, lpSrcRect);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SetDIBits(int dx, int dy, const BITMAPINFO* lpbmi, LPCVOID lpvBits, const RECT* lpSrcRect = NULL)
// �T�v: DIB ��(�K�v�Ȃ�Εϊ�����) �T�[�t�F�X�֓]��
// ����: int dx                  ... �]���� X ���W
//       int dy                  ... �]���� Y ���W
//       const BITMAPINFO *lpbmi ... �]���� DIB
//       LPCVOID lpvBits         ... �]���� DIB �̃r�b�g�f�[�^�ւ̃|�C���^(NULL �Ȃ�΁Alpbmi ����v�Z)
//		 const RECT* lpSrcRect   ... �]����`(NULL �Ȃ�ΑS��)
// �ߒl: �����Ȃ� TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetDIBits(int dx, int dy, const BITMAPINFO* lpbmi, LPCVOID lpvBits, const RECT* lpSrcRect)
{
	_ASSERTE(m_hBitmap != NULL);

	CNxDIBImage srcDIBImage;
	if (!srcDIBImage.Create(const_cast<LPBITMAPINFO>(lpbmi), const_cast<LPVOID>(lpvBits)))
		return FALSE;
	else
		return SetDIBits(dx, dy, &srcDIBImage, lpSrcRect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::GetDIBits(int dx, int dy, LPBITMAPINFO lpbmi, LPVOID lpvBits, const RECT* lpSrcRect = NULL) const
// �T�v: �T�[�t�F�X�̓��e�� DIB �Ƃ��Ď擾
// ����: int dx                ... DIB ��̓]���� X ���W
//       int dy                ... DIB ��̓]���� Y ���W
//       LPBITMAPINFO lpbmi    ... �]���� DIB �̏������� BITMAPINFO �\���̂ւ̃|�C���^
//       LPVOID lpvBits        ... �r�b�g�f�[�^���擾����o�b�t�@�ւ̃|�C���^(NULL ��)
//       const RECT* lpSrcRect ... DIB �֓]������T�[�t�F�X����`������ RECT �\���̂ւ̃|�C���^(NULL = �S��)
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::GetDIBits(int dx, int dy, LPBITMAPINFO lpbmi, LPVOID lpvBits, const RECT* lpSrcRect) const
{
	_ASSERTE(m_hBitmap != NULL);
	_ASSERTE(lpbmi != NULL);

	if (lpbmi->bmiHeader.biSize < sizeof(BITMAPINFOHEADER) ||
		lpbmi->bmiHeader.biPlanes != 1 ||
		lpbmi->bmiHeader.biCompression != BI_RGB)
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::GetDIBits() : BITMAPINFO �\���̂̓��e���ُ�ł�.\n");
		return FALSE;
	}
		
	// �]�����N���b�v��`(���̃T�[�t�F�X�S�̂�������`)���擾
	RECT rcSrcClip;
	GetRect(&rcSrcClip);

	RECT rcSrc;

	// lpSrcRect == NULL �Ȃ�΁A�T�[�t�F�X�S�̂�����
	if (lpSrcRect == NULL)
		GetRect(&rcSrc);
	else
		rcSrc = *lpSrcRect;


	// �]����N���b�v��`������
	RECT rcDestClip;
	rcDestClip.left = 0;
	rcDestClip.top = 0;
	rcDestClip.right = lpbmi->bmiHeader.biWidth;
	rcDestClip.bottom = abs(lpbmi->bmiHeader.biHeight);

	RECT rcDest = rcDestClip;
	::OffsetRect(&rcDest, dx, dy);

	// BITMAPINFO �\���̂̐ݒ�
	LONG lDIBPitch = (((((lpbmi->bmiHeader.biBitCount * lpbmi->bmiHeader.biWidth) + 7) / 8) + 3) / 4) * 4;
	lpbmi->bmiHeader.biSizeImage = lDIBPitch * abs(lpbmi->bmiHeader.biHeight);
	lpbmi->bmiHeader.biXPelsPerMeter = 0;
	lpbmi->bmiHeader.biYPelsPerMeter = 0;
	lpbmi->bmiHeader.biClrUsed = 0;
	lpbmi->bmiHeader.biClrImportant = 0;

	// �N���b�v����
	if (!NxDrawLocal::ClipRect(&rcDest, &rcSrc, &rcDestClip, &rcSrcClip))
	{	// �]����`����
		return TRUE;
	}
	if (lpvBits == NULL)
		return TRUE;	// �r�b�g�f�[�^�͎擾���Ȃ�

	CNxDIBImage destDIBImage;
	if (!destDIBImage.Create(lpbmi, lpvBits))
		return FALSE;

	return destDIBImage.Blt(rcDest.left, rcDest.top, &GetDIBImage(), &rcSrc);
}

//////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSurface::SetColorTable(const CNxDIBImage* pDIBImage)
// �T�v: CNxDIBImage �I�u�W�F�N�g�̃p���b�g����J���[�e�[�u����ݒ�
// ����: const CNxDIBImage* pDIBImage ... �ݒ茳 CNxDIBImage �I�u�W�F�N�g�ւ̃|�C���^
// �ߒl: �����Ȃ� TRUE
//////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SetColorTable(const CNxDIBImage* pDIBImage)
{
	if (GetBitCount() > 8)
		return FALSE;		// �T�[�t�F�X�� 8bpp �ł͂Ȃ�

	if (pDIBImage->GetBitCount() > 8)
		return FALSE;		// DIB ���p���b�g�t���ł͂Ȃ�

	memcpy(m_dibImage.GetColorTable(), pDIBImage->GetColorTable(), pDIBImage->GetColorCount() * sizeof(RGBQUAD));
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SaveBitmapFile(LPCTSTR lpszFileName, const RECT* lpRect = NULL) const
// �T�v: �T�[�t�F�X�̓��e���r�b�g�}�b�v�t�@�C���Ƃ��ĕۑ�����
// ����: LPCTSTR lspzFileName ... �ۑ��t�@�C����
//       const RECT* lpRect   ... �ۑ������`(NULL �Ȃ�ΑS��)
// �ߒl: �����Ȃ� TRUE
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SaveImage(LPCTSTR lpszFileName, const RECT* lpRect) const
{
	_ASSERTE(m_hBitmap != NULL);
	_ASSERTE(lpszFileName != NULL);

	CNxFile nxfile;
	if (!nxfile.Open(lpszFileName, CNxFile::modeCreate|CNxFile::modeWrite))
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::SaveBitmapFile() : �t�@�C���̍쐬�Ɏ��s���܂���.\n");
		return FALSE;
	}
	return SaveImage(nxfile, lpRect);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::SaveBitmapFile(CNxFile& nxfile, const RECT* lpRect = NULL) const
// �T�v: �T�[�t�F�X�̓��e���r�b�g�}�b�v�t�@�C���Ƃ��ĕۑ�����
// ����: CNxFile& nxfile ... �������݉\�ȃt�@�C���ւ̎Q��
//       const RECT* lpRect ... �ۑ������`(NULL �Ȃ�ΑS��)
// �ߒl: �����Ȃ� TRUE/ ���s�Ȃ� FALSE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::SaveImage(CNxFile& nxfile, const RECT* lpRect) const
{
	_ASSERTE(m_hBitmap != NULL);

	if (!nxfile.IsOpen())
	{
		_RPTF0(_CRT_ASSERT, "CNxSurface::SaveBitmapFile() : �t�@�C���͊J����Ă��܂���.\n");
		return FALSE;
	}

	// 24bpp ���� 8bpp �� BMP �Ƃ��ĕۑ�
	CNxBMPImageSaver saver(CNxBMPImageSaver::stripAlpha);
	return saver.SaveDIBImage(nxfile, GetDIBImage(), lpRect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	static void CNxSurface::FontSmoothing4x4(LPBYTE lpSurface, LONG lPitch,
//											 UINT uWidth, UINT uHeight)
// �T�v: �X���[�W���O�`��p�������[�`�� (4x4 -> 1x1)
// ����: LPVOID lpSurface     ... ���f�[�^�y�яk����̃f�[�^���u�����T�[�t�F�X�ւ̃|�C���^
//       LONG   lPitch        ... �T�[�t�F�X�̑��蕝(pSurface->GetPitch() �̒l)
//       LONG   lSrcDistance  ... �]����(�k���O)�̍Ō�̃s�N�Z�����玟�̍s�ւ̋���
//       UINT   uWidth        ... �k����̕�(���T�C�Y�� 1/4)
//       UINT   uHeight       ... �k����̍���(���T�C�Y�� 1/4)
// �ڍ�:
//       ���̃s�N�Z���l�� 15(0x0f) �ŕ`��B���ڂ��Ă��鍶�����Ƃ��ďc�� 4 dot (���v 16 dot) ��P���ɉ��Z�B
//       ���v���ő�ł� 15 * 16 = 0xf8 �ƂȂ��Ă��܂����߁A5 - 3bit �� 2 - 0bit �փR�s�[���� 255 �ɍ��킹��B
//       ���ʂ́A0 - 255 �Ńs�N�Z���̕s�����x�������B
//       ���f�[�^�͔j�󂳂��B
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  __declspec(naked) CNxSurface::FontSmoothing4x4(LPVOID /*lpSurface*/, LONG /*lPitch*/,
													 UINT /*uWidth*/, UINT /*uHeight*/)
{
#pragma pack(push, 4)
	struct StackFrame
	{
		DWORD	esi;
		DWORD	ebp;
		DWORD	ebx;
		DWORD	edi;
		DWORD	eip;
		LPVOID	lpSurface;
		LONG	lPitch;
		UINT	uWidth;
		UINT	uHeight;
	};
#pragma pack(pop)

	__asm
	{
		push	edi
		push	ebx
		push	ebp
		push	esi
		mov		eax, [esp]StackFrame.uWidth
		mov		esi, [esp]StackFrame.lPitch
		mov		ebx, [esp]StackFrame.uHeight
		shl		eax, 2
		mov		edi, [esp]StackFrame.lpSurface
		sub		esi, eax
		mov		ebp, edi

loop_y:
		
		mov		edx, [esp]StackFrame.uWidth
		mov		ecx, edx

		// y mod 4 = 0
loop_x_base:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		mov		[ebp], al
		inc		ebp
		loop	loop_x_base

		add		edi, esi
		sub		ebp, edx
		mov		ecx, edx

		// y mod 4 = 1
loop_x_add1:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		add		[ebp], al
		inc		ebp
		loop	loop_x_add1

		add		edi, esi
		sub		ebp, edx
		mov		ecx, edx

		// y mod 4 = 2
	loop_x_add2:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		add		[ebp], al
		inc		ebp
		loop	loop_x_add2

		add		edi, esi
		sub		ebp, edx
		mov		ecx, edx

		// y mod 4 = 3
	loop_x_add3:

		mov		eax, [edi]
		shr		eax, 16
		add		eax, [edi]
		add		edi, 4
		add		al, ah
		add		al, [ebp]
		mov		ah, al
		shr		al, 3
		and		al, 7
		or		al, ah
		mov		[ebp], al
		inc		ebp
		loop	loop_x_add3

		add		ebp, [esp]StackFrame.lPitch
		add		edi, esi
		sub		ebp, [esp]StackFrame.uWidth
		dec		ebx
		jnz		loop_y

		pop		esi
		pop		ebp
		pop		ebx
		pop		edi
		ret
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	 const RECT* CNxSurface::SetAbbreviateRect(LPRECT lpRect, const RECT* lpSrcRect) const
// �T�v: this �� NULL ���AlpSrcRect �� NULL �ł���΁A��̋�`��ݒ肷��
//       lpSrcRect �� NULL �Ȃ�΁A�T�[�t�F�X�S�̂�������`��ݒ肵�ĕԂ�
//       ����ȊO�́AlpSrcRect �����̂܂ܐݒ肷��
// ����: LPRECT lpRect         ... ���ʂ��󂯂Ƃ� RECT �\���̂ւ̃|�C���^
//       const RECT *lpSrcRect ... ���͋�`
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////////////////////////////

void CNxSurface::SetAbbreviateRect(LPRECT lpRect, const RECT* lpSrcRect) const
{
	if (lpSrcRect != NULL)
		*lpRect = *lpSrcRect;			// lpSrcRect != NULL
	else
	{
		if (this == NULL)
			::SetRectEmpty(lpRect);		// lpSrcRect == NULL && this == NULL
		else
			GetRect(lpRect);			// lpSrcRect == NULL && this != NULL
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxSurface::SetClipRect(const RECT* lpClipRect)
// �T�v: �`��N���b�s���O��`��ݒ�
// ����: const RECT* lpClipRect ... �`��\�ȋ�`�BNULL �Ȃ�΃T�[�t�F�X�S��
// �ߒl: �Ȃ�
///////////////////////////////////////////////////////////////////////////////////////

void CNxSurface::SetClipRect(const RECT* lpClipRect)
{
	_ASSERTE(m_hBitmap != NULL);

	if (lpClipRect == NULL)
	{
		GetRect(&m_rcClip);
	}
	else
	{
		m_rcClip.left = max(lpClipRect->left, 0);
		m_rcClip.right = min(lpClipRect->right, static_cast<int>(GetWidth()));
		m_rcClip.top = max(lpClipRect->top, 0);
		m_rcClip.bottom = min(lpClipRect->bottom, static_cast<int>(GetHeight()));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxSurface::ClipBltRect(LPRECT lpDestRect) const
//
// �T�v: �T�[�t�F�X�̃N���b�s���O���Ɋ�Â��A�]����݂̂̃N���b�v
// ����: LPRECT lpDestRect ... ���͋�`
// �ߒl: �]����`������� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::ClipBltRect(RECT& destRect) const
{
	destRect.top += m_ptOrg.y;
	if (destRect.top < m_rcClip.top)
		destRect.top = m_rcClip.top;

	destRect.bottom += m_ptOrg.y;
	if (destRect.bottom > m_rcClip.bottom)
		destRect.bottom = m_rcClip.bottom;

	if (destRect.top >= destRect.bottom)
		return FALSE;

	destRect.left += m_ptOrg.x;
	if (destRect.left < m_rcClip.left)
		destRect.left = m_rcClip.left;

	destRect.right += m_ptOrg.x;
	if (destRect.right > m_rcClip.right)
		destRect.right = m_rcClip.right;

	if (destRect.left >= destRect.right)
		return FALSE;
	else
		return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::ClipBltRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpSrcClipRect) const
// �T�v: �T�[�t�F�X�̃N���b�s���O���Ɋ�Â��A�]�����Ɠ]�����`�̃N���b�v (Blt �֐��p)
// ����: LPRECT lpDestRect         ... �]�����`
//       LPRECT lpSrcRect          ... �]������`
//       const RECT* lpSrcClipRect ... �]�����N���b�s���O��`
// �ߒl: �]����`������� TRUE
// ���l: �㉺���E���]�ɑΉ�
//       left > right �̏ꍇ�͍��E���]�B�N���b�v��́A
//		 left ������������l�� right > left �ɏC�������(�㉺�����l)�B
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::ClipBltRect(RECT& destRect, RECT& srcRect, const RECT& srcClipRect) const
{
	destRect.left += m_ptOrg.x;
	destRect.right += m_ptOrg.x;
	destRect.top += m_ptOrg.y;
	destRect.bottom += m_ptOrg.y;


	if (srcRect.top > srcRect.bottom)
	{	// �㉺���] (top > bottom)
		if (srcRect.bottom < srcClipRect.top)
		{
			destRect.top += srcClipRect.top - srcRect.bottom;
			srcRect.bottom = srcClipRect.top;
		}
		if (destRect.top < m_rcClip.top)
		{
			srcRect.top -= m_rcClip.top - destRect.top;
			destRect.top = m_rcClip.top;
		}
		if (srcClipRect.bottom - srcRect.top < 0)
		{
			destRect.bottom += srcClipRect.bottom - srcRect.top;
			srcRect.top = srcClipRect.bottom;
		}
		if (m_rcClip.bottom - destRect.bottom < 0)
		{
			srcRect.bottom -= m_rcClip.bottom - destRect.bottom;
			destRect.bottom = m_rcClip.bottom;
		}
		std::swap(srcRect.top, srcRect.bottom);		// �㉺����ւ�(top < bottom �ɂ���)
	}
	else
	{	// �ʏ� (top < bottom)
		if (srcRect.top < srcClipRect.top)
		{
			destRect.top += srcClipRect.top - srcRect.top;
			srcRect.top = srcClipRect.top;
		}
		if (destRect.top < m_rcClip.top)
		{
			srcRect.top += m_rcClip.top - destRect.top;
			destRect.top = m_rcClip.top;
		}
		if (srcClipRect.bottom - srcRect.bottom < 0)
		{
			destRect.bottom += srcClipRect.bottom - srcRect.bottom;
			srcRect.bottom = srcClipRect.bottom;
		}
		if (m_rcClip.bottom - destRect.bottom < 0)
		{
			srcRect.bottom += m_rcClip.bottom - destRect.bottom;
			destRect.bottom = m_rcClip.bottom;
		}
	}
	if (destRect.top >= destRect.bottom)
		return FALSE;
		
	if (srcRect.left > srcRect.right)
	{	// ���E���] (left > right)
		if (srcRect.right < srcClipRect.left)
		{
			destRect.left += srcClipRect.left - srcRect.right;
			srcRect.right = srcClipRect.left;
		}
		if (destRect.left < m_rcClip.left)
		{
			srcRect.left -= m_rcClip.left - destRect.left;
			destRect.left = m_rcClip.left;
		}
		if (srcClipRect.right - srcRect.left < 0)
		{
			destRect.right += srcClipRect.right - srcRect.left;
			srcRect.left = srcClipRect.right;
		}
		if (m_rcClip.right - destRect.right < 0)
		{
			srcRect.right -= m_rcClip.right - destRect.right;
			destRect.right = m_rcClip.right;
		}
		std::swap(srcRect.left, srcRect.right);		// ���E����ւ�
	}
	else
	{	// �ʏ� (left < right)
		if (srcRect.left < srcClipRect.left)
		{
			destRect.left += srcClipRect.left - srcRect.left;
			srcRect.left = srcClipRect.left;
		}
		if (destRect.left < m_rcClip.left)
		{
			srcRect.left += m_rcClip.left - destRect.left;
			destRect.left = m_rcClip.left;
		}
		if (srcClipRect.right - srcRect.right < 0)
		{
			destRect.right += srcClipRect.right - srcRect.right;
			srcRect.right = srcClipRect.right;
		}
		if (m_rcClip.right - destRect.right < 0)
		{
			srcRect.right += m_rcClip.right - destRect.right;
			destRect.right = m_rcClip.right;
		}
	}
	if (destRect.left >= destRect.right)
		return FALSE;
	else
		return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxSurface::ClipStretchBltRect(LPRECT lpDestRect, LPRECT lpSrcRect, const RECT* lpSrcClipRect,
//										StretchBltInfo* pStretchBltInfo) const
// �T�v: �T�[�t�F�X�̃N���b�s���O���Ɋ�Â��A�]�����Ɠ]�����`�̃N���b�v (�g��k�� Blt �p)
// ����: LPRECT lpDestRect				 ... �]�����`
//       LPRECT lpSrcRect				 ... �]������`
//       const RECT* lpSrcClipRect		 ... �]�����N���b�s���O��`
//		 StretchBltInfo* pStretchBltInfo ... �␳�����󂯂Ƃ� StretchBltInfo �\���̂ւ̃|�C���^
// �ߒl: �]����`������� TRUE
// ���l: ���]���Ή�
//////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxSurface::ClipStretchBltRect(RECT& destRect, RECT& srcRect, const RECT& srcClipRect,
									StretchBltInfo& stretchBltInfo) const
{
	UINT uSrcWidth = abs(srcRect.right - srcRect.left);
	UINT uSrcHeight = abs(srcRect.bottom - srcRect.top);

	UINT uDestWidth = (destRect.right - destRect.left);
	destRect.left += m_ptOrg.x;
	destRect.right += m_ptOrg.x;
	UINT uDestHeight = (destRect.bottom - destRect.top);
	destRect.top += m_ptOrg.y;
	destRect.bottom += m_ptOrg.y;

	// �]�����A�]����̍������͕����[���Ȃ�Γ]����`����(devide by zero ���)
	if ((uDestWidth - 1 | uSrcWidth  - 1 | uDestHeight - 1 | uSrcHeight - 1) + 1 == 0)
	{
		// ��ł� 0 ���������Ă���� uValue - 1 �ɂ���āA���ʂ� 0xffffffff
		// �������A�l�͈̔͂� 0 �` 0x7fffffff ���O��
		return FALSE;
	}


	ULARGE_INTEGER ul64Adjust;		// ���_�␳�p temporary �ϐ�

	stretchBltInfo.uSrcOrgY = 0;
	stretchBltInfo.ul64SrcDeltaY.LowPart = 0;
	stretchBltInfo.ul64SrcDeltaY.HighPart = uSrcHeight;
	stretchBltInfo.ul64SrcDeltaY.QuadPart /= uDestHeight;	// �]�������_�ψ�

	// ����
	if (srcRect.top > srcRect.bottom)
	{	// �㉺���]
		if (srcClipRect.bottom < srcRect.top)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.top - srcClipRect.bottom);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.top += ul64Adjust.HighPart;
			srcRect.top = srcClipRect.bottom;
		}
		if (m_rcClip.bottom < destRect.bottom)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.bottom - m_rcClip.bottom);
			srcRect.bottom += ul64Adjust.HighPart;
			destRect.bottom = m_rcClip.bottom;
		}
		if (srcClipRect.top > srcRect.bottom)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.top - srcRect.bottom);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.bottom -= ul64Adjust.HighPart + 1;
			srcRect.bottom = srcClipRect.top;
		}
		if (m_rcClip.top > destRect.top)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.top - destRect.top);
			srcRect.top -= ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgY = ul64Adjust.LowPart;
			destRect.top = m_rcClip.top;
		}
		std::swap(srcRect.top, srcRect.bottom);
	}
	else
	{	// �㉺���]���Ȃ�
		if (srcClipRect.top > srcRect.top)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.top - srcRect.top);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.top += ul64Adjust.HighPart;
			srcRect.top = srcClipRect.top;
		}
		if (m_rcClip.top > destRect.top)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.top - destRect.top);
			srcRect.top += ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgY = ul64Adjust.LowPart;
			destRect.top = m_rcClip.top;
		}
		if (srcClipRect.bottom < srcRect.bottom)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestHeight;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.bottom - srcClipRect.bottom);
			ul64Adjust.QuadPart /= uSrcHeight;
			destRect.bottom -= ul64Adjust.HighPart + 1;
			srcRect.bottom = srcClipRect.bottom;
		}
		if (m_rcClip.bottom < destRect.bottom)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaY;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.bottom - m_rcClip.bottom);
			srcRect.bottom -= ul64Adjust.HighPart;
			destRect.bottom = m_rcClip.bottom;
		}
	}
	if (destRect.top >= destRect.bottom)
		return FALSE;

	stretchBltInfo.uSrcOrgX = 0;
	stretchBltInfo.ul64SrcDeltaX.LowPart = 0;
	stretchBltInfo.ul64SrcDeltaX.HighPart = uSrcWidth;
	stretchBltInfo.ul64SrcDeltaX.QuadPart /= uDestWidth;

	// ����
	if (srcRect.right < srcRect.left)
	{	// ���E���]
		if (srcClipRect.right < srcRect.left)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.left - srcClipRect.right);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.left += ul64Adjust.HighPart;
			srcRect.left = srcClipRect.right;
		}
		if (m_rcClip.right < destRect.right)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.right - m_rcClip.right);
			srcRect.right += ul64Adjust.HighPart;
			destRect.right = m_rcClip.right;
		}
		if (srcClipRect.left > srcRect.right)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.left - srcRect.right);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.right -= ul64Adjust.HighPart + 1;
			srcRect.right = srcClipRect.left;
		}
		if (m_rcClip.left > destRect.left)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.left - destRect.left);
			srcRect.left -= ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgX = ul64Adjust.LowPart;
			destRect.left = m_rcClip.left;
		}
		std::swap(srcRect.left, srcRect.right);
	}
	else
	{	// ���E���]���Ȃ�
		if (srcClipRect.left > srcRect.left)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcClipRect.left - srcRect.left);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.left += ul64Adjust.HighPart;
			srcRect.left = srcClipRect.left;
		}
		if (m_rcClip.left > destRect.left)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(m_rcClip.left - destRect.left);
			srcRect.left += ul64Adjust.HighPart;
			stretchBltInfo.uSrcOrgX = ul64Adjust.LowPart;
			destRect.left = m_rcClip.left;
		}
		if (srcClipRect.right < srcRect.right)
		{
			ul64Adjust.LowPart = 0;
			ul64Adjust.HighPart = uDestWidth;
			ul64Adjust.QuadPart *= static_cast<DWORD>(srcRect.right - srcClipRect.right);
			ul64Adjust.QuadPart /= uSrcWidth;
			destRect.right -= ul64Adjust.HighPart + 1;
			srcRect.right = srcClipRect.right;
		}
		if (m_rcClip.right < destRect.right)
		{
			ul64Adjust = stretchBltInfo.ul64SrcDeltaX;
			ul64Adjust.QuadPart *= static_cast<DWORD>(destRect.right - m_rcClip.right);
			srcRect.right -= ul64Adjust.HighPart;
			destRect.right = m_rcClip.right;
		}
	}
	if (destRect.left >= destRect.right)
		return FALSE;
	else
		return TRUE;
}
