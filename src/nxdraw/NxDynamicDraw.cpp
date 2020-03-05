// NxDynamicDraw.cpp: CNxDynamicDraw �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi / Y.Ojima
//
// �T�v: ���I�R�[�h�ɂ��T�[�t�F�X�������ւ̒��ڕ`��(32bpp ��p)
//
//////////////////////////////////////////////////////////////////////

#include <NxDraw.h>
#include "NxDynamicDraw.h"

using namespace NxDrawLocal;

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxDynamicDraw::CNxDynamicDraw()
 : m_lpExecuteBuffer(NULL)
{
	::InitializeCriticalSection(&m_csExecuteBuffer);
}

CNxDynamicDraw::~CNxDynamicDraw()
{
	// ���s�p�o�b�t�@�����
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
// �T�v: �L���Ȏ��s�o�b�t�@�ւ̃|�C���^��Ԃ�
// ����: �Ȃ�
// �ߒl: ���s�o�b�t�@�ւ̃|�C���^
// ���l: �g�p��� CNxDynamicDraw::UnlockExecuteBuffer() ���Ăяo������
////////////////////////////////////////////////////////////////////////////

LPVOID CNxDynamicDraw::LockExecuteBuffer()
{
	// ���s�o�b�t�@�̃��b�N
	::EnterCriticalSection(&m_csExecuteBuffer);
	if (m_lpExecuteBuffer == NULL)
	{	// �܂��쐬����Ă��Ȃ��c���s�p�o�b�t�@���쐬
		static const DWORD dwExecuteBufferSize = 2048;
		m_lpExecuteBuffer = ::VirtualAlloc(NULL, dwExecuteBufferSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		_ASSERTE(m_lpExecuteBuffer != NULL);
	}
	return m_lpExecuteBuffer;
}
