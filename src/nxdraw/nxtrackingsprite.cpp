// NxTrackingSprite.cpp: CNxTrackingSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: CNxSprite 派生クラス。差分追跡機能付き最上位スプライトクラス
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxUpdateRegion.h"
#include "NxTrackingSprite.h"

class CNxTrackingSprite::CNxTrackingUpdateRegion : public NxDrawLocal::CNxUpdateRegion
{
public:
	enum
	{	// デフォルトの単位
		DefaultXUnit = 32,
		DefaultYUnit =  8
	};


	CNxTrackingUpdateRegion(CNxSprite* pSprite, int nWidth, int nHeight, int nXUnit = DefaultXUnit, int nYUnit = DefaultYUnit);
	virtual ~CNxTrackingUpdateRegion();

	// CNxUpdateRegion override
	virtual void AddRect(const RECT* lpRect);

	void EnumRect(int nMaxWidth, int nMaxHeight, LPVOID lpContext);
	int GetWidth() const {
		return m_nRealWidth; }
	int GetHeight() const {
		return m_nRealHeight; }
	void Invalidate() {
		memset(m_pbFlag, 1, m_nLength); }
	static void GetUnitShiftRound(int nUnit, UINT* lpuShift, UINT* lpuRound);

private:

	inline int GetLineLength(const BYTE* p, int nX, int nMaxWidth) const;

	int m_nWidth;					// フラグの物理的な幅と高さ
	int m_nHeight;
	int m_nRealWidth;				// コンストラクタへ渡された真の幅と高さ
	int m_nRealHeight;
	int m_nLength;
	UINT m_uXShift;
	UINT m_uXRound;
	UINT m_uYShift;
	UINT m_uYRound;
	LPBYTE m_pbFlag;
	CNxSprite* m_pSpriteOwner;		// このオブジェクトを所有する CNxSprite クラス

	CNxTrackingUpdateRegion& operator=(const CNxTrackingUpdateRegion&);
	CNxTrackingUpdateRegion(const CNxTrackingUpdateRegion&);
};

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxTrackingSprite::CNxTrackingSprite()
 : CNxSprite(NULL /*常に最上位*/)
 , m_bTracking(FALSE)
{
	m_sizeTrackingUnit.cx = CNxTrackingUpdateRegion::DefaultXUnit;
	m_sizeTrackingUnit.cy = CNxTrackingUpdateRegion::DefaultYUnit;
}

CNxTrackingSprite::~CNxTrackingSprite()
{

}

///////////////////////////////////////////////////////////////////////////////////////////
// private:
//	BOOL CNxTrackingSprite::CreateDrawRect(BOOL bForce)
// 概要: CNxDrawRect オブジェクトを作成。既に作成済みであり、サイズが同じならば初期化しない
// 引数: BOOL bForce  .... 強制的に再生成するならば TRUE
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CreateDrawRect(BOOL bForce)
{
	if (!bForce)
	{
		if (m_pUpdateRegion.get() != 0)
		{	// 作成済みの場合...
			if ((m_pUpdateRegion->GetWidth() - GetWidth() | m_pUpdateRegion->GetHeight() - GetHeight()) == 0)
			{	// サイズが同じならば再生成せずに内容の初期化のみ
				m_pUpdateRegion->Invalidate();
				return;
			}
		}
	}
	// 再生成
	if (GetWidth() == 0 || GetHeight() == 0)
	{
		m_pUpdateRegion.reset();
	}
	else
	{
		m_pUpdateRegion.reset(new CNxTrackingUpdateRegion(this, GetWidth(), GetHeight(), m_sizeTrackingUnit.cx, m_sizeTrackingUnit.cy));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	BOOL CNxTrackingSprite::Refresh(int nMaxWidthOfEnum, int nMaxHeightOfEnum, LPVOID lpContext, BOOL bForce = FALSE)
// 概要: 追跡によって生じた差分の描画
//		 int nMaxWidthOfEnum  ... 列挙される矩形の最大の幅
//       int nMaxHeightOfEnum ... 列挙される矩形の最大の高さ
//		 LPVOID lpContext     ... コンテキスト(任意のポインタ等)へのポインタ
//       BOOL bForce          ... TRUE にすると強制的に全域を再描画(全領域が再描画対象)
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::Refresh(int nMaxWidthOfEnum, int nMaxHeightOfEnum, LPVOID lpContext, BOOL bForce) 
{
	_ASSERT(nMaxWidthOfEnum > 0 && nMaxHeightOfEnum > 0);
	
	BOOL bUpdate = FALSE;
	BOOL bDoPreUpdate = TRUE;

	if (bForce)
	{
		// 更新矩形を全体として設定
		if (m_pUpdateRegion.get() != 0)
		{
			m_pUpdateRegion->Invalidate();
		}

		// bFore = true ならば CNxSprite::PreUpdate() は呼び出さない
		bDoPreUpdate = FALSE;

		// 強制更新
		bUpdate = TRUE;
	}

	if (GetUpdateRegion(m_pUpdateRegion.get(), bDoPreUpdate) | bUpdate)
	{
		// 処理開始
		if (!RefreshBegin(lpContext))
		{
			return FALSE;
		}
		if (m_pUpdateRegion.get() == 0)
		{	// 差分追跡を行わない
			nMaxWidthOfEnum = static_cast<UINT>(max(nMaxWidthOfEnum + m_sizeTrackingUnit.cx - 1, m_sizeTrackingUnit.cx));
			nMaxWidthOfEnum = static_cast<UINT>(nMaxWidthOfEnum) / m_sizeTrackingUnit.cx * m_sizeTrackingUnit.cx;
			nMaxHeightOfEnum = static_cast<UINT>(max(nMaxHeightOfEnum + m_sizeTrackingUnit.cy - 1, m_sizeTrackingUnit.cy));
			nMaxHeightOfEnum = static_cast<UINT>(nMaxHeightOfEnum) / m_sizeTrackingUnit.cy * m_sizeTrackingUnit.cy;

			RECT rect;
			for (rect.top = 0; rect.top < GetHeight(); rect.top += nMaxHeightOfEnum)
			{
				rect.bottom = min(GetHeight(), rect.top + nMaxHeightOfEnum);
				for (rect.left = 0; rect.left < GetWidth(); rect.left += nMaxWidthOfEnum)
				{
					rect.right = min(GetWidth(), rect.left + nMaxWidthOfEnum);
					RefreshRect(&rect, lpContext);
				}
			}
		}
		else
		{	// 差分追跡を行う
			m_pUpdateRegion->EnumRect(nMaxWidthOfEnum, nMaxHeightOfEnum, lpContext);
		}
		// 処理終了
		RefreshEnd(lpContext);
	}
	m_fps.Increment();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual BOOL CNxTrackingSprite::RefreshBegin(LPVOID lpContext) const
// 概要: 更新矩形がある場合、矩形の列挙される直前に呼び出される関数
// 引数: Refresh() 関数へ渡された lpContext 引数
// 戻値: TRUE ならば矩形の列挙を開始、FALSE ならば中止
//////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::RefreshBegin(LPVOID /*lpContext*/) const
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual void CNxTrackingSprite::RefreshEnd(LPVOID lpContext) const
// 概要: 更新矩形がある場合、全ての矩形の列挙を終了した後に呼び出される関数
// 引数: Refresh() 関数へ渡された lpContext 引数
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::RefreshEnd(LPVOID /*lpContext*/) const
{

}


//////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxTrackingSprite::SetRect(const RECT* lpRect)
// 概要: スプライトの矩形を設定
// 引数: const RECT* lpRect ... 設定する矩形(NULL ならば空になる)
// 戻値: 成功なら TRUE
//////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::SetRect(const RECT* lpRect)
{
	if (!CNxSprite::SetRect(lpRect))
		return FALSE;

	// 追跡が有効ならば、CNxDrawRect オブジェクトも作り替える
	if (m_bTracking)
		CreateDrawRect(FALSE);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	virtual void CNxTrackingSprite::SetTrackingUnit(int nXUnit, int nYUnit)
// 概要: 差分追跡の単位を設定
// 引数: int nXUnit ... 横方向の単位
//       int nYUnit ... 縦方向の単位
// 戻値: 成功なら TRUE
///////////////////////////////////////////////////////////////////////////////

BOOL CNxTrackingSprite::SetTrackingUnit(int nXUnit, int nYUnit)
{
	// それぞれ 2^n サイズへ切り上げる
	UINT uXRound;
	UINT uYRound;
	CNxTrackingUpdateRegion::GetUnitShiftRound(nXUnit, NULL, &uXRound);		// 加算値のみを取得
	CNxTrackingUpdateRegion::GetUnitShiftRound(nYUnit, NULL, &uYRound);
	m_sizeTrackingUnit.cx = uXRound + 1;
	m_sizeTrackingUnit.cy = uYRound + 1;

	// CNxDrawRect を強制的に再生成
	if (m_bTracking)
		CreateDrawRect(TRUE);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxTrackingSprite::EnableTracking(BOOL bEnable)
// 概要: 差分追跡を有効/無効にする
// 引数: BOOL bEnable ... TRUE ならば差分追跡を有効にする
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::EnableTracking(BOOL bEnable) 
{
	if (bEnable == m_bTracking)
		return;	// 現在と同じ

	m_bTracking = bEnable;

	if (!m_bTracking)
	{	// 無効化
		m_pUpdateRegion.reset();
	}
	else
	{	// 有効化
		CreateDrawRect(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// public:
//	int CNxTrackingSprite::GetFPS()
// 概要: FPS (Frame per second) 値を返す
// 引数: なし
// 戻値: 直前の FPS を 1000倍した値または -1
//       連続して呼ばれた場合、約1秒間隔で FPS を返す
//       一度取得すると次回更新時までは -1 を返す
////////////////////////////////////////////////////////////////////////////////////

int CNxTrackingSprite::GetFPS()
{
	int result = m_fps.Get();
	m_fps.Clear();
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
// 
// CNxTrackingSprite::CNxFPS
//
// FPS 計測用ローカルクラス
//
////////////////////////////////////////////////////////////////////////////////////////

CNxTrackingSprite::CNxFPS::CNxFPS()
{
	Reset();
}


////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxTrackingSprite::CNxFPS::Increment()
// 概要: FPS 内部カウンタを増加する。画面更新毎に呼ぶ
// 引数: なし
// 戻値: なし
////////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxFPS::Increment()
{
	m_nRefreshCount++;
	DWORD dwCurrentTime = ::GetTickCount();
	if (m_dwPrevTime + 1000 <= dwCurrentTime)
	{
		m_nFPS = static_cast<int>(static_cast<DWORDLONG>(m_nRefreshCount) * 1000000i64
								  / static_cast<DWORDLONG>(dwCurrentTime - m_dwPrevTime));
		m_dwPrevTime = dwCurrentTime - (dwCurrentTime - m_dwPrevTime - 1000);
		m_nRefreshCount = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxTrackingSprite::CNxFPS::Reset()
// 概要: 内部カウンタを初期化する。FPS は未計測状態になる
// 引数: なし
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxFPS::Reset()
{
	m_nRefreshCount = 0;				// 更新カウント数
	m_dwPrevTime = ::GetTickCount();	// 以前の計測開始時刻
	m_nFPS = -1;						// FPS 結果(-1 = 未計測)
}


//////////////////////////////////////////////////////////////////////////////
// public:
//	static void CNxDrawRect::GetUnitShiftRound(int nUnit, UINT* lpuShift, UINT* lpuRound)
// 概要: 分割単位をシフト回数と加算値へ変換
// 引数: int  nUnit     ... 分割単位(2^n である事)
//       UINT* lpuShift ... シフト回数を受け取る UNIT 型変数へのポインタ(NULL可)
//       UINT* lpuRound ... 加算値を受け取る UNIT 型変数へのポインタ(NULL可)
// 戻値: なし
// 備考: nUnit  shift  round
//           1      0      0
//           2      1      1
//           4      2      3
//           8      3      7
//          16      4     15
//                  :
//                  :
//       nUnit が 2^n でない場合、適当に丸める
///////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxTrackingUpdateRegion::GetUnitShiftRound(int nUnit, UINT* lpuShift, UINT* lpuRound)
{
	UINT uShift = 0;
	UINT uRound;
	if (--nUnit != 0)
	{	// uUnit != 1
		while (!(nUnit & 0x80000000))
		{
			nUnit <<= 1;
			uShift++;
		}
		uRound = 0xffffffff >> uShift;
	}
	else
	{	// nUnit == 1
		uShift = 32;
		uRound = 0x0;
	}
	
	
	if (lpuRound != NULL)
		*lpuRound = uRound;

	if (lpuShift != NULL)
		*lpuShift = 32 - uShift;
}

///////////////////////////////////////////////////////////////////////////////
// public:
//	CNxDrawRect::CNxDrawRect(UINT uWidth, UINT uHeight)
// 概要: CNxDrawRect クラスの構築
// 引数: UINT uWidth  ... 幅
//       UINT uHeight ... 高さ
// 戻値: ---
///////////////////////////////////////////////////////////////////////////////

CNxTrackingSprite::CNxTrackingUpdateRegion::CNxTrackingUpdateRegion(CNxSprite* pSpriteOwner, int nWidth, int nHeight, int nXUnit, int nYUnit)
{
	_ASSERTE(nWidth > 0 && nHeight > 0);
	
	// 真の幅と高さを保存
	m_nRealWidth = nWidth;
	m_nRealHeight = nHeight;

	m_pSpriteOwner = pSpriteOwner;

	GetUnitShiftRound(nXUnit, &m_uXShift, &m_uXRound);
	GetUnitShiftRound(nYUnit, &m_uYShift, &m_uYRound);
	
	// ブロック単位の幅と高さ
	m_nWidth = (nWidth + m_uXRound) >> m_uXShift;
	m_nHeight = (nHeight + m_uYRound) >> m_uYShift;

	// メモリを確保する
	m_nLength = m_nWidth * m_nHeight;
	m_pbFlag = new BYTE[m_nLength];

	// 全てフラグを立てた状態にする
	Invalidate();
}


CNxTrackingSprite::CNxTrackingUpdateRegion::~CNxTrackingUpdateRegion()
{
	delete []m_pbFlag;
}

///////////////////////////////////////////////////////////////////////////////////////
// private:
//	inline UINT CNxDrawRect::GetLineLength(const BYTE *p, int nX) const
// 概要: 指定された行の更新フラグの長さを返す(行の端で停止)
// 引数: const BYTE *p  ... 長さを調べる更新フラグの行の左端ポインタ
//       int nX         ... 検索を開始する x 座標
//       int nMaxWidth  ... 最大幅
// 戻値: フラグの長さ
// 備考: 内部用インライン関数
///////////////////////////////////////////////////////////////////////////////////////

int CNxTrackingSprite::CNxTrackingUpdateRegion::GetLineLength(const BYTE* p, int nX, int nMaxWidth) const
{
	p += nX;
	const BYTE *ps = p;
	int nMax = m_nWidth - nX;
	nMax = min(nMax, nMaxWidth);

	while (--nMax >= 0 && *p != 0)
		p++;
	return p - ps;
}


///////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDrawRect::SetRect(const RECT* lpRect)
// 概要: 指定された矩形のフラグを立てる
// 引数: const RECT* lpRect ... フラグを立てる矩形
// 戻値: なし
///////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxTrackingUpdateRegion::AddRect(const RECT* lpRect)
{
	RECT rect;
	// フラグ座標系へ変換
	rect.left = lpRect->left >> m_uXShift;
	rect.top = lpRect->top >> m_uYShift;
	rect.right = (lpRect->right + m_uXRound) >> m_uXShift;
	rect.bottom = (lpRect->bottom + m_uYRound) >> m_uYShift;

	// for bug
	_ASSERTE(rect.left >= 0 && rect.right <= m_nWidth);
	_ASSERTE(rect.top >= 0 && rect.bottom <= m_nHeight);

	LPBYTE lpFlag = m_pbFlag + rect.left + (rect.top * m_nWidth);
	for (int n = 0; n < rect.bottom - rect.top; n++)
	{
		memset(lpFlag, 1, rect.right - rect.left);
		lpFlag += m_nWidth;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// public:
//	void CNxDrawRect::EnumRect(BOOL bScanY, EnumRectProc pfnEnumRectProc, LPVOID lpContext)
// 概要: フラグを調査して矩形が見つかる度に pfnEnumRectProc 関数を呼び出す
// 引数: UINT uMaxWidth           ... 列挙される矩形を制限する最大の幅
//       EnumRectProc pfnEnumRect ... 呼び出される関数
//       LPVOID lpContext         ... コンテキスト
// 戻値: なし
/////////////////////////////////////////////////////////////////////////////////////////////////

void CNxTrackingSprite::CNxTrackingUpdateRegion::EnumRect(int nMaxWidth, int nMaxHeight, LPVOID lpContext)
{
	_ASSERTE(nMaxWidth > 0 && nMaxHeight > 0);
	
	// 最小幅と高さを合わせる
	nMaxWidth = (nMaxWidth + m_uXRound) >> m_uXShift;
	nMaxHeight = (nMaxHeight + m_uYRound) >> m_uYShift;
	nMaxWidth = (nMaxWidth == 0) ? 1 : nMaxWidth;
	nMaxHeight = (nMaxHeight == 0) ? 1 : nMaxHeight;

	LPBYTE pbScanFlag;
	int sx, sy, nx, ny;

	pbScanFlag = m_pbFlag;
	for (sy = 0; sy < m_nHeight; pbScanFlag += m_nWidth, sy++)
	{
		// 横方向に分断されている矩形を右隣と結合して(気持ち)平す
		for (sx = 0; sx < m_nWidth - 2; sx += 2)
		{
			*(pbScanFlag + sx + 1) |= *(pbScanFlag + sx + 0) & *(pbScanFlag + sx + 2);
		}

		for (sx = 0; sx < m_nWidth; sx++)
		{
			if (*(pbScanFlag + sx) != 0)
			{
				// フラグを発見した
				nx = GetLineLength(pbScanFlag, sx, nMaxWidth);					// 開始ラインのフラグ長を数える
				memset(pbScanFlag + sx, 0, nx);									// フラグ降ろす
				int nScanLimit = min(m_nHeight - sy, nMaxHeight);				// 縦方向への探索 Limit

				LPBYTE pbStartLine = pbScanFlag + m_nWidth;
				for (ny = 1; ny < nScanLimit; ny++, pbStartLine += m_nWidth)
				{
					// 左端でなければ 左が 0(境界)である事を確認する
					if (sx != 0 && *(pbStartLine + sx - 1) != 0)
						break;

					// 行のフラグ長を数えて異なれば中止
					if (GetLineLength(pbStartLine, sx, nMaxWidth) != nx)
						break;

					// フラグを降ろす
					memset(pbStartLine + sx, 0, nx);
				}

				// (Pixel単位の)更新座標へ変換
				RECT rcConv;
				rcConv.top  = sy << m_uYShift;
				rcConv.bottom  = min((sy + ny) << m_uYShift, m_nRealHeight);
				rcConv.left    = sx << m_uXShift;
				rcConv.right   = min((sx + nx) << m_uXShift, m_nRealWidth);
				sx += nx - 1;

				// 列挙関数呼び出し
				static_cast<CNxTrackingSprite*>(m_pSpriteOwner)->RefreshRect(&rcConv, lpContext);
			}
		}
	}
}
