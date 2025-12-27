;============================================================================
;  Object-Oriented C (OOC)
;  Copyright (C) 2015-2019, 2023 by Zack T Smith.
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

bits	64
cpu	default

; Note:
; Unix ABI says integer param are put in these registers in this order:
;	rdi, rsi, rdx, rcx, r8, r9

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

global	_get_cpuid1_ecx
global	_get_cpuid1_edx
global	_get_cpuid_80000001_ecx
global	_get_cpuid_80000001_edx
global	_get_cpuid_family1
global	_get_cpuid_family2
global	_get_cpuid_family3
global	_get_cpuid_model
global	_get_cpuid7_ebx
global	_get_cpuid7_ecx
global	_get_cpuid7_edx
global	_get_cpuid_cache_info

global	get_cpuid1_ecx
global	get_cpuid1_edx
global	get_cpuid_80000001_ecx
global	get_cpuid_80000001_edx
global	get_cpuid_family1
global	get_cpuid_family2
global	get_cpuid_family3
global	get_cpuid_model
global	get_cpuid7_ebx
global	get_cpuid7_ecx
global	get_cpuid7_edx
global	get_cpuid_cache_info

%ifidn __OUTPUT_FORMAT__, elf64
	section .note.GNU-stack
%endif

	section .text

;------------------------------------------------------------------------------
; Name:		get_cpuid_family1
;
get_cpuid_family1:
_get_cpuid_family1:
	push	rbx
	push 	rcx
	push 	rdx
	push 	rdi
	xor	eax, eax
	cpuid
	xor	eax, eax
	mov	eax, ebx
	pop	rdi
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_family2
;
get_cpuid_family2:
_get_cpuid_family2:
	push	rbx
	push 	rcx
	push 	rdx
	push 	rdi
	xor	eax, eax
	cpuid
	xor	eax, eax
	mov	eax, edx
	pop	rdi
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_family3
;
get_cpuid_family3:
_get_cpuid_family3:
	push	rbx
	push 	rcx
	push 	rdx
	push 	rdi
	xor	eax, eax
	cpuid
	xor	eax, eax
	mov	eax, ecx
	pop	rdi
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
	push 	rdi
	mov	rax, 7
	xor	ecx, ecx
	cpuid
        mov	rax, rbx
	pop	rdi
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid7_ecx
;
get_cpuid7_ecx:
_get_cpuid7_ecx:
	push	rbx
	push 	rcx
	push 	rdx
	push 	rdi
	mov	rax, 7
	xor	ecx, ecx
	cpuid
        mov	rax, rcx
	pop	rdi
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid7_edx
;
get_cpuid7_edx:
_get_cpuid7_edx:
	push	rbx
	push 	rcx
	push 	rdx
	push 	rdi
	mov	rax, 7
	xor	ecx, ecx
	cpuid
        mov	rax, rdx
	pop	rdi
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
	push 	rdi
	mov	rax, 1
	cpuid
        mov	rax, rdx
	pop	rdi
	pop	rdx
	pop	rcx
	pop	rbx
	ret

;------------------------------------------------------------------------------
; Name:		get_cpuid_model
;
get_cpuid_model:
_get_cpuid_model:
	push	rbx
	push 	rcx
	push 	rdx
	push 	rdi

	; On Windows, this will be mov rdi, rcx.
	; On Linux, this will be mov rdi, rdi.
	mov	rdi, P1

	mov	eax, 0x80000002
	cpuid
	mov	[rdi], eax
	mov	[rdi+4], ebx
	mov	[rdi+8], ecx
	mov	[rdi+12], edx

	mov	eax, 0x80000003
	cpuid
	mov	[rdi+16], eax
	mov	[rdi+20], ebx
	mov	[rdi+24], ecx
	mov	[rdi+28], edx

	mov	eax, 0x80000004
	cpuid
	mov	[rdi+32], eax
	mov	[rdi+36], ebx
	mov	[rdi+40], ecx
	mov	[rdi+48], edx

	pop	rdi
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
	push 	rdi
	mov	rax, 0x80000001
	cpuid
        mov	rax, rdx
	pop	rdi
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
	push 	rdi
	mov	rax, 0x80000001
	cpuid
        mov	rax, rcx
	pop	rdi
	pop	rdx
	pop	rcx
	pop	rbx
	ret

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
