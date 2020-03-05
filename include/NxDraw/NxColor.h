// NxColor.h: CNxColor クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: RGBA カラー操作をサポートするクラス
//		 全て inline 関数
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NxDraw.h"

typedef DWORD NxColor;		// 下位バイトから B,G,R,A の順

class CNxColor
{
public:

	static const NxColor aliceblue = 0xfff0f8ff;
	static const NxColor antiquewhite = 0xfffafbd7;
	static const NxColor aqua = 0xff00ffff;
	static const NxColor aquamarine = 0xff7fffd4;
	static const NxColor azure = 0xfff0ffff;
	static const NxColor beige = 0xfff5f5dc;
	static const NxColor bisque = 0xffffe4c4;
	static const NxColor blanchedalmond = 0xffffebcd;
	static const NxColor black = 0xff000000;
	static const NxColor blue = 0xff0000ff;
	static const NxColor blueviolet = 0xff8a2be2;
	static const NxColor brown = 0xffa52a2a;
	static const NxColor burlywood = 0xffdeb887;
	static const NxColor cadetblue = 0xff5f9ea0;
	static const NxColor chartreuse = 0xff7fff00;
	static const NxColor chocolate = 0xffd2691e;
	static const NxColor coral = 0xffff7f50;
	static const NxColor cormflowerblue = 0xff6495ed;
	static const NxColor comsilk = 0xfffff8dc;
	static const NxColor crimson = 0xffdc143c;
	static const NxColor cyan = 0xff00ffff;
	static const NxColor darkblue = 0xff00008b;
	static const NxColor darkcyan = 0xff008b8b;
	static const NxColor darkgoldenrod = 0xff8b860b;
	static const NxColor darkgray = 0xffa9a9a9;
	static const NxColor darkgreen = 0xff006400;
	static const NxColor darkkhaki = 0xffbdb76b;
	static const NxColor darkmagenta = 0xff8b008b;
	static const NxColor darkolivegreen = 0xff556b2f;
	static const NxColor darkorange = 0xffff8c00;
	static const NxColor darkorchid = 0xff9932cc;
	static const NxColor darkred = 0xff8b0000;
	static const NxColor darksalmon = 0xffe9967a;
	static const NxColor darkseagreen = 0xff8fbc8f;
	static const NxColor darkslateblue = 0xff483d8b;
	static const NxColor darkslategray = 0xff2f4f4f;
	static const NxColor darkturquoise = 0xff00ced1;
	static const NxColor darkviolet = 0xff9400d3;
	static const NxColor deeppink = 0xffff1493;
	static const NxColor deepskyblue = 0xff00bfff;
	static const NxColor dimgray = 0xff696969;
	static const NxColor dodgerblue = 0xff1e90ff;
	static const NxColor firebrick = 0xffb22222;
	static const NxColor floralwhite = 0xfffffaf0;
	static const NxColor forestgreen = 0xff228b22;
	static const NxColor fuchsia = 0xffff00ff;
	static const NxColor gainsboro = 0xffdcdcdc;
	static const NxColor ghostwhite = 0xfff8f8ff;
	static const NxColor gold = 0xffffd700;
	static const NxColor goldenrod = 0xffdaa520;
	static const NxColor gray = 0xff808080;
	static const NxColor green = 0xff008000;
	static const NxColor greenyellow = 0xffadff2f;
	static const NxColor honeydew = 0xfff0fff0;
	static const NxColor hotpink = 0xffff69b4;
	static const NxColor indianred = 0xffcd5c5c;
	static const NxColor indigo = 0xff4b0082;
	static const NxColor ivory = 0xfffffff0;
	static const NxColor khaki = 0xfff0e68c;
	static const NxColor lavender = 0xffe6e6fa;
	static const NxColor lavenderblush = 0xfffff0f5;
	static const NxColor lawngreen = 0xff7cfc00;
	static const NxColor lemonchiffon = 0xfffffacd;
	static const NxColor lightblue = 0xffadd8e6;
	static const NxColor lightcoral = 0xfff08080;
	static const NxColor lightcyan = 0xffe0ffff;
	static const NxColor lightgoldenrodyellow = 0xfffafad2;
	static const NxColor lightgreen = 0xff90ee90;
	static const NxColor lightgrey = 0xffd3d3d3;
	static const NxColor lightpink = 0xffffb6c1;
	static const NxColor lightsalmon = 0xffffa07a;
	static const NxColor lightseagreen = 0xff20b2aa;
	static const NxColor lightskyblue = 0xff87cefa;
	static const NxColor lightslategray = 0xff778899;
	static const NxColor lightsteelblue = 0xffb0c4de;
	static const NxColor lightyellow = 0xffffffe0;
	static const NxColor lime = 0xff00ff00;
	static const NxColor limegreen = 0xff32cd32;
	static const NxColor linen = 0xfffaf0e6;
	static const NxColor magenta = 0xffff00ff;
	static const NxColor maroon = 0xff800000;
	static const NxColor mediumaquamarine = 0xff66cdaa;
	static const NxColor mediumblue = 0xff0000cd;
	static const NxColor mediumorchid = 0xffba55d3;
	static const NxColor mediumpurple = 0xff9370db;
	static const NxColor mediumseagreen = 0xff3cb371;
	static const NxColor mediumspringgreen = 0xff00fa9a;
	static const NxColor mediumturquoise = 0xff48d1cc;
	static const NxColor midnightblue = 0xff191970;
	static const NxColor mintcream = 0xfff5fffa;
	static const NxColor mistyrose = 0xffffe4e1;
	static const NxColor moccasin = 0xffffe4b5;
	static const NxColor navajowhite = 0xffffdead;
	static const NxColor navy = 0xff000080;
	static const NxColor oldlace = 0xfffdf5e6;
	static const NxColor olive = 0xff808000;
	static const NxColor olivedrab = 0xff6b8e23;
	static const NxColor orange = 0xffffa500;
	static const NxColor orangered = 0xffff4500;
	static const NxColor orchid = 0xffda70d6;
	static const NxColor palegoldenrod = 0xffeee8aa;
	static const NxColor palegreen = 0xff98fb98;
	static const NxColor paleturquoise = 0xffafeeee;
	static const NxColor palevioletred = 0xffdb7093;
	static const NxColor papayawhip = 0xffffefd5;
	static const NxColor peachpuff = 0xffffdab9;
	static const NxColor peru = 0xffcd853f;
	static const NxColor pink = 0xffffc0cb;
	static const NxColor powderblue = 0xffb0e0e6;
	static const NxColor purple = 0xff800080;
	static const NxColor red = 0xffff0000;
	static const NxColor rosybrown = 0xffbc8f8f;
	static const NxColor royalblue = 0xff4169e1;
	static const NxColor saddlenbrown = 0xff8b4513;
	static const NxColor salmon = 0xfffa8072;
	static const NxColor sandybrown = 0xfff4a460;
	static const NxColor seagreen = 0xff2e8b57;
	static const NxColor seashell = 0xfffff5ee;
	static const NxColor sienna = 0xffa0522d;
	static const NxColor silver = 0xffc0c0c0;
	static const NxColor skyblue = 0xff87ceeb;
	static const NxColor slateblue = 0xff6a5acd;
	static const NxColor slategray = 0xff708090;
	static const NxColor snow = 0xfffffafa;
	static const NxColor springgreen = 0xff00ff7f;
	static const NxColor steelblue = 0xff4682b4;
	static const NxColor tan = 0xffd2b48c;
	static const NxColor teal = 0xff008080;
	static const NxColor thistle = 0xffd8bfd8;
	static const NxColor tomato = 0xffff6347;
	static const NxColor turquoise = 0xff40e0d0;
	static const NxColor violet = 0xffee82ee;
	static const NxColor wheat = 0xfff5deb3;
	static const NxColor white = 0xffffffff;
	static const NxColor whitesmoke = 0xfff5f5f5;
	static const NxColor yellow = 0xffffff00;
	static const NxColor yellowgreen = 0xff9acd32;

	CNxColor();
	CNxColor(NxColor nxColor);
	CNxColor(const CNxColor& nxColor);
	CNxColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha = 255);

	CNxColor& SetRGBA(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha = 255);
	CNxColor& SetNxColor(NxColor nxColor);
	NxColor GetNxColor() const;
	CNxColor& SetRed(BYTE byRed);
	BYTE GetRed() const;
	CNxColor& SetGreen(BYTE byGreen);
	BYTE GetGreen() const;
	CNxColor& SetBlue(BYTE byBlue);
	BYTE GetBlue() const;
	CNxColor& SetAlpha(BYTE byAlpha);
	BYTE GetAlpha() const;
	CNxColor& SetColorRef(COLORREF crColor);
	COLORREF GetColorRef() const;
	BYTE GetGrayscale() const;
	operator NxColor() const;

private:
#pragma warning (push)
#pragma warning (disable : 4201)	// 無名の構造体または共用体
	union
	{
		NxColor m_nxColor;		// 下位バイトから B, G, R, A
		struct
		{
			BYTE m_byBlue;
			BYTE m_byGreen;
			BYTE m_byRed;
			BYTE m_byAlpha;
		};
	};
#pragma warning (pop)
};

inline CNxColor::CNxColor() {
	SetNxColor(0); }

inline CNxColor::CNxColor(NxColor nxColor) {
	SetNxColor(nxColor); }

inline CNxColor::CNxColor(const CNxColor& nxColor) {
	SetNxColor(nxColor.m_nxColor); }
	
inline CNxColor::CNxColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha) {
	SetRGBA(byRed, byGreen, byBlue, byAlpha); }

inline CNxColor& CNxColor::SetRGBA(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha) {
	SetNxColor(static_cast<NxColor>((static_cast<DWORD>(byAlpha) << 24) | (static_cast<DWORD>(byRed) << 16) |
									(static_cast<DWORD>(byGreen) << 8) | static_cast<DWORD>(byBlue))); return *this; }

inline CNxColor& CNxColor::SetNxColor(NxColor nxColor) {
	m_nxColor = nxColor; return *this; }
	
inline NxColor CNxColor::GetNxColor() const {
	return m_nxColor; }
	
inline CNxColor& CNxColor::SetRed(BYTE byRed) {
	m_byRed = static_cast<BYTE>(byRed); return *this; }

inline BYTE CNxColor::GetRed() const {
	return m_byRed; }

inline CNxColor& CNxColor::SetGreen(BYTE byGreen) {
	m_byGreen = byGreen; return *this; }

inline BYTE CNxColor::GetGreen() const {
	return m_byGreen; }

inline CNxColor& CNxColor::SetBlue(BYTE byBlue) {
	m_byBlue = byBlue; return *this; }

inline BYTE CNxColor::GetBlue() const {
	return m_byBlue; }

inline CNxColor& CNxColor::SetAlpha(BYTE byAlpha) {
	m_byAlpha = byAlpha; return *this; }

inline BYTE CNxColor::GetAlpha() const {
	return m_byAlpha; }

inline CNxColor& CNxColor::SetColorRef(COLORREF crColor) {
	return SetRGBA(GetRValue(crColor), GetGValue(crColor), GetBValue(crColor), 255); }

inline COLORREF CNxColor::GetColorRef() const {
	return RGB(GetRed(), GetGreen(), GetBlue()); }

inline CNxColor::operator NxColor() const {
	return GetNxColor(); }
