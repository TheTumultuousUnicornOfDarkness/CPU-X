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
#include <time.h>

#include "defs.h"
#include "ObjectOriented.h"
#include "Object.h"
#include "OOC/DateTime.h"
#include "TestingARM.h"
#include "Console.h"
#include "routines.h"

TestingARMClass* _TestingARMClass = NULL;

extern Console* console;

//============================================================================
// Tests.
//============================================================================

static void TestingARM_destroy (Any* self)
{
	if (!self)
		return;
	verifyCorrectClass(self,TestingARM);
}

static void TestingARM_describe (TestingARM* self, FILE *outputFile)
{
        if (!self)
                return;
        verifyCorrectClass(self,TestingARM);

        if (!outputFile)
                outputFile = stdout;

        fprintf (outputFile, "%s", $(self,className));
}

//----------------------------------------------------------------------------
// Name:	TestingARM_write
// Purpose:	Performs write on chunk of memory of specified size.
//----------------------------------------------------------------------------
long TestingARM_write (TestingARM *self, unsigned long size, TestingMode mode, bool random)
{
	if (size == CHECK_WHETHER_SUPPORTED) {
		switch (mode) {
		case SIZE_MAIN_REGISTER:
		case SIZE_VECTOR_128:
			return TEST_SUPPORTED;

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
#ifdef IS_64BIT
			return !random ? TEST_SUPPORTED : TEST_UNSUPPORTED;
#else
			return TEST_UNSUPPORTED;
#endif

		default:
			return TEST_UNSUPPORTED;
		}
	}

	unsigned char *chunk;
	unsigned char *chunk0;
	unsigned long loops;
	unsigned long long total_count=0;
#ifdef IS_64BIT
	unsigned long value = 0x1234567689abcdef;
#else
	unsigned long value = 0x12345678;
#endif
	unsigned long diff=0, t0;
	unsigned long **chunk_ptrs = NULL;

	if (size & 255) {
		error (__FUNCTION__, "Chunk size is not multiple of 256.");
	}

	//-------------------------------------------------
	chunk0 = malloc (size+128);
	if (!chunk0)
		error (__FUNCTION__, "Out of memory");

	chunk = chunk0;
	unsigned long tmp = (unsigned long) chunk;
	if (tmp & 31) {
		tmp -= (tmp & 31);
		tmp += 32;
		chunk = (unsigned char*) tmp;
	}

	//----------------------------------------
	// Set up random pointers to chunks.
	//
	if (random) {
		unsigned long nChunks = size/256;
		chunk_ptrs = (unsigned long**) malloc (sizeof (unsigned long*) * nChunks);
		if (!chunk_ptrs)
			error (__FUNCTION__, "Out of memory.");

		//-----------------------------------------
		// Store pointers to all chunks in an array.
		//
		int i;
		for (i = 0; i < nChunks; i++) {
			chunk_ptrs [i] = (unsigned long*) (((char*)chunk) + 256 * i);
		}

		//----------------------------------------
		// Randomize the array of chunk pointers.
		//
		int k = N_RANDOMIZATION_LOOPS;
		while (k--) {
			for (i = 0; i < nChunks; i++) {
				int j = rand() % nChunks;
				if (i != j) {
					unsigned long *ptr = chunk_ptrs [i];
					chunk_ptrs [i] = chunk_ptrs [j];
					chunk_ptrs [j] = ptr;
				}
			}
		}
	}

	//-------------------------------------------------
	if (random)
		$(console, printf, "Random write ");
	else
		$(console, printf, "Sequential write ");

	switch (mode) {
	case SIZE_MAIN_REGISTER:
#ifdef IS_64BIT
		$(console, printf, "(64-bit), size = ");
#else
		$(console, printf, "(32-bit), size = ");
#endif
		break;
	case SIZE_VECTOR_128:
		$(console, printf, "(128-bit), size = ");
		break;

	case SIZE_MAIN_REGISTER_NONTEMPORAL:
#ifdef IS_64BIT
		$(console, printf, "nontemporal (64-bit), size = ");
#else
		$(console, printf, "nontemporal (32-bit), size = ");
#endif
		break;

	default:
		break;
	}

	$(self, printSize, size);
	$(console, printf, ", ");

	loops = (1 << 26) / size;
	if (loops < 1)
		loops = 1;

	t0 = DateTime_getMicrosecondTime ();

	while (diff < usec_per_test) {
		total_count += loops;

		switch (mode) {
		case SIZE_MAIN_REGISTER:
			if (random)
				RandomWriter (chunk_ptrs, size/256, loops, value);
			else {
				Writer (chunk, size, loops, value);
			}
			break;

		case SIZE_VECTOR_128:
			if (random)
				RandomWriterVector (chunk_ptrs, size/256, loops, value);
			else
                        	WriterVector (chunk, size, loops, value);
			break;

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
			if (!random) {
				Writer_nontemporal (chunk, size, loops, value);
			}
			break;

		default:
			break;
		}

		diff = DateTime_getMicrosecondTime () - t0;
	}

	$(console, printf, "loops = ");
	$(console, printUnsigned, total_count);
	$(console, printf, ", ");

	$(console, flush);

	int result = $(self, calculateResult, size, total_count, diff);
	$(console, flush);

	free ((void*)chunk0);

	if (chunk_ptrs)
		free (chunk_ptrs);

	return result;
}

//----------------------------------------------------------------------------
// Name:	TestingARM_read
// Purpose:	Performs sequential read on chunk of memory of specified size.
//----------------------------------------------------------------------------
long TestingARM_read (TestingARM *self, unsigned long size, TestingMode mode, bool random)
{
	if (size == CHECK_WHETHER_SUPPORTED) {
		switch (mode) {
		case SIZE_MAIN_REGISTER:
		case SIZE_VECTOR_128:
			return TEST_SUPPORTED;

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
#ifdef IS_64BIT
			return !random ? TEST_SUPPORTED : TEST_UNSUPPORTED;
#else
			return TEST_UNSUPPORTED;
#endif

		default:
			return TEST_UNSUPPORTED;
		}
	}

	unsigned long *chunk;
	unsigned long *chunk0;
	unsigned long **chunk_ptrs = NULL;

	if (size & 255) {
		error (__FUNCTION__, "Chunk size is not multiple of 256.");
	}

	//-------------------------------------------------
	chunk0 = malloc (size+128);
	if (!chunk0) {
		error (__FUNCTION__, "Out of memory");
	}

	chunk = chunk0;
	unsigned long tmp = (unsigned long) chunk;
	if (tmp & 31) {
		tmp -= (tmp & 31);
		tmp += 32;
		chunk = (unsigned long*) tmp;
	}

	// Touch all memory blocks, in case a read from an unwritten block
	// is a no-op for the CPU.
	unsigned long nChunks = size/256;
	char *touchPtr = (char*) chunk;
	for (unsigned long i=0; i < nChunks; i += 16) {
		*touchPtr = 0;
		touchPtr += 4096;
	}

	//----------------------------------------
	// Set up random pointers to chunks.
	//
	if (random) {
		int nChunks = size/256;
		chunk_ptrs = (unsigned long**) malloc (sizeof (unsigned long*) * nChunks);
		if (!chunk_ptrs)
			error (__FUNCTION__, "Out of memory.");

		//----------------------------------------
		// Store pointers to all chunks into array.
		//
		int i;
		for (i = 0; i < nChunks; i++) {
			chunk_ptrs [i] = (unsigned long*) (((char*)chunk) + 256 * i);
		}

		//----------------------------------------
		// Randomize the array of chunk pointers.
		//
		int k = N_RANDOMIZATION_LOOPS;
		while (k--) {
			for (i = 0; i < nChunks; i++) {
				int j = rand() % nChunks;
				if (i != j) {
					unsigned long *ptr = chunk_ptrs [i];
					chunk_ptrs [i] = chunk_ptrs [j];
					chunk_ptrs [j] = ptr;
				}
			}
		}
	}

	//-------------------------------------------------
	if (random)
		$(console, printf, "Random read ");
	else
		$(console, printf, "Sequential read ");

	switch (mode) {
	case SIZE_MAIN_REGISTER:
#ifdef IS_64BIT
		$(console, printf, "(64-bit), size = ");
#else
		$(console, printf, "(32-bit), size = ");
#endif
		break;
	case SIZE_VECTOR_128:
		$(console, printf, "(128-bit), size = ");
		break;

	case SIZE_MAIN_REGISTER_NONTEMPORAL:
#ifdef IS_64BIT
		$(console, printf, "nontemporal (64-bit), size = ");
#else
		$(console, printf, "nontemporal (32-bit), size = ");
#endif
		break;

	default:
		break;
	}

	$(self, printSize, size);
	$(console, printf, ", ");

	$(console, flush);

	uint64_t loops;
	uint64_t total_count = 0;
	uint64_t t0 = DateTime_getMicrosecondTime ();
	uint64_t diff = 0;

	loops = (1 << 26) / size;
	if (loops < 1)
		loops = 1;

	while (diff < usec_per_test) {
		total_count += loops;

		switch (mode) {
		case SIZE_MAIN_REGISTER:
			if (random) {
				RandomReader (chunk_ptrs, size/256, loops);
			} else {
				Reader (chunk, size, loops);
			}
			break;

		case SIZE_VECTOR_128:
			if (random)
				RandomReaderVector (chunk_ptrs, size/256, loops);
			else
				ReaderVector (chunk, size, loops);
			break;

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
			if (!random) {
				Reader_nontemporal (chunk, size, loops);
			}
			break;

		default:
			break;
		}

		diff = DateTime_getMicrosecondTime () - t0;
	}

	$(console, printf, "loops = ");
	$(console, printUnsigned, total_count);
	$(console, printf, ", ");

	int result = $(self, calculateResult, size, total_count, diff);
	$(console, flush);

	free (chunk0);

	if (chunk_ptrs) {
		free (chunk_ptrs);
	}

	return result;
}

//----------------------------------------------------------------------------
// Name:	TestingARM_copy
// Purpose:	Performs sequential memory copy.
//----------------------------------------------------------------------------
long TestingARM_copy (TestingARM *self, unsigned long size, TestingMode mode, bool random)
{
	if (size == CHECK_WHETHER_SUPPORTED) {
		switch (mode) {
		case SIZE_MAIN_REGISTER:
		case SIZE_VECTOR_128:
			return TEST_SUPPORTED;
		default:
			return TEST_UNSUPPORTED;
		}
	}

	//-------------------------------------------------
	unsigned long loops;
	unsigned long long total_count = 0;
	unsigned long t0, diff=0;
	unsigned char *chunk_src;
	unsigned char *chunk_dest;
	unsigned char *chunk_src0;
	unsigned char *chunk_dest0;

	chunk_src0 = malloc (size+64);
	if (!chunk_src0) {
		error (__FUNCTION__, "Out of memory");
	}
	chunk_dest0 = malloc (size+64);
	if (!chunk_dest0) {
		error (__FUNCTION__, "Out of memory");
	}

	chunk_src = chunk_src0;
	chunk_dest = chunk_dest0;

	unsigned long tmp = (unsigned long) chunk_src;
	if (tmp & 31) {
		tmp -= (tmp & 31);
		tmp += 32;
		chunk_src = (unsigned char*) tmp;
	}
	tmp = (unsigned long) chunk_dest;
	if (tmp & 31) {
		tmp -= (tmp & 31);
		tmp += 32;
		chunk_dest = (unsigned char*) tmp;
	}

	ooc_bzero (chunk_src, size);
	ooc_bzero (chunk_dest, size);

	//-------------------------------------------------
	$(console, printf, "Sequential copy ");

	if (mode == SIZE_MAIN_REGISTER) {
#ifdef IS_64BIT
		$(console, printf, "(64-bit), size = ");
#else
		$(console, printf, "(32-bit), size = ");
#endif
	}
	else if (mode == SIZE_VECTOR_128) {
		$(console, printf, "(128-bit), size = ");
	}

	$(self, printSize, size);
	$(console, printf, ", ");
	$(console, flush);

	loops = (1 << 26) / size;
	if (loops < 1) {
		loops = 1;
	}

	t0 = DateTime_getMicrosecondTime ();

	while (diff < usec_per_test) {
		total_count += loops;

		if (mode == SIZE_MAIN_REGISTER ) {
			CopyWithMainRegisters (chunk_dest, chunk_src, size, loops);
		}
		else if (mode == SIZE_VECTOR_128) {
			CopyWithVector128Registers (chunk_dest, chunk_src, size, loops);
		}

		diff = DateTime_getMicrosecondTime () - t0;
	}

	$(console, printf, "loops = %llu, ", total_count);
	$(console, flush);

	int result = $(self, calculateResult, size, total_count, diff);
	$(console, flush);

	free (chunk_src0);
	free (chunk_dest0);

	return result;
}

TestingARMClass* TestingARMClass_init (TestingARMClass *class)
{
	SET_SUPERCLASS(Testing);

	SET_OVERRIDDEN_METHOD_POINTER(TestingARM,describe);
	SET_OVERRIDDEN_METHOD_POINTER(TestingARM,destroy);

        SET_INHERITED_METHOD_POINTER(TestingARM,Object,print);
        SET_INHERITED_METHOD_POINTER(TestingARM,Object,equals);

	SET_METHOD_POINTER(TestingARM,read);
	SET_METHOD_POINTER(TestingARM,write);
	SET_METHOD_POINTER(TestingARM,copy);

	SET_ABSTRACT_METHOD_POINTER(registerToVectorTest);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegisterTest);
	SET_ABSTRACT_METHOD_POINTER(vectorToVectorTest256);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister8);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister16);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister32);
	SET_ABSTRACT_METHOD_POINTER(vectorToRegister64);
	SET_ABSTRACT_METHOD_POINTER(registerToVector8);
	SET_ABSTRACT_METHOD_POINTER(registerToVector16);
	SET_ABSTRACT_METHOD_POINTER(registerToVector32);
	SET_ABSTRACT_METHOD_POINTER(registerToVector64);

        VALIDATE_CLASS_STRUCT(_TestingARMClass);
	return _TestingARMClass;
}

TestingARM *TestingARM_init (TestingARM *self)
{
	ENSURE_CLASS_READY(TestingARM);

        Testing_init ((Testing*) self);

        self->is_a = _TestingARMClass;

        return self;
}
