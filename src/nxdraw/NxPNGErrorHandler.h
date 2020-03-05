// NxPNGErrorHandler.h
// Copyright(c) 2000 S.Ainoguchi
//
// �T�v: CNxPNGImage Loader / Saver �Ŏg�p���� libpng �p�G���[�����n���h��
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

