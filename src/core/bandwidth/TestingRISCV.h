/*============================================================================
  TestingRISCV, an Object-Oriented C benchmark for riscv64.
  Copyright (C) 2023 by Zack T Smith.

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

#ifndef _OOC_TESTING_RISCV_H
#define _OOC_TESTING_RISCV_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "OOC/Object.h"
#include "Testing.h"

#define DECLARE_TESTING_RISCV_INSTANCE_VARS(TYPE_POINTER) 

#define DECLARE_TESTING_RISCV_METHODS(TYPE_POINTER) 

struct testing_riscv;

typedef struct testing_riscv_class {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct testing_riscv*)
        DECLARE_TESTING_METHODS(struct testing_riscv*)
        DECLARE_TESTING_RISCV_METHODS(struct testing_riscv*)
} TestingRISCVClass;

extern TestingRISCVClass *_TestingRISCVClass;
extern TestingRISCVClass *TestingRISCVClass_init (TestingRISCVClass*);

typedef struct testing_riscv {
        TestingRISCVClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct testing_riscv*)
	DECLARE_TESTING_INSTANCE_VARS(struct testing_riscv*)
	DECLARE_TESTING_RISCV_INSTANCE_VARS(struct testing_riscv*)
} TestingRISCV;

extern TestingRISCV *TestingRISCV_new ();
extern TestingRISCV *TestingRISCV_init (TestingRISCV *self);

#endif
