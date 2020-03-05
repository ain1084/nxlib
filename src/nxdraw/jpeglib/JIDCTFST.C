/*
 * jidctfst.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a fast, not so accurate integer implementation of the
 * inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
 * must also perform dequantization of the input coefficients.
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
 * The primary disadvantage of this method is that with fixed-point math,
 * accuracy is lost due to imprecise representation of the scaled
 * quantization values.  The smaller the quantization table entry, the less
 * precise the scaled value, so this implementation does worse with high-
 * quality-setting files than with low-quality ones.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		/* Private declarations for DCT subsystem */

#ifdef DCT_IFAST_SUPPORTED


/*
 * This module is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif


/* Scaling decisions are generally the same as in the LL&M algorithm;
 * see jidctint.c for more details.  However, we choose to descale
 * (right shift) multiplication products as soon as they are formed,
 * rather than carrying additional fractional bits into subsequent additions.
 * This compromises accuracy slightly, but it lets us save a few shifts.
 * More importantly, 16-bit arithmetic is then adequate (for 8-bit samples)
 * everywhere except in the multiplications proper; this saves a good deal
 * of work on 16-bit-int machines.
 *
 * The dequantized coefficients are not integers because the AA&N scaling
 * factors have been incorporated.  We represent them scaled up by PASS1_BITS,
 * so that the first and second IDCT rounds have the same input scaling.
 * For 8-bit JSAMPLEs, we choose IFAST_SCALE_BITS = PASS1_BITS so as to
 * avoid a descaling shift; this compromises accuracy rather drastically
 * for small quantization table entries, but it saves a lot of shifts.
 * For 12-bit JSAMPLEs, there's no hope of using 16x16 multiplies anyway,
 * so we use a much larger scaling factor to preserve accuracy.
 *
 * A final compromise is to represent the multiplicative constants to only
 * 8 fractional bits, rather than 13.  This saves some shifting work on some
 * machines, and may also reduce the cost of multiplication (since there
 * are fewer one-bits in the constants).
 */

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  8
#define PASS1_BITS  2
#else
#define CONST_BITS  8
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

/* Some C compilers fail to reduce "FIX(constant)" at compile time, thus
 * causing a lot of useless floating-point operations at run time.
 * To get around this we use the following pre-calculated constants.
 * If you change CONST_BITS you may want to add appropriate values.
 * (With a reasonable C compiler, you can just rely on the FIX() macro...)
 */

#if CONST_BITS == 8
#define FIX_1_082392200  ((INT32)  277)		/* FIX(1.082392200) */
#define FIX_1_414213562  ((INT32)  362)		/* FIX(1.414213562) */
#define FIX_1_847759065  ((INT32)  473)		/* FIX(1.847759065) */
#define FIX_2_613125930  ((INT32)  669)		/* FIX(2.613125930) */
#else
#define FIX_1_082392200  FIX(1.082392200)
#define FIX_1_414213562  FIX(1.414213562)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_2_613125930  FIX(2.613125930)
#endif


/* We can gain a little more speed, with a further compromise in accuracy,
 * by omitting the addition in a descaling shift.  This yields an incorrectly
 * rounded result half the time...
 */

#ifndef USE_ACCURATE_ROUNDING
#undef DESCALE
#define DESCALE(x,n)  RIGHT_SHIFT(x, n)
#endif


/* Multiply a DCTELEM variable by an INT32 constant, and immediately
 * descale to yield a DCTELEM result.
 */

#define MULTIPLY(var,const)  ((DCTELEM) DESCALE((var) * (const), CONST_BITS))


/* Dequantize a coefficient by multiplying it by the multiplier-table
 * entry; produce a DCTELEM result.  For 8-bit data a 16x16->16
 * multiplication will do.  For 12-bit data, the multiplier table is
 * declared INT32, so a 32-bit multiply will be used.
 */

#if BITS_IN_JSAMPLE == 8
#define DEQUANTIZE(coef,quantval)  (((IFAST_MULT_TYPE) (coef)) * (quantval))
#else
#define DEQUANTIZE(coef,quantval)  \
	DESCALE((coef)*(quantval), IFAST_SCALE_BITS-PASS1_BITS)
#endif


/* Like DESCALE, but applies to a DCTELEM and produces an int.
 * We assume that int right shift is unsigned if INT32 right shift is.
 */

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define ISHIFT_TEMPS	DCTELEM ishift_temp;
#if BITS_IN_JSAMPLE == 8
#define DCTELEMBITS  16		/* DCTELEM may be 16 or 32 bits */
#else
#define DCTELEMBITS  32		/* DCTELEM must be 32 bits */
#endif
#define IRIGHT_SHIFT(x,shft)  \
    ((ishift_temp = (x)) < 0 ? \
     (ishift_temp >> (shft)) | ((~((DCTELEM) 0)) << (DCTELEMBITS-(shft))) : \
     (ishift_temp >> (shft)))
#else
#define ISHIFT_TEMPS
#define IRIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif

#ifdef USE_ACCURATE_ROUNDING
#define IDESCALE(x,n)  ((int) IRIGHT_SHIFT((x) + (1 << ((n)-1)), n))
#else
#define IDESCALE(x,n)  ((int) IRIGHT_SHIFT(x, n))
#endif


/*
 * Perform dequantization and inverse DCT on one block of coefficients.
 */

/* This assembly routine written by S.Ainoguchi */
static void jpeg_idct_ifast_asm (j_decompress_ptr cinfo, jpeg_component_info * compptr,
								   JCOEFPTR coef_block,
								   JSAMPARRAY output_buf,
								   JDIMENSION output_col)
{
	_asm
	{
#define SSTP_SIZE			16 + DCTSIZE2 * 4
#define SSTP_WORKSPACE		(esp + 16)
#define SSTP_CTR			(esp + 12)
#define SSTP_OUTPUT_COL		(esp + 8)
#define SSTP_OUTPUT_BUF		(esp + 4)
#define SSTP_RANGE_LIMIT	(esp + 0)

		push	ebp
		mov		ebx, cinfo
		push	ecx
		sub		esp, SSTP_SIZE
		mov		eax, [ebx]cinfo.sample_range_limit
		mov		ecx, output_col
		mov		ebx, output_buf
		add		eax, CENTERJSAMPLE
		mov		[SSTP_OUTPUT_COL], ecx
		mov		esi, coef_block					; inptr
		mov		[SSTP_OUTPUT_BUF], ebx
		mov		ebx, compptr
		mov		[SSTP_RANGE_LIMIT], eax
		lea		edi, [SSTP_WORKSPACE]
		mov		eax, DCTSIZE
		mov		ebp, [ebx]compptr.dct_table


		;******************** pass1

pass1_loop:
		; all zero checking

		mov		ebx, [esi + DCTSIZE * 1 * 4]
		mov		[SSTP_CTR], eax
		mov		ecx, [esi + DCTSIZE * 3 * 4]
		mov		edx, [esi + DCTSIZE * 4 * 4]
		or		ebx, ecx						; ebx = 1|3
		mov		eax, [esi + DCTSIZE * 5 * 4]
		or		ebx, edx						; ebx = 1|3|4
		mov		ecx, [esi + DCTSIZE * 7 * 4]
		or		eax, ebx						; eax = 1|3|4|5
		mov		edx, [esi + DCTSIZE * 2 * 4]
		or		eax, ecx						; eax = 1|3|4|5|7
		mov		ebx, [esi + DCTSIZE * 6 * 4]
		or		eax, edx						; eax = 1|3|4|5|6|7
		or		eax, ebx						; eax = 1|2|3|4|5|6|7
		; EBX = [esi + DCTSIZE * 6 * 4]
		; EDX = [esi + DCTSIZE * 2 * 4]
		jnz		short pass1_non_zero

		mov		eax, [esi + DCTSIZE * 0 *4]
		imul	dword ptr [ebp + DCTSIZE * 0 * 4]
		mov		[edi                  ], eax
		mov		[edi + DCTSIZE * 4 * 4], eax
		mov		[edi + DCTSIZE * 1 * 4], eax
		mov		[edi + DCTSIZE * 5 * 4], eax
		mov		[edi + DCTSIZE * 2 * 4], eax
		mov		[edi + DCTSIZE * 6 * 4], eax
		mov		[edi + DCTSIZE * 3 * 4], eax
		mov		[edi + DCTSIZE * 7 * 4], eax
		jmp		pass1_next
		align	8
pass1_non_zero:

		; EBX = [esi + DCTSIZE * 6 * 4]
		; EDX = [esi + DCTSIZE * 2 * 4]
		imul	ebx, [ebp + DCTSIZE * 6 * 4]
		imul	edx, [ebp + DCTSIZE * 2 * 4]
		push	esi
		lea		ecx, [ebx + edx]
		sub		edx, ebx						; tmp1 - tmp3
		push	edi

		; ecx = tmp13
		; edx = tmp1 -tmp3
		
	; FIX_1_414213562(362) = *2 + *8 + *32 + *64 + *256


		lea		edi, [edx + edx * 4]			; eax*5
		lea		edi, [edi + edi * 8]			; eax*5 + eax*40
		lea		eax, [edx + edi * 4]			; eax*1 + (eax*180)
		mov		ebx, [esi + DCTSIZE * 0 * 4]
		mov		edx, [esi + DCTSIZE * 4 * 4]
		sar		eax, 7

		; eax = tmp12
		; ecx = tmp13

		imul	ebx, [ebp + DCTSIZE * 0 * 4]		; ebx = tmp0
		imul	edx, [ebp + DCTSIZE * 4 * 4]		; edx = tmp2
		sub		eax, ecx
		lea		edi, [ebx + edx]					; tmp10
		add		edi, ecx							; tmp0 = tmp10 + tmp13
		push	edi									; esp + 12 save tmp0 =======================
		lea		edi, [ebx + edx]					; tmp10
		sub		edi, ecx
		sub		ebx, edx							; tmp11
		push	edi									; esp +  8 save tmp3 =======================
		lea		edx, [ebx + eax]					; tmp11 + tmp12
		mov		ecx, [esi + DCTSIZE * 7 * 4]
		sub		ebx, eax
		push	edx									; esp +  4 save tmp1 =======================
		push	ebx									; esp +  0 save tmp2 =======================
		
	; Odd part

		mov		ebx, [esi + DCTSIZE * 1 * 4]
		imul	ebx, [ebp + DCTSIZE * 1 * 4]	; tmp4
		imul	ecx, [ebp + DCTSIZE * 7 * 4]	; tmp7
		lea		eax, [ebx + ecx]				; z11
		sub		ebx, ecx						; z12

		; eax = z11
		; ebx = z12

		mov		edx, [esi + DCTSIZE * 5 * 4]
		mov		ecx, [esi + DCTSIZE * 3 * 4]
		imul	edx, [ebp + DCTSIZE * 5 * 4]	; tmp6
		imul	ecx, [ebp + DCTSIZE * 3 * 4]	; tmp5
		lea		edi, [edx + ecx]				; z13 = tmp6 + tmp5
		lea		esi, [edx + ecx]				; z13
		add		edi, eax						; tmp7 = z13 + z11
		sub		eax, esi						; z11 - z13
		sub		edx, ecx						; z10

		; eax = z11 - z13
		; edx = z10
		; edi = tmp7
		; ebx = z12

	; calculation tmp11
	; FIX_1_414213562 = *362 = *2 + *8 + *32 + *64 + *256
		
		lea		ecx, [eax + eax * 4]			; ecx = eax*5
		lea		ecx, [ecx + ecx * 8]			; ecx = eax*5 + (eax*40)
		lea		ecx, [eax + ecx * 4]			; ecx = eax*1 + (eax*180)
		lea		eax, [edx + ebx]
		sar		ecx, 7

		; ecx = tmp11
		; edx = z10
		; edi = tmp7
		; ebx = z12

	; calculation z5
	; FIX_1_847759065 = *1 + *8 + *16 + *64 + *128 *256
				
		mov		esi, eax
		shl		esi, 4
		sub		esi, eax
		shl		esi, 2
		sub		esi, eax
		lea		eax, [eax + esi * 8]
		sar		eax, 8

		; ecx = tmp11
		; edx = z10
		; edi = tmp7
		; ebx = z12
		; eax = z5

	; calculation tmp10
	; FIX_1_082392200(277) = *256 + *16 + *4 + *1

		; FIX_1_082392200(277) = *256 + *16 + *4 + *1		ecx = z12

		lea		esi, [ebx + ebx * 2]	; *3
		shl		esi, 3					; *24
		sub		esi, ebx				; *23
		lea		esi, [esi + esi * 2]	; *69 (*23 + *46)
		lea		esi, [ebx + esi * 4]	; *1 + *276 (*69 * 4)
		sar		esi, 8
		mov		ebx, edx
		sub		esi, eax

		; ecx = tmp11
		; edx = z10
		; edi = tmp7
		; eax = z5
		; esi = tmp10

	; calculation tmp12
	; FIX_2_613125930(669) = *512 + *128 + *16 + *8 + *4 + *1

		shl		ebx, 3
		sub		ebx, edx
		shl		ebx, 5
		sub		ebx, edx
		lea		ebx, [ebx + ebx * 2]
		mov		edx, [esp + 16]
		neg		ebx
		sar		ebx, 8
		add		eax, ebx

		; eax = tmp12
		; ecx = tmp11
		; edi = tmp7
		; esi = tmp10
		
		pop		ebx					; ebx = tmp2
		sub		eax, edi			; tmp6
		sub		ecx, eax			; tmp5
		add		esi, ecx			; tmp4

		; eax = tmp6
		; ecx = tmp5
		; esi = tmp4
		; edi = tmp7
		; ebx = free
		; edx = free
		; esp + 20 = ESI save
		; esp + 16 = EDI save
		; esp + 12 = tmp0
		; esp +  8 = tmp3
		; esp +  4 = tmp1
		; esp +  0 = tmp2


		; edx = wsptr
		; ebx = free
		; ecx = tmp5
		; eax = tmp6
		; esi = tmp4
		; edi = tmp7

		add		ebx, ecx
		shl		ecx, 1
		mov		[edx + DCTSIZE * 2 * 4], ebx
		sub		ebx, ecx
		mov		[edx + DCTSIZE * 5 * 4], ebx

		; edx = wsptr
		; ebx = free
		; ecx = free
		; eax = tmp6
		; esi = tmp4
		; edi = tmp7

		pop		ecx								; tmp1
		lea		ebx, [ecx + eax]
		sub		ecx, eax
		mov		[edx + DCTSIZE * 1 * 4], ebx	; tmp1 + tmp6
		pop		eax								; tmp3
		mov		[edx + DCTSIZE * 6 * 4], ecx	; tmp1 - tmp6

		; edx = wsptr
		; ebx = free
		; ecx = free
		; eax = tmp3
		; esi = tmp4
		; edi = tmp7

		lea		ebx, [eax + esi]
		pop		ecx								; tmp0
		sub		eax, esi
		mov		[edx + DCTSIZE * 4 * 4], ebx	; tmp3 + tmp4
		add		esp, 4
		mov		[edx + DCTSIZE * 3 * 4], eax	; tmp3 - tmp4

		; edx = wsptr
		; ebx = free
		; ecx = free
		; eax = free
		; esi = free
		; edi = tmp7

		lea		eax, [ecx + edi]
		pop		esi
		sub		ecx, edi
		mov		[edx + DCTSIZE * 7 * 4], ecx	; tmp0 - tmp7
		mov		edi, edx
		mov		[edx + DCTSIZE * 0 * 4], eax	; tmp0 + tmp7

pass1_next:
		mov		ebx, 4
		mov		eax, [SSTP_CTR]
		add		edi, ebx
		add		esi, ebx
		add		ebp, ebx
		dec		eax
		jnz		pass1_loop

	;**************** pass2

		mov		ecx, DCTSIZE
		lea		esi, [SSTP_WORKSPACE]

pass2_loop:
		mov		edx, DCTSIZE
		mov		eax, [SSTP_OUTPUT_BUF]
		sub		edx, ecx
		mov		ebx, [SSTP_OUTPUT_COL]
		mov		eax, [eax + edx * 4]
		mov		[SSTP_CTR], ecx
		lea		edi, [eax + ebx]			; output_ptr

		mov		ecx, [esi + 3 * 4]
		mov		eax, [esi + 1 * 4]
		mov		ebx, [esi + 7 * 4]
		or		eax, ecx				; 3*4
		mov		edx, [esi + 4 * 4]
		or		eax, ebx				; 2*4
		mov		ecx, [esi + 5 * 4]
		or		eax, edx
		mov		ebx, [esi + 6 * 4]
		or		eax, ecx
		mov		edx, [esi + 2 * 4]
		or		eax, ebx
		or		eax, edx
		jnz		short pass2_non_zero

		mov		eax, [esi]
		mov		ebp, [SSTP_RANGE_LIMIT]
		sar		eax, PASS1_BITS + 3
		and		eax, RANGE_MASK				;1023
		mov		al, [ebp + eax]
		mov		ah, al
		mov		edx, eax
		shl		edx, 16
		or		eax, edx
		mov		[edi    ], eax
		mov		[edi + 4], eax
		jmp		pass2_next

		align	16
pass2_non_zero:
		; ebx = [esp + 6 * 4]  (wsptr[2])
		; edx = [esp + 2 * 4]  (wsptr[6])
		lea		ecx, [ebx + edx]
		push	esi						; ESP + 20
		push	edi						; ESP + 16 ===================
		sub		edx, ebx
		
		; ecx = tmp13
		; ebx = wsptr[2] - wsptr[6]

	; imul FIX_1_414213562(362) = *2 + *8 + *32 + *64 + *256

	
		mov		ebp, [esi + 4 * 4]
		lea		edi, [edx + edx * 4]
		lea		edi, [edi + edi * 8]
		mov		ebx, [esi + 4 * 0]
		lea		eax, [edx + edi * 4]

		; eax = tmp12 (*7 ‚Ü‚¾ -tmp13 ‚µ‚Ä‚È‚¢)
		; ecx = tmp13

		sar		eax, 7					; tmp12 / 7
		lea		edx, [ebx + ebp]		; edx = tmp10
		sub		eax, ecx				; tmp12 ( - tmp13)
		add		edx, ecx
		lea		edi, [ebx + ebp]
		push	edx						; ESP + 12 ======= save tmp0
		sub		edi, ecx				; edi : tmp3 = (tmp10 - tmp13)
		sub		ebx, ebp				; ebx = tmp11 (wsptr[0] - wsptr[4])
		push	edi						; ESP + 8 ======== save tmp3
		lea		ecx, [ebx + eax]		; ecx = tmp1 (tmp11 + tmp12)
		sub		ebx, eax				; ebx = tmp2 (tmp11 - tmp12)
		push	ecx						; ESP + 4 ======== save tmp1
		push	ebx						; ESP + 0 ======== save tmp2

		; Odd part

		mov		ecx, [esi + 3 * 4]		; wsptr[3]
		mov		ebx, [esi + 5 * 4]		; wsptr[5]
		lea		edx, [ebx + ecx]		; z13
		sub		ebx, ecx				; z10

		mov		eax, [esi + 7 * 4]		; wsptr[7]
		mov		ecx, [esi + 1 * 4]		; wsptr[1]
		lea		ebp, [ecx + eax]		; z11
		sub		ecx, eax				; z12

		; ‚±‚±‚æ‚è esi Žg—p‰Â”\
		; edx = z13
		; ebx = z10
		; ebp = z11
		; ecx = z12

		lea		eax, [ebp + edx]		; tmp7
		sub		ebp, edx				; ebp = z11 - z13

		; eax = tmp7
		; ebp = z11 -z13
		

	; imul FIX_1_414213562(362) = *2 + *8 + *32 + *64 + *256

		lea		edi, [ebp + ebp * 4]	; edi = ebp*5
		lea		edi, [edi + edi * 8]	; edi = ebp*5 + ebp*40
		lea		ebp, [ebp + edi * 4]	; ebp = ebp*1 + ebp*180
		mov		edx, eax				; edx = tmp7
		sar		ebp, 7

		; ebp = tmp11
		; ebx = z10
		; ecx = z12
		; edx = eax = tmp7

		; calculation 'z5'
		; FIX_1_847759065(473) = *1 + *8 + *16 + *64 + *128 *256

		lea		eax, [ebx + ecx]		;eax = z10 + z12
		mov		edi, eax
		shl		edi, 4					;edi = *16
		sub		edi, eax				;edi = *15
		shl		edi, 2					;edi = *60
		sub		edi, eax				;edi = *59
		lea		eax, [eax + edi * 8]	;1 + *472
		shr		eax, 8


		; eax = z5
		; ebp = tmp11
		; ebx = z10
		; ecx = z12
		; edx = tmp7
		
		; calculation 'tmp10'
		; FIX_1_082392200(277) = *256 + *16 + *4 + *1		ecx = z12

		lea		edi, [ecx + ecx * 2]	; *3
		shl		edi, 3					; *24
		sub		edi, ecx				; *23
		lea		edi, [edi + edi * 2]	; *69 (*23 + *46)
		lea		ecx, [ecx + edi * 4]	; *1 + *276 (*69 * 4)

		; ecx = tmp10
		; edx = tmp7
		; eax = z5
		; ebx = z10
		; ebp = tmp11

		; -FIX_2_613125930(669) = *512 + *128 + *16 + *8 + *4 + *1

		sar		ecx, 8
		mov		edi, ebx
		shl		edi, 3
		sub		ecx, eax
		sub		edi, ebx				; *7
		shl		edi, 5
		sub		edi, ebx				; *223
		lea		ebx, [edi + edi * 2]
		neg		ebx
		sar		ebx, 8
		add		ebx, eax
		
		
		; ebx = tmp12
		; ebp = tmp11
		; ecx = tmp10
		; edx = tmp7

		sub		ebx, edx						; tmp12 - tmp7  (ebx = tmp6)
		mov		edi, [esp + 4 * 4]				; resotre EDI
		sub		ebp, ebx						; tmp11 - tmp6  (ebp = tmp5)
		mov		esi, [SSTP_RANGE_LIMIT + 24]	; range_limit table
		add		ecx, ebp						; tmp10 + tmp5  (ecx = tmp4)

		; eax = free
		; ecx = tmp4
		; ebx = tmp6
		; edx = tmp7
		; ebp = tmp5
		; [esp +  0] = tmp2
		; [esp +  4] = tmp1
		; [esp +  8] = tmp3
		; [esp + 12] = tmp0
		; [esp + 16] = edi
		; [esp + 20] = esi


	; outptr[2] & outptr[5]
		; eax = free
		; ebp = tmp5
		mov		eax, [esp +  0]			; eax = tmp2
		add		eax, ebp				; eax = tmp2 + tmp5
		sar		eax, PASS1_BITS + 3
		and		eax, RANGE_MASK
		mov		al, [esi + eax]
		mov		[edi + 2], al
		pop		eax
		sub		eax, ebp
		sar		eax, PASS1_BITS + 3
		and		eax, RANGE_MASK
		mov		al, [esi + eax]
		mov		[edi + 5], al
		
	; outptr[1]
		; eax = free
		; ebp = free
		; ebx = tmp6

		pop		ebp						; ebp = tmp1
		lea		eax, [ebp + ebx]		; tmp1 + tmp6
		sar		eax, PASS1_BITS + 3
		sub		ebp, ebx				; tmp1 - tmp6
		and		eax, RANGE_MASK
		sar		ebp, PASS1_BITS + 3
		mov		al, [esi + eax]
		and		ebp, RANGE_MASK
		mov		[edi + 1], al
		mov		al, [esi + ebp]
		pop		ebx						; tmp3
		mov		[edi + 6], al

	; outptr[4] & outptr[3]
		; eax = free
		; ebp = free
		; ebx = tmp3
		; ecx = tmp4

		lea		ebp, [ebx + ecx]
		sar		ebp, PASS1_BITS + 3
		sub		ebx, ecx
		and		ebp, RANGE_MASK
		sar		ebx, PASS1_BITS + 3
		mov		al, [esi + ebp]
		and		ebx, RANGE_MASK
		mov		[edi + 4], al
		mov		al, [esi + ebx]
		pop		ebx						; tmp0
		mov		[edi + 3], al
		
	; outptr[0]
		; eax = free
		; ebp = free
		; ebx = tmp0
		; ecx = free
		; edx = tmp7

		lea		ecx, [ebx + edx]
		sar		ecx, PASS1_BITS + 3
		add		esp, 4
		sub		ebx, edx
		and		ecx, RANGE_MASK
		sar		ebx, PASS1_BITS + 3
		mov		al, [esi + ecx]
		and		ebx, RANGE_MASK
		mov		[edi], al				; tmp0 + tmp7
		mov		dl, [esi + ebx]
		pop		esi						; restore ESI
		mov		[edi + 7], dl			; tmp0 - tmp7


pass2_next:
		mov		ecx, [SSTP_CTR]
		add		esi, DCTSIZE * 4
		dec		ecx
		jnz		pass2_loop

		add		esp, SSTP_SIZE
		pop		ecx
		pop		ebp
	}
}
	
	
GLOBAL(void)
jpeg_idct_ifast (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col)
{
#if 1
	jpeg_idct_ifast_asm (cinfo, compptr, coef_block, output_buf, output_col);
#else

  DCTELEM tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  DCTELEM tmp10, tmp11, tmp12, tmp13;
  DCTELEM z5, z10, z11, z12, z13;
  JCOEFPTR inptr;
  IFAST_MULT_TYPE * quantptr;
  int * wsptr;
  JSAMPROW outptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[DCTSIZE2];	/* buffers data between passes */
  SHIFT_TEMPS			/* for DESCALE */
  ISHIFT_TEMPS			/* for IDESCALE */

  /* Pass 1: process columns from input, store into work array. */

  inptr = coef_block;
  quantptr = (IFAST_MULT_TYPE *) compptr->dct_table;
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
      int dcval = (int) DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);

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
    tmp12 = MULTIPLY(tmp1 - tmp3, FIX_1_414213562) - tmp13; /* 2*c4 */

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
    tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); /* 2*c4 */

    z5 = MULTIPLY(z10 + z12, FIX_1_847759065); /* 2*c2 */
    tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
    tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    wsptr[DCTSIZE*0] = (int) (tmp0 + tmp7);
    wsptr[DCTSIZE*7] = (int) (tmp0 - tmp7);
    wsptr[DCTSIZE*1] = (int) (tmp1 + tmp6);
    wsptr[DCTSIZE*6] = (int) (tmp1 - tmp6);
    wsptr[DCTSIZE*2] = (int) (tmp2 + tmp5);
    wsptr[DCTSIZE*5] = (int) (tmp2 - tmp5);
    wsptr[DCTSIZE*4] = (int) (tmp3 + tmp4);
    wsptr[DCTSIZE*3] = (int) (tmp3 - tmp4);

    inptr++;			/* advance pointers to next column */
    quantptr++;
    wsptr++;
  }
  
  /* Pass 2: process rows from work array, store into output array. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  wsptr = workspace;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    outptr = output_buf[ctr] + output_col;
    /* Rows of zeroes can be exploited in the same way as we did with columns.
     * However, the column calculation has created many nonzero AC terms, so
     * the simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */
    
#ifndef NO_ZERO_ROW_TEST
    if ((wsptr[1] | wsptr[2] | wsptr[3] | wsptr[4] | wsptr[5] | wsptr[6] |
	 wsptr[7]) == 0) {
      /* AC terms all zero */
      JSAMPLE dcval = range_limit[IDESCALE(wsptr[0], PASS1_BITS+3)
				  & RANGE_MASK];
      
      outptr[0] = dcval;
      outptr[1] = dcval;
      outptr[2] = dcval;
      outptr[3] = dcval;
      outptr[4] = dcval;
      outptr[5] = dcval;
      outptr[6] = dcval;
      outptr[7] = dcval;

      wsptr += DCTSIZE;		/* advance pointer to next row */
      continue;
    }
#endif
    
    /* Even part */

    tmp10 = ((DCTELEM) wsptr[0] + (DCTELEM) wsptr[4]);
    tmp11 = ((DCTELEM) wsptr[0] - (DCTELEM) wsptr[4]);

    tmp13 = ((DCTELEM) wsptr[2] + (DCTELEM) wsptr[6]);
    tmp12 = MULTIPLY((DCTELEM) wsptr[2] - (DCTELEM) wsptr[6], FIX_1_414213562)
	    - tmp13;

    tmp0 = tmp10 + tmp13;
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    z13 = (DCTELEM) wsptr[5] + (DCTELEM) wsptr[3];
    z10 = (DCTELEM) wsptr[5] - (DCTELEM) wsptr[3];
    z11 = (DCTELEM) wsptr[1] + (DCTELEM) wsptr[7];
    z12 = (DCTELEM) wsptr[1] - (DCTELEM) wsptr[7];

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); /* 2*c4 */

    z5 = MULTIPLY(z10 + z12, FIX_1_847759065); /* 2*c2 */
    tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
    tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    /* Final output stage: scale down by a factor of 8 and range-limit */

    outptr[0] = range_limit[IDESCALE(tmp0 + tmp7, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[7] = range_limit[IDESCALE(tmp0 - tmp7, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[1] = range_limit[IDESCALE(tmp1 + tmp6, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[6] = range_limit[IDESCALE(tmp1 - tmp6, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[2] = range_limit[IDESCALE(tmp2 + tmp5, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[5] = range_limit[IDESCALE(tmp2 - tmp5, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[4] = range_limit[IDESCALE(tmp3 + tmp4, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[3] = range_limit[IDESCALE(tmp3 - tmp4, PASS1_BITS+3)
			    & RANGE_MASK];

    wsptr += DCTSIZE;		/* advance pointer to next row */
  }
#endif
}

#endif /* DCT_IFAST_SUPPORTED */
