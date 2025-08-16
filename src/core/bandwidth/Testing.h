/*============================================================================
  Testing, an abstract Object-Oriented C class for benchmarking.
  Copyright (C) 2019 by Zack T Smith.

  "bandwidth" is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  "bandwidth" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this software.  If not, see <http://www.gnu.org/licenses/>.

  The author may be reached at 1 at zsmith dot co.
 *===========================================================================*/

#ifndef _OOC_TESTING_H
#define _OOC_TESTING_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "OOC/Object.h"

#define TEST_UNSUPPORTED (-1)
#define TEST_SUPPORTED (0)
#define CHECK_WHETHER_SUPPORTED (0)

#define N_RANDOMIZATION_LOOPS (2)

#define N_STACK_OPS_PER_LOOP 64LU
#define STACK_OPERATION_LOOPS_OUTER 17LU
#define STACK_OPERATION_LOOPS_INNER 1023LU

#define N_REG_TO_REG_LOOPS 1023LU
#define N_REG_TO_REG_PER_LOOP 64LU
#define REGISTER_TRANSFERS_COUNT 9997LU

#define N_VREG_TO_VREG_LOOPS 1023LU
#define N_VREG_TO_VREG_PER_LOOP 32LU
#define VREGISTER_TRANSFERS_COUNT 9977LU

#define N_VECTOR_INSERTS_EXTRACTS_PER_LOOP 64LU

#define N_INC_OUTER_LOOPS 17LU
#define N_INC_INNER_LOOPS 32771LU
#define N_INC_PER_INNER 32LU

typedef enum {
	SIZE_MAIN_REGISTER,	// 32 or 64 bits
	SIZE_VECTOR_128,	// NEON, SSE
	SIZE_VECTOR_256,	// AVX
	SIZE_VECTOR_512,	// AVX512
	SIZE_MAIN_REGISTER_NONTEMPORAL,	// SSE2, aarch64 LDNP/STNP
	SIZE_VECTOR_128_NONTEMPORAL,	// SSE2/SSE4
	SIZE_VECTOR_256_NONTEMPORAL,	// AVX512
	SIZE_VECTOR_512_NONTEMPORAL,	// AVX512
} TestingMode;

#define DECLARE_TESTING_INSTANCE_VARS(TYPE_POINTER)

#define DECLARE_TESTING_METHODS(TYPE_POINTER) \
	void (*printSize) (TYPE_POINTER, size_t chunk_size); \
	int (*calculateResult) (TYPE_POINTER, uint64_t chunk_size, uint64_t total_loops, uint64_t diff); \
	long (*read) (TYPE_POINTER, unsigned long size, TestingMode mode, bool random); \
	long (*write) (TYPE_POINTER, unsigned long size, TestingMode mode, bool random); \
	long (*copy) (TYPE_POINTER, unsigned long size, TestingMode mode, bool random); \
	long (*registerToRegisterTest) (TYPE_POINTER);\
	long (*registerToVectorTest) (TYPE_POINTER);\
	long (*vectorToRegisterTest) (TYPE_POINTER);\
	long (*vectorToVectorTest128) (TYPE_POINTER);\
	long (*vectorToVectorTest256) (TYPE_POINTER);\
	long (*vectorToVectorTest512) (TYPE_POINTER);\
	long (*vectorToRegister8) (TYPE_POINTER);\
	long (*vectorToRegister16) (TYPE_POINTER);\
	long (*vectorToRegister32) (TYPE_POINTER);\
	long (*vectorToRegister64) (TYPE_POINTER);\
	long (*registerToVector8) (TYPE_POINTER);\
	long (*registerToVector16) (TYPE_POINTER);\
	long (*registerToVector32) (TYPE_POINTER);\
	long (*registerToVector64) (TYPE_POINTER);\
	long (*stackRead) (TYPE_POINTER);\
	long (*stackWrite) (TYPE_POINTER);\
	long (*incrementRegisters) (TYPE_POINTER);\
	long (*incrementStack) (TYPE_POINTER);

struct testing;

typedef struct testingclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct testing*)
        DECLARE_TESTING_METHODS(struct testing*)
} TestingClass;

extern TestingClass *_TestingClass;
extern TestingClass *TestingClass_init (TestingClass*);

typedef struct testing {
        TestingClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct testing*)
	DECLARE_TESTING_INSTANCE_VARS(struct testing*)
} Testing;

extern Testing *Testing_new ();
extern Testing *Testing_init (Testing *self);

#endif
