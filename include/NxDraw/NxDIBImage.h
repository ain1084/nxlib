// NxDIBImage.h: CNxDIBImage クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: DIB を扱うクラス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxColor.h"

class CNxDIBImage  
{
public:
	CNxDIBImage();
	virtual ~CNxDIBImage();
	BOOL Create(const BITMAPINFO* lpbmi);
	BOOL Create(HGLOBAL hGlobal);
	BOOL Create(LPBITMAPINFO lpbmi, LPVOID lpvBits);
	BOOL Create(int nWidth, int nHeight, UINT uBitCount);
	
	const BITMAPINFOHEADER* GetInfoHeader() const;
	const BITMAPINFO* GetInfo() const;

	RGBQUAD* GetColorTable();
	const RGBQUAD* GetColorTable() const;
	LPVOID GetDIBits();
	LPCVOID GetDIBits() const;
	LPVOID GetBits();
	LPCVOID GetBits() const;
	UINT GetWidth() const;
	UINT GetHeight() const;
	void GetRect(LPRECT lpRect) const;
	UINT GetBitCount() const;
	BOOL Blt(int dx, int dy, const CNxDIBImage* pDestImage, const RECT* lpSrcRect = NULL);
	UINT GetColorCount() const;
	static UINT GetColorCount(const BITMAPINFO* lpbmi);
	DWORD GetImageSize() const;
	DWORD GetInfoSize() const;
	LONG GetPitch() const;

private:

	BOOL blt1to1(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt1to4(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt1to8(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt1to16(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt1to24(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt1to32(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt4to1(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt4to4(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt4to8(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt4to16(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt4to24(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt4to32(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt8to1(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt8to4(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt8to8(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt8to16(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt8to24(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt8to32(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt16to8(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt16to16(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt16to24(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt16to32(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt24to8(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt24to16(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt24to24(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt24to32(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt32to8(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt32to16(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt32to24(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);
	BOOL blt32to32(UINT uDestX, LPBYTE lpDestBits, const CNxDIBImage* pSrcDIBImage, UINT uSrcX, const BYTE* lpSrcBits, UINT uWidth, UINT uHeight);

	static BOOL isValidInfo(const BITMAPINFO* lpbmi);

	LPBITMAPINFO m_lpbmi;
	LPVOID		 m_lpvBits;
	LPVOID		 m_lpvDIBits;
	UINT		 m_uColorCount;
	LONG		 m_lPitch;
	BOOL		 m_bAutoDelete;

private:
	// 代入とコピーは禁止
	CNxDIBImage& operator=(const CNxDIBImage& image);
	CNxDIBImage(const CNxDIBImage& dibImage);
};

inline LPVOID CNxDIBImage::GetBits() {
	_ASSERT(m_lpbmi != NULL);
	return m_lpvBits; }

inline LPCVOID CNxDIBImage::GetBits() const {
	_ASSERT(m_lpbmi != NULL);
	return m_lpvBits; }

inline LPVOID CNxDIBImage::GetDIBits() {
	_ASSERT(m_lpbmi != NULL);
	return m_lpvDIBits; }

inline LPCVOID CNxDIBImage::GetDIBits() const {
	_ASSERT(m_lpbmi != NULL);
	return m_lpvDIBits; }

inline DWORD CNxDIBImage::GetImageSize() const {
	_ASSERT(m_lpbmi != NULL);
	return abs(GetPitch()) * GetHeight(); }

inline UINT CNxDIBImage::GetWidth() const {
	_ASSERT(m_lpbmi != NULL);
	return GetInfoHeader()->biWidth; }

inline UINT CNxDIBImage::GetHeight() const {
	_ASSERT(m_lpbmi != NULL);
	return abs(GetInfoHeader()->biHeight); }

inline UINT CNxDIBImage::GetBitCount() const {
	_ASSERT(m_lpbmi != NULL);
	return GetInfoHeader()->biBitCount; }

inline void CNxDIBImage::GetRect(LPRECT lpRect) const {
	_ASSERT(m_lpbmi != NULL);
	lpRect->left = 0; lpRect->top = 0; lpRect->right = GetWidth(); lpRect->bottom = GetHeight(); }

inline UINT CNxDIBImage::GetColorCount() const {
	_ASSERT(m_lpbmi != NULL);
	return m_uColorCount; }

inline LONG CNxDIBImage::GetPitch() const {
	_ASSERT(m_lpbmi != NULL);
	return m_lPitch; }

inline const BITMAPINFOHEADER* CNxDIBImage::GetInfoHeader() const {
	_ASSERT(m_lpbmi != NULL);
	return &m_lpbmi->bmiHeader; }

inline const BITMAPINFO* CNxDIBImage::GetInfo() const {
	_ASSERT(m_lpbmi != NULL);
	return m_lpbmi; }

inline DWORD CNxDIBImage::GetInfoSize() const {
	_ASSERT(m_lpbmi != NULL);
	return m_lpbmi->bmiHeader.biSize + m_uColorCount * sizeof(RGBQUAD); }

inline RGBQUAD* CNxDIBImage::GetColorTable() {
	_ASSERT(m_lpbmi != NULL);
	return (m_uColorCount == 0) ? NULL : m_lpbmi->bmiColors; }

inline const RGBQUAD* CNxDIBImage::GetColorTable() const {
	_ASSERT(m_lpbmi != NULL);
	return (m_uColorCount == 0) ? NULL : m_lpbmi->bmiColors; }
