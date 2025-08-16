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
#include "OOC/ObjectOriented.h"
#include "OOC/Object.h"
#include "OOC/DateTime.h"
#include "TestingX86.h"
#include "OOC/Console.h"
#include "routines.h"

TestingX86Class* _TestingX86Class = NULL;

extern Console* console;

//============================================================================
// Tests.
//============================================================================

static void TestingX86_destroy (Any* self)
{
	if (!self)
		return;
	verifyCorrectClass(self,TestingX86);
}

static void TestingX86_describe (TestingX86* self, FILE *outputFile)
{
        if (!self)
                return;
        verifyCorrectClass(self,TestingX86);

        if (!outputFile)
                outputFile = stdout;

        fprintf (outputFile, "%s", $(self,className));
}

//----------------------------------------------------------------------------
// Name:	TestingX86_write
// Purpose:	Performs write on chunk of memory of specified size.
//----------------------------------------------------------------------------
long TestingX86_write (TestingX86 *self, unsigned long size, TestingMode mode, bool random)
{
	if (size == CHECK_WHETHER_SUPPORTED) {
#ifdef IS_64BIT
		// x64
		if (!random) {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // Writer
				return TEST_SUPPORTED;

			case SIZE_MAIN_REGISTER_NONTEMPORAL: // Writer_nontemporal
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_128: // WriterSSE2
			case SIZE_VECTOR_128_NONTEMPORAL: // WriterSSE2_nontemporal
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256: // WriterAVX
			case SIZE_VECTOR_256_NONTEMPORAL: // WriterAVX_nontemporal
				return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_512: // WriterAVX512
			case SIZE_VECTOR_512_NONTEMPORAL: // WriterAVX512_nontemporal
				return (!self->use_avx512) ? TEST_UNSUPPORTED : TEST_SUPPORTED;
			}
		} else {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // RandomWriter
				return TEST_SUPPORTED;

			case SIZE_MAIN_REGISTER_NONTEMPORAL:
				return TEST_UNSUPPORTED;

			case SIZE_VECTOR_128: // RandomWriterSSE2
			case SIZE_VECTOR_128_NONTEMPORAL: // RandomWriterSSE2_nontemporal
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256: // RandomWriterAVX
			case SIZE_VECTOR_256_NONTEMPORAL: // RandomWriterAVX_nontemporal
				return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_512:
			case SIZE_VECTOR_512_NONTEMPORAL:
				return TEST_UNSUPPORTED;
			}
		}
#else
		// i386
		if (!random) {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // Writer
				return TEST_SUPPORTED;

			case SIZE_MAIN_REGISTER_NONTEMPORAL:
				return TEST_UNSUPPORTED;

			case SIZE_VECTOR_128:	// WriterSSE2
			case SIZE_VECTOR_128_NONTEMPORAL: // WriterSSE2_nontemporal
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256:	// WriterAVX
			case SIZE_VECTOR_256_NONTEMPORAL: // WriterAVX_nontemporal
				return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_512:
			case SIZE_VECTOR_512_NONTEMPORAL:
				return TEST_UNSUPPORTED;
			}
		} else {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // RandomWriter
			case SIZE_VECTOR_128:	// RandomWriterSSE2
			case SIZE_VECTOR_128_NONTEMPORAL: // RandomWriterSSE2_nontemporal
				return TEST_SUPPORTED;

			case SIZE_MAIN_REGISTER_NONTEMPORAL:
			case SIZE_VECTOR_256:
			case SIZE_VECTOR_256_NONTEMPORAL:
			case SIZE_VECTOR_512:
			case SIZE_VECTOR_512_NONTEMPORAL:
				return TEST_UNSUPPORTED;
			default:
				break;
			}
		}
#endif

		return TEST_SUPPORTED;
	}

	//-------------------------------------------------
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

	chunk0 = malloc (size+128);
	if (!chunk0) {
		error (__FUNCTION__, "Out of memory");
	}

	chunk = chunk0;
	if (mode != SIZE_VECTOR_512 && mode != SIZE_VECTOR_512_NONTEMPORAL) {
		unsigned long tmp = (unsigned long) chunk;
		if (tmp & 31) {
			tmp -= (tmp & 31);
			tmp += 32;
			chunk = (unsigned char*) tmp;
		}
	} else {
		unsigned long tmp = (unsigned long) chunk;
		if (tmp & 63) {
			tmp -= (tmp & 63);
			tmp += 64;
			chunk = (unsigned char*) tmp;
		}
	}

	unsigned long nChunks = size/256;

	//----------------------------------------
	// Set up random pointers to chunks.
	//
	if (random) {
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
	case SIZE_VECTOR_128:
		$(console, printf, "(128-bit), size = ");
		break;
	case SIZE_VECTOR_256:
		$(console, printf, "(256-bit), size = ");
		break;
	case SIZE_VECTOR_512:
		$(console, printf, "(512-bit), size = ");
		break;
	case SIZE_VECTOR_128_NONTEMPORAL:
                $(console, printf, "nontemporal (128-bit), size = ");
		break;
	case SIZE_VECTOR_256_NONTEMPORAL:
                $(console, printf, "nontemporal (256-bit), size = ");
		break;
	case SIZE_VECTOR_512_NONTEMPORAL:
                $(console, printf, "nontemporal (512-bit), size = ");
		break;
	case SIZE_MAIN_REGISTER:
#ifdef IS_64BIT
		$(console, printf, "(64-bit), size = ");
#else
		$(console, printf, "(32-bit), size = ");
#endif
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

	loops = (1 << 26) / size;
	if (loops < 1)
		loops = 1;

	t0 = DateTime_getMicrosecondTime ();

	while (diff < usec_per_test) {
		total_count += loops;

		switch (mode) {
		case SIZE_VECTOR_128:
			if (random)
				RandomWriterSSE2 (chunk_ptrs, size/256, loops, value);
			else {
				WriterSSE2 (chunk, size, loops, value);
			}
			break;

		case SIZE_VECTOR_128_NONTEMPORAL:
			if (random)
				RandomWriterSSE2_nontemporal (chunk_ptrs, size/256, loops, value);
			else {
				WriterSSE2_nontemporal (chunk, size, loops, value);
			}
			break;

		case SIZE_VECTOR_256:
			if (!random) {
				WriterAVX (chunk, size, loops, value);
			} else {
				RandomWriterAVX (chunk_ptrs, size/256, loops, value);
			}
			break;

		case SIZE_VECTOR_512:
			if (!random) {
				WriterAVX512(chunk, size, loops, value);
			}
			break;

		case SIZE_VECTOR_256_NONTEMPORAL:
			if (!random) {
				WriterAVX_nontemporal (chunk, size, loops, value);
			} else {
				RandomWriterAVX_nontemporal (chunk_ptrs, size/256, loops, value);
			}
			break;

		case SIZE_VECTOR_512_NONTEMPORAL:
			if (!random) {
				WriterAVX512_nontemporal (chunk, size, loops, value);
			} else {
				return TEST_UNSUPPORTED;
			}
			break;

		case SIZE_MAIN_REGISTER:
			if (random)
				RandomWriter (chunk_ptrs, size/256, loops, value);
			else {
				Writer (chunk, size, loops, value);
			}
			break;

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
			if (!random) {
				Writer_nontemporal (chunk, size, loops, value);
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
	$(console, flush);

	int result = $(self, calculateResult, size, total_count, diff);
	$(console, flush);

	free ((void*)chunk0);

	if (chunk_ptrs) {
		free (chunk_ptrs);
	}

	return result;
}

//----------------------------------------------------------------------------
// Name:	TestingX86_read
// Purpose:	Performs sequential read on chunk of memory of specified size.
//----------------------------------------------------------------------------
long TestingX86_read (TestingX86 *self, unsigned long size, TestingMode mode, bool random)
{
	if (!self) {
		error_null_parameter(__FUNCTION__);
		return 0;
	}

	if (size == CHECK_WHETHER_SUPPORTED) {
#ifdef IS_64BIT
		// x64
		if (!random) {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // Reader
				return TEST_SUPPORTED;

			case SIZE_MAIN_REGISTER_NONTEMPORAL: // Reader_nontemporal
				return TEST_UNSUPPORTED;

			case SIZE_VECTOR_128: // ReaderSSE2
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_128_NONTEMPORAL: // ReaderSSE4_nontemporal
				return (!self->use_sse4) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256: // ReaderAVX
			case SIZE_VECTOR_256_NONTEMPORAL: // ReaderAVX_nontemporal
				return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_512: // ReaderAVX512
			case SIZE_VECTOR_512_NONTEMPORAL: // ReaderAVX512_nontemporal
				return (!self->use_avx512) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			default:
				return TEST_UNSUPPORTED;
			}
		} else {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // RandomReader
				return TEST_SUPPORTED;

			case SIZE_MAIN_REGISTER_NONTEMPORAL:
				return TEST_UNSUPPORTED;

			case SIZE_VECTOR_128: // RandomReaderSSE2
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;
			case SIZE_VECTOR_128_NONTEMPORAL: // RandomReaderSSE4_nontemporal
				return (!self->use_sse4) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256: // RandomReaderAVX
				return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256_NONTEMPORAL:
			case SIZE_VECTOR_512:
			case SIZE_VECTOR_512_NONTEMPORAL:
				return TEST_UNSUPPORTED;
			}
		}
#else
		// i386
		if (!random) {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // Reader
				return TEST_SUPPORTED;

			case SIZE_MAIN_REGISTER_NONTEMPORAL:
				return TEST_UNSUPPORTED;

			case SIZE_VECTOR_128:	// ReaderSSE2
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;
			case SIZE_VECTOR_128_NONTEMPORAL: // ReaderSSE4_nontemporal
				return (!self->use_sse4) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256:	// ReaderAVX
				return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256_NONTEMPORAL:
			case SIZE_VECTOR_512:
			case SIZE_VECTOR_512_NONTEMPORAL:
				return TEST_UNSUPPORTED;
			}
		} else {
			switch (mode) {
			case SIZE_MAIN_REGISTER: // RandomReader

			case SIZE_MAIN_REGISTER_NONTEMPORAL:
				return TEST_UNSUPPORTED;

			case SIZE_VECTOR_128:	// RandomReaderSSE2
				return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_128_NONTEMPORAL: // RandomReaderSSE4_nontemporal
				return (!self->use_sse4) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

			case SIZE_VECTOR_256:
			case SIZE_VECTOR_256_NONTEMPORAL:
			case SIZE_VECTOR_512:
			case SIZE_VECTOR_512_NONTEMPORAL:
				return TEST_UNSUPPORTED;
			default:
				break;
			}
		}
#endif

		return TEST_SUPPORTED;
	}

	//-------------------------------------------------
	unsigned long long loops;
	unsigned long long total_count = 0;
	unsigned long t0, diff=0;
	unsigned long *chunk;
	unsigned long *chunk0;
	unsigned long **chunk_ptrs = NULL;

	chunk0 = malloc (size+128);
	if (!chunk0) {
		error (__FUNCTION__, "Out of memory");
	}

	chunk = chunk0;
	if (mode != SIZE_VECTOR_512 && mode != SIZE_VECTOR_512_NONTEMPORAL) {
		unsigned long tmp = (unsigned long) chunk;
		if (tmp & 31) {
			tmp -= (tmp & 31);
			tmp += 32;
			chunk = (unsigned long*) tmp;
		}
	} else {
		unsigned long tmp = (unsigned long) chunk;
		if (tmp & 63) {
			tmp -= (tmp & 63);
			tmp += 64;
			chunk = (unsigned long*) tmp;
		}
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
		chunk_ptrs = (unsigned long**) malloc (sizeof (unsigned long*) * nChunks);
		if (!chunk_ptrs) {
			error (__FUNCTION__, "Out of memory.");
		}

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
	case SIZE_VECTOR_128:
		$(console, printf, "(128-bit), size = ");
		break;
	case SIZE_VECTOR_128_NONTEMPORAL:
                $(console, printf, "nontemporal (128-bit), size = ");
		break;
	case SIZE_VECTOR_256:
		$(console, printf, "(256-bit), size = ");
		break;
	case SIZE_VECTOR_256_NONTEMPORAL:
                $(console, printf, "nontemporal (256-bit), size = ");
		break;
	case SIZE_VECTOR_512:
		$(console, printf, "(512-bit), size = ");
		break;
	case SIZE_VECTOR_512_NONTEMPORAL:
                $(console, printf, "nontemporal (512-bit), size = ");
		break;
	case SIZE_MAIN_REGISTER:
#ifdef IS_64BIT
		$(console, printf, "(64-bit), size = ");
#else
		$(console, printf, "(32-bit), size = ");
#endif
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

	loops = (1 << 29) / size;
	if (loops < 1) {
		loops = 1;
	}

	t0 = DateTime_getMicrosecondTime ();

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

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
			if (!random) {
				Reader_nontemporal (chunk, size, loops);
			}
			break;

		case SIZE_VECTOR_128:
			if (random)
				RandomReaderSSE2 (chunk_ptrs, size/256, loops);
			else {
				ReaderSSE2 (chunk, size, loops);
			}
			break;

		case SIZE_VECTOR_128_NONTEMPORAL:
			if (random) {
				RandomReaderSSE4_nontemporal (chunk_ptrs, size/256, loops);
			}
			else {
				ReaderSSE4_nontemporal (chunk, size, loops);
			}
			break;

		case SIZE_VECTOR_256:
			if (!random) {
				ReaderAVX (chunk, size, loops);
			} else {
				RandomReaderAVX (chunk_ptrs, size/256, loops);
			}
			break;

		case SIZE_VECTOR_256_NONTEMPORAL:
			if (!random) {
				ReaderAVX_nontemporal (chunk, size, loops);
			} else {
				return TEST_UNSUPPORTED;
			}
			break;

		case SIZE_VECTOR_512:
			if (!random) {
				ReaderAVX512 (chunk, size, loops);
			} else {
				return TEST_UNSUPPORTED;
			}
			break;

		case SIZE_VECTOR_512_NONTEMPORAL:
			if (!random) {
				ReaderAVX512_nontemporal (chunk, size, loops);
			} else {
				return TEST_UNSUPPORTED;
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

	free (chunk0);

	if (chunk_ptrs) {
		free (chunk_ptrs);
	}

	return result;
}

//----------------------------------------------------------------------------
// Name:	TestingX86_copy
// Purpose:	Performs sequential memory copy.
//----------------------------------------------------------------------------
long TestingX86_copy (TestingX86 *self, unsigned long size, TestingMode mode, bool random)
{
	if (size == CHECK_WHETHER_SUPPORTED) {
#ifdef IS_64BIT
		// x64
		switch (mode) {
		case SIZE_MAIN_REGISTER: // CopyWithMainRegisters
			return TEST_SUPPORTED;

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
			return TEST_UNSUPPORTED;

		case SIZE_VECTOR_128: // CopySSE2
			return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

		case SIZE_VECTOR_256: // CopyAVX
			return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

		case SIZE_VECTOR_512: // CopyAVX512
			return (!self->use_avx512) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

		case SIZE_VECTOR_128_NONTEMPORAL:
		case SIZE_VECTOR_256_NONTEMPORAL:
		case SIZE_VECTOR_512_NONTEMPORAL:
			return TEST_UNSUPPORTED;
		}
#else
		// i386
		switch (mode) {
		case SIZE_MAIN_REGISTER: // CopyWithMainRegisters
			return TEST_SUPPORTED;

		case SIZE_MAIN_REGISTER_NONTEMPORAL:
			return TEST_UNSUPPORTED;

		case SIZE_VECTOR_128:	// CopySSE2
			return (!self->use_sse2) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

		case SIZE_VECTOR_256:	// CopyAVX
			return (!self->use_avx) ? TEST_UNSUPPORTED : TEST_SUPPORTED;

		case SIZE_VECTOR_128_NONTEMPORAL:
		case SIZE_VECTOR_256_NONTEMPORAL:
		case SIZE_VECTOR_512:
		case SIZE_VECTOR_512_NONTEMPORAL:
			return TEST_UNSUPPORTED;
		}
#endif

		return TEST_SUPPORTED;
	}

	unsigned long loops;
	unsigned long long total_count = 0;
	unsigned long t0, diff=0;
	unsigned char *chunk_src;
	unsigned char *chunk_dest;
	unsigned char *chunk_src0;
	unsigned char *chunk_dest0;

	switch (mode) {
	case SIZE_VECTOR_128_NONTEMPORAL:
	case SIZE_VECTOR_256_NONTEMPORAL:
	case SIZE_VECTOR_512_NONTEMPORAL:
		return TEST_UNSUPPORTED;
	default:
		break;
	}

	if (size == CHECK_WHETHER_SUPPORTED) {
		return TEST_SUPPORTED;
	}

	//-------------------------------------------------
	chunk_src0 = malloc (size+128);
	if (!chunk_src0) {
		error (__FUNCTION__, "Out of memory");
	}
	chunk_dest0 = malloc (size+128);
	if (!chunk_dest0) {
		error (__FUNCTION__, "Out of memory");
	}

	chunk_src = chunk_src0;
	chunk_dest = chunk_dest0;
	ooc_bzero (chunk_src, size);
	ooc_bzero (chunk_dest, size);

	// Make sure both memory chunks are 64-byte aligned.
	unsigned long tmp = (unsigned long) chunk_src;
	if (tmp & 63) {
		tmp -= (tmp & 63);
		tmp += 64;
		chunk_src = (unsigned char*) tmp;
	}
	tmp = (unsigned long) chunk_dest;
	if (tmp & 63) {
		tmp -= (tmp & 63);
		tmp += 64;
		chunk_dest = (unsigned char*) tmp;
	}

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
	else if (mode == SIZE_VECTOR_256) {
		$(console, printf, "(256-bit), size = ");
	}
	else if (mode == SIZE_VECTOR_512) {
		$(console, printf, "(512-bit), size = ");
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
			CopySSE (chunk_dest, chunk_src, size, loops);
		}
		else if (mode == SIZE_VECTOR_256) {
			CopyAVX (chunk_dest, chunk_src, size, loops);
		}
		else if (mode == SIZE_VECTOR_512) {
			CopyAVX512 (chunk_dest, chunk_src, size, loops);
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

static long TestingX86_registerToVectorTest (TestingX86 *self)
{
#ifdef IS_64BIT
	$(console, printf, "Main register to vector register transfers (64-bit): ");
#else
	$(console, printf, "Main register to vector register transfers (32-bit): ");
#endif
	$(console, flush);

	long long total_count = 0;
	unsigned long diff = 0;
	unsigned long t0 = DateTime_getMicrosecondTime ();

	while (diff < usec_per_test)
	{
		RegisterToVector (VREGISTER_TRANSFERS_COUNT);
		total_count += VREGISTER_TRANSFERS_COUNT;

		diff = DateTime_getMicrosecondTime () - t0;
	}

	long double d = total_count;
	d *= N_VREG_TO_VREG_PER_LOOP;
	d /= diff;
	d *= 1000000; // usec->sec
	d /= 1000000000; // billions/sec
	$(console, printf, "%.2Lf billion/second\n", d);
	$(console, flush);

	return 0;
}

static long TestingX86_vectorToRegisterTest (TestingX86 *self)
{
#ifdef IS_64BIT
	$(console, printf, "Vector register to main register transfers (64-bit): ");
#else
	$(console, printf, "Vector register to main register transfers (32-bit): ");
#endif
	$(console, flush);

	unsigned long t0 = DateTime_getMicrosecondTime ();
	unsigned long diff = 0;
	long long total_count = 0;

	while (diff < usec_per_test)
	{
		VectorToRegister (VREGISTER_TRANSFERS_COUNT);
		total_count += VREGISTER_TRANSFERS_COUNT;

		diff = DateTime_getMicrosecondTime () - t0;
	}

	long double d = total_count;
	d *= N_VREG_TO_VREG_PER_LOOP;
	d /= diff;
	d *= 1000000; // usec->sec
	d /= 1000000000; // billions/sec
	$(console, printf, "%.2Lf billion/second\n", d);
	$(console, flush);

	return 0;
}

static long TestingX86_vectorToVectorTest256 (TestingX86 *self)
{
	if (self->use_avx) {
		$(console, printf, "Vector register to vector register transfers (256-bit): ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			VectorToVector256 (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VREG_TO_VREG_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

static long TestingX86_vectorToVectorTest512 (TestingX86 *self)
{
	if (self->use_avx512) {
		$(console, printf, "Vector register to vector register transfers (512-bit): ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			VectorToVector512 (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VREG_TO_VREG_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

static long TestingX86_vectorToRegister8 (TestingX86 *self)
{
	if (self->use_sse4) {
		$(console, printf, "Vector 8-bit datum to main register transfers: ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			Vector8ToRegister (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

static long TestingX86_vectorToRegister16 (TestingX86 *self)
{
	$(console, printf, "Vector 16-bit datum to main register transfers: ");
	$(console, flush);

	long long total_count = 0;
	unsigned long diff = 0;
	unsigned long t0 = DateTime_getMicrosecondTime ();

	while (diff < usec_per_test)
	{
		Vector16ToRegister (VREGISTER_TRANSFERS_COUNT);
		total_count += VREGISTER_TRANSFERS_COUNT;

		diff = DateTime_getMicrosecondTime () - t0;
	}

	long double d = total_count;
	d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
	d /= diff;
	d *= 1000000; // usec->sec
	d /= 1000000000; // billions/sec
	$(console, printf, "%.2Lf billion/second\n", d);
	$(console, flush);
	return 0;
}

static long TestingX86_vectorToRegister32 (TestingX86 *self)
{
	if (self->use_sse4) {
		$(console, printf, "Vector 32-bit datum to main register transfers: ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			Vector32ToRegister (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

static long TestingX86_vectorToRegister64 (TestingX86 *self)
{
	if (self->use_sse4) {
		$(console, printf, "Vector 64-bit datum to main register transfers: ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			Vector64ToRegister (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

static long TestingX86_registerToVector8 (TestingX86 *self)
{
	if (self->use_sse4) {
		$(console, printf, "Main register 8-bit datum to vector register transfers: ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			Register8ToVector (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

static long TestingX86_registerToVector16 (TestingX86 *self)
{
	$(console, printf, "Main register 16-bit datum to vector register transfers: ");
	$(console, flush);

	long long total_count = 0;
	unsigned long diff = 0;
	unsigned long t0 = DateTime_getMicrosecondTime ();

	while (diff < usec_per_test)
	{
		Register16ToVector (VREGISTER_TRANSFERS_COUNT);
		total_count += VREGISTER_TRANSFERS_COUNT;

		diff = DateTime_getMicrosecondTime () - t0;
	}

	long double d = total_count;
	d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
	d /= diff;
	d *= 1000000; // usec->sec
	d /= 1000000000; // billions/sec
	$(console, printf, "%.2Lf billion/second\n", d);
	$(console, flush);
	return 0;
}

static long TestingX86_registerToVector32 (TestingX86 *self)
{
	if (self->use_sse4) {
		$(console, printf, "Main register 32-bit datum to vector register transfers: ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			Register32ToVector (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

static long TestingX86_registerToVector64 (TestingX86 *self)
{
	if (self->use_sse4) {
		$(console, printf, "Main register 64-bit datum to vector register transfers: ");
		$(console, flush);

		long long total_count = 0;
		unsigned long diff = 0;
		unsigned long t0 = DateTime_getMicrosecondTime ();

		while (diff < usec_per_test)
		{
			Register64ToVector (VREGISTER_TRANSFERS_COUNT);
			total_count += VREGISTER_TRANSFERS_COUNT;

			diff = DateTime_getMicrosecondTime () - t0;
		}

		long double d = total_count;
		d *= N_VECTOR_INSERTS_EXTRACTS_PER_LOOP;
		d /= diff;
		d *= 1000000; // usec->sec
		d /= 1000000000; // billions/sec
		$(console, printf, "%.2Lf billion/second\n", d);
		$(console, flush);
	}
	return 0;
}

TestingX86Class* TestingX86Class_init (TestingX86Class* class)
{
	SET_SUPERCLASS(Testing);

	SET_OVERRIDDEN_METHOD_POINTER(TestingX86,describe);
	SET_OVERRIDDEN_METHOD_POINTER(TestingX86,destroy);

        SET_INHERITED_METHOD_POINTER(TestingX86,Object,print);
        SET_INHERITED_METHOD_POINTER(TestingX86,Object,equals);

	SET_METHOD_POINTER(TestingX86,read);
	SET_METHOD_POINTER(TestingX86,write);
	SET_METHOD_POINTER(TestingX86,copy);
	SET_METHOD_POINTER(TestingX86,registerToVectorTest);
	SET_METHOD_POINTER(TestingX86,vectorToRegisterTest);
	SET_METHOD_POINTER(TestingX86,vectorToVectorTest256);
	SET_METHOD_POINTER(TestingX86,vectorToVectorTest512);
	SET_METHOD_POINTER(TestingX86,vectorToRegister8);
	SET_METHOD_POINTER(TestingX86,vectorToRegister16);
	SET_METHOD_POINTER(TestingX86,vectorToRegister32);
	SET_METHOD_POINTER(TestingX86,vectorToRegister64);
	SET_METHOD_POINTER(TestingX86,registerToVector8);
	SET_METHOD_POINTER(TestingX86,registerToVector16);
	SET_METHOD_POINTER(TestingX86,registerToVector32);
	SET_METHOD_POINTER(TestingX86,registerToVector64);

        VALIDATE_CLASS_STRUCT(_TestingX86Class);
	return _TestingX86Class;
}

TestingX86 *TestingX86_init (TestingX86 *self)
{
	ENSURE_CLASS_READY(TestingX86);

        Testing_init ((Testing*) self);

        self->is_a = _TestingX86Class;

        return self;
}
