;============================================================================
;  bandwidth, a benchmark to estimate memory transfer bandwidth.
;  Copyright (C) 2005-2023 by Zack T Smith.
;
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
;
;  The author may be reached at 1 at zsmith dot co.
;=============================================================================

%ifidn __OUTPUT_FORMAT__, win64
; Windows 64 ABI says integer param are put in these registers in this order:
;	rcx, rdx, r8, r9 (floating point is xmm0)
	%define P1 rcx
	%define P2 rdx
	%define P3 r8
	%define P4 r9
%else
; Unix ABI says integer param are put in these registers in this order:
;	rdi, rsi, rdx, rcx, r8, r9
	%define P1 rdi
	%define P2 rsi
	%define P3 rdx
	%define P4 rcx
	%define P5 r8
	%define P6 r9
%endif

bits	64
cpu	ia64

global	CopyWithMainRegisters
global	_CopyWithMainRegisters
global	CopySSE
global	_CopySSE
global	CopyAVX
global	_CopyAVX
global	CopyAVX512
global	_CopyAVX512

global	IncrementRegisters
global	_IncrementRegisters
global	IncrementStack
global	_IncrementStack

global	Reader_nontemporal
global	_Reader_nontemporal
global	Writer_nontemporal
global	_Writer_nontemporal
global	RandomReader
global	RandomReaderSSE2
global	RandomReaderSSE4_nontemporal
global	RandomWriter
global	RandomWriterSSE2
global	RandomWriterSSE2_nontemporal
global	Reader
global	ReaderAVX
global	ReaderAVX512
global	RandomReaderAVX
global	ReaderSSE2
global	ReaderSSE4_nontemporal
global	ReaderAVX_nontemporal
global	_ReaderAVX_nontemporal
global	ReaderAVX512_nontemporal
global	_ReaderAVX512_nontemporal
global	Register16ToVector
global	Register32ToVector
global	Register64ToVector
global	Register8ToVector
global	RegisterToRegister
global	RegisterToVector
global	StackReader
global	StackWriter
global	Vector16ToRegister
global	Vector32ToRegister
global	Vector64ToRegister
global	Vector8ToRegister
global	VectorToRegister
global	VectorToVector128
global	VectorToVector256
global	VectorToVector512
global	Writer
global	WriterAVX
global	WriterAVX512
global	RandomWriterAVX
global	RandomWriterAVX_nontemporal
global	WriterSSE2
global	WriterSSE2_nontemporal
global	WriterAVX_nontemporal
global	_WriterAVX_nontemporal
global	WriterAVX512_nontemporal
global	_WriterAVX512_nontemporal
global	_RandomReader
global	_RandomReaderSSE2
global	_RandomReaderSSE4_nontemporal
global	_RandomWriter
global	_RandomWriterSSE2
global	_RandomWriterSSE2_nontemporal
global	_Reader
global	_ReaderAVX
global	_ReaderAVX512
global	_RandomReaderAVX
global	_ReaderSSE2
global	_ReaderSSE4_nontemporal
global	_Register16ToVector
global	_Register32ToVector
global	_Register64ToVector
global	_Register8ToVector
global	_RegisterToRegister
global	_RegisterToVector
global	_StackReader
global	_StackWriter
global	_Vector16ToRegister
global	_Vector32ToRegister
global	_Vector64ToRegister
global	_Vector8ToRegister
global	_VectorToRegister
global	_VectorToVector
global	_VectorToVectorAVX
global	_VectorToVector128
global	_VectorToVector256
global	_VectorToVector512
global	_Writer
global	_WriterSSE2
global	_WriterAVX
global	_WriterAVX512
global	_RandomWriterAVX
global	_RandomWriterAVX_nontemporal
global	_WriterSSE2_nontemporal

%ifidn __OUTPUT_FORMAT__, elf64
	; Not compatible with macOS:
	section .note.GNU-stack 
%endif

	section .text

;------------------------------------------------------------------------------
; Name:		Reader
; Purpose:	Reads 64-bit values sequentially from an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
Reader:
_Reader:
	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	mov	rax, [r10]
	mov	rax, [8+r10]
	mov	rax, [16+r10]
	mov	rax, [24+r10]
	mov	rax, [32+r10]
	mov	rax, [40+r10]
	mov	rax, [48+r10]
	mov	rax, [56+r10]
	mov	rax, [64+r10]
	mov	rax, [72+r10]
	mov	rax, [80+r10]
	mov	rax, [88+r10]
	mov	rax, [96+r10]
	mov	rax, [104+r10]
	mov	rax, [112+r10]
	mov	rax, [120+r10]
	mov	rax, [128+r10]
	mov	rax, [136+r10]
	mov	rax, [144+r10]
	mov	rax, [152+r10]
	mov	rax, [160+r10]
	mov	rax, [168+r10]
	mov	rax, [176+r10]
	mov	rax, [184+r10]
	mov	rax, [192+r10]
	mov	rax, [200+r10]
	mov	rax, [208+r10]
	mov	rax, [216+r10]
	mov	rax, [224+r10]
	mov	rax, [232+r10]
	mov	rax, [240+r10]
	mov	rax, [248+r10]

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomReader
; Purpose:	Reads 64-bit values randomly from an area of memory.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
RandomReader:
_RandomReader:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]	; Note, 64-bit pointers.

	mov	rax, [96+r10]
	mov	rax, [r10]
	mov	rax, [120+r10]
	mov	rax, [184+r10]
	mov	rax, [160+r10]
	mov	rax, [176+r10]
	mov	rax, [112+r10]
	mov	rax, [80+r10]
	mov	rax, [32+r10]
	mov	rax, [128+r10]
	mov	rax, [88+r10]
	mov	rax, [40+r10]
	mov	rax, [48+r10]
	mov	rax, [72+r10]
	mov	rax, [200+r10]
	mov	rax, [24+r10]
	mov	rax, [152+r10]
	mov	rax, [16+r10]
	mov	rax, [248+r10]
	mov	rax, [56+r10]
	mov	rax, [240+r10]
	mov	rax, [208+r10]
	mov	rax, [104+r10]
	mov	rax, [216+r10]
	mov	rax, [136+r10]
	mov	rax, [232+r10]
	mov	rax, [64+r10]
	mov	rax, [224+r10]
	mov	rax, [144+r10]
	mov	rax, [192+r10]
	mov	rax, [8+r10]
	mov	rax, [168+r10]

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomReaderSSE2
; Purpose:	Reads 128-bit values randomly from an area of memory.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
RandomReaderSSE2:
_RandomReaderSSE2:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]

	movdqa	xmm0, [240+r10]
	movdqa	xmm0, [128+r10]
	movdqa	xmm0, [64+r10]
	movdqa	xmm0, [208+r10]
	movdqa	xmm0, [112+r10]
	movdqa	xmm0, [176+r10]
	movdqa	xmm0, [144+r10]
	movdqa	xmm0, [r10]
	movdqa	xmm0, [96+r10]
	movdqa	xmm0, [16+r10]
	movdqa	xmm0, [192+r10]
	movdqa	xmm0, [160+r10]
	movdqa	xmm0, [32+r10]
	movdqa	xmm0, [48+r10]
	movdqa	xmm0, [224+r10]
	movdqa	xmm0, [80+r10]

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomReaderSSE4_nontemporal
; Purpose:	Reads 128-bit values randomly from an area of memory.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
RandomReaderSSE4_nontemporal:
_RandomReaderSSE4_nontemporal:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]

	prefetchnta	[r10+192]
	movntdqa	xmm0, [240+r10]
	prefetchnta	[r10]
	movntdqa	xmm0, [r10]
	prefetchnta	[r10+128]
	movntdqa	xmm0, [128+r10]
	prefetchnta	[r10+64]
	movntdqa	xmm0, [64+r10]
	movntdqa	xmm0, [208+r10]
	movntdqa	xmm0, [112+r10]
	movntdqa	xmm0, [48+r10]
	movntdqa	xmm0, [176+r10]
	movntdqa	xmm0, [144+r10]
	movntdqa	xmm0, [96+r10]
	movntdqa	xmm0, [16+r10]
	movntdqa	xmm0, [160+r10]
	movntdqa	xmm0, [32+r10]
	movntdqa	xmm0, [224+r10]
	movntdqa	xmm0, [80+r10]
	movntdqa	xmm0, [192+r10]

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriter
; Purpose:	Writes 64-bit values randomly to an area of memory.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
; 		P4 = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriter:
_RandomWriter:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]	; Note, 64-bit pointers.

	mov	[96+r10], P4
	mov	[r10], P4
	mov	[120+r10], P4
	mov	[184+r10], P4
	mov	[160+r10], P4
	mov	[176+r10], P4
	mov	[112+r10], P4
	mov	[80+r10], P4
	mov	[32+r10], P4
	mov	[128+r10], P4
	mov	[88+r10], P4
	mov	[40+r10], P4
	mov	[48+r10], P4
	mov	[72+r10], P4
	mov	[200+r10], P4
	mov	[24+r10], P4
	mov	[152+r10], P4
	mov	[16+r10], P4
	mov	[248+r10], P4
	mov	[56+r10], P4
	mov	[240+r10], P4
	mov	[208+r10], P4
	mov	[104+r10], P4
	mov	[216+r10], P4
	mov	[136+r10], P4
	mov	[232+r10], P4
	mov	[64+r10], P4
	mov	[224+r10], P4
	mov	[144+r10], P4
	mov	[192+r10], P4
	mov	[8+r10], P4
	mov	[168+r10], P4

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriterSSE2
; Purpose:	Writes 128-bit values randomly to an area of memory.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
; 		P4 = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriterSSE2:
_RandomWriterSSE2:
	push	r10
	push	r11

	movq	xmm0, P4	; Create duplicated 128-bit datum
	movq	xmm1, P4
	pslldq	xmm1, 64
	por	xmm0, xmm1

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]	; Note, 64-bit pointers.

	movdqa	[240+r10], xmm0
	movdqa	[128+r10], xmm0
	movdqa	[208+r10], xmm0
	movdqa	[112+r10], xmm0
	movdqa	[64+r10], xmm0
	movdqa	[176+r10], xmm0
	movdqa	[144+r10], xmm0
	movdqa	[r10], xmm0
	movdqa	[96+r10], xmm0
	movdqa	[16+r10], xmm0
	movdqa	[192+r10], xmm0
	movdqa	[160+r10], xmm0
	movdqa	[32+r10], xmm0
	movdqa	[48+r10], xmm0
	movdqa	[224+r10], xmm0
	movdqa	[80+r10], xmm0

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriterSSE2_nontemporal
; Purpose:	Nontemporal writes of 128-bit values randomly into memory.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
; 		P4 = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriterSSE2_nontemporal:
_RandomWriterSSE2_nontemporal:
	push	r10
	push	r11

	movq	xmm0, P4	; Create duplicated 128-bit datum
	movq	xmm1, P4
	pslldq	xmm1, 64
	por	xmm0, xmm1

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]	; Note, 64-bit pointers.

	movntdq	[240+r10], xmm0
	movntdq	[128+r10], xmm0
	movntdq	[208+r10], xmm0
	movntdq	[112+r10], xmm0
	movntdq	[64+r10], xmm0
	movntdq	[176+r10], xmm0
	movntdq	[144+r10], xmm0
	movntdq	[r10], xmm0
	movntdq	[96+r10], xmm0
	movntdq	[16+r10], xmm0
	movntdq	[192+r10], xmm0
	movntdq	[160+r10], xmm0
	movntdq	[32+r10], xmm0
	movntdq	[48+r10], xmm0
	movntdq	[224+r10], xmm0
	movntdq	[80+r10], xmm0

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		ReaderSSE2
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2:
_ReaderSSE2:
	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	movdqa	xmm0, [r10]	; Read aligned to 16-byte boundary.
	movdqa	xmm0, [16+r10]
	movdqa	xmm0, [32+r10]
	movdqa	xmm0, [48+r10]
	movdqa	xmm0, [64+r10]
	movdqa	xmm0, [80+r10]
	movdqa	xmm0, [96+r10]
	movdqa	xmm0, [112+r10]

	movdqa	xmm0, [128+r10]
	movdqa	xmm0, [144+r10]
	movdqa	xmm0, [160+r10]
	movdqa	xmm0, [176+r10]
	movdqa	xmm0, [192+r10]
	movdqa	xmm0, [208+r10]
	movdqa	xmm0, [224+r10]
	movdqa	xmm0, [240+r10]

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1
	
	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		ReaderAVX
; Purpose:	Reads 256-bit values sequentially from an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
ReaderAVX:
_ReaderAVX:
	sub	rsp, 32
	vmovdqu	[rsp], ymm0
	sub	rsp, 32
	vmovdqu	[rsp], ymm1
	sub	rsp, 32
	vmovdqu	[rsp], ymm2
	sub	rsp, 32
	vmovdqu	[rsp], ymm3

	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	vmovdqa	ymm0, [r10]	; Read aligned to 32-byte boundary.
	vmovdqa	ymm1, [32+r10]
	vmovdqa	ymm2, [64+r10]
	vmovdqa	ymm3, [96+r10]
	vmovdqa	ymm0, [128+r10]
	vmovdqa	ymm1, [160+r10]
	vmovdqa	ymm2, [192+r10]
	vmovdqa	ymm3, [224+r10]

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1
	
	pop	r10
	vmovdqu	ymm3, [rsp]
	add	rsp, 32
	vmovdqu	ymm2, [rsp]
	add	rsp, 32
	vmovdqu	ymm1, [rsp]
	add	rsp, 32
	vmovdqu	ymm0, [rsp]
	add	rsp, 32

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		ReaderAVX512
; Purpose:	Reads 512-bit values sequentially from an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
ReaderAVX512:
_ReaderAVX512:
	sub	rsp, 64
	vmovdqu64	[rsp], zmm0
	sub	rsp, 64
	vmovdqu64	[rsp], zmm1
	sub	rsp, 64
	vmovdqu64	[rsp], zmm2
	sub	rsp, 64
	vmovdqu64	[rsp], zmm3

	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	vmovdqu64	zmm0, [r10]	; Read aligned to 64-byte boundary.
	vmovdqu64	zmm1, [64+r10]
	vmovdqu64	zmm2, [128+r10]
	vmovdqu64	zmm3, [192+r10]

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1
	
	pop	r10

	vmovdqu64	zmm3, [rsp]
	add	rsp, 64
	vmovdqu64	zmm2, [rsp]
	add	rsp, 64
	vmovdqu64	zmm1, [rsp]
	add	rsp, 64
	vmovdqu64	zmm0, [rsp]
	add	rsp, 64

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomReaderAVX
; Purpose:	Reads 256-bit values in somewhat random order from RAM.
; Params:	P1 = ptr to chunk pointers 
; 		P2 = # chunks
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
RandomReaderAVX:
_RandomReaderAVX:
	sub	rsp, 32
	vmovdqu	[rsp], ymm0
	sub	rsp, 32
	vmovdqu	[rsp], ymm1
	sub	rsp, 32
	vmovdqu	[rsp], ymm2
	sub	rsp, 32
	vmovdqu	[rsp], ymm3

	push	r10
	push	r11

.L1:
	xor	r11, r11
.L2:
        mov     r10, [P1 + 8*r11]

	; Read aligned to 32-byte boundary.
	vmovdqa	ymm0, [96+r10]
	vmovdqa	ymm1, [192+r10]
	vmovdqa	ymm2, [64+r10]
	vmovdqa	ymm3, [224+r10]
	vmovdqa	ymm0, [r10]	
	vmovdqa	ymm1, [160+r10]
	vmovdqa	ymm2, [128+r10]
	vmovdqa	ymm3, [32+r10]

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1
	
	pop	r11
	pop	r10

	vmovdqu	ymm3, [rsp]
	add	rsp, 32
	vmovdqu	ymm2, [rsp]
	add	rsp, 32
	vmovdqu	ymm1, [rsp]
	add	rsp, 32
	vmovdqu	ymm0, [rsp]
	add	rsp, 32

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		ReaderSSE4_nontemporal
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE4_nontemporal:
_ReaderSSE4_nontemporal:
	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	prefetchnta	[r10]
	prefetchnta	[r10+64]

	movntdqa	xmm0, [r10]	; Read aligned to 16-byte boundary.
	movntdqa	xmm0, [16+r10]
	movntdqa	xmm0, [32+r10]
	movntdqa	xmm0, [48+r10]
	movntdqa	xmm0, [64+r10]
	movntdqa	xmm0, [80+r10]
	movntdqa	xmm0, [96+r10]
	movntdqa	xmm0, [112+r10]

	prefetchnta	[r10+128]
	prefetchnta	[r10+192]

	movntdqa	xmm0, [128+r10]
	movntdqa	xmm0, [144+r10]
	movntdqa	xmm0, [160+r10]
	movntdqa	xmm0, [176+r10]
	movntdqa	xmm0, [192+r10]
	movntdqa	xmm0, [208+r10]
	movntdqa	xmm0, [224+r10]
	movntdqa	xmm0, [240+r10]

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1
	
	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		Writer
; Purpose:	Writes 64-bit value sequentially to an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
Writer:
_Writer:
	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	mov	[r10], P4
	mov	[8+r10], P4
	mov	[16+r10], P4
	mov	[24+r10], P4
	mov	[32+r10], P4
	mov	[40+r10], P4
	mov	[48+r10], P4
	mov	[56+r10], P4
	mov	[64+r10], P4
	mov	[72+r10], P4
	mov	[80+r10], P4
	mov	[88+r10], P4
	mov	[96+r10], P4
	mov	[104+r10], P4
	mov	[112+r10], P4
	mov	[120+r10], P4
	mov	[128+r10], P4
	mov	[136+r10], P4
	mov	[144+r10], P4
	mov	[152+r10], P4
	mov	[160+r10], P4
	mov	[168+r10], P4
	mov	[176+r10], P4
	mov	[184+r10], P4
	mov	[192+r10], P4
	mov	[200+r10], P4
	mov	[208+r10], P4
	mov	[216+r10], P4
	mov	[224+r10], P4
	mov	[232+r10], P4
	mov	[240+r10], P4
	mov	[248+r10], P4

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		Writer_nontemporal
; Purpose:	Writes 64-bit value sequentially to memory w/nontemporal hint.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
Writer_nontemporal:
_Writer_nontemporal:
	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	movnti	[r10], P4
	movnti	[8+r10], P4
	movnti	[16+r10], P4
	movnti	[24+r10], P4
	movnti	[32+r10], P4
	movnti	[40+r10], P4
	movnti	[48+r10], P4
	movnti	[56+r10], P4
	movnti	[64+r10], P4
	movnti	[72+r10], P4
	movnti	[80+r10], P4
	movnti	[88+r10], P4
	movnti	[96+r10], P4
	movnti	[104+r10], P4
	movnti	[112+r10], P4
	movnti	[120+r10], P4
	movnti	[128+r10], P4
	movnti	[136+r10], P4
	movnti	[144+r10], P4
	movnti	[152+r10], P4
	movnti	[160+r10], P4
	movnti	[168+r10], P4
	movnti	[176+r10], P4
	movnti	[184+r10], P4
	movnti	[192+r10], P4
	movnti	[200+r10], P4
	movnti	[208+r10], P4
	movnti	[216+r10], P4
	movnti	[224+r10], P4
	movnti	[232+r10], P4
	movnti	[240+r10], P4
	movnti	[248+r10], P4

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2
; Purpose:	Writes 128-bit value sequentially to an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
WriterSSE2:
_WriterSSE2:
	push	r10

	add	P2, P1	; P2 now points to end.

	movq	xmm0, P4

.L1:
	mov	r10, P1

.L2:
	movdqa	[r10], xmm0
	movdqa	[16+r10], xmm0
	movdqa	[32+r10], xmm0
	movdqa	[48+r10], xmm0
	movdqa	[64+r10], xmm0
	movdqa	[80+r10], xmm0
	movdqa	[96+r10], xmm0
	movdqa	[112+r10], xmm0

	movdqa	[128+r10], xmm0
	movdqa	[144+r10], xmm0
	movdqa	[160+r10], xmm0
	movdqa	[176+r10], xmm0
	movdqa	[192+r10], xmm0
	movdqa	[208+r10], xmm0
	movdqa	[224+r10], xmm0
	movdqa	[240+r10], xmm0

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX
; Purpose:	Writes 256-bit value sequentially to an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
WriterAVX:
_WriterAVX:
	vzeroupper

	push	r10

	add	P2, P1	; P2 now points to end.

	pinsrq	xmm0, P4, 0
	pinsrq	xmm0, P4, 1

.L1:
	mov	r10, P1

.L2:
	vmovdqa	[r10], ymm0
	vmovdqa	[32+r10], ymm0
	vmovdqa	[64+r10], ymm0
	vmovdqa	[96+r10], ymm0
	vmovdqa	[128+r10], ymm0
	vmovdqa	[160+r10], ymm0
	vmovdqa	[192+r10], ymm0
	vmovdqa	[224+r10], ymm0

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX512
; Purpose:	Writes 512-bit value sequentially to an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
WriterAVX512:
_WriterAVX512:
	sub	rsp, 64
	vmovdqu64	[rsp], zmm0

	push	r10

	add	P2, P1	; P2 now points to end.

	push	P4
	push	P4
	vbroadcasti64x2	zmm0, [rsp]
	add	rsp, 16

.L1:
	mov	r10, P1

.L2:
	vmovdqa64	[r10], zmm0
	vmovdqa64	[64+r10], zmm0
	vmovdqa64	[128+r10], zmm0
	vmovdqa64	[192+r10], zmm0

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	vmovdqu64	zmm0, [rsp]
	add	rsp, 64

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX512_nontemporal
; Purpose:	Writes 512-bit value sequentially to an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
WriterAVX512_nontemporal:
_WriterAVX512_nontemporal:
	sub	rsp, 64
	vmovdqu64	[rsp], zmm0

	push	r10

	add	P2, P1	; P2 now points to end.

	push	P4
	push	P4
	vbroadcasti64x2	zmm0, [rsp]
	add	rsp, 16

.L1:
	mov	r10, P1

.L2:
	vmovntdq	[r10], zmm0
	vmovntdq	[64+r10], zmm0
	vmovntdq	[128+r10], zmm0
	vmovntdq	[192+r10], zmm0

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	vmovdqu64	zmm0, [rsp]
	add	rsp, 64

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriterAVX
; Purpose:	Writes 256-bit value in somewhat random order to RAM.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
; 		P4 = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriterAVX:
_RandomWriterAVX:
	push	r10
	push	r11

	sub	rsp, 32
	mov	[rsp], P4
	mov	[rsp+8], P4
	mov	[rsp+16], P4
	mov	[rsp+24], P4
	vmovdqu	ymm0, [rsp]
	add	rsp, 32

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]	; Note, 64-bit pointers.

	vmovdqa	[192+r10], ymm0
	vmovdqa	[224+r10], ymm0
	vmovdqa	[64+r10], ymm0
	vmovdqa	[128+r10], ymm0
	vmovdqa	[r10], ymm0
	vmovdqa	[96+r10], ymm0
	vmovdqa	[160+r10], ymm0
	vmovdqa	[32+r10], ymm0

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriterAVX_nontemporal
; Purpose:	Writes 256-bit value in somewhat random order to RAM.
; Params:	P1 = ptr to array of chunk pointers
; 		P2 = # of chunks
; 		P3 = loops
; 		P4 = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriterAVX_nontemporal:
_RandomWriterAVX_nontemporal:
	push	r10
	push	r11

	sub	rsp, 32
	mov	[rsp], P4
	mov	[rsp+8], P4
	mov	[rsp+16], P4
	mov	[rsp+24], P4
	vmovdqu	ymm0, [rsp]
	add	rsp, 32

.L1:
	xor	r11, r11

.L2:
	mov	r10, [P1 + 8*r11]	; Note, 64-bit pointers.

	vmovntdq	[192+r10], ymm0
	vmovntdq	[64+r10], ymm0
	vmovntdq	[224+r10], ymm0
	vmovntdq	[r10], ymm0
	vmovntdq	[128+r10], ymm0
	vmovntdq	[32+r10], ymm0
	vmovntdq	[96+r10], ymm0
	vmovntdq	[160+r10], ymm0

	inc	r11
	cmp	r11, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r11
	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2_nontemporal
; Purpose:	Writes 128-bit value sequentially to an area of memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
WriterSSE2_nontemporal:
_WriterSSE2_nontemporal:
	push	r10

	add	P2, P1	; P2 now points to end.

	movq	xmm0, P4

.L1:
	mov	r10, P1

.L2:
	movntdq	[r10], xmm0	
	movntdq	[16+r10], xmm0
	movntdq	[32+r10], xmm0
	movntdq	[48+r10], xmm0
	movntdq	[64+r10], xmm0
	movntdq	[80+r10], xmm0
	movntdq	[96+r10], xmm0
	movntdq	[112+r10], xmm0

	movntdq	[128+r10], xmm0
	movntdq	[144+r10], xmm0
	movntdq	[160+r10], xmm0
	movntdq	[176+r10], xmm0
	movntdq	[192+r10], xmm0
	movntdq	[208+r10], xmm0
	movntdq	[224+r10], xmm0
	movntdq	[240+r10], xmm0

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX_nontemporal
; Purpose:	Nontemporal writes of 256-bit values sequentially into memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
; 		P4 = quad to write
;------------------------------------------------------------------------------
	align 64
WriterAVX_nontemporal:
_WriterAVX_nontemporal:
	vzeroupper

	push	r10

	add	P2, P1	; P2 now points to end.

	movq	xmm0, P4

.L1:
	mov	r10, P1

.L2:
	vmovntdq	[r10], ymm0	
	vmovntdq	[32+r10], ymm0
	vmovntdq	[64+r10], ymm0
	vmovntdq	[96+r10], ymm0
	vmovntdq	[128+r10], ymm0
	vmovntdq	[160+r10], ymm0
	vmovntdq	[192+r10], ymm0
	vmovntdq	[224+r10], ymm0

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		ReaderAVX_nontemporal
; Purpose:	Nontemporal reads of 256-bit values sequentially from memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
ReaderAVX_nontemporal:
_ReaderAVX_nontemporal:
	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	; This requires AVX512F.
	vmovntdqa	ymm0, [r10]	
	vmovntdqa	ymm0, [32+r10]
	vmovntdqa	ymm0, [64+r10]
	vmovntdqa	ymm0, [96+r10]
	vmovntdqa	ymm0, [128+r10]
	vmovntdqa	ymm0, [160+r10]
	vmovntdqa	ymm0, [192+r10]
	vmovntdqa	ymm0, [224+r10]

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		ReaderAVX512_nontemporal
; Purpose:	Nontemporal reads of 512-bit values sequentially from memory.
; Params:	P1 = ptr to memory area
; 		P2 = length in bytes
; 		P3 = loops
;------------------------------------------------------------------------------
	align 64
ReaderAVX512_nontemporal:
_ReaderAVX512_nontemporal:
	push	r10

	add	P2, P1	; P2 now points to end.

.L1:
	mov	r10, P1

.L2:
	; This requires AVX512F.
	vmovntdqa	zmm0, [r10]	
	vmovntdqa	zmm0, [64+r10]
	vmovntdqa	zmm0, [128+r10]
	vmovntdqa	zmm0, [192+r10]

	add	r10, 256
	cmp	r10, P2
	jb	.L2

	dec	P3
	jnz	.L1

	pop	r10

	sfence
	ret

;------------------------------------------------------------------------------
; Name:		StackReader
; Purpose:	Reads 64-bit values off the stack into registers of
;		the main register set, effectively testing L1 cache access
;		*and* effective-address calculation speed.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
StackReader:
_StackReader:
	push	qword 8000	; [rsp+56]
	push	qword 7000	; [rsp+48]
	push	qword 6000	; [rsp+40]
	push	qword 5000	; [rsp+32]
	push	qword 4000	; [rsp+24]
	push	qword 3000	; [rsp+16]
	push	qword 2000	; [rsp+8]
	push	qword 1000	; [rsp]

.L1:
	; 64 transfers
	mov	rax, [rsp]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp]
	mov	rax, [rsp+56]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+56]
	mov	rax, [rsp]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+8]
	mov	rax, [rsp+56]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+56]
	mov	rax, [rsp]

	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp]
	mov	rax, [rsp+56]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+56]
	mov	rax, [rsp]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+8]
	mov	rax, [rsp+56]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+56]

	sub	P1, 1
	jnz	.L1

	add	rsp, 64

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		StackWriter
; Purpose:	Writes 64-bit values into the stack from registers of
;		the main register set, effectively testing L1 cache access
;		*and* effective-address calculation speed.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
StackWriter:
_StackWriter:
	push	qword 8000	; [rsp+56]
	push	qword 7000	; [rsp+48]
	push	qword 6000	; [rsp+40]
	push	qword 5000	; [rsp+32]
	push	qword 4000	; [rsp+24]
	push	qword 3000	; [rsp+16]
	push	qword 2000	; [rsp+8]
	push	qword 1000	; [rsp]

	xor	rax, rax

.L1:
	; 64 transfers
	mov	[rsp], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp], rax
	mov	[rsp+56], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+56], rax
	mov	[rsp], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+8], rax
	mov	[rsp+56], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+56], rax

	mov	[rsp], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp], rax
	mov	[rsp+56], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+56], rax
	mov	[rsp], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+8], rax
	mov	[rsp+56], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+56], rax

	sub	P1, 1
	jnz	.L1

	add	rsp, 64

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		RegisterToRegister
; Purpose:	Reads/writes 64-bit values between registers of 
;		the main register set. Total bytes transferred is 8*32=256.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
RegisterToRegister:
_RegisterToRegister:
.L1:
	mov	rax, rbx	; 64 transfers
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx

	mov	rax, rbx
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, P4
	mov	rax, P3
	mov	rax, P2
	mov	rax, P1
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx

	sub	P1, 1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToVector128
; Purpose:	Reads/writes 128-bit values between registers of 
;		the vector register set, in this case XMM.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
VectorToVector128:
_VectorToVector128:
.L1:
	; 32 transfers
	movq	xmm0, xmm1
	movq	xmm14, xmm2
	movq	xmm0, xmm4
	movq	xmm9, xmm0
	movq	xmm1, xmm12
	movq	xmm7, xmm1
	movq	xmm8, xmm3
	movq	xmm3, xmm1

	movq	xmm13, xmm2
	movq	xmm11, xmm3
	movq	xmm2, xmm10
	movq	xmm6, xmm1
	movq	xmm1, xmm2
	movq	xmm15, xmm1
	movq	xmm0, xmm3
	movq	xmm3, xmm14

	movq	xmm0, xmm1
	movq	xmm14, xmm2
	movq	xmm0, xmm4
	movq	xmm9, xmm0
	movq	xmm1, xmm12
	movq	xmm7, xmm1
	movq	xmm8, xmm3
	movq	xmm3, xmm1

	movq	xmm13, xmm2
	movq	xmm11, xmm3
	movq	xmm2, xmm10
	movq	xmm6, xmm1
	movq	xmm1, xmm2
	movq	xmm15, xmm1
	movq	xmm0, xmm3
	movq	xmm3, xmm14

	sub	P1, 1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToVector256
; Purpose:	Reads/writes 256-bit values between vector registers,
;		which on the x86 are YMM registers.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
VectorToVector256:
_VectorToVector256:
	vzeroupper

.L1:
	; 32 transfers
	vmovdqa	ymm0, ymm1	
	vmovdqa	ymm0, ymm2
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm2, ymm0
	vmovdqa	ymm1, ymm2
	vmovdqa	ymm2, ymm1
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm3, ymm1

	vmovdqa	ymm0, ymm1	
	vmovdqa	ymm0, ymm2
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm2, ymm0
	vmovdqa	ymm1, ymm2
	vmovdqa	ymm2, ymm1
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm3, ymm1

	vmovdqa	ymm0, ymm1	
	vmovdqa	ymm0, ymm2
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm2, ymm0
	vmovdqa	ymm1, ymm2
	vmovdqa	ymm2, ymm1
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm3, ymm1

	vmovdqa	ymm0, ymm1	
	vmovdqa	ymm0, ymm2
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm2, ymm0
	vmovdqa	ymm1, ymm2
	vmovdqa	ymm2, ymm1
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm3, ymm1

	sub	P1, 1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToVector512
; Purpose:	Reads/writes 512-bit values between vector registers,
;		which on the x86 are ZMM registers.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
VectorToVector512:
_VectorToVector512:

.L1:
	; 32 transfers
	vmovdqa64	zmm0, zmm1	
	vmovdqa64	zmm0, zmm2
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm2, zmm0
	vmovdqa64	zmm1, zmm2
	vmovdqa64	zmm2, zmm1
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm3, zmm1

	vmovdqa64	zmm0, zmm1	
	vmovdqa64	zmm0, zmm2
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm2, zmm0
	vmovdqa64	zmm1, zmm2
	vmovdqa64	zmm2, zmm1
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm3, zmm1

	vmovdqa64	zmm0, zmm1	
	vmovdqa64	zmm0, zmm2
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm2, zmm0
	vmovdqa64	zmm1, zmm2
	vmovdqa64	zmm2, zmm1
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm3, zmm1

	vmovdqa64	zmm0, zmm1	
	vmovdqa64	zmm0, zmm2
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm2, zmm0
	vmovdqa64	zmm1, zmm2
	vmovdqa64	zmm2, zmm1
	vmovdqa64	zmm0, zmm3
	vmovdqa64	zmm3, zmm1

	sub	P1, 1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		RegisterToVector
; Purpose:	Writes 64-bit main register values into 128-bit vector register
;		clearing the upper unused bits.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
RegisterToVector:
_RegisterToVector:
.L1:
	movq	xmm1, rax 	; Each movq transfers 8 bytes, so we need
	movq	xmm2, P2	; 32 transfers to move a 256-byte chunk.
	movq	xmm3, rbx
	movq	xmm1, P4
	movq	xmm2, P2
	movq	xmm3, rsp
	movq	xmm0, P1
	movq	xmm0, P3

	movq	xmm0, rax 	
	movq	xmm1, P2
	movq	xmm2, rbx
	movq	xmm3, P4
	movq	xmm0, P2
	movq	xmm3, rsp
	movq	xmm2, P1
	movq	xmm1, P3

	movq	xmm0, rax 	
	movq	xmm1, P2
	movq	xmm2, rbx
	movq	xmm3, P4
	movq	xmm0, P2
	movq	xmm3, rsp
	movq	xmm2, P1
	movq	xmm1, P3

	movq	xmm0, rax 	
	movq	xmm1, P2
	movq	xmm2, rbx
	movq	xmm3, P4
	movq	xmm0, P2
	movq	xmm3, rsp
	movq	xmm2, P1
	movq	xmm1, P3

	dec	P1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToRegister
; Purpose:	Writes lower 64 bits of vector register into 64-bit main 
;		register.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
VectorToRegister:
_VectorToRegister:
.L1:
	movq	rax, xmm1
	movq	rax, xmm2
	movq	rax, xmm3
	movq	rax, xmm1
	movq	rax, xmm2
	movq	rax, xmm3
	movq	rax, xmm0
	movq	rax, xmm0

	movq	rax, xmm0
	movq	rax, xmm1
	movq	rax, xmm2
	movq	rax, xmm3
	movq	rax, xmm0
	movq	rax, xmm3
	movq	rax, xmm2
	movq	rax, xmm1

	movq	rax, xmm0
	movq	rax, xmm1
	movq	rax, xmm2
	movq	rax, xmm3
	movq	rax, xmm0
	movq	rax, xmm3
	movq	rax, xmm2
	movq	rax, xmm1

	movq	rax, xmm0
	movq	rax, xmm1
	movq	rax, xmm2
	movq	rax, xmm3
	movq	rax, xmm0
	movq	rax, xmm3
	movq	rax, xmm2
	movq	rax, xmm1

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register8ToVector
; Purpose:	Writes 8-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Register8ToVector:
_Register8ToVector:

.L1:
	pinsrb	xmm1, al, 0	; 64 transfers x 1 byte = 64 bytes
	pinsrb	xmm2, bl, 1
	pinsrb	xmm3, cl, 2
	pinsrb	xmm1, dl, 3
	pinsrb	xmm2, sil, 4
	pinsrb	xmm3, dil, 5
	pinsrb	xmm0, bpl, 6
	pinsrb	xmm0, spl, 7

	pinsrb	xmm0, al, 0
	pinsrb	xmm1, bl, 1
	pinsrb	xmm2, cl, 2
	pinsrb	xmm3, dl, 3
	pinsrb	xmm3, al, 4
	pinsrb	xmm2, bl, 5
	pinsrb	xmm1, bpl, 6
	pinsrb	xmm0, spl, 7

	pinsrb	xmm1, r8b, 0
	pinsrb	xmm2, r9b, 1
	pinsrb	xmm3, r10b, 2
	pinsrb	xmm1, r11b, 3
	pinsrb	xmm2, r12b, 4
	pinsrb	xmm3, al, 5
	pinsrb	xmm0, cl, 6
	pinsrb	xmm0, bl, 7

	pinsrb	xmm0, r8b, 0
	pinsrb	xmm0, r9b, 1
	pinsrb	xmm0, r10b, 2
	pinsrb	xmm0, r11b, 3
	pinsrb	xmm0, r12b, 4
	pinsrb	xmm0, al, 5
	pinsrb	xmm0, cl, 6
	pinsrb	xmm0, bl, 7

	pinsrb	xmm1, al, 0
	pinsrb	xmm2, bl, 1
	pinsrb	xmm3, cl, 2
	pinsrb	xmm1, dl, 3
	pinsrb	xmm2, sil, 4
	pinsrb	xmm3, dil, 5
	pinsrb	xmm0, bpl, 6
	pinsrb	xmm0, spl, 7

	pinsrb	xmm0, al, 10
	pinsrb	xmm1, bl, 11
	pinsrb	xmm2, cl, 12
	pinsrb	xmm3, dl, 13
	pinsrb	xmm3, dil, 14
	pinsrb	xmm2, cl, 15
	pinsrb	xmm1, al, 6
	pinsrb	xmm0, bpl, 7

	pinsrb	xmm1, r8b, 10
	pinsrb	xmm2, r9b, 11
	pinsrb	xmm3, r10b, 12
	pinsrb	xmm1, r11b, 13
	pinsrb	xmm2, r12b, 14
	pinsrb	xmm3, al, 15
	pinsrb	xmm0, cl, 6
	pinsrb	xmm0, bl, 7

	pinsrb	xmm0, r8b, 9
	pinsrb	xmm0, r9b, 8
	pinsrb	xmm0, r10b, 11
	pinsrb	xmm0, r11b, 3
	pinsrb	xmm0, r12b, 4
	pinsrb	xmm0, al, 5
	pinsrb	xmm0, cl, 6
	pinsrb	xmm0, bl, 7

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register16ToVector
; Purpose:	Writes 16-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Register16ToVector:
_Register16ToVector:

.L1:
	pinsrw	xmm1, ax, 0	; 64 transfers x 2 bytes = 128 bytes
	pinsrw	xmm2, bx, 1
	pinsrw	xmm3, cx, 2
	pinsrw	xmm1, dx, 3
	pinsrw	xmm2, si, 4
	pinsrw	xmm3, di, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, sp, 7

	pinsrw	xmm0, ax, 0
	pinsrw	xmm1, bx, 1
	pinsrw	xmm2, cx, 2
	pinsrw	xmm3, dx, 3
	pinsrw	xmm3, si, 4
	pinsrw	xmm2, di, 5
	pinsrw	xmm1, bp, 6
	pinsrw	xmm0, sp, 7

	pinsrw	xmm1, r8w, 0
	pinsrw	xmm2, r9w, 1
	pinsrw	xmm3, r10w, 2
	pinsrw	xmm1, r11w, 3
	pinsrw	xmm2, r12w, 4
	pinsrw	xmm3, ax, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, bx, 7

	pinsrw	xmm0, r8w, 0
	pinsrw	xmm0, r9w, 1
	pinsrw	xmm0, r10w, 2
	pinsrw	xmm0, r11w, 3
	pinsrw	xmm0, r12w, 4
	pinsrw	xmm0, ax, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, bx, 7

	pinsrw	xmm1, ax, 0
	pinsrw	xmm2, bx, 1
	pinsrw	xmm3, cx, 2
	pinsrw	xmm1, dx, 3
	pinsrw	xmm2, si, 4
	pinsrw	xmm3, di, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, sp, 7

	pinsrw	xmm0, ax, 0
	pinsrw	xmm1, bx, 1
	pinsrw	xmm2, cx, 2
	pinsrw	xmm3, dx, 3
	pinsrw	xmm3, si, 4
	pinsrw	xmm2, di, 5
	pinsrw	xmm1, bp, 6
	pinsrw	xmm0, sp, 7

	pinsrw	xmm1, r8w, 0
	pinsrw	xmm2, r9w, 1
	pinsrw	xmm3, r10w, 2
	pinsrw	xmm1, r11w, 3
	pinsrw	xmm2, r12w, 4
	pinsrw	xmm3, ax, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, bx, 7

	pinsrw	xmm0, r8w, 0
	pinsrw	xmm0, r9w, 1
	pinsrw	xmm0, r10w, 2
	pinsrw	xmm0, r11w, 3
	pinsrw	xmm0, r12w, 4
	pinsrw	xmm0, ax, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, bx, 7

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register32ToVector
; Purpose:	Writes 32-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Register32ToVector:
_Register32ToVector:
	mov	eax, 0xcafef00d
.L1:
	pinsrd	xmm1, eax, 0	; Each xfer moves 4 bytes so to move 256 bytes
	pinsrd	xmm2, ebx, 1	; we need 64 transfers.
	pinsrd	xmm3, ecx, 2
	pinsrd	xmm1, edx, 3
	pinsrd	xmm2, esi, 0
	pinsrd	xmm3, edi, 1
	pinsrd	xmm0, ebp, 2
	pinsrd	xmm0, esp, 3

	pinsrd	xmm0, eax, 0
	pinsrd	xmm1, ebx, 1
	pinsrd	xmm2, ecx, 2
	pinsrd	xmm3, edx, 3
	pinsrd	xmm3, esi, 3
	pinsrd	xmm2, edi, 2
	pinsrd	xmm1, ebp, 1
	pinsrd	xmm0, esp, 0

	pinsrd	xmm1, r8d, 0
	pinsrd	xmm2, r9d, 1
	pinsrd	xmm3, r10d, 2
	pinsrd	xmm1, r11d, 3
	pinsrd	xmm2, r12d, 0
	pinsrd	xmm3, eax, 1
	pinsrd	xmm0, ebp, 2
	pinsrd	xmm0, ebx, 3

	pinsrd	xmm0, r8d, 0
	pinsrd	xmm0, r9d, 1
	pinsrd	xmm0, r10d, 2
	pinsrd	xmm0, r11d, 3
	pinsrd	xmm0, r12d, 0
	pinsrd	xmm0, eax, 0
	pinsrd	xmm0, ebp, 0
	pinsrd	xmm0, ebx, 0

	pinsrd	xmm1, eax, 0	
	pinsrd	xmm2, ebx, 1
	pinsrd	xmm3, ecx, 2
	pinsrd	xmm1, edx, 3
	pinsrd	xmm2, esi, 0
	pinsrd	xmm3, edi, 1
	pinsrd	xmm0, ebp, 2
	pinsrd	xmm0, esp, 3

	pinsrd	xmm0, eax, 0
	pinsrd	xmm1, ebx, 1
	pinsrd	xmm2, ecx, 2
	pinsrd	xmm3, edx, 3
	pinsrd	xmm3, esi, 3
	pinsrd	xmm2, edi, 2
	pinsrd	xmm1, ebp, 1
	pinsrd	xmm0, esp, 0

	pinsrd	xmm1, r8d, 0
	pinsrd	xmm2, r9d, 1
	pinsrd	xmm3, r10d, 2
	pinsrd	xmm1, r11d, 3
	pinsrd	xmm2, r12d, 0
	pinsrd	xmm3, eax, 1
	pinsrd	xmm0, ebp, 2
	pinsrd	xmm0, ebx, 3

	pinsrd	xmm0, r8d, 0
	pinsrd	xmm0, r9d, 1
	pinsrd	xmm0, r10d, 2
	pinsrd	xmm0, r11d, 3
	pinsrd	xmm0, r12d, 0
	pinsrd	xmm0, eax, 0
	pinsrd	xmm0, ebp, 0
	pinsrd	xmm0, ebx, 0

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register64ToVector
; Purpose:	Writes 64-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Register64ToVector:
_Register64ToVector:

.L1:
	pinsrq	xmm1, r8, 0	; Each xfer moves 8 bytes, therefore to do
	pinsrq	xmm2, r9, 1	; 256 bytes we need 32 transfers.
	pinsrq	xmm3, r10, 0
	pinsrq	xmm1, r11, 1
	pinsrq	xmm2, r12, 0
	pinsrq	xmm3, rax, 1
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 1

	pinsrq	xmm0, r8, 0
	pinsrq	xmm0, r9, 1
	pinsrq	xmm0, r10, 1
	pinsrq	xmm0, r11, 1
	pinsrq	xmm0, r12, 0
	pinsrq	xmm0, rax, 0
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 0

	pinsrq	xmm0, r8, 0
	pinsrq	xmm0, r9, 1
	pinsrq	xmm0, r10, 1
	pinsrq	xmm0, r11, 1
	pinsrq	xmm0, r12, 0
	pinsrq	xmm0, rax, 0
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 0

	pinsrq	xmm0, r8, 0
	pinsrq	xmm0, r9, 1
	pinsrq	xmm0, r10, 1
	pinsrq	xmm0, r11, 1
	pinsrq	xmm0, r12, 0
	pinsrq	xmm0, rax, 0
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 0

	pinsrq	xmm1, r8, 0	; Each xfer moves 8 bytes, therefore to do
	pinsrq	xmm2, r9, 1	; 256 bytes we need 32 transfers.
	pinsrq	xmm3, r10, 0
	pinsrq	xmm1, r11, 1
	pinsrq	xmm2, r12, 0
	pinsrq	xmm3, rax, 1
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 1

	pinsrq	xmm0, r8, 0
	pinsrq	xmm0, r9, 1
	pinsrq	xmm0, r10, 1
	pinsrq	xmm0, r11, 1
	pinsrq	xmm0, r12, 0
	pinsrq	xmm0, rax, 0
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 0

	pinsrq	xmm0, r8, 0
	pinsrq	xmm0, r9, 1
	pinsrq	xmm0, r10, 1
	pinsrq	xmm0, r11, 1
	pinsrq	xmm0, r12, 0
	pinsrq	xmm0, rax, 0
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 0

	pinsrq	xmm0, r8, 0
	pinsrq	xmm0, r9, 1
	pinsrq	xmm0, r10, 1
	pinsrq	xmm0, r11, 1
	pinsrq	xmm0, r12, 0
	pinsrq	xmm0, rax, 0
	pinsrq	xmm0, rbp, 0
	pinsrq	xmm0, rbx, 0

	dec	P1
	jnz .L1
	ret


;------------------------------------------------------------------------------
; Name:		Vector8ToRegister
; Purpose:	Writes 8-bit vector register values into main register.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Vector8ToRegister:
_Vector8ToRegister:

.L1:
	pextrb	eax, xmm1, 0	; 64 transfers x 1 bytes = 64 bytes 
	pextrb	eax, xmm2, 1
	pextrb	eax, xmm3, 2
	pextrb	eax, xmm1, 3
	pextrb	eax, xmm2, 4
	pextrb	eax, xmm3, 5
	pextrb	eax, xmm0, 6
	pextrb	eax, xmm0, 7

	pextrb	eax, xmm0, 0
	pextrb	eax, xmm1, 1
	pextrb	eax, xmm2, 2
	pextrb	eax, xmm3, 3
	pextrb	eax, xmm3, 4
	pextrb	eax, xmm2, 5
	pextrb	eax, xmm1, 6
	pextrb	eax, xmm0, 7

	pextrb	eax, xmm1, 0
	pextrb	eax, xmm2, 1
	pextrb	eax, xmm3, 2
	pextrb	eax, xmm1, 3
	pextrb	eax, xmm2, 4
	pextrb	eax, xmm3, 5
	pextrb	eax, xmm0, 6
	pextrb	eax, xmm0, 7

	pextrb	eax, xmm0, 0
	pextrb	eax, xmm0, 1
	pextrb	eax, xmm0, 2
	pextrb	eax, xmm0, 3
	pextrb	eax, xmm0, 4
	pextrb	eax, xmm0, 5
	pextrb	eax, xmm0, 6
	pextrb	eax, xmm0, 7

	pextrb	eax, xmm1, 0
	pextrb	eax, xmm2, 1
	pextrb	eax, xmm3, 2
	pextrb	eax, xmm1, 3
	pextrb	eax, xmm2, 4
	pextrb	eax, xmm3, 5
	pextrb	eax, xmm0, 6
	pextrb	eax, xmm0, 7

	pextrb	eax, xmm0, 0
	pextrb	eax, xmm1, 1
	pextrb	eax, xmm2, 2
	pextrb	eax, xmm3, 3
	pextrb	eax, xmm3, 4
	pextrb	eax, xmm2, 5
	pextrb	eax, xmm1, 6
	pextrb	eax, xmm0, 7

	pextrb	eax, xmm1, 0
	pextrb	eax, xmm2, 1
	pextrb	eax, xmm3, 2
	pextrb	eax, xmm1, 3
	pextrb	eax, xmm2, 4
	pextrb	eax, xmm3, 5
	pextrb	eax, xmm0, 6
	pextrb	eax, xmm0, 7

	pextrb	eax, xmm0, 0
	pextrb	eax, xmm0, 1
	pextrb	eax, xmm0, 2
	pextrb	eax, xmm0, 3
	pextrb	eax, xmm0, 4
	pextrb	eax, xmm0, 5
	pextrb	eax, xmm0, 6
	pextrb	eax, xmm0, 7

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Vector16ToRegister
; Purpose:	Writes 16-bit vector register values into main register.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Vector16ToRegister:
_Vector16ToRegister:

.L1:
	pextrw	eax, xmm1, 0	; 64 transfers x 2 bytes = 128 bytes 
	pextrw	eax, xmm2, 1
	pextrw	eax, xmm3, 2
	pextrw	eax, xmm1, 3
	pextrw	eax, xmm2, 4
	pextrw	eax, xmm3, 5
	pextrw	eax, xmm0, 6
	pextrw	eax, xmm0, 7

	pextrw	eax, xmm0, 0
	pextrw	eax, xmm1, 1
	pextrw	eax, xmm2, 2
	pextrw	eax, xmm3, 3
	pextrw	eax, xmm3, 4
	pextrw	eax, xmm2, 5
	pextrw	eax, xmm1, 6
	pextrw	eax, xmm0, 7

	pextrw	eax, xmm1, 0
	pextrw	eax, xmm2, 1
	pextrw	eax, xmm3, 2
	pextrw	eax, xmm1, 3
	pextrw	eax, xmm2, 4
	pextrw	eax, xmm3, 5
	pextrw	eax, xmm0, 6
	pextrw	eax, xmm0, 7

	pextrw	eax, xmm0, 0
	pextrw	eax, xmm0, 1
	pextrw	eax, xmm0, 2
	pextrw	eax, xmm0, 3
	pextrw	eax, xmm0, 4
	pextrw	eax, xmm0, 5
	pextrw	eax, xmm0, 6
	pextrw	eax, xmm0, 7

	pextrw	eax, xmm1, 0	
	pextrw	eax, xmm2, 1
	pextrw	eax, xmm3, 2
	pextrw	eax, xmm1, 3
	pextrw	eax, xmm2, 4
	pextrw	eax, xmm3, 5
	pextrw	eax, xmm0, 6
	pextrw	eax, xmm0, 7

	pextrw	eax, xmm0, 0
	pextrw	eax, xmm1, 1
	pextrw	eax, xmm2, 2
	pextrw	eax, xmm3, 3
	pextrw	eax, xmm3, 4
	pextrw	eax, xmm2, 5
	pextrw	eax, xmm1, 6
	pextrw	eax, xmm0, 7

	pextrw	eax, xmm1, 0
	pextrw	eax, xmm2, 1
	pextrw	eax, xmm3, 2
	pextrw	eax, xmm1, 3
	pextrw	eax, xmm2, 4
	pextrw	eax, xmm3, 5
	pextrw	eax, xmm0, 6
	pextrw	eax, xmm0, 7

	pextrw	eax, xmm0, 0
	pextrw	eax, xmm0, 1
	pextrw	eax, xmm0, 2
	pextrw	eax, xmm0, 3
	pextrw	eax, xmm0, 4
	pextrw	eax, xmm0, 5
	pextrw	eax, xmm0, 6
	pextrw	eax, xmm0, 7

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Vector32ToRegister
; Purpose:	Writes 32-bit vector register values into main register.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Vector32ToRegister:
_Vector32ToRegister:
	
.L1:
	pextrd	eax, xmm1, 0	; 64 xfers x 4 bytes = 256 bytes
	pextrd	eax, xmm2, 1
	pextrd	eax, xmm3, 2
	pextrd	eax, xmm1, 3
	pextrd	eax, xmm2, 0
	pextrd	eax, xmm3, 1
	pextrd	eax, xmm0, 2
	pextrd	eax, xmm0, 3

	pextrd	eax, xmm0, 0
	pextrd	eax, xmm1, 1
	pextrd	eax, xmm2, 2
	pextrd	eax, xmm3, 3
	pextrd	eax, xmm3, 3
	pextrd	eax, xmm2, 2
	pextrd	eax, xmm1, 1
	pextrd	eax, xmm0, 0

	pextrd	eax, xmm1, 0
	pextrd	eax, xmm2, 1
	pextrd	eax, xmm3, 2
	pextrd	eax, xmm1, 3
	pextrd	eax, xmm2, 0
	pextrd	eax, xmm3, 1
	pextrd	eax, xmm0, 2
	pextrd	eax, xmm0, 3

	pextrd	eax, xmm0, 0
	pextrd	eax, xmm0, 1
	pextrd	eax, xmm0, 2
	pextrd	eax, xmm0, 3
	pextrd	eax, xmm0, 0
	pextrd	eax, xmm0, 0
	pextrd	eax, xmm0, 0
	pextrd	eax, xmm0, 0

	pextrd	eax, xmm1, 0
	pextrd	eax, xmm2, 1
	pextrd	eax, xmm3, 2
	pextrd	eax, xmm1, 3
	pextrd	eax, xmm2, 0
	pextrd	eax, xmm3, 1
	pextrd	eax, xmm0, 2
	pextrd	eax, xmm0, 3

	pextrd	eax, xmm0, 0
	pextrd	eax, xmm1, 1
	pextrd	eax, xmm2, 2
	pextrd	eax, xmm3, 3
	pextrd	eax, xmm3, 3
	pextrd	eax, xmm2, 2
	pextrd	eax, xmm1, 1
	pextrd	eax, xmm0, 0

	pextrd	eax, xmm1, 0
	pextrd	eax, xmm2, 1
	pextrd	eax, xmm3, 2
	pextrd	eax, xmm1, 3
	pextrd	eax, xmm2, 0
	pextrd	eax, xmm3, 1
	pextrd	eax, xmm0, 2
	pextrd	eax, xmm0, 3

	pextrd	eax, xmm0, 0
	pextrd	eax, xmm0, 1
	pextrd	eax, xmm0, 2
	pextrd	eax, xmm0, 3
	pextrd	eax, xmm0, 0
	pextrd	eax, xmm0, 1
	pextrd	eax, xmm0, 2
	pextrd	eax, xmm0, 3

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Vector64ToRegister
; Purpose:	Writes 64-bit vector register values into main register.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
Vector64ToRegister:
_Vector64ToRegister:
	
.L1:
	pextrq	rax, xmm1, 0	; 64 transfers 
	pextrq	rax, xmm2, 1
	pextrq	rax, xmm3, 0
	pextrq	rax, xmm1, 1
	pextrq	rax, xmm2, 0
	pextrq	rax, xmm3, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	pextrq	rax, xmm1, 0
	pextrq	rax, xmm2, 1
	pextrq	rax, xmm3, 0
	pextrq	rax, xmm1, 1
	pextrq	rax, xmm2, 0
	pextrq	rax, xmm3, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	pextrq	rax, xmm1, 0	; 64 transfers 
	pextrq	rax, xmm2, 1
	pextrq	rax, xmm3, 0
	pextrq	rax, xmm1, 1
	pextrq	rax, xmm2, 0
	pextrq	rax, xmm3, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	pextrq	rax, xmm1, 0
	pextrq	rax, xmm2, 1
	pextrq	rax, xmm3, 0
	pextrq	rax, xmm1, 1
	pextrq	rax, xmm2, 0
	pextrq	rax, xmm3, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1
	pextrq	rax, xmm0, 0
	pextrq	rax, xmm0, 1

	dec	P1
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		CopyWithMainRegisters
; Purpose:	Copies memory chunks that are 32-byte aligned.
; Params:	P1 = ptr to destination memory area
;		P2 = ptr to source memory area
; 		P3 = length in bytes
; 		P4 = loops
;------------------------------------------------------------------------------
	align 64
CopyWithMainRegisters:
_CopyWithMainRegisters:

	push	r10
	push	r11
	push	r12
	push	r13
	push	r14

	shr	P3, 8	; Ensure length is multiple of 256.
	shl	P3, 8

	prefetcht0	[P2]

.L1:
	mov	r10, P3

.L2:
	mov	r11, [P2]
	mov	r12, [8+P2]
	mov	r13, [16+P2]
	mov	r14, [24+P2]
	mov	[P1], r11
	mov	[8+P1], r12
	mov	[16+P1], r13
	mov	[24+P1], r14

	mov	r11, [32+P2]
	mov	r12, [40+P2]
	mov	r13, [48+P2]
	mov	r14, [56+P2]
	mov	[32+P1], r11
	mov	[40+P1], r12
	mov	[48+P1], r13
	mov	[56+P1], r14

	mov	r11, [64+P2]
	mov	r12, [72+P2]
	mov	r13, [80+P2]
	mov	r14, [88+P2]
	mov	[64+P1], r11
	mov	[72+P1], r12
	mov	[80+P1], r13
	mov	[88+P1], r14

	mov	r11, [96+P2]
	mov	r12, [104+P2]
	mov	r13, [112+P2]
	mov	r14, [120+P2]
	mov	[96+P1], r11
	mov	[104+P1], r12
	mov	[112+P1], r13
	mov	[120+P1], r14

	mov	r11, [128+P2]
	mov	r12, [128+8+P2]
	mov	r13, [128+16+P2]
	mov	r14, [128+24+P2]
	mov	[128+P1], r11
	mov	[128+8+P1], r12
	mov	[128+16+P1], r13
	mov	[128+24+P1], r14

	mov	r11, [128+32+P2]
	mov	r12, [128+40+P2]
	mov	r13, [128+48+P2]
	mov	r14, [128+56+P2]
	mov	[128+32+P1], r11
	mov	[128+40+P1], r12
	mov	[128+48+P1], r13
	mov	[128+56+P1], r14

	mov	r11, [128+64+P2]
	mov	r12, [128+72+P2]
	mov	r13, [128+80+P2]
	mov	r14, [128+88+P2]
	mov	[128+64+P1], r11
	mov	[128+72+P1], r12
	mov	[128+80+P1], r13
	mov	[128+88+P1], r14

	mov	r11, [128+96+P2]
	mov	r12, [128+104+P2]
	mov	r13, [128+112+P2]
	mov	r14, [128+120+P2]
	mov	[128+96+P1], r11
	mov	[128+104+P1], r12
	mov	[128+112+P1], r13
	mov	[128+120+P1], r14

	add	P2, 256
	add	P1, 256

	sub	r10, 256
	jnz	.L2

	sub	P2, P3	; P2 now points to start.
	sub	P1, P3	; P1 now points to start.

	dec	P4
	jnz	.L1

	pop	r14
	pop	r13
	pop	r12
	pop	r11
	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		CopyAVX512
; Purpose:	Copies memory chunks that are 64-byte aligned.
; Params:	P1 = ptr to destination memory area
;		P2 = ptr to source memory area
; 		P3 = length in bytes
; 		P4 = loops
;------------------------------------------------------------------------------
	align 64
CopyAVX512:
_CopyAVX512:

	push	r10

	shr	P3, 8	; Ensure length is multiple of 256.
	shl	P3, 8

	prefetcht0	[P2]

.L1:
	mov	r10, P3

.L2:
	vmovdqa64	zmm0, [P2]
	vmovdqa64	zmm1, [64+P2]
	vmovdqa64	zmm2, [128+P2]
	vmovdqa64	zmm3, [192+P2]

	vmovdqa64	[P1], zmm0
	vmovdqa64	[64+P1], zmm1
	vmovdqa64	[128+P1], zmm2
	vmovdqa64	[192+P1], zmm3

	add	P2, 256
	add	P1, 256

	sub	r10, 256
	jnz	.L2

	sub	P2, P3	; P2 now points to start.
	sub	P1, P3	; P1 now points to start.

	dec	P4
	jnz	.L1

	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		CopyAVX
; Purpose:	Copies memory chunks that are 32-byte aligned.
; Params:	P1 = ptr to destination memory area
;		P2 = ptr to source memory area
; 		P3 = length in bytes
; 		P4 = loops
;------------------------------------------------------------------------------
	align 64
CopyAVX:
_CopyAVX:
	vzeroupper

	push	r10

	shr	P3, 8	; Ensure length is multiple of 256.
	shl	P3, 8

	prefetcht0	[P2]

.L1:
	mov	r10, P3

.L2:
	vmovdqa	ymm0, [P2]
	vmovdqa	ymm1, [32+P2]
	vmovdqa	ymm2, [64+P2]
	vmovdqa	ymm3, [96+P2]

	vmovdqa	[P1], ymm0
	vmovdqa	[32+P1], ymm1
	vmovdqa	[64+P1], ymm2
	vmovdqa	[96+P1], ymm3

	vmovdqa	ymm0, [128+P2]
	vmovdqa	ymm1, [128+32+P2]
	vmovdqa	ymm2, [128+64+P2]
	vmovdqa	ymm3, [128+96+P2]

	vmovdqa	[128+P1], ymm0
	vmovdqa	[128+32+P1], ymm1
	vmovdqa	[128+64+P1], ymm2
	vmovdqa	[128+96+P1], ymm3

	add	P2, 256
	add	P1, 256

	sub	r10, 256
	jnz	.L2

	sub	P2, P3	; P2 now points to start.
	sub	P1, P3	; P1 now points to start.

	dec	P4
	jnz	.L1

	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		CopySSE
; Purpose:	Copies memory chunks that are 16-byte aligned.
; Params:	P1 = ptr to destination memory area
;		P2 = ptr to source memory area
; 		P3 = length in bytes
; 		P4 = loops
;------------------------------------------------------------------------------
	align 64
CopySSE:
_CopySSE:
	push	r10

	shr	P3, 8	; Ensure length is multiple of 256.
	shl	P3, 8

	prefetcht0	[P2]

	; Save our non-parameter XMM registers.
	sub	rsp, 192
	movdqu	[rsp], xmm4
	movdqu	[16+rsp], xmm5
	movdqu	[32+rsp], xmm6
	movdqu	[48+rsp], xmm7
	movdqu	[64+rsp], xmm8
	movdqu	[80+rsp], xmm9
	movdqu	[96+rsp], xmm10
	movdqu	[112+rsp], xmm11
	movdqu	[128+rsp], xmm12
	movdqu	[144+rsp], xmm13
	movdqu	[160+rsp], xmm14
	movdqu	[176+rsp], xmm15

.L1:
	mov	r10, P3

.L2:
	movdqa	xmm0, [P2]
	movdqa	xmm1, [16+P2]
	movdqa	xmm2, [32+P2]
	movdqa	xmm3, [48+P2]

	movdqa	[P1], xmm0
	movdqa	[16+P1], xmm1
	movdqa	[32+P1], xmm2
	movdqa	[48+P1], xmm3

	movdqa	xmm4, [64+P2]
	movdqa	xmm5, [80+P2]
	movdqa	xmm6, [96+P2]
	movdqa	xmm7, [112+P2]

	movdqa	[64+P1], xmm4
	movdqa	[80+P1], xmm5
	movdqa	[96+P1], xmm6
	movdqa	[112+P1], xmm7

	movdqa	xmm8, [128+P2]
	movdqa	xmm9, [144+P2]
	movdqa	xmm10, [160+P2]
	movdqa	xmm11, [176+P2]

	movdqa	[128+P1], xmm8
	movdqa	[144+P1], xmm9
	movdqa	[160+P1], xmm10
	movdqa	[176+P1], xmm11

	movdqa	xmm12, [192+P2]
	movdqa	xmm13, [208+P2]
	movdqa	xmm14, [224+P2]
	movdqa	xmm15, [240+P2]

	movdqa	[192+P1], xmm12
	movdqa	[208+P1], xmm13
	movdqa	[224+P1], xmm14
	movdqa	[240+P1], xmm15

	add	P2, 256
	add	P1, 256

	sub	r10, 256
	jnz	.L2

	sub	P2, P3	; P2 now points to start.
	sub	P1, P3	; P1 now points to start.

	dec	P4
	jnz	.L1

	movdqu	xmm4, [rsp]
	movdqu	xmm5, [16+rsp]
	movdqu	xmm6, [32+rsp]
	movdqu	xmm7, [48+rsp]
	movdqu	xmm8, [64+rsp]
	movdqu	xmm9, [80+rsp]
	movdqu	xmm10, [96+rsp]
	movdqu	xmm11, [112+rsp]
	movdqu	xmm12, [128+rsp]
	movdqu	xmm13, [144+rsp]
	movdqu	xmm14, [160+rsp]
	movdqu	xmm15, [176+rsp]
	add	rsp, 192

	pop	r10

	mfence
	ret

;------------------------------------------------------------------------------
; Name:		IncrementRegisters
; Purpose:	Increments 64-bit values in registers.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
IncrementRegisters:
_IncrementRegisters:
.L1:
	inc	rax
	inc	rbx
	inc	P4
	inc	P3
	inc	P2
	inc	P1
	inc	rbp
	inc	rsp
	inc	r8
	inc	r9
	inc	r10
	inc	r11
	inc	r12
	inc	r13
	inc	r14
	inc	r15

	dec	rax
	dec	rbx
	dec	P4
	dec	P3
	dec	P2
	dec	P1
	dec	rbp
	dec	rsp
	dec	r8
	dec	r9
	dec	r10
	dec	r11
	dec	r12
	dec	r13
	dec	r14
	dec	r15

	dec	P1
	jnz	.L1
	ret


;------------------------------------------------------------------------------
; Name:		IncrementStack
; Purpose:	Increments 64-bit values on stack.
; Params:	P1 = loops
;------------------------------------------------------------------------------
	align 64
IncrementStack:
_IncrementStack:
	sub	rsp, 128
.L1:
	inc	qword [rsp]
	inc	qword [rsp+8]
	inc	qword [rsp+16]
	inc	qword [rsp+24]
	inc	qword [rsp+32]
	inc	qword [rsp+40]
	inc	qword [rsp+48]
	inc	qword [rsp+56]
	inc	qword [rsp+64]
	inc	qword [rsp+72]
	inc	qword [rsp+80]
	inc	qword [rsp+88]
	inc	qword [rsp+96]
	inc	qword [rsp+104]
	inc	qword [rsp+112]
	inc	qword [rsp+120]

	dec	qword [rsp]
	dec	qword [rsp+8]
	dec	qword [rsp+16]
	dec	qword [rsp+24]
	dec	qword [rsp+32]
	dec	qword [rsp+40]
	dec	qword [rsp+48]
	dec	qword [rsp+56]
	dec	qword [rsp+64]
	dec	qword [rsp+72]
	dec	qword [rsp+80]
	dec	qword [rsp+88]
	dec	qword [rsp+96]
	dec	qword [rsp+104]
	dec	qword [rsp+112]
	dec	qword [rsp+120]

	dec	P1
	jnz	.L1

	add	rsp, 128

	mfence
	ret

Reader_nontemporal:
_Reader_nontemporal:
	ret

