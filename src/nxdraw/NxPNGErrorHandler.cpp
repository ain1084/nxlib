// NxPNGErrorHandler.cpp
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: CNxPNGImage Loader / Saver で使用する libpng 用エラー処理ハンドラ
//
////////////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxPNGErrorHandler.h"
#include "libpng/png.h"

namespace NxDrawLocal
{
namespace NxPNGErrorHandler
{

void ShowErrorMessageBox(png_structp png_ptr, png_const_charp msg, LPCTSTR lpszCaption, UINT uMsgBoxFlags)
{
	static const int BUFFER_SIZE = 1024;
	TCHAR szBuffer[BUFFER_SIZE];
	CNxFile* pFile = static_cast<CNxFile*>(png_ptr->error_ptr);
	int nNext;
#if defined(UNICODE)
	mbstowcs(szBuffer, BUFFER_SIZE, msg, strlen(msg) + 1);
	nNext = wcslen(szBuffer);
#else
	strcpy_s(szBuffer, BUFFER_SIZE, msg);
	nNext = strlen(szBuffer);
#endif
	_tcscpy_s(&szBuffer[nNext], BUFFER_SIZE - nNext, _T("\n\nCNxFile object:\n"));
	nNext = _tcslen(szBuffer);
	pFile->GetFileName(&szBuffer[nNext], BUFFER_SIZE - nNext);
	::MessageBox(CNxDraw::GetInstance()->GetFrameWnd(), szBuffer, lpszCaption, uMsgBoxFlags);
}

void UserError(png_structp png_ptr, png_const_charp error_msg)
{
	ShowErrorMessageBox(png_ptr, error_msg, _T("libpng error"), MB_ICONERROR|MB_OK);
}

void UserWarning(png_structp png_ptr, png_const_charp warning_msg)
{
	ShowErrorMessageBox(png_ptr, warning_msg, _T("libpng warning"), MB_ICONWARNING|MB_OK);
}

}	// namespace NxPNGErrorHandler

}	// namespace NxDrawLocal

