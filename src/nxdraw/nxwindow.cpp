// NxWindow.cpp: CNxWindow クラスのインプリメンテーション
// Copyright(c) 2000,2001 S.Ainoguchi
//
// 概要: CNxSprite 派生クラス
//       結び付けたウィンドウのクライアント領域へ差分描画
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include <algorithm>
#include "NxWindow.h"
#include "NxSurface.h"

namespace
{	// ウィンドウプロパティ名
	const TCHAR szWindowPropertyName[] = _T("CNXWINDOW_THIS");
};

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxWindow::CNxWindow()
 : m_hWnd(NULL)								// 結び付けられている HWND
 , m_uBrightness(255)						// 明るさ(255 = default)
 , m_nxcrBkColor(CNxColor().SetColorRef(::GetSysColor(COLOR_WINDOW)))
 , m_nxcrBrightColor(CNxColor::white)		// 明るい色(brightness 0 ～ 254)
 , m_nxcrDarkColor(CNxColor::black)			// 暗い色(brightness 256 - 511)
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
// 概要: 差分追跡の単位を設定
// 引数: int nXUnit ... 横方向の単位
//       int nYUnit ... 縦方向の単位
// 戻値: 成功なら TRUE
// 備考: CNxTrackingSprite::SetTrackingUnit() のオーバーライド
///////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::SetTrackingUnit(int nXUnit, int nYUnit)
{
	if (!CNxTrackingSprite::SetTrackingUnit(nXUnit, nYUnit))
		return FALSE;

	// nXUnit, nYUnit に応じてバッファを再生成
	return createBufferSurface();
}

////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxWindow::Attach(HWND hWnd)
// 概要: ウィンドウを結び付ける
// 引数: HWND hWnd ... 結び付けるウィンドウハンドル
// 戻値: 成功なら TRUE
////////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::Attach(HWND hWnd)
{
	_ASSERT(::IsWindow(hWnd));
	
	if (m_hWnd != NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxWindow::Attach() : 既にウィンドウが結び付けられています.\n");
		return FALSE;
	}

	// this ポインタを渡す為にウィンドウプロパティを設定
#if defined(_DEBUG)
	if (::GetProp(hWnd, szWindowPropertyName) != NULL)
		_RPT0(_CRT_WARN, "CNxWindow::Attach() : 指定されたウィンドウは既に他の CNxWindow クラスと結び付けられている可能性があります.\n");
#endif
	if (!::SetProp(hWnd, szWindowPropertyName, reinterpret_cast<HANDLE>(this)))
	{
		_RPT0(_CRT_ASSERT, "CNxWindow::Attach() : ウィンドウプロパティの設定に失敗しました.\n");
		return FALSE;
	}

	// バッファ用サーフェス作成
	createBufferSurface();

	m_hWnd = hWnd;

	// サブクラス化(ウィンドウプロシージャ差し替え)
	m_pfnWndProcPrev = reinterpret_cast<WNDPROC>(::SetWindowLong(m_hWnd, GWL_WNDPROC, reinterpret_cast<LONG>(wndProc)));

	// 現在のサイズで初期値として設定
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	return SetSize(rcClient.right, rcClient.bottom);
}

///////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxWindow::createBufferSurface()
// 概要: バッファ用サーフェスを作成
// 引数: なし
// 戻値: 成功なら TRUE
// 備考: 横幅は固定
///////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::createBufferSurface()
{
	SIZE sizeTracking;
	GetTrackingUnit(&sizeTracking);		// 更新単位を取得

	if (m_pBufferSurface.get() != 0)
	{	// 作成済み
		int nWidth = m_pBufferSurface->GetWidth();
		int nHeight = m_pBufferSurface->GetHeight();
		if (nHeight == sizeTracking.cy && (nWidth % sizeTracking.cx) == 0)
		{
			return TRUE;		// 現在のサイズと同じ
		}
	}

	// バッファ用サーフェスの生成
	m_pBufferSurface.reset(new CNxSurface(32));
	if (!m_pBufferSurface->Create(640 - 640 % sizeTracking.cx, sizeTracking.cy))
	{
		_RPTF0(_CRT_ERROR, "CNxWindow : バッファ用サーフェスの作成に失敗しました.\n");
		m_pBufferSurface.reset();
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual HWND CNxWindow::Detach()
// 概要: ウィンドウを切り離す
// 引数: なし
// 戻値: 切り離されたウィンドウハンドル
// 備考: 子スプライトが消えないだけで、ほとんどのリソースが開放される
///////////////////////////////////////////////////////////////////////////////////

HWND CNxWindow::Detach()
{
	HWND hWndResult = m_hWnd;
	if (m_hWnd != NULL)
	{
		// サブクラス化解除
		::SetWindowLong(m_hWnd, GWL_WNDPROC, reinterpret_cast<LONG>(m_pfnWndProcPrev));
		// ウィンドウプロパティ削除
		::RemoveProp(m_hWnd, szWindowPropertyName);
		// バッファ用サーフェス削除
		m_pBufferSurface.reset();
		m_hWnd = NULL;
	}
	return hWndResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static LRESULT CALLBACK CNxWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// 概要: サブクラス化後に呼ばれるウィンドウプロシージャ
// 引数: HWND hWnd     ... ウィンドウハンドル
//       UINT uMsg     ... メッセージ
//       WPARAM wParam ... メッセージの追加情報1
//       LPARAM lParam ... メッセージの追加情報2
// 戻値: メッセージの種類により異なる
///////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ウィンドウプロパティから CNxWindow へのポインタを取り出す
	CNxWindow* pThis = reinterpret_cast<CNxWindow*>(::GetProp(hWnd, szWindowPropertyName));

	// サブクラス化前のウィンドウプロシージャのポインタを取り出す
	WNDPROC pfnWndProcPrev = pThis->m_pfnWndProcPrev;

	// 仮想関数呼び出し
	pThis->OnWndMessage(uMsg, wParam, lParam);
	
	// 以前のウィンドウプロシージャへメッセージを渡す
	return ::CallWindowProc(pfnWndProcPrev, hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindow::OnWndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
// 概要: ウィンドウメッセージを処理
// 引数: UINT uMsg     ... メッセージ
//       WPARAM wParam ... メッセージの追加情報1
//       LPARAM lParam ... メッセージの追加情報2
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
// 概要: 背景色の設定
// 引数: NxColor nxcrBkColor ... 背景色
// 戻値: 以前の背景色
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
// 概要: SetBrightness() によって、暗くする時の目的色を設定
// 引数: NxColor nxcrDarkColor ... 設定色
// 戻値: 以前の設定色
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
// 概要: SetBrightness() によって、明るくする時の目的色を設定
// 引数: NxColor nxcrBrightColor ... 設定色
// 戻値: 以前の設定色
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
// 概要: 画面の明るさを設定
// 引数: UINT uBrightness ... 画面の明るさ(0 ～ 254, (255), 256 ～ 511)
// 戻値: 以前の明るさ
//////////////////////////////////////////////////////////////////////////////////

UINT CNxWindow::SetBrightness(UINT uBrightness)
{
	if (uBrightness > 511)
	{
		_RPTF1(_CRT_ASSERT, "CNxWindow::SetBrightness() : 範囲外の値(%d) が指定されました.\n", uBrightness);
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
// 概要: フレームの描画
// 引数: HDC  hDC           ... 描画先デバイスコンテキスト(NULL ならば現在のウィンドウへ)
//       BOOL bForce        ... TRUE にすると強制的に再描画(全領域が再描画対象)
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::Refresh(HDC hDC, BOOL bForce)
{
	RefreshContext rc;
	if (hDC == NULL)
	{
		if (m_hWnd == NULL)
		{
			_RPTF0(_CRT_WARN, "CNxWindow::Refresh() : ウィンドウが Attach されていない場合、hDC へ NULL を指定する事はできません.\n");
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
// 概要: 更新矩形がある場合、矩形の列挙される直前に呼び出される関数
// 引数: Refresh() 関数へ渡された lpContext 引数
// 戻値: TRUE ならば矩形の列挙を開始、FALSE ならば中止
//////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::RefreshBegin(LPVOID lpContext) const
{
	RefreshContext* prc = static_cast<RefreshContext*>(lpContext);

	// バッファ用サーフェスの HDC を取得
	prc->hdcBitmap = m_pBufferSurface->GetDC();
	if (prc->hdcBitmap == NULL)
	{
		_RPTF0(_CRT_WARN, "CNxWindow::RefreshBegin() : デバイスコンテストの取得に失敗しました.\n");
		return FALSE;
	}

	if (prc->hdcClient == NULL)
	{
		// hdcClient が NULL (Refresh() 関数で HDC が省略された場合) ならば、
		// GetDC() によってウィンドウデバイスコンテキストを取得する
		prc->hdcClient = ::GetDC(m_hWnd);
		if (prc->hdcClient == NULL)
		{	// 取得失敗
			m_pBufferSurface->ReleaseDC();
			prc->hdcBitmap = NULL;
			_RPTF0(_CRT_WARN, "CNxWindow::RefreshBegin() : デバイスコンテストの取得に失敗しました.\n");
			return FALSE;
		}
		prc->bReleaseDC = TRUE;		// 要 ReleaseDC() フラグ
	}
	else
	{
		prc->bReleaseDC = FALSE;	// ReleaseDC() は不要
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindow::RefreshEnd(LPVOID lpContext) const
// 概要: 更新矩形がある場合、全ての矩形の列挙を終了した後に呼び出される関数
// 引数: Refresh() 関数へ渡された lpContext 引数
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////

void CNxWindow::RefreshEnd(LPVOID lpContext) const
{
	RefreshContext* prc = static_cast<RefreshContext*>(lpContext);

	// RefreshBegin() 内で GetDC() によって、
	// ウィンドウデバイスコンテストを取得したならば、デバイスコンテキストを開放
	if (prc->bReleaseDC)
		::ReleaseDC(m_hWnd, prc->hdcClient);

	// バッファサーフェスへの HDC を開放
	m_pBufferSurface->ReleaseDC();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindow::RefreshRect(const RECT* lpRect, LPVOID lpContext) const
// 概要: 更新された部分を HDC へ転送(CNxWindow::Refresh() 内部から呼び出される)
// 引数: const RECT* lpRect ... 更新する矩形
//       LPVOID lpContent   ... コンテキスト(EnumRectContext 構造体へのポインタ)
// 戻値: なし
// 備考: lpContext は RefreshContext 構造体へのポインタ
////////////////////////////////////////////////////////////////////////////////////////////////////

void CNxWindow::RefreshRect(const RECT* lpRect, LPVOID lpContext) const
{
	// バッファサーフェスへスプライトを描画
	DrawSurface(m_pBufferSurface.get(), 0, 0, lpRect);

	RefreshContext* prc = static_cast<RefreshContext*>(lpContext);
	// 画面(HDC)へ転送
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
// 概要: スプライト描画
// 引数: CNxSurface* pSurface ... 描画先サーフェスへのポインタ
//       const RECT* lpRect   ... スプライト内の描画矩形を示す RECT 構造体へのポインタ
// 戻値: 子スプライトの描画を続けるならば TRUE
// 備考: 最上位スプライトなので、非表示の状態でも呼び出される
////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxWindow::Draw(CNxSurface* pSurface, const RECT* lpRect) const
{
	if (IsVisible())
	{	// 表示中
		if (CNxColor(m_nxcrBkColor).GetAlpha() != 0)
			pSurface->FillRect(lpRect, m_nxcrBkColor);
	}
	else
	{	// 非表示
		pSurface->FillRect(lpRect, m_nxcrDarkColor);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxWindows::DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const
// 概要: 全ての子スプライトの後に描画
// 引数: CNxSurface* pSurface ... 描画先サーフェスへのポインタ
//       const RECT* lpRect   ... スプライト内の描画矩形を示す RECT 構造体へのポインタ
// 戻値: なし
// 備考: brightness 適用処理
/////////////////////////////////////////////////////////////////////////////////////////

void CNxWindow::DrawBehindChildren(CNxSurface* pSurface, const RECT* lpRect) const
{
	// brightness 適用
	if (m_uBrightness != 255)
	{
		NxBlt nxb;
		if (m_uBrightness > 255)
		{	// 明るくする (256 - 511)
			nxb.nxbColor = CNxColor(m_nxcrBrightColor).SetAlpha(static_cast<BYTE>(m_uBrightness - 256));
		}
		else
		{	// 暗くする (0 - 254)
			nxb.nxbColor = CNxColor(m_nxcrDarkColor).SetAlpha(static_cast<BYTE>(255 - m_uBrightness));
		}
		nxb.dwFlags = NxBlt::colorFill | NxBlt::blendNormal;
		pSurface->Blt(lpRect, NULL, NULL, &nxb);
	}
}

/////////////////////////////////////////////////////////////////////
// public:
//	void CNxWindow::GetCursorPos(LPPOINT lpPoint) const
// 概要: マウスカーソルの座標を得る
// 引数: LPPOINT lpPoint ... 座標を受けとる POINT 構造体へのポインタ
// 戻値: 成功ならば TRUE
// 備考: 戻値はウィンドウのクライアント座標を示す
//////////////////////////////////////////////////////////////////////

BOOL CNxWindow::GetCursorPos(LPPOINT lpPoint) const
{
	_ASSERTE(lpPoint != NULL);
	if (m_hWnd == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxWindow::GetCursorPos() : ウィンドウが Attach() されていません.\n");
		return FALSE;
	}	
	::GetCursorPos(lpPoint);
	::ScreenToClient(m_hWnd, lpPoint);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxWindow::SetCursorPos(int x, int y)
// 概要: マウスカーソルの座標を設定
// 引数: int x ... X 座標(スプライト内座標)
//       int y ... Y 座標(スプライト内座標)
// 戻値: 成功ならば TRUE
//////////////////////////////////////////////////////////////////////

BOOL CNxWindow::SetCursorPos(int x, int y)
{
	if (m_hWnd == NULL)
	{
		_RPTF0(_CRT_ASSERT, "CNxWindow::GetCursorPos() : ウィンドウが Attach() されていません.\n");
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
// 概要: 親を変更する
// 引数: CNxSprite* pNewParent ... 新しい親
// 戻値: 直前の親。NULL ならば失敗
// 備考: CNxSprite::SetParent() のオーバーライド。常に失敗する
//////////////////////////////////////////////////////////////////////////////

CNxSprite* CNxWindow::SetParent(CNxSprite* /*pNewParent*/)
{
	_RPTF0(_CRT_ASSERT, "CNxWindow::SetParent() : CNxWindow の親は設定できません.\n");
	return NULL;
}
