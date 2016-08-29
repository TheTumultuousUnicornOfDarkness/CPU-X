;============================================================================
;  bandwidth, a benchmark to estimate memory transfer bandwidth.
;  Copyright (C) 2005-2014 by Zack T Smith.
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
;  The author may be reached at veritas@comcast.net.
;=============================================================================

bits	64
cpu	ia64

global	CopySSE
global	CopySSE_128bytes

global CopyAVX
global _CopyAVX

global	ReaderLODSQ
global	_ReaderLODSQ

global	ReaderLODSD
global	_ReaderLODSD

global	ReaderLODSW
global	_ReaderLODSW

global	ReaderLODSB
global	_ReaderLODSB

global	RandomReader
global	RandomReaderSSE2
global	RandomReaderSSE2_bypass
global	RandomWriter
global	RandomWriterSSE2
global	RandomWriterSSE2_bypass
global	Reader
global	Reader_128bytes
global	ReaderAVX
global	ReaderSSE2
global	ReaderSSE2_128bytes
global	ReaderSSE2_bypass
global	ReaderSSE2_128bytes_bypass
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
global	VectorToVector
global	VectorToVectorAVX
global	Writer
global	Writer_128bytes
global	WriterAVX
global	WriterSSE2
global	WriterSSE2_128bytes
global	WriterSSE2_bypass
global	WriterSSE2_128bytes_bypass
global	WriterAVX_bypass
global	_WriterAVX_bypass
global	_CopySSE
global	_CopySSE_128bytes
global	_RandomReader
global	_RandomReaderSSE2
global	_RandomReaderSSE2_bypass
global	_RandomWriter
global	_RandomWriterSSE2
global	_RandomWriterSSE2_bypass
global	_Reader
global	_ReaderAVX
global	_Reader_128bytes
global	_ReaderSSE2
global	_ReaderSSE2_bypass
global	_ReaderSSE2_128bytes
global	_ReaderSSE2_128bytes_bypass
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
global	_Writer
global	_Writer_128bytes
global	_WriterSSE2
global	_WriterAVX
global	_WriterSSE2_128bytes
global	_WriterSSE2_bypass
global	_WriterSSE2_128bytes_bypass

global  get_cpuid_cache_info
global  _get_cpuid_cache_info

global	get_cpuid_family
global	_get_cpuid_family

global	get_cpuid1_ecx
global	_get_cpuid1_ecx

global	get_cpuid1_edx
global	_get_cpuid1_edx

global	get_cpuid7_ebx
global	_get_cpuid7_ebx

global	get_cpuid_80000001_ecx
global	_get_cpuid_80000001_ecx

global	get_cpuid_80000001_edx
global	_get_cpuid_80000001_edx

; Note:
; Unix ABI says integer param are put in these registers in this order:
;	rdi, rsi, rdx, rcx, r8, r9

	section .text

;------------------------------------------------------------------------------
; Name:         get_cpuid_cache_info
; 
get_cpuid_cache_info:
_get_cpuid_cache_info:
        push    rbx
        push    rcx
        push    rdx
        mov     rax, 4
	mov	rcx, rsi
        cpuid
	mov	[rdi], eax
	mov	[rdi+4], ebx
	mov	[rdi+8], ecx
	mov	[rdi+12], edx
        pop     rdx
        pop     rcx
        pop     rbx
        ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_family
; 
get_cpuid_family:
_get_cpuid_family:
	push	rbx
	push 	rcx
	push 	rdx
	xor	rax, rax
	cpuid
	mov	[rdi], ebx
	mov	[rdi+4], edx
	mov	[rdi+8], ecx
	mov	byte [rdi+12], 0
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid1_ecx
; 
get_cpuid1_ecx:
_get_cpuid1_ecx:
	push	rbx
	push 	rcx
	push 	rdx
	mov	rax, 1
	cpuid
        mov	rax, rcx
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid7_ebx
; 
get_cpuid7_ebx:
_get_cpuid7_ebx:
	push	rbx
	push 	rcx
	push 	rdx
	mov	rax, 7
	xor	rcx, rcx
	cpuid
        mov	rax, rbx
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid1_edx
; 
get_cpuid1_edx:
_get_cpuid1_edx:
	push	rbx
	push 	rcx
	push 	rdx
	mov	rax, 1
	cpuid
        mov	rax, rdx
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_80000001_ecx
; 
get_cpuid_80000001_ecx:
_get_cpuid_80000001_ecx:
	push	rbx
	push 	rcx
	push 	rdx
	mov	rax, 0x80000001
	cpuid
        mov	rax, rcx
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_80000001_edx
; 
get_cpuid_80000001_edx:
_get_cpuid_80000001_edx:
	push	rbx
	push 	rcx
	push 	rdx
	mov	rax, 0x80000001
	cpuid
        mov	rax, rdx
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		ReaderLODSQ
; Purpose:	Reads 64-bit values sequentially from an area of memory
;		using LODSQ instruction.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 32
ReaderLODSQ:
_ReaderLODSQ:
	push	rcx	; REP counter
	push	r10
	push	r11
	mov	r10, rdi
	mov	r11, rsi
	shr	r11, 3  	; length in quadwords rounded down.

.L1:
	mov	rsi, r10	; buffer start
	mov	rcx, r11	; # of quadwords

	rep	lodsq

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	pop	rcx
	ret 

;------------------------------------------------------------------------------
; Name:		ReaderLODSD
; Purpose:	Reads 32-bit values sequentially from an area of memory
;		using LODSD instruction.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 32
ReaderLODSD:
_ReaderLODSD:
	push	rcx	; REP counter
	push	r10
	push	r11
	mov	r10, rdi
	mov	r11, rsi
	shr	r11, 2  	; length in double words rounded down.

.L1:
	mov	rsi, r10	; buffer start
	mov	rcx, r11	; # of double words

	rep	lodsd

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	pop	rcx
	ret 

;------------------------------------------------------------------------------
; Name:		ReaderLODSW
; Purpose:	Reads 16-bit values sequentially from an area of memory
;		using LODSW instruction.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 32
ReaderLODSW:
_ReaderLODSW:
	push	rcx	; REP counter
	push	r10
	push	r11
	mov	r10, rdi
	mov	r11, rsi
	shr	r11, 1  	; length in words rounded down.

.L1:
	mov	rsi, r10	; buffer start
	mov	rcx, r11	; # of words

	rep	lodsw

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	pop	rcx
	ret 

;------------------------------------------------------------------------------
; Name:		ReaderLODSB
; Purpose:	Reads 8-bit values sequentially from an area of memory
;		using LODSB instruction.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 32
ReaderLODSB:
_ReaderLODSB:
	push	rcx	; REP counter
	push	r10
	push	r11
	mov	r10, rdi
	mov	r11, rsi

.L1:
	mov	rsi, r10	; buffer start
	mov	rcx, r11	; # of bytes

	rep	lodsb

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	pop	rcx
	ret 

;------------------------------------------------------------------------------
; Name:		Reader
; Purpose:	Reads 64-bit values sequentially from an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
Reader:
_Reader:
	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

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
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		Reader_128bytes
; Purpose:	Reads 64-bit values sequentially from an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
Reader_128bytes:
_Reader_128bytes:
	push	r10

	add	rsi, rdi	; rdi now points to end.

.L1:
	mov	r10, rdi

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

	add	r10, 128
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		RandomReader
; Purpose:	Reads 64-bit values randomly from an area of memory.
; Params:	rdi = ptr to array of chunk pointers
; 		rsi = # of chunks
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
RandomReader:
_RandomReader:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [rdi + 8*r11]	; Note, 64-bit pointers.

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
	cmp	r11, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		RandomReaderSSE2
; Purpose:	Reads 128-bit values randomly from an area of memory.
; Params:	rdi = ptr to array of chunk pointers
; 		rsi = # of chunks
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
RandomReaderSSE2:
_RandomReaderSSE2:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [rdi + 8*r11]

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
	cmp	r11, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		RandomReaderSSE2_bypass
; Purpose:	Reads 128-bit values randomly from an area of memory.
; Params:	rdi = ptr to array of chunk pointers
; 		rsi = # of chunks
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
RandomReaderSSE2_bypass:
_RandomReaderSSE2_bypass:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [rdi + 8*r11]

	; SSE 4.1 required
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
	cmp	r11, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriter
; Purpose:	Writes 64-bit values randomly to an area of memory.
; Params:	rdi = ptr to array of chunk pointers
; 		rsi = # of chunks
; 		rdx = loops
; 		rcx = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriter:
_RandomWriter:
	push	r10
	push	r11

.L1:
	xor	r11, r11

.L2:
	mov	r10, [rdi + 8*r11]	; Note, 64-bit pointers.

	mov	[96+r10], rcx
	mov	[r10], rcx
	mov	[120+r10], rcx
	mov	[184+r10], rcx
	mov	[160+r10], rcx
	mov	[176+r10], rcx
	mov	[112+r10], rcx
	mov	[80+r10], rcx
	mov	[32+r10], rcx
	mov	[128+r10], rcx
	mov	[88+r10], rcx
	mov	[40+r10], rcx
	mov	[48+r10], rcx
	mov	[72+r10], rcx
	mov	[200+r10], rcx
	mov	[24+r10], rcx
	mov	[152+r10], rcx
	mov	[16+r10], rcx
	mov	[248+r10], rcx
	mov	[56+r10], rcx
	mov	[240+r10], rcx
	mov	[208+r10], rcx
	mov	[104+r10], rcx
	mov	[216+r10], rcx
	mov	[136+r10], rcx
	mov	[232+r10], rcx
	mov	[64+r10], rcx
	mov	[224+r10], rcx
	mov	[144+r10], rcx
	mov	[192+r10], rcx
	mov	[8+r10], rcx
	mov	[168+r10], rcx

	inc	r11
	cmp	r11, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriterSSE2
; Purpose:	Writes 128-bit values randomly to an area of memory.
; Params:	rdi = ptr to array of chunk pointers
; 		rsi = # of chunks
; 		rdx = loops
; 		rcx = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriterSSE2:
_RandomWriterSSE2:
	push	r10
	push	r11

	movq	xmm0, rcx	; Create duplicated 128-bit datum
	movq	xmm1, rcx
	pslldq	xmm1, 64
	por	xmm0, xmm1

.L1:
	xor	r11, r11

.L2:
	mov	r10, [rdi + 8*r11]	; Note, 64-bit pointers.

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
	cmp	r11, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriterSSE2_bypass
; Purpose:	Writes 128-bit values randomly into memory, bypassing caches.
; Params:	rdi = ptr to array of chunk pointers
; 		rsi = # of chunks
; 		rdx = loops
; 		rcx = datum to write
;------------------------------------------------------------------------------
	align 64
RandomWriterSSE2_bypass:
_RandomWriterSSE2_bypass:
	push	r10
	push	r11

	movq	xmm0, rcx	; Create duplicated 128-bit datum
	movq	xmm1, rcx
	pslldq	xmm1, 64
	por	xmm0, xmm1

.L1:
	xor	r11, r11

.L2:
	mov	r10, [rdi + 8*r11]	; Note, 64-bit pointers.

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
	cmp	r11, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r11
	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		ReaderSSE2_128bytes
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2_128bytes:
_ReaderSSE2_128bytes:
	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

.L2:
	movdqa	xmm0, [r10]	; Read aligned to 16-byte boundary.
	movdqa	xmm0, [16+r10]
	movdqa	xmm0, [32+r10]
	movdqa	xmm0, [48+r10]
	movdqa	xmm0, [64+r10]
	movdqa	xmm0, [80+r10]
	movdqa	xmm0, [96+r10]
	movdqa	xmm0, [112+r10]

	add	r10, 128
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1
	
	pop	r10
	ret


;------------------------------------------------------------------------------
; Name:		ReaderSSE2
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2:
_ReaderSSE2:
	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

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
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1
	
	pop	r10
	ret


;------------------------------------------------------------------------------
; Name:		ReaderAVX
; Purpose:	Reads 256-bit values sequentially from an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
ReaderAVX:
_ReaderAVX:
	vzeroupper

	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

.L2:
	vmovdqa	ymm0, [r10]	; Read aligned to 32-byte boundary.
	vmovdqa	ymm0, [32+r10]
	vmovdqa	ymm0, [64+r10]
	vmovdqa	ymm0, [96+r10]
	vmovdqa	ymm0, [128+r10]
	vmovdqa	ymm0, [160+r10]
	vmovdqa	ymm0, [192+r10]
	vmovdqa	ymm0, [224+r10]

	add	r10, 256
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1
	
	pop	r10
	ret


;------------------------------------------------------------------------------
; Name:		ReaderSSE2_bypass
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2_bypass:
_ReaderSSE2_bypass:
	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

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
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1
	
	pop	r10
	ret


;------------------------------------------------------------------------------
; Name:		ReaderSSE2_128bytes_bypass
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2_128bytes_bypass:
_ReaderSSE2_128bytes_bypass:
	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

.L2:
	prefetchnta	[r10]
	movntdqa	xmm0, [r10]	; Read aligned to 16-byte boundary.
	movntdqa	xmm0, [16+r10]
	movntdqa	xmm0, [32+r10]
	movntdqa	xmm0, [48+r10]
	prefetchnta	[r10+64]
	movntdqa	xmm0, [64+r10]
	movntdqa	xmm0, [80+r10]
	movntdqa	xmm0, [96+r10]
	movntdqa	xmm0, [112+r10]

	add	r10, 128
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1
	
	pop	r10
	ret


;------------------------------------------------------------------------------
; Name:		Writer
; Purpose:	Writes 64-bit value sequentially to an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
Writer:
_Writer:
	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

.L2:
	mov	[r10], rcx
	mov	[8+r10], rcx
	mov	[16+r10], rcx
	mov	[24+r10], rcx
	mov	[32+r10], rcx
	mov	[40+r10], rcx
	mov	[48+r10], rcx
	mov	[56+r10], rcx
	mov	[64+r10], rcx
	mov	[72+r10], rcx
	mov	[80+r10], rcx
	mov	[88+r10], rcx
	mov	[96+r10], rcx
	mov	[104+r10], rcx
	mov	[112+r10], rcx
	mov	[120+r10], rcx
	mov	[128+r10], rcx
	mov	[136+r10], rcx
	mov	[144+r10], rcx
	mov	[152+r10], rcx
	mov	[160+r10], rcx
	mov	[168+r10], rcx
	mov	[176+r10], rcx
	mov	[184+r10], rcx
	mov	[192+r10], rcx
	mov	[200+r10], rcx
	mov	[208+r10], rcx
	mov	[216+r10], rcx
	mov	[224+r10], rcx
	mov	[232+r10], rcx
	mov	[240+r10], rcx
	mov	[248+r10], rcx

	add	r10, 256
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		Writer_128bytes
; Purpose:	Writes 64-bit value sequentially to an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
Writer_128bytes:
_Writer_128bytes:
	push	r10

	add	rsi, rdi	; rsi now points to end.

.L1:
	mov	r10, rdi

.L2:
	mov	[r10], rcx
	mov	[8+r10], rcx
	mov	[16+r10], rcx
	mov	[24+r10], rcx
	mov	[32+r10], rcx
	mov	[40+r10], rcx
	mov	[48+r10], rcx
	mov	[56+r10], rcx
	mov	[64+r10], rcx
	mov	[72+r10], rcx
	mov	[80+r10], rcx
	mov	[88+r10], rcx
	mov	[96+r10], rcx
	mov	[104+r10], rcx
	mov	[112+r10], rcx
	mov	[120+r10], rcx

	add	r10, 128
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2
; Purpose:	Writes 128-bit value sequentially to an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
WriterSSE2:
_WriterSSE2:
	push	r10

	add	rsi, rdi	; rsi now points to end.

	movq	xmm0, rcx

.L1:
	mov	r10, rdi

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
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX
; Purpose:	Writes 256-bit value sequentially to an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
WriterAVX:
_WriterAVX:
	vzeroupper

	push	r10

	add	rsi, rdi	; rsi now points to end.

	pinsrq	xmm0, rcx, 0
	pinsrq	xmm0, rcx, 1

.L1:
	mov	r10, rdi

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
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2_128bytes
; Purpose:	Writes 128-bit value sequentially to an area of memory,
;		chunks are 128 bytes rather than 256.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
WriterSSE2_128bytes:
_WriterSSE2_128bytes:
	push	r10

	add	rsi, rdi	; rsi now points to end.

	movq	xmm0, rcx

.L1:
	mov	r10, rdi

.L2:
	movdqa	[r10], xmm0
	movdqa	[16+r10], xmm0
	movdqa	[32+r10], xmm0
	movdqa	[48+r10], xmm0
	movdqa	[64+r10], xmm0
	movdqa	[80+r10], xmm0
	movdqa	[96+r10], xmm0
	movdqa	[112+r10], xmm0

	add	r10, 128
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2_bypass
; Purpose:	Writes 128-bit value sequentially to an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
WriterSSE2_bypass:
_WriterSSE2_bypass:
	push	r10

	add	rsi, rdi	; rsi now points to end.

	movq	xmm0, rcx

.L1:
	mov	r10, rdi

.L2:
	movntdq	[r10], xmm0	; Write bypassing cache.
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
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX_bypass
; Purpose:	Writes 256-bit value sequentially to an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
WriterAVX_bypass:
_WriterAVX_bypass:
	vzeroupper

	push	r10

	add	rsi, rdi	; rsi now points to end.

	movq	xmm0, rcx

.L1:
	mov	r10, rdi

.L2:
	vmovntdq	[r10], xmm0	; Write bypassing cache.
	vmovntdq	[32+r10], xmm0
	vmovntdq	[64+r10], xmm0
	vmovntdq	[96+r10], xmm0
	vmovntdq	[128+r10], xmm0
	vmovntdq	[160+r10], xmm0
	vmovntdq	[192+r10], xmm0
	vmovntdq	[224+r10], xmm0

	add	r10, 256
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2_128bytes_bypass
; Purpose:	Writes 128-bit value sequentially to an area of memory.
; Params:	rdi = ptr to memory area
; 		rsi = length in bytes
; 		rdx = loops
; 		rcx = quad to write
;------------------------------------------------------------------------------
	align 64
WriterSSE2_128bytes_bypass:
_WriterSSE2_128bytes_bypass:
	push	r10

	add	rsi, rdi	; rsi now points to end.

	movq	xmm0, rcx

.L1:
	mov	r10, rdi

.L2:
	movntdq	[r10], xmm0	; Write bypassing cache.
	movntdq	[16+r10], xmm0
	movntdq	[32+r10], xmm0
	movntdq	[48+r10], xmm0
	movntdq	[64+r10], xmm0
	movntdq	[80+r10], xmm0
	movntdq	[96+r10], xmm0
	movntdq	[112+r10], xmm0

	add	r10, 128
	cmp	r10, rsi
	jb	.L2

	dec	rdx
	jnz	.L1

	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		StackReader
; Purpose:	Reads 64-bit values off the stack into registers of
;		the main register set, effectively testing L1 cache access
;		*and* effective-address calculation speed.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
StackReader:
_StackReader:
	push	qword 7000	; [rsp+48]
	push	qword 6000	; [rsp+40]
	push	qword 5000	; [rsp+32]
	push	qword 4000	; [rsp+24]
	push	qword 3000	; [rsp+16]
	push	qword 2000	; [rsp+8]
	push	qword 1000	; [rsp]

.L1:
	mov	rax, [rsp]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp]
	mov	rax, [rsp]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp]
	mov	rax, [rsp]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+8]
	mov	rax, [rsp+8]
	mov	rax, [rsp+16]
	mov	rax, [rsp+24]
	mov	rax, [rsp+32]
	mov	rax, [rsp+40]
	mov	rax, [rsp+8]
	mov	rax, [rsp+48]
	mov	rax, [rsp+8]

	sub	rdi, 1
	jnz	.L1

	add	rsp, 56
	ret

;------------------------------------------------------------------------------
; Name:		StackWriter
; Purpose:	Writes 64-bit values into the stack from registers of
;		the main register set, effectively testing L1 cache access
;		*and* effective-address calculation speed.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
StackWriter:
_StackWriter:
	push	qword 7000	; [rsp+48]
	push	qword 6000	; [rsp+40]
	push	qword 5000	; [rsp+32]
	push	qword 4000	; [rsp+24]
	push	qword 3000	; [rsp+16]
	push	qword 2000	; [rsp+8]
	push	qword 1000	; [rsp]

	xor	rax, rax

.L1:
	mov	[rsp], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp], rax
	mov	[rsp], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp], rax
	mov	[rsp], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+8], rax
	mov	[rsp+8], rax
	mov	[rsp+16], rax
	mov	[rsp+24], rax
	mov	[rsp+32], rax
	mov	[rsp+40], rax
	mov	[rsp+8], rax
	mov	[rsp+48], rax
	mov	[rsp+8], rax

	sub	rdi, 1
	jnz	.L1

	add	rsp, 56
	ret

;------------------------------------------------------------------------------
; Name:		RegisterToRegister
; Purpose:	Reads/writes 64-bit values between registers of 
;		the main register set.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
RegisterToRegister:
_RegisterToRegister:
.L1:
	mov	rax, rbx
	mov	rax, rcx
	mov	rax, rdx
	mov	rax, rsi
	mov	rax, rdi
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, rcx
	mov	rax, rdx
	mov	rax, rsi
	mov	rax, rdi
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, rcx
	mov	rax, rdx
	mov	rax, rsi
	mov	rax, rdi
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx
	mov	rax, rbx
	mov	rax, rcx
	mov	rax, rdx
	mov	rax, rsi
	mov	rax, rdi
	mov	rax, rbp
	mov	rax, rsp
	mov	rax, rbx

	sub	rdi, 1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToVector
; Purpose:	Reads/writes 128-bit values between registers of 
;		the vector register set, in this case XMM.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
VectorToVector:
_VectorToVector:
.L1:
	movq	xmm0, xmm1	; Each move moves 16 bytes, so we need 16
	movq	xmm0, xmm2	; moves to transfer a 256 byte chunk.
	movq	xmm0, xmm3
	movq	xmm2, xmm0
	movq	xmm1, xmm2
	movq	xmm2, xmm1
	movq	xmm0, xmm3
	movq	xmm3, xmm1

	movq	xmm3, xmm2
	movq	xmm1, xmm3
	movq	xmm2, xmm1
	movq	xmm0, xmm1
	movq	xmm1, xmm2
	movq	xmm0, xmm1
	movq	xmm0, xmm3
	movq	xmm3, xmm0

	sub	rdi, 1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToVectorAVX
; Purpose:	Reads/writes 256-bit values between registers of 
;		the vector register set, in this case YMM.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
VectorToVectorAVX:
_VectorToVectorAVX:
	vzeroupper

.L1:
	vmovdqa	ymm0, ymm1	; Each move moves 32 bytes, so we need 8
	vmovdqa	ymm0, ymm2	; moves to transfer a 256 byte chunk.
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm2, ymm0
	vmovdqa	ymm1, ymm2
	vmovdqa	ymm2, ymm1
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm3, ymm1

	sub	rdi, 1
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		RegisterToVector
; Purpose:	Writes 64-bit main register values into 128-bit vector register
;		clearing the upper unused bits.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
RegisterToVector:
_RegisterToVector:
.L1:
	movq	xmm1, rax 	; Each movq transfers 8 bytes, so we need
	movq	xmm2, rsi	; 32 transfers to move a 256-byte chunk.
	movq	xmm3, rbx
	movq	xmm1, rcx
	movq	xmm2, rsi
	movq	xmm3, rsp
	movq	xmm0, rdi
	movq	xmm0, rdx

	movq	xmm0, rax 	
	movq	xmm1, rsi
	movq	xmm2, rbx
	movq	xmm3, rcx
	movq	xmm0, rsi
	movq	xmm3, rsp
	movq	xmm2, rdi
	movq	xmm1, rdx

	movq	xmm0, rax 	
	movq	xmm1, rsi
	movq	xmm2, rbx
	movq	xmm3, rcx
	movq	xmm0, rsi
	movq	xmm3, rsp
	movq	xmm2, rdi
	movq	xmm1, rdx

	movq	xmm0, rax 	
	movq	xmm1, rsi
	movq	xmm2, rbx
	movq	xmm3, rcx
	movq	xmm0, rsi
	movq	xmm3, rsp
	movq	xmm2, rdi
	movq	xmm1, rdx

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToRegister
; Purpose:	Writes lower 64 bits of vector register into 64-bit main 
;		register.
; Params:	rdi = loops
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register8ToVector
; Purpose:	Writes 8-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Register8ToVector:
_Register8ToVector:
	sal	rdi, 2  	; Force some repetition.
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register16ToVector
; Purpose:	Writes 16-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Register16ToVector:
_Register16ToVector:
	sal	rdi, 1  	; Force some repetition.
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register32ToVector
; Purpose:	Writes 32-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Register32ToVector:
_Register32ToVector:
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register64ToVector
; Purpose:	Writes 64-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Register64ToVector:
_Register64ToVector:
	add	rdi, rdi
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

	dec	rdi
	jnz .L1
	ret


;------------------------------------------------------------------------------
; Name:		Vector8ToRegister
; Purpose:	Writes 8-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Vector8ToRegister:
_Vector8ToRegister:
	sal	rdi, 3  	; Force some repetition.
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Vector16ToRegister
; Purpose:	Writes 16-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Vector16ToRegister:
_Vector16ToRegister:
	sal	rdi, 2  	; Force some repetition.
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Vector32ToRegister
; Purpose:	Writes 32-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Vector32ToRegister:
_Vector32ToRegister:
	add 	rdi, rdi
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Vector64ToRegister
; Purpose:	Writes 64-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Vector64ToRegister:
_Vector64ToRegister:
	add	rdi, rdi
.L1:
	pextrq	rax, xmm1, 0	; 32 transfers by 8 bytes = 256 bytes 
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

	dec	rdi
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		CopyAVX
; Purpose:	Copies memory chunks that are 32-byte aligned.
; Params:	rdi = ptr to destination memory area
;		rsi = ptr to source memory area
; 		rdx = length in bytes
; 		rcx = loops
;------------------------------------------------------------------------------
	align 64
CopyAVX:
_CopyAVX:
	vzeroupper

	push	r10

	shr	rdx, 8	; Ensure length is multiple of 256.
	shl	rdx, 8

	prefetcht0	[rsi]

.L1:
	mov	r10, rdx

.L2:
	vmovdqa	ymm0, [rsi]
	vmovdqa	ymm1, [32+rsi]
	vmovdqa	ymm2, [64+rsi]
	vmovdqa	ymm3, [96+rsi]

	vmovdqa	[rdi], ymm0
	vmovdqa	[32+rdi], ymm1
	vmovdqa	[64+rdi], ymm2
	vmovdqa	[96+rdi], ymm3

	vmovdqa	ymm0, [128+rsi]
	vmovdqa	ymm1, [128+32+rsi]
	vmovdqa	ymm2, [128+64+rsi]
	vmovdqa	ymm3, [128+96+rsi]

	vmovdqa	[128+rdi], ymm0
	vmovdqa	[128+32+rdi], ymm1
	vmovdqa	[128+64+rdi], ymm2
	vmovdqa	[128+96+rdi], ymm3

	add	rsi, 256
	add	rdi, 256

	sub	r10, 256
	jnz	.L2

	sub	rsi, rdx	; rsi now points to start.
	sub	rdi, rdx	; rdi now points to start.

	dec	rcx
	jnz	.L1

	pop	r10

	ret


;------------------------------------------------------------------------------
; Name:		CopySSE
; Purpose:	Copies memory chunks that are 16-byte aligned.
; Params:	rdi = ptr to destination memory area
;		rsi = ptr to source memory area
; 		rdx = length in bytes
; 		rcx = loops
;------------------------------------------------------------------------------
	align 64
CopySSE:
_CopySSE:
	push	r10

	shr	rdx, 8	; Ensure length is multiple of 256.
	shl	rdx, 8

	prefetcht0	[rsi]

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
	mov	r10, rdx

.L2:
	movdqa	xmm0, [rsi]
	movdqa	xmm1, [16+rsi]
	movdqa	xmm2, [32+rsi]
	movdqa	xmm3, [48+rsi]

	movdqa	[rdi], xmm0
	movdqa	[16+rdi], xmm1
	movdqa	[32+rdi], xmm2
	movdqa	[48+rdi], xmm3

	movdqa	xmm4, [64+rsi]
	movdqa	xmm5, [80+rsi]
	movdqa	xmm6, [96+rsi]
	movdqa	xmm7, [112+rsi]

	movdqa	[64+rdi], xmm4
	movdqa	[80+rdi], xmm5
	movdqa	[96+rdi], xmm6
	movdqa	[112+rdi], xmm7

	movdqa	xmm8, [128+rsi]
	movdqa	xmm9, [144+rsi]
	movdqa	xmm10, [160+rsi]
	movdqa	xmm11, [176+rsi]

	movdqa	[128+rdi], xmm8
	movdqa	[144+rdi], xmm9
	movdqa	[160+rdi], xmm10
	movdqa	[176+rdi], xmm11

	movdqa	xmm12, [192+rsi]
	movdqa	xmm13, [208+rsi]
	movdqa	xmm14, [224+rsi]
	movdqa	xmm15, [240+rsi]

	movdqa	[192+rdi], xmm12
	movdqa	[208+rdi], xmm13
	movdqa	[224+rdi], xmm14
	movdqa	[240+rdi], xmm15

	add	rsi, 256
	add	rdi, 256

	sub	r10, 256
	jnz	.L2

	sub	rsi, rdx	; rsi now points to start.
	sub	rdi, rdx	; rdi now points to start.

	dec	rcx
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

	ret


;------------------------------------------------------------------------------
; Name:		CopySSE_128bytes
; Purpose:	Copies memory chunks that are 16-byte aligned.
; Params:	rdi = ptr to destination memory area
;		rsi = ptr to source memory area
; 		rdx = length in bytes
; 		rcx = loops
;------------------------------------------------------------------------------
	align 64
CopySSE_128bytes:
_CopySSE_128bytes:
	push	r10

	shr	rdx, 7	; Ensure length is multiple of 128.
	shl	rdx, 7

	prefetcht0	[rsi]

	; Save our non-parameter XMM registers.
	sub	rsp, 64
	movdqu	[rsp], xmm4
	movdqu	[16+rsp], xmm5
	movdqu	[32+rsp], xmm6
	movdqu	[48+rsp], xmm7

.L1:
	mov	r10, rdx

.L2:
	movdqa	xmm0, [rsi]
	movdqa	xmm1, [16+rsi]
	movdqa	xmm2, [32+rsi]
	movdqa	xmm3, [48+rsi]

	movdqa	[rdi], xmm0
	movdqa	[16+rdi], xmm1
	movdqa	[32+rdi], xmm2
	movdqa	[48+rdi], xmm3

	movdqa	xmm4, [64+rsi]
	movdqa	xmm5, [80+rsi]
	movdqa	xmm6, [96+rsi]
	movdqa	xmm7, [112+rsi]

	movdqa	[64+rdi], xmm4
	movdqa	[80+rdi], xmm5
	movdqa	[96+rdi], xmm6
	movdqa	[112+rdi], xmm7

	add	rsi, 128
	add	rdi, 128

	sub	r10, 128
	jnz	.L2

	sub	rsi, rdx	; rsi now points to start.
	sub	rdi, rdx	; rdi now points to start.

	dec	rcx
	jnz	.L1

	movdqu	xmm4, [rsp]
	movdqu	xmm5, [16+rsp]
	movdqu	xmm6, [32+rsp]
	movdqu	xmm7, [48+rsp]
	add	rsp, 64

	pop	r10

	ret


