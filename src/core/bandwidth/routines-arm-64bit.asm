#============================================================================
# bandwidth, a benchmark to estimate memory transfer bandwidth.
#
# 64-bit ARM (aarch64) routines.
#
# Copyright (C) 2010-2023 by Zack T Smith.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# The author may be reached at 1 at zsmith.co.
#=============================================================================

# Version 0.11 for Raspberry Pi and Apple Silicon

.arch armv8-a

# For Linux:
.global Writer
.global WriterVector
.global RandomWriter
.global RandomWriterVector
.global Reader
.global ReaderVector
.global RandomReader
.global RandomReaderVector
.global RegisterToRegister
.global VectorToVector128
.global StackReader
.global StackWriter
.global IncrementRegisters
.global IncrementStack
.global CopyWithMainRegisters
.global CopyWithVector128Registers
.global Reader_nontemporal
.global Writer_nontemporal

# For MacOS:
.global _Writer
.global _WriterVector
.global _RandomWriter
.global _RandomWriterVector
.global _Reader
.global _ReaderVector
.global _RandomReader
.global _RandomReaderVector
.global _RegisterToRegister
.global _VectorToVector128
.global _StackReader
.global _StackWriter
.global _IncrementRegisters
.global _IncrementStack
.global _CopyWithMainRegisters
.global _CopyWithVector128Registers
.global _Reader_nontemporal
.global _Writer_nontemporal

.text

#-----------------------------------------------------------------------------
# Name: 	Writer
# Purpose:	Performs sequential write into memory, as fast as possible.
# Params:
#	x0 = address, 16-byte aligned
#	x1 = length, multiple of 256
#	x2 = count
# 	x3 = value to write
#-----------------------------------------------------------------------------
.align 4
Writer:
_Writer:
	stp	x5, x6, [sp, -16]!

	lsr	x1, x1, #4
	lsl	x1, x1, #4
	
	mov	x4, x0
	mov	x5, x1
	mov	x6, x3

.L0:
	mov	x0, x4
	mov	x1, x5

.L1:
	## Do 16 transfers, 16 bytes each = 256 bytes total.
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16

	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16

	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16

	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16
	stp	x3, x6, [x0], 16

	subs	x1, x1, 256
	bne	.L1

	subs	x2, x2, 1
	bne	.L0

	ldp	x5, x6, [sp], 16

	dsb	st
	ret

#-----------------------------------------------------------------------------
# Name: 	Writer_nontemporal
# Purpose:	Performs sequential write into memory, w/nontemporal hint.
# Params:
#	x0 = address, 16-byte aligned
#	x1 = length, multiple of 256
#	x2 = count
# 	x3 = value to write
#-----------------------------------------------------------------------------
.align 4
Writer_nontemporal:
_Writer_nontemporal:
	stp	x5, x6, [sp, -16]!

	lsr	x1, x1, #4
	lsl	x1, x1, #4
	
	mov	x4, x0
	mov	x5, x1
	mov	x6, x3

.Lstnp0:
	mov	x0, x4
	mov	x1, x5

.Lstnp1:
	## Do 16 transfers, 16 bytes each = 256 bytes total.
	stnp	x3, x6, [x0]
	stnp	x3, x6, [x0, #16]
	stnp	x3, x6, [x0, #32]
	stnp	x3, x6, [x0, #48]

	stnp	x3, x6, [x0, #64]
	stnp	x3, x6, [x0, #80]
	stnp	x3, x6, [x0, #96]
	stnp	x3, x6, [x0, #112]

	stnp	x3, x6, [x0, #128]
	stnp	x3, x6, [x0, #144]
	stnp	x3, x6, [x0, #160]
	stnp	x3, x6, [x0, #176]

	stnp	x3, x6, [x0, #192]
	stnp	x3, x6, [x0, #208]
	stnp	x3, x6, [x0, #224]
	stnp	x3, x6, [x0, #240]

	add	x0, x0, #256

	subs	x1, x1, 256
	bne	.Lstnp1

	subs	x2, x2, 1
	bne	.Lstnp0

	ldp	x5, x6, [sp], 16

	dsb	st
	ret

#-----------------------------------------------------------------------------
# Name: 	WriterVector
# Purpose:	Performs sequential write into memory, as fast as possible.
# Params:
#	x0 = address
#	x1 = length, multiple of 256
#	x2 = count
# 	x3 = value to write
#-----------------------------------------------------------------------------
.align 4
WriterVector:
_WriterVector:
	lsr	x1, x1, 5
	lsl	x1, x1, 5

	mov	x4, x0
	mov	x5, x1

.L0v:
	mov	x0, x4
	mov	x1, x5

.L1v:
	## 16 transfers, 16 bytes each = 256 bytes total.
	stp	q0, q1, [x0], 32
	stp	q2, q3, [x0], 32
	stp	q4, q5, [x0], 32
	stp	q6, q7, [x0], 32
	
	stp	q0, q1, [x0], 32
	stp	q2, q3, [x0], 32
	stp	q4, q5, [x0], 32
	stp	q6, q7, [x0], 32
	
	subs	x1, x1, 256
	bne	.L1v

	subs	x2, x2, 1
	bne	.L0v

	dsb	st
	ret

#-----------------------------------------------------------------------------
# Name: 	Reader
# Purpose:	Performs sequential reads from memory, as fast as possible.
# Params:
#	x0 = address
#	x1 = length, multiple of 256
#	x2 = count
#-----------------------------------------------------------------------------
.align 4
Reader:
_Reader:
	stp	x1, x2, [sp, -16]!
	stp	x3, x4, [sp, -16]!
	stp	x5, x6, [sp, -16]!
	stp	x7, x8, [sp, -16]!
	stp	x9, x10, [sp, -16]!
	stp	x11, x12, [sp, -16]!

# x3 = temp

	lsr	x1, x1, 5
	lsl	x1, x1, 5

	mov	x4, x0
	mov	x5, x1

.L2:
	mov	x0, x4
	mov	x1, x5

.L3:
	## 32 transfers, 8 bytes each = 256 bytes total.
	ldp	x3, x6, [x0], 16
	ldp	x7, x8, [x0], 16
	ldp	x9, x10, [x0], 16
	ldp	x11, x12, [x0], 16

	ldp	x7, x10, [x0], 16
	ldp	x3, x6, [x0], 16
	ldp	x11, x12, [x0], 16
	ldp	x9, x8, [x0], 16

	ldp	x11, x3, [x0], 16
	ldp	x8, x10, [x0], 16
	ldp	x7, x9, [x0], 16
	ldp	x3, x6, [x0], 16

	ldp	x8, x6, [x0], 16
	ldp	x9, x11, [x0], 16
	ldp	x7, x3, [x0], 16
	ldp	x10, x12, [x0], 16

	subs	x1, x1, #256
	bne	.L3

	subs	x2, x2, #1
	bne	.L2

	ldp	x12, x11, [sp], 16
	ldp	x10, x9, [sp], 16
	ldp	x8, x7, [sp], 16
	ldp	x6, x5, [sp], 16
	ldp	x4, x3, [sp], 16
	ldp	x2, x1, [sp], 16

	dsb	ld
	ret

#-----------------------------------------------------------------------------
# Name: 	Reader_nontemporal
# Purpose:	Performs sequential reads from memory, with nontemporal hint.
# Params:
#	x0 = address
#	x1 = length, multiple of 256
#	x2 = count
#-----------------------------------------------------------------------------
.align 4
Reader_nontemporal:
_Reader_nontemporal:
	stp	x1, x2, [sp, -16]!
	stp	x3, x4, [sp, -16]!
	stp	x5, x6, [sp, -16]!
	stp	x7, x8, [sp, -16]!
	stp	x9, x10, [sp, -16]!
	stp	x11, x12, [sp, -16]!

# x3 = temp

	lsr	x1, x1, 5
	lsl	x1, x1, 5

	mov	x4, x0
	mov	x5, x1

.Lldnp2:
	mov	x0, x4
	mov	x1, x5

.Lldnp3:
	## 32 transfers, 8 bytes each = 256 bytes total.
	ldnp	x3, x6, [x0]
	ldnp	x7, x8, [x0, #16]
	ldnp	x9, x10, [x0, #32]
	ldnp	x11, x12, [x0, #48]

	ldnp	x7, x10, [x0, #64]
	ldnp	x3, x6, [x0, #80]
	ldnp	x11, x12, [x0, #96]
	ldnp	x9, x8, [x0, #112]

	ldnp	x11, x3, [x0, #128]
	ldnp	x8, x10, [x0, #144]
	ldnp	x7, x9, [x0, #160]
	ldnp	x3, x6, [x0, #176]

	ldnp	x8, x6, [x0, #192]
	ldnp	x9, x11, [x0, #208]
	ldnp	x7, x3, [x0, #224]
	ldnp	x10, x12, [x0, #240]

	add	x0, x0, #256

	subs	x1, x1, #256
	bne	.Lldnp3

	subs	x2, x2, #1
	bne	.Lldnp2

	ldp	x12, x11, [sp], 16
	ldp	x10, x9, [sp], 16
	ldp	x8, x7, [sp], 16
	ldp	x6, x5, [sp], 16
	ldp	x4, x3, [sp], 16
	ldp	x2, x1, [sp], 16

	dsb	ld
	ret

#-----------------------------------------------------------------------------
# Name: 	ReaderVector
# Purpose:	Performs sequential reads from memory, as fast as possible.
# Params:
#	x0 = address
#	x1 = length, multiple of 256
#	x2 = count
#-----------------------------------------------------------------------------
.align 4
ReaderVector:
_ReaderVector:
	stp	x5, x6, [sp, -16]!

	lsr	x1, x1, 5
	lsl	x1, x1, 5

	mov	x4, x0
	mov	x5, x1

.L2v:
	mov	x0, x4
	mov	x1, x5

.L3v:
	## 16 transfers, 16 bytes each = 256 bytes total.
	ldp	q0, q1, [x0], 32
	ldp	q2, q3, [x0], 32
	ldp	q4, q5, [x0], 32
	ldp	q6, q7, [x0], 32
	
	ldp	q0, q1, [x0], 32
	ldp	q2, q3, [x0], 32
	ldp	q4, q5, [x0], 32
	ldp	q6, q7, [x0], 32
	
	subs	x1, x1, 256
	bne	.L3v

	subs	x2, x2, 1
	bne	.L2v

	ldp	x5, x6, [sp], 16

	dsb	ld
	ret

#-----------------------------------------------------------------------------
# Name: 	RandomWriter
# Purpose:	Performs random write into memory, as fast as possible.
# Params:
# 	x0 = pointer to array of chunk pointers
# 	x1 = # of 256-byte chunks
# 	x2 = # loops to do
# 	x3 = value to write
#-----------------------------------------------------------------------------
.align 4
RandomWriter:
_RandomWriter:

.L4:
	mov	x5, xzr

.L5:
	## Get pointer to chunk in memory.
	ldr	x4, [x0, x5, lsl 3]

	## 32 transfers, 8 bytes each = 256 bytes total.
	str	x3, [x4, 160]
	str	x3, [x4, 224]
	str	x3, [x4, 232]
	str	x3, [x4, 96]
	str	x3, [x4, 248]
	str	x3, [x4, 104]
	str	x3, [x4, 136]
	str	x3, [x4, 112]
	str	x3, [x4, 200]
	str	x3, [x4, 128]
	str	x3, [x4, 216]
	str	x3, [x4]
	str	x3, [x4, 184]
	str	x3, [x4, 48]
	str	x3, [x4, 64]
	str	x3, [x4, 240]
	str	x3, [x4, 24]
	str	x3, [x4, 72]
	str	x3, [x4, 32]
	str	x3, [x4, 80]
	str	x3, [x4, 56]
	str	x3, [x4, 8]
	str	x3, [x4, 208]
	str	x3, [x4, 40]
	str	x3, [x4, 120]
	str	x3, [x4, 176]
	str	x3, [x4, 16]
	str	x3, [x4, 168]
	str	x3, [x4, 88]
	str	x3, [x4, 152]
	str	x3, [x4, 192]
	str	x3, [x4, 144]

	add	x5, x5, 1
	cmp	x5, x1
	bne	.L5

	subs	x2, x2, 1
	bne	.L4

	dsb	st
	ret

#-----------------------------------------------------------------------------
# Name: 	RandomWriterVector
# Purpose:	Performs random write into memory, as fast as possible.
# Params:
# 	x0 = pointer to array of chunk pointers
# 	x1 = # of 256-byte chunks
# 	x2 = # loops to do
# 	x3 = value to write
#-----------------------------------------------------------------------------
.align 4
RandomWriterVector:
_RandomWriterVector:
	stp	x4, x5, [sp, -16]!

.L4v:
	mov	x5, xzr

.L5v:
	## Get pointer to chunk in memory.
	ldr	x4, [x0, x5, lsl 3]

	## Does 16 transfers, 16 bytes each = 256 bytes total.
	str	q0, [x4, 144]
	str	q0, [x4, 48]
	str	q0, [x4, 240]
	str	q0, [x4, 16]
	str	q0, [x4, 192]
	str	q0, [x4, 80]
	str	q0, [x4, 176]
	str	q0, [x4, 64]
	str	q0, [x4, 224]
	str	q0, [x4, 32]
	str	q0, [x4, 128]
	str	q0, [x4]
	str	q0, [x4, 160]
	str	q0, [x4, 96]
	str	q0, [x4, 208]
	str	q0, [x4, 112]

	add	x5, x5, 1
	cmp	x5, x1
	bne	.L5v

	subs	x2, x2, 1
	bne	.L4v

	ldp	x4, x5, [sp], 16

	dsb	st
	ret

#-----------------------------------------------------------------------------
# Name: 	RandomReader
# Purpose:	Performs random reads from memory, as fast as possible.
# Params:
# 	x0 = pointer to array of chunk pointers
# 	x1 = # of 256-byte chunks
# 	x2 = # loops to do
#-----------------------------------------------------------------------------
.align 4
RandomReader:
_RandomReader:
	stp	x4, x5, [sp, -16]!

.L6:
	mov	x5, xzr

.L7:
	# Get pointer to chunk in memory.
	ldr	x4, [x0, x5, lsl 3]

	## Does 32 transfers, 8 bytes each = 256 bytes total.
	ldr	x3, [x4, 160]
	ldr	x3, [x4, 224]
	ldr	x3, [x4, 232]
	ldr	x3, [x4, 96]
	ldr	x3, [x4, 248]
	ldr	x3, [x4, 104]
	ldr	x3, [x4, 136]
	ldr	x3, [x4, 112]
	ldr	x3, [x4, 200]
	ldr	x3, [x4, 128]
	ldr	x3, [x4, 216]
	ldr	x3, [x4]
	ldr	x3, [x4, 184]
	ldr	x3, [x4, 48]
	ldr	x3, [x4, 64]
	ldr	x3, [x4, 240]
	ldr	x3, [x4, 24]
	ldr	x3, [x4, 72]
	ldr	x3, [x4, 32]
	ldr	x3, [x4, 80]
	ldr	x3, [x4, 56]
	ldr	x3, [x4, 8]
	ldr	x3, [x4, 208]
	ldr	x3, [x4, 40]
	ldr	x3, [x4, 120]
	ldr	x3, [x4, 176]
	ldr	x3, [x4, 16]
	ldr	x3, [x4, 168]
	ldr	x3, [x4, 88]
	ldr	x3, [x4, 152]
	ldr	x3, [x4, 192]
	ldr	x3, [x4, 144]

	add	x5, x5, 1
	cmp	x5, x1
	bne	.L7

	subs	x2, x2, 1
	bne	.L6

	ldp	x4, x5, [sp], 16

	dsb	ld
	ret

#-----------------------------------------------------------------------------
# Name: 	RandomReaderVector
# Purpose:	Performs random reads from memory, as fast as possible.
# Params:
# 	x0 = pointer to array of chunk pointers
# 	x1 = # of 256-byte chunks
# 	x2 = # loops to do
#-----------------------------------------------------------------------------
.align 4
RandomReaderVector:
_RandomReaderVector:
	stp	x4, x5, [sp, -16]!
	str	q0, [sp, -16]!

.L6v:
	mov	x5, xzr

.L7v:
	# Get pointer to chunk in memory.
	ldr	x4, [x0, x5, lsl 3]

	## Does 16 transfers, 16 bytes each = 256 bytes total.
	ldr	q0, [x4, 144]
	ldr	q0, [x4, 48]
	ldr	q0, [x4, 240]
	ldr	q0, [x4, 16]
	ldr	q0, [x4, 192]
	ldr	q0, [x4, 80]
	ldr	q0, [x4, 176]
	ldr	q0, [x4, 64]
	ldr	q0, [x4, 224]
	ldr	q0, [x4, 32]
	ldr	q0, [x4, 128]
	ldr	q0, [x4]
	ldr	q0, [x4, 160]
	ldr	q0, [x4, 96]
	ldr	q0, [x4, 208]
	ldr	q0, [x4, 112]

	add	x5, x5, 1
	cmp	x5, x1
	bne	.L7v

	subs	x2, x2, 1
	bne	.L6v

	ldr	q0, [sp], 16
	ldp	x4, x5, [sp], 16

	dsb	ld
	ret

#-----------------------------------------------------------------------------
# Name: 	RegisterToRegister
# Purpose:	Performs register-to-register transfers.
# Params:
#	x0 = count
#-----------------------------------------------------------------------------
.align 4
RegisterToRegister:
_RegisterToRegister:

.L8:
	# Do 64 transfers 
	mov	x1, x2
	mov	x1, x3
	mov	x1, x4
	mov	x1, x5
	mov	x1, x6
	mov	x1, x7
	mov	x1, x8
	mov	x1, x9
	mov	x2, x1
	mov	x2, x3
	mov	x2, x4
	mov	x2, x5
	mov	x2, x6
	mov	x2, x7
	mov	x2, x8
	mov	x2, x9
	mov	x1, x2
	mov	x1, x3
	mov	x1, x4
	mov	x1, x5
	mov	x1, x6
	mov	x1, x7
	mov	x1, x8
	mov	x1, x9
	mov	x1, x2
	mov	x1, x3
	mov	x1, x4
	mov	x1, x5
	mov	x1, x6
	mov	x1, x7
	mov	x1, x8
	mov	x1, x9

	mov	x1, x2
	mov	x1, x3
	mov	x1, x4
	mov	x1, x5
	mov	x1, x6
	mov	x1, x7
	mov	x1, x8
	mov	x1, x9
	mov	x2, x1
	mov	x2, x3
	mov	x2, x4
	mov	x2, x5
	mov	x2, x6
	mov	x2, x7
	mov	x2, x8
	mov	x2, x9
	mov	x1, x2
	mov	x1, x3
	mov	x1, x4
	mov	x1, x5
	mov	x1, x6
	mov	x1, x7
	mov	x1, x8
	mov	x1, x9
	mov	x1, x2
	mov	x1, x3
	mov	x1, x4
	mov	x1, x5
	mov	x1, x6
	mov	x1, x7
	mov	x1, x8
	mov	x1, x9

	subs	x0, x0, 1
	bne	.L8

	ret

#-----------------------------------------------------------------------------
# Name: 	VectorToVector128
# Purpose:	Performs register-to-register transfers.
# Params:
#	x0 = count
#-----------------------------------------------------------------------------
.align 4
VectorToVector128:
_VectorToVector128:

# x1 = temp

.L8v:
	# Do 32 transfers (16 bytes each)
	mov	v0.16b, v1.16b
	mov	v7.16b, v2.16b
	mov	v3.16b, v6.16b
	mov	v4.16b, v0.16b
	mov	v8.16b, v9.16b
	mov	v2.16b, v4.16b
	mov	v10.16b, v1.16b
	mov	v1.16b, v2.16b

	mov	v5.16b, v3.16b
	mov	v7.16b, v2.16b
	mov	v3.16b, v6.16b
	mov	v4.16b, v5.16b
	mov	v8.16b, v9.16b
	mov	v2.16b, v4.16b
	mov	v30.16b, v3.16b
	mov	v3.16b, v2.16b

	mov	v0.16b, v1.16b
	mov	v7.16b, v2.16b
	mov	v3.16b, v6.16b
	mov	v4.16b, v0.16b
	mov	v8.16b, v9.16b
	mov	v2.16b, v4.16b
	mov	v10.16b, v1.16b
	mov	v1.16b, v2.16b

	mov	v5.16b, v3.16b
	mov	v7.16b, v2.16b
	mov	v3.16b, v6.16b
	mov	v4.16b, v5.16b
	mov	v8.16b, v9.16b
	mov	v2.16b, v4.16b
	mov	v30.16b, v3.16b
	mov	v3.16b, v2.16b

	subs	x0, x0, 1
	bne	.L8v

	ret

#-----------------------------------------------------------------------------
# Name: 	StackReader
# Purpose:	Performs stack-to-register transfers.
# Params:
#	x0 = count
#-----------------------------------------------------------------------------
.align 4
StackReader:
_StackReader:

	sub	sp, sp, 64

.L9:
	# 64 transfers
	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	ldr	x1, [sp]
	ldr	x1, [sp, 8]
	ldr	x1, [sp, 16]
	ldr	x1, [sp, 24]
	ldr	x1, [sp, 32]
	ldr	x1, [sp, 40]
	ldr	x1, [sp, 48]
	ldr	x1, [sp, 56]

	subs	x0, x0, #1
	bne	.L9

	add	sp, sp, 64

	dsb	ld
	ret

#-----------------------------------------------------------------------------
# Name: 	StackWriter
# Purpose:	Performs register-to-stack transfers.
# Params:
#	x0 = count
#-----------------------------------------------------------------------------
.align 4
StackWriter:
_StackWriter:

	sub	sp, sp, 64

.L10:
	# Do 64 transfers
	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	str	x1, [sp]
	str	x1, [sp, 8]
	str	x1, [sp, 16]
	str	x1, [sp, 24]
	str	x1, [sp, 32]
	str	x1, [sp, 40]
	str	x1, [sp, 48]
	str	x1, [sp, 56]

	subs	x0, x0, 1
	bne	.L10

	add	sp, sp, 64

	dsb	st
	ret

#------------------------------------------------------------------------------
# Name:		IncrementRegisters
# Purpose:	Increments/decrements 64-bit values in registers.
# Params:	r0 = count
#------------------------------------------------------------------------------
.align 4
IncrementRegisters:
_IncrementRegisters:
	
.Li1:
	# 32 operations
	# Note, Rpi4 CPU can do two operations per cycle.
	add	x1, x1, 1
	add	x2, x2, 1
	add	x3, x3, 1
	add	x4, x4, 1
	add	x5, x5, 1
	add	x6, x6, 1
	add	x7, x7, 1
	add	x8, x8, 1

	sub	x1, x1, 1
	sub	x2, x2, 1
	sub	x3, x3, 1
	sub	x4, x4, 1
	sub	x5, x5, 1
	sub	x6, x6, 1
	sub	x7, x7, 1
	sub	x8, x8, 1

	add	x1, x1, 1
	add	x2, x2, 1
	add	x3, x3, 1
	add	x4, x4, 1
	add	x5, x5, 1
	add	x6, x6, 1
	add	x7, x7, 1
	add	x8, x8, 1

	sub	x1, x1, 1
	sub	x2, x2, 1
	sub	x3, x3, 1
	sub	x4, x4, 1
	sub	x5, x5, 1
	sub	x6, x6, 1
	sub	x7, x7, 1
	sub	x8, x8, 1

	subs	x0, x0, 1
	bne	.Li1

	ret

#------------------------------------------------------------------------------
# Name:		IncrementStack
# Purpose:	Increments 64-bit values on stack.
# Params:	r0 = count
#------------------------------------------------------------------------------
.align 4
IncrementStack:
_IncrementStack:
	sub	sp, sp, 64

.Lis1:
	# Perform 32 increments
	ldr	x1, [sp]
	ldr	x2, [sp, 8]
	ldr	x3, [sp, 16]
	ldr	x4, [sp, 24]
	ldr	x5, [sp, 32]
	ldr	x6, [sp, 40]
	ldr	x7, [sp, 48]
	ldr	x8, [sp, 56]
	add	x1, x1, 1
	add	x2, x2, 1
	add	x3, x3, 1
	add	x4, x4, 1
	add	x5, x5, 1
	add	x6, x6, 1
	add	x7, x7, 1
	add	x8, x8, 1
	str	x1, [sp]
	str	x2, [sp, 8]
	str	x3, [sp, 16]
	str	x4, [sp, 24]
	str	x5, [sp, 32]
	str	x6, [sp, 40]
	str	x7, [sp, 48]
	str	x8, [sp, 56]

	ldr	x1, [sp]
	ldr	x2, [sp, 8]
	ldr	x3, [sp, 16]
	ldr	x4, [sp, 24]
	ldr	x5, [sp, 32]
	ldr	x6, [sp, 40]
	ldr	x7, [sp, 48]
	ldr	x8, [sp, 56]
	add	x1, x1, 1
	add	x2, x2, 1
	add	x3, x3, 1
	add	x4, x4, 1
	add	x5, x5, 1
	add	x6, x6, 1
	add	x7, x7, 1
	add	x8, x8, 1
	str	x1, [sp]
	str	x2, [sp, 8]
	str	x3, [sp, 16]
	str	x4, [sp, 24]
	str	x5, [sp, 32]
	str	x6, [sp, 40]
	str	x7, [sp, 48]
	str	x8, [sp, 56]

	ldr	x1, [sp]
	ldr	x2, [sp, 8]
	ldr	x3, [sp, 16]
	ldr	x4, [sp, 24]
	ldr	x5, [sp, 32]
	ldr	x6, [sp, 40]
	ldr	x7, [sp, 48]
	ldr	x8, [sp, 56]
	add	x1, x1, 1
	add	x2, x2, 1
	add	x3, x3, 1
	add	x4, x4, 1
	add	x5, x5, 1
	add	x6, x6, 1
	add	x7, x7, 1
	add	x8, x8, 1
	str	x1, [sp]
	str	x2, [sp, 8]
	str	x3, [sp, 16]
	str	x4, [sp, 24]
	str	x5, [sp, 32]
	str	x6, [sp, 40]
	str	x7, [sp, 48]
	str	x8, [sp, 56]

	ldr	x1, [sp]
	ldr	x2, [sp, 8]
	ldr	x3, [sp, 16]
	ldr	x4, [sp, 24]
	ldr	x5, [sp, 32]
	ldr	x6, [sp, 40]
	ldr	x7, [sp, 48]
	ldr	x8, [sp, 56]
	add	x1, x1, 1
	add	x2, x2, 1
	add	x3, x3, 1
	add	x4, x4, 1
	add	x5, x5, 1
	add	x6, x6, 1
	add	x7, x7, 1
	add	x8, x8, 1
	str	x1, [sp]
	str	x2, [sp, 8]
	str	x3, [sp, 16]
	str	x4, [sp, 24]
	str	x5, [sp, 32]
	str	x6, [sp, 40]
	str	x7, [sp, 48]
	str	x8, [sp, 56]

	subs	x0, x0, 1
	bne	.Lis1

	add	sp, sp, 64

	dsb	st
	ret

#-----------------------------------------------------------------------------
# Name: 	CopyWithMainRegisters
# Purpose:	Performs memory copy, as fast as possible.
# Params:
# 	x0 = pointer to destination array 
# 	x1 = pointer to source array
# 	x2 = # of bytes (multiple of 256)
# 	x3 = # loops to do
#-----------------------------------------------------------------------------
.align 4
CopyWithMainRegisters:
_CopyWithMainRegisters:
	mov	x12, x0
	mov	x13, x1
	mov	x4, x2

.Lcr0:
	mov	x0, x12
	mov	x1, x13
	mov	x2, x4

.Lcr1:
	ldp	x5, x6, [x1], 16
	ldp	x7, x8, [x1], 16
	ldp	x9, x11, [x1], 16
	ldp	x14, x15, [x1], 16

	stp	x5, x6, [x0], 16
	stp	x7, x8, [x0], 16
	stp	x9, x11, [x0], 16
	stp	x14, x15, [x0], 16

	ldp	x5, x6, [x1], 16
	ldp	x7, x8, [x1], 16
	ldp	x9, x11, [x1], 16
	ldp	x14, x15, [x1], 16

	stp	x5, x6, [x0], 16
	stp	x7, x8, [x0], 16
	stp	x9, x11, [x0], 16
	stp	x14, x15, [x0], 16

	subs	x2, x2, 128
	bne	.Lcr1

	subs	x3, x3, 1
	bne	.Lcr0

	dsb	st
	ret

#-----------------------------------------------------------------------------
# Name: 	CopyWithVector128Registers
# Purpose:	Performs memory copy, as fast as possible.
# Params:
# 	x0 = pointer to destination array 
# 	x1 = pointer to source array
# 	x2 = # of bytes (multiple of 256)
# 	x3 = # loops to do
#-----------------------------------------------------------------------------
.align 4
CopyWithVector128Registers:
_CopyWithVector128Registers:
	mov	x12, x0
	mov	x13, x1
	mov	x4, x2

.Lcv128a:
	mov	x0, x12
	mov	x1, x13
	mov	x2, x4

.Lcv128b:
	ldp	q0, q1, [x1], 32
	ldp	q2, q3, [x1], 32
	ldp	q4, q5, [x1], 32
	ldp	q6, q7, [x1], 32

	stp	q0, q1, [x0], 32
	stp	q2, q3, [x0], 32
	stp	q4, q5, [x0], 32
	stp	q6, q7, [x0], 32

	ldp	q0, q1, [x1], 32
	ldp	q2, q3, [x1], 32
	ldp	q4, q5, [x1], 32
	ldp	q6, q7, [x1], 32

	stp	q0, q1, [x0], 32
	stp	q2, q3, [x0], 32
	stp	q4, q5, [x0], 32
	stp	q6, q7, [x0], 32

	subs	x2, x2, 256
	bne	.Lcv128b

	subs	x3, x3, 1
	bne	.Lcv128a

	dsb	st
	ret

