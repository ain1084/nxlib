// NxDynamicDraw.h: CNxDynamicDraw �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// �T�v: ���I�R�[�h�����ɂ��T�[�t�F�X�������ւ̒��ڕ`��
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CNxSurface;
struct NxBlt;

namespace NxDrawLocal
{

class CNxDynamicDraw  
{
public:
	CNxDynamicDraw();
	virtual ~CNxDynamicDraw();

	virtual BOOL Blt(CNxSurface* pDestSurface, const RECT* lpDestRect,
					 const CNxSurface* pSrcSurface, const RECT* lpSrcRect,
					 const NxBlt* pNxBlt) = 0;

protected:
	LPVOID LockExecuteBuffer();
	void UnlockExecuteBuffer();

private:
	LPVOID m_lpExecuteBuffer;
	CRITICAL_SECTION m_csExecuteBuffer;
};

inline void CNxDynamicDraw::UnlockExecuteBuffer() {
	::LeaveCriticalSection(&m_csExecuteBuffer);
}

}	// namespace NxDrawLocal
