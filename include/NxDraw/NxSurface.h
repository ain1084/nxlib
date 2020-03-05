// NxSurface.h: CNxSurface �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: �T�[�t�F�X�Ǘ��N���X
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDraw.h"
#include "NxColor.h"
#include "NxDIBImage.h"
#include <vector>

// CNxSurface::FilterBlt() �֐��Ŏg�p����\����
struct NxFilterBlt
{
	static const DWORD grayscale		= 0x00000000;
	static const DWORD hueTransform		= 0x00000001;
	static const DWORD rgbColorBalance  = 0x00000002;
	static const DWORD negative			= 0x00000003;
	static const DWORD operationMask    = 0x00000003;
	static const DWORD opacity          = 0x80000000;

	DWORD dwFlags;
	UINT  uOpacity;		// �t�B���^�K�p���ʂ̕s�����x(0 �` 255)

	union
	{
		// nxfRGBColorBalance
		struct RGBColorBalance
		{
			struct
			{
				WORD wBlue;
				WORD wGreen;
				WORD wRed;
				WORD wDummy;
			} Multiplier;
			struct
			{
				short sBlue;
				short sGreen;
				short sRed;
				short wDummy;
			} Adder;
		} nxfRGBColorBalance;
		// nxfHueTransform
		struct HueTransform
		{
			int  nHue;
			int  nSaturation;
			int  nLightness;
			BOOL bSingleness;
		} nxfHueTransform;
	};
};

// Blt �Ŏg�p����G�t�F�N�g�����w�肷��\����
struct NxBlt
{
	static const DWORD blendNormal     = 0x00000000;	// �ʏ�u�����h
	static const DWORD blendAdd        = 0x00000001;	// ���Z�u�����h
	static const DWORD blendSub        = 0x00000002;	// ���Z�u�����h
	static const DWORD blendMulti      = 0x00000003;	// ��Z�u�����h
	static const DWORD blendScreen     = 0x00000004;	// �X�N���[���u�����h
	static const DWORD blendBrighten   = 0x00000005;	// ���x(���邭)�u�����h
	static const DWORD blendDarken     = 0x00000006;	// ���x(�Â�)�u�����h
	static const DWORD blendTypeMask   = 0x00000007;

	static const DWORD opacity			= 0x00000010;	// uOpacity �L��
	static const DWORD destAlpha		= 0x00000020;	// �]����T�[�t�F�X�̃A���t�@(��)�g�p
	static const DWORD color			= 0x00000040;	// nxbColor �����o�L��
	static const DWORD fill				= 0x00000080;	// �h��ׂ�
	static const DWORD colorFill		= color | fill;	// �P�F�h��ׂ�
	static const DWORD rgbaMask			= 0x00000200;	// nxbfRGBAMask �L���B�P�� Blt, Fill �ɂ����āA�}�X�N���g�p
	static const DWORD srcAlpha			= 0x00000800;	// �]�����T�[�t�F�X�̃A���t�@���g�p
	static const DWORD rule				= 0x00001000;	// rule �摜�ɏ]���ē]��
	static const DWORD linearFilter		= 0x00002000;	// �g�厞�� linear filter ��K�p
	static const DWORD mirrorLeftRight	= 0x00004000;	// ���E���]
	static const DWORD mirrorTopDown	= 0x00008000;	// �㉺���]
	static const DWORD constDestAlpha	= 0x00010000;	// �]����A���t�@���Œ�(��荂���ȕ��@�֐؂�ւ��Ȃ�)
	static const DWORD blurHorz			= 0x00020000;	// �����ڂ���
	static const DWORD noRegardAlpha	= 0x00040000;	// �]���� alpha channel �l���[���� pixel ����ꈵ�����Ȃ�(srcAlpha|linearFilter ���̂�)
	static const DWORD grayscale		= 0x00080000;	// grayscale ��
	static const DWORD dynamic			= 0x80000000;	// ���I�R�[�h�g�p

	DWORD dwFlags;

	union
	{
		DWORD dwRGBAMask;
		struct RGBAMask
		{
			BYTE byBlue;
			BYTE byGreen;
			BYTE byRed;
			BYTE byAlpha;
		} nxbRGBAMask;
	};

	struct NxRule
	{
		UINT  uLevel;
		UINT  uVague;
		POINT ptOffset;
		const CNxSurface* pSurface;
	};

	union
	{
		NxColor	nxbColor;
		NxRule	nxbRule;
	};
	UINT uBlurRange;
	UINT uOpacity;
};




class CNxFont;
class CNxFile;
namespace NxDrawLocal
{
	class CNxCustomDraw;
	class CNxDynamicDraw;
}

#include "NxDIBImage.h"
#include "NxSprite.h"


class CNxSurface
{
public:
	CNxSurface(UINT uBitCount = 32);
	virtual ~CNxSurface();

	// ������
	BOOL Create(CNxFile& nxfile);
	BOOL Create(const BITMAPINFO* lpbmi, LPCVOID lpvBits = NULL);
	BOOL Create(const CNxDIBImage* pDIBImage);
	virtual BOOL Create(int nWidth, int nHeight);

	// �`��
	BOOL Blt(int dx, int dy, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt = NULL);
	BOOL Blt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxBlt* pNxBlt = NULL);
	BOOL TileBlt(const RECT* lpDestRect, const CNxSurface* pSurface, const RECT* lpSrcRect, int nSrcXOrg, int nSrcYOrg, NxBlt* pNxBlt = NULL);
	BOOL FilterBlt(int dx, int dy, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt);
	BOOL FilterBlt(const RECT* lpDestRect, const CNxSurface* pSrcSurface, const RECT* lpSrcRect, const NxFilterBlt* pNxFilterBlt);
	BOOL FillRect(const RECT* lpRect, NxColor nxColor);
	BOOL FillRect(int dx, int dy, int cx, int cy, NxColor nxColor);
	BOOL LoadImage(int dx, int dy, CNxFile& nxfile, const RECT* lpSrcRect = NULL, BOOL bUpdateColorTable = FALSE);
	CNxFont* SetFont(CNxFont* pFont);
	CNxFont* GetFont() const;
	BOOL DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, NxColor nxColor);
	BOOL DrawText(int dx, int dy, const RECT* lpRect, LPCTSTR lpszString, const NxBlt* pNxBlt);
	BOOL GetTextExtent(LPCTSTR lpString, LPRECT lpRect);
	HDC GetDC();
	UINT ReleaseDC();
	HBITMAP GetHandle() const;
	BOOL SetTextSmoothing(BOOL bSmoothing);
	BOOL GetTextSmoothing() const;

	// ���ƍ���
	UINT GetWidth() const;
	UINT GetHeight() const;
	void GetRect(LPRECT lpRect) const;

	// �T�[�t�F�X����������
	UINT GetBitCount() const;
	LONG GetPitch() const;
	LPVOID GetBits();
	LPCVOID GetBits() const;
	BOOL SetDIBits(int dx, int dy, const BITMAPINFO* lpbmi, LPCVOID lpvBits = NULL, const RECT* lpSrcRect = NULL);
	BOOL SetDIBits(int dx, int dy, const CNxDIBImage* pDIBImage, const RECT* lpSrcRect = NULL);
	BOOL GetDIBits(int dx, int dy, LPBITMAPINFO lpbmi, LPVOID lpvBits = NULL, const RECT* lpSrcRect = NULL) const;

	// �J���[�e�[�u������
	NxColor* GetColorTable();
	const NxColor* GetColorTable() const;
	BOOL UpdateDIBColorTable();

	CNxDIBImage& GetDIBImage() const;

	// �ۑ�(�ł��邾�� CNxImageSaver �N���X���g�p���ĉ�����)
	BOOL SaveImage(CNxFile& nxfile, const RECT* lpRect = NULL) const;
	BOOL SaveImage(LPCTSTR lpszFileName, const RECT* lpRect = NULL) const;

	// �N���b�s���O��`�̐ݒ�/�擾
	void SetClipRect(const RECT* lpClipRect);
	void GetClipRect(LPRECT lpClipRect) const;

	// �]���挴�_�̐ݒ�/�擾
	void SetOrg(int x, int y);
	void GetOrg(LPPOINT ptPoint) const;

	// �g��k���]���p���(�����p : �g�p���Ȃ��ŉ�����)
	struct StretchBltInfo
	{
		UINT uSrcOrgX;					// �]�����Q�ƊJ�n���_�̏�����
		UINT uSrcOrgY;
		ULARGE_INTEGER ul64SrcDeltaX;	// �]�����Q�ƌ��_�ψ�(HighPart = ������, LowPart = ������)
		ULARGE_INTEGER ul64SrcDeltaY;
	};

	// Blt() �p�N���b�v (�����p�ł��B�g�p���Ȃ��ł�������)
	BOOL ClipBltRect(RECT& destRect) const;
	BOOL ClipBltRect(RECT& destRect, RECT& srcRect, const RECT& rcClipRect) const;
	BOOL ClipStretchBltRect(RECT& destRect, RECT& srcRect, const RECT& srcClipRect, StretchBltInfo& stretchBltInfo) const;

	enum { SmoothFontRatio = 4 };	// �X���[�W���O�p�t�H���g�̔{��

private:

	void SetAbbreviateRect(LPRECT lpRect, const RECT* lpSrcRect) const;
	static void __cdecl FontSmoothing4x4(LPVOID lpSurface, LONG lPitch, UINT uWidth, UINT uHeight);	// �⊮�k��
	BOOL SetColorTable(const CNxDIBImage* pDIBImage);

private:
	CNxDIBImage m_dibImage;
	LPBITMAPINFO m_lpbmi;
	const NxDrawLocal::CNxCustomDraw &m_customDraw;		// CNxCustomDraw �N���X(const)�ւ̎Q��
	NxDrawLocal::CNxDynamicDraw* m_pDynamicDraw;		// CNxDynamicDraw �N���X�ւ̃|�C���^
	HBITMAP		 m_hBitmap;								// DIBSection �̃n���h��
	HDC			 m_hdcCurrent;							// �擾���� HDC
	UINT		 m_udcRefCount;							// HDC �̎Q�ƃJ�E���g(GetDC() �ő���, ReleaseDC() �Ō���)
	POINT		 m_ptOrg;
	RECT		 m_rcClip;
	CNxFont*	 m_pFont;
	BOOL		 m_bTextSmoothing;

private:
	// ����ƃR�s�[�֎~
	CNxSurface(const CNxSurface& surface);
	CNxSurface& operator=(const CNxSurface&);
};

inline HBITMAP CNxSurface::GetHandle() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_hBitmap; }

inline UINT CNxSurface::GetWidth() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetWidth(); }

inline UINT CNxSurface::GetHeight() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetHeight(); }

inline UINT CNxSurface::GetBitCount() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetBitCount(); }

inline void CNxSurface::GetRect(LPRECT lpRect) const {
	_ASSERTE(lpRect != NULL);
	_ASSERTE(m_hBitmap != NULL);
	lpRect->left = 0; lpRect->top = 0; lpRect->right = GetWidth();	lpRect->bottom = GetHeight(); }

inline LONG CNxSurface::GetPitch() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetPitch(); }

inline LPVOID CNxSurface::GetBits() {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetBits(); }

inline LPCVOID CNxSurface::GetBits() const {
	_ASSERTE(m_hBitmap != NULL);
	return m_dibImage.GetBits(); }

inline void CNxSurface::GetClipRect(LPRECT lpClipRect) const {
	_ASSERTE(lpClipRect != NULL);
	_ASSERTE(m_hBitmap != NULL);
	*lpClipRect = m_rcClip; }

inline void CNxSurface::SetOrg(int x, int y) {
	m_ptOrg.x = x; m_ptOrg.y = y; }

inline void CNxSurface::GetOrg(LPPOINT ptPoint) const {
	*ptPoint = m_ptOrg; }

inline BOOL CNxSurface::GetTextSmoothing() const {
	return m_bTextSmoothing; }

inline CNxFont* CNxSurface::GetFont() const {
	return m_pFont; }

inline NxColor* CNxSurface::GetColorTable() {
	return reinterpret_cast<NxColor*>(m_dibImage.GetColorTable()); }

inline const NxColor* CNxSurface::GetColorTable() const {
	return reinterpret_cast<const NxColor*>(m_dibImage.GetColorTable()); }

inline CNxDIBImage& CNxSurface::GetDIBImage() const {
	return (CNxDIBImage&)m_dibImage; }
