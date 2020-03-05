// NxFPSSprite.cpp: CNxFPSSprite クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: 簡易 FPS 表示スプライト
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <NxDraw/NxWindow.h>
#include <NxDraw/NxHLSColor.h>
#include "NxFPSSprite.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxFPSSprite::CNxFPSSprite(CNxWindow* pParent) : CNxLayerSprite(pParent, 32)
{
	Create(120, 20);		// サイズ適当にサーフェスを作成
	SetZPos(INT_MAX);		// 最上位

	m_font = CNxFont(_T("ＭＳ Ｐゴシック"), -20);
	SetFont(&m_font);

	// テキストのスムージングを行う
	SetTextSmoothing(TRUE);

	// このサーフェスを画面へ転送する為の NxBlt を設定
	NxBlt nxb;
	nxb.dwFlags = NxBlt::srcAlpha;
	SetNxBlt(&nxb);
}

CNxFPSSprite::~CNxFPSSprite()
{

}

// スプライトの更新状態が調べられる直前に CNxSprite クラス内部から呼び出される仮想関数
void CNxFPSSprite::PreUpdate()
{
	CNxWindow* pWindow = static_cast<CNxWindow*>(GetParent());
	int nFPS = pWindow->GetFPS();
	if (nFPS == -1)
		return;		// FPS 変化無し

	// 以前の内容をクリア
	FillRect(NULL, 0);

	// FPS を描画
	TCHAR szBuf[32];
	if (nFPS < 10000 * 1000)
		_stprintf(szBuf, _T("%3.2f FPS"), static_cast<float>(nFPS) / 1000);
	else
		_stprintf(szBuf, _T("%5d FPS"), nFPS / 1000);

	DrawText(2, 2, NULL, szBuf, CNxColor(0, 0, 0, 128));		// 影
	DrawText(0, 0, NULL, szBuf, CNxHLSColor(35, 200, 250));

	// スプライト更新
	SetUpdate();
}
