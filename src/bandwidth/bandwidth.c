/*============================================================================
  bandwidth, a benchmark to estimate memory transfer bandwidth.
  Copyright (C) 2005-2016 by Zack T Smith.

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

#define VREGISTER_COUNT 3333

#ifndef __arm__
#define x86
#endif

#define TITLE_MEMORY_NET "Network benchmark results from bandwidth " RELEASE " by Zack Smith, http://zsmith.co"
#define TITLE_MEMORY_GRAPH "Memory benchmark results from bandwidth " RELEASE " by Zack Smith, http://zsmith.co"

#ifdef __WIN32__
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#endif

/* Needed by CPU-X */
#include "../cpu-x.h"
#include "libbandwidth.h"
#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
#endif

#ifdef __x86_64__
#define IS_64BIT
#endif

static enum {
	OUTPUT_MODE_GRAPH=0,
	OUTPUT_MODE_CSV=1,
} outputMode;

static int network_port = NETWORK_DEFAULT_PORTNUM;

enum {
	NO_SSE2,

	// x86
	SSE2,
	SSE2_BYPASS,
	AVX,
	AVX_BYPASS,
	LODSQ,
	LODSD,
	LODSW,
	LODSB,

	// ARM
	NEON_64BIT,
	NEON_128BIT,
};

static BMPGraph *graph = NULL;

#ifndef __arm__
static bool use_sse2 = true;
static bool use_sse4 = true;
#else
static bool use_sse2 = false;
static bool use_sse4 = false;
#endif
static bool is_intel = false;
static bool is_amd = false;

static FILE *csv_output_file = NULL;
static char *csv_file_path = NULL;

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

static long usec_per_test = 5000;	// 5 ms per memory test.

static int chunk_sizes[] = {
#ifdef x86
	128,
#endif
	256,
#ifdef x86
	384,
#endif
	512,
#ifdef x86
	640,
#endif
	768,
#ifdef x86
	896,
#endif
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
#ifndef __arm__
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
#define TEST_L4
#ifdef TEST_L4
	160 * 1024 * 1024,
	192 * 1024 * 1024,
	224 * 1024 * 1024,
	256 * 1024 * 1024,
	320 * 1024 * 1024,
	512 * 1024 * 1024,
#endif
#endif
	0
};

static double chunk_sizes_log2 [sizeof(chunk_sizes)/sizeof(int)];

//----------------------------------------------------------------------------
// Name:	internal
// Purpose:	Complain and exit.
//----------------------------------------------------------------------------
void internal (char *s)
{
	fprintf (stderr, "Internal error: %s\n", s);
	exit (2);
}

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
// Output multiplexor.
//============================================================================

void dataBegins (char *title)
{
	switch (outputMode) {
	case OUTPUT_MODE_GRAPH:
		if (graph)
			internal ("Graphing already initialized.");

		graph = BMPGraphing_new (GRAPH_WIDTH, GRAPH_HEIGHT, MODE_X_AXIS_LOG2);
		BMPGraphing_set_title (graph, title? title : TITLE_MEMORY_GRAPH);
		break;

	case OUTPUT_MODE_CSV:
		if (csv_output_file)
			internal ("CSV file already initialized.");

		csv_output_file = fopen (csv_file_path, "wb");
		if (!csv_output_file) {
			error ("Cannot open CSV output file.");
		}
		if (title)
			fprintf (csv_output_file, "%s\n", title);
		break;

	default:
		internal ("Bad output mode.");
	}
}

void dataBeginSection (const char *name, uint32_t parameter)
{
	if (!name)
		internal ("dataBeginSection: NULL name.");

	switch (outputMode) {
	case OUTPUT_MODE_GRAPH:
		if (!graph)
			internal ("Graphing not initialized.");

		BMPGraphing_new_line (graph, name, parameter);
		break;

	case OUTPUT_MODE_CSV:
		if (!csv_output_file)
			internal ("CSV output not initialized.");

		fprintf (csv_output_file, "%s\n", name);
		break;

	default:
		internal ("Bad output mode.");
	}
}

void dataEnds (const char *parameter)
{
	switch (outputMode) {
	case OUTPUT_MODE_GRAPH:
		if (!graph)
			internal ("Graphing not initialized.");
		if (!parameter)
			internal ("dataEnds: NULL name.");

		BMPGraphing_make (graph);
		BMP_write (graph->image, parameter);
		BMPGraphing_destroy (graph);
		graph= NULL;
		puts ("Wrote graph to bandwidth.bmp.");
		break;

	case OUTPUT_MODE_CSV:
		if (!csv_output_file)
			internal ("CSV output not initialized.");
		fclose (csv_output_file);
		printf ("Wrote %s.\n", csv_file_path);
		break;

	default:
		internal ("Bad output mode.");
	}
}

void dataAddDatum (Value x, Value y)
{
	switch (outputMode) {
	case OUTPUT_MODE_GRAPH:
		if (!graph)
			internal ("Graphing not initialized.");

		BMPGraphing_add_point (graph, x, y);
		break;

	case OUTPUT_MODE_CSV:
		if (!csv_output_file)
			internal ("CSV output not initialized.");

		fprintf (csv_output_file, "%lld, %.1Lf\n", (long long)x, (long double)y/10.);
		fflush (csv_output_file);
		break;

	default:
		internal ("Bad output mode.");
	}
}

//============================================================================
// Output buffer logic.
//============================================================================

void print (const char *s)
{
	if(BANDWIDTH_MODE)
		printf ("%s",s);
}

void newline ()
{
	if(BANDWIDTH_MODE)
		puts ("");
}

void println (char *s)
{
	print (s);
	newline ();
}

void print_int (int d)
{
	if(BANDWIDTH_MODE)
		printf ("%d", d);
}

void print_uint (unsigned long d)
{
	if(BANDWIDTH_MODE)
		printf ("%lu", d);
}

void println_int (int d)
{
	print_int (d);
	newline ();
}

void print_result (long double result)
{
	if(BANDWIDTH_MODE)
		printf ("%.1Lf MB/s", result);
}

void flush ()
{
	fflush (stdout);
}

void print_size (unsigned long size)
{
	if (size < 1536) {
		print_int (size);
		print (" B");
	}
	else if (size < (1<<20)) {
		print_int (size >> 10);
		print (" kB");
	} else {
		print_int (size >> 20);
		switch ((size >> 18) & 3) {
		case 1: print (".25"); break;
		case 2: print (".5"); break;
		case 3: print (".75"); break;
		}
		print (" MB");
	}
}

//============================================================================
// Timing logic.
//============================================================================

//----------------------------------------------------------------------------
// Name:	mytime
// Purpose:	Reports time in microseconds.
//----------------------------------------------------------------------------
unsigned long
mytime ()
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
#ifdef IS_64BIT
	unsigned long value = 0x1234567689abcdef;
#else
	unsigned long value = 0x12345678;
#endif
	unsigned long diff=0, t0;
	unsigned long tmp;
	unsigned long **chunk_ptrs = NULL;

#ifdef x86
	if (size & 127)
		error ("do_write(): chunk size is not multiple of 128.");
#else
	if (size & 255)
		error ("do_write(): chunk size is not multiple of 256.");
#endif

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
		print ("Random write ");
	else
		print ("Sequential write ");

	switch (mode) {
	case NEON_64BIT:
		print ("(64-bit), size = ");
		break;
	case NEON_128BIT:
		print ("(128-bit), size = ");
		break;
	case SSE2:
		print ("(128-bit), size = ");
		break;
	case AVX:
		print ("(256-bit), size = ");
		break;
	case AVX_BYPASS:
                print ("bypassing cache (256-bit), size = ");
		break;
	case SSE2_BYPASS:
                print ("bypassing cache (128-bit), size = ");
		break;
	default:
#ifdef IS_64BIT
		print ("(64-bit), size = ");
#else
		print ("(32-bit), size = ");
#endif
	}

	print_size (size);
	print (", ");

	loops = (1 << 26) / size;// XX need to adjust for CPU MHz
	if (loops < 1)
		loops = 1;

	t0 = mytime ();

	while (diff < usec_per_test) {
		total_count += loops;

		switch (mode) {
#ifdef __arm__
                case NEON_64BIT:
			if (random)
				RandomWriterVector (chunk_ptrs, size/256, loops, value);
                        break;
                case NEON_128BIT:
			if (!random)
                        	WriterVector (chunk, size, loops, value);
                        break;
#endif

#ifdef x86
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
#endif

		default:
			if (random)
				RandomWriter (chunk_ptrs, size/256, loops, value);
			else {
#ifdef x86
				if (size & 128)
					Writer_128bytes (chunk, size, loops, value);
				else
#endif
					Writer (chunk, size, loops, value);
			}
		}

		diff = mytime () - t0;
	}

	print ("loops = ");
	print_uint (total_count);
	print (", ");

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
		print ("Random read ");
	else
		print ("Sequential read ");

	switch (mode) {
#ifdef __arm__
	case NEON_64BIT:
		print ("(64-bit), size = ");
		break;
	case NEON_128BIT:
		print ("(128-bit), size = ");
		break;
#endif

#ifdef x86
	case SSE2:
		print ("(128-bit), size = ");
		break;
	case LODSB:
		print ("(8-bit LODSB), size = ");
		break;
	case LODSW:
		print ("(16-bit LODSW), size = ");
		break;
	case LODSD:
		print ("(32-bit LODSD), size = ");
		break;
	case LODSQ:
		print ("(64-bit LODSQ), size = ");
		break;
	case AVX:
		print ("(256-bit), size = ");
		break;
	case AVX_BYPASS:
                print ("bypassing cache (256-bit), size = ");
		break;
	case SSE2_BYPASS:
                print ("bypassing cache (128-bit), size = ");
		break;
#endif

	default:
#ifdef IS_64BIT
		print ("(64-bit), size = ");
#else
		print ("(32-bit), size = ");
#endif
	}

	print_size (size);
	print (", ");

	flush ();

	loops = (1 << 26) / size;	// XX need to adjust for CPU MHz
	if (loops < 1)
		loops = 1;

	t0 = mytime ();

	while (diff < usec_per_test) {
		total_count += loops;

		switch (mode) {
#ifdef __arm__
		case NEON_64BIT:
			if (random)
				RandomReaderVector (chunk_ptrs, size/256, loops);
			break;
		case NEON_128BIT:
			if (!random)
				ReaderVector (chunk, size, loops);
			break;
#endif

#ifdef x86
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
#endif

		default:
			if (random) {
				RandomReader (chunk_ptrs, size/256, loops);
			} else {
#ifdef x86
				if (size & 128)
					Reader_128bytes (chunk, size, loops);
				else
#endif
					Reader (chunk, size, loops);
			}
		}

		diff = mytime () - t0;
	}

	print ("loops = ");
	print_uint (total_count);
	print (", ");

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
do_copy (unsigned long size, int mode, bool random)
{
#ifdef x86
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
	print ("Sequential copy ");

	if (mode == SSE2) {
		print ("(128-bit), size = ");
	}
	else if (mode == AVX) {
		print ("(256-bit), size = ");
	}
	else {
#ifdef IS_64BIT
		print ("(64-bit), size = ");
#else
		print ("(32-bit), size = ");
#endif
	}

	print_size (size);
	print (", ");

	flush ();

	loops = (1 << 26) / size;	// XX need to adjust for CPU MHz
	if (loops < 1)
		loops = 1;

	t0 = mytime ();

	while (diff < usec_per_test) {
		total_count += loops;

		if (mode == SSE2)  {
#ifdef IS_64BIT
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

	print ("loops = ");
	print_uint (total_count);
	print (", ");

	int result = calculate_result (size, total_count, diff);
	newline ();

	flush ();

	free (chunk_src0);
	free (chunk_dest0);

	return result;
#else
	return 0;
#endif
}


//----------------------------------------------------------------------------
// Name:	fb_readwrite
// Purpose:	Performs sequential read & write tests on framebuffer memory.
//----------------------------------------------------------------------------
#if defined(__linux__) && defined(FBIOGET_FSCREENINFO)
void
fb_readwrite (bool use_sse2)
{
	unsigned long total_count;
	unsigned long length;
	unsigned long diff, t0;
	static struct fb_fix_screeninfo fi;
	static struct fb_var_screeninfo vi;
	unsigned long *fb = NULL;
	int fd;
#ifdef IS_64BIT
	unsigned long value = 0x1234567689abcdef;
#else
	unsigned long value = 0x12345678;
#endif

	//-------------------------------------------------

	fd = open ("/dev/fb0", O_RDWR);
	if (fd < 0)
		fd = open ("/dev/fb/0", O_RDWR);
	if (fd < 0) {
		println ("Cannot open framebuffer device.");
		return;
	}

	if (ioctl (fd, FBIOGET_FSCREENINFO, &fi)) {
		close (fd);
		println ("Cannot get framebuffer info");
		return;
	}
	else
	if (ioctl (fd, FBIOGET_VSCREENINFO, &vi)) {
		close (fd);
		println ("Cannot get framebuffer info");
		return;
	}
	else
	{
		if (fi.visual != FB_VISUAL_TRUECOLOR &&
		    fi.visual != FB_VISUAL_DIRECTCOLOR ) {
			close (fd);
			println ("Need direct/truecolor framebuffer device.");
			return;
		} else {
			unsigned long fblen;

			print ("Framebuffer resolution: ");
			print_int (vi.xres);
			print ("x");
			print_int (vi.yres);
			print (", ");
			print_int (vi.bits_per_pixel);
			println (" bpp\n");

			fb = (unsigned long*) fi.smem_start;
			fblen = fi.smem_len;

			fb = mmap (fb, fblen,
				PROT_WRITE | PROT_READ,
				MAP_SHARED, fd, 0);
			if (fb == MAP_FAILED) {
				close (fd);
				println ("Cannot access framebuffer memory.");
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
	print ("Framebuffer memory sequential read ");
	flush ();

	t0 = mytime ();

	total_count = FBLOOPS_R;

#ifdef x86
	if (use_sse2)
		ReaderSSE2 (fb, length, FBLOOPS_R);
	else
#endif
		Reader (fb, length, FBLOOPS_R);

	diff = mytime () - t0;

	calculate_result (length, total_count, diff);
	newline ();

	//-------------------
	// WRITE
	//
	print ("Framebuffer memory sequential write ");
	flush ();

	t0 = mytime ();

	total_count = FBLOOPS_W;

#ifdef x86
	if (use_sse2)
		WriterSSE2_bypass (fb, length, FBLOOPS_W, value);
	else
#endif
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
#ifdef IS_64BIT
	print ("Main register to main register transfers (64-bit) ");
#else
	print ("Main register to main register transfers (32-bit) ");
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
#ifdef x86
#ifdef IS_64BIT
	print ("Main register to vector register transfers (64-bit) ");
#else
	print ("Main register to vector register transfers (32-bit) ");
#endif
	flush ();

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
#ifdef IS_64BIT
	print ("Vector register to main register transfers (64-bit) ");
#else
	print ("Vector register to main register transfers (32-bit) ");
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
#endif

	//--------------------------------------
	print ("Vector register to vector register transfers (128-bit) ");
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

#ifdef x86
	//--------------------------------------
	if (cpu_has_avx) {
		print ("Vector register to vector register transfers (256-bit) ");
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
		print ("Vector 8-bit datum to main register transfers ");
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
	print ("Vector 16-bit datum to main register transfers ");
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
		print ("Vector 32-bit datum to main register transfers ");
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
		print ("Vector 64-bit datum to main register transfers ");
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
		print ("Main register 8-bit datum to vector register transfers ");
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
	print ("Main register 16-bit datum to vector register transfers ");
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
		print ("Main register 32-bit datum to vector register transfers ");
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
		print ("Main register 64-bit datum to vector register transfers ");
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
#endif
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

#ifdef IS_64BIT
	print ("Stack-to-register transfers (64-bit) ");
#else
	print ("Stack-to-register transfers (32-bit) ");
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

#ifdef IS_64BIT
	print ("Register-to-stack transfers (64-bit) ");
#else
	print ("Register-to-stack transfers (32-bit) ");
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

	print ("Library: memset ");
	calculate_result (NT_SIZE, NT_SIZE2, t-t0);
	newline ();

	flush ();

	//--------------------------------------
	t0 = mytime ();
	for (i=0; i<NT_SIZE2; i++) {
		memcpy (a2, a1, NT_SIZE);
	}
	t = mytime ();

	print ("Library: memcpy ");
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

			dataAddDatum (amt_in_kb, tmp);

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

			dataAddDatum (amt_in_kb, recv_rates[j-NETSIZE_MIN]);
		}

		newline ();
	}

	return true;
}

//----------------------------------------------------------------------------
// Name:	usage
//----------------------------------------------------------------------------
void
usage ()
{
	printf ("Usage: bandwidth [--slow] [--fast] [--faster] [--fastest] [--title string] [--csv file]\n");
        //printf ("Usage for starting network tests: bandwidth --network <ipaddr1> [<ipaddr2...] [--port <port#>]\n");
        //printf ("Usage for receiving network tests: bandwidth --transponder [--port <port#>]\n");

	exit (0);
}

//----------------------------------------------------------------------------
// Name:	main
//----------------------------------------------------------------------------
int bandwidth(void *p_data)
{
	int t, i, chunk_size;
	outputMode = OUTPUT_MODE_GRAPH;
	char graph_title [512] = {0};

	/* Needed by CPU-X */
	int count             = 0;
	uint8_t cache_level   = 0;
	double total_amount   = 0;
	Labels *data          = p_data;
	BandwidthData *w_data = (p_data != NULL) ? data->w_data : NULL;

	for (i = 0; chunk_sizes[i] && i < sizeof(chunk_sizes)/sizeof(int); i++) {
		chunk_sizes_log2[i] = log2 (chunk_sizes[i]);
	}

	if(BANDWIDTH_MODE)
	{
		printf ("This is bandwidth version %s (built-in with %s).\n", RELEASE, PRGNAME);
		printf ("Copyright (C) 2005-2016 by Zack T Smith.\n\n");
		printf ("This software is covered by the GNU Public License.\n");
		printf ("It is provided AS-IS, use at your own risk.\n");
		printf ("See the file COPYING for more information.\n\n");
		fflush (stdout);
	}

	uint32_t ecx = get_cpuid1_ecx ();
	uint32_t edx = get_cpuid1_edx ();
	uint32_t ecx2 = get_cpuid_80000001_ecx ();
	uint32_t edx2 = get_cpuid_80000001_edx ();
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
	cpu_has_xd = edx2 & CPUID_EDX_XD;
	cpu_has_64bit = edx2 & CPUID_EDX_INTEL64;
	cpu_has_sse4a = 0;

	if (cpu_has_avx) {
		cpu_has_avx2 = get_cpuid7_ebx ();
		cpu_has_avx2 &= CPUID_EBX_AVX2;
	}

	/* Needed by CPU-X */
	if (!cpu_has_sse41)
		use_sse4 = false;
	if (!cpu_has_sse2)
		use_sse2 = false;

	const struct Tests tests[] =
	{
		/* Test              Name                                   Color               Flag         Mask   Func      Mode         Rand */
		{ SEQ_128_R,         "Sequential 128-bit reads",            RGB_RED,            use_sse2,    false, do_read,  SSE2,        false },
		{ SEQ_256_R,         "Sequential 256-bit reads",            RGB_TURQUOISE,      cpu_has_avx, true,  do_read,  AVX,         false },
		{ RAND_128_R,        "Random 128-bit reads",                RGB_MAROON,         use_sse2,    true,  do_read,  SSE2,        true  },
		{ SEQ_128_CACHE_W,   "Sequential 128-bit cache writes",     RGB_PURPLE,         use_sse2,    false, do_write, SSE2,        false },
		{ SEQ_256_CACHE_W,   "Sequential 256-bit cache writes",     RGB_PINK,           cpu_has_avx, true,  do_write, AVX,         false },
		{ RAND_128_CACHE_W,  "Random 128-bit cache writes",         RGB_NAVYBLUE,       use_sse2,    true,  do_write, SSE2,        true  },
		{ SEQ_128_BYPASS_R,  "Sequential 128-bit bypassing reads",  0xA04040,           use_sse4,    false, do_read,  SSE2_BYPASS, false },
		{ RAND_128_BYPASS_R, "Random 128-bit bypassing reads",      0x301934,           use_sse4,    true,  do_read,  SSE2_BYPASS, true  },
		{ SEQ_128_BYPASS_W,  "Sequential 128-bit bypassing writes", RGB_DARKORANGE,     use_sse4,    false, do_write, SSE2_BYPASS, false },
		{ SEQ_256_BYPASS_W,  "Sequential 256-bit bypassing writes", RGB_DARKOLIVEGREEN, cpu_has_avx, true,  do_write, AVX_BYPASS,  false },
		{ RAND_128_BYPASS_W, "Random 128-bit bypassing writes",     RGB_LEMONYELLOW,    use_sse4,    true,  do_write, SSE2_BYPASS, true  },
		{ SEQ_128_C,         "Sequential 128-bit copy",             0x8f8844,           use_sse2,    false, do_copy,  SSE2,        false },
		{ SEQ_256_C,         "Sequential 256-bit copy",             RGB_CHARTREUSE,     cpu_has_avx, true,  do_copy,  AVX,         false },
		{ SEQ_32_LR,         "Sequential 32-bit LODSD reads",       RGB_GRAY8,          true,        false, do_read,  LODSD,       false },
		{ SEQ_16_LR,         "Sequential 16-bit LODSW reads",       RGB_GRAY10,         true,        false, do_read,  LODSW,       false },
		{ SEQ_8_LR,          "Sequential 8-bit LODSB reads",        RGB_GRAY12,         true,        false, do_read,  LODSB,       false },
#ifdef __x86_64__
		{ SEQ_64_LR,         "Sequential 64-bit LODSQ reads",       RGB_GRAY6,          true,        false, do_read,  LODSQ,       false },
		{ SEQ_64_R,          "Sequential 64-bit reads",             RGB_BLUE,           true,        false, do_read,  NO_SSE2,     false },
		{ RAND_64_R,         "Random 64-bit reads",                 RGB_CYAN,           true,        true,  do_read,  NO_SSE2,     true  },
		{ SEQ_64_W,          "Sequential 64-bit writes",            RGB_DARKGREEN,      true,        false, do_write, NO_SSE2,     false },
		{ RAND_64_W,         "Random 64-bit writes",                RGB_GREEN,          true,        true,  do_write, NO_SSE2,     true  },

#else
		{ SEQ_32_R,          "Sequential 32-bit reads",             RGB_BLUE,           true,        false, do_read,  NO_SSE2,     false },
		{ RAND_32_R,         "Random 32-bit reads",                 RGB_CYAN,           true,        true,  do_read,  NO_SSE2,     true  },
		{ SEQ_32_W,          "Sequential 32-bit writes",            RGB_DARKGREEN,      true,        false, do_write, NO_SSE2,     false },
		{ RAND_32_W,         "Random 32-bit writes",                RGB_GREEN,          true,        true,  do_write, NO_SSE2,     true  },
#endif
		{ -1,                NULL,                                  0x0,                false,       false, NULL,     0,           false }
	};

#if HAS_LIBCPUID
	if(!BANDWIDTH_MODE)
		goto fast_initialization;
#endif /* HAS_LIBCPUID */

	static char family [17];
	get_cpuid_family (family);
	family [16] = 0;
	printf ("CPU family: %s\n", family);

	if (!strcmp ("AuthenticAMD", family)) {
		is_amd = true;
		cpu_has_sse4a = ecx2 & CPUID_ECX_SSE4A;
	}
	else
	if (!strcmp ("GenuineIntel", family)) {
		is_intel = true;
	}

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
			newline ();
			i++;
		}
	}

	newline ();

	println ("Notation: B = byte, kB = 1024 B, MB = 1048576 B.");

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
				newline ();
				printf ("CPU speed is %g MHz.\n", cpu_speed);
			}
			fclose (f);
		}
	} else {
		printf ("CPU information is not available (/proc/cpuinfo).\n");
	}
	fflush (stdout);
#endif

	dataBegins (graph_title);
	goto end_initialization;

#if HAS_LIBCPUID
fast_initialization:
	switch(data->l_data->cpu_vendor_id)
	{
		case VENDOR_INTEL:
			is_intel = true;
			break;
		case VENDOR_AMD:
			is_amd = true;
			cpu_has_sse4a = ecx2 & CPUID_ECX_SSE4A;
			break;
		default:
			break;
	}
#endif /* HAS_LIBCPUID */

end_initialization:

	if(!BANDWIDTH_MODE)
	{
		/* Check if selectionned test is valid */
		if(opts->bw_test >= LASTTEST)
			opts->bw_test = 0;

		/* Set test names */
		if(w_data->test_count == 0)
		{
			w_data->test_count = LASTTEST;
			w_data->test_name  = malloc(LASTTEST * sizeof(char *));
			if(w_data->test_name == NULL)
				w_data->test_count = 0;

			for(i = 0; i < w_data->test_count; i++)
			{
				w_data->test_name[i] = NULL;
				casprintf(&w_data->test_name[i], false, "#%2i: %s", i, tests[i].name);
			}
		}
	}

	for(t = (BANDWIDTH_MODE) ? 0 : opts->bw_test; tests[t].name != NULL; t++)
	{
		if(tests[t].need_flag)
		{
			if(BANDWIDTH_MODE)
			{
				dataBeginSection(tests[t].name, tests[t].color);
				newline ();
			}

			if(tests[t].random)
				srand(time (NULL));

			i = 0;
			while((chunk_size = chunk_sizes [i++]))
			{
				if(!tests[t].need_mask || (tests[t].need_mask && !(chunk_size & 128)))
				{
					if(!BANDWIDTH_MODE && chunk_size > w_data->size[cache_level] * 1024)
					{
						w_data->speed[cache_level] = total_amount / count;
						count        = 0;
						total_amount = 0;
						cache_level++;

						if(cache_level >= data->cache_count || w_data->size[cache_level] < 1)
							return 0;
					}

					int amount = (*tests[t].func_ptr)(chunk_size, tests[t].mode, tests[t].random);

					if(BANDWIDTH_MODE)
						dataAddDatum (chunk_size, amount);
					else
					{
						total_amount += amount;
						count++;
					}
				}
			}
		}
		else
		{
			for(i = 0; i < LASTCACHES / CACHEFIELDS; i++)
				w_data->speed[i] = 0;
			return 1;
		}

		if(!BANDWIDTH_MODE)
			return 0;
	}

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

	flush ();

	newline ();
	dataEnds ("bandwidth.bmp");

	newline ();
	puts ("Done.");

	return 0;
}
