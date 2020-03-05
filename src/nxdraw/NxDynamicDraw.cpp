// NxDynamicDraw.cpp: CNxDynamicDraw クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// 概要: 動的コードによるサーフェスメモリへの直接描画(32bpp 専用)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxDynamicDraw.h"

using namespace NxDrawLocal;

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CNxDynamicDraw::CNxDynamicDraw()
 : m_lpExecuteBuffer(NULL)
{
	::InitializeCriticalSection(&m_csExecuteBuffer);
}

CNxDynamicDraw::~CNxDynamicDraw()
{
	// 実行用バッファを解放
	if (m_lpExecuteBuffer != NULL)
	{
		::VirtualFree(m_lpExecuteBuffer, 0, MEM_RELEASE);
		m_lpExecuteBuffer = NULL;
	}
	::DeleteCriticalSection(&m_csExecuteBuffer);
}


////////////////////////////////////////////////////////////////////////////
// protected:
//	LPVOID CNxDynamicDraw::LockExecuteBuffer()
// 概要: 有効な実行バッファへのポインタを返す
// 引数: なし
// 戻値: 実行バッファへのポインタ
// 備考: 使用後は CNxDynamicDraw::UnlockExecuteBuffer() を呼び出すこと
////////////////////////////////////////////////////////////////////////////

LPVOID CNxDynamicDraw::LockExecuteBuffer()
{
	// 実行バッファのロック
	::EnterCriticalSection(&m_csExecuteBuffer);
	if (m_lpExecuteBuffer == NULL)
	{	// まだ作成されていない…実行用バッファを作成
		static const DWORD dwExecuteBufferSize = 2048;
		m_lpExecuteBuffer = ::VirtualAlloc(NULL, dwExecuteBufferSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		_ASSERTE(m_lpExecuteBuffer != NULL);
	}
	return m_lpExecuteBuffer;
}
