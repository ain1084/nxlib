// NxMMIOProc.h: CNxMMIOProc �N���X�̃C���^�[�t�F�C�X
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: mmio �̃J�X�^���v���V�[�W���p���ۃN���X
//////////////////////////////////////////////////////////////////////

#pragma once

namespace NxStorageLocal
{
	class __declspec(novtable) CNxMMIOProc
	{
	public:
		CNxMMIOProc();
		~CNxMMIOProc();
		
		HMMIO Create(LPCTSTR lpszFileName, DWORD dwFlags, DWORD dwParam1 = 0, DWORD dwParam2 = 0);

	protected:
		virtual LRESULT Open(LPMMIOINFO lpmmioinfo, LPCSTR lpszFileName) = 0;
		virtual LRESULT Close(LPMMIOINFO lpmmioinfo, UINT fuOption) = 0;
		virtual LRESULT Read(LPMMIOINFO lpmmioinfo, LPVOID lpBuf, LONG lBytesToRead) = 0;
		virtual LRESULT Write(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite) = 0;
		virtual LRESULT WriteFlush(LPMMIOINFO lpmmioinfo, LPCVOID lpBuffer, LONG lBytesToWrite) = 0;
		virtual LRESULT Seek(LPMMIOINFO lpmmioinfo, LONG lOffset, LONG nSeekOrigin) = 0;
		virtual LRESULT User(LPMMIOINFO lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2);
		static LRESULT CALLBACK MMIOProc(LPSTR lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2);
	};
}
