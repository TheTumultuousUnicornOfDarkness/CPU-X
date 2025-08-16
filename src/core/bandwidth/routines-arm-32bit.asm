#============================================================================
# bandwidth, a benchmark to estimate memory transfer bandwidth.
# 32-bit ARM assembly module for Raspberry Pi.
# Copyright (C) 2010, 2016, 2021, 2023 by Zack T Smith.
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
# The author may be reached at 1 at zsmith dot co.
#=============================================================================

# Version 0.10 for the 32-bit Raspberry pi.

# Some instructions may not be supported by the ARM CPU in the Raspberry pi 2.

.section code 

.globl Writer
.globl WriterVector
.globl RandomWriter
.globl RandomWriterVector
.globl Reader
.globl ReaderVector
.globl RandomReader
.globl RandomReaderVector
.globl RegisterToRegister
.globl VectorToVector128
.globl StackReader
.globl StackWriter
.globl IncrementRegisters
.globl IncrementStack
.globl CopyWithMainRegisters
.globl CopyWithVector128Registers
.globl Writer_nontemporal
.globl Reader_nontemporal

.text

#-----------------------------------------------------------------------------
# Name: 	Writer
# Purpose:	Performs sequential write into memory, as fast as possible.
# Params:
#	r0 = address
#	r1 = length, multiple of 256
#	r2 = count
# 	r3 = value to write
#-----------------------------------------------------------------------------
Writer:
	push	{r4, r5, r6, r7, r8, r9, r10, r11, r12}

# r4 = temp
# r5 = temp

	and	r1, #0xffffff80
	mov	r4, r0
	mov	r5, r1

	mov	r6, r3
	mov	r7, r3
	mov	r8, r3
	mov	r9, r3
	mov	r10, r3
	mov	r11, r3
	mov	r12, r3

.Lw0:
	mov	r0, r4
	mov	r1, r5

.Lw1:
# Does 64 transfers, 4 bytes each = 256 bytes total.
# The "stmia" instruction automatically increments r0.
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
        stmia   r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }

	subs	r1, #256
	bne	.Lw1

	subs	r2, #1
	bne	.Lw0

	pop	{r4, r5, r6, r7, r8, r9, r10, r11, r12}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	WriterVector
# Purpose:	Performs sequential write into memory with 128-bit writes.
# Params:
#	r0 = address
#	r1 = length, multiple of 256
#	r2 = count
# 	r3 = value to write
#-----------------------------------------------------------------------------
WriterVector:
	push	{r4, r5}

# r4 = temp
# r5 = temp

	and	r1, #0xffffff80
	mov	r4, r0
	mov	r5, r1

.Lwv0:
	mov	r0, r4
	mov	r1, r5

.Lwv1:
	# This does 16 transfers, 16 bytes each = 256 bytes total.
	vstmia   r0!, {q0,q1,q2,q3,q4,q5,q6,q7}
	vstmia   r0!, {q0,q1,q2,q3,q4,q5,q6,q7}

	subs	r1, #256
	bne	.Lwv1

	subs	r2, #1
	bne	.Lwv0

	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	Reader
# Purpose:	Performs sequential reads from memory, as fast as possible.
# Params:
#	r0 = address
#	r1 = length, multiple of 256
#	r2 = count
#-----------------------------------------------------------------------------
Reader:
	push	{r4, r5, r6, r7, r8, r9, r10, r11, r12}

# r3 = temp

	and	r1, #0xffffff80
	mov	r4, r0
	mov	r5, r1

.Lr0:
	mov	r0, r4
	mov	r1, r5

.Lr1:
# Does 64 transfers, 4 bytes each = 256 bytes total.
# The "ldmia" instruction automatically increments r0.

	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }
	ldmia	r0!, { r3, r6, r7, r8, r9, r10, r11, r12 }

	subs	r1, #256
	bne	.Lr1

	subs	r2, #1
 	bne	.Lr0

	pop	{r4, r5, r6, r7, r8, r9, r10, r11, r12}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	ReaderVector
# Purpose:	Performs sequential reads from memory, as fast as possible.
# Params:
#	r0 = address
#	r1 = length, multiple of 256
#	r2 = count
#-----------------------------------------------------------------------------
ReaderVector:
	push	{r4, r5}
	vpush	{q4, q5, q6, q7}

# r3 = temp

	and	r1, #0xffffff80
	mov	r4, r0
	mov	r5, r1

.Lrv0:
	mov	r0, r4
	mov	r1, r5

.Lrv1:
	# 16 bytes * 16 transfers = 256 bytes
	vldmia	r0!, {q0,q1,q2,q3,q4,q5,q6,q7}
	vldmia	r0!, {q0,q1,q2,q3,q4,q5,q6,q7}

	subs	r1, #256
	bne	.Lrv1

	subs	r2, #1
 	bne	.Lrv0

	vpop	{q4, q5, q6, q7}
	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	RandomWriter
# Purpose:	Performs random write into memory, as fast as possible.
# Params:
# 	r0 = pointer to array of chunk pointers
# 	r1 = # of 256-byte chunks
# 	r2 = # loops to do
# 	r3 = value to write
#-----------------------------------------------------------------------------
RandomWriter:
	push	{r4, r5}

# r4 = temp
# r5 = temp

.Lrw0:
	mov	r5, #0

.Lrw1:
# Get pointer to chunk in memory.
	ldr	r4, [r0, r5, LSL #2]

# Does 64 transfers, 4 bytes each = 256 bytes total.

	str	r3, [r4, #160]
	str	r3, [r4, #232]
	str	r3, [r4, #224]
	str	r3, [r4, #96]
	str	r3, [r4, #164]
	str	r3, [r4, #76]
	str	r3, [r4, #100]
	str	r3, [r4, #220]
	str	r3, [r4, #248]
	str	r3, [r4, #104]
	str	r3, [r4, #4]
	str	r3, [r4, #136]
	str	r3, [r4, #112]
	str	r3, [r4, #200]
	str	r3, [r4, #12]
	str	r3, [r4, #128]
	str	r3, [r4, #148]
	str	r3, [r4, #196]
	str	r3, [r4, #216]
	str	r3, [r4]
	str	r3, [r4, #84]
	str	r3, [r4, #140]
	str	r3, [r4, #204]
	str	r3, [r4, #184]
	str	r3, [r4, #124]
	str	r3, [r4, #48]
	str	r3, [r4, #64]
	str	r3, [r4, #212]
	str	r3, [r4, #240]
	str	r3, [r4, #236]
	str	r3, [r4, #24]
	str	r3, [r4, #252]
	str	r3, [r4, #68]
	str	r3, [r4, #20]
	str	r3, [r4, #72]
	str	r3, [r4, #32]
	str	r3, [r4, #28]
	str	r3, [r4, #52]
	str	r3, [r4, #244]
	str	r3, [r4, #180]
	str	r3, [r4, #80]
	str	r3, [r4, #60]
	str	r3, [r4, #8]
	str	r3, [r4, #56]
	str	r3, [r4, #208]
	str	r3, [r4, #228]
	str	r3, [r4, #40]
	str	r3, [r4, #172]
	str	r3, [r4, #120]
	str	r3, [r4, #176]
	str	r3, [r4, #108]
	str	r3, [r4, #132]
	str	r3, [r4, #16]
	str	r3, [r4, #44]
	str	r3, [r4, #92]
	str	r3, [r4, #168]
	str	r3, [r4, #152]
	str	r3, [r4, #156]
	str	r3, [r4, #188]
	str	r3, [r4, #36]
	str	r3, [r4, #88]
	str	r3, [r4, #116]
	str	r3, [r4, #192]
	str	r3, [r4, #144]

	add	r5, #1
	cmp	r5, r1
	bne	.Lrw1

	subs	r2, #1
	bne	.Lrw0

	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	RandomWriterVector
# Purpose:	Performs random writes into memory at 128 bits per write.
# Params:
# 	r0 = pointer to array of chunk pointers
# 	r1 = # of 256-byte chunks
# 	r2 = # loops to do
# 	r3 = value to write
#-----------------------------------------------------------------------------
RandomWriterVector:
	push	{r4, r5}

# r4 = temp
# r5 = temp

.Lrwv0:
	mov	r5, #0

.Lrwv1:
	# Get pointer to chunk in memory.
	ldr	r4, [r0, r5, LSL #2]

	# Does 16 transfers, 16 bytes each = 256 bytes total.
	add	r4, #48
	vstm	r4, { q0 }
	sub	r4, #48

	add	r4, #128
	vstm	r4, { q0 }
	sub	r4, #128

	add	r4, #16
	vstm	r4, { q0 }
	sub	r4, #16

	add	r4, #208
	vstm	r4, { q0 }
	sub	r4, #208

	add	r4, #80
	vstm	r4, { q0 }
	sub	r4, #80

	vstm	r4, { q0 }

	add	r4, #32
	vstm	r4, { q0 }
	sub	r4, #32

	add	r4, #224
	vstm	r4, { q0 }
	sub	r4, #224

	add	r4, #112
	vstm	r4, { q0 }
	sub	r4, #112

	add	r4, #96
	vstm	r4, { q0 }
	sub	r4, #96

	add	r4, #192
	vstm	r4, { q0 }
	sub	r4, #192

	add	r4, #160
	vstm	r4, { q0 }
	sub	r4, #160

	add	r4, #176
	vstm	r4, { q0 }
	sub	r4, #176

	add	r4, #144
	vstm	r4, { q0 }
	sub	r4, #144

	add	r4, #64
	vstm	r4, { q0 }
	sub	r4, #64

	add	r4, #240
	vstm	r4, { q0 }

	add	r5, #1
	cmp	r5, r1
	bne	.Lrwv1

	subs	r2, #1
	bne	.Lrwv0

	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	RandomWriterVector64
# Purpose:	Performs random writes into memory at 64 bits per write.
# Params:
# 	r0 = pointer to array of chunk pointers
# 	r1 = # of 256-byte chunks
# 	r2 = # loops to do
# 	r3 = value to write
# Note:
#	ARM NEON d vector registers are 64 bit.
#-----------------------------------------------------------------------------
RandomWriterVector64:
	push	{r4, r5}

# r4 = temp
# r5 = temp

.Lrwv64a:
	mov	r5, #0

.Lrwv64b:
	# Get pointer to chunk in memory.
	ldr	r4, [r0, r5, LSL #2]

	# Does 32 transfers, 8 bytes each = 256 bytes total.
	vstr	d0, [r4, #56]
	vstr	d0, [r4, #136]
	vstr	d0, [r4, #200]
	vstr	d0, [r4, #80]
	vstr	d0, [r4, #16]
	vstr	d0, [r4, #64]
	vstr	d0, [r4, #192]
	vstr	d0, [r4, #240]
	vstr	d0, [r4, #24]
	vstr	d0, [r4, #104]
	vstr	d0, [r4, #192]
	vstr	d0, [r4, #168]
	vstr	d0, [r4, #96]
	vstr	d0, [r4, #16]
	vstr	d0, [r4, #152]
	vstr	d0, [r4, #56]
	vstr	d0, [r4, #184]
	vstr	d0, [r4, #136]
	vstr	d0, [r4, #192]
	vstr	d0, [r4, #160]
	vstr	d0, [r4, #120]
	vstr	d0, [r4, #136]
	vstr	d0, [r4, #104]
	vstr	d0, [r4, #40]
	vstr	d0, [r4, #192]
	vstr	d0, [r4, #168]
	vstr	d0, [r4, #88]
	vstr	d0, [r4, #176]
	vstr	d0, [r4, #64]
	vstr	d0, [r4, #56]
	vstr	d0, [r4, #200]
	vstr	d0, [r4, #144]

	add	r5, #1
	cmp	r5, r1
	bne	.Lrwv64b

	subs	r2, #1
	bne	.Lrwv64a

	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	RandomReader
# Purpose:	Performs random reads from memory, as fast as possible.
# Params:
# 	r0 = pointer to array of chunk pointers
# 	r1 = # of 256-byte chunks
# 	r2 = # loops to do
#-----------------------------------------------------------------------------
RandomReader:
	push	{r4, r5}

.Lrr0:
	mov	r5, #0

.Lrr1:
	# Get pointer to chunk in memory.
	ldr	r4, [r0, r5, LSL #2]

	# Do 64 transfers, 4 bytes each = 256 bytes total.
	ldr	r3, [r4, #160]
	ldr	r3, [r4, #232]
	ldr	r3, [r4, #224]
	ldr	r3, [r4, #96]
	ldr	r3, [r4, #164]
	ldr	r3, [r4, #76]
	ldr	r3, [r4, #100]
	ldr	r3, [r4, #220]
	ldr	r3, [r4, #248]
	ldr	r3, [r4, #104]
	ldr	r3, [r4, #4]
	ldr	r3, [r4, #136]
	ldr	r3, [r4, #112]
	ldr	r3, [r4, #200]
	ldr	r3, [r4, #12]
	ldr	r3, [r4, #128]
	ldr	r3, [r4, #148]
	ldr	r3, [r4, #196]
	ldr	r3, [r4, #216]
	ldr	r3, [r4]
	ldr	r3, [r4, #84]
	ldr	r3, [r4, #140]
	ldr	r3, [r4, #204]
	ldr	r3, [r4, #184]
	ldr	r3, [r4, #124]
	ldr	r3, [r4, #48]
	ldr	r3, [r4, #64]
	ldr	r3, [r4, #212]
	ldr	r3, [r4, #240]
	ldr	r3, [r4, #236]
	ldr	r3, [r4, #24]
	ldr	r3, [r4, #252]
	ldr	r3, [r4, #68]
	ldr	r3, [r4, #20]
	ldr	r3, [r4, #72]
	ldr	r3, [r4, #32]
	ldr	r3, [r4, #28]
	ldr	r3, [r4, #52]
	ldr	r3, [r4, #244]
	ldr	r3, [r4, #180]
	ldr	r3, [r4, #80]
	ldr	r3, [r4, #60]
	ldr	r3, [r4, #8]
	ldr	r3, [r4, #56]
	ldr	r3, [r4, #208]
	ldr	r3, [r4, #228]
	ldr	r3, [r4, #40]
	ldr	r3, [r4, #172]
	ldr	r3, [r4, #120]
	ldr	r3, [r4, #176]
	ldr	r3, [r4, #108]
	ldr	r3, [r4, #132]
	ldr	r3, [r4, #16]
	ldr	r3, [r4, #44]
	ldr	r3, [r4, #92]
	ldr	r3, [r4, #168]
	ldr	r3, [r4, #152]
	ldr	r3, [r4, #156]
	ldr	r3, [r4, #188]
	ldr	r3, [r4, #36]
	ldr	r3, [r4, #88]
	ldr	r3, [r4, #116]
	ldr	r3, [r4, #192]
	ldr	r3, [r4, #144]

	add	r5, #1
	cmp	r5, r1
	bne	.Lrr1

	subs	r2, #1
	bne	.Lrr0

	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	RandomReaderVector
# Purpose:	Performs random reads from memory at 128 bits per read.
# Params:
# 	r0 = pointer to array of chunk pointers
# 	r1 = # of 256-byte chunks
# 	r2 = # loops to do
# Note:
#	ARM NEON q registers are 128 bits wide.
#-----------------------------------------------------------------------------
RandomReaderVector:
	push	{r4, r5}
	vpush	{q0}

.Lrrv0:
	mov	r5, #0

.Lrrv1:
	# Get pointer to chunk in memory.
	ldr	r4, [r0, r5, LSL #2]

	# Do 16 transfers, 16 bytes each = 256 bytes total.
	add	r4, #48
	vldm	r4, { q0 }
	sub	r4, #48

	add	r4, #128
	vldm	r4, { q0 }
	sub	r4, #128

	add	r4, #16
	vldm	r4, { q0 }
	sub	r4, #16

	add	r4, #208
	vldm	r4, { q0 }
	sub	r4, #208

	add	r4, #80
	vldm	r4, { q0 }
	sub	r4, #80

	vldm	r4, { q0 }

	add	r4, #32
	vldm	r4, { q0 }
	sub	r4, #32

	add	r4, #224
	vldm	r4, { q0 }
	sub	r4, #224

	add	r4, #112
	vldm	r4, { q0 }
	sub	r4, #112

	add	r4, #96
	vldm	r4, { q0 }
	sub	r4, #96

	add	r4, #192
	vldm	r4, { q0 }
	sub	r4, #192

	add	r4, #160
	vldm	r4, { q0 }
	sub	r4, #160

	add	r4, #176
	vldm	r4, { q0 }
	sub	r4, #176

	add	r4, #144
	vldm	r4, { q0 }
	sub	r4, #144

	add	r4, #64
	vldm	r4, { q0 }
	sub	r4, #64

	add	r4, #240
	vldm	r4, { q0 }

	add	r5, #1
	cmp	r5, r1
	bne	.Lrrv1

	subs	r2, #1
	bne	.Lrrv0

	vpop	{q0}
	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	RandomReaderVector64
# Purpose:	Performs random reads from memory at 64 bits per read.
# Params:
# 	r0 = pointer to array of chunk pointers
# 	r1 = # of 256-byte chunks
# 	r2 = # loops to do
# Note:
#	ARM NEON d registers are 64 bits wide.
#-----------------------------------------------------------------------------
RandomReaderVector64:
	push	{r4, r5}
	vpush	{q0}

.Lrrv64a:
	mov	r5, #0

.Lrrv64b:
	# Get pointer to chunk in memory.
	ldr	r4, [r0, r5, LSL #2]

	# Does 32 transfers, 8 bytes each = 256 bytes total.
	vldr	d0, [r4, #56]
	vldr	d0, [r4, #136]
	vldr	d0, [r4, #200]
	vldr	d0, [r4, #80]
	vldr	d0, [r4, #16]
	vldr	d0, [r4, #64]
	vldr	d0, [r4, #192]
	vldr	d0, [r4, #240]
	vldr	d0, [r4, #24]
	vldr	d0, [r4, #104]
	vldr	d0, [r4, #192]
	vldr	d0, [r4, #168]
	vldr	d0, [r4, #96]
	vldr	d0, [r4, #16]
	vldr	d0, [r4, #152]
	vldr	d0, [r4, #56]
	vldr	d0, [r4, #184]
	vldr	d0, [r4, #136]
	vldr	d0, [r4, #192]
	vldr	d0, [r4, #160]
	vldr	d0, [r4, #120]
	vldr	d0, [r4, #136]
	vldr	d0, [r4, #104]
	vldr	d0, [r4, #40]
	vldr	d0, [r4, #192]
	vldr	d0, [r4, #168]
	vldr	d0, [r4, #88]
	vldr	d0, [r4, #176]
	vldr	d0, [r4, #64]
	vldr	d0, [r4, #56]
	vldr	d0, [r4, #200]
	vldr	d0, [r4, #144]

	add	r5, #1
	cmp	r5, r1
	bne	.Lrrv64b

	subs	r2, #1
	bne	.Lrrv64a

	vpop	{q0}
	pop	{r4, r5}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	VectorToVector128
# Purpose:	Performs vector register to vector register transfers.
# Params:
#	r0 = count
# Note:
#	ARM NEON q registers are 128 bits wide.
#-----------------------------------------------------------------------------
VectorToVector128:
	vpush	{q0-q7}

# r1 = temp

.Lvv128:
	# 32 transfers
	vmov	q1, q2
	vmov	q2, q3
	vmov	q3, q4
	vmov	q1, q5
	vmov	q5, q6
	vmov	q6, q1
	vmov	q5, q7
	vmov	q7, q1
	vmov	q1, q0
	vmov	q7, q1
	vmov	q1, q2
	vmov	q2, q3
	vmov	q4, q6
	vmov	q5, q2
	vmov	q6, q7
	vmov	q1, q3
	vmov	q1, q2
	vmov	q2, q3
	vmov	q3, q4
	vmov	q1, q5
	vmov	q5, q6
	vmov	q6, q1
	vmov	q5, q7
	vmov	q7, q1
	vmov	q1, q0
	vmov	q7, q1
	vmov	q1, q2
	vmov	q2, q3
	vmov	q4, q6
	vmov	q5, q2
	vmov	q6, q7
	vmov	q1, q3

	subs	r0, #1
	bne	.Lvv128

	vpop	{q0-q7}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	StackReader
# Purpose:	Performs stack-to-register transfers.
# Params:
#	r0 = count
#-----------------------------------------------------------------------------
StackReader:

# r1 = temp

	sub	sp, #32
.Lsr:
	# 64 transfers, 4 bytes each = 256 bytes total.

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	ldr	r1, [sp]
	ldr	r1, [sp, #4]
	ldr	r1, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r1, [sp, #16]
	ldr	r1, [sp, #20]
	ldr	r1, [sp, #24]
	ldr	r1, [sp, #28]

	subs	r0, #1
	bne	.Lsr

	add	sp, #32
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	StackWriter
# Purpose:	Performs register-to-stack transfers.
# Params:
#	r0 = count
#-----------------------------------------------------------------------------
StackWriter:
	sub	sp, #32

.Lsw:
	# 64 transfers, 4 bytes each = 256 bytes total.

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	str	r1, [sp]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	str	r1, [sp, #12]
	str	r1, [sp, #16]
	str	r1, [sp, #20]
	str	r1, [sp, #24]
	str	r1, [sp, #28]

	subs	r0, #1
	bne	.Lsw

	add	sp, #32

	mov	pc, lr

#------------------------------------------------------------------------------
# Name:		RegisterToRegister
# Purpose:	Copies 32-bit values between 32-bit registers.
# Params:	r0 = count
#------------------------------------------------------------------------------

RegisterToRegister:
	push	{r1-r8}
	
.Lr2r:
	# 64 moves
	mov	r1, r8
	mov	r2, r7
	mov	r3, r6
	mov	r4, r5
	mov	r5, r4
	mov	r6, r3
	mov	r7, r2
	mov	r8, r1

	mov	r1, r4
	mov	r4, r5
	mov	r5, r1
	mov	r2, r3
	mov	r3, r6
	mov	r6, r2
	mov	r7, r8
	mov	r8, r0

	mov	r1, r8
	mov	r2, r7
	mov	r3, r6
	mov	r4, r5
	mov	r5, r4
	mov	r6, r3
	mov	r7, r2
	mov	r8, r1

	mov	r1, r4
	mov	r4, r5
	mov	r5, r1
	mov	r2, r3
	mov	r3, r6
	mov	r6, r2
	mov	r7, r8
	mov	r8, r0

	mov	r1, r8
	mov	r2, r7
	mov	r3, r6
	mov	r4, r5
	mov	r5, r4
	mov	r6, r3
	mov	r7, r2
	mov	r8, r1

	mov	r1, r4
	mov	r4, r5
	mov	r5, r1
	mov	r2, r3
	mov	r3, r6
	mov	r6, r2
	mov	r7, r8
	mov	r8, r0

	mov	r1, r8
	mov	r2, r7
	mov	r3, r6
	mov	r4, r5
	mov	r5, r4
	mov	r6, r3
	mov	r7, r2
	mov	r8, r1

	mov	r1, r4
	mov	r4, r5
	mov	r5, r1
	mov	r2, r3
	mov	r3, r6
	mov	r6, r2
	mov	r7, r8
	mov	r8, r0

	subs	r0, #1
	bne	.Lr2r

	pop	{r1-r8}
	mov	pc, lr

#------------------------------------------------------------------------------
# Name:		IncrementRegisters
# Purpose:	Increments 32-bit values in registers.
# Params:	r0 = count
#------------------------------------------------------------------------------

IncrementRegisters:
	
.Lir:
	# 32 operations
	add	r1, #1
	add	r2, #1
	add	r3, #1
	add	r4, #1
	add	r5, #1
	add	r6, #1
	add	r7, #1
	add	r8, #1

	sub	r1, #1
	sub	r2, #1
	sub	r3, #1
	sub	r4, #1
	sub	r5, #1
	sub	r6, #1
	sub	r7, #1
	sub	r8, #1

	add	r1, #1
	add	r2, #1
	add	r3, #1
	add	r4, #1
	add	r5, #1
	add	r6, #1
	add	r7, #1
	add	r8, #1

	sub	r1, #1
	sub	r2, #1
	sub	r3, #1
	sub	r4, #1
	sub	r5, #1
	sub	r6, #1
	sub	r7, #1
	sub	r8, #1

	subs	r0, #1
	bne	.Lir

	mov	pc, lr

#------------------------------------------------------------------------------
# Name:		IncrementStack
# Purpose:	Increments 32-bit values on stack.
# Params:	r0 = count
#------------------------------------------------------------------------------

IncrementStack:
	push	{r1, r2, r3, r4, r5, r6, r7, r8}

	sub	sp, #64

.Lis:
	# 32 increment operations
	ldr	r1, [sp]
	ldr	r2, [sp, #4]
	add	r1, #1
	add	r2, #1
	ldr	r3, [sp, #8]
	ldr	r4, [sp, #12]
	add	r3, #1
	add	r4, #1
	ldr	r5, [sp, #16]
	ldr	r6, [sp, #20]
	add	r5, #1
	add	r6, #1
	ldr	r7, [sp, #24]
	ldr	r8, [sp, #28]
	add	r7, #1
	add	r8, #1
	str	r1, [sp]
	str	r2, [sp, #4]
	str	r3, [sp, #8]
	str	r4, [sp, #12]
	str	r5, [sp, #16]
	str	r6, [sp, #20]
	str	r7, [sp, #24]
	str	r8, [sp, #28]

	ldr	r1, [sp]
	ldr	r2, [sp, #4]
	add	r1, #1
	add	r2, #1
	ldr	r3, [sp, #8]
	ldr	r4, [sp, #12]
	add	r3, #1
	add	r4, #1
	ldr	r5, [sp, #16]
	ldr	r6, [sp, #20]
	add	r5, #1
	add	r6, #1
	ldr	r7, [sp, #24]
	ldr	r8, [sp, #28]
	add	r7, #1
	add	r8, #1
	str	r1, [sp]
	str	r2, [sp, #4]
	str	r3, [sp, #8]
	str	r4, [sp, #12]
	str	r5, [sp, #16]
	str	r6, [sp, #20]
	str	r7, [sp, #24]
	str	r8, [sp, #28]

	ldr	r1, [sp]
	ldr	r2, [sp, #4]
	add	r1, #1
	add	r2, #1
	ldr	r3, [sp, #8]
	ldr	r4, [sp, #12]
	add	r3, #1
	add	r4, #1
	ldr	r5, [sp, #16]
	ldr	r6, [sp, #20]
	add	r5, #1
	add	r6, #1
	ldr	r7, [sp, #24]
	ldr	r8, [sp, #28]
	add	r7, #1
	add	r8, #1
	str	r1, [sp]
	str	r2, [sp, #4]
	str	r3, [sp, #8]
	str	r4, [sp, #12]
	str	r5, [sp, #16]
	str	r6, [sp, #20]
	str	r7, [sp, #24]
	str	r8, [sp, #28]

	ldr	r1, [sp]
	ldr	r2, [sp, #4]
	add	r1, #1
	add	r2, #1
	ldr	r3, [sp, #8]
	ldr	r4, [sp, #12]
	add	r3, #1
	add	r4, #1
	ldr	r5, [sp, #16]
	ldr	r6, [sp, #20]
	add	r5, #1
	add	r6, #1
	ldr	r7, [sp, #24]
	ldr	r8, [sp, #28]
	add	r7, #1
	add	r8, #1
	str	r1, [sp]
	str	r2, [sp, #4]
	str	r3, [sp, #8]
	str	r4, [sp, #12]
	str	r5, [sp, #16]
	str	r6, [sp, #20]
	str	r7, [sp, #24]
	str	r8, [sp, #28]

	subs	r0, #1
	bne	.Lis

	add	sp, #64
	pop	{r1, r2, r3, r4, r5, r6, r7, r8}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	CopyWithMainRegisters
# Purpose:	Performs memory copy, as fast as possible.
# Params:
# 	r0 = pointer to destination array 
# 	r1 = pointer to source array
# 	r2 = # of bytes (multiple of 256)
# 	r3 = # loops to do
#-----------------------------------------------------------------------------
.align 4
CopyWithMainRegisters:
	push	{r4, r5, r6, r7, r8, r9, r10, r11, r12}

	mov	r11, r0
	mov	r12, r1
	mov	r10, r2

.Lcr0:
	mov	r0, r11
	mov	r1, r12
	mov	r2, r10

.Lcr1:
	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 24 bytes
	ldmia	r1!, { r4, r5, r6, r7, r8, r9 }
	stmia	r0!, { r4, r5, r6, r7, r8, r9 }

	# 16 bytes
	ldmia	r1!, { r4, r5, r6, r7 }
	stmia	r0!, { r4, r5, r6, r7 }

	subs	r2, #256
	bne	.Lcr1

	subs	r3, #1
	bne	.Lcr0

	pop	{r4, r5, r6, r7, r8, r9, r10, r11, r12}
	mov	pc, lr

#-----------------------------------------------------------------------------
# Name: 	CopyWithVector128Registers
# Purpose:	Performs memory copy, as fast as possible.
# Params:
# 	r0 = pointer to destination array 
# 	r1 = pointer to source array
# 	r2 = # of bytes (multiple of 256)
# 	r3 = # loops to do
#-----------------------------------------------------------------------------
.align 4
CopyWithVector128Registers:
	vpush	{q0-q7}
	push	{r4, r11, r12}

	mov	r11, r0
	mov	r12, r1
	mov	r4, r2

.Lcv128a:
	mov	r0, r11
	mov	r1, r12
	mov	r2, r4

.Lcv128b:
	vldmia	r1!, {q0,q1,q2,q3,q4,q5,q6,q7}
	vstmia	r0!, {q0,q1,q2,q3,q4,q5,q6,q7}

	vldmia	r1!, {q0,q1,q2,q3,q4,q5,q6,q7}
	vstmia	r0!, {q0,q1,q2,q3,q4,q5,q6,q7}

	subs	r2, #256
	bne	.Lcv128b

	subs	r3, #1
	bne	.Lcv128a

	pop	{r4, r11, r12}
	vpop	{q0-q7}
	mov	pc, lr

Writer_nontemporal:
Reader_nontemporal:
	mov	pc, lr
