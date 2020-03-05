// NxWindow.h: CNxWindow クラスのインターフェイス
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: CNxSprite 派生クラス。
//       結び付けたウィンドウのクライアント領域へ描画する。
//////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include "NxColor.h"
#include "NxTrackingSprite.h"

class CNxSurface;
class CNxWindow : public CNxTrackingSprite
{
public:
	CNxWindow();
	virtual ~CNxWindow();
	virtual BOOL Attach(HWND hWnd);
	virtual HWND Detach();

	BOOL Refresh(HDC hDC = NULL, BOOL bForce = FALSE);
	NxColor GetBkColor() const;
	NxColor SetBkColor(NxColor nxcrBkColor);
	NxColor GetDarkColor() const;
	NxColor SetDarkColor(NxColor nxcrDarkColor);
	NxColor GetBrightColor() const;
	NxColor SetBrightColor(NxColor nxcrBrightColor);
	UINT SetBrightness(UINT uBrightness);
	UINT GetBrightness() const;
	BOOL GetCursorPos(LPPOINT lpPoint) const;
	HWND GetHWND() const;

	// CNxSprite override
	virtual CNxSprite* SetParent(CNxSprite* pNewParent);
	
	// CNxTrackingSprite override
	virtual BOOL SetTrackingUnit(int nXUnit, int nYUnit);

	// CNxWindow virtual function
	virtual BOOL SetCursorPos(int x, int y);

protected:

	// CNxSprite override
	virtual BOOL Draw(CNxSurface* pSurface, const RECT* lpRect) const;
	virtual void DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const;

	// CNxTrackingSprite override
	virtual void RefreshRect(const RECT* lpRect, LPVOID lpContext) const;
	virtual BOOL RefreshBegin(LPVOID lpContext) const;
	virtual void RefreshEnd(LPVOID lpContext) const;
	
	// CNxWindow protected virtual function
	virtual void OnWndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	BOOL createBufferSurface();
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	struct RefreshContext
	{
		HDC hdcClient;
		HDC hdcBitmap;
		BOOL bReleaseDC;
	};

	NxColor m_nxcrBkColor;
	NxColor m_nxcrBrightColor;
	NxColor m_nxcrDarkColor;
	UINT m_uBrightness;
	WNDPROC m_pfnWndProcPrev;
	std::auto_ptr<CNxSurface> m_pBufferSurface;
	HWND m_hWnd;

private:
	CNxWindow(const CNxWindow&);
	CNxWindow& operator=(const CNxWindow&);
};

inline NxColor CNxWindow::GetBkColor() const {
	return m_nxcrBkColor; }

inline NxColor CNxWindow::GetDarkColor() const {
	return m_nxcrDarkColor; }

inline NxColor CNxWindow::GetBrightColor() const {
	return m_nxcrBrightColor; }

inline UINT CNxWindow::GetBrightness() const {
	return m_uBrightness; }

inline HWND CNxWindow::GetHWND() const {
	return m_hWnd; }
