// NxJPEGImageLoader.cpp: CNxJPEGImageLoader クラスのインプリメンテーション
// Copyright(c) 2000 S.Ainoguchi
//
// 概要: JPEG 画像の読み込み CNxDIBImage を返す、CNxDIBImageLoader 派生クラス
//
//////////////////////////////////////////////////////////////////////

#include "NxDraw.h"
#include <memory>
#include <NxStorage/NxStorage.h>
#include <NxStorage/NxFile.h>
#include "NxDIBImage.h"
#include "NxJPEGImageLoader.h"

extern "C"
{
#if _MSC_VER >= 1100
#define XMD_H		/* VC++6.0 以上ならば、どこかで INT32 が宣言されている(らしい) */
#endif

#include "jpeglib/jinclude.h"
#undef FAR		/* windows.h 内で定義されるので */
#include "jpeglib/jpeglib.h"
#include "jpeglib/jerror.h"
}
#include <setjmp.h>

namespace
{
	// 入力バッファサイズ
	const int INPUT_BUF_SIZE = 4096;
	
	// jmp_buf をつけ足した jpeg_error_mgr 型
	typedef struct
	{
		jpeg_error_mgr pub;
		jmp_buf env;
	} nxjpeg_error_mgr;

	// ソースマネージャ構造体(CNxFile へのポインタを追加)
	typedef struct
	{
		jpeg_source_mgr pub;
		CNxFile* pFile;
		JOCTET* buffer;
		boolean start_of_file;
	} my_source_mgr;
	typedef my_source_mgr* my_src_ptr;

	// exit method のすり替え(jpeglib/JERROR.C 参照)
	static void jerror_exit(j_common_ptr cinfo)
	{
		(*cinfo->err->output_message)(cinfo);
		nxjpeg_error_mgr* err = (nxjpeg_error_mgr*)cinfo->err;
		longjmp(err->env, err->pub.msg_code);
	}

	// output_message method のすり替え(jpeglib/JERROR.C 参照)
	static void jerror_output_message(j_common_ptr cinfo)
	{
		char buffer[JMSG_LENGTH_MAX];
		(*cinfo->err->format_message)(cinfo, buffer);
	#if defined(_DEBUG)
		::MessageBoxA(CNxDraw::GetInstance()->GetFrameWnd(), buffer, "jpeglib error", MB_ICONEXCLAMATION|MB_OK);
	#endif
	}

	// JPEG 読み込み用、差し替えソースマネージャ

	/* Expanded data source object for memory block input */

	/*
	 * Initialize source --- called by jpeg_read_header
	 * before any data is actually read.
	 */


	void
	//METHODDEF(void)
	init_source (j_decompress_ptr cinfo)
	{
		my_src_ptr src = (my_src_ptr) cinfo->src;
	  /* We reset the empty-input-file flag for each image,
	   * but we don't clear the input buffer.
	   * This is correct behavior for reading a series of images from one source.
	   */
		src->start_of_file = TRUE;
	}


	/*
	 * Fill the input buffer --- called whenever buffer is emptied.
	 *
	 * In typical applications, this should read fresh data into the buffer
	 * (ignoring the current state of next_input_byte & bytes_in_buffer),
	 * reset the pointer & count to the start of the buffer, and return TRUE
	 * indicating that the buffer has been reloaded.  It is not necessary to
	 * fill the buffer entirely, only to obtain at least one more byte.
	 *
	 * There is no such thing as an EOF return.  If the end of the file has been
	 * reached, the routine has a choice of ERREXIT() or inserting fake data into
	 * the buffer.  In most cases, generating a warning message and inserting a
	 * fake EOI marker is the best course of action --- this will allow the
	 * decompressor to output however much of the image is there.  However,
	 * the resulting error message is misleading if the real problem is an empty
	 * input file, so we handle that case specially.
	 *
	 * In applications that need to be able to suspend compression due to input
	 * not being available yet, a FALSE return indicates that no more data can be
	 * obtained right now, but more may be forthcoming later.  In this situation,
	 * the decompressor will return to its caller (with an indication of the
	 * number of scanlines it has read, if any).  The application should resume
	 * decompression after it has loaded more data into the input buffer.  Note
	 * that there are substantial restrictions on the use of suspension --- see
	 * the documentation.
	 *
	 * When suspending, the decompressor will back up to a convenient restart point
	 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
	 * indicate where the restart point will be if the current call returns FALSE.
	 * Data beyond this point must be rescanned after resumption, so move it to
	 * the front of the buffer rather than discarding it.
	 */

	boolean
	//METHODDEF(boolean)
	fill_input_buffer (j_decompress_ptr cinfo)
	{
		my_src_ptr src = (my_src_ptr) cinfo->src;
		size_t nbytes;

		nbytes = src->pFile->Read(src->buffer, INPUT_BUF_SIZE);

		if (nbytes <= 0) {
			if (src->start_of_file)	/* Treat empty input file as fatal error */
				ERREXIT(cinfo, JERR_INPUT_EMPTY);
			WARNMS(cinfo, JWRN_JPEG_EOF);
			/* Insert a fake EOI marker */
			src->buffer[0] = (JOCTET) 0xFF;
			src->buffer[1] = (JOCTET) JPEG_EOI;
			nbytes = 2;

		}
		src->pub.next_input_byte = src->buffer;
		src->pub.bytes_in_buffer = nbytes;
		return TRUE;
	}


	/*
	 * Skip data --- used to skip over a potentially large amount of
	 * uninteresting data (such as an APPn marker).
	 *
	 * Writers of suspendable-input applications must note that skip_input_data
	 * is not granted the right to give a suspension return.  If the skip extends
	 * beyond the data currently in the buffer, the buffer can be marked empty so
	 * that the next read will cause a fill_input_buffer call that can suspend.
	 * Arranging for additional bytes to be discarded before reloading the input
	 * buffer is the application writer's problem.
	 */

	void
	//METHODDEF(void)
	skip_input_data (j_decompress_ptr cinfo, long num_bytes)
	{
	  my_src_ptr src = (my_src_ptr) cinfo->src;

	  /* Just a dumb implementation for now.  Could use fseek() except
	   * it doesn't work on pipes.  Not clear that being smart is worth
	   * any trouble anyway --- large skips are infrequent.
	   */
	  if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
		  num_bytes -= (long) src->pub.bytes_in_buffer;
		  (void) fill_input_buffer(cinfo);
		  /* note we assume that fill_input_buffer will never return FALSE,
		   * so suspension need not be handled.
		   */
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	  }
	}

	/*
	 * An additional method that can be provided by data source modules is the
	 * resync_to_restart method for error recovery in the presence of RST markers.
	 * For the moment, this source module just uses the default resync method
	 * provided by the JPEG library.  That method assumes that no backtracking
	 * is possible.
	 */


	/*
	 * Terminate source --- called by jpeg_finish_decompress
	 * after all data has been read.  Often a no-op.
	 *
	 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
	 * application must deal with any cleanup that should happen even
	 * for error exit.
	 */

	void
	//METHODDEF(void)
	term_source (j_decompress_ptr /*cinfo*/)
	{
		/* no work necessary here */
	}


	/*
	 * Prepare for input from a stdio stream.
	 * The caller must have already opened the stream, and is responsible
	 * for closing it after finishing decompression.
	 */

	void jpeg_nxfile_src (j_decompress_ptr cinfo, CNxFile* pFile)
	{
		my_src_ptr src;

	  /* The source object and input buffer are made permanent so that a series
	   * of JPEG images can be read from the same file by calling jpeg_stdio_src
	   * only before the first one.  (If we discarded the buffer at the end of
	   * one image, we'd likely lose the start of the next one.)
	   * This makes it unsafe to use this manager and a different source
	   * manager serially with the same JPEG object.  Caveat programmer.
	   */
	  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
		  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					  SIZEOF(my_source_mgr));
		src = (my_src_ptr) cinfo->src;
		src->buffer = (JOCTET *)
		  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					  INPUT_BUF_SIZE * SIZEOF(JOCTET));
	  }

	  src = (my_src_ptr) cinfo->src;
	  src->pub.init_source = init_source;
	  src->pub.fill_input_buffer = fill_input_buffer;
	  src->pub.skip_input_data = skip_input_data;
	  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	  src->pub.term_source = term_source;
	  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	  src->pub.next_input_byte = NULL; /* until buffer loaded */
	  src->pFile = pFile;
	}
}

//////////////////////////////////////////////////////////////////////
// public:
//	CNxJPEGImageLoader::CNxJPEGImageLoader()
// 概要: CNxJPEGImageLoader クラスのデフォルトコンストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxJPEGImageLoader::CNxJPEGImageLoader()
{

}

//////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxJPEGImageLoader::~CNxJPEGImageLoader()
// 概要: CNxJPEGImageLoader クラスのデストラクタ
// 引数: なし
// 戻値: なし
//////////////////////////////////////////////////////////////////////

CNxJPEGImageLoader::~CNxJPEGImageLoader()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual BOOL CNxJPEGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
// 概要: 展開可能なデータ形式であるかを調べる
// 引数: LPCVOID lpvBuf ... データの最初から 2048byte を読み込んだバッファへのポインタ
//       LONG lLength   ... データのサイズ(通常は 2048)
// 戻値: 展開可能であれば TRUE
///////////////////////////////////////////////////////////////////////////////////////

BOOL CNxJPEGImageLoader::IsSupported(LPCVOID lpvBuf, LONG lLength) const
{
	// 先頭の SOI マーカーのみを調査...
	static const BYTE byCheckSOI[] = { 0xff, 0xd8 };
	
	if (lLength < 2)
		return FALSE;

	return memcmp(lpvBuf, byCheckSOI, 2) == 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// public:
//	virtual CNxDIBImage* CNxJPEGImageLoader::CreateDIBImage(CNxFile& nxfile)
// 概要: 画像を展開して CNxDIBImage オブジェクトを返す
// 引数: CNxFile& nxfile ... CNxFile オブジェクトへの参照
// 戻値: 成功ならば、作成した CNxDIBImage オブジェクトへのポインタ。失敗ならば NULL
///////////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4611)

CNxDIBImage* CNxJPEGImageLoader::CreateDIBImage(CNxFile& nxfile)
{
	jpeg_decompress_struct cinfo;
	nxjpeg_error_mgr jerr;
	int i;

	// decompress 準備 / エラーマネージャ設定
	jpeg_create_decompress(&cinfo);
	cinfo.err = jpeg_std_error((struct jpeg_error_mgr*)&jerr);
	cinfo.err->error_exit = jerror_exit;
	cinfo.err->output_message = jerror_output_message;
	jpeg_nxfile_src(&cinfo, &nxfile);

	if (setjmp(jerr.env) != 0)
	{
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}

	// ヘッダの読み込み
	jpeg_read_header(&cinfo, TRUE);

	int cPalette, cBitsPerPixel;

	// set DCT method (float)
	cinfo.dct_method = JDCT_FLOAT;
	if (cinfo.out_color_space == JCS_RGB)
	{
		if (cinfo.quantize_colors)
		{	// Colormapped RGB
			cBitsPerPixel = 8;
			cPalette = 256;
		}
		else
		{	// full color
			cBitsPerPixel = 24;
			cPalette = 0;
		}
	}
	else
	{	// Grayscale
		cBitsPerPixel = 8;
		cPalette = 256;
	}
	jpeg_calc_output_dimensions(&cinfo);

	// BITMAPINFO 構造体を準備
	LPBITMAPINFO lpbmi = (LPBITMAPINFO)_alloca(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * cPalette);
	lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbmi->bmiHeader.biWidth = cinfo.output_width;
	lpbmi->bmiHeader.biHeight = cinfo.output_height;
	lpbmi->bmiHeader.biPlanes = 1;
	lpbmi->bmiHeader.biBitCount = static_cast<WORD>(cBitsPerPixel);
	lpbmi->bmiHeader.biCompression = BI_RGB;
	lpbmi->bmiHeader.biSizeImage = 0;
	lpbmi->bmiHeader.biXPelsPerMeter = 0;
	lpbmi->bmiHeader.biYPelsPerMeter = 0;
	lpbmi->bmiHeader.biClrUsed = cPalette;
	lpbmi->bmiHeader.biClrImportant = 0;

	// パレットデータ構築
	if (cPalette != 0)
	{
		RGBQUAD *pPalette = lpbmi->bmiColors;
		JSAMPARRAY colormap = cinfo.colormap;
		int nColors = cinfo.actual_number_of_colors;
		if (cinfo.colormap != NULL)
		{
			if (cinfo.out_color_components == 3)
			{	// normal RGB colormap
				for (i = 0; i < nColors; i++)
				{
					pPalette->rgbRed = colormap[RGB_RED][i];
					pPalette->rgbGreen = colormap[RGB_GREEN][i];
					pPalette->rgbBlue = colormap[RGB_BLUE][i];
					pPalette->rgbReserved = 0;
					pPalette++;
				}
			}
			else
			{	// grayscale
				for (i = 0; i < nColors; i++)
				{
					pPalette->rgbRed = colormap[RGB_BLUE][i];
					pPalette->rgbGreen = colormap[RGB_BLUE][i];
					pPalette->rgbBlue = colormap[RGB_BLUE][i];
					pPalette->rgbReserved = 0;
					pPalette++;
				}
			}
		}
		else
		{	// no colormap (grayscale?)
			for (i = 0; i < 256; i++)
			{
				pPalette->rgbRed = (BYTE)i;
				pPalette->rgbGreen = (BYTE)i;
				pPalette->rgbBlue = (BYTE)i;
				pPalette->rgbReserved = 0;
				pPalette++;
			}
			nColors = 256;
		}
		memset(pPalette, 0, sizeof(RGBQUAD) * (cPalette - nColors));
	}

	std::auto_ptr<CNxDIBImage> pDIBImage(new CNxDIBImage);
	if (!pDIBImage->Create(lpbmi))
	{
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}
	
	// ビットデータの取得開始
	jpeg_start_decompress(&cinfo);

	if (setjmp(jerr.env) != 0)
	{
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}

	int nHeight = lpbmi->bmiHeader.biHeight;
	for (int n = 0; n < nHeight; n++)
	{
		LPBYTE lpbBits = static_cast<LPBYTE>(pDIBImage->GetBits()) + pDIBImage->GetPitch() * n;
		jpeg_read_scanlines(&cinfo, &lpbBits, 1);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return pDIBImage.release();
}
