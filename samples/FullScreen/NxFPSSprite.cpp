// NxFPSSprite.cpp: CNxFPSSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 簡易 FPS 表示スプライト
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxFPSSprite.h"
#include <NxDraw/NxScreen.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxFPSSprite::CNxFPSSprite(CNxScreen* pParent) : CNxLayerSprite(pParent, 8)
{
	Create(140, 24);		// サイズ適当にサーフェスを作成
	SetZPos(INT_MAX);		// 最上位

	m_font = CNxFont(_T("ＭＳ Ｐゴシック"), -24);
	SetFont(&m_font);

	SetTextSmoothing(TRUE);
	
	// 8bpp サーフェスへ描画した結果を画面へ転送する為の NxBlt を設定
	NxBlt nxb;
	nxb.dwFlags = NxBlt::colorFill | NxBlt::srcAlpha;	// 転送元アルファ付き塗り潰し
	nxb.nxbColor = CNxColor::white;						// 塗り潰し色
	SetNxBlt(&nxb);
}

CNxFPSSprite::~CNxFPSSprite()
{
}

// スプライトの更新状態が調べられる直前に CNxSprite クラス内部から呼び出される仮想関数
void CNxFPSSprite::PreUpdate()
{
	CNxScreen* pScreen = static_cast<CNxScreen*>(GetParent());
	int nFPS = pScreen->GetFPS();
	if (nFPS == -1)
		return;		// FPS 変化無し

	// 以前の内容をクリア
	FillRect(NULL, 0);

	// FPS を描画
	CString str;
	if (nFPS < 10000 * 1000)
		str.Format(_T("%3.2f FPS"), static_cast<float>(nFPS) / 1000);
	else
		str.Format(_T("%5d FPS"), nFPS / 1000);		// 10000 FPS 以上ならば小数点は表示しない
	DrawText(0, 0, NULL, str, static_cast<const NxBlt*>(NULL));
	
	// スプライト更新
	SetUpdate();
}
