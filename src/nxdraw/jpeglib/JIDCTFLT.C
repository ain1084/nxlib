/*
 * jidctflt.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a floating-point implementation of the
 * inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
 * must also perform dequantization of the input coefficients.
 *
 * This implementation should be more accurate than either of the integer
 * IDCT implementations.  However, it may not give the same results on all
 * machines because of differences in roundoff behavior.  Speed will depend
 * on the hardware's floating point capacity.
 *
 * A 2-D IDCT can be done by 1-D IDCT on each column followed by 1-D IDCT
 * on each row (or vice versa, but it's more convenient to emit a row at
 * a time).  Direct algorithms are also available, but they are much more
 * complex and seem not to be any faster when reduced to code.
 *
 * This implementation is based on Arai, Agui, and Nakajima's algorithm for
 * scaled DCT.  Their original paper (Trans. IEICE E-71(11):1095) is in
 * Japanese, but the algorithm is described in the Pennebaker & Mitchell
 * JPEG textbook (see REFERENCES section in file README).  The following code
 * is based directly on figure 4-8 in P&M.
 * While an 8-point DCT cannot be done in less than 11 multiplies, it is
 * possible to arrange the computation so that many of the multiplies are
 * simple scalings of the final outputs.  These multiplies can then be
 * folded into the multiplications or divisions by the JPEG quantization
 * table entries.  The AA&N method leaves only 5 multiplies and 29 adds
 * to be done in the DCT itself.
 * The primary disadvantage of this method is that with a fixed-point
 * implementation, accuracy is lost due to imprecise representation of the
 * scaled quantization values.  However, that problem does not arise if
 * we use floating point arithmetic.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		/* Private declarations for DCT subsystem */

#ifdef DCT_FLOAT_SUPPORTED

/*
 * This module is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif


/* Dequantize a coefficient by multiplying it by the multiplier-table
 * entry; produce a float result.
 */

#define DEQUANTIZE(coef,quantval)  (((FAST_FLOAT) (coef)) * (quantval))


/*
 * Perform dequantization and inverse DCT on one block of coefficients.
 */

 /* This assembly routine written by S.Ainoguchi */
static void jpeg_idct_float_asm (j_decompress_ptr cinfo, jpeg_component_info * compptr,
					 JCOEFPTR coef_block,
					 JSAMPARRAY output_buf,
					 JDIMENSION output_col)
{
	FAST_FLOAT workspace[DCTSIZE2];
	FAST_FLOAT tmp1;
	int tmp;
	int tmp2;

	/* c2 = cos(PI / 8) * 2 = 1.847759065023 */
	/* c4 = sqrt(2)         = 1.414213562373 */
	/* c6 = sin(PI / 8) * 2 = 0.7653668647302 */

	static const FAST_FLOAT FL1_414213562 = (FAST_FLOAT)1.414213562;		/* c4 */
	static const FAST_FLOAT FL1_847759065 = (FAST_FLOAT)1.847759065;		/* c2 */
	static const FAST_FLOAT FL1_082392200 = (FAST_FLOAT)1.082392200;		/* c2 - c6 */
	static const FAST_FLOAT FL2_613125930 = (FAST_FLOAT)-2.613125930;		/* -(c2 + c6) */
	
	UINT16 wFpuControl, wFpuControlSave;
	
	_asm
	{
		push	ecx
		mov		edi, coef_block						; inptr
		mov		esi, compptr
		mov		ecx, DCTSIZE
		lea		ebx, workspace
		mov		esi, [esi]compptr.dct_table

		; edi = inptr
		; esi = quant_ptr
		; ebx = workspace

pass1_loop:
		mov		eax, [edi + DCTSIZE * 1 * 4]
		mov		edx, [edi + DCTSIZE * 2 * 4]
		or		eax, [edi + DCTSIZE * 3 * 4]
		or		edx, [edi + DCTSIZE * 4 * 4]
		or		eax, [edi + DCTSIZE * 5 * 4]
		or		edx, [edi + DCTSIZE * 6 * 4]
		or		eax, [edi + DCTSIZE * 7 * 4]
		or		eax, edx
		jnz		short pass1_non_zero

		fild	dword ptr [edi + DCTSIZE * 0 * 4]		;inptr[DCTSIZE*0]
		fmul	dword ptr [esi + DCTSIZE * 0 * 4]		;quantptr[DCTSIZE*0]
		add		ebx, 4
		add		edi, 4
		add		esi, 4
		dec		ecx
		fstp	dword ptr [ebx + DCTSIZE * 0 * 4 - 4]
		mov		eax, [ebx + DCTSIZE * 0 * 4 - 4]
		mov		[ebx + DCTSIZE * 1 * 4 - 4], eax
		mov		[ebx + DCTSIZE * 2 * 4 - 4], eax
		mov		[ebx + DCTSIZE * 3 * 4 - 4], eax
		mov		[ebx + DCTSIZE * 4 * 4 - 4], eax
		mov		[ebx + DCTSIZE * 5 * 4 - 4], eax
		mov		[ebx + DCTSIZE * 6 * 4 - 4], eax
		mov		[ebx + DCTSIZE * 7 * 4 - 4], eax
		jmp		short pass1_next

pass1_non_zero:
		/* Event part */
		fild	dword ptr [edi + DCTSIZE * 2 * 4]
		fmul	dword ptr [esi + DCTSIZE * 2 * 4]
		fld		ST(0)
		fild	dword ptr [edi + DCTSIZE * 6 * 4]
		fmul	dword ptr [esi + DCTSIZE * 6 * 4]
		/* phase 5-3 */
		/* ST(0) = tmp3, ST(1) = tmp1, ST(2) = tmp1 */
		fsub	ST(1), ST	; ST(0) = tmp3,   ST(1) = tmp1-3, ST(2) = tmp1
		faddp	ST(2), ST	; ST(0) = tmp1-3, ST(1) = tmp13
		fmul	dword ptr [FL1_414213562]
		fsub	ST, ST(1)	; ST(0) = tmp12,  ST(1) = tmp13
		/* phase 3*/
		/* ST(0) = tmp12, ST(1) = tmp13 */
		fild	dword ptr [edi + DCTSIZE * 0 * 4]
		fmul	dword ptr [esi + DCTSIZE * 0 * 4]
		fld		ST(0)
		fild	dword ptr [edi + DCTSIZE * 4 * 4]
		fmul	dword ptr [esi + DCTSIZE * 4 * 4]		; ST(0) = tmp2,  ST(1) = tmp0,  ST(2) = tmp0
		; tmp0 - tmp2 (tmp11)
		fsub	ST(1), ST								; ST(0) = tmp2,  ST(1) = tmp11, ST(2) = tmp0
		; tmp0 + tmp2 (tmp10)
		faddp	ST(2), ST								; ST(0) = tmp11, ST(1) = tmp10
		/* pahse2 */
		/* ST(0) = tmp11, ST(1) = tmp10, ST(2) = tmp12, ST(3) = tmp13 */
		fld		ST(2)				; ST(0) = tmp12, ST(1) = tmp11, ST(2) = tmp10, ST(3) = tmp12, ST(4) = tmp13
		fadd	ST, ST(1)			; ST(0) = tmp1,  ST(1) = tmp11, ST(2) = tmp10, ST(3) = tmp12, ST(4) = tmp13
		fstp	dword ptr [tmp1]	; ST(0) = tmp11  ST(1) = tmp10, ST(2) = tmp12, ST(3) = tmp13
		fsubrp	ST(2), ST			; ST(0) = tmp10, ST(1) = tmp2,  ST(2) = tmp13
		fld		ST(2)				; ST(0) = tmp13, ST(1) = tmp10, ST(2) = tmp2,  ST(3) = tmp13
		fadd	ST, ST(1)			; ST(0) = tmp0,  ST(1) = tmp10, ST(2) = tmp2,  ST(3) = tmp13
		fxch	ST(1)				; ST(0) = tmp10, ST(1) = tmp0,  ST(2) = tmp2,  ST(3) = tmp13
		fsubrp	ST(3), ST			; ST(0) = tmp0,  ST(1) = tmp2,  ST(2) = tmp3
		/* Odd */
		fild	dword ptr [edi + DCTSIZE * 3 * 4]
		fmul	dword ptr [esi + DCTSIZE * 3 * 4]		; ST(0) = tmp5
		fld		ST(0)
		fild	dword ptr [edi + DCTSIZE * 5 * 4]
		fmul	dword ptr [esi + DCTSIZE * 5 * 4]		; ST(0) = tmp6, ST(1) = tmp5, ST(2) = tmp5
		fadd	ST(2), ST								; ST(0) = tmp6, ST(1) = tmp5, ST(2) = z13
		fsubr											; ST(0) = z10,  ST(1) = z13
		fild	dword ptr [edi + DCTSIZE * 7 * 4]
		fmul	dword ptr [esi + DCTSIZE * 7 * 4]		; ST(0) = tmp7, ST(1) = z10,  ST(2) = z13
		fld		ST(0)									; ST(0) = tmp7, ST(1) = tmp7, ST(2) = z10, ST(3) = z13
		fild	dword ptr [edi + DCTSIZE * 1 * 4]
		fmul	dword ptr [esi + DCTSIZE * 1 * 4]		; ST(0) = tmp4, ST(1) = tmp7, ST(2) = tmp7, ST(3) = z10, ST(4) = z13
		fadd	ST(2), ST								; ST(0) = tmp4, ST(1) = tmp7, ST(2) = z11,  ST(3) = z10, ST(4) = z13
		fsubr											; ST(0) = z12,  ST(1) = z11,  ST(2) = z10,  ST(3) = z13

		fld		ST(2)									; ST0:z10  ST1:z12  ST2:z11  ST3:z10  ST4:z13
		fadd	ST, ST(1)
		fmul	dword ptr [FL1_847759065]				; ST0:z5   ST1:z12  ST2:z11  ST3:z10  ST4:z13
		fxch	ST(1)									; ST0:z12  ST1:z5
		fmul	dword ptr [FL1_082392200]				; ST0:t10* ST1:z5
		fsub	ST, ST(1)								; ST0:t10  ST1:z5
		fxch	ST(3)									; ST0:z10  ST1:z5   ST2:z11  ST3:t10  ST4:z13
		fmul	dword ptr [FL2_613125930]				; ST0:t12* ST1:z5
		fadd											; ST0:t12  ST1:z11  ST2:t10  ST3:z13
		fld		ST(1)									; ST0:z11  ST1:t12  ST2:z11  ST3:t10  ST4:z13
		fsub	ST, ST(4)								; ST0:z1-3
		fmul	dword ptr [FL1_414213562]				; ST0:t11  ST1:t12  ST2:z11  ST3:t10  ST4:z13
		fxch	ST(2)									; ST0:z11  ST1:t12  ST2:t11  ST3:t10  ST4:z13
		faddp	ST(4), ST								; ST0:t12  ST1:t11  ST2:t10  ST3:t7
		fsub	ST, ST(3)								; ST0:t6   ST1:t11  ST2:t10  ST3:t7
		fsub	ST(1), ST								; ST0:t6   ST1:t5   ST2:t10  ST3:t7
		fxch	ST(2)									; ST0:t10  ST1:t5   ST2:t6   ST3:t7
		fadd	ST, ST(1)								; ST0:t4   ST1:t5   ST2:t6   ST3:t7
		/*****************/
		; ST(0) = tmp4, ST(1) = tmp5, ST(2) = tmp6, ST(3) = tmp7
		; ST(4) = tmp0, ST(5) = tmp2, ST(6) = tmp3
		fld		ST(0)
		fadd	ST, ST(7)
		fstp	dword ptr [ebx + DCTSIZE * 4 * 4]
		fsubp	ST(6), ST							; ST0:tmp5 ST1:tmp6 ST2:tmp7 ST3:tmp0 ST4:tmp2 ST5:tmp3-4
		fld		ST(0)								; ST0:tmp5 ST1:tmp5 ST2:tmp6 ST3:tmp7 ST4:tmp0 ST5:tmp2 ST6:tmp3-4
		fadd	ST, ST(5)
		fstp	dword ptr [ebx + DCTSIZE * 2 * 4]	; ST0:tmp5 ST1:tmp6 ST2:tmp7 ST3:tmp0   ST4:tmp2 ST5:tmp3-4
		fsubp	ST(4), ST							; ST0:tmp6 ST1:tmp7 ST2:tmp0 ST3:tmp2-5 ST4:tmp3-4
		fld		dword ptr [tmp1]
		fld		ST(0)								; ST0:tmp1 ST1:tmp1 ST2:tmp6 ST3:tmp7   ST4:tmp0 ST5:tmp2-5 ST6:tmp3-4
		fadd	ST, ST(2)
		fstp	dword ptr [ebx + DCTSIZE * 1 * 4]	; ST0:tmp1 ST1:tmp6 ST2:tmp7 ST3:tmp0   ST4:tmp2-5 ST5:tmp3-4
		fsubr
		fstp	dword ptr [ebx + DCTSIZE * 6 * 4]
		fld		ST(0)								; ST0:tmp7 ST1:tmp7 ST2:tmp0 ST3:tmp2-5 ST4:tmp3-4
		fadd	ST, ST(2)
		add		esi, 4
		fstp	dword ptr [ebx + DCTSIZE * 0 * 4]	; ST0:tmp7 ST1:tmp0 ST2:tmp2-5 ST3:tmp3-4
		fsub
		add		ebx, 4
		add		edi, 4
		fstp	dword ptr [ebx + DCTSIZE * 7 * 4 - 4]
		fstp	dword ptr [ebx + DCTSIZE * 5 * 4 - 4]
		dec		ecx
		fstp	dword ptr [ebx + DCTSIZE * 3 * 4 - 4]
pass1_next:
		jnz		pass1_loop

		; pass2

		fnstcw	word ptr [wFpuControlSave]		; Store FPU control word
		mov		ecx, cinfo
		mov		ax, [wFpuControlSave]
		lea		esi, workspace					; wsptr
		or		ah, 0ch							; Modify rounding control
		mov		ebx, [ecx]cinfo.sample_range_limit
		mov		[wFpuControl], ax
		mov		ecx, DCTSIZE
		add		ebx, CENTERJSAMPLE				; range_limit
		fldcw	word ptr [wFpuControl]			; load FPU control word

pass2_loop:
		; esi = workspace
		; ecx = counter
		; ebx = range_mask

		mov		edx, DCTSIZE
		mov		edi, output_buf
		sub		edx, ecx
		mov		edi, [edi + edx * 4]
		mov		eax, [esi + 4 * 1]
		add		edi, [output_col]		; output_buf
		mov		edx, [esi + 4 * 2]
		or		eax, [esi + 4 * 3]
		or		edx, [esi + 4 * 4]
		or		eax, [esi + 4 * 5]
		or		edx, [esi + 4 * 6]
		or		eax, [esi + 4 * 7]
		or		eax, edx
		add		eax, eax
		jnz		short pass2_non_zero

;pass2_all_zero:

		fld		dword ptr [esi + 4 * 0]
		fistp	dword ptr [tmp]
		mov		eax, [tmp]
		add		eax, 4
		sar		eax, 3
		and		eax, RANGE_MASK
		mov		al, [ebx + eax]
		mov		[edi + 0], al
		mov		[edi + 1], al
		mov		[edi + 2], al
		mov		[edi + 3], al
		mov		[edi + 4], al
		mov		[edi + 5], al
		mov		[edi + 6], al
		mov		[edi + 7], al
		jmp		short pass2_next

pass2_non_zero:
		; edi = output_buf
		; ebx = range_mask
		; esi = workspace
		; edx = temp

		fld		dword ptr [esi + 4 * 4]					; ST0:wsptr[4]
		fld		ST(0)									; ST0:wsptr[4] ST1:wsptr[4]

		/* Even */
		fld		dword ptr [esi + 0 * 4]					; ST0:wsptr[0] ST1:wsptr[4] ST2:wsptr[4]
		fadd	ST(2), ST								; ST0:wsptr[0] ST1:wsptr[4] ST2:tmp10
		fsubr											; ST0:tmp11, ST1:tmp10

		fld		dword ptr [esi + 6 * 4]					; ST0:w[6]  ST1:tmp11 ST2:tmp10
		fld		ST(0)									; ST0:w[6]  ST1:w[6]  ST2:tmp11 ST3:tmp10
		fld		dword ptr [esi + 2 * 4]					; ST0:w[2]  ST1:w[6]  ST2:w[6]  ST3:tmp11 ST4:tmp10
		fadd	ST(2), ST								; ST0:w[2]  ST1:w[6]  ST2:tmp13 ST3:tmp11 ST4:tmp10
		fsubr											; ST0:2-6   ST1:tmp13 ST2:tmp11 ST3:tmp10
		fmul	dword ptr [FL1_414213562]
		fsub	ST, ST(1)								; ST0:tmp12 ST1:tmp13 ST2:tmp11 ST3:tmp10

		fld		ST(2)									; ST0:tmp11 ST1:tmp12 ST2:tmp13 ST3:tmp11 ST4:tmp10
		fadd	ST, ST(1)								; ST0:tmp1  ST1:tmp12 ST2:tmp13 ST3:tmp11 ST4:tmp10
		fstp	dword ptr [tmp1]						; ST0:tmp12 ST1:tmp13 ST2:tmp11 ST3:tmp10
		fsubp	ST(2), ST								; ST0:tmp13 ST1:tmp2  ST2:tmp10
		fld		ST(2)									; ST0:tmp10 ST1:tmp13 ST2:tmp2  ST3:tmp10
		fadd	ST, ST(1)								; ST0:tmp0  ST1:tmp13 ST2:tmp2  ST3:tmp10
		fxch	ST(1)									; ST0:tmp13 ST0:tmp0  ST2:tmp2  ST3:tmp10
		fsubp	ST(3), ST								; ST0:tmp0  ST1:tmp2  ST3:tmp3
	
		/* Odd */
		fld		dword ptr [esi + 3 * 4]					; ST0:w[3]
		fld		ST(0)									; ST0:w[3] ST1:w[3]
		fld		dword ptr [esi + 5 * 4]					; ST0:w[5] ST1:w[3] ST2:w[3]
		fadd	ST(2), ST								; ST0:w[5] ST1:w[3] ST2:z13
		fsubr											; ST0:z10  ST1:z13
		fld		dword ptr [esi + 7 * 4]					; ST0:w[7] ST1:z10  ST2:z13
		fld		ST(0)									; ST0:w[7] ST1:w[7] ST2:z10  ST3:z13
		fld		dword ptr [esi + 1 * 4]					; ST0:w[1] ST1:w[7] ST2:w[7] ST3:z10  ST4:z13
		fadd	ST(2), ST								; ST0:w[1] ST1:w[7] ST2:z11  ST3:z10  ST4:z13
		fsubr											; ST0:z12  ST1:z11  ST2:z10  ST3:z13

		fld		ST(2)									; ST0:z10  ST1:z12  ST2:z11  ST3:z10  ST4:z13
		fadd	ST, ST(1)
		fmul	dword ptr [FL1_847759065]				; ST0:z5   ST1:z12  ST2:z11  ST3:z10  ST4:z13
		fxch	ST(1)									; ST0:z12  ST1:z5
		fmul	dword ptr [FL1_082392200]				; ST0:t10* ST1:z5
		fsub	ST, ST(1)								; ST0:t10  ST1:z5
		fxch	ST(3)									; ST0:z10  ST1:z5   ST2:z11  ST3:t10  ST4:z13
		fmul	dword ptr [FL2_613125930]				; ST0:t12* ST1:z5
		fadd											; ST0:t12  ST1:z11  ST2:t10  ST3:z13
		fld		ST(1)									; ST0:z11  ST1:t12  ST2:z11  ST3:t10  ST4:z13
		fsub	ST, ST(4)								; ST0:z1-3
		fmul	dword ptr [FL1_414213562]				; ST0:t11  ST1:t12  ST2:z11  ST3:t10  ST4:z13
		fxch	ST(2)									; ST0:z11  ST1:t12  ST2:t11  ST3:t10  ST4:z13
		faddp	ST(4), ST								; ST0:t12  ST1:t11  ST2:t10  ST3:t7
		fsub	ST, ST(3)								; ST0:t6   ST1:t11  ST2:t10  ST3:t7
		fsub	ST(1), ST								; ST0:t6   ST1:t5   ST2:t10  ST3:t7
		fxch	ST(2)									; ST0:t10  ST1:t5   ST2:t6   ST3:t7
		fadd	ST, ST(1)								; ST0:t4   ST1:t5   ST2:t6   ST3:t7
		; ST0:tmp4 ST1:tmp5 ST2:tmp6 ST3:tmp7
		; ST4:tmp0 ST5:tmp2 ST6:tmp3
		fld		ST(0)									; ST0:tmp4 ST1:tmp4 ST2:tmp5 ST3:tmp6 ST4:tmp7 ST5:tmp0 ST6:tmp2 ST7:tmp3
		fadd	ST, ST(7)

		; edx = bias(4)
		; ebx = workspace
		; eax = free
		; edi = outptr
		; esi = inptr

		fistp	dword ptr [tmp]							; 1-6  ST0:tmp4 ST1:tmp5 ST2:tmp6 ST3:tmp7 ST5:tmp0 ST6:tmp2 ST6:tmp3
		fsubp	ST(6), ST								; 1-3 ST0:tmp5 ST1:tmp6 ST2:tmp7 ST3:tmp0 ST4:tmp2 ST5:tmp3-4
		fld		ST(0)									; 1-1 ST0:tmp5 ST1:tmp5 ST2:tmp6 ST3:tmp7 ST4:tmp0 ST5:tmp2 ST6:tmp3-4
		fadd	ST, ST(5)								; 1-2
		fistp	dword ptr [tmp2]						; 1-6 ST0:tmp5 ST1:tmp6 ST2:tmp7 ST3:tmp0 ST4:tmp2 ST5:tmp3-4
		mov		eax, [tmp]								; 1
		add		eax, 4									; 1
		sar		eax, 3									; 1
		fsubp	ST(4), ST								; + 1-3 ST0:tmp6 ST1:tmp7 ST2:tmp0 ST3:tmp2-5 ST4:tmp3-4
		and		eax, RANGE_MASK							; 1
		fld		dword ptr [tmp1]
		mov		al, [ebx + eax]							; 1-2 outptr[4]
		fld		ST(0)									; ST0:tmp1 ST1:tmp1 ST2:tmp6 ST3:tmp7 ST4:tmp0 ST5:tmp2-5 ST6:tmp3-4
		fadd	ST, ST(2)								; +3
		mov		[edi + 4], al							; 1-1

		fistp	dword ptr [tmp]							; +6 ST0:tmp1 ST1:tmp6 ST2:tmp7 ST3:tmp0 ST4:tmp2-5 ST5:tmp3-4
		mov		eax, [tmp2]								; + 4-5
		mov		edx, [tmp]
		add		eax, 4
		add		edx, 4
		sar		eax, 3
		fsubr
		sar		edx, 3
		and		eax, RANGE_MASK
		and		edx, RANGE_MASK
		fistp	dword ptr [tmp2]						; ST0:tmp7 ST1:tmp0 ST2:tmp2-5 ST3:tmp3-4
		mov		al, [ebx + eax]							; outptr[2]
		mov		dl, [ebx + edx]
		fld		ST(0)
		fadd	ST, ST(2)
		fistp	dword ptr [tmp]
		mov		[edi + 2] ,al
		mov		[edi + 1], dl

		mov		eax, [tmp2]
		mov		edx, [tmp]
		add		eax, 4
		add		edx, 4
		sar		eax, 3
		fsub
		sar		edx, 3
		and		eax, RANGE_MASK
		and		edx, RANGE_MASK
		fistp	dword ptr [tmp2]
		mov		al, [ebx + eax]							; outptr[6]
		mov		dl, [ebx + edx]
		fistp	dword ptr [tmp]
		mov		[edi + 6], al
		mov		[edi + 0], dl

		mov		eax, [tmp2]
		mov		edx, [tmp]
		add		eax, 4
		add		edx, 4
		sar		eax, 3
		sar		edx, 3
		and		eax, RANGE_MASK
		and		edx, RANGE_MASK
		mov		al, [ebx + eax]
		mov		dl, [ebx + edx]
		fistp	dword ptr [tmp2]
		mov		[edi + 7], al			; outptr[7]
		mov		eax, [tmp2]
		add		eax, 4
		sar		eax, 3
		and		eax, RANGE_MASK
		mov		[edi + 5], dl
		mov		al, [ebx + eax]
		mov		[edi + 3], al

pass2_next:
		add		esi, DCTSIZE * 4
		dec		ecx
		jnz		pass2_loop
		
		fldcw	word ptr [wFpuControlSave]				; Restore FPU control word
		pop		ecx
	}
}

GLOBAL(void)
jpeg_idct_float (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col)
{
#if 1
	jpeg_idct_float_asm (cinfo, compptr, coef_block, output_buf, output_col);
#else
  FAST_FLOAT tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  FAST_FLOAT tmp10, tmp11, tmp12, tmp13;
  FAST_FLOAT z5, z10, z11, z12, z13;
  JCOEFPTR inptr;
  FLOAT_MULT_TYPE * quantptr;
  FAST_FLOAT * wsptr;
  JSAMPROW outptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  FAST_FLOAT workspace[DCTSIZE2]; /* buffers data between passes */
  SHIFT_TEMPS

  /* Pass 1: process columns from input, store into work array. */

  inptr = coef_block;
  quantptr = (FLOAT_MULT_TYPE *) compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any column in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * column DCT calculations can be simplified this way.
     */
    
    if ((inptr[DCTSIZE*1] | inptr[DCTSIZE*2] | inptr[DCTSIZE*3] |
	 inptr[DCTSIZE*4] | inptr[DCTSIZE*5] | inptr[DCTSIZE*6] |
	 inptr[DCTSIZE*7]) == 0) {
      /* AC terms all zero */
      FAST_FLOAT dcval = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
      
      wsptr[DCTSIZE*0] = dcval;
      wsptr[DCTSIZE*1] = dcval;
      wsptr[DCTSIZE*2] = dcval;
      wsptr[DCTSIZE*3] = dcval;
      wsptr[DCTSIZE*4] = dcval;
      wsptr[DCTSIZE*5] = dcval;
      wsptr[DCTSIZE*6] = dcval;
      wsptr[DCTSIZE*7] = dcval;
      
      inptr++;			/* advance pointers to next column */
      quantptr++;
      wsptr++;
      continue;
    }
    
    /* Even part */

    tmp0 = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
    tmp1 = DEQUANTIZE(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE*4], quantptr[DCTSIZE*4]);
    tmp3 = DEQUANTIZE(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

    tmp10 = tmp0 + tmp2;	/* phase 3 */
    tmp11 = tmp0 - tmp2;

    tmp13 = tmp1 + tmp3;	/* phases 5-3 */
    tmp12 = (tmp1 - tmp3) * ((FAST_FLOAT) 1.414213562) - tmp13; /* 2*c4 */

    tmp0 = tmp10 + tmp13;	/* phase 2 */
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;
    
    /* Odd part */

    tmp4 = DEQUANTIZE(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
    tmp5 = DEQUANTIZE(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
    tmp6 = DEQUANTIZE(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
    tmp7 = DEQUANTIZE(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);

    z13 = tmp6 + tmp5;		/* phase 6 */
    z10 = tmp6 - tmp5;
    z11 = tmp4 + tmp7;
    z12 = tmp4 - tmp7;

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = (z11 - z13) * ((FAST_FLOAT) 1.414213562); /* 2*c4 */

    z5 = (z10 + z12) * ((FAST_FLOAT) 1.847759065); /* 2*c2 */
    tmp10 = ((FAST_FLOAT) 1.082392200) * z12 - z5; /* 2*(c2-c6) */
    tmp12 = ((FAST_FLOAT) -2.613125930) * z10 + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    wsptr[DCTSIZE*0] = tmp0 + tmp7;
    wsptr[DCTSIZE*7] = tmp0 - tmp7;
    wsptr[DCTSIZE*1] = tmp1 + tmp6;
    wsptr[DCTSIZE*6] = tmp1 - tmp6;
    wsptr[DCTSIZE*2] = tmp2 + tmp5;
    wsptr[DCTSIZE*5] = tmp2 - tmp5;
    wsptr[DCTSIZE*4] = tmp3 + tmp4;
    wsptr[DCTSIZE*3] = tmp3 - tmp4;

    inptr++;			/* advance pointers to next column */
    quantptr++;
    wsptr++;
  }
  
  /* Pass 2: process rows from work array, store into output array. */
  /* Note that we must descale the results by a factor of 8 == 2**3. */

  wsptr = workspace;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    outptr = output_buf[ctr] + output_col;
    /* Rows of zeroes can be exploited in the same way as we did with columns.
     * However, the column calculation has created many nonzero AC terms, so
     * the simplification applies less often (typically 5% to 10% of the time).
     * And testing floats for zero is relatively expensive, so we don't bother.
     */

    /* Even part */

    tmp10 = wsptr[0] + wsptr[4];
    tmp11 = wsptr[0] - wsptr[4];

    tmp13 = wsptr[2] + wsptr[6];
    tmp12 = (wsptr[2] - wsptr[6]) * ((FAST_FLOAT) 1.414213562) - tmp13;

    tmp0 = tmp10 + tmp13;
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    z13 = wsptr[5] + wsptr[3];
    z10 = wsptr[5] - wsptr[3];
    z11 = wsptr[1] + wsptr[7];
    z12 = wsptr[1] - wsptr[7];

    tmp7 = z11 + z13;
    tmp11 = (z11 - z13) * ((FAST_FLOAT) 1.414213562);

    z5 = (z10 + z12) * ((FAST_FLOAT) 1.847759065); /* 2*c2 */
    tmp10 = ((FAST_FLOAT) 1.082392200) * z12 - z5; /* 2*(c2-c6) */
    tmp12 = ((FAST_FLOAT) -2.613125930) * z10 + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    /* Final output stage: scale down by a factor of 8 and range-limit */

    outptr[0] = range_limit[(int) DESCALE((INT32) (tmp0 + tmp7), 3)
			    & RANGE_MASK];
    outptr[7] = range_limit[(int) DESCALE((INT32) (tmp0 - tmp7), 3)
			    & RANGE_MASK];
    outptr[1] = range_limit[(int) DESCALE((INT32) (tmp1 + tmp6), 3)
			    & RANGE_MASK];
    outptr[6] = range_limit[(int) DESCALE((INT32) (tmp1 - tmp6), 3)
			    & RANGE_MASK];
    outptr[2] = range_limit[(int) DESCALE((INT32) (tmp2 + tmp5), 3)
			    & RANGE_MASK];
    outptr[5] = range_limit[(int) DESCALE((INT32) (tmp2 - tmp5), 3)
			    & RANGE_MASK];
    outptr[4] = range_limit[(int) DESCALE((INT32) (tmp3 + tmp4), 3)
			    & RANGE_MASK];
    outptr[3] = range_limit[(int) DESCALE((INT32) (tmp3 - tmp4), 3)
			    & RANGE_MASK];
    
    wsptr += DCTSIZE;		/* advance pointer to next row */
  }
#endif
}
#endif /* DCT_FLOAT_SUPPORTED */
