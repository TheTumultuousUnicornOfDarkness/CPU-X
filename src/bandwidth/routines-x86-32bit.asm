;============================================================================
;  bandwidth, a benchmark to estimate memory transfer bandwidth.
;  Copyright (C) 2005-2015 by Zack T Smith.
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

bits	32
cpu	ia64

global	ReaderLODSQ
global	_ReaderLODSQ

global	ReaderLODSD
global	_ReaderLODSD

global	ReaderLODSW
global	_ReaderLODSW

global	ReaderLODSB
global	_ReaderLODSB

; Cygwin requires the underbar-prefixed symbols.
global	_WriterSSE2
global	WriterSSE2

global	_WriterAVX
global	WriterAVX

global	_WriterSSE2_128bytes
global	WriterSSE2_128bytes

global	_ReaderAVX
global	ReaderAVX

global	_ReaderSSE2
global	ReaderSSE2

global	ReaderSSE2_bypass
global	_ReaderSSE2_bypass

global	_ReaderSSE2_128bytes
global	ReaderSSE2_128bytes

global	ReaderSSE2_128bytes_bypass
global	_ReaderSSE2_128bytes_bypass

global	_RandomReaderSSE2
global	RandomReaderSSE2

global	_RandomReaderSSE2_bypass
global	RandomReaderSSE2_bypass

global  WriterAVX_bypass
global  _WriterAVX_bypass

global	_WriterSSE2_bypass
global	WriterSSE2_bypass

global	_WriterSSE2_128bytes_bypass
global	WriterSSE2_128bytes_bypass

global	_RandomWriterSSE2_bypass
global	RandomWriterSSE2_bypass

global	Reader
global	_Reader

global	Writer
global	_Writer

global	Reader_128bytes
global	_Reader_128bytes

global	Writer_128bytes
global	_Writer_128bytes

global	RandomReader
global	_RandomReader

global	RandomWriter
global	_RandomWriter

global	RandomWriterSSE2
global	_RandomWriterSSE2

global	get_cpuid_family
global	_get_cpuid_family

global	get_cpuid_cache_info
global	_get_cpuid_cache_info

global	get_cpuid1_ecx
global	_get_cpuid1_ecx

global	get_cpuid1_edx
global	_get_cpuid1_edx

global	get_cpuid7_ebx
global	_get_cpuid7_ebx

global  get_cpuid_80000001_ecx
global  _get_cpuid_80000001_ecx

global  get_cpuid_80000001_edx
global  _get_cpuid_80000001_edx

global	CopySSE
global	_CopySSE

global CopyAVX
global _CopyAVX

global	CopySSE_128bytes
global	_CopySSE_128bytes

global	RegisterToRegister
global	_RegisterToRegister

global	VectorToVector
global	_VectorToVector

global	VectorToVectorAVX
global	_VectorToVectorAVX

global	RegisterToVector
global	_RegisterToVector

global	VectorToRegister
global	_VectorToRegister

global	Register8ToVector
global	Register16ToVector
global	Register32ToVector
global	Register64ToVector
global	Vector8ToRegister
global	Vector16ToRegister
global	Vector32ToRegister
global	Vector64ToRegister

global	_Register8ToVector
global	_Register16ToVector
global	_Register32ToVector
global	_Register64ToVector
global	_Vector8ToRegister
global	_Vector16ToRegister
global	_Vector32ToRegister
global	_Vector64ToRegister

global	StackReader
global	_StackReader

global	StackWriter
global	_StackWriter

	section .text

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
	; N/A
	ret 

;------------------------------------------------------------------------------
; Name:		ReaderLODSD
; Purpose:	Reads 32-bit values sequentially from an area of memory
;		using LODSD instruction.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 32
ReaderLODSD:
_ReaderLODSD:
	shr	dword [esp+8], 2  	; length in double words rounded down.

	push	ebx
	push	ecx	; REP counter
	push 	edx
	
	mov	edx, [esp+12+12]
.L1:
	mov	esi, [esp+4+12]
	mov	ecx, [esp+8+12]

	rep	lodsd

	dec	edx
	jnz	.L1

	pop	edx
	pop	ecx
	pop	ebx
	ret 

;------------------------------------------------------------------------------
; Name:		ReaderLODSW
; Purpose:	Reads 16-bit values sequentially from an area of memory
;		using LODSW instruction.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 32
ReaderLODSW:
_ReaderLODSW:
	shr	dword [esp+8], 1  	; length in words rounded down.

	push	ebx
	push	ecx	; REP counter
	push 	edx
	
	mov	edx, [esp+12+12]
.L1:
	mov	esi, [esp+4+12]
	mov	ecx, [esp+8+12]

	rep	lodsw

	dec	edx
	jnz	.L1

	pop	edx
	pop	ecx
	pop	ebx
	ret 

;------------------------------------------------------------------------------
; Name:		ReaderLODSB
; Purpose:	Reads 8-bit values sequentially from an area of memory
;		using LODSB instruction.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 32
ReaderLODSB:
_ReaderLODSB:
	push	ebx
	push	ecx	; REP counter
	push 	edx
	
	mov	edx, [esp+12+12]
.L1:
	mov	esi, [esp+4+12]
	mov	ecx, [esp+8+12]

	rep	lodsb

	dec	edx
	jnz	.L1

	pop	edx
	pop	ecx
	pop	ebx
	ret 

;------------------------------------------------------------------------------
; Name:		Reader
; Purpose:	Reads 32-bit values sequentially from an area of memory.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
Reader:
_Reader:
	push	ebx
	push	ecx
	push	edx

	mov	ecx, [esp+12+12]	; loops to do.

	mov	edx, [esp+4+12]	; ptr to memory chunk.
	mov	ebx, edx	; ebx = limit in memory
	add	ebx, [esp+8+12]

.L1:
	mov	edx, [esp+4+12]	

.L2:
	mov	eax, [edx]
	mov	eax, [4+edx]
	mov	eax, [8+edx]
	mov	eax, [12+edx]
	mov	eax, [16+edx]
	mov	eax, [20+edx]
	mov	eax, [24+edx]
	mov	eax, [28+edx]
	mov	eax, [32+edx]
	mov	eax, [36+edx]
	mov	eax, [40+edx]
	mov	eax, [44+edx]
	mov	eax, [48+edx]
	mov	eax, [52+edx]
	mov	eax, [56+edx]
	mov	eax, [60+edx]
	mov	eax, [64+edx]
	mov	eax, [68+edx]
	mov	eax, [72+edx]
	mov	eax, [76+edx]
	mov	eax, [80+edx]
	mov	eax, [84+edx]
	mov	eax, [88+edx]
	mov	eax, [92+edx]
	mov	eax, [96+edx]
	mov	eax, [100+edx]
	mov	eax, [104+edx]
	mov	eax, [108+edx]
	mov	eax, [112+edx]
	mov	eax, [116+edx]
	mov	eax, [120+edx]
	mov	eax, [124+edx]

	mov	eax, [edx+128]
	mov	eax, [edx+132]
	mov	eax, [edx+136]
	mov	eax, [edx+140]
	mov	eax, [edx+144]
	mov	eax, [edx+148]
	mov	eax, [edx+152]
	mov	eax, [edx+156]
	mov	eax, [edx+160]
	mov	eax, [edx+164]
	mov	eax, [edx+168]
	mov	eax, [edx+172]
	mov	eax, [edx+176]
	mov	eax, [edx+180]
	mov	eax, [edx+184]
	mov	eax, [edx+188]
	mov	eax, [edx+192]
	mov	eax, [edx+196]
	mov	eax, [edx+200]
	mov	eax, [edx+204]
	mov	eax, [edx+208]
	mov	eax, [edx+212]
	mov	eax, [edx+216]
	mov	eax, [edx+220]
	mov	eax, [edx+224]
	mov	eax, [edx+228]
	mov	eax, [edx+232]
	mov	eax, [edx+236]
	mov	eax, [edx+240]
	mov	eax, [edx+244]
	mov	eax, [edx+248]
	mov	eax, [edx+252]

	add	edx, 256
	cmp	edx, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1

	pop	edx
	pop	ecx
	pop	ebx
	ret


;------------------------------------------------------------------------------
; Name:		Writer
; Purpose:	Writes 32-bit value sequentially to an area of memory.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = long to write
;------------------------------------------------------------------------------
	align 64
Writer:
_Writer:
	push	ebx
	push	ecx
	push	edx

	mov	ecx, [esp+12+12]
	mov	eax, [esp+16+12]

	mov	edx, [esp+4+12]	; edx = ptr to chunk
	mov	ebx, edx
	add	ebx, [esp+8+12]	; ebx = limit in memory

.L1:
	mov	edx, [esp+4+12]

.L2:
	mov	[edx], eax
	mov	[4+edx], eax
	mov	[8+edx], eax
	mov	[12+edx], eax
	mov	[16+edx], eax
	mov	[20+edx], eax
	mov	[24+edx], eax
	mov	[28+edx], eax
	mov	[32+edx], eax
	mov	[36+edx], eax
	mov	[40+edx], eax
	mov	[44+edx], eax
	mov	[48+edx], eax
	mov	[52+edx], eax
	mov	[56+edx], eax
	mov	[60+edx], eax
	mov	[64+edx], eax
	mov	[68+edx], eax
	mov	[72+edx], eax
	mov	[76+edx], eax
	mov	[80+edx], eax
	mov	[84+edx], eax
	mov	[88+edx], eax
	mov	[92+edx], eax
	mov	[96+edx], eax
	mov	[100+edx], eax
	mov	[104+edx], eax
	mov	[108+edx], eax
	mov	[112+edx], eax
	mov	[116+edx], eax
	mov	[120+edx], eax
	mov	[124+edx], eax

	mov	[edx+128], eax
	mov	[edx+132], eax
	mov	[edx+136], eax
	mov	[edx+140], eax
	mov	[edx+144], eax
	mov	[edx+148], eax
	mov	[edx+152], eax
	mov	[edx+156], eax
	mov	[edx+160], eax
	mov	[edx+164], eax
	mov	[edx+168], eax
	mov	[edx+172], eax
	mov	[edx+176], eax
	mov	[edx+180], eax
	mov	[edx+184], eax
	mov	[edx+188], eax
	mov	[edx+192], eax
	mov	[edx+196], eax
	mov	[edx+200], eax
	mov	[edx+204], eax
	mov	[edx+208], eax
	mov	[edx+212], eax
	mov	[edx+216], eax
	mov	[edx+220], eax
	mov	[edx+224], eax
	mov	[edx+228], eax
	mov	[edx+232], eax
	mov	[edx+236], eax
	mov	[edx+240], eax
	mov	[edx+244], eax
	mov	[edx+248], eax
	mov	[edx+252], eax

	add	edx, 256
	cmp	edx, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1

	pop	edx
	pop	ecx
	pop	ebx
	ret


;------------------------------------------------------------------------------
; Name:		Reader_128bytes
; Purpose:	Reads 32-bit values sequentially from an area of memory.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
Reader_128bytes:
_Reader_128bytes:
	push	ebx
	push	ecx
	push	edx

	mov	ecx, [esp+12+12]	; loops to do.

	mov	edx, [esp+4+12]	; ptr to memory chunk.
	mov	ebx, edx	; ebx = limit in memory
	add	ebx, [esp+8+12]

.L1:
	mov	edx, [esp+4+12]	

.L2:
	mov	eax, [edx]
	mov	eax, [4+edx]
	mov	eax, [8+edx]
	mov	eax, [12+edx]
	mov	eax, [16+edx]
	mov	eax, [20+edx]
	mov	eax, [24+edx]
	mov	eax, [28+edx]
	mov	eax, [32+edx]
	mov	eax, [36+edx]
	mov	eax, [40+edx]
	mov	eax, [44+edx]
	mov	eax, [48+edx]
	mov	eax, [52+edx]
	mov	eax, [56+edx]
	mov	eax, [60+edx]
	mov	eax, [64+edx]
	mov	eax, [68+edx]
	mov	eax, [72+edx]
	mov	eax, [76+edx]
	mov	eax, [80+edx]
	mov	eax, [84+edx]
	mov	eax, [88+edx]
	mov	eax, [92+edx]
	mov	eax, [96+edx]
	mov	eax, [100+edx]
	mov	eax, [104+edx]
	mov	eax, [108+edx]
	mov	eax, [112+edx]
	mov	eax, [116+edx]
	mov	eax, [120+edx]
	mov	eax, [124+edx]

	add	edx, 128
	cmp	edx, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1

	pop	edx
	pop	ecx
	pop	ebx
	ret


;------------------------------------------------------------------------------
; Name:		Writer_128bytes
; Purpose:	Writes 32-bit value sequentially to an area of memory.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = long to write
;------------------------------------------------------------------------------
	align 64
Writer_128bytes:
_Writer_128bytes:
	push	ebx
	push	ecx
	push	edx

	mov	ecx, [esp+12+12]
	mov	eax, [esp+16+12]

	mov	edx, [esp+4+12]	; edx = ptr to chunk
	mov	ebx, edx
	add	ebx, [esp+8+12]	; ebx = limit in memory

.L1:
	mov	edx, [esp+4+12]

.L2:
	mov	[edx], eax
	mov	[4+edx], eax
	mov	[8+edx], eax
	mov	[12+edx], eax
	mov	[16+edx], eax
	mov	[20+edx], eax
	mov	[24+edx], eax
	mov	[28+edx], eax
	mov	[32+edx], eax
	mov	[36+edx], eax
	mov	[40+edx], eax
	mov	[44+edx], eax
	mov	[48+edx], eax
	mov	[52+edx], eax
	mov	[56+edx], eax
	mov	[60+edx], eax
	mov	[64+edx], eax
	mov	[68+edx], eax
	mov	[72+edx], eax
	mov	[76+edx], eax
	mov	[80+edx], eax
	mov	[84+edx], eax
	mov	[88+edx], eax
	mov	[92+edx], eax
	mov	[96+edx], eax
	mov	[100+edx], eax
	mov	[104+edx], eax
	mov	[108+edx], eax
	mov	[112+edx], eax
	mov	[116+edx], eax
	mov	[120+edx], eax
	mov	[124+edx], eax

	add	edx, 128
	cmp	edx, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1

	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name: 	get_cpuid_cache_info
; 
get_cpuid_cache_info:
_get_cpuid_cache_info:
	push	ebp
	push	ebx
	push 	ecx
	push 	edx
	mov	eax, 4
	mov	ecx, [esp + 16 + 4 + 4]
	cpuid
	mov	ebp, eax
	mov	eax, [esp + 16 + 4]
	mov	[eax], ebp
	mov	[eax+4], ebx
	mov	[eax+8], ecx
	mov	[eax+12], edx
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_family
; 
get_cpuid_family:
_get_cpuid_family:
	push	ebx
	push 	ecx
	push 	edx
	xor	eax, eax
	cpuid
	mov	eax, [esp + 12 + 4]
	mov	[eax], ebx
	mov	[eax+4], edx
	mov	[eax+8], ecx
	mov	byte [eax+12], 0
	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid1_ecx
; 
get_cpuid1_ecx:
_get_cpuid1_ecx:
	push	ebx
	push 	ecx
	push 	edx
	mov	eax, 1
	cpuid
	mov	eax, ecx
	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid7_ebx
; 
get_cpuid7_ebx:
_get_cpuid7_ebx:
	push	ebx
	push 	ecx
	push 	edx
	mov	eax, 7
	xor	ecx, ecx
	cpuid
        mov	eax, ebx
	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_80000001_ecx
; 
get_cpuid_80000001_ecx:
_get_cpuid_80000001_ecx:
	push	ebx
	push 	ecx
	push 	edx
	mov	eax, 0x80000001
	cpuid
	mov	eax, ecx
	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_80000001_edx
; 
get_cpuid_80000001_edx:
_get_cpuid_80000001_edx:
	push	ebx
	push 	ecx
	push 	edx
	mov	eax, 0x80000001
	cpuid
	mov	eax, edx
	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid1_edx
; 
get_cpuid1_edx:
_get_cpuid1_edx:
	push	ebx
	push 	ecx
	push 	edx
	mov	eax, 1
	cpuid
	mov	eax, edx
	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		ReaderAVX
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
ReaderAVX:
_ReaderAVX:
	vzeroupper

	push	ebx
	push	ecx

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	vmovdqa	xmm0, [eax]	; Read aligned @ 16-byte boundary.
	vmovdqa	xmm0, [32+eax]
	vmovdqa	xmm0, [64+eax]
	vmovdqa	xmm0, [96+eax]
	vmovdqa	xmm0, [128+eax]
	vmovdqa	xmm0, [160+eax]
	vmovdqa	xmm0, [192+eax]
	vmovdqa	xmm0, [224+eax]

	add	eax, 256
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		ReaderSSE2
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2:
_ReaderSSE2:
	push	ebx
	push	ecx

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	movdqa	xmm0, [eax]	; Read aligned @ 16-byte boundary.
	movdqa	xmm0, [16+eax]
	movdqa	xmm0, [32+eax]
	movdqa	xmm0, [48+eax]
	movdqa	xmm0, [64+eax]
	movdqa	xmm0, [80+eax]
	movdqa	xmm0, [96+eax]
	movdqa	xmm0, [112+eax]

	movdqa	xmm0, [128+eax]
	movdqa	xmm0, [144+eax]
	movdqa	xmm0, [160+eax]
	movdqa	xmm0, [176+eax]
	movdqa	xmm0, [192+eax]
	movdqa	xmm0, [208+eax]
	movdqa	xmm0, [224+eax]
	movdqa	xmm0, [240+eax]

	add	eax, 256
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		ReaderSSE2_bypass
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2_bypass:
_ReaderSSE2_bypass:
	push	ebx
	push	ecx

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
        prefetchnta     [eax]
	movntdqa	xmm0, [eax]	; Read aligned @ 16-byte boundary.
	movntdqa	xmm0, [16+eax]
	movntdqa	xmm0, [32+eax]
	movntdqa	xmm0, [48+eax]

        prefetchnta     [eax+64]
	movntdqa	xmm0, [64+eax]
	movntdqa	xmm0, [80+eax]
	movntdqa	xmm0, [96+eax]
	movntdqa	xmm0, [112+eax]

        prefetchnta     [eax+128]
	movntdqa	xmm0, [128+eax]
	movntdqa	xmm0, [144+eax]
	movntdqa	xmm0, [160+eax]
	movntdqa	xmm0, [176+eax]

        prefetchnta     [eax+192]
	movntdqa	xmm0, [192+eax]
	movntdqa	xmm0, [208+eax]
	movntdqa	xmm0, [224+eax]
	movntdqa	xmm0, [240+eax]

	add	eax, 256
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		ReaderSSE2_128bytes_bypass
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2_128bytes_bypass:
_ReaderSSE2_128bytes_bypass:
	push	ebx
	push	ecx

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
        prefetchnta     [eax]
	movntdqa	xmm0, [eax]	; Read aligned @ 16-byte boundary.
	movntdqa	xmm0, [16+eax]
	movntdqa	xmm0, [32+eax]
	movntdqa	xmm0, [48+eax]
        prefetchnta     [eax+64]
	movntdqa	xmm0, [64+eax]
	movntdqa	xmm0, [80+eax]
	movntdqa	xmm0, [96+eax]
	movntdqa	xmm0, [112+eax]

	add	eax, 128
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		ReaderSSE2_128bytes
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
ReaderSSE2_128bytes:
_ReaderSSE2_128bytes:
	push	ebx
	push	ecx

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	movdqa	xmm0, [eax]	; Read aligned @ 16-byte boundary.
	movdqa	xmm0, [16+eax]
	movdqa	xmm0, [32+eax]
	movdqa	xmm0, [48+eax]
	movdqa	xmm0, [64+eax]
	movdqa	xmm0, [80+eax]
	movdqa	xmm0, [96+eax]
	movdqa	xmm0, [112+eax]

	add	eax, 128
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX
; Purpose:	Write 256-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = value (ignored)
;------------------------------------------------------------------------------
	align 64
WriterAVX:
_WriterAVX:
	vzeroupper

	push	ebx
	push	ecx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	vmovdqa	[eax], xmm0	
	vmovdqa	[32+eax], xmm0
	vmovdqa	[64+eax], xmm0
	vmovdqa	[96+eax], xmm0
	vmovdqa	[128+eax], xmm0
	vmovdqa	[160+eax], xmm0
	vmovdqa	[192+eax], xmm0
	vmovdqa	[224+eax], xmm0

	add	eax, 256
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2
; Purpose:	Write 128-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = value (ignored)
;------------------------------------------------------------------------------
	align 64
WriterSSE2:
_WriterSSE2:
	push	ebx
	push	ecx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	movdqa	[eax], xmm0	
	movdqa	[16+eax], xmm0
	movdqa	[32+eax], xmm0
	movdqa	[48+eax], xmm0
	movdqa	[64+eax], xmm0
	movdqa	[80+eax], xmm0
	movdqa	[96+eax], xmm0
	movdqa	[112+eax], xmm0

	movdqa	[128+eax], xmm0
	movdqa	[144+eax], xmm0
	movdqa	[160+eax], xmm0
	movdqa	[176+eax], xmm0
	movdqa	[192+eax], xmm0
	movdqa	[208+eax], xmm0
	movdqa	[224+eax], xmm0
	movdqa	[240+eax], xmm0

	add	eax, 256
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2
; Purpose:	Write 128-bit values sequentially from an area of memory.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = value (ignored)
;------------------------------------------------------------------------------
	align 64
WriterSSE2_128bytes:
_WriterSSE2_128bytes:
	push	ebx
	push	ecx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	movdqa	[eax], xmm0	
	movdqa	[16+eax], xmm0
	movdqa	[32+eax], xmm0
	movdqa	[48+eax], xmm0
	movdqa	[64+eax], xmm0
	movdqa	[80+eax], xmm0
	movdqa	[96+eax], xmm0
	movdqa	[112+eax], xmm0

	add	eax, 128
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		WriterAVX_bypass
; Purpose:	Write 256-bit values sequentially from an area of memory,
;		bypassing the cache.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = value (ignored)
;------------------------------------------------------------------------------

	align 64
WriterAVX_bypass:
_WriterAVX_bypass:
	vzeroupper

	push	ebx
	push	ecx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	vmovntdq	[eax], xmm0	; Write bypassing cache.
	vmovntdq	[32+eax], xmm0
	vmovntdq	[64+eax], xmm0
	vmovntdq	[96+eax], xmm0
	vmovntdq	[128+eax], xmm0
	vmovntdq	[160+eax], xmm0
	vmovntdq	[192+eax], xmm0
	vmovntdq	[224+eax], xmm0

	add	eax, 256
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2_bypass
; Purpose:	Write 128-bit values sequentially from an area of memory,
;		bypassing the cache.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = value (ignored)
;------------------------------------------------------------------------------
	align 64
WriterSSE2_bypass:
_WriterSSE2_bypass:
	push	ebx
	push	ecx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	movntdq	[eax], xmm0	; Write bypassing cache.
	movntdq	[16+eax], xmm0
	movntdq	[32+eax], xmm0
	movntdq	[48+eax], xmm0
	movntdq	[64+eax], xmm0
	movntdq	[80+eax], xmm0
	movntdq	[96+eax], xmm0
	movntdq	[112+eax], xmm0

	movntdq	[128+eax], xmm0
	movntdq	[144+eax], xmm0
	movntdq	[160+eax], xmm0
	movntdq	[176+eax], xmm0
	movntdq	[192+eax], xmm0
	movntdq	[208+eax], xmm0
	movntdq	[224+eax], xmm0
	movntdq	[240+eax], xmm0

	add	eax, 256
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		WriterSSE2_128bytes_bypass
; Purpose:	Write 128-bit values sequentially from an area of memory,
;		bypassing the cache.
; Params:	[esp+4] = ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = value (ignored)
;------------------------------------------------------------------------------
	align 64
WriterSSE2_128bytes_bypass:
_WriterSSE2_128bytes_bypass:
	push	ebx
	push	ecx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+8]

	mov	eax, [esp+4+8]
	mov	ebx, eax
	add	ebx, [esp+8+8]	; ebx points to end.

.L1:
	mov	eax, [esp+4+8]

.L2:
	movntdq	[eax], xmm0	; Write bypassing cache.
	movntdq	[16+eax], xmm0
	movntdq	[32+eax], xmm0
	movntdq	[48+eax], xmm0
	movntdq	[64+eax], xmm0
	movntdq	[80+eax], xmm0
	movntdq	[96+eax], xmm0
	movntdq	[112+eax], xmm0

	add	eax, 128
	cmp	eax, ebx
	jb	.L2

	sub	ecx, 1
	jnz	.L1
	
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		RandomReader
; Purpose:	Reads 32-bit values randomly from an area of memory.
; Params:	
;		[esp+4]	= ptr to array of chunk pointers
; 		[esp+8] = # of 128-byte chunks
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
RandomReader:
_RandomReader:
	push	ebx
	push	ecx
	push	edx

	mov	ecx, [esp+12+12]	; loops to do.

.L0:
	mov	ebx, [esp+8+12]		; # chunks to do

.L1:
	sub	ebx, 1
	jc	.L2

	mov	edx, [esp+4+12]  	; get ptr to memory chunk.
	mov	edx, [edx + 4*ebx]

	mov	eax, [edx+160]
	mov	eax, [edx+232]
	mov	eax, [edx+224]
	mov	eax, [96+edx]
	mov	eax, [edx+164]
	mov	eax, [76+edx]
	mov	eax, [100+edx]
	mov	eax, [edx+220]
	mov	eax, [edx+248]
	mov	eax, [104+edx]
	mov	eax, [4+edx]
	mov	eax, [edx+136]
	mov	eax, [112+edx]
	mov	eax, [edx+200]
	mov	eax, [12+edx]
	mov	eax, [edx+128]
	mov	eax, [edx+148]
	mov	eax, [edx+196]
	mov	eax, [edx+216]
	mov	eax, [edx]
	mov	eax, [84+edx]
	mov	eax, [edx+140]
	mov	eax, [edx+204]
	mov	eax, [edx+184]
	mov	eax, [124+edx]
	mov	eax, [48+edx]
	mov	eax, [64+edx]
	mov	eax, [edx+212]
	mov	eax, [edx+240]
	mov	eax, [edx+236]
	mov	eax, [24+edx]
	mov	eax, [edx+252]
	mov	eax, [68+edx]
	mov	eax, [20+edx]
	mov	eax, [72+edx]
	mov	eax, [32+edx]
	mov	eax, [28+edx]
	mov	eax, [52+edx]
	mov	eax, [edx+244]
	mov	eax, [edx+180]
	mov	eax, [80+edx]
	mov	eax, [60+edx]
	mov	eax, [8+edx]
	mov	eax, [56+edx]
	mov	eax, [edx+208]
	mov	eax, [edx+228]
	mov	eax, [40+edx]
	mov	eax, [edx+172]
	mov	eax, [120+edx]
	mov	eax, [edx+176]
	mov	eax, [108+edx]
	mov	eax, [edx+132]
	mov	eax, [16+edx]
	mov	eax, [44+edx]
	mov	eax, [92+edx]
	mov	eax, [edx+168]
	mov	eax, [edx+152]
	mov	eax, [edx+156]
	mov	eax, [edx+188]
	mov	eax, [36+edx]
	mov	eax, [88+edx]
	mov	eax, [116+edx]
	mov	eax, [edx+192]
	mov	eax, [edx+144]

	jmp	.L1

.L2:
	sub	ecx, 1
	jnz	.L0

	pop	edx
	pop	ecx
	pop	ebx
	ret


;------------------------------------------------------------------------------
; Name:		RandomReaderSSE2
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	
;		[esp+4]	= ptr to array of chunk pointers
; 		[esp+8] = # of 128-byte chunks
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
RandomReaderSSE2:
_RandomReaderSSE2:
	push	ebx
	push	ecx
	push	edx

	mov	ecx, [esp+12+12]	; loops to do.

.L0:
	mov	ebx, [esp+8+12]		; # chunks to do

.L1:
	sub	ebx, 1
	jc	.L2

	mov	edx, [esp+4+12]  	; get ptr to memory chunk.
	mov	edx, [edx + 4*ebx]

; Read aligned @ 16-byte boundary.
	movdqa	xmm0, [240+edx]
	movdqa	xmm0, [128+edx]
	movdqa	xmm0, [64+edx]
	movdqa	xmm0, [208+edx]
	movdqa	xmm0, [112+edx]
	movdqa	xmm0, [176+edx]
	movdqa	xmm0, [144+edx]
	movdqa	xmm0, [edx]
	movdqa	xmm0, [96+edx]
	movdqa	xmm0, [16+edx]
	movdqa	xmm0, [192+edx]
	movdqa	xmm0, [160+edx]
	movdqa	xmm0, [32+edx]
	movdqa	xmm0, [48+edx]
	movdqa	xmm0, [224+edx]
	movdqa	xmm0, [80+edx]

	jmp	.L1

.L2:
	sub	ecx, 1
	jnz	.L0

	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		RandomReaderSSE2_bypass
; Purpose:	Reads 128-bit values sequentially from an area of memory.
; Params:	
;		[esp+4]	= ptr to array of chunk pointers
; 		[esp+8] = # of 128-byte chunks
; 		[esp+12] = loops
;------------------------------------------------------------------------------
	align 64
RandomReaderSSE2_bypass:
_RandomReaderSSE2_bypass:
	push	ebx
	push	ecx
	push	edx

	mov	ecx, [esp+12+12]	; loops to do.

.L0:
	mov	ebx, [esp+8+12]		; # chunks to do

.L1:
	sub	ebx, 1
	jc	.L2

	mov	edx, [esp+4+12]  	; get ptr to memory chunk.
	mov	edx, [edx + 4*ebx]

; Read aligned @ 16-byte boundary.
        prefetchnta     [edx+192]
	movntdqa	xmm0, [240+edx]
        prefetchnta     [edx]
	movntdqa	xmm0, [edx]
        prefetchnta     [edx+128]
	movntdqa	xmm0, [128+edx]
        prefetchnta     [edx+64]
	movntdqa	xmm0, [64+edx]
	movntdqa	xmm0, [208+edx]
	movntdqa	xmm0, [112+edx]
	movntdqa	xmm0, [32+edx]
	movntdqa	xmm0, [176+edx]
	movntdqa	xmm0, [144+edx]
	movntdqa	xmm0, [96+edx]
	movntdqa	xmm0, [16+edx]
	movntdqa	xmm0, [160+edx]
	movntdqa	xmm0, [192+edx]
	movntdqa	xmm0, [48+edx]
	movntdqa	xmm0, [224+edx]
	movntdqa	xmm0, [80+edx]

	jmp	.L1

.L2:
	sub	ecx, 1
	jnz	.L0

	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriter
; Purpose:	Writes 32-bit value sequentially to an area of memory.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = long to write
;------------------------------------------------------------------------------
	align 64
RandomWriter:
_RandomWriter:
	push	ebx
	push	ecx
	push	edx

	mov	eax, [esp+16+12]	; get datum.
	mov	ecx, [esp+12+12]	; loops to do.

.L0:
	mov	ebx, [esp+8+12]		; # chunks to do

.L1:
	sub	ebx, 1
	jc	.L2

	mov	edx, [esp+4+12]  	; get ptr to memory chunk.
	mov	edx, [edx + 4*ebx]

	mov	[edx+212], eax
	mov	[edx+156], eax
	mov	[edx+132], eax
	mov	[20+edx], eax
	mov	[edx+172], eax
	mov	[edx+196], eax
	mov	[edx+248], eax
	mov	[edx], eax
	mov	[edx+136], eax
	mov	[edx+228], eax
	mov	[edx+160], eax
	mov	[80+edx], eax
	mov	[76+edx], eax
	mov	[32+edx], eax
	mov	[64+edx], eax
	mov	[68+edx], eax
	mov	[120+edx], eax
	mov	[edx+216], eax
	mov	[124+edx], eax
	mov	[28+edx], eax
	mov	[edx+152], eax
	mov	[36+edx], eax
	mov	[edx+220], eax
	mov	[edx+188], eax
	mov	[48+edx], eax
	mov	[104+edx], eax
	mov	[72+edx], eax
	mov	[96+edx], eax
	mov	[edx+184], eax
	mov	[112+edx], eax
	mov	[edx+236], eax
	mov	[edx+224], eax
	mov	[edx+252], eax
	mov	[88+edx], eax
	mov	[edx+180], eax
	mov	[60+edx], eax
	mov	[24+edx], eax
	mov	[edx+192], eax
	mov	[edx+164], eax
	mov	[edx+204], eax
	mov	[44+edx], eax
	mov	[edx+168], eax
	mov	[92+edx], eax
	mov	[edx+208], eax
	mov	[8+edx], eax
	mov	[edx+144], eax
	mov	[edx+148], eax
	mov	[edx+128], eax
	mov	[52+edx], eax
	mov	[4+edx], eax
	mov	[108+edx], eax
	mov	[12+edx], eax
	mov	[56+edx], eax
	mov	[edx+200], eax
	mov	[edx+232], eax
	mov	[16+edx], eax
	mov	[edx+244], eax
	mov	[40+edx], eax
	mov	[edx+140], eax
	mov	[84+edx], eax
	mov	[100+edx], eax
	mov	[116+edx], eax
	mov	[edx+176], eax
	mov	[edx+240], eax

	jmp	.L1

.L2:
	sub	ecx, 1
	jnz	.L0

	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		RandomWriterSSE2
; Purpose:	Writes 128-bit value randomly to an area of memory.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = long to write
;------------------------------------------------------------------------------
	align 64
RandomWriterSSE2:
_RandomWriterSSE2:
	push	ebx
	push	ecx
	push	edx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+12]	; loops to do.

.L0:
	mov	ebx, [esp+8+12]		; # chunks to do

.L1:
	sub	ebx, 1
	jc	.L2

	mov	edx, [esp+4+12]  	; get ptr to memory chunk.
	mov	edx, [edx + 4*ebx]

	movdqa	[64+edx], xmm0
	movdqa	[208+edx], xmm0
	movdqa	[128+edx], xmm0
	movdqa	[112+edx], xmm0
	movdqa	[176+edx], xmm0
	movdqa	[144+edx], xmm0
	movdqa	[edx], xmm0
	movdqa	[96+edx], xmm0
	movdqa	[48+edx], xmm0
	movdqa	[16+edx], xmm0
	movdqa	[192+edx], xmm0
	movdqa	[160+edx], xmm0
	movdqa	[32+edx], xmm0
	movdqa	[240+edx], xmm0
	movdqa	[224+edx], xmm0
	movdqa	[80+edx], xmm0

	jmp	.L1

.L2:
	sub	ecx, 1
	jnz	.L0

	pop	edx
	pop	ecx
	pop	ebx
	ret


;------------------------------------------------------------------------------
; Name:		RandomWriterSSE2_bypass
; Purpose:	Writes 128-bit value randomly into memory, bypassing caches.
; Params:	
;		[esp+4]	= ptr to memory area
; 		[esp+8] = length in bytes
; 		[esp+12] = loops
; 		[esp+16] = long to write
;------------------------------------------------------------------------------
	align 64
RandomWriterSSE2_bypass:
_RandomWriterSSE2_bypass:
	push	ebx
	push	ecx
	push	edx

	mov	eax, [esp+16+8]
	movd	xmm0, eax	; Create a 128-bit replication of the 32-bit
	movd	xmm1, eax	; value that was provided.
	movd	xmm2, eax
	movd	xmm3, eax
	pslldq	xmm1, 32
	pslldq	xmm2, 64
	pslldq	xmm3, 96
	por	xmm0, xmm1
	por	xmm0, xmm2
	por	xmm0, xmm3

	mov	ecx, [esp+12+12]	; loops to do.

.L0:
	mov	ebx, [esp+8+12]		; # chunks to do

.L1:
	sub	ebx, 1
	jc	.L2

	mov	edx, [esp+4+12]  	; get ptr to memory chunk.
	mov	edx, [edx + 4*ebx]

	movntdq	[128+edx], xmm0
	movntdq	[240+edx], xmm0
	movntdq	[112+edx], xmm0
	movntdq	[64+edx], xmm0
	movntdq	[176+edx], xmm0
	movntdq	[144+edx], xmm0
	movntdq	[edx], xmm0
	movntdq	[208+edx], xmm0
	movntdq	[80+edx], xmm0
	movntdq	[96+edx], xmm0
	movntdq	[48+edx], xmm0
	movntdq	[16+edx], xmm0
	movntdq	[192+edx], xmm0
	movntdq	[160+edx], xmm0
	movntdq	[224+edx], xmm0
	movntdq	[32+edx], xmm0

	jmp	.L1

.L2:
	sub	ecx, 1
	jnz	.L0

	pop	edx
	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		RegisterToRegister
; Purpose:	Reads/writes 32-bit values between registers of 
;		the main register set.
; Params:	
; 		dword [esp+4] = loops
;------------------------------------------------------------------------------
	align 64
RegisterToRegister:
_RegisterToRegister:
	push	ebx
	push	ecx

	mov	ecx, [esp+4+8]	; loops to do.

.L1:
	mov	eax, ebx	; 64 transfers by 4 bytes = 256 bytes
	mov	eax, ecx
	mov	eax, edx
	mov	eax, esi
	mov	eax, edi
	mov	eax, ebp
	mov	eax, esp
	mov	eax, ebx
	mov	eax, ebx
	mov	eax, ecx
	mov	eax, edx
	mov	eax, esi
	mov	eax, edi
	mov	eax, ebp
	mov	eax, esp
	mov	eax, ebx
	mov	eax, ebx
	mov	eax, ecx
	mov	eax, edx
	mov	eax, esi
	mov	eax, edi
	mov	eax, ebp
	mov	eax, esp
	mov	eax, ebx
	mov	eax, ebx
	mov	eax, ecx
	mov	eax, edx
	mov	eax, esi
	mov	eax, edi
	mov	eax, ebp
	mov	eax, esp
	mov	eax, ebx

	mov	ebx, eax
	mov	ebx, ecx
	mov	ebx, edx
	mov	ebx, esi
	mov	ebx, edi
	mov	ebx, ebp
	mov	ebx, esp
	mov	ebx, eax
	mov	ebx, eax
	mov	ebx, ecx
	mov	ebx, edx
	mov	ebx, esi
	mov	ebx, edi
	mov	ebx, ebp
	mov	ebx, esp
	mov	ebx, eax
	mov	ebx, eax
	mov	ebx, ecx
	mov	ebx, edx
	mov	ebx, esi
	mov	ebx, edi
	mov	ebx, ebp
	mov	ebx, esp
	mov	ebx, eax
	mov	ebx, eax
	mov	ebx, ecx
	mov	ebx, edx
	mov	ebx, esi
	mov	ebx, edi
	mov	ebx, ebp
	mov	ebx, esp
	mov	ebx, eax

	dec	ecx
	jnz	.L1

	pop	ecx
	pop	ebx
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

	mov	eax, [esp + 4]
.L1:
	vmovdqa	ymm0, ymm1	; Each move moves 32 bytes, so we need 8
	vmovdqa	ymm0, ymm2	; moves to transfer a 256 byte chunk.
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm2, ymm0
	vmovdqa	ymm1, ymm2
	vmovdqa	ymm2, ymm1
	vmovdqa	ymm0, ymm3
	vmovdqa	ymm3, ymm1

	dec	eax
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToVector
; Purpose:	Reads/writes 128-bit values between registers of 
;		the vector register set, in this case XMM.
; Params:	dword [esp + 4] = count.
;------------------------------------------------------------------------------
	align 64
VectorToVector:
_VectorToVector:
	mov	eax, [esp + 4]
.L1:
	movdqa	xmm0, xmm1
	movdqa	xmm0, xmm2
	movdqa	xmm0, xmm3
	movdqa	xmm2, xmm0
	movdqa	xmm1, xmm2
	movdqa	xmm2, xmm1
	movdqa	xmm0, xmm3
	movdqa	xmm3, xmm1

	movdqa	xmm3, xmm2
	movdqa	xmm1, xmm3
	movdqa	xmm2, xmm1
	movdqa	xmm0, xmm1
	movdqa	xmm1, xmm2
	movdqa	xmm0, xmm1
	movdqa	xmm0, xmm3
	movdqa	xmm3, xmm0

	dec	eax
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		RegisterToVector
; Purpose:	Writes 32-bit main register values into 128-bit vector register
;		clearing the upper unused bits.
; Params:	dword [esp + 4] = count.
;------------------------------------------------------------------------------
	align 64
RegisterToVector:
_RegisterToVector:
	mov 	eax, [esp + 4]
	add	eax, eax	; Double # of loops.
.L1:
	movd	xmm1, eax	; 32 transfers of 4 bytes = 128 bytes
	movd	xmm2, eax
	movd	xmm3, eax
	movd	xmm0, eax
	movd	xmm1, eax
	movd	xmm2, eax
	movd	xmm3, eax
	movd	xmm0, eax

	movd	xmm1, eax
	movd	xmm3, eax
	movd	xmm2, eax
	movd	xmm0, eax
	movd	xmm1, eax
	movd	xmm2, eax
	movd	xmm3, eax
	movd	xmm0, eax

	movd	xmm0, eax
	movd	xmm2, eax
	movd	xmm0, eax
	movd	xmm3, eax
	movd	xmm1, eax
	movd	xmm3, eax
	movd	xmm2, eax
	movd	xmm0, eax

	movd	xmm0, eax
	movd	xmm3, eax
	movd	xmm1, eax
	movd	xmm2, eax
	movd	xmm0, eax
	movd	xmm2, eax
	movd	xmm3, eax
	movd	xmm0, eax

	dec	eax
	jnz	.L1
	ret

;------------------------------------------------------------------------------
; Name:		VectorToRegister
; Purpose:	Writes lowest 32 bits of vector registers into 32-bit main
;		register.
; Params:	dword [esp + 4] = count.
;------------------------------------------------------------------------------
	align 64
VectorToRegister:
_VectorToRegister:
	mov 	eax, [esp + 4]
	add	eax, eax	; Double # of loops.
	push	ebx
.L1:
	movd	ebx, xmm1	; 4 bytes per transfer therefore need 64
	movd	ebx, xmm2	; to transfer 256 bytes.
	movd	ebx, xmm3
	movd	ebx, xmm0
	movd	ebx, xmm1
	movd	ebx, xmm2
	movd	ebx, xmm3
	movd	ebx, xmm0

	movd	ebx, xmm1
	movd	ebx, xmm3
	movd	ebx, xmm2
	movd	ebx, xmm0
	movd	ebx, xmm1
	movd	ebx, xmm2
	movd	ebx, xmm3
	movd	ebx, xmm0

	movd	ebx, xmm0
	movd	ebx, xmm2
	movd	ebx, xmm0
	movd	ebx, xmm3
	movd	ebx, xmm1
	movd	ebx, xmm3
	movd	ebx, xmm2
	movd	ebx, xmm0

	movd	ebx, xmm0
	movd	ebx, xmm3
	movd	ebx, xmm1
	movd	ebx, xmm2
	movd	ebx, xmm0
	movd	ebx, xmm2
	movd	ebx, xmm3
	movd	ebx, xmm0

	dec	eax
	jnz	.L1

	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		StackReader
; Purpose:	Reads 32-bit values off the stack into registers of
;		the main register set, effectively testing L1 cache access
;		*and* effective-address calculation speed.
; Params:	
; 		dword [esp+4] = loops
;------------------------------------------------------------------------------
	align 64
StackReader:
_StackReader:
	push	ebx
	push	ecx

	mov	ecx, [esp+4+8]	; loops to do.

	push	dword 7000	; [esp+24]
	push	dword 6000	; [esp+20]
	push	dword 5000	; [esp+16]
	push	dword 4000	; [esp+12]
	push	dword 3000	; [esp+8]
	push	dword 2000	; [esp+4]
	push	dword 1000	; [esp]

.L1:
	mov	eax, [esp]
	mov	eax, [esp+8]
	mov	eax, [esp+12]
	mov	eax, [esp+16]
	mov	eax, [esp+20]
	mov	eax, [esp+4]
	mov	eax, [esp+24]
	mov	eax, [esp]
	mov	eax, [esp]
	mov	eax, [esp+8]
	mov	eax, [esp+12]
	mov	eax, [esp+16]
	mov	eax, [esp+20]
	mov	eax, [esp+4]
	mov	eax, [esp+24]
	mov	eax, [esp]
	mov	eax, [esp]
	mov	eax, [esp+8]
	mov	eax, [esp+12]
	mov	eax, [esp+16]
	mov	eax, [esp+20]
	mov	eax, [esp+4]
	mov	eax, [esp+24]
	mov	eax, [esp+4]
	mov	eax, [esp+4]
	mov	eax, [esp+8]
	mov	eax, [esp+12]
	mov	eax, [esp+16]
	mov	eax, [esp+20]
	mov	eax, [esp+4]
	mov	eax, [esp+24]
	mov	eax, [esp+4]

	mov	ebx, [esp]
	mov	ebx, [esp+8]
	mov	ebx, [esp+12]
	mov	ebx, [esp+16]
	mov	ebx, [esp+20]
	mov	ebx, [esp+4]
	mov	ebx, [esp+24]
	mov	ebx, [esp]
	mov	ebx, [esp]
	mov	ebx, [esp+8]
	mov	ebx, [esp+12]
	mov	ebx, [esp+16]
	mov	ebx, [esp+20]
	mov	ebx, [esp+4]
	mov	ebx, [esp+24]
	mov	ebx, [esp]
	mov	ebx, [esp]
	mov	ebx, [esp+8]
	mov	ebx, [esp+12]
	mov	ebx, [esp+16]
	mov	ebx, [esp+20]
	mov	ebx, [esp+4]
	mov	ebx, [esp+24]
	mov	ebx, [esp+4]
	mov	ebx, [esp+4]
	mov	ebx, [esp+8]
	mov	ebx, [esp+12]
	mov	ebx, [esp+16]
	mov	ebx, [esp+20]
	mov	ebx, [esp+4]
	mov	ebx, [esp+24]
	mov	ebx, [esp+4]

	dec	ecx
	jnz	.L1

	add	esp, 28

	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		StackWriter
; Purpose:	Writes 32-bit values into the stack from registers of
;		the main register set, effectively testing L1 cache access
;		*and* effective-address calculation speed.
; Params:	
; 		dword [esp+4] = loops
;------------------------------------------------------------------------------
	align 64
StackWriter:
_StackWriter:
	push	ebx
	push	ecx

	mov	ecx, [esp+4+8]	; loops to do.

	push	dword 7000	; [esp+24]
	push	dword 6000	; [esp+20]
	push	dword 5000	; [esp+16]
	push	dword 4000	; [esp+12]
	push	dword 3000	; [esp+8]
	push	dword 2000	; [esp+4]
	push	dword 1000	; [esp]

	xor	eax, eax
	mov	ebx, 0xffffffff

.L1:
	mov	[esp], eax
	mov	[esp+8], eax
	mov	[esp+12], eax
	mov	[esp+16], eax
	mov	[esp+20], eax
	mov	[esp+4], eax
	mov	[esp+24], eax
	mov	[esp], eax
	mov	[esp], eax
	mov	[esp+8], eax
	mov	[esp+12], eax
	mov	[esp+16], eax
	mov	[esp+20], eax
	mov	[esp+4], eax
	mov	[esp+24], eax
	mov	[esp], eax
	mov	[esp], eax
	mov	[esp+8], eax
	mov	[esp+12], eax
	mov	[esp+16], eax
	mov	[esp+20], eax
	mov	[esp+4], eax
	mov	[esp+24], eax
	mov	[esp+4], eax
	mov	[esp+4], eax
	mov	[esp+8], eax
	mov	[esp+12], eax
	mov	[esp+16], eax
	mov	[esp+20], eax
	mov	[esp+4], eax
	mov	[esp+24], eax
	mov	[esp+4], eax

	mov	[esp], ebx
	mov	[esp+8], ebx
	mov	[esp+12], ebx
	mov	[esp+16], ebx
	mov	[esp+20], ebx
	mov	[esp+4], ebx
	mov	[esp+24], ebx
	mov	[esp], ebx
	mov	[esp], ebx
	mov	[esp+8], ebx
	mov	[esp+12], ebx
	mov	[esp+16], ebx
	mov	[esp+20], ebx
	mov	[esp+4], ebx
	mov	[esp+24], ebx
	mov	[esp], ebx
	mov	[esp], ebx
	mov	[esp+8], ebx
	mov	[esp+12], ebx
	mov	[esp+16], ebx
	mov	[esp+20], ebx
	mov	[esp+4], ebx
	mov	[esp+24], ebx
	mov	[esp+4], ebx
	mov	[esp+4], ebx
	mov	[esp+8], ebx
	mov	[esp+12], ebx
	mov	[esp+16], ebx
	mov	[esp+20], ebx
	mov	[esp+4], ebx
	mov	[esp+24], ebx
	mov	[esp+4], ebx

	sub	ecx, 1
	jnz	.L1

	add	esp, 28

	pop	ecx
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		Register8ToVector
; Purpose:	Writes 8-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	dword [esp + 4]
;------------------------------------------------------------------------------
	align 64
Register8ToVector:
_Register8ToVector:
	mov	eax, [esp + 4]
	sal	eax, 4  	; Force some repetition.
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

	pinsrb	xmm1, al, 0
	pinsrb	xmm2, al, 1
	pinsrb	xmm3, al, 2
	pinsrb	xmm1, al, 3
	pinsrb	xmm2, al, 4
	pinsrb	xmm3, al, 5
	pinsrb	xmm0, cl, 6
	pinsrb	xmm0, bl, 7

	pinsrb	xmm0, al, 0
	pinsrb	xmm0, al, 1
	pinsrb	xmm0, al, 2
	pinsrb	xmm0, al, 3
	pinsrb	xmm0, al, 4
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

	pinsrb	xmm1, al, 10
	pinsrb	xmm2, al, 11
	pinsrb	xmm3, al, 12
	pinsrb	xmm1, al, 13
	pinsrb	xmm2, al, 14
	pinsrb	xmm3, al, 15
	pinsrb	xmm0, cl, 6
	pinsrb	xmm0, bl, 7

	pinsrb	xmm0, al, 9
	pinsrb	xmm0, al, 8
	pinsrb	xmm0, al, 11
	pinsrb	xmm0, al, 3
	pinsrb	xmm0, al, 4
	pinsrb	xmm0, al, 5
	pinsrb	xmm0, cl, 6
	pinsrb	xmm0, bl, 7

	dec	eax
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
	mov	eax, [esp + 4]
	sal	eax, 3  	; Force some repetition.
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

	pinsrw	xmm1, ax, 0
	pinsrw	xmm2, ax, 1
	pinsrw	xmm3, ax, 2
	pinsrw	xmm1, ax, 3
	pinsrw	xmm2, ax, 4
	pinsrw	xmm3, ax, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, bx, 7

	pinsrw	xmm0, ax, 0
	pinsrw	xmm0, ax, 1
	pinsrw	xmm0, ax, 2
	pinsrw	xmm0, ax, 3
	pinsrw	xmm0, ax, 4
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

	pinsrw	xmm1, ax, 0
	pinsrw	xmm2, ax, 1
	pinsrw	xmm3, ax, 2
	pinsrw	xmm1, ax, 3
	pinsrw	xmm2, ax, 4
	pinsrw	xmm3, ax, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, bx, 7

	pinsrw	xmm0, ax, 0
	pinsrw	xmm0, ax, 1
	pinsrw	xmm0, ax, 2
	pinsrw	xmm0, ax, 3
	pinsrw	xmm0, ax, 4
	pinsrw	xmm0, ax, 5
	pinsrw	xmm0, bp, 6
	pinsrw	xmm0, bx, 7

	dec	eax
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
	mov	eax, [esp + 4]
	sal	eax, 2  	; Force some repetition.
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

	pinsrd	xmm1, eax, 0
	pinsrd	xmm2, eax, 1
	pinsrd	xmm3, eax, 2
	pinsrd	xmm1, eax, 3
	pinsrd	xmm2, eax, 0
	pinsrd	xmm3, eax, 1
	pinsrd	xmm0, ebp, 2
	pinsrd	xmm0, ebx, 3

	pinsrd	xmm0, eax, 0
	pinsrd	xmm0, eax, 1
	pinsrd	xmm0, eax, 2
	pinsrd	xmm0, eax, 3
	pinsrd	xmm0, eax, 0
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

	pinsrd	xmm1, eax, 0
	pinsrd	xmm2, eax, 1
	pinsrd	xmm3, eax, 2
	pinsrd	xmm1, eax, 3
	pinsrd	xmm2, eax, 0
	pinsrd	xmm3, eax, 1
	pinsrd	xmm0, ebp, 2
	pinsrd	xmm0, ebx, 3

	pinsrd	xmm0, eax, 0
	pinsrd	xmm0, eax, 1
	pinsrd	xmm0, eax, 2
	pinsrd	xmm0, eax, 3
	pinsrd	xmm0, eax, 0
	pinsrd	xmm0, eax, 0
	pinsrd	xmm0, ebp, 0
	pinsrd	xmm0, ebx, 0
	pinsrd	xmm0, esp, 0

	dec	eax
	jnz .L1
	ret

;------------------------------------------------------------------------------
; Name:		Register64ToVector
; Purpose:	Writes 64-bit main register values into 128-bit vector register
;		without clearing the unused bits.
; Params:	rdi = loops
;------------------------------------------------------------------------------
Register64ToVector:
_Register64ToVector:
	ret


;------------------------------------------------------------------------------
; Name:		Vector8ToRegister
; Purpose:	Writes 8-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Vector8ToRegister:
_Vector8ToRegister:
	mov	eax, [esp + 4]
	sal	eax, 4  	; Force some repetition.
	push 	ebx
.L1:
	pextrb	ebx, xmm1, 0
	pextrb	ebx, xmm2, 1
	pextrb	ebx, xmm3, 2
	pextrb	ebx, xmm1, 3
	pextrb	ebx, xmm2, 4
	pextrb	ebx, xmm3, 5
	pextrb	ebx, xmm0, 6
	pextrb	ebx, xmm0, 7

	pextrb	ebx, xmm0, 0
	pextrb	ebx, xmm1, 1
	pextrb	ebx, xmm2, 2
	pextrb	ebx, xmm3, 3
	pextrb	ebx, xmm3, 4
	pextrb	ebx, xmm2, 15
	pextrb	ebx, xmm1, 6
	pextrb	ebx, xmm0, 7

	pextrb	ebx, xmm1, 0
	pextrb	ebx, xmm2, 1
	pextrb	ebx, xmm3, 2
	pextrb	ebx, xmm1, 3
	pextrb	ebx, xmm2, 4
	pextrb	ebx, xmm3, 5
	pextrb	ebx, xmm0, 6
	pextrb	ebx, xmm0, 7

	pextrb	ebx, xmm0, 0
	pextrb	ebx, xmm1, 1
	pextrb	ebx, xmm2, 2
	pextrb	ebx, xmm3, 3
	pextrb	ebx, xmm3, 4
	pextrb	ebx, xmm2, 5
	pextrb	ebx, xmm1, 6
	pextrb	ebx, xmm0, 7

	pextrb	ebx, xmm1, 0
	pextrb	ebx, xmm2, 1
	pextrb	ebx, xmm3, 2
	pextrb	ebx, xmm1, 13
	pextrb	ebx, xmm2, 14
	pextrb	ebx, xmm3, 15
	pextrb	ebx, xmm0, 6
	pextrb	ebx, xmm0, 7

	pextrb	ebx, xmm0, 10
	pextrb	ebx, xmm1, 11
	pextrb	ebx, xmm2, 12
	pextrb	ebx, xmm3, 13
	pextrb	ebx, xmm3, 14
	pextrb	ebx, xmm2, 15
	pextrb	ebx, xmm1, 6
	pextrb	ebx, xmm0, 7

	pextrb	ebx, xmm1, 0
	pextrb	ebx, xmm2, 1
	pextrb	ebx, xmm3, 2
	pextrb	ebx, xmm1, 3
	pextrb	ebx, xmm2, 4
	pextrb	ebx, xmm3, 5
	pextrb	ebx, xmm0, 6
	pextrb	ebx, xmm0, 7

	pextrb	ebx, xmm0, 0
	pextrb	ebx, xmm1, 1
	pextrb	ebx, xmm2, 2
	pextrb	ebx, xmm3, 3
	pextrb	ebx, xmm3, 4
	pextrb	ebx, xmm2, 5
	pextrb	ebx, xmm1, 6
	pextrb	ebx, xmm0, 7

	dec	eax
	jnz .L1
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		Vector16ToRegister
; Purpose:	Writes 16-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Vector16ToRegister:
_Vector16ToRegister:
	mov	eax, [esp + 4]
	sal	eax, 3  	; Force some repetition.
	push 	ebx
.L1:
	pextrw	ebx, xmm1, 0
	pextrw	ebx, xmm2, 1
	pextrw	ebx, xmm3, 2
	pextrw	ebx, xmm1, 3
	pextrw	ebx, xmm2, 4
	pextrw	ebx, xmm3, 5
	pextrw	ebx, xmm0, 6
	pextrw	ebx, xmm0, 7

	pextrw	ebx, xmm0, 0
	pextrw	ebx, xmm1, 1
	pextrw	ebx, xmm2, 2
	pextrw	ebx, xmm3, 3
	pextrw	ebx, xmm3, 4
	pextrw	ebx, xmm2, 5
	pextrw	ebx, xmm1, 6
	pextrw	ebx, xmm0, 7

	pextrw	ebx, xmm1, 0
	pextrw	ebx, xmm2, 1
	pextrw	ebx, xmm3, 2
	pextrw	ebx, xmm1, 3
	pextrw	ebx, xmm2, 4
	pextrw	ebx, xmm3, 5
	pextrw	ebx, xmm0, 6
	pextrw	ebx, xmm0, 7

	pextrw	ebx, xmm0, 0
	pextrw	ebx, xmm1, 1
	pextrw	ebx, xmm2, 2
	pextrw	ebx, xmm3, 3
	pextrw	ebx, xmm3, 4
	pextrw	ebx, xmm2, 5
	pextrw	ebx, xmm1, 6
	pextrw	ebx, xmm0, 7

	pextrw	ebx, xmm1, 0
	pextrw	ebx, xmm2, 1
	pextrw	ebx, xmm3, 2
	pextrw	ebx, xmm1, 3
	pextrw	ebx, xmm2, 4
	pextrw	ebx, xmm3, 5
	pextrw	ebx, xmm0, 6
	pextrw	ebx, xmm0, 7

	pextrw	ebx, xmm0, 0
	pextrw	ebx, xmm1, 1
	pextrw	ebx, xmm2, 2
	pextrw	ebx, xmm3, 3
	pextrw	ebx, xmm3, 4
	pextrw	ebx, xmm2, 5
	pextrw	ebx, xmm1, 6
	pextrw	ebx, xmm0, 7

	pextrw	ebx, xmm1, 0
	pextrw	ebx, xmm2, 1
	pextrw	ebx, xmm3, 2
	pextrw	ebx, xmm1, 3
	pextrw	ebx, xmm2, 4
	pextrw	ebx, xmm3, 5
	pextrw	ebx, xmm0, 6
	pextrw	ebx, xmm0, 7

	pextrw	ebx, xmm0, 0
	pextrw	ebx, xmm1, 1
	pextrw	ebx, xmm2, 2
	pextrw	ebx, xmm3, 3
	pextrw	ebx, xmm3, 4
	pextrw	ebx, xmm2, 5
	pextrw	ebx, xmm1, 6
	pextrw	ebx, xmm0, 7

	dec	eax
	jnz .L1
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		Vector32ToRegister
; Purpose:	Writes 32-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
	align 64
Vector32ToRegister:
_Vector32ToRegister:
	mov	eax, [esp + 4]
	sal	eax, 2  	; Force some repetition.
	push 	ebx
.L1:
	pextrd	ebx, xmm1, 0
	pextrd	ebx, xmm2, 1
	pextrd	ebx, xmm3, 2
	pextrd	ebx, xmm1, 3
	pextrd	ebx, xmm2, 0
	pextrd	ebx, xmm3, 1
	pextrd	ebx, xmm0, 2
	pextrd	ebx, xmm0, 3

	pextrd	ebx, xmm0, 0
	pextrd	ebx, xmm1, 1
	pextrd	ebx, xmm2, 2
	pextrd	ebx, xmm3, 3
	pextrd	ebx, xmm3, 3
	pextrd	ebx, xmm2, 2
	pextrd	ebx, xmm1, 1
	pextrd	ebx, xmm0, 0

	pextrd	ebx, xmm1, 0
	pextrd	ebx, xmm2, 1
	pextrd	ebx, xmm3, 2
	pextrd	ebx, xmm1, 3
	pextrd	ebx, xmm2, 0
	pextrd	ebx, xmm3, 1
	pextrd	ebx, xmm0, 2
	pextrd	ebx, xmm0, 3

	pextrd	ebx, xmm0, 0
	pextrd	ebx, xmm1, 1
	pextrd	ebx, xmm2, 2
	pextrd	ebx, xmm3, 3
	pextrd	ebx, xmm3, 3
	pextrd	ebx, xmm2, 2
	pextrd	ebx, xmm1, 1
	pextrd	ebx, xmm0, 0

	dec	eax
	jnz .L1
	pop	ebx
	ret

;------------------------------------------------------------------------------
; Name:		Vector64ToRegister
; Purpose:	Writes 64-bit vector register values into main register.
; Params:	rdi = loops
;------------------------------------------------------------------------------
Vector64ToRegister:
_Vector64ToRegister:
	ret

;------------------------------------------------------------------------------
; Name:		CopyAVX
; Purpose:	Copies memory chunks that are 32-byte aligned.
; Params:	[esp + 4]	= ptr to destination memory area
;		[esp + 8]	= ptr to source memory area
; 		[esp + 12]	= length in bytes
; 		[esp + 16]	= loops
;------------------------------------------------------------------------------
	align 64
CopyAVX:
_CopyAVX:
	vzeroupper
	; Register usage:
	; esi = source
	; edi = dest
	; ecx = loops
	; edx = length
	push	esi
	push	edi
	push	ecx
	push	edx

	mov	edi, [esp + 4 + 16]
	mov	esi, [esp + 8 + 16]
	mov	edx, [esp + 12 + 16]
	mov	ecx, [esp + 16 + 16]

	shr	edx, 8	; Ensure length is multiple of 256.
	shl	edx, 8

.L1:
	mov	eax, edx

.L2:
	vmovdqa	ymm0, [esi]
	vmovdqa	ymm1, [32+esi]
	vmovdqa	ymm2, [64+esi]
	vmovdqa	ymm3, [96+esi]

	vmovdqa	[edi], ymm0
	vmovdqa	[32+edi], ymm1
	vmovdqa	[64+edi], ymm2
	vmovdqa	[96+edi], ymm3

	vmovdqa	ymm0, [128+esi]
	vmovdqa	ymm1, [128+32+esi]
	vmovdqa	ymm2, [128+64+esi]
	vmovdqa	ymm3, [128+96+esi]

	vmovdqa	[128+edi], ymm0
	vmovdqa	[128+32+edi], ymm1
	vmovdqa	[128+64+edi], ymm2
	vmovdqa	[128+96+edi], ymm3

	add	esi, 256
	add	edi, 256

	sub	eax, 256
	jnz	.L2

	sub	esi, edx	; rsi now points to start.
	sub	edi, edx	; rdi now points to start.

	dec	ecx
	jnz	.L1

	pop	edx
	pop	ecx
	pop	edi
	pop	esi
	ret

;------------------------------------------------------------------------------
; Name:		CopySSE
; Purpose:	Copies memory chunks that are 16-byte aligned.
; Params:	[esp + 4]	= ptr to destination memory area
;		[esp + 8]	= ptr to source memory area
; 		[esp + 12]	= length in bytes
; 		[esp + 16]	= loops
;------------------------------------------------------------------------------
	align 64
CopySSE:
_CopySSE:
	; Register usage:
	; esi = source
	; edi = dest
	; ecx = loops
	; edx = length
	push	esi
	push	edi
	push	ecx
	push	edx

	mov	edi, [esp + 4 + 16]
	mov	esi, [esp + 8 + 16]
	mov	edx, [esp + 12 + 16]
	mov	ecx, [esp + 16 + 16]

	shr	edx, 7	; Ensure length is multiple of 128.
	shl	edx, 7

	; Save our non-parameter XMM registers.
	sub	esp, 64
	movdqu	[esp], xmm4
	movdqu	[16+esp], xmm5
	movdqu	[32+esp], xmm6
	movdqu	[48+esp], xmm7

.L1:
	mov	eax, edx

.L2:
	movdqa	xmm0, [esi]
	movdqa	xmm1, [16+esi]
	movdqa	xmm2, [32+esi]
	movdqa	xmm3, [48+esi]
	movdqa	xmm4, [64+esi]
	movdqa	xmm5, [80+esi]
	movdqa	xmm6, [96+esi]
	movdqa	xmm7, [112+esi]

	; 32-bit lacks xmm8 - xmm15.

	movdqa	[edi], xmm0
	movdqa	[16+edi], xmm1
	movdqa	[32+edi], xmm2
	movdqa	[48+edi], xmm3
	movdqa	[64+edi], xmm4
	movdqa	[80+edi], xmm5
	movdqa	[96+edi], xmm6
	movdqa	[112+edi], xmm7

	add	esi, 128
	add	edi, 128

	sub	eax, 128
	jnz	.L2

	sub	esi, edx	; rsi now points to start.
	sub	edi, edx	; rdi now points to start.

	dec	ecx
	jnz	.L1

	movdqu	xmm4, [0+esp]
	movdqu	xmm5, [16+esp]
	movdqu	xmm6, [32+esp]
	movdqu	xmm7, [48+esp]
	add	esp, 64

	pop	edx
	pop	ecx
	pop	edi
	pop	esi
	ret

;------------------------------------------------------------------------------
; Name:		CopySSE_128bytes
; Purpose:	Copies memory chunks that are 16-byte aligned.
; Params:	[esp + 4]	= ptr to destination memory area
;		[esp + 8]	= ptr to source memory area
; 		[esp + 12]	= length in bytes
; 		[esp + 16]	= loops
;------------------------------------------------------------------------------
	align 64
CopySSE_128bytes:
_CopySSE_128bytes:
	jmp	CopySSE

