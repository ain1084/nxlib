// NxDIBImage.cpp: CNxDIBImage �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000,2001 S.Ainoguchi
//
// �T�v: DIB �������N���X
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxDIBImage.h"
#include "NxDrawLocal.h"

using namespace NxDrawLocal;

namespace
{
	class CNxHighColor
	{
	public:
		CNxHighColor(const CNxDIBImage& dibImage);
		CNxHighColor(DWORD dwRedMask, DWORD dwGreenMask, DWORD dwBlueMask);

		NxColor GetTrueColor(WORD wPixel) const;
		WORD GetHighColor(NxColor nxcolor) const;
		DWORD GetRedMask(void) const;
		DWORD GetGreenMask(void) const;
		DWORD GetBlueMask(void) const;

	private:
		class CNxColorMask  
		{
		public:
			CNxColorMask();
			CNxColorMask(DWORD dwMask);

			void SetColorMask(DWORD dwMask);
			BYTE GetColor(DWORD dwPixel) const;
			WORD GetPackedElement(BYTE byElement) const;
			BYTE GetRightBits() const;
			BYTE GetColorBits() const;
			DWORD GetMask() const;
		private:
			BYTE  m_byRightBits;
			BYTE  m_byColorBits;
			DWORD m_dwMask;
		} m_red, m_green, m_blue;
	};

	CNxHighColor::CNxColorMask::CNxColorMask(void)
		: m_byRightBits(0), m_byColorBits(0) {
	}

	CNxHighColor::CNxColorMask::CNxColorMask(DWORD dwMask) {
		SetColorMask(dwMask); }

	inline BYTE CNxHighColor::CNxColorMask::GetColor(DWORD dwPixel) const {
		dwPixel >>= m_byRightBits; dwPixel <<= (8 - m_byColorBits);
		return static_cast<BYTE>(dwPixel | ((dwPixel & 0xff) >> m_byColorBits)); }

	inline WORD CNxHighColor::CNxColorMask::GetPackedElement(BYTE byElement) const {
		byElement >>= (8 - m_byColorBits); return static_cast<WORD>(byElement << m_byRightBits); }

	inline BYTE CNxHighColor::CNxColorMask::GetRightBits() const {
		return m_byRightBits; }

	inline BYTE CNxHighColor::CNxColorMask::GetColorBits() const {
		return m_byColorBits; }

	inline DWORD CNxHighColor::CNxColorMask::GetMask() const {
		return m_dwMask; }

	void CNxHighColor::CNxColorMask::SetColorMask(DWORD dwMask)
	{
		// �}�X�N�̉E���̃r�b�g���𐔂���
		m_byRightBits = 0;
		while (!(dwMask & 1))
		{
			m_byRightBits++;
			dwMask >>= 1;
		}

		// �}�X�N�{�̂̃r�b�g���𐔂���
		m_byColorBits = 0;
		do
		{
			m_byColorBits++;
			dwMask >>= 1;
		} while (dwMask & 1);
	}

	CNxHighColor::CNxHighColor(DWORD dwRedMask, DWORD dwGreenMask, DWORD dwBlueMask)
	{
		m_red.SetColorMask(dwRedMask);
		m_green.SetColorMask(dwGreenMask);
		m_blue.SetColorMask(dwBlueMask);
	}

	CNxHighColor::CNxHighColor(const CNxDIBImage& dibImage)
	{
		DWORD dwRedMask;
		DWORD dwGreenMask;
		DWORD dwBlueMask;
		if (dibImage.GetInfoHeader()->biCompression == BI_BITFIELDS)
		{
			dwRedMask = *reinterpret_cast<const DWORD*>(dibImage.GetColorTable() + 0);
			dwGreenMask = *reinterpret_cast<const DWORD*>(dibImage.GetColorTable() + 1);
			dwBlueMask = *reinterpret_cast<const DWORD*>(dibImage.GetColorTable() + 2);
		}
		else
		{	// default �� 5-5-5
			dwRedMask = 0x00007c00;
			dwGreenMask = 0x000003e0;
			dwBlueMask = 0x0000001f;
		}
		m_red.SetColorMask(dwRedMask);
		m_green.SetColorMask(dwGreenMask);
		m_blue.SetColorMask(dwBlueMask);
	}

	inline DWORD CNxHighColor::GetRedMask(void) const {
		return m_red.GetMask(); }

	inline DWORD CNxHighColor::GetGreenMask(void) const {
		return m_green.GetMask(); }

	inline DWORD CNxHighColor::GetBlueMask(void) const {
		return m_blue.GetMask(); }

	inline NxColor CNxHighColor::GetTrueColor(WORD wPixel) const {
		return CNxColor(m_red.GetColor(wPixel), m_green.GetColor(wPixel), m_blue.GetColor(wPixel)); }

	inline WORD CNxHighColor::GetHighColor(NxColor nxcolor) const {
		CNxColor cr = nxcolor;
		return static_cast<WORD>(m_red.GetPackedElement(cr.GetRed())
								| m_green.GetPackedElement(cr.GetGreen())
								| m_blue.GetPackedElement(cr.GetBlue())); }

}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxDIBImage::CNxDIBImage()
// �T�v: CNxDIBImage �N���X�̃f�t�H���g�R���X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxDIBImage::CNxDIBImage()
 : m_lpbmi(NULL)				// BITMAPINFO �\���̂ւ̃|�C���^
 , m_lpvBits(NULL)				// �r�b�g�f�[�^�̍�����ւ̃|�C���^
 , m_lpvDIBits(NULL)			// DIB �r�b�g�f�[�^�̐擪�A�h���X
 , m_lPitch(0)					// ��s������̕�(bottom-up �Ȃ畉�̒l)
 , m_uColorCount(0)				// �J���[�e�[�u���̃T�C�Y(�o�C�g�P��)
 , m_bAutoDelete(FALSE)
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage::~CNxDIBImage()
// �T�v: CNxDIBImage �N���X�̃f�X�g���N�^
// ����: �Ȃ�
// �ߒl: �Ȃ�
//////////////////////////////////////////////////////////////////////

CNxDIBImage::~CNxDIBImage()
{
	if (m_bAutoDelete)
	{
		free(m_lpbmi);
	}
}

///////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDIBImage::Create(HGLOBAL hGlobal)
// �T�v: HGLOBAL ���� CNxDIBImage ���쐬
// ����: HGLOBAL hGlobal ... �O���[�o���������n���h��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::Create(HGLOBAL hGlobal)
{
	LPBITMAPINFO lpbmi = static_cast<LPBITMAPINFO>(::GlobalLock(hGlobal));
	if (lpbmi == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxDIBImage::Create() : �O���[�o���������� Lock �Ɏ��s���܂���.\n");
		return FALSE;
	}
	BOOL bResult = CNxDIBImage::Create(lpbmi);
	if (bResult)
	{	// �r�b�g�f�[�^���R�s�[
		memcpy(GetDIBits(), reinterpret_cast<LPBYTE>(lpbmi) + GetInfoSize(), GetImageSize());
	}
	::GlobalUnlock(hGlobal);
	return bResult;
}

///////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDIBImage::Create(int nWidth, int nHeight, UINT uBitCount)
// �T�v: �傫���ƃr�b�g�[�x���w�肵�āACNxDIBImage ���쐬
// ����: int nWidth     ... ��
//       int nHeight    ... ����
//       UINT uBitCount ... �r�b�g�[�x
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::Create(int nWidth, int nHeight, UINT uBitCount)
{
	UINT nColorTable;

	switch (uBitCount)
	{
	case 1:
	case 4:
	case 8:
		nColorTable = 1 << uBitCount;
		break;
	case 16:
	case 24:
	case 32:
		nColorTable = 0;
		break;
	default:
		_RPTF0(_CRT_ASSERT, "CNxDIBImage::Create() : �T�|�[�g����Ȃ��r�b�g�[�x�ł�.");
		nColorTable = 0;
	}

	LPBITMAPINFO lpbmi = static_cast<LPBITMAPINFO>(malloc(sizeof(BITMAPINFOHEADER) + nColorTable * sizeof(DWORD)));
	lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbmi->bmiHeader.biWidth = nWidth;
	lpbmi->bmiHeader.biHeight = nHeight;
	lpbmi->bmiHeader.biPlanes = 1;
	lpbmi->bmiHeader.biBitCount = static_cast<WORD>(uBitCount);
	lpbmi->bmiHeader.biCompression = BI_RGB;
	lpbmi->bmiHeader.biXPelsPerMeter = 1000;	/* �K�� */
	lpbmi->bmiHeader.biYPelsPerMeter = 1000;	/* �K�� */
	lpbmi->bmiHeader.biClrUsed = 0;
	lpbmi->bmiHeader.biClrImportant = 0;

	if (nColorTable != 0)
	{
		for (UINT n = 0; n < nColorTable; n++)
		{
			BYTE byLevel = static_cast<BYTE>((255 * n) / nColorTable);
			lpbmi->bmiColors[n].rgbRed = byLevel;
			lpbmi->bmiColors[n].rgbGreen = byLevel;
			lpbmi->bmiColors[n].rgbBlue = byLevel;
			lpbmi->bmiColors[n].rgbReserved = 0;
		}
	}
	BOOL bResult = Create(lpbmi);
	free(lpbmi);
	return bResult;
}

///////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDIBImage::Create(const BITMAPINFO* lpbmi)
// �T�v: BITMAPINFO �\���̂̓��e���� CNxDIBImage ���쐬
//       �r�b�g�f�[�^�̓R�s�[����Ȃ�
// ����: const BITMAPINFO* lpbmi ... BITMAPINFO �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::Create(const BITMAPINFO* lpbmi)
{
	_ASSERTE(lpbmi != NULL);

	if (!isValidInfo(lpbmi))
		return FALSE;

	// BITMAPINFO �̃T�C�Y
	UINT uColorCount = GetColorCount(lpbmi);
	DWORD dwInfoSize = lpbmi->bmiHeader.biSize + uColorCount * sizeof(RGBQUAD);

	// �r�b�g�f�[�^�̃T�C�Y
	DWORD dwImageSize = ((((((lpbmi->bmiHeader.biBitCount * static_cast<UINT>(lpbmi->bmiHeader.biWidth)) + 7) / 8) + 3) / 4) * 4) * abs(lpbmi->bmiHeader.biHeight);

	// ���������m��(& zero initialize)
	// BITMAPINFO �\����, �r�b�g�f�[�^���A������l�Ɋm��
	LPBITMAPINFO lpbmiBits = static_cast<LPBITMAPINFO>(calloc(1, dwInfoSize + dwImageSize));

	// BITMAPINFO �\���̂��R�s�[
	memcpy(lpbmiBits, lpbmi, dwInfoSize);
	lpbmiBits->bmiHeader.biSizeImage = dwImageSize;	// �r�b�g�f�[�^�̃T�C�Y��ݒ�

	// �r�b�g�f�[�^�ւ̃|�C���^
	LPVOID lpvDIBits = reinterpret_cast<LPBYTE>(lpbmiBits) + dwInfoSize;

	m_bAutoDelete = TRUE;
	return CNxDIBImage::Create(lpbmiBits, lpvDIBits);
}

///////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDIBImage::Create(LPBITMAPINFO lpbmi, LPVOID lpvDIBits)
// �T�v: �w�肳�ꂽ BITMAPINFO �\���̂ƃr�b�g�f�[�^���Q�Ƃ���l��
//       CNxDIBImage ���쐬
// ����: LPBIMTPAINFO lpbmi ... BITMAPINFO �\���̂ւ̃|�C���^
//       LPVOID lpvDIBits   ... �r�b�g�f�[�^�ւ̃|�C���^
//								NULL �Ȃ�� lpbmi ����v�Z
// �ߒl: �����Ȃ�� TRUE
///////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::Create(LPBITMAPINFO lpbmi, LPVOID lpvDIBits)
{
	_ASSERTE(lpbmi != NULL);

	if (!isValidInfo(lpbmi))
		return FALSE;

	m_uColorCount = GetColorCount(lpbmi);

	if (lpvDIBits == NULL)
	{	// NULL �Ȃ�� LPBITMAPINFO �\���̂̌�Ƀr�b�g�f�[�^���z�u����Ă���ƌ��Ȃ�
		m_lpvDIBits = reinterpret_cast<LPBYTE>(lpbmi) + lpbmi->bmiHeader.biSize + m_uColorCount * sizeof(RGBQUAD);
	}
	else
		m_lpvDIBits = lpvDIBits;

	m_lpbmi = lpbmi;

	// �s�̕����v�Z(���̎��_�ł͏�ɐ��̒l)
	m_lPitch = (((((lpbmi->bmiHeader.biBitCount * static_cast<UINT>(lpbmi->bmiHeader.biWidth)) + 7) / 8) + 3) / 4) * 4;

	// �r�b�g�f�[�^�̍�������� m_lpvBits ��ݒ�
	if (lpbmi->bmiHeader.biHeight >= 0)
	{	// bottom-up
		m_lpvBits = static_cast<LPBYTE>(m_lpvDIBits) + m_lPitch * (lpbmi->bmiHeader.biHeight - 1);
		m_lPitch = -m_lPitch;
	}
	else
	{	// top-down
		m_lpvBits = m_lpvDIBits;
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxDIBImage::Blt(int dx, int dy, const CNxDIBImage* pSrcDIBImage, const RECT* lpSrcRect) const
// �T�v: DIBImage �Ԃ̃r�b�g�u���b�N�]��
// ����: int dx						   ... �]���� X ���W
//		 int dy						   ... �]���� Y ���W
//		 const CDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 const RECT* lpSrcRect		   ... �]������`������ RECT �\���̂ւ̃|�C���^
// �ߒl: �����Ȃ�� TRUE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::Blt(int dx, int dy, const CNxDIBImage* pSrcDIBImage, const RECT* lpSrcRect)
{
	RECT rcSrc;
	if (lpSrcRect == NULL)
		pSrcDIBImage->GetRect(&rcSrc);
	else
		rcSrc = *lpSrcRect;

	// �]�����`
	RECT rcDest;
	rcDest.top = dy;
	rcDest.left = dx;
	rcDest.bottom = dy + (rcSrc.bottom - rcSrc.top);
	rcDest.right = dx + (rcSrc.right - rcSrc.left);

	// �]�����̃N���b�v��`
	RECT rcSrcClip;
	pSrcDIBImage->GetRect(&rcSrcClip);

	// �]����̃N���b�v��`
	RECT rcDestClip;
	GetRect(&rcDestClip);

	// �N���b�v
	if (!NxDrawLocal::ClipRect(&rcDest, &rcSrc, &rcDestClip, &rcSrcClip))
		return TRUE;		// ��`����

	UINT uDestBitCount = GetBitCount();
	UINT uSrcBitCount = pSrcDIBImage->GetBitCount();

	// �]����r�b�g�f�[�^�ւ̃|�C���^
	LPBYTE lpbDestBits = static_cast<LPBYTE>(GetBits()) + rcDest.top * GetPitch() + rcDest.left * uDestBitCount / 8;

	// �]�����r�b�g�f�[�^�ւ̃|�C���^
	const BYTE* lpbSrcBits = static_cast<const BYTE*>(pSrcDIBImage->GetBits()) + rcSrc.top * pSrcDIBImage->GetPitch() + rcSrc.left * uSrcBitCount / 8;


	static const BYTE nBitToIndex[] =
	{
		//  0  4  8 12 16 20 24 28 32 (�ϊ��O)
		//  0  1  2  3  4  5  6  7  8 (�ϊ���)
			0, 1, 2, 0, 3, 0, 4, 0, 5
	};

	typedef BOOL (CNxDIBImage::*BltProc)(UINT uDestX, LPBYTE lpDestBits,
										 const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits,
										 UINT uWidth, UINT uHeight);
	static const BltProc bltProc[/*src*/6][/*dest*/6] =
	{
		 &CNxDIBImage::blt1to1, &CNxDIBImage::blt1to4,  &CNxDIBImage::blt1to8,  &CNxDIBImage::blt1to16,  &CNxDIBImage::blt1to24,  &CNxDIBImage::blt1to32,
		     NULL, &CNxDIBImage::blt4to4,  &CNxDIBImage::blt4to8,  &CNxDIBImage::blt4to16,  &CNxDIBImage::blt4to24,  &CNxDIBImage::blt4to32,
		     NULL,     NULL,  &CNxDIBImage::blt8to8,  &CNxDIBImage::blt8to16,  &CNxDIBImage::blt8to24,  &CNxDIBImage::blt8to32,
		     NULL,     NULL,  &CNxDIBImage::blt16to8, &CNxDIBImage::blt16to16, &CNxDIBImage::blt16to24, &CNxDIBImage::blt16to32,
		     NULL,     NULL,  &CNxDIBImage::blt24to8, &CNxDIBImage::blt24to16, &CNxDIBImage::blt24to24, &CNxDIBImage::blt24to32,
		     NULL,     NULL,  &CNxDIBImage::blt32to8, &CNxDIBImage::blt32to16, &CNxDIBImage::blt32to24, &CNxDIBImage::blt32to32,
	};

	BltProc proc = bltProc[nBitToIndex[uSrcBitCount / 4]][nBitToIndex[uDestBitCount / 4]];
	if (proc == NULL)
	{
		_RPTF2(_CRT_ASSERT, "CNxDIBImage::Blt() : %d bpp ���� %d bpp �ւ̓]���̓T�|�[�g���Ă��܂���.\n", uSrcBitCount, uDestBitCount);
		return FALSE;
	}

	UINT uWidth = static_cast<UINT>(rcSrc.right - rcSrc.left);
	UINT uHeight = static_cast<UINT>(rcSrc.bottom - rcSrc.top);
	return (this->*proc)(rcDest.left, lpbDestBits, pSrcDIBImage, rcSrc.left, lpbSrcBits, uWidth, uHeight);
}



///////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxDIBImage::isValidInfo(const BITMAPINFO* lpbmi)
// �T�v: �Ή����Ă��� BITMAPINFO �\���̂ł��邩�𒲂ׂ�
// ����: const BITMAPINFO* lpbmi ... BITMAPINFO �\���̂ւ̃|�C���^
// �ߒl: �Ή����Ă���`���Ȃ�� TRUE�A����ȊO�� FALSE
///////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::isValidInfo(const BITMAPINFO* lpbmi)
{
	if (lpbmi->bmiHeader.biSize < sizeof(BITMAPINFOHEADER))
	{
		_RPTF0(_CRT_ASSERT, "CNxDIBImage : BITMAPINFOHEADER �\���̂̃T�C�Y(biSize)���ُ�ł�.\n");
		return FALSE;
	}

	if (lpbmi->bmiHeader.biCompression != BI_RGB && lpbmi->bmiHeader.biCompression != BI_BITFIELDS)
	{
		_RPTF0(_CRT_ASSERT, "CNxDIBImage : BITMAPINFOHEADER �\���̂Ɏw�肳�ꂽ���k�`��(biCompression)�̓T�|�[�g���Ă��܂���.\n");
		return FALSE;
	}
	if (lpbmi->bmiHeader.biBitCount != 1 && lpbmi->bmiHeader.biBitCount != 4
		&& lpbmi->bmiHeader.biBitCount != 8 && lpbmi->bmiHeader.biBitCount != 16
		&& lpbmi->bmiHeader.biBitCount != 24 && lpbmi->bmiHeader.biBitCount != 32)
	{
		_RPTF0(_CRT_ASSERT, "CNxDIBImage : BITMAPINFOHEADER �\���̂̃r�b�g�[�x(biBitCount)���s���ł�.\n");
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// public:
//	static UINT CNxDIBImage::GetColorCount(const BITMAPINFO* lpbmi)
// �T�v: �J���[�e�[�u��(RGBQUAD)�̐����擾
// ����: const BITMAPINFO* lpbmi ... BITMAPINFO �\���̂ւ̃|�C���^
// �ߒl: �J���[�e�[�u���̐�
///////////////////////////////////////////////////////////////////////////

UINT CNxDIBImage::GetColorCount(const BITMAPINFO* lpbmi)
{
	switch (lpbmi->bmiHeader.biBitCount)
	{
	case 1:
	case 4:
	case 8:
		// 1,4,8bpp �� biClrUsed ���p���b�g�̌�������(0 �Ȃ�΍ő吔[2,16,256])
		return (lpbmi->bmiHeader.biClrUsed == 0) ? (1 << lpbmi->bmiHeader.biBitCount) : lpbmi->bmiHeader.biClrUsed;
	case 16:
	case 32:
		// 16bpp �� 32bpp �́AbiCompression == BI_BITFILEDS �Ȃ�΁AcolorMask �t��
		// �X�� biClrUsed �̌����� RGBQUAD ������(�������A���ʂ� 0 �Ȃ̂Ŗ��m�F)
		if (lpbmi->bmiHeader.biCompression == BI_BITFIELDS)
			return lpbmi->bmiHeader.biClrUsed + 3;
		else
			return lpbmi->bmiHeader.biClrUsed;
	case 24:
		// 24bpp �͒���Ƀr�b�g�f�[�^������
	default:
		return 0;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt32to8(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 32bpp ���� 8bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt32to8(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 4;
	LONG lDestDistance = GetPitch() - uWidth;

	do
	{
		UINT u = uWidth;
		do
		{
			CNxColor cr(*reinterpret_cast<const NxColor*>(lpbSrcBits));
			lpbSrcBits += 4;
			*lpbDestBits++ = cr.GetGrayscale();
		} while (--u != 0);
		lpbDestBits += lDestDistance;
		lpbSrcBits += lSrcDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt32to16(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 32bpp ���� 16bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt32to16(UINT /*uDestX*/, LPBYTE lpbDestBits,
							const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 4;
	LONG lDestDistance = GetPitch() - uWidth * 2;
	CNxHighColor hc(*this);

	do
	{
		UINT u = uWidth;
		do
		{
			*reinterpret_cast<LPWORD>(lpbDestBits) = hc.GetHighColor(*reinterpret_cast<const DWORD*>(lpbSrcBits));
			lpbSrcBits += 4;
			lpbDestBits += 2;
		} while (--u != 0);
		lpbDestBits += lDestDistance;
		lpbSrcBits += lSrcDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt32to24(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 32bpp ���� 24bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt32to24(UINT /*uDestX*/, LPBYTE lpbDestBits,
							const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 4;
	LONG lDestDistance = GetPitch() - uWidth * 3;

	do
	{
		for (UINT u = 0; u < uWidth - 1; u++)
		{
			*reinterpret_cast<LPDWORD>(lpbDestBits) = *reinterpret_cast<const DWORD*>(lpbSrcBits);
			lpbDestBits += 3;
			lpbSrcBits += 4;
		}
		CNxColor color = *reinterpret_cast<const NxColor*>(lpbSrcBits);
		*(lpbDestBits + 0) = color.GetBlue();
		*(lpbDestBits + 1) = color.GetGreen();
		*(lpbDestBits + 2) = color.GetRed();
		lpbDestBits += 3;
		lpbSrcBits += 4;
		lpbDestBits += lDestDistance;
		lpbSrcBits += lSrcDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt32to32(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 32bpp ���� 32bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt32to32(UINT /*uDestX*/, LPBYTE lpbDestBits,
						    const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	do
	{
		memcpy(lpbDestBits, lpbSrcBits, uWidth * 4);
		lpbDestBits += GetPitch();
		lpbSrcBits += pSrcDIBImage->GetPitch();
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt24to8(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 24bpp ���� 8bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt24to8(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 3;
	LONG lDestDistance = GetPitch() - uWidth;

	do
	{
		UINT u = uWidth;
		do
		{
			BYTE byBlue = *(lpbSrcBits + 0);
			BYTE byGreen = *(lpbSrcBits + 1);
			BYTE byRed = *(lpbSrcBits + 2);
			lpbSrcBits += 3;
			*lpbDestBits++ = CNxColor(byRed, byGreen, byBlue).GetGrayscale();
		} while (--u != 0);
		lpbDestBits += lDestDistance;
		lpbSrcBits += lSrcDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt24to16(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 24bpp ���� 16bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt24to16(UINT /*uDestX*/, LPBYTE lpbDestBits,
						    const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 3;
	LONG lDestDistance = GetPitch() - uWidth * 2;
	CNxHighColor hc(*this);

	do
	{
		UINT u = uWidth;
		do
		{
			BYTE byBlue = *(lpbSrcBits + 0);
			BYTE byGreen = *(lpbSrcBits + 1);
			BYTE byRed = *(lpbSrcBits + 2);
			*reinterpret_cast<LPWORD>(lpbDestBits) = hc.GetHighColor(CNxColor(byRed, byGreen, byBlue));
			lpbDestBits += 2;
			lpbSrcBits += 3;
		} while (--u != 0);
		lpbDestBits += lDestDistance;
		lpbSrcBits += lSrcDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt24to24(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 24bpp ���� 24bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt24to24(UINT /*uDestX*/, LPBYTE lpbDestBits,
						    const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	do
	{
		memcpy(lpbDestBits, lpbSrcBits, uWidth * 3);
		lpbDestBits += GetPitch();
		lpbSrcBits += pSrcDIBImage->GetPitch();
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt24to32(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 24bpp ���� 32bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt24to32(UINT /*uDestX*/, LPBYTE lpbDestBits,
						    const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 3;
	LONG lDestDistance = GetPitch() - uWidth * 4;

	do
	{
		for (UINT u = 0; u < uWidth -1; u++)
		{
			*reinterpret_cast<LPDWORD>(lpbDestBits) = *reinterpret_cast<const DWORD*>(lpbSrcBits) | 0xff000000;
			lpbSrcBits += 3;
			lpbDestBits += 4;
		};
		*(lpbDestBits + 0) = *(lpbSrcBits + 0);
		*(lpbDestBits + 1) = *(lpbSrcBits + 1);
		*(lpbDestBits + 2) = *(lpbSrcBits + 2);
		*(lpbDestBits + 3) = 0xff;
		lpbDestBits += 4;
		lpbSrcBits += 3;
		lpbDestBits += lDestDistance;
		lpbSrcBits += lSrcDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt16to8(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 16bpp ���� 8bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt16to8(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 2;
	LONG lDestDistance = GetPitch() - uWidth;

	CNxHighColor hc(*pSrcDIBImage);

	do
	{
		UINT u = uWidth;
		do
		{
			*lpbDestBits++ = CNxColor(hc.GetTrueColor(*reinterpret_cast<const WORD*>(lpbSrcBits))).GetGrayscale();
			lpbSrcBits += 2;
		} while (--u != 0);
		lpbDestBits += lDestDistance;
		lpbSrcBits += lSrcDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt16to16(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 16bpp ���� 16bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt16to16(UINT /*uDestX*/, LPBYTE lpbDestBits,
						    const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	CNxHighColor hcSrc(*pSrcDIBImage);
	CNxHighColor hcDest(*this);

	if (hcDest.GetRedMask() == hcSrc.GetRedMask()
		&& hcDest.GetGreenMask() == hcSrc.GetGreenMask()
		&& hcDest.GetBlueMask() == hcSrc.GetBlueMask())
	{	// �J���[�}�X�N������
		do
		{
			memcpy(lpbDestBits, lpbSrcBits, uWidth * 2);
			lpbDestBits += GetPitch();
			lpbSrcBits += pSrcDIBImage->GetPitch();
		} while (--uHeight != 0);
	}
	else
	{	// �J���[�}�X�N���قȂ�
		// ��x true color �֕ϊ����Ă���A�]����� high color �֕ϊ�
		CNxHighColor hcSrc(*pSrcDIBImage);
		CNxHighColor hcDest(*this);
		LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 2;
		LONG lDestDistance = GetPitch() - uWidth * 2;
		do
		{
			UINT u = uWidth;
			do
			{
				*reinterpret_cast<LPWORD>(lpbDestBits)
					= hcDest.GetHighColor(hcSrc.GetTrueColor(*reinterpret_cast<const WORD*>(lpbSrcBits)));
				lpbDestBits += 2;
				lpbSrcBits += 2;
			} while (--u != 0);
			lpbDestBits += lDestDistance;
			lpbSrcBits += lSrcDistance;
		} while (--uHeight != 0);
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt16to24(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 16bpp ���� 24bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt16to24(UINT /*uDestX*/, LPBYTE lpbDestBits,
						    const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 2;
	LONG lDestDistance = GetPitch() - uWidth * 3;
	CNxHighColor hc(*pSrcDIBImage);

	do
	{
		UINT u = uWidth;
		do
		{
			CNxColor cr(hc.GetTrueColor(*reinterpret_cast<const WORD*>(lpbSrcBits)));
			*(lpbDestBits + 0) = cr.GetBlue();
			*(lpbDestBits + 1) = cr.GetGreen();
			*(lpbDestBits + 2) = cr.GetRed();
			lpbDestBits += 3;
			lpbSrcBits += 2;
		} while (--u != 0);
		lpbSrcBits += lSrcDistance;
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt16to32(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							   UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 16bpp ���� 32bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt16to32(UINT /*uDestX*/, LPBYTE lpbDestBits,
						    const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						    UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth * 2;
	LONG lDestDistance = GetPitch() - uWidth * 4;
	CNxHighColor hc(*pSrcDIBImage);

	do
	{
		UINT u = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpbDestBits) = hc.GetTrueColor(*reinterpret_cast<const WORD*>(lpbSrcBits));
			lpbDestBits += 4;
			lpbSrcBits += 2;
		} while (--u != 0);
		lpbSrcBits += lSrcDistance;
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt8to8(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							 UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 8bpp ���� 8bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt8to8(UINT /*uDestX*/, LPBYTE lpbDestBits,
						  const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						  UINT uWidth, UINT uHeight)
{
	do
	{
		memcpy(lpbDestBits, lpbSrcBits, uWidth);
		lpbDestBits += GetPitch();
		lpbSrcBits += pSrcDIBImage->GetPitch();
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt8to16(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 8bpp ���� 16bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt8to16(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth;
	LONG lDestDistance = GetPitch() - uWidth * 2;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());
	CNxHighColor hc(*this);

	do
	{
		UINT u = uWidth;
		do
		{
			*reinterpret_cast<WORD*>(lpbDestBits) = hc.GetHighColor(*(lpdwSrcColorTable + *lpbSrcBits));
			lpbDestBits += 2;
			lpbSrcBits++;
		} while (--u != 0);
		lpbSrcBits += lSrcDistance;
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt8to24(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 8bpp ���� 24bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt8to24(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth;
	LONG lDestDistance = GetPitch() - uWidth * 3;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());

	do
	{
		for (UINT u = 0; u < uWidth - 1; u++)
		{
			*reinterpret_cast<LPDWORD>(lpbDestBits) = *(lpdwSrcColorTable + *lpbSrcBits);
			lpbSrcBits++;
			lpbDestBits += 3;
		}
		CNxColor color(*(lpdwSrcColorTable + *lpbSrcBits++));
		*(lpbDestBits + 0) = color.GetBlue();
		*(lpbDestBits + 1) = color.GetGreen();
		*(lpbDestBits + 2) = color.GetRed();
		lpbDestBits += 3;
		lpbSrcBits += lSrcDistance;
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt8to32(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 8bpp ���� 32bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt8to32(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT /*uSrcX*/, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lSrcDistance = pSrcDIBImage->GetPitch() - uWidth;
	LONG lDestDistance = GetPitch() - uWidth * 4;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());

	do
	{
		UINT u = uWidth;
		do
		{
			*reinterpret_cast<LPDWORD>(lpbDestBits) = *(lpdwSrcColorTable + *lpbSrcBits) | 0xff000000;
			lpbSrcBits++;
			lpbDestBits += 4;
		} while (--u != 0);
		lpbSrcBits += lSrcDistance;
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt4to4(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							 UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 4bpp ���� 4bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt4to4(UINT uDestX, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	if ((uDestX % 2) != 0)
	{
		_RPT0(_CRT_ASSERT, "CNxDIBImage::Blt() : 4bpp �̓]���� DIB ���W�̓o�C�g�P�ʂłȂ���΂Ȃ�܂���.");
		return FALSE;
	}

	if ((uSrcX % 2) != 0)
	{
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;
			LPBYTE lpbDestBitsX = lpbDestBits;

			for (UINT u = 0; u < uWidth / 2; u++)
			{
				*lpbDestBitsX++ = static_cast<BYTE>((*lpbSrcBitsX << 4) | (*(lpbSrcBitsX + 1) >> 4));
				lpbSrcBitsX++;
			}
			if (uWidth % 2 != 0)
				*lpbDestBitsX = static_cast<BYTE>((*lpbDestBitsX & 0x0f) | (*lpbSrcBitsX << 4));

			lpbSrcBits += pSrcDIBImage->GetPitch();
			lpbDestBits += GetPitch();
		} while (--uHeight != 0);
	}
	else
	{
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;
			LPBYTE lpbDestBitsX = lpbDestBits;

			for (UINT u = 0; u < uWidth / 2; u++)
				*lpbDestBitsX++ = *lpbSrcBitsX++;
			if (uWidth % 2 != 0)
				*lpbDestBitsX = static_cast<BYTE>((*lpbDestBitsX & 0x0f) | (*lpbSrcBitsX & 0xf0));

			lpbSrcBits += pSrcDIBImage->GetPitch();
			lpbDestBits += GetPitch();
		} while (--uHeight != 0);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt4to8(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							 UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 4bpp ���� 8bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt4to8(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth;

	if ((uSrcX % 2) != 0)
	{	// 1dot ����̏ꍇ
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (UINT u = 0; u < uWidth / 2; u++)
			{
				*lpbDestBits++ = static_cast<BYTE>(*(lpbSrcBitsX + 0) & 0x0f);
				*lpbDestBits++ = static_cast<BYTE>(*(lpbSrcBitsX + 1) >> 4);
				lpbSrcBitsX++;
			}
			// �]��
			if (uWidth % 2 != 0)
				*lpbDestBits++ = static_cast<BYTE>(*(lpbSrcBitsX + 0) & 0x0f);

			lpbSrcBits += pSrcDIBImage->GetPitch();
			lpbDestBits += lDestDistance;
		} while (--uHeight != 0);
	}
	else
	{	// �o�C�g���E
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (UINT u = 0; u < uWidth / 2; u++)
			{
				*lpbDestBits++ = static_cast<BYTE>(*(lpbSrcBitsX + 0) >> 4);
				*lpbDestBits++ = static_cast<BYTE>(*(lpbSrcBitsX + 0) & 0x0f);
				lpbSrcBitsX++;
			}
			// �]��
			if (uWidth % 2 != 0)
				*lpbDestBits++ = static_cast<BYTE>(*(lpbSrcBitsX + 0) >> 4);

			lpbSrcBits += pSrcDIBImage->GetPitch();
			lpbDestBits += lDestDistance;
		} while (--uHeight != 0);
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt4to16(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 4bpp ���� 16bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt4to16(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth * 2;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());
	CNxHighColor hc(*this);

	UINT u;
	CNxColor color;
	BYTE byPixel;

	if ((uSrcX % 2) != 0)
	{	// 1dot ����̏ꍇ
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (u = 0; u < uWidth / 2; u++)
			{
				byPixel = *(lpbSrcBitsX + 0) & 0x0f;
				*reinterpret_cast<LPWORD>(lpbDestBits + 0) = hc.GetHighColor(*(lpdwSrcColorTable + byPixel));
				byPixel = *(lpbSrcBitsX + 1) >> 4;
				*reinterpret_cast<LPWORD>(lpbDestBits + 2) = hc.GetHighColor(*(lpdwSrcColorTable + byPixel));
				lpbDestBits += 4;
				lpbSrcBitsX++;
			}
			// �]��
			if (uWidth % 2 != 0)
			{
				byPixel = *(lpbSrcBitsX + 0) & 0x0f;
				*reinterpret_cast<LPWORD>(lpbDestBits + 0) = hc.GetHighColor(*(lpdwSrcColorTable + byPixel));
				lpbDestBits += 2;
			}
			lpbDestBits += lDestDistance;
			lpbSrcBits += pSrcDIBImage->GetPitch();
		} while (--uHeight != 0);
	}
	else
	{	// �o�C�g���E
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (u = 0; u < uWidth / 2; u++)
			{
				byPixel = *(lpbSrcBitsX + 0) >> 4;
				*reinterpret_cast<LPWORD>(lpbDestBits + 0) = hc.GetHighColor(*(lpdwSrcColorTable + byPixel));
				byPixel = *(lpbSrcBitsX + 0) & 0x0f;
				*reinterpret_cast<LPWORD>(lpbDestBits + 2) = hc.GetHighColor(*(lpdwSrcColorTable + byPixel));
				lpbDestBits += 4;
				lpbSrcBitsX++;
			}
			// �]��
			if (uWidth % 2 != 0)
			{
				byPixel = *(lpbSrcBitsX + 0) >> 4;
				*reinterpret_cast<LPWORD>(lpbDestBits + 0) = hc.GetHighColor(*(lpdwSrcColorTable + byPixel));
				lpbDestBits += 2;
			}
			lpbDestBits += lDestDistance;
			lpbSrcBits += pSrcDIBImage->GetPitch();
		} while (--uHeight != 0);
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt4to24(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 4bpp ���� 24bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt4to24(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth * 3;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());

	CNxColor color;
	if ((uSrcX % 2) != 0)
	{	// 1dot ����̏ꍇ
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (UINT u = 0; u < uWidth / 2; u++)
			{
				color = *(lpdwSrcColorTable + (*(lpbSrcBitsX + 0) & 0x0f));
				*(lpbDestBits + 0) = color.GetBlue();
				*(lpbDestBits + 1) = color.GetGreen();
				*(lpbDestBits + 2) = color.GetRed();
				color = *(lpdwSrcColorTable + (*(lpbSrcBitsX + 1) >> 4));
				*(lpbDestBits + 3) = color.GetBlue();
				*(lpbDestBits + 4) = color.GetGreen();
				*(lpbDestBits + 5) = color.GetRed();
				lpbSrcBitsX++;
				lpbDestBits += 6;
			}
			// �]��
			if (uWidth % 2 != 0)
			{
				color = *(lpdwSrcColorTable + (*(lpbSrcBitsX + 0) & 0x0f));
				*(lpbDestBits + 0) = color.GetBlue();
				*(lpbDestBits + 1) = color.GetGreen();
				*(lpbDestBits + 2) = color.GetRed();
				lpbDestBits += 3;
			}

			lpbSrcBits += pSrcDIBImage->GetPitch();
			lpbDestBits += lDestDistance;
		} while (--uHeight != 0);
	}
	else
	{	// �o�C�g���E
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (UINT u = 0; u < uWidth / 2; u++)
			{
				color = *(lpdwSrcColorTable + (*(lpbSrcBitsX + 0) >> 4));
				*(lpbDestBits + 0) = color.GetBlue();
				*(lpbDestBits + 1) = color.GetGreen();
				*(lpbDestBits + 2) = color.GetRed();
				color = *(lpdwSrcColorTable + (*(lpbSrcBitsX + 0) & 0x0f));
				*(lpbDestBits + 3) = color.GetBlue();
				*(lpbDestBits + 4) = color.GetGreen();
				*(lpbDestBits + 5) = color.GetRed();
				lpbSrcBitsX++;
				lpbDestBits += 6;
			}
			// �]��
			if (uWidth % 2 != 0)
			{
				color = *(lpdwSrcColorTable + (*(lpbSrcBitsX + 0) >> 4));
				*(lpbDestBits + 0) = color.GetBlue();
				*(lpbDestBits + 1) = color.GetGreen();
				*(lpbDestBits + 2) = color.GetRed();
				lpbDestBits += 3;
			}
			lpbSrcBits += pSrcDIBImage->GetPitch();
			lpbDestBits += lDestDistance;
		} while (--uHeight != 0);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt4to32(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 4bpp ���� 32bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt4to32(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth * 4;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());

	UINT u;
	CNxColor color;
	BYTE byPixel;

	if ((uSrcX % 2) != 0)
	{	// 1dot ����̏ꍇ
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (u = 0; u < uWidth / 2; u++)
			{
				byPixel = *(lpbSrcBitsX + 0) & 0x0f;
				*reinterpret_cast<NxColor*>(lpbDestBits + 0) = *(lpdwSrcColorTable + byPixel) | 0xff000000;
				byPixel = *(lpbSrcBitsX + 1) >> 4;
				*reinterpret_cast<NxColor*>(lpbDestBits + 4) = *(lpdwSrcColorTable + byPixel) | 0xff000000;
				lpbDestBits += 8;
				lpbSrcBitsX++;
			}
			// �]��
			if (uWidth % 2 != 0)
			{
				byPixel = *(lpbSrcBitsX + 0) & 0x0f;
				*reinterpret_cast<NxColor*>(lpbDestBits) = *(lpdwSrcColorTable + byPixel) | 0xff000000;
				lpbDestBits += 4;
			}
			lpbDestBits += lDestDistance;
			lpbSrcBits += pSrcDIBImage->GetPitch();
		} while (--uHeight != 0);
	}
	else
	{	// �o�C�g���E
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;

			for (u = 0; u < uWidth / 2; u++)
			{
				byPixel = *(lpbSrcBitsX + 0) >> 4;
				*reinterpret_cast<NxColor*>(lpbDestBits + 0) = *(lpdwSrcColorTable + byPixel) | 0xff000000;
				byPixel = *(lpbSrcBitsX + 0) & 0x0f;
				*reinterpret_cast<NxColor*>(lpbDestBits + 4) = *(lpdwSrcColorTable + byPixel) | 0xff000000;
				lpbDestBits += 8;
				lpbSrcBitsX++;
			}
			// �]��
			if (uWidth % 2 != 0)
			{
				byPixel = *(lpbSrcBitsX + 0) >> 4;
				*reinterpret_cast<NxColor*>(lpbDestBits) = *(lpdwSrcColorTable + byPixel) | 0xff000000;
				lpbDestBits += 4;
			}
			lpbDestBits += lDestDistance;
			lpbSrcBits += pSrcDIBImage->GetPitch();
		} while (--uHeight != 0);
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt1to1(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 1bpp ���� 1bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt1to1(UINT uDestX, LPBYTE lpbDestBits,
						  const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						  UINT uWidth, UINT uHeight)
{
	if ((uDestX % 8) != 0)
	{
		_RPT0(_CRT_ASSERT, "CNxDIBImage::Blt() : 1bpp �̓]���� DIB ���W�̓o�C�g�P�ʂłȂ���΂Ȃ�܂���.");
		return FALSE;
	}

	if ((uSrcX % 8) != 0)
	{	// 1 �` 7dot ����̏ꍇ
		UINT uSrcXR = uSrcX % 8;
		do
		{
			const BYTE* lpbSrcBitsX = lpbSrcBits;
			LPBYTE lpbDestBitsX = lpbDestBits;

			for (UINT nCount = 0; nCount < uWidth / 8; nCount++)
			{
				*lpbDestBitsX++ = static_cast<BYTE>((*lpbSrcBitsX << uSrcXR) | (*(lpbSrcBitsX + 1) >> (8 - uSrcXR)));
				lpbSrcBitsX++;
			}
			if (uWidth % 8 != 0)
				*lpbDestBitsX = static_cast<BYTE>(*lpbSrcBitsX << uSrcXR);

		lpbSrcBits += pSrcDIBImage->GetPitch();
		lpbDestBits += GetPitch();
		} while (--uHeight != 0);
	}
	else
	{	// �o�C�g���E
		do
		{
			memcpy(lpbDestBits, lpbSrcBits, uWidth / 8);
			if (uWidth % 8 != 0)
				*(lpbDestBits + uWidth / 8) = static_cast<BYTE>(*(lpbSrcBits + uWidth / 8) & (0xff << (uWidth % 8))); // �]���� dot ���}�X�N

			lpbSrcBits += pSrcDIBImage->GetPitch();
			lpbDestBits += GetPitch();
		} while (--uHeight != 0);
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt1to4(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							 UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 1bpp ���� 4bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt1to4(UINT uDestX, LPBYTE lpbDestBits,
						  const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						  UINT uWidth, UINT uHeight)
{
	if ((uDestX % 2) != 0)
	{
		_RPT0(_CRT_ASSERT, "CNxDIBImage::Blt() : 4bpp �̓]���� DIB ���W�̓o�C�g�P�ʂłȂ���΂Ȃ�܂���.");
		return FALSE;
	}

	UINT uSrcXR = uSrcX % 8;
	do
	{
		const BYTE* lpbSrcBitsX = lpbSrcBits;
		LPBYTE lpbDestBitsX = lpbDestBits;

		BYTE byData = static_cast<BYTE>(*lpbSrcBitsX++ << uSrcXR);
		BYTE byBitCount = static_cast<BYTE>(8 - uSrcXR);
		bool half = false;
		BYTE byTemp = 0x00;
		UINT u = uWidth;
		do
		{
			if (byBitCount == 0)
			{
				byData = *lpbSrcBitsX++;
				byBitCount = 8;
			}
			if (half)
			{
				*lpbDestBitsX++ = byTemp | ((byData & 0x80) >> 7);
			}
			else
				byTemp = (byData & 0x80) >> 3;
			half = !half;
			byData <<= 1;
			byBitCount--;
		} while (--u != 0);
		if (half)
			*lpbDestBitsX = (*lpbDestBitsX & 0x0f) | byTemp;

		lpbSrcBits += pSrcDIBImage->GetPitch();
		lpbDestBits += GetPitch();
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt1to8(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							 UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 1bpp ���� 8bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt1to8(UINT /*uDestX*/, LPBYTE lpbDestBits,
						  const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						  UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth;

	UINT uSrcXR = uSrcX % 8;
	do
	{
		const BYTE* lpbSrcBitsX = lpbSrcBits;

		BYTE byData = static_cast<BYTE>(*lpbSrcBitsX++ << uSrcXR);
		BYTE byBitCount = static_cast<BYTE>(8 - uSrcXR);
		UINT u = uWidth;
		do
		{
			if (byBitCount == 0)
			{
				byData = *lpbSrcBitsX++;
				byBitCount = 8;
			}
			*lpbDestBits++ = byData >> 7;
			byData <<= 1;
			byBitCount--;
		} while (--u != 0);

		lpbSrcBits += pSrcDIBImage->GetPitch();
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt1to16(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 1bpp ���� 16bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt1to16(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth * 2;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());
	UINT uSrcXR = uSrcX % 8;
	CNxHighColor hc(*this);

	do
	{
		const BYTE* lpbSrcBitsX = lpbSrcBits;

		BYTE byData = static_cast<BYTE>(*lpbSrcBitsX++ << uSrcXR);
		BYTE byBitCount = static_cast<BYTE>(8 - uSrcXR);
		BYTE byPixel;

		UINT u = uWidth;
		do
		{
			if (byBitCount == 0)
			{
				byData = *lpbSrcBitsX++;
				byBitCount = 8;
			}
			byPixel = (byData & 0x80) >> 7;
			byData <<= 1;
			byBitCount--;
			*reinterpret_cast<LPWORD>(lpbDestBits) = hc.GetHighColor(*(lpdwSrcColorTable + byPixel));
			lpbDestBits += 2;
		} while (--u != 0);
		lpbSrcBits += pSrcDIBImage->GetPitch();
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt1to24(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 1bpp ���� 24bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt1to24(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth * 3;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());

	UINT uSrcXR = uSrcX % 8;
	do
	{
		const BYTE* lpbSrcBitsX = lpbSrcBits;

		BYTE byData = static_cast<BYTE>(*lpbSrcBitsX++ << uSrcXR);
		BYTE byBitCount = static_cast<BYTE>(8 - uSrcXR);
		BYTE byPixel;
		UINT u = uWidth;
		do
		{
			if (byBitCount == 0)
			{
				byData = *lpbSrcBitsX++;
				byBitCount = 8;
			}
			byPixel = (byData & 0x80) >> 7;
			byData <<= 1;
			byBitCount--;
			CNxColor color = *(lpdwSrcColorTable + byPixel);
			*(lpbDestBits + 0) = color.GetBlue();
			*(lpbDestBits + 1) = color.GetGreen();
			*(lpbDestBits + 2) = color.GetRed();
			lpbDestBits += 3;
		} while (--u != 0);

		lpbSrcBits += pSrcDIBImage->GetPitch();
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL CNxDIBImage::blt1to32(UINT uDestX, LPBYTE lpbDestBits, const CNxDIBImage* pSrcDIBImage,
//							  UINT uSrcX, const BYTE* lpbSrcBits, UINT uWidth, UINT uHeight)
// �T�v: 1bpp ���� 32bpp �ւ̃r�b�g�C���[�W�]��
// ����: UINT   uDestX                   ... �]���� X ���W
//       LPBYTE lpbDestBits              ... �]����r�b�g�f�[�^�ւ̃|�C���^
//		 const CNxDIBImage* pSrcDIBImage ... �]���� CNxDIBImage �ւ̃|�C���^
//		 UINT uSrcX						 ... �]���� X ���W
//		 const BYTE* lpbSrcBits			 ... �]�����r�b�g�f�[�^�ւ̃|�C���^
//		 UINT uWidth					 ... �]�����镝
//		 UINT uHeight					 ... �]�����鍂��
// �ߒl: �����Ȃ�� TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxDIBImage::blt1to32(UINT /*uDestX*/, LPBYTE lpbDestBits,
						   const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpbSrcBits,
						   UINT uWidth, UINT uHeight)
{
	LONG lDestDistance = GetPitch() - uWidth * 4;
	const DWORD* lpdwSrcColorTable = reinterpret_cast<const DWORD*>(pSrcDIBImage->GetColorTable());
	UINT uSrcXR = uSrcX % 8;

	do
	{
		const BYTE* lpbSrcBitsX = lpbSrcBits;

		BYTE byData = static_cast<BYTE>(*lpbSrcBitsX++ << uSrcXR);
		BYTE byBitCount = static_cast<BYTE>(8 - uSrcXR);
		BYTE byPixel;

		UINT u = uWidth;
		do
		{
			if (byBitCount == 0)
			{
				byData = *lpbSrcBitsX++;
				byBitCount = 8;
			}
			byPixel = (byData & 0x80) >> 7;
			byData <<= 1;
			byBitCount--;
			*reinterpret_cast<NxColor*>(lpbDestBits) = *(lpdwSrcColorTable + byPixel) | 0xff000000;
			lpbDestBits += 4;
		} while (--u != 0);
		lpbSrcBits += pSrcDIBImage->GetPitch();
		lpbDestBits += lDestDistance;
	} while (--uHeight != 0);

	return TRUE;
}
