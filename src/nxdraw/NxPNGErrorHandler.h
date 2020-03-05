// NxPNGErrorHandler.h
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: CNxPNGImage Loader / Saver で使用する libpng 用エラー処理ハンドラ
//
////////////////////////////////////////////////////////////////////////////

#include "libpng/png.h"

namespace NxDrawLocal
{
	namespace NxPNGErrorHandler
	{
		void ShowErrorMessageBox(png_structp png_ptr, png_const_charp msg, LPCTSTR lpszCaption, UINT uMsgBoxFlags);
		void UserError(png_structp png_ptr, png_const_charp error_msg);
		void UserWarning(png_structp png_ptr, png_const_charp warning_msg);
	}
}

