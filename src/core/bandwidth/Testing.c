/*============================================================================
  bandwidth, a benchmark to estimate memory transfer bandwidth.
  Copyright (C) 2005-2023 by Zack T Smith.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  The author may be reached at 1 at zsmith dot co.
 *===========================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "defs.h"
#include "routines.h"
#include "OOC/ObjectOriented.h"
#include "OOC/Object.h"
#include "OOC/DateTime.h"
#include "Testing.h"
#include "OOC/Console.h"

TestingClass* _TestingClass = NULL;

extern Console* console;

static void Testing_printSize (Testing *self, size_t size)
{
	if (size < 1536) {
		$(console, printInt, size);
		$(console, printf, " B");
	}
	else if (size < (1<<20)) {
		$(console, printInt, size >> 10);
		$(console, printf, " kB");
	} else {
		$(console, printInt, size >> 20);
		switch ((size >> 18) & 3) {
		case 1: $(console, printf, ".25"); break;
		case 2: $(console, printf, ".5"); break;
		case 3: $(console, printf, ".75"); break;
		}
		$(console, printf, " MB");
	}
}

static void Testing_destroy (Any* self)
{
	if (!self)
		return;
	verifyCorrectClassOrSubclass(self,Testing);
}

static void Testing_describe (Testing* self, FILE *outputFile)
{
        if (!self)
                return;
        verifyCorrectClass(self,Testing);

        if (!outputFile)
                outputFile = stdout;

        fprintf (outputFile, "%s", $(self,className));
}

//----------------------------------------------------------------------------
// Name:	calculateResult
// Purpose:	Calculates and prints a result.
// Returns:	10 times the number of megabytes per second.
//----------------------------------------------------------------------------
static int Testing_calculateResult (Testing* self, uint64_t chunk_size, uint64_t total_loops, uint64_t diff)
{
	if (!diff) {
		warning (__FUNCTION__, "Zero time difference... ignoring.");
		return 0;
	}

	long double result = (long double) chunk_size;
	result *= (long double) total_loops;
	result *= 1000000.;
	result /= 1048576.;
	result /= (long double) diff;

	$(console, printf, "%.1Lf MB/s\n", result);

	return (long) (10.0 * result);
}

//============================================================================

static long Testing_registerToRegisterTest (Testing *self)
{
        time_t t0 = DateTime_getMicrosecondTime ();

#ifdef IS_64BIT
	$(console, printf, "64-bit main register to main register transfers: ");
#else
	$(console, printf, "32-bit main register to main register transfers: ");
#endif
	$(console, flush);

	for (int i=0; i < N_REG_TO_REG_LOOPS; i++) {
		RegisterToRegister (REGISTER_TRANSFERS_COUNT);
	}
	long diff = DateTime_getMicrosecondTime () - t0;

	long double d = N_REG_TO_REG_LOOPS;
	d *= REGISTER_TRANSFERS_COUNT;
	d *= N_REG_TO_REG_PER_LOOP;
	d /= diff;
	d *= 1000000; // usec->sec
	d /= 1000000000; // billions/sec
#ifdef IS_64BIT
	$(console, printf, "%.2Lf billion/second\n", d);
#else
	$(console, printf, "%.2Lf billion/second\n", d);
#endif
	$(console, flush);

	return 0;
}

static long Testing_stackRead (Testing *self)
{
        time_t t0 = DateTime_getMicrosecondTime ();

	for (int i=0; i<STACK_OPERATION_LOOPS_OUTER; i++) {
		StackReader (STACK_OPERATION_LOOPS_INNER);
	}
	time_t diff = DateTime_getMicrosecondTime () - t0;
	if (diff > 0) {
		long double d = N_STACK_OPS_PER_LOOP;
		d *= STACK_OPERATION_LOOPS_OUTER;
		d *= STACK_OPERATION_LOOPS_INNER;
		d *= 1000000.; // usec->sec
		d /= diff;
		d /= 1000000000.; // billions/sec
#ifdef IS_64BIT
		$(console, printf, "64-bit stack-to-register transfers: %.2Lf billion/second\n", d);
#else
		$(console, printf, "32-bit stack-to-register transfers: %.2Lf billion/second\n", d);
#endif
	}
	$(console, flush);
	return 0;
}

static long Testing_stackWrite (Testing *self)
{
        time_t t0 = DateTime_getMicrosecondTime ();

	for (int i=0; i<STACK_OPERATION_LOOPS_OUTER; i++) {
		StackWriter (STACK_OPERATION_LOOPS_INNER);
	}
	time_t diff = DateTime_getMicrosecondTime () - t0;
	if (diff > 0) {
		long double d = N_STACK_OPS_PER_LOOP;
		d *= STACK_OPERATION_LOOPS_OUTER;
		d *= STACK_OPERATION_LOOPS_INNER;
		d *= 1000000.; // usec->sec
		d /= diff;
		d /= 1000000000.; // billions/sec
#ifdef IS_64BIT
		$(console, printf, "64-bit register-to-stack transfers: %.2Lf billion/second\n", d);
#else
		$(console, printf, "32-bit register-to-stack transfers: %.2Lf billion/second\n", d);
#endif
	}
	$(console, flush);
	return 0;
}

static long Testing_incrementRegisters (Testing *self)
{
        time_t t0 = DateTime_getMicrosecondTime ();

#ifdef IS_64BIT
	$(console, printf, "64-bit register increments: ");
#else
	$(console, printf, "32-bit register increments: ");
#endif
	$(console, flush);

	int i;
	for (i=0; i<N_INC_OUTER_LOOPS; i++) {
		IncrementRegisters (N_INC_INNER_LOOPS);
	}
	time_t diff = DateTime_getMicrosecondTime () - t0;
	if (diff > 0) {
		unsigned long tmp = N_INC_OUTER_LOOPS;
		tmp *= N_INC_INNER_LOOPS;
		tmp *= N_INC_PER_INNER;
		long double dt = diff;
		dt /= 1000000.;
		long double d = tmp;
		d /= dt;
		d /= 1000000000.; // billions/sec
#ifdef IS_64BIT
		$(console, printf, "%.2Lf billion/second\n", d);
#else
		$(console, printf, "%.2Lf billion/second\n", d);
#endif
	}
	$(console, flush);

	return 0;
}

static long Testing_incrementStack (Testing *self)
{
        time_t t0 = DateTime_getMicrosecondTime ();

#ifdef IS_64BIT
	$(console, printf, "64-bit stack value increments: ");
#else
	$(console, printf, "32-bit stack value increments: ");
#endif
	$(console, flush);

	int i;
	for (i=0; i < N_INC_OUTER_LOOPS; i++) {
		IncrementStack (N_INC_INNER_LOOPS);
	}
	long diff = DateTime_getMicrosecondTime () - t0;

	if (diff > 0) {
		unsigned long tmp = N_INC_OUTER_LOOPS;
		tmp *= N_INC_INNER_LOOPS;
		tmp *= N_INC_PER_INNER;
		long double d = tmp;
		d *= 1000000; // usec->sec
		d /= diff;
		d /= 1000000000; // billions/sec
#ifdef IS_64BIT
		$(console, printf, "%.2Lf billion/second\n", d);
#else
		$(console, printf, "%.2Lf billion/second\n", d);
#endif
	}
	$(console, flush);

	return 0;
}

static long Testing_vectorToVectorTest128 (Testing *self)
{
        time_t t0 = DateTime_getMicrosecondTime ();

	$(console, printf, "Vector register to vector register transfers (128-bit): ");
	$(console, flush);

	int i;
	for (i=0; i < N_VREG_TO_VREG_LOOPS; i++) {
		VectorToVector128 (VREGISTER_TRANSFERS_COUNT);
	}
	long diff = DateTime_getMicrosecondTime () - t0;

	long double d = N_VREG_TO_VREG_LOOPS;
	d *= VREGISTER_TRANSFERS_COUNT;
	d *= N_VREG_TO_VREG_PER_LOOP;
	d /= diff;
	d *= 1000000; // usec->sec
	d /= 1000000000; // billions/sec
	$(console, printf, "%.2Lf billion/second\n", d);
	$(console, flush);

	return 0;
}

TestingClass* TestingClass_init (TestingClass* class)
{
	SET_SUPERCLASS(Object);

	SET_METHOD_POINTER(Testing,calculateResult);
	SET_METHOD_POINTER(Testing,printSize);

	SET_OVERRIDDEN_METHOD_POINTER(Testing,describe);
	SET_OVERRIDDEN_METHOD_POINTER(Testing,destroy);
	SET_OVERRIDDEN_METHOD_POINTER(Testing,stackRead);
	SET_OVERRIDDEN_METHOD_POINTER(Testing,stackWrite);
	SET_OVERRIDDEN_METHOD_POINTER(Testing,incrementRegisters);
	SET_OVERRIDDEN_METHOD_POINTER(Testing,incrementStack);
	SET_OVERRIDDEN_METHOD_POINTER(Testing,registerToRegisterTest);
	SET_OVERRIDDEN_METHOD_POINTER(Testing,vectorToVectorTest128);

	SET_ABSTRACT_METHOD_POINTER(read);
	SET_ABSTRACT_METHOD_POINTER(write);
	SET_ABSTRACT_METHOD_POINTER(copy);
	SET_ABSTRACT_METHOD_POINTER(registerToVectorTest);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegisterTest);
	SET_ABSTRACT_METHOD_POINTER(vectorToVectorTest256);
	SET_ABSTRACT_METHOD_POINTER(vectorToVectorTest512);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister8);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister16);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister32);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister64);
	SET_ABSTRACT_METHOD_POINTER(registerToVector8);
	SET_ABSTRACT_METHOD_POINTER(registerToVector16);
	SET_ABSTRACT_METHOD_POINTER(registerToVector32);
	SET_ABSTRACT_METHOD_POINTER(registerToVector64);

	VALIDATE_CLASS_STRUCT(class);

	return _TestingClass;
}

Testing *Testing_init (Testing *self)
{
	ENSURE_CLASS_READY(Testing);

        Object_init ((Object*) self);

        self->is_a = _TestingClass;

        return self;
}
