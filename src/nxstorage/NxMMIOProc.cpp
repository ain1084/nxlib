// NxMMIOProc.cpp: CNxMMIOProc �N���X�̃C���v�������e�[�V����
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: mmio �̃J�X�^���v���V�[�W���p���ۃN���X
//////////////////////////////////////////////////////////////////////

#include <NxStorage.h>
#include "NxMMIOProc.h"

using namespace NxStorageLocal;

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CNxMMIOProc::CNxMMIOProc()
{

}

CNxMMIOProc::~CNxMMIOProc()
{

}


////////////////////////////////////////////////////////////////////////////
// protected:
//	CNxMMIOProc::Create(LPCTSTR lpszFileName, DWORD dwFlags)
// �T�v: �J�X�^�����o�̓v���V�[�W�����g�p����l�� mmio ���I�[�v��
// ����: LPCTSTR lpszFileName ... �t�@�C�����ւ̃|�C���^
//       DWORD dwFlags        ... mmioOpen �֓n���t���O(MMIO_READ ��)
// �ߒl: �����Ȃ� mmio �ւ̃n���h���B����ȊO�� NULL
////////////////////////////////////////////////////////////////////////////

HMMIO CNxMMIOProc::Create(LPCTSTR lpszFileName, DWORD dwFlags, DWORD dwParam1, DWORD dwParam2)
{
	MMIOINFO mmioinfo;
	memset(&mmioinfo, 0, sizeof(mmioinfo));
	mmioinfo.pIOProc = MMIOProc;
	mmioinfo.adwInfo[0] = reinterpret_cast<DWORD>(this);
	mmioinfo.adwInfo[1] = dwParam1;
	mmioinfo.adwInfo[2] = dwParam2;

	// �Ȃ��� UNICODE ���ƃA�N�Z�X�ᔽ�ɂȂ�̂�...
	if (lpszFileName == NULL)
		lpszFileName = _T("");
	
	return ::mmioOpen(const_cast<LPTSTR>(lpszFileName), &mmioinfo, dwFlags);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	static LRESULT CNxMMIOProc::MMIOProc(LPSTR lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2)
// �T�v: mmio �̃J�X�^�����o�̓v���V�[�W��
// ����: LPSTR lpmmioinfo ... MMIOINFO �\���̂ւ̃|�C���^(cast ���K�v)
//       UINT  uMsg       ... ���b�Z�[�W
//       LONG lParam1     ... ����1(�Ӗ��̓��b�Z�[�W�ɂ��قȂ�)
//       LONG lParam2     ... ����2(�Ӗ��̓��b�Z�[�h�ɂ��قȂ�)
// �ߒl: ���s�Ȃ�� - 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxMMIOProc::MMIOProc(LPSTR lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2)
{
	LPMMIOINFO lpmi = reinterpret_cast<LPMMIOINFO>(lpmmioinfo);
	
	CNxMMIOProc* This = reinterpret_cast<CNxMMIOProc*>(lpmi->adwInfo[0]);
	LRESULT lResult = 0;
	switch (uMsg)
	{
	case MMIOM_OPEN:
		lResult = This->Open(lpmi, reinterpret_cast<LPCSTR>(lParam1));// �Ȃ� LPCTSTR �łȂ��̂�...?
		break;
	case MMIOM_CLOSE:
		lResult = This->Close(lpmi, static_cast<UINT>(lParam1));
		break;
	case MMIOM_READ:
		lResult = This->Read(lpmi, reinterpret_cast<LPVOID>(lParam1), lParam2);
		break;
	case MMIOM_WRITE:
		lResult = This->Write(lpmi, reinterpret_cast<const VOID*>(lParam1), lParam2);
		break;
	case MMIOM_WRITEFLUSH:
		lResult = This->WriteFlush(lpmi, reinterpret_cast<const VOID*>(lParam1), lParam2);
		break;
	case MMIOM_SEEK:
		lResult = This->Seek(lpmi, lParam1, lParam2);
		break;
	default:
		lResult = This->User(lpmi, uMsg, lParam1, lParam2);
	}
	return lResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// protected:
//	virtual LRESULT CNxMMIOProc::User(LPMMIOINFO lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2)
// �T�v: mmioSendMessage �֐��ɂ���đ�����A���[�U��`���b�Z�[�W�̉����֐�
// ����: LPMMIOINFO lpmmioinfo ... MMIOINFO �\���̂ւ̃|�C���^
//       UINT uMsg             ... ���b�Z�[�W
//       LONG lParam1          ... �p�����[�^1
//       LONG lParam2          ... �p�����[�^2
// �ߒl: �F�����Ȃ����b�Z�[�W�Ȃ�� 0
//////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CNxMMIOProc::User(LPMMIOINFO /*lpmmioinfo*/, UINT /*uMsg*/, LONG /*lParam1*/, LONG /*lParam2*/)
{
	return 0;
}
