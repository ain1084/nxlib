// NxFont.h: CNxFont クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxFont  
{
public:
	CNxFont();
	CNxFont(const CNxFont& nxFont);
	CNxFont(const LOGFONT* lpLogFont);
	CNxFont(LPCTSTR lpszFaceName, LONG lSize);
	CNxFont& operator=(const CNxFont&);
	~CNxFont();

	void SetHeight(LONG lHeight);
	void SetWidth(LONG lWidth);
	void SetEscapement(LONG lEscapement);
	void SetOrientation(LONG lOrientation);
	void SetWeight(LONG lWeight);
	void SetItalic(BOOL bItalic);
	void SetUnderline(BOOL bUnderline);
	void SetStrikeOut(BOOL bStrikeOut);
	void SetCharSet(BYTE byCharSet);
	void SetOutPrecision(BYTE byPrecision);
	void SetClipPrecision(BYTE byClipPrecision);
	void SetQuality(BYTE byQuality);
	void SetPitchAndFamily(BYTE byPitchAndFamily);
	void SetFaceName(LPCTSTR lpszName);
	void SetLogFont(const LOGFONT* lplf);
	void SetSize(LONG lSize);

	LONG GetHeight() const;
	LONG GetWidth() const;
	LONG GetEscapement() const;
	LONG GetOrientation() const;
	LONG GetWeight() const;
	BOOL GetItalic() const;
	BOOL GetUnderline() const;
	BOOL GetStrikeOut() const;
	BYTE GetCharSet() const;
	BYTE GetOutPrecision() const;
	BYTE GetClipPrecision() const;
	BYTE GetQuality() const;
	BYTE GetPitchAndFamily() const;
	void GetFaceName(TCHAR faceName[LF_FACESIZE]) const;
	LPCTSTR GetFaceName() const;
	void GetLogFont(LOGFONT* lplf) const;

	HFONT GetHandle();
	BOOL Attach(HFONT hFont);
	HFONT Detach();

	enum FontType
	{
		FontType_Normal,
		FontType_Smooth
	};
	// この関数は CNxSurface クラス専用です。使用しないで下さい
	HFONT GetHandleInternal(FontType fontType);

private:
	void deleteFont();

	LOGFONT m_lf;
	HFONT m_hFont;
	HFONT m_hFontSmooth;
	BOOL m_bDirty;
};

inline void CNxFont::SetHeight(LONG lHeight) {
	m_lf.lfHeight = lHeight; m_bDirty = TRUE; }

inline void CNxFont::SetWidth(LONG lWidth) {
	m_lf.lfWidth = lWidth; m_bDirty = TRUE; }

inline void CNxFont::SetEscapement(LONG lEscapement) {
	m_lf.lfEscapement = lEscapement; m_bDirty = TRUE; }

inline void CNxFont::SetOrientation(LONG lOrientation) {
	m_lf.lfOrientation = lOrientation; m_bDirty = TRUE; }

inline void CNxFont::SetWeight(LONG lWeight) {
	m_lf.lfWeight = lWeight; m_bDirty = TRUE; }

inline void CNxFont::SetItalic(BOOL bItalic) {
	m_lf.lfItalic = static_cast<BYTE>(bItalic); m_bDirty = TRUE; }

inline void CNxFont::SetUnderline(BOOL bUnderline) {
	m_lf.lfUnderline = static_cast<BYTE>(bUnderline); m_bDirty = TRUE; }

inline void CNxFont::SetStrikeOut(BOOL bStrikeOut) {
	m_lf.lfStrikeOut = static_cast<BYTE>(bStrikeOut); m_bDirty = TRUE; }

inline void CNxFont::SetCharSet(BYTE byCharSet) {
	m_lf.lfCharSet = byCharSet; m_bDirty = TRUE; }

inline void CNxFont::SetOutPrecision(BYTE byOutPrecision) {
	m_lf.lfOutPrecision = byOutPrecision; m_bDirty = TRUE; }

inline void CNxFont::SetClipPrecision(BYTE byClipPrecision) {
	m_lf.lfClipPrecision = byClipPrecision; m_bDirty = TRUE; }

inline void CNxFont::SetQuality(BYTE byQuality) {
	m_lf.lfQuality = byQuality; m_bDirty = TRUE; }

inline void CNxFont::SetPitchAndFamily(BYTE byPitchAndFamily) {
	m_lf.lfPitchAndFamily = byPitchAndFamily; m_bDirty = TRUE; }

inline void CNxFont::SetFaceName(LPCTSTR lpszFaceName) {
	_ASSERTE(lpszFaceName != NULL);
	::lstrcpyn(m_lf.lfFaceName, lpszFaceName, LF_FACESIZE); m_bDirty = TRUE; }

inline void CNxFont::SetLogFont(const LOGFONT* lplf) {
	m_lf = *lplf; m_bDirty = TRUE; }

inline LONG CNxFont::GetHeight() const {
	return m_lf.lfHeight; }

inline LONG CNxFont::GetWidth() const {
	return m_lf.lfWidth; }

inline LONG CNxFont::GetEscapement() const {
	return m_lf.lfEscapement; }

inline LONG CNxFont::GetOrientation() const {
	return m_lf.lfOrientation; }

inline LONG CNxFont::GetWeight() const {
	return m_lf.lfWeight; }

inline BOOL CNxFont::GetItalic() const {
	return m_lf.lfItalic; }

inline BOOL CNxFont::GetStrikeOut() const {
	return m_lf.lfStrikeOut; }

inline BYTE CNxFont::GetCharSet() const {
	return m_lf.lfCharSet; }

inline BYTE CNxFont::GetOutPrecision() const {
	return m_lf.lfOutPrecision; }

inline BYTE CNxFont::GetClipPrecision() const {
	return m_lf.lfClipPrecision; }

inline BYTE CNxFont::GetQuality() const {
	return m_lf.lfQuality; }

inline BYTE CNxFont::GetPitchAndFamily() const {
	return m_lf.lfPitchAndFamily; }

inline void CNxFont::GetFaceName(TCHAR faceName[LF_FACESIZE]) const {
	::lstrcpyn(faceName, m_lf.lfFaceName, LF_FACESIZE); }

inline LPCTSTR CNxFont::GetFaceName() const {
	return m_lf.lfFaceName; }

inline void CNxFont::GetLogFont(LOGFONT* lplf) const {
	_ASSERTE(lplf != NULL);
	*lplf = m_lf; }

inline void CNxFont::SetSize(LONG lSize) {
	m_lf.lfWidth = 0; m_lf.lfHeight = lSize; m_bDirty = TRUE; }

inline HFONT CNxFont::GetHandle() {
	return GetHandleInternal(FontType_Normal); }
