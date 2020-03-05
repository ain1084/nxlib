// NxHLSColor.h: CNxHLSColor クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: HLS カラー操作をサポートするクラス
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxColor.h"

class CNxHLSColor
{
public:
	CNxHLSColor();
	CNxHLSColor(NxColor nxColor);
	CNxHLSColor(const CNxHLSColor& nxhlsColor);
	CNxHLSColor(WORD wHue, BYTE byLightness, BYTE bySaturation, BYTE nAlpha = 255);

	CNxHLSColor& SetNxColor(NxColor nxColor);
	NxColor GetNxColor() const;
	CNxHLSColor& SetHLS(WORD wHue, BYTE byLightness, BYTE bySaturation);
	CNxHLSColor& SetHue(WORD wHue);
	WORD GetHue() const;
	CNxHLSColor& SetLightness(BYTE byLightness);
	BYTE GetLightness() const;
	CNxHLSColor& SetSaturation(BYTE bySaturation);
	BYTE GetSaturation() const;
	CNxHLSColor& SetAlpha(BYTE byAlpha);
	BYTE GetAlpha() const;
	COLORREF GetColorRef() const;
	CNxHLSColor& SetColorRef(COLORREF crColor);
	operator NxColor() const;

private:
#pragma warning (push)
#pragma warning (disable : 4201)

	union HLSColor
	{
		DWORD dwValue;		// 下位バイトから Saturation(BYTE), Lightness(BYTE), Hue(WORD)
		struct
		{
			BYTE bySaturation;
			BYTE byLightness;
			WORD wHue;
		};
	} m_hlsColor;
#pragma warning (pop)
	BYTE m_byAlpha;			// α値

	static NxColor  __cdecl HLStoRGB(HLSColor hlsColor);
	static HLSColor __cdecl RGBtoHLS(NxColor nxColor);
};

inline CNxHLSColor::CNxHLSColor() {
	m_hlsColor.dwValue = 0; m_byAlpha = 0; }

inline CNxHLSColor::CNxHLSColor(NxColor nxColor) {
	SetNxColor(nxColor); }
	
inline CNxHLSColor::CNxHLSColor(const CNxHLSColor& nxhlsColor) {
	m_hlsColor.dwValue = nxhlsColor.m_hlsColor.dwValue; m_byAlpha = nxhlsColor.m_byAlpha; }
	
inline CNxHLSColor::CNxHLSColor(WORD wHue, BYTE byLightness, BYTE bySaturation, BYTE nAlpha) {
	SetHLS(wHue, byLightness, bySaturation); m_byAlpha = nAlpha; }

inline CNxHLSColor& CNxHLSColor::SetNxColor(NxColor nxColor) {
	m_hlsColor = RGBtoHLS(nxColor); m_byAlpha = static_cast<BYTE>((nxColor >> 24) & 0xff); return *this; }

inline NxColor CNxHLSColor::GetNxColor() const {
	return HLStoRGB(m_hlsColor) | ((DWORD)m_byAlpha << 24); }

inline CNxHLSColor& CNxHLSColor::SetHLS(WORD wHue, BYTE byLightness, BYTE bySaturation) {
	_ASSERTE(wHue < 360);
	m_hlsColor.dwValue = (static_cast<DWORD>(wHue) << 16) | (static_cast<DWORD>(byLightness) << 8) | static_cast<DWORD>(bySaturation); return *this; }

inline CNxHLSColor& CNxHLSColor::SetHue(WORD wHue) {
	_ASSERTE(wHue < 360);
	m_hlsColor.wHue = wHue; return *this; }

inline WORD CNxHLSColor::GetHue() const {
	return m_hlsColor.wHue; }

inline CNxHLSColor& CNxHLSColor::SetLightness(BYTE byLightness) {
	m_hlsColor.byLightness = byLightness; return *this; }

inline BYTE CNxHLSColor::GetLightness() const {
	return m_hlsColor.byLightness; }

inline CNxHLSColor& CNxHLSColor::SetSaturation(BYTE bySaturation) {
	m_hlsColor.bySaturation = bySaturation; return *this; }

inline BYTE CNxHLSColor::GetSaturation() const {
	return m_hlsColor.bySaturation; }

inline CNxHLSColor& CNxHLSColor::SetAlpha(BYTE byAlpha) {
	m_byAlpha = byAlpha; return *this; }

inline BYTE CNxHLSColor::GetAlpha() const {
	return m_byAlpha; }

inline COLORREF CNxHLSColor::GetColorRef() const {
	return CNxColor(GetNxColor()).GetColorRef(); }

inline CNxHLSColor& CNxHLSColor::SetColorRef(COLORREF crColor) {
	return SetNxColor(CNxColor().SetColorRef(crColor)); }

inline CNxHLSColor::operator NxColor() const {
	return GetNxColor(); }
