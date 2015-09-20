/*============================================================================
  bandwidth 1.1, a benchmark to estimate memory transfer bandwidth.
  Copyright (C) 2005-2014 by Zack T Smith.

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

  The author may be reached at veritas@comcast.net.
 *===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <wchar.h>
#include <math.h>

#include <netdb.h> // gethostbyname
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define GRAPH_WIDTH 1440
#define GRAPH_HEIGHT 900

#include "defs.h"
#include "BMP.h"
#include "BMPGraphing.h"

#define TITLE_MEMORY_NET "Network benchmark results from bandwidth " RELEASE " by Zack Smith, http://zsmith.co"
#define TITLE_MEMORY_GRAPH "Memory benchmark results from bandwidth " RELEASE " by Zack Smith, http://zsmith.co"

#ifdef __WIN32__
#include <windows.h>
#endif

#ifdef __linux__
#include <linux/fb.h>
#include <sys/mman.h>
#endif

static int network_port = NETWORK_DEFAULT_PORTNUM;

enum {
	NO_SSE2,
	SSE2,	
	SSE2_BYPASS,
	AVX,
	AVX_BYPASS,
	LODSQ,
	LODSD,
	LODSW,
	LODSB
};

static BMPGraph *graph = NULL;

static bool use_sse2 = true;
static bool use_sse4 = true;
static bool is_intel = false;
static bool is_amd = false;

static uint32_t cpu_has_mmx = 0;
static uint32_t cpu_has_sse = 0;
static uint32_t cpu_has_sse2 = 0;
static uint32_t cpu_has_sse3 = 0;
static uint32_t cpu_has_ssse3 = 0;
static uint32_t cpu_has_sse4a = 0;
static uint32_t cpu_has_sse41 = 0;
static uint32_t cpu_has_sse42 = 0;
static uint32_t cpu_has_aes = 0;
static uint32_t cpu_has_avx = 0;
static uint32_t cpu_has_avx2 = 0;
static uint32_t cpu_has_64bit = 0;
static uint32_t cpu_has_xd = 0;

//----------------------------------------
// Parameters for the tests.
//

static long usec_per_test = 5000000;	// 5 seconds per memory test.

static int chunk_sizes[] = {
	128,
	256,
	384,
	512,
	640,
	768,
	896,
	1024,
	1280,
	2048,
	3072,
	4096,
	6144,
	8192,	// Some processors' L1 data caches are only 8kB.
	12288,
	16384,
	20480,
	24576,
	28672,
	32768,	// Common L1 data cache size.
	34*1024,
	36*1024,
	40960,
	49152,
	65536,
	131072,	// Old L2 cache size.
	192 * 1024,
	256 * 1024,	// Old L2 cache size.
	320 * 1024,
	384 * 1024,
	512 * 1024,	// Old L2 cache size.
	768 * 1024,
	1 << 20,	// 1 MB = common L2 cache size.
	(1024 + 256) * 1024,	// 1.25
	(1024 + 512) * 1024,	// 1.5
	(1024 + 768) * 1024,	// 1.75
	1 << 21,	// 2 MB = common L2 cache size.
	(2048 + 256) * 1024,	// 2.25
	(2048 + 512) * 1024,	// 2.5
	(2048 + 768) * 1024,	// 2.75
	3072 * 1024,	// 3 MB = common L2 cache size. 
	3407872, // 3.25 MB
	3 * 1024 * 1024 + 1024 * 512,	// 3.5 MB
	1 << 22,	// 4 MB
	5242880,	// 5 megs
	6291456,	// 6 megs (common L2 cache size)
	7 * 1024 * 1024,
	8 * 1024 * 1024, // Xeon E3's often has 8MB L3
	9 * 1024 * 1024,
	10 * 1024 * 1024, // Xeon E5-2609 has 10MB L3
	12 * 1024 * 1024,
	14 * 1024 * 1024,
	15 * 1024 * 1024, // Xeon E6-2630 has 15MB L3
	16 * 1024 * 1024,
	20 * 1024 * 1024, // Xeon E5-2690 has 20MB L3
	21 * 1024 * 1024,
	32 * 1024 * 1024,
	48 * 1024 * 1024,
	64 * 1024 * 1024,
	72 * 1024 * 1024,
	96 * 1024 * 1024,
	128 * 1024 * 1024,
	0
};

static double chunk_sizes_log2 [sizeof(chunk_sizes)/sizeof(int)];

//----------------------------------------------------------------------------
// Name:	error
// Purpose:	Complain and exit.
//----------------------------------------------------------------------------
void error (char *s)
{
#ifndef __WIN32__
	fprintf (stderr, "Error: %s\n", s);
	exit (1);
#else
	wchar_t tmp [200];
	int i;
	for (i = 0; s[i]; i++)
		tmp[i] = s[i];
	tmp[i] = 0;
	MessageBoxW (0, tmp, L"Error", 0);
	ExitProcess (0);
#endif
}

//============================================================================
// Output buffer logic. 
// This is somewhat vestigial code, originating with Windows Mobile ARM port.
//============================================================================

#define MSGLEN 10000
static wchar_t msg [MSGLEN];

void print (wchar_t *s)
{
	wcsncat (msg, s, MSGLEN-1);
}

void newline ()
{
	wcsncat (msg, L"\n", MSGLEN-1);
}

void println (wchar_t *s)
{
	wcsncat (msg, s, MSGLEN-1);
	newline ();
}

void print_int (int d)
{
	swprintf (msg + wcslen (msg), MSGLEN, L"%d", d);
}

void print_uint (unsigned int d)
{
	swprintf (msg + wcslen (msg), MSGLEN, L"%lu", d);
}

void println_int (int d)
{
	print_int (d);
	newline ();
}

void print_result (long double result)
{
	swprintf (msg + wcslen (msg), MSGLEN, L"%.1Lf MB/s", result);
}

void dump (FILE *f)
{
	if (!f)
		f = stdout;

	int i = 0;
	while (msg[i]) {
		char ch = (char) msg[i];
		fputc (ch, f);
		i++;
	}

	msg [0] = 0;
}

void flush ()
{
	dump (NULL);
	fflush (stdout);
}

void print_size (unsigned long size)
{
	if (size < 1536) {
		print_int (size);
		print (L" B");
	}
	else if (size < (1<<20)) {
		print_int (size >> 10);
		print (L" kB");
	} else {
		print_int (size >> 20);
		switch ((size >> 18) & 3) {
		case 1: print (L".25"); break;
		case 2: print (L".5"); break;
		case 3: print (L".75"); break;
		}
		print (L" MB");
	}
}

//============================================================================
// Timing logic.
//============================================================================

//----------------------------------------------------------------------------
// Name:	mytime
// Purpose:	Reports time in microseconds.
//----------------------------------------------------------------------------
unsigned long mytime ()
{
#ifndef __WIN32__
	struct timeval tv;
	struct timezone tz;
	memset (&tz, 0, sizeof(struct timezone));
	gettimeofday (&tv, &tz);
	return 1000000 * tv.tv_sec + tv.tv_usec;
#else
	return 1000 * GetTickCount ();	// accurate enough.
#endif
}

//----------------------------------------------------------------------------
// Name:	calculate_result 
// Purpose:	Calculates and prints a result.
// Returns:	10 times the number of megabytes per second.
//----------------------------------------------------------------------------
int
calculate_result (unsigned long chunk_size, long long total_loops, long diff)
{
	if (!diff)
		error ("Zero time difference.");

// printf ("\nIn calculate_result, chunk_size=%ld, total_loops=%lld, diff=%ld\n", chunk_size, total_loops, diff);
	long double result = (long double) chunk_size;
	result *= (long double) total_loops;
	result *= 1000000.; // Convert to microseconds.
	result /= 1048576.;
	result /= (long double) diff;

	print_result (result);

	return (long) (10.0 * result);
}

//============================================================================
// Tests.
//============================================================================

//----------------------------------------------------------------------------
// Name:	do_write
// Purpose:	Performs write on chunk of memory of specified size.
//----------------------------------------------------------------------------
int
do_write (unsigned long size, int mode, bool random)
{
	unsigned char *chunk;
	unsigned char *chunk0;
	unsigned long loops;
	unsigned long long total_count=0;
#ifdef __x86_64__
	unsigned long value = 0x1234567689abcdef;
#else
	unsigned long value = 0x12345678;
#endif
	unsigned long diff=0, t0;
	unsigned long tmp;
	unsigned long **chunk_ptrs = NULL;

	if (size & 127)
		error ("do_write(): chunk size is not multiple of 128.");

	//-------------------------------------------------
	chunk0 = malloc (size+64);
	chunk = chunk0;
	if (!chunk) 
		error ("Out of memory");
	
	tmp = (unsigned long) chunk;
	if (tmp & 31) {
		tmp -= (tmp & 31);
		tmp += 32;
		chunk = (unsigned char*) tmp;
	}

	//----------------------------------------
	// Set up random pointers to chunks.
	//
	if (random) {
		tmp = size/256;
		chunk_ptrs = (unsigned long**) malloc (sizeof (unsigned long*) * tmp);
		if (!chunk_ptrs) 
			error ("Out of memory.");

		//----------------------------------------
		// Store pointers to all chunks into array.
		//
		int i;
		for (i = 0; i < tmp; i++) {
			chunk_ptrs [i] = (unsigned long*) (chunk + 256 * i);
		}

		//----------------------------------------
		// Randomize the array of chunk pointers.
		//
		int k = 100;
		while (k--) {
			for (i = 0; i < tmp; i++) {
				int j = rand() % tmp;
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
		print (L"Random write ");
	else
		print (L"Sequential write ");

	switch (mode) {
	case SSE2:
		print (L"(128-bit), size = ");
		break;
	case AVX:
		print (L"(256-bit), size = ");
		break; 
	case AVX_BYPASS:
                print (L"bypassing cache (256-bit), size = ");
		break;
	case SSE2_BYPASS:
                print (L"bypassing cache (128-bit), size = ");
		break;
	default:
#ifdef __x86_64__
		print (L"(64-bit), size = ");
#else
		print (L"(32-bit), size = ");
#endif
	}

	print_size (size);
	print (L", ");

	loops = (1 << 26) / size;// XX need to adjust for CPU MHz
	if (loops < 1)
		loops = 1;

	t0 = mytime ();

	while (diff < usec_per_test) {
		total_count += loops;

		switch (mode) {
		case SSE2:
			if (random)
				RandomWriterSSE2 (chunk_ptrs, size/256, loops, value);
			else {
				if (size & 128) 
					WriterSSE2_128bytes (chunk, size, loops, value);
				else
					WriterSSE2 (chunk, size, loops, value);
			}
			break;

		case SSE2_BYPASS:
			if (random)
				RandomWriterSSE2_bypass (chunk_ptrs, size/256, loops, value);
			else {
				if (size & 128) 
					WriterSSE2_128bytes_bypass (chunk, size, loops, value);
				else
					WriterSSE2_bypass (chunk, size, loops, value);
			}
			break;

		case AVX:
			if (!random) {
				WriterAVX (chunk, size, loops, value);
			}
			break;

		case AVX_BYPASS:
			if (!random) {
				WriterAVX_bypass (chunk, size, loops, value);
			}
			break;
		
		default:
			if (random)
				RandomWriter (chunk_ptrs, size/256, loops, value);
			else {
				if (size & 128) 
					Writer_128bytes (chunk, size, loops, value);
				else
					Writer (chunk, size, loops, value);
			}
		}

		diff = mytime () - t0;
	}

	print (L"loops = ");
	print_uint (total_count);
	print (L", ");

	flush ();

	int result = calculate_result (size, total_count, diff);
	newline ();

	flush ();

	free ((void*)chunk0);

	if (chunk_ptrs)
		free (chunk_ptrs);

	return result;
}


//----------------------------------------------------------------------------
// Name:	do_read
// Purpose:	Performs sequential read on chunk of memory of specified size.
//----------------------------------------------------------------------------
int
do_read (unsigned long size, int mode, bool random)
{
	unsigned long loops;
	unsigned long long total_count = 0;
	unsigned long t0, diff=0;
	unsigned char *chunk;
	unsigned char *chunk0;
	unsigned long tmp;
	unsigned long **chunk_ptrs = NULL;

	if (size & 127)
		error ("do_read(): chunk size is not multiple of 128.");

	//-------------------------------------------------
	chunk0 = chunk = malloc (size+64);
	if (!chunk) 
		error ("Out of memory");

	memset (chunk, 0, size);

	tmp = (unsigned long) chunk;
	if (tmp & 31) {
		tmp -= (tmp & 31);
		tmp += 32;
		chunk = (unsigned char*) tmp;
	}

	//----------------------------------------
	// Set up random pointers to chunks.
	//
	if (random) {
		int tmp = size/256;
		chunk_ptrs = (unsigned long**) malloc (sizeof (unsigned long*) * tmp);
		if (!chunk_ptrs) 
			error ("Out of memory.");

		//----------------------------------------
		// Store pointers to all chunks into array.
		//
		int i;
		for (i = 0; i < tmp; i++) {
			chunk_ptrs [i] = (unsigned long*) (chunk + 256 * i);
		}

		//----------------------------------------
		// Randomize the array of chunk pointers.
		//
		int k = 100;
		while (k--) {
			for (i = 0; i < tmp; i++) {
				int j = rand() % tmp;
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
		print (L"Random read ");
	else
		print (L"Sequential read ");

	switch (mode) {
	case SSE2:
		print (L"(128-bit), size = ");
		break;
	case LODSB:
		print (L"(8-bit LODSB), size = ");
		break; 
	case LODSW:
		print (L"(16-bit LODSW), size = ");
		break; 
	case LODSD:
		print (L"(32-bit LODSD), size = ");
		break; 
	case LODSQ:
		print (L"(64-bit LODSQ), size = ");
		break; 
	case AVX:
		print (L"(256-bit), size = ");
		break; 
	case AVX_BYPASS:
                print (L"bypassing cache (256-bit), size = ");
		break;
	case SSE2_BYPASS:
                print (L"bypassing cache (128-bit), size = ");
		break;
	default:
#ifdef __x86_64__
		print (L"(64-bit), size = ");
#else
		print (L"(32-bit), size = ");
#endif
	}

	print_size (size);
	print (L", ");

	flush ();

	loops = (1 << 26) / size;	// XX need to adjust for CPU MHz
	if (loops < 1)
		loops = 1;
	
	t0 = mytime ();

	while (diff < usec_per_test) {
		total_count += loops;

		switch (mode) {
		case SSE2:
			if (random)
				RandomReaderSSE2 (chunk_ptrs, size/256, loops);
			else {
				if (size & 128) 
					ReaderSSE2_128bytes (chunk, size, loops);
				else
					ReaderSSE2 (chunk, size, loops);
			}
			break;
		
		case SSE2_BYPASS:
			// No random reader for bypass.
			//
			if (random)
				RandomReaderSSE2_bypass (chunk_ptrs, size/256, loops);
			else {
				if (size & 128) 
					ReaderSSE2_128bytes_bypass (chunk, size, loops);
				else
					ReaderSSE2_bypass (chunk, size, loops);
			}
			break;

		case AVX:
			if (!random) {
				ReaderAVX (chunk, size, loops);
			}
			break;
		
		case LODSB:
			if (!random) {
				ReaderLODSB (chunk, size, loops);
			}
			break;
		
		case LODSW:
			if (!random) {
				ReaderLODSW (chunk, size, loops);
			}
			break;
		
		case LODSD:
			if (!random) {
				ReaderLODSD (chunk, size, loops);
			}
			break;
		
		case LODSQ:
			if (!random) {
				ReaderLODSQ (chunk, size, loops);
			}
			break;
		
		default:
			if (random) {
				RandomReader (chunk_ptrs, size/256, loops);
			} else {
				if (size & 128) 
					Reader_128bytes (chunk, size, loops);
				else
					Reader (chunk, size, loops);
			}
		}

		diff = mytime () - t0;
	}

	print (L"loops = ");
	print_uint (total_count);
	print (L", ");

	int result = calculate_result (size, total_count, diff);
	newline ();

	flush ();

	free (chunk0);

	if (chunk_ptrs)
		free (chunk_ptrs);

	return result;
}



//----------------------------------------------------------------------------
// Name:	do_copy
// Purpose:	Performs sequential memory copy.
//----------------------------------------------------------------------------
int
do_copy (unsigned long size, int mode)
{
	unsigned long loops;
	unsigned long long total_count = 0;
	unsigned long t0, diff=0;
	unsigned char *chunk_src;
	unsigned char *chunk_dest;
	unsigned char *chunk_src0;
	unsigned char *chunk_dest0;
	unsigned long tmp;

	if (size & 127)
		error ("do_copy(): chunk size is not multiple of 128.");

	//-------------------------------------------------
	chunk_src0 = chunk_src = malloc (size+64);
	if (!chunk_src) 
		error ("Out of memory");
	chunk_dest0 = chunk_dest = malloc (size+64);
	if (!chunk_dest) 
		error ("Out of memory");

	memset (chunk_src, 100, size);
	memset (chunk_dest, 200, size);
	
	tmp = (unsigned long) chunk_src;
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

	//-------------------------------------------------
	print (L"Sequential copy ");

	if (mode == SSE2) {
		print (L"(128-bit), size = ");
	} 
	else if (mode == AVX) {
		print (L"(256-bit), size = ");
	} 
	else {
#ifdef __x86_64__
		print (L"(64-bit), size = ");
#else
		print (L"(32-bit), size = ");
#endif
	}

	print_size (size);
	print (L", ");

	flush ();

	loops = (1 << 26) / size;	// XX need to adjust for CPU MHz
	if (loops < 1)
		loops = 1;
	
	t0 = mytime ();

	while (diff < usec_per_test) {
		total_count += loops;

		if (mode == SSE2)  {
#ifdef __x86_64__
			if (size & 128)
				CopySSE_128bytes (chunk_dest, chunk_src, size, loops);
			else
				CopySSE (chunk_dest, chunk_src, size, loops);
#else
			CopySSE (chunk_dest, chunk_src, size, loops);
#endif
		}
		else if (mode == AVX) {
			if (!(size & 128))
				CopyAVX (chunk_dest, chunk_src, size, loops);
		}

		diff = mytime () - t0;
	}

	print (L"loops = ");
	print_uint (total_count);
	print (L", ");

	int result = calculate_result (size, total_count, diff);
	newline ();

	flush ();

	free (chunk_src0);
	free (chunk_dest0);

	return result;
}


//----------------------------------------------------------------------------
// Name:	fb_readwrite
// Purpose:	Performs sequential read & write tests on framebuffer memory.
//----------------------------------------------------------------------------
#if defined(__linux__) && defined(FBIOGET_FSCREENINFO)
void
fb_readwrite (bool use_sse2)
{
	unsigned long counter, total_count;
	unsigned long length;
	unsigned long diff, t0;
	static struct fb_fix_screeninfo fi;
	static struct fb_var_screeninfo vi;
	unsigned long *fb = NULL;
	unsigned long datum;
	int fd;
	register unsigned long foo;
#ifdef __x86_64__
	unsigned long value = 0x1234567689abcdef;
#else
	unsigned long value = 0x12345678;
#endif

	//-------------------------------------------------

	fd = open ("/dev/fb0", O_RDWR);
	if (fd < 0) 
		fd = open ("/dev/fb/0", O_RDWR);
	if (fd < 0) {
		println (L"Cannot open framebuffer device.");
		return;	
	}

	if (ioctl (fd, FBIOGET_FSCREENINFO, &fi)) {
		close (fd);
		println (L"Cannot get framebuffer info");
		return;
	}
	else
	if (ioctl (fd, FBIOGET_VSCREENINFO, &vi)) {
		close (fd);
		println (L"Cannot get framebuffer info");
		return;
	}
	else
	{
		if (fi.visual != FB_VISUAL_TRUECOLOR &&
		    fi.visual != FB_VISUAL_DIRECTCOLOR ) {
			close (fd);
			println (L"Need direct/truecolor framebuffer device.");
			return;
		} else {
			unsigned long fblen;

			print (L"Framebuffer resolution: ");
			print_int (vi.xres);
			print (L"x");
			print_int (vi.yres);
			print (L", ");
			print_int (vi.bits_per_pixel);
			println (L" bpp\n");

			fb = (unsigned long*) fi.smem_start;
			fblen = fi.smem_len;

			fb = mmap (fb, fblen,
				PROT_WRITE | PROT_READ,
				MAP_SHARED, fd, 0);
			if (fb == MAP_FAILED) {
				close (fd);
				println (L"Cannot access framebuffer memory.");
				return;
			}
		}
	}

	//-------------------
	// Use only the upper half of the display.
	//
	length = FB_SIZE;

	//-------------------
	// READ
	//
	print (L"Framebuffer memory sequential read ");
	flush ();

	t0 = mytime ();

	total_count = FBLOOPS_R;

	if (use_sse2)
		ReaderSSE2 (fb, length, FBLOOPS_R);
	else
		Reader (fb, length, FBLOOPS_R);

	diff = mytime () - t0;

	calculate_result (length, total_count, diff);
	newline ();

	//-------------------
	// WRITE
	//
	print (L"Framebuffer memory sequential write ");
	flush ();

	t0 = mytime ();

	total_count = FBLOOPS_W;

	if (use_sse2) 
		WriterSSE2_bypass (fb, length, FBLOOPS_W, value);
	else
		Writer (fb, length, FBLOOPS_W, value);

	diff = mytime () - t0;

	calculate_result (length, total_count, diff);
	newline ();
}
#endif

//----------------------------------------------------------------------------
// Name:	register_test
// Purpose:	Determines bandwidth of register-to-register transfers.
//----------------------------------------------------------------------------
void
register_test () 
{
	long long total_count = 0;
	unsigned long t0;
	unsigned long diff = 0;

	//--------------------------------------
#ifdef __x86_64__
	print (L"Main register to main register transfers (64-bit) ");
#else
	print (L"Main register to main register transfers (32-bit) ");
#endif
	flush ();
#define REGISTER_COUNT 10000

	t0 = mytime ();
	while (diff < usec_per_test) 
	{
		RegisterToRegister (REGISTER_COUNT);
		total_count += REGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (256, total_count, diff);
	newline ();
	flush ();

	//--------------------------------------
#ifdef __x86_64__
	print (L"Main register to vector register transfers (64-bit) ");
#else
	print (L"Main register to vector register transfers (32-bit) ");
#endif
	flush ();
#define VREGISTER_COUNT 3333

	t0 = mytime ();
	diff = 0;
	total_count = 0;
	while (diff < usec_per_test) 
	{
		RegisterToVector (VREGISTER_COUNT);
		total_count += VREGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (256, total_count, diff);
	newline ();
	flush ();

	//--------------------------------------
#ifdef __x86_64__
	print (L"Vector register to main register transfers (64-bit) ");
#else
	print (L"Vector register to main register transfers (32-bit) ");
#endif
	flush ();

	t0 = mytime ();
	diff = 0;
	total_count = 0;
	while (diff < usec_per_test) 
	{
		VectorToRegister (VREGISTER_COUNT);
		total_count += VREGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (256, total_count, diff);
	newline ();
	flush ();

	//--------------------------------------
	print (L"Vector register to vector register transfers (128-bit) ");
	flush ();

	t0 = mytime ();
	diff = 0;
	total_count = 0;
	while (diff < usec_per_test) 
	{
		VectorToVector (VREGISTER_COUNT);
		total_count += VREGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (256, total_count, diff);
	newline ();
	flush ();

	//--------------------------------------
	if (cpu_has_avx) {
		print (L"Vector register to vector register transfers (256-bit) ");
		flush ();

		t0 = mytime ();
		diff = 0;
		total_count = 0;
		while (diff < usec_per_test) 
		{
			VectorToVectorAVX (VREGISTER_COUNT);
			total_count += VREGISTER_COUNT;

			diff = mytime () - t0;
		}

		calculate_result (256, total_count, diff);
		newline ();
		flush ();
	}

	//--------------------------------------
	if (use_sse4) {
		print (L"Vector 8-bit datum to main register transfers ");
		flush ();

		t0 = mytime ();
		diff = 0;
		total_count = 0;
		while (diff < usec_per_test) 
		{
			Vector8ToRegister (VREGISTER_COUNT);
			total_count += VREGISTER_COUNT;

			diff = mytime () - t0;
		}

		calculate_result (64, total_count, diff);
		newline ();
		flush ();
	}

	//--------------------------------------
	print (L"Vector 16-bit datum to main register transfers ");
	flush ();

	t0 = mytime ();
	diff = 0;
	total_count = 0;
	while (diff < usec_per_test) 
	{
		Vector16ToRegister (VREGISTER_COUNT);
		total_count += VREGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (128, total_count, diff);
	newline ();
	flush ();

	//--------------------------------------
	if (use_sse4) {
		print (L"Vector 32-bit datum to main register transfers ");
		flush ();

		t0 = mytime ();
		diff = 0;
		total_count = 0;
		while (diff < usec_per_test) 
		{
			Vector32ToRegister (VREGISTER_COUNT);
			total_count += VREGISTER_COUNT;

			diff = mytime () - t0;
		}

		calculate_result (256, total_count, diff);
		newline ();
		flush ();
	}

	//--------------------------------------
	if (use_sse4) {
		print (L"Vector 64-bit datum to main register transfers ");
		flush ();

		t0 = mytime ();
		diff = 0;
		total_count = 0;
		while (diff < usec_per_test) 
		{
			Vector64ToRegister (VREGISTER_COUNT);
			total_count += VREGISTER_COUNT;

			diff = mytime () - t0;
		}

		calculate_result (256, total_count, diff);
		newline ();
		flush ();
	}

	//--------------------------------------
	if (use_sse4) {
		print (L"Main register 8-bit datum to vector register transfers ");
		flush ();

		t0 = mytime ();
		diff = 0;
		total_count = 0;
		while (diff < usec_per_test) 
		{
			Register8ToVector (VREGISTER_COUNT);
			total_count += VREGISTER_COUNT;

			diff = mytime () - t0;
		}

		calculate_result (64, total_count, diff);
		newline ();
		flush ();
	}

	//--------------------------------------
	print (L"Main register 16-bit datum to vector register transfers ");
	flush ();

	t0 = mytime ();
	diff = 0;
	total_count = 0;
	while (diff < usec_per_test) 
	{
		Register16ToVector (VREGISTER_COUNT);
		total_count += VREGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (128, total_count, diff);
	newline ();
	flush ();

	//--------------------------------------
	if (use_sse4) {
		print (L"Main register 32-bit datum to vector register transfers ");
		flush ();

		t0 = mytime ();
		diff = 0;
		total_count = 0;
		while (diff < usec_per_test) 
		{
			Register32ToVector (VREGISTER_COUNT);
			total_count += VREGISTER_COUNT;

			diff = mytime () - t0;
		}

		calculate_result (256, total_count, diff);
		newline ();
		flush ();
	}

	//--------------------------------------
	if (use_sse4) {
		print (L"Main register 64-bit datum to vector register transfers ");
		flush ();

		t0 = mytime ();
		diff = 0;
		total_count = 0;
		while (diff < usec_per_test) 
		{
			Register64ToVector (VREGISTER_COUNT);
			total_count += VREGISTER_COUNT;

			diff = mytime () - t0;
		}

		calculate_result (256, total_count, diff);
		newline ();
		flush ();
	}
}

//----------------------------------------------------------------------------
// Name:	stack_test
// Purpose:	Determines bandwidth of stack-to/from-register transfers.
//----------------------------------------------------------------------------
void
stack_test () 
{
	long long total_count = 0;
	unsigned long t0;
	unsigned long diff = 0;

#ifdef __x86_64__
	print (L"Stack-to-register transfers (64-bit) ");
#else
	print (L"Stack-to-register transfers (32-bit) ");
#endif
	flush ();

	//--------------------------------------
	diff = 0;
	total_count = 0;
	t0 = mytime ();
	while (diff < usec_per_test) 
	{
		StackReader (REGISTER_COUNT);
		total_count += REGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (256, total_count, diff);
	newline ();
	flush ();

#ifdef __x86_64__
	print (L"Register-to-stack transfers (64-bit) ");
#else
	print (L"Register-to-stack transfers (32-bit) ");
#endif
	flush ();

	//--------------------------------------
	diff = 0;
	total_count = 0;
	t0 = mytime ();
	while (diff < usec_per_test) 
	{
		StackWriter (REGISTER_COUNT);
		total_count += REGISTER_COUNT;

		diff = mytime () - t0;
	}

	calculate_result (256, total_count, diff);
	newline ();
	flush ();
}

//----------------------------------------------------------------------------
// Name:	library_test
// Purpose:	Performs C library tests (memset, memcpy).
//----------------------------------------------------------------------------
void
library_test () 
{
	char *a1, *a2;
	unsigned long t, t0;
	int i;

	#define NT_SIZE (64*1024*1024)
	#define NT_SIZE2 (100)

	a1 = malloc (NT_SIZE);
	if (!a1) 
		error ("Out of memory");
	
	a2 = malloc (NT_SIZE);
	if (!a2) 
		error ("Out of memory");

	//--------------------------------------
	t0 = mytime ();
	for (i=0; i<NT_SIZE2; i++) {
		memset (a1, i, NT_SIZE);
	}
	t = mytime ();

	print (L"Library: memset ");
	calculate_result (NT_SIZE, NT_SIZE2, t-t0);
	newline ();

	flush ();

	//--------------------------------------
	t0 = mytime ();
	for (i=0; i<NT_SIZE2; i++) {
		memcpy (a2, a1, NT_SIZE);
	}
	t = mytime ();

	print (L"Library: memcpy ");
	calculate_result (NT_SIZE, NT_SIZE2, t-t0);
	newline ();

	flush ();

	free (a1);
	free (a2);
}

//----------------------------------------------------------------------------
// Name:	network_test_core
// Purpose:	Performs the network test, talking to and receiving data
//		back from a transponder node.
// Note:	Port number specified using server:# notation.
// Returns:	-1 on error, else the network duration in microseconds.
//----------------------------------------------------------------------------
bool
network_test_core (const char *hostname, char *chunk, 
			unsigned long chunk_size, 
			unsigned long n_chunks,
			long *duration_send_return,
			long *duration_recv_return)
{
	if (!hostname || !chunk || !n_chunks || !chunk_size ||
		!duration_send_return ||
		!duration_recv_return)
		return false;

	struct hostent* host = gethostbyname (hostname);
	if (!host)
		return false;
	
	char *host_ip = inet_ntoa (*(struct in_addr *)*host->h_addr_list);
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(host_ip);
	addr.sin_port = htons(network_port);

	if (connect (sock, (struct sockaddr*) &addr, sizeof (struct sockaddr)))
	{
		// perror ("connect");
		close (sock);
		return false;
	}

	//------------------------------------
	// Start stopwatch just before the send.
	// It will be stopped on receipt of
	// the response.
	//
	unsigned long t0 = mytime ();

	//------------------------------------
	// Put # of chunks in the chunk.
	// Send all of our data.
	//
	sprintf (chunk, "%lu\n", n_chunks);
	int i;
	for (i = 0; i < n_chunks; i++) 
		send (sock, chunk, chunk_size, 0);

#if 0
	//------------------------------------
	// Set nonblocking mode.
	//
	int opt = 1;
	ioctl (sock, FIONBIO, &opt);
#endif

	unsigned long t1 = mytime ();

	//------------------------------------
	// Read the response.
	//
	int amount = recv (sock, chunk, chunk_size, 0);
	if (amount < 16) {
		close (sock);
		return false;
	}

	unsigned long duration_send = mytime() - t0;

	//------------------------------------
	// Validate the response, which 
	// contains the transponder's 
	// perceived read duration. This value
	// may be as little as half our number.
	//
	unsigned long duration2 = -1;
	if (strncmp ("OK: ", chunk, 4)) {
		close (sock);
		return false;
	}
	if (1 != sscanf (4+chunk, "%lu", &duration2)) {
		close (sock);
		return false;
	}

	unsigned long remaining = chunk_size * n_chunks - amount;
	while (remaining > 0) {
		int amount = recv (sock, chunk, chunk_size, 0);
		if (amount <= 0) {
			perror ("recv");
			close (sock);
			return false;
		}
		remaining -= amount;
	}

	unsigned long duration_recv = mytime () - t1;

	*duration_send_return = duration_send;
	*duration_recv_return = duration_recv;

	close (sock);
	return true;
}

//----------------------------------------------------------------------------
// Name:	ip_to_str
//----------------------------------------------------------------------------
void 
ip_to_str (unsigned long addr, char *str)
{
	if (!str)
		return;

	unsigned short a = 0xff & addr;
	unsigned short b = 0xff & (addr >> 8);
	unsigned short c = 0xff & (addr >> 16);
	unsigned short d = 0xff & (addr >> 24);
	sprintf (str, "%u.%u.%u.%u", a,b,c,d);
}

//----------------------------------------------------------------------------
// Name:	network_transponder
// Purpose:	Act as a transponder, receiving chunks of data and sending
//		back an acknowledgement once the enture chunk is read.
// Returns:	False if a problem occurs setting up the network socket.
//----------------------------------------------------------------------------
bool
network_transponder ()
{
	struct sockaddr_in sin, from;

	//------------------------------
	// Get listening socket for port.
	// Then listen on given port#.
	//
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(network_port);
	int listensock;
	if ((listensock = socket (AF_INET, SOCK_STREAM, 0)) < 0)  {
		perror ("socket");
		return false;
	}
	if (bind (listensock, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
		perror ("bind");
		close (listensock);
		return false;
	}
	if (listen (listensock, 500) < 0) {
		perror ("listen");
		close (listensock);
		return false;
	}

	bool done = false;
	while (!done) {
		//----------------------------------------
		// Wait for a client to contact us.
		//
		socklen_t len = sizeof (struct sockaddr);
		int sock = accept (listensock, (struct sockaddr*) &from, &len);
		if (sock < 0) {
			perror ("accept");
			close (listensock);
			return false;
		}

		//----------------------------------------
		// Clockwatch starts when we accept the
		// connection.
		//
		unsigned long t0 = mytime ();

		if (len != sizeof (struct sockaddr_in)) {
			close (sock); 
			close (listensock);
			return false;
		}

#if 0
		unsigned long ipaddr = from.sin_addr.s_addr;
		char ipstring[30];
		ip_to_str (ipaddr, ipstring);
		fprintf (stderr, "Incoming connection from %s\n", ipstring);
#endif

		//----------------------------------------
		// Read the first chunk only, in order to
		// get the # of bytes that will be sent.
		//
		char chunk [NETWORK_CHUNK_SIZE+1];
		long n_chunks = 0;
		int amount_read = read (sock, chunk, NETWORK_CHUNK_SIZE);
		chunk [amount_read] = 0;
		if (1 != sscanf (chunk, "%ld", &n_chunks)) {
			close (sock);
			close (listensock); 
			return false;
		}

		//----------------------------------------
		// If the leader sends us a chunk count of
		// -99, this indicates that we should exit.
		//
		if (n_chunks == -99) {
			close (sock); 
			close (listensock); 
			return true;
		}

//		printf ("Reading %lu chunks of %d bytes...\n", n_chunks, NETWORK_CHUNK_SIZE);

		unsigned long long remaining = n_chunks;
		remaining *= NETWORK_CHUNK_SIZE;

//		printf ("remaining="); dump_hex64(remaining); puts("");

		remaining -= amount_read;
		while (remaining > 0) {
			amount_read = read (sock, chunk, NETWORK_CHUNK_SIZE);
			remaining -= amount_read;

			if (amount_read < 0) {
				perror ("read");
				break;
			} else 
			if (!amount_read)
				break;
		}

		unsigned long duration = mytime() - t0;

		//------------------------------------
		// Send response of same size.
		//
		sprintf (chunk, "OK: %lu\n", duration);
		chunk[14] = '\n';

		//------------------------------------
		// Send all of our data.
		//
		int i;
		for (i = 0; i < n_chunks; i++) 
			send (sock, chunk, NETWORK_CHUNK_SIZE, 0);

		close (sock);
	}

	return true;
}

//----------------------------------------------------------------------------
// Name:	network_test
//----------------------------------------------------------------------------
bool
network_test (char **destinations, int n_destinations)
{
	int i;

	//----------------------------------------
	// The memory chunk starts with a 12-byte
	// length of the overall send size.
	// The memory chunk will have a list of 
	// the destinations in it.
	// In future, there will be a mechanism 
	// for testing bandwidth between all nodes,
	// not just the leader & each of the 
	// transponders.
	//
	char chunk [NETWORK_CHUNK_SIZE];
	memset (chunk, 0, NETWORK_CHUNK_SIZE);
	sprintf (chunk, "000000000000\n%d\n", n_destinations);
	for (i = 0; i < n_destinations; i++) {
		char *s = destinations [i];
		int chunk_len = strlen (chunk);
		int len = strlen (s);
		if (len + chunk_len < NETWORK_CHUNK_SIZE-1) {
			//----------------------------------------
			// "transp" indicates that the given node
			// has not yet been a leader.
			// In future, "done" will indicate it has.
			//
			sprintf (chunk + chunk_len, "%s %s\n", s, "transp");
		}
	}

	static unsigned long colors [] = {
		RGB_RED, RGB_GREEN, RGB_BLUE, RGB_ORANGE, RGB_PURPLE,
		RGB_BLACK, RGB_CORAL,
		RGB_CYAN, RGB_NAVYBLUE, RGB_BRASS, RGB_DARKORANGE,
		RGB_DARKGREEN, RGB_SALMON, RGB_MAGENTA, RGB_LEMONYELLOW,
		RGB_ROYALBLUE, RGB_DODGERBLUE, RGB_TURQUOISE, RGB_CADETBLUE,
		RGB_CHARTREUSE, RGB_DARKOLIVEGREEN, RGB_VIOLET,
		RGB_KHAKI, RGB_DARKKHAKI, RGB_GOLDENROD
	};
#define NCOLORS (sizeof(colors)/sizeof(unsigned long))

	//----------------------------------------
	// For each destination, run the test.
	//
	for (i = 0; i < n_destinations; i++) {
		bool problem = false;

		char *hostname = destinations[i];
		printf ("Bandwidth sending to %s:\n", hostname);

		char title [PATH_MAX];
		sprintf (title, "%s send (solid)", hostname);
		BMPGraphing_new_line (graph, title, i < NCOLORS? colors[i] : RGB_GRAY);

		//----------------------------------------
		// Cache the receive durations for later.
		//
		unsigned long recv_rates [NETSIZE_MAX];
		int recv_ix = 0;

		//----------------------------------------
		// Send data of increasing sizes.
		//
		int j = NETSIZE_MIN;
		int n_runs = 64;
		while (!problem && j <= NETSIZE_MAX) {
			unsigned long chunk_count = 1 << (j-NETSIZE_MIN);
			unsigned long long amt_to_send = chunk_count;
			amt_to_send *= NETWORK_CHUNK_SIZE;

			if (!amt_to_send) // unlikely
				break;

			//----------------------------------------
			// Send the data; do this n_runs times.
			//
			unsigned long long total_duration_send = 0;
			unsigned long long total_duration_recv = 0;

			int k = n_runs;
			while (k--) {
				long duration_send, duration_recv;

				if (! network_test_core (hostname,
					chunk, NETWORK_CHUNK_SIZE, chunk_count,
					&duration_send, &duration_recv))
				{
					problem = true;
					fprintf (stderr, "\nCan't connect to %s\n", hostname);
					break;
				}

				total_duration_send += duration_send;
				total_duration_recv += duration_recv;
			}

			if (problem)
				break;

			total_duration_send += n_runs/2;	// Round up
			total_duration_send /= n_runs;	// Get average
			long duration = (long) total_duration_send;
			
			total_duration_recv += n_runs/2;	// Round up
			total_duration_recv /= n_runs;	// Get average
			
			unsigned long amt_in_kb = amt_to_send / 1024;
			unsigned long amt_in_mb = amt_to_send / 1048576;
			if (!amt_in_mb) {
				printf ("\r\tChunk %lu kB x %d: \t", amt_in_kb,
					n_runs);
			} else {
				printf ("\r\tChunk %lu MB x %d: \t", amt_in_mb,
					n_runs);
			}

			//------------------------------
			// Calculate send rate in MB/sec.
			//
			// Get total # bytes.
			unsigned long long tmp = NETWORK_CHUNK_SIZE;
			tmp *= chunk_count;

			// Get total bytes per second.
			tmp *= 1000000;
			tmp /= duration;

			// Bytes to megabytes.
			tmp /= 1000;
			tmp /= 10;
			unsigned long whole = tmp / 100;
			unsigned long frac = tmp % 100;
			printf ("%lu.%02lu MB/s (sent)\t", whole, frac);
			fflush (stdout);

			BMPGraphing_add_point (graph, amt_in_kb, tmp);

			//------------------------------
			// Calculate recv rate in MB/sec.
			//
			// Get total # bytes.
			tmp = NETWORK_CHUNK_SIZE;
			tmp *= chunk_count;

			// Get total bytes per second.
			tmp *= 1000000;
			tmp /= total_duration_recv;

			// Bytes to megabytes.
			tmp /= 1000;
			tmp /= 10;
			whole = tmp / 100;
			frac = tmp % 100;
			printf ("%lu.%02lu MB/s (received)\n", whole, frac);

			recv_rates [recv_ix++] = tmp;

			j++;
			n_runs >>= 1;
			if (!n_runs)
				n_runs = 1;
		}

		//----------------------------------------
		// Now add the line for the receive rates.
		//
		sprintf (title, "%s receive (dashed)", hostname);
		BMPGraphing_new_line (graph, title, DASHED |
				(i < NCOLORS? colors[i] : RGB_GRAY));
		for (j = NETSIZE_MIN; j <= NETSIZE_MAX; j++) {
			unsigned long chunk_count = 1 << (j-NETSIZE_MIN);
			unsigned long long amt_to_send = chunk_count;
			amt_to_send *= NETWORK_CHUNK_SIZE;
			unsigned long amt_in_kb = amt_to_send / 1024;
// printf ("amt_in_kb=%ld\n",amt_in_kb);

			BMPGraphing_add_point (graph, amt_in_kb, recv_rates[j-NETSIZE_MIN]);
		}

		puts ("");
	}

	return true;
}

//----------------------------------------------------------------------------
// Name:	usage
//----------------------------------------------------------------------------
void
usage ()
{
	printf ("Usage: bandwidth [--slow] [--fast] [--faster] [--fastest] [--title string]\n");
        printf ("Usage for starting network tests: bandwidth --network <ipaddr1> [<ipaddr2...] [--port <port#>]\n");
        printf ("Usage for receiving network tests: bandwidth --transponder [--port <port#>]\n");

	exit (0);
}

//----------------------------------------------------------------------------
// Name:	main
//----------------------------------------------------------------------------
int
main (int argc, char **argv)
{
	int i, chunk_size;

	--argc;
	++argv;

        bool network_mode = false;
        bool network_leader = false; // false => transponder
        int network_destinations_size = 0;
        int n_network_destinations = 0;
        char **network_destinations = NULL;

	char graph_title [512] = {0};

	i = 0;
	while (i < argc) {
		char *s = argv [i++];
		
		if (!strcmp ("--network", s)) {
			network_mode = true;
			network_leader = true;
			network_destinations_size = 20;
			network_destinations = (char**) malloc (network_destinations_size * sizeof (char*));
		}
		else
		if (!strcmp ("--transponder", s)) {
			network_mode = true;
		}
		else
		if (!strcmp ("--port", s)) {
			if (i != argc)
				network_port = atoi (argv[i++]);
		}
		else
		if (!strcmp ("--slow", s)) {
			usec_per_test=20000000;	// 20 seconds per test.
		}
		else
		if (!strcmp ("--fast", s)) {
			usec_per_test = 500000;	// 0.5 seconds per test.
		}
		else
		if (!strcmp ("--faster", s)) {
			usec_per_test = 50000;	// 0.05 seconds per test.
		}
		else
		if (!strcmp ("--fastest", s)) {
			usec_per_test = 5000;	// 0.005 seconds per test.
		}
		else
		if (!strcmp ("--nosse2", s)) {
			use_sse2 = false;
			use_sse4 = false;
		}
		else
		if (!strcmp ("--nosse4", s)) {
			use_sse4 = false;
		}
		else
		if (!strcmp ("--help", s)) {
			usage ();
		}
		else
		if (!strcmp ("--title", s) && i != argc) {
			snprintf (graph_title, 511, "%s", argv[i++]);
		}
		else {
			if ('-' == *s)
				usage ();
		}
	}

	msg[0] = 0;

	for (i = 0; chunk_sizes[i] && i < sizeof(chunk_sizes)/sizeof(int); i++) {
		chunk_sizes_log2[i] = log2 (chunk_sizes[i]);
	}

	printf ("This is bandwidth version %s.\n", RELEASE);
	printf ("Copyright (C) 2005-2014 by Zack T Smith.\n\n");
	printf ("This software is covered by the GNU Public License.\n");
	printf ("It is provided AS-IS, use at your own risk.\n");
	printf ("See the file COPYING for more information.\n\n");
	fflush (stdout);

	//----------------------------------------
	// If network mode selected, enter it now.
	// Currently cannot combine memory tests
	// & network tests.
	//
	if (network_mode) {
		if (network_leader) {
		        graph = BMPGraphing_new (GRAPH_WIDTH, GRAPH_HEIGHT, MODE_X_AXIS_LINEAR);
			strcpy (graph_title, TITLE_MEMORY_NET);
			BMPGraphing_set_title (graph, graph_title);

			network_test (network_destinations, n_network_destinations);
			
		        BMPGraphing_make (graph);

			BMP_write (graph->image, "network_bandwidth.bmp");

#if defined(__linux__) || defined(__CYGWIN__) || defined(__APPLE__)
			puts ("Wrote graph to network_bandwidth.bmp.");
			puts ("");
			puts ("Done.");
#endif
			BMPGraphing_destroy (graph);
		} else {
			network_transponder ();
		}

		return 0;
	}

	uint32_t ecx = get_cpuid1_ecx ();
	uint32_t edx = get_cpuid1_edx ();
	cpu_has_mmx = edx & CPUID_EDX_MMX;
	cpu_has_sse = edx & CPUID_EDX_SSE;
	cpu_has_sse2 = edx & CPUID_EDX_SSE2;
	cpu_has_sse3 = ecx & CPUID_ECX_SSE3;
	cpu_has_ssse3 = ecx & CPUID_ECX_SSSE3;
	cpu_has_sse41 = ecx & CPUID_ECX_SSE41;
	cpu_has_sse42 = ecx & CPUID_ECX_SSE42;
	cpu_has_aes = ecx & CPUID_ECX_AES;
	cpu_has_avx = ecx & CPUID_ECX_AVX;
	cpu_has_avx2 = 0;

	if (cpu_has_avx) {
		cpu_has_avx2 = get_cpuid7_ebx ();
		cpu_has_avx2 &= CPUID_EBX_AVX2;
	}

	use_sse2 = true;
	use_sse4 = true;

	cpu_has_sse4a = 0;
	cpu_has_64bit = 0;
	cpu_has_xd = 0;

	static char family [17];
	get_cpuid_family (family);
	family [16] = 0;
	printf ("CPU family: %s\n", family);

	uint32_t ecx2 = get_cpuid_80000001_ecx ();
	uint32_t edx2 = get_cpuid_80000001_edx ();

	if (!strcmp ("AuthenticAMD", family)) {
		is_amd = true;
		cpu_has_sse4a = ecx2 & CPUID_ECX_SSE4A;
	}
	else
	if (!strcmp ("GenuineIntel", family)) {
		is_intel = true;
	}

	cpu_has_xd = edx2 & CPUID_EDX_XD;
	cpu_has_64bit = edx2 & CPUID_EDX_INTEL64;

	printf ("CPU features: ");
	if (cpu_has_mmx) printf ("MMX ");
	if (cpu_has_sse) printf ("SSE ");
	if (cpu_has_sse2) printf ("SSE2 ");
	if (cpu_has_sse3) printf ("SSE3 ");
	if (cpu_has_ssse3) printf ("SSSE3 ");
	if (cpu_has_sse4a) printf ("SSE4A ");
	if (cpu_has_sse41) printf ("SSE4.1 ");
	if (cpu_has_sse42) printf ("SSE4.2 ");
	if (cpu_has_aes) printf ("AES ");
	if (cpu_has_avx) printf ("AVX ");
	if (cpu_has_avx2) printf ("AVX2 ");
	if (cpu_has_xd) printf ("XD ");
	if (cpu_has_64bit) {
		if (!is_amd)
			printf ("Intel64 ");
		else 
			printf ("LongMode ");
	}
	puts ("\n");

	if (is_intel) {
		uint32_t cache_info[4];
		i = 0;
		while (1) {
			get_cpuid_cache_info (cache_info, i);
			if (!(cache_info[0] & 31))
				break;

#if 0
			printf ("Cache info %d = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", i, 
				cache_info [0],
				cache_info [1],
				cache_info [2],
				cache_info [3]);
#endif
			printf ("Cache %d: ", i);
			switch ((cache_info[0] >> 5) & 7) {
			case 1: printf ("L1 "); break;
			case 2: printf ("L2 "); break;
			case 3: printf ("L3 "); break;
			}
			switch (cache_info[0] & 31) {
			case 1: printf ("data cache,        "); break;
			case 2: printf ("instruction cache, "); break;
			case 3: printf ("unified cache,     "); break;
			}
			uint32_t n_ways = 1 + (cache_info[1] >> 22);
			uint32_t line_size = 1 + (cache_info[1] & 2047);
			uint32_t n_sets = 1 + cache_info[2];
			printf ("line size %d, ", line_size);
			printf ("%2d-way%s, ", n_ways, n_ways>1 ? "s" : "");
			printf ("%5d sets, ", n_sets);
			unsigned size = (n_ways * line_size * n_sets) >> 10;
			printf ("size %dk ", size);
			puts ("");
			i++;
		} 
	}

	if (!cpu_has_sse41)
		use_sse4 = false;
	if (!cpu_has_sse2)
		use_sse2 = false;

	println (L"\nNotation: B = byte, kB = 1024 B, MB = 1048576 B.");

	flush ();

	//------------------------------------------------------------
	// Attempt to obtain information about the CPU.
	//
#ifdef __linux__
	struct stat st;
	if (!stat ("/proc/cpuinfo", &st)) {
#define TMPFILE "/tmp/bandw_tmp"
		unlink (TMPFILE);
		if (-1 == system ("grep MHz /proc/cpuinfo | uniq | sed \"s/[\\t\\n: a-zA-Z]//g\" > "TMPFILE))
			perror ("system");

		FILE *f = fopen (TMPFILE, "r");
		if (f) {
			float cpu_speed = 0.0;

			if (1 == fscanf (f, "%g", &cpu_speed)) {
				puts ("");
				printf ("CPU speed is %g MHz.\n", cpu_speed);
			}
			fclose (f);
		}
	} else {
		printf ("CPU information is not available (/proc/cpuinfo).\n");
	}
	fflush (stdout);
#endif

	graph = BMPGraphing_new (GRAPH_WIDTH, GRAPH_HEIGHT, MODE_X_AXIS_LOG2);
	strcpy (graph_title, TITLE_MEMORY_GRAPH);
	BMPGraphing_set_title (graph, graph_title);

	//------------------------------------------------------------
	// SSE2 sequential reads.
	//
	if (use_sse2) {
		BMPGraphing_new_line (graph, "Sequential 128-bit reads", RGB_RED);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			int amount = do_read (chunk_size, SSE2, false);
			BMPGraphing_add_point (graph, chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// AVX sequential reads.
	//
	if (cpu_has_avx) {
		BMPGraphing_new_line (graph, "Sequential 256-bit reads", RGB_TURQUOISE);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_read (chunk_size, AVX, false);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// SSE2 random reads.
	//
	if (use_sse2) {
		BMPGraphing_new_line (graph, "Random 128-bit reads", RGB_MAROON);

		newline ();
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_read (chunk_size, SSE2, true);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// SSE2 sequential writes that do not bypass the caches.
	//
	if (use_sse2) {
		BMPGraphing_new_line (graph, "Sequential 128-bit cache writes", RGB_PURPLE);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			int amount = do_write (chunk_size, SSE2, false);
			BMPGraphing_add_point (graph, chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// AVX sequential writes that do not bypass the caches.
	//
	if (cpu_has_avx) {
		BMPGraphing_new_line (graph, "Sequential 256-bit cache writes", RGB_PINK);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_write (chunk_size, AVX, false);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// SSE2 random writes that do not bypass the caches.
	//
	if (use_sse2) {
		BMPGraphing_new_line (graph, "Random 128-bit cache writes", RGB_NAVYBLUE);

		newline ();
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_write (chunk_size, SSE2, true);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// SSE4 sequential reads that do bypass the caches.
	//
	if (use_sse4) {
		BMPGraphing_new_line (graph, "Sequential 128-bit bypassing reads", RGB_BLACK);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			int amount = do_read (chunk_size, SSE2_BYPASS, false);
			BMPGraphing_add_point (graph, chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// SSE4 random reads that do bypass the caches.
	//
	if (use_sse4) {
		BMPGraphing_new_line (graph, "Random 128-bit bypassing reads", 0xdeadbeef);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_read (chunk_size, SSE2_BYPASS, true);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// SSE4 sequential writes that do bypass the caches.
	//
	if (use_sse4) {
		BMPGraphing_new_line (graph, "Sequential 128-bit bypassing writes", RGB_DARKORANGE);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			int amount = do_write (chunk_size, SSE2_BYPASS, false);
			BMPGraphing_add_point (graph, chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// AVX sequential writes that do bypass the caches.
	// Currently on Intel CPUs (including Xeon) there is a 
	// microcode bug that leads to a severe drop in performance
	// in this part of the test.
	//
	if (cpu_has_avx) {
		BMPGraphing_new_line (graph, "Sequential 256-bit bypassing writes", RGB_DARKOLIVEGREEN);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_write (chunk_size, AVX_BYPASS, false);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// SSE4 random writes that bypass the caches.
	//
	if (use_sse4) {
		BMPGraphing_new_line (graph, "Random 128-bit bypassing writes", RGB_LEMONYELLOW);

		newline ();
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_write (chunk_size, SSE2_BYPASS, true);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// Sequential non-SSE2 reads.
	//
	newline ();
#ifdef __x86_64__
	BMPGraphing_new_line (graph, "Sequential 64-bit reads", RGB_BLUE);
#else
	BMPGraphing_new_line (graph, "Sequential 32-bit reads", RGB_BLUE);
#endif

	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		int amount = do_read (chunk_size, NO_SSE2, false);
		BMPGraphing_add_point (graph, chunk_size, amount);
	}

	//------------------------------------------------------------
	// Random non-SSE2 reads.
	//
	newline ();
#ifdef __x86_64__
	BMPGraphing_new_line (graph, "Random 64-bit reads", RGB_CYAN);
#else
	BMPGraphing_new_line (graph, "Random 32-bit reads", RGB_CYAN);
#endif
	srand (time (NULL));
	
	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		if (!(chunk_size & 128)) {
			int amount = do_read (chunk_size, NO_SSE2, true);
			BMPGraphing_add_point (graph, chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// Sequential non-SSE2 writes.
	//
#ifdef __x86_64__
	BMPGraphing_new_line (graph, "Sequential 64-bit writes", RGB_DARKGREEN);
#else
	BMPGraphing_new_line (graph, "Sequential 32-bit writes", RGB_DARKGREEN);
#endif

	newline ();

	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		int amount = do_write (chunk_size, NO_SSE2, false);
		BMPGraphing_add_point (graph, chunk_size, amount);
	}

	//------------------------------------------------------------
	// Random non-SSE2 writes.
	//
#ifdef __x86_64__
	BMPGraphing_new_line (graph, "Random 64-bit writes", RGB_GREEN);
#else
	BMPGraphing_new_line (graph, "Random 32-bit writes", RGB_GREEN);
#endif

	newline ();
	srand (time (NULL));

	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		if (!(chunk_size & 128)) {
			int amount = do_write (chunk_size, NO_SSE2, true);
			BMPGraphing_add_point (graph, chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// SSE2 sequential copy.
	//
	if (use_sse2) {
		BMPGraphing_new_line (graph, "Sequential 128-bit copy", 0x8f8844);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			int amount = do_copy (chunk_size, SSE2);
			BMPGraphing_add_point (graph, chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// AVX sequential copy.
	//
	if (cpu_has_avx) {
		BMPGraphing_new_line (graph, "Sequential 256-bit copy", RGB_CHARTREUSE);

		newline ();

		i = 0;
		while ((chunk_size = chunk_sizes [i++])) {
			if (!(chunk_size & 128)) {
				int amount = do_copy (chunk_size, AVX);
				BMPGraphing_add_point (graph, chunk_size, amount);
			}
		}
	}

#ifdef DOING_LODS
#ifdef __x86_64__
	//------------------------------------------------------------
	// LODSQ 64-bit sequential reads.
	//
	BMPGraphing_new_line (graph, "Sequential 64-bit LODSQ reads", RGB_GRAY6);

	newline ();

	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		int amount = do_read (chunk_size, LODSQ, false);
		BMPGraphing_add_point (graph, chunk_size, amount);
	}
#endif

	//------------------------------------------------------------
	// LODSD 32-bit sequential reads.
	//
	BMPGraphing_new_line (graph, "Sequential 32-bit LODSD reads", RGB_GRAY8);

	newline ();

	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		int amount = do_read (chunk_size, LODSD, false);
		BMPGraphing_add_point (graph, chunk_size, amount);
	}

	//------------------------------------------------------------
	// LODSW 16-bit sequential reads.
	//
	BMPGraphing_new_line (graph, "Sequential 16-bit LODSW reads", RGB_GRAY10);

	newline ();

	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		int amount = do_read (chunk_size, LODSW, false);
		BMPGraphing_add_point (graph, chunk_size, amount);
	}

	//------------------------------------------------------------
	// LODSB 64-bit sequential reads.
	//
	BMPGraphing_new_line (graph, "Sequential 8-bit LODSB reads", RGB_GRAY12);

	newline ();

	i = 0;
	while ((chunk_size = chunk_sizes [i++])) {
		int amount = do_read (chunk_size, LODSB, false);
		BMPGraphing_add_point (graph, chunk_size, amount);
	}
#endif

	//------------------------------------------------------------
	// Register to register.
	//
	newline ();
	register_test ();

	//------------------------------------------------------------
	// Stack to/from register.
	//
	newline ();
	stack_test ();

	//------------------------------------------------------------
	// C library performance.
	//
	newline ();
	library_test ();

	//------------------------------------------------------------
	// Framebuffer read & write.
	//
#if defined(__linux__) && defined(FBIOGET_FSCREENINFO)
	newline ();
	fb_readwrite (true);
#endif

premature_end_for_testing:
	flush ();

	BMPGraphing_make (graph);

	BMP_write (graph->image, "bandwidth.bmp");

	puts ("\nWrote graph to bandwidth.bmp.");
	puts ("");
	puts ("Done.");

	BMPGraphing_destroy (graph);

	return 0;
}
