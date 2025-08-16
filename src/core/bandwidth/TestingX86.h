/*============================================================================
  TestingX86, an Object-Oriented C benchmark for i386 and x86_64.
  Copyright (C) 2022, 2023 by Zack T Smith.

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

#ifndef _OOC_TESTING_X86_H
#define _OOC_TESTING_X86_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "OOC/Object.h"
#include "Testing.h"

#define DECLARE_TESTING_X86_INSTANCE_VARS(TYPE_POINTER) \
	bool use_sse2; \
	bool use_sse4; \
	bool use_avx; \
	bool use_avx512; \
	bool use_direct_transfers; 

#define DECLARE_TESTING_X86_METHODS(TYPE_POINTER) 

struct testing_x86;

typedef struct testing_x86_class {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct testing_x86*)
        DECLARE_TESTING_METHODS(struct testing_x86*)
        DECLARE_TESTING_X86_METHODS(struct testing_x86*)
} TestingX86Class;

extern TestingX86Class *_TestingX86Class;
extern TestingX86Class *TestingX86Class_init (TestingX86Class*);

typedef struct testing_x86 {
        TestingX86Class *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct testing_x86*)
	DECLARE_TESTING_INSTANCE_VARS(struct testing_x86*)
	DECLARE_TESTING_X86_INSTANCE_VARS(struct testing_x86*)
} TestingX86;

extern TestingX86 *TestingX86_new ();
extern TestingX86 *TestingX86_init (TestingX86 *self);

#endif
