/*============================================================================
  bandwidth, a benchmark to estimate memory transfer bandwidth.
  Copyright (C) 2005-2024 by Zack T Smith.

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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define GRAPH_WIDTH 1920
#define GRAPH_HEIGHT 1080

#include "defs.h"
#include "OOC/MutableImage.h"
#include "OOC/SimpleGraphing.h"
#include "OOC/Console.h"
#include "OOC/OperatingSystem.h"
#include "OOC/CPU.h"
#include "OOC/Hardware.h"
#include "OOC/colors.h"
#include "TestingX86.h"
#include "TestingARM.h"
#include "TestingRISCV.h"

#define TITLE_MEMORY_GRAPH "Results from ''bandwidth'' " RELEASE " by Zack Smith, https://zsmith.co"

#if defined(__WIN32__) || defined(__WIN64__)
#include <w32api/windows.h> // Cygwin
#include <inttypes.h>
#endif

/* Needed by CPU-X */
#include <pthread.h>
#include "libbandwidth.h"
#include "../../util.hpp"
#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
#endif

static enum {
	OUTPUT_MODE_GRAPH=1,
	OUTPUT_MODE_CSV=2,
} outputMode;

static FILE *csv_output_file = NULL;
static const char *csv_file_path = NULL;

// Mode to be nice and to keep CPU temperature low, since Mac x86 laptops tend to overheat.
#if defined(__APPLE__) && !defined(__aarch64__)
  static bool nice_mode = true;
#else
  static bool nice_mode = false;
#endif
#define NICE_DURATION (2)

Console *console = NULL;
static SimpleGraphing *graph = NULL;
static bool do_invert_graph = false;
static bool do_rotate_graph = false;

unsigned long usec_per_test;
bool diagnostic_mode = false;

static size_t chunk_sizes[] = {
	256,
	512,
	768,
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
	72*1024,
	80*1024,
	96*1024,
	131072,	// Tiny L2 cache size from yesteryear.
	192 * 1024,
	256 * 1024,	// Old L2 cache size.
	320 * 1024,
	384 * 1024,
	480 * 1024,
	512 * 1024,	// Old L2 cache size.
	768 * 1024,
	ONE_MEGABYTE,	// 1 MB = common L2 cache size.
	(1024 + 256) * 1024,	// 1.25
	(1024 + 512) * 1024,	// 1.5
	(1024 + 768) * 1024,	// 1.75
	2 * ONE_MEGABYTE,	// 2 MB = common L2 cache size.
	(2048 + 256) * 1024,	// 2.25
	(2048 + 512) * 1024,	// 2.5
	(2048 + 768) * 1024,	// 2.75
	3 * ONE_MEGABYTE ,	// 3 MB = common L2 cache size.
	3407872, // 3.25 MB
	3 * ONE_MEGABYTE + 1024 * 512,	// 3.5 MB
	4 * ONE_MEGABYTE,	// 4 MB
	5 * ONE_MEGABYTE,	// 5 megs (Core i7-11xxxH has 5 MB L2)
	6 * ONE_MEGABYTE,	// 6 megs (common L2 cache size)
	7 * ONE_MEGABYTE,
	8 * ONE_MEGABYTE, // Xeon E3's often has 8MB L3
	9 * ONE_MEGABYTE,
	10 * ONE_MEGABYTE, // Xeon E5-2609 has 10MB L3
	11 * ONE_MEGABYTE,
	12 * ONE_MEGABYTE, // Core i7-11xx[x] often have 12MB L3
	13 * ONE_MEGABYTE,
	14 * ONE_MEGABYTE,
	15 * ONE_MEGABYTE, // Xeon E6-2630 has 15MB L3
	16 * ONE_MEGABYTE,
	20 * ONE_MEGABYTE, // Xeon E5-2690 has 20MB L3
	21 * ONE_MEGABYTE,
	32 * ONE_MEGABYTE,
	48 * ONE_MEGABYTE,
	64 * ONE_MEGABYTE,
	72 * ONE_MEGABYTE,
	96 * ONE_MEGABYTE,
	128 * ONE_MEGABYTE,
	160 * ONE_MEGABYTE,
	192 * ONE_MEGABYTE,
	224 * ONE_MEGABYTE,
	256 * ONE_MEGABYTE,
	320 * ONE_MEGABYTE,
	384 * ONE_MEGABYTE,
	480 * ONE_MEGABYTE,
	512 * ONE_MEGABYTE,
	640 * ONE_MEGABYTE,
	800 * ONE_MEGABYTE,
	1024 * ONE_MEGABYTE,
};
#define N_CHUNK_SIZES sizeof(chunk_sizes)/sizeof(unsigned long)

static double chunk_sizes_log2 [N_CHUNK_SIZES];

//============================================================================
// Output multiplexor.
//============================================================================

void dataBegins (MutableString *title, String *subtitle)
{
	require(outputMode) else { error (__FUNCTION__, "Bad output mode."); }
	require(title) else { error_null_parameter (__FUNCTION__); }
	//==========

	if (!title) {
		title = _MutableString(TITLE_MEMORY_GRAPH);
		subtitle = NULL;
	}
	else if (!subtitle) {
		subtitle = _String(TITLE_MEMORY_GRAPH);
	}

	if (outputMode & OUTPUT_MODE_GRAPH) {
		if (graph) {
			error (__FUNCTION__, "Graphing already initialized.");
		}

		graph = SimpleGraphing_withSize (GRAPH_WIDTH, GRAPH_HEIGHT);

		$(graph, setXAxisMode, MODE_X_AXIS_LOG2);
		if (title) {
			$(graph, setTitle, title);
		}
		if (subtitle) {
			$(graph, setSubtitle, subtitle);
		}
	}

	if (outputMode & OUTPUT_MODE_CSV) {
		if (csv_output_file) {
			error (__FUNCTION__, "CSV file already initialized.");
		}
		csv_output_file = fopen (csv_file_path, "wb");
		if (!csv_output_file) {
			error (__FUNCTION__, "Cannot open CSV output file.");
		}
		if (title) {
			$(title, print, csv_output_file);
			fputc (' ', csv_output_file);
		}
	}
}

void dataBeginSection (const char *name, uint32_t parameter)
{
	require(outputMode) else { error (__FUNCTION__, "Bad output mode."); }
	require(name) else { error_null_parameter (__FUNCTION__); }
	//==========

	if (nice_mode) {
		sleep (NICE_DURATION);

	}

	if (outputMode & OUTPUT_MODE_GRAPH) {
		if (!graph)
			error (__FUNCTION__, "Graphing not initialized.");

		$(graph, addLine, name, parameter);
	}

	if (outputMode & OUTPUT_MODE_CSV) {
		if (!csv_output_file)
			error (__FUNCTION__, "CSV output not initialized.");

		fprintf (csv_output_file, "%s\n", name);
	}
}

void dataEnds (const char *path)
{
	require(outputMode) else { error (__FUNCTION__, "Bad output mode."); }
	require(path) else { error_null_parameter (__FUNCTION__); }
	//==========

	if (outputMode & OUTPUT_MODE_GRAPH) {
		if (!graph)
			error (__FUNCTION__, "Graphing not initialized.");

		$(graph, make);

		MutableImage *image = $(graph, image);
		if (do_invert_graph) {
			$(image, invert);
		}
		if (do_rotate_graph) {
			$(image, rotate, MutableImageRotationAngle180);
		}

		//if ($(image, writeTIFF, path)) {
		if ($(image, writeBMP, path)) {
			$(console, printf, "Wrote graph to %s\n", path);
		} else {
			$(console, printf, "Failed to write graph to %s\n", path);
		}
	}

	if (outputMode & OUTPUT_MODE_CSV) {
		if (!csv_output_file)
			error (__FUNCTION__, "CSV output not initialized.");
		fclose (csv_output_file);
		$(console, puts, "Wrote CSV.");
	}
}

void dataAddDatum (long x, long y)
{
	require(outputMode) else { error (__FUNCTION__, "Bad output mode."); }
	//==========

	if (outputMode & OUTPUT_MODE_GRAPH) {
		if (!graph)
			error (__FUNCTION__, "Graphing not initialized.");

		$(graph, addPoint, x, y);
	}

	if (outputMode & OUTPUT_MODE_CSV) {
		if (!csv_output_file)
			error (__FUNCTION__, "CSV output not initialized.");

		fprintf (csv_output_file, "%lld, %.1Lf\n", (long long)x, (long double)y/10.);
		fflush (csv_output_file);
	}
}

//----------------------------------------------------------------------------
// Name:	usage
//----------------------------------------------------------------------------
void
usage ()
{
	puts ("Usage: bandwidth (options)");
	puts ("Options include:");
	puts ("  --slow");
	puts ("  --fast");
	puts ("  --faster");
	puts ("  --fastest");
#ifdef x86
	puts ("  --nosse2");
	puts ("  --nosse4");
	puts ("  --noavx");
	puts ("  --noavx512");
#endif
	puts ("  --norandom");
	puts ("  --nowrite");
	puts ("  --nonontemporal");
	puts ("  --noregister");
	puts ("  --unlimited");
	puts ("  --reverse");
	puts ("  --invert");
	puts ("  --rotate");
	puts ("  --noviewer");
	puts ("  --title string");
	puts ("  --csv file");
	puts ("  --nice");
	exit (-1);
}

//----------------------------------------------------------------------------
// Name:	main
//----------------------------------------------------------------------------
int
bandwidth_main (int argc, char *const argv[])
{
	if (!argc)
		return -1;

	struct {
		bool perform_mainregister_nontemporal_tests;
		bool perform_128bit_tests;
		bool perform_128bit_nontemporal_tests;
		bool perform_256bit_tests;
		bool perform_256bit_nontemporal_tests;
		bool perform_512bit_tests;
		bool perform_512bit_nontemporal_tests;
		bool perform_read_tests;
		bool perform_write_tests;
		bool perform_copy_tests;
		bool perform_register_and_stack_tests;
		bool perform_random_tests;
		bool perform_direct_tests;
		bool limit_at_128MB;
		bool launch_viewer;
	} options;
	options.perform_mainregister_nontemporal_tests = true;
	options.perform_128bit_tests = true;
	options.perform_128bit_nontemporal_tests = true;
	options.perform_256bit_tests = true;
	options.perform_256bit_nontemporal_tests = true;
	options.perform_512bit_tests = true;
	options.perform_512bit_nontemporal_tests = true;
	options.perform_read_tests = true;
	options.perform_write_tests = true;
	options.perform_copy_tests = true;
	options.perform_register_and_stack_tests = true;
	options.perform_random_tests = true;
	options.perform_direct_tests = true;
	options.limit_at_128MB = true;
	options.launch_viewer = true;

	console = new(Console);

	CPU *cpu = new(CPU);
	Hardware *hardware = new(Hardware);
	OperatingSystem *os = new(OperatingSystem);

#if defined(x86)
	TestingX86 *benchmarks = new(TestingX86);
#endif

#if defined(__arm__) || defined(__aarch64__)
	TestingARM *benchmarks = new(TestingARM);
#endif

#if defined(RISCV64)
	TestingRISCV *benchmarks = new(TestingRISCV);
#endif

	usec_per_test = 5000000;	// 5 seconds per test.

	outputMode = OUTPUT_MODE_GRAPH;

	--argc;
	++argv;

	MutableString *graphTitle = new(MutableString);
	bool reverse_chunk_size_order = false;

	int i = 0;
	while (i < argc) {
		const char *s = argv [i++];

		if (!strcmp ("--nice", s)) {
			nice_mode = true;
		}
		else if (!strcmp ("--slow", s)) {
			usec_per_test=20000000;	// 20 seconds per test.
		}
		else if (!strcmp ("--unlimited", s)) {
			if (sizeof(long) == 8) {
				options.limit_at_128MB = false;
			}
		}
		else if (!strcmp ("--fast", s)) {
			usec_per_test = 1000000; // 1 second per test.
		}
		else if (!strcmp ("--faster", s)) {
			usec_per_test = 500000;	// 0.5 seconds per test.
		}
		else if (!strcmp ("--fastest", s)) {
			usec_per_test = 50000;	// 0.05 seconds per test.
		}
		else if (!strcmp ("--noviewer", s)) {
			options.launch_viewer = false;
		}
		else if (!strcmp ("--noread", s)) {
			options.perform_read_tests = false;
		}
		else if (!strcmp ("--nowrite", s)) {
			options.perform_write_tests = false;
		}
		else if (!strcmp ("--norandom", s)) {
			options.perform_random_tests = false;
		}
		else if (!strcmp ("--nocopy", s)) {
			options.perform_copy_tests = false;
		}
		else if (!strcmp ("--nonontemporal", s)) {
			options.perform_mainregister_nontemporal_tests = false;
                	options.perform_128bit_nontemporal_tests = false;
                	options.perform_256bit_nontemporal_tests = false;
                	options.perform_512bit_nontemporal_tests = false;
		}
#ifdef x86
		else if (!strcmp ("--nosse2", s)) {
			options.perform_128bit_tests = false;
			options.perform_128bit_nontemporal_tests = false;
			options.perform_256bit_tests = false;
			options.perform_512bit_tests = false;
		}
		else if (!strcmp ("--nosse4", s)) {
			options.perform_128bit_nontemporal_tests = false;
			options.perform_256bit_tests = false;
			options.perform_512bit_tests = false;
		}
		else if (!strcmp ("--noavx", s)) {
			options.perform_256bit_tests = false;
			options.perform_512bit_tests = false;
		}
		else if (!strcmp ("--noavx512", s)) {
			options.perform_512bit_tests = false;
		}
#endif
		else if (!strcmp ("--noregister", s)) {
			options.perform_register_and_stack_tests = false;
		}
		else if (!strcmp ("--invert", s)) {
			do_invert_graph = true;
		}
		else if (!strcmp ("--rotate", s)) {
			do_rotate_graph = true;
		}
		else if (!strcmp ("--diagnostic", s)) {
			diagnostic_mode = true;
		}
		else if (!strcmp ("--reverse", s)) {
			reverse_chunk_size_order = true;
		}
		else if (!strcmp ("--help", s)) {
			usage ();
		}
		else
		if (!strcmp ("--nograph", s)) {
			outputMode &= ~OUTPUT_MODE_GRAPH;
		}
		else
		if (!strcmp ("--csv", s) && i != argc) {
			outputMode |= OUTPUT_MODE_CSV;
			if (i < argc)
				csv_file_path = argv[i++];
			else
				usage ();
		}
		else
		if (!strcmp ("--title", s) && i != argc) {
			$(graphTitle, appendFormat, "%s", argv[i++]);
		}
		else {
			if ('-' == *s)
				usage ();
		}
	}

	size_t chunk_limit = 1;
	if (options.limit_at_128MB) {
		chunk_limit <<= 27;
	} else {
		chunk_limit <<= 30;
	}

	if (reverse_chunk_size_order) {
		int last = 0;
		while (last < N_CHUNK_SIZES && chunk_sizes[last] != chunk_limit) {
			last++;
		}

		for (int i = 0; i < last/2; i++) {
			unsigned long tmp = chunk_sizes[i];
			chunk_sizes[i] = chunk_sizes[last-i];
			chunk_sizes[last-i] = tmp;
		}
	}

	for (i = 0; chunk_sizes[i] && i < N_CHUNK_SIZES; i++) {
		chunk_sizes_log2[i] = log2 (chunk_sizes[i]);
	}

	$(console, puts, "This is bandwidth version " RELEASE);
	$(console, puts, "Copyright (C) 2005-2024 by Zack T Smith.");
	$(console, newline);
	$(console, puts, "This software is covered by the GNU Public License.");
	$(console, puts, "It is provided AS-IS, use at your own risk.");
	$(console, puts, "See the file COPYING for more information.");
	$(console, newline);
	$(console, flush);

	if (!$(cpu, has128bitVectors)) {
		options.perform_128bit_tests = false;
		options.perform_128bit_nontemporal_tests = false;
	}
	if (!$(cpu, has256bitVectors)) {
		options.perform_256bit_tests = false;
	}
	if (!$(cpu, has512bitVectors)) {
		options.perform_512bit_tests = false;
	}

	MutableArray *features = $(cpu, features);

#if defined(__x86_64__) || defined(__i386__)
	String *sse4String = _String("sse41");
	if ($(features, contains, sse4String)) {
		benchmarks->use_sse4 = true;
	}
	release(sse4String);

	if (options.perform_direct_tests) {
		String *string;
#ifdef IS_64BIT
		string = _String("movdir64b");
#else
		string = _String("movdiri");
#endif
		if ($(features, contains, string)) {
			benchmarks->use_direct_transfers = true;
		}
		release(string);
	}

	benchmarks->use_sse2 = options.perform_128bit_tests;
	benchmarks->use_avx = options.perform_256bit_tests;
	benchmarks->use_avx512 = options.perform_512bit_tests;
#endif

	//------------------------------------------------------------
	// Attempt to obtain information about the system.
	//
	String *string = $(hardware, systemMake);
	if (string) {
		$(console, printf, "Computer make: ");
		$(console, printObject, string);
		$(console, newline);
		$(graphTitle, appendString, string);
		$(graphTitle, appendCharacter, ' ');
		release (string);
	}

	string = $(hardware, systemModel);
	if (string) {
		$(console, printf, "Computer model: ");
		$(console, printObject, string);
		$(console, newline);
		$(graphTitle, appendString, string);
		$(graphTitle, appendCharacter, ' ');
		release (string);
	}

	string = $(cpu, family);
	if (string) {
		$(console, printf, "CPU family: ");
		$(console, printObject, string);
		$(console, newline);
		release (string);
	}

	String *cpuMake = $(cpu, make);
	String *cpuModel = $(cpu, model);
	bool dont_append_make = cpuModel != NULL && $(cpuModel, hasPrefix, cpuMake);

	if (!dont_append_make) {
		if (cpuMake) {
			$(console, printf, "CPU make: ");
			$(console, printObject, cpuMake);
			$(console, newline);
			$(graphTitle, appendString, cpuMake);
			$(graphTitle, appendCharacter, ' ');
		}
	}
	release (cpuMake);

	if (cpuModel) {
		if ($(string, containsCString, "(R)")) {
			// TODO Remove the (R)
		}
		$(console, printf, "CPU model: ");
		$(console, printObject, cpuModel);
		$(console, newline);
		$(graphTitle, appendString, cpuModel);
		$(graphTitle, appendCharacter, ' ');
		release (cpuModel);
	}

	unsigned regSize = $(cpu, registerSize);
	$(console, printf, "CPU register size: %u\n", regSize);

	$(console, printf, "CPU vector sizes: ");
	bool hasVectors = false;
	if ($(cpu, has128bitVectors)) {
		$(console, printf, "128 ");
		hasVectors = true;
	}
	if ($(cpu, has256bitVectors)) {
		$(console, printf, "256 ");
		hasVectors = true;
	}
	if ($(cpu, has512bitVectors)) {
		$(console, printf, "512 ");
		hasVectors = true;
	}
	if (!hasVectors) {
		$(console, printf, "(none)");
	}
	$(console, newline);

	string = $(cpu, instructionSet);
	$(console, printf, "CPU instruction set: ");
	$(console, printObject, string);
	$(console, newline);
	$(graphTitle, appendString, string);
	$(graphTitle, appendCharacter, ' ');
	release (string);

	if ($(features, count)) {
		$(console, printf, "CPU features: ");
		$(features, print, NULL);
		$(console, newline);
	}

	bool printed_core_info = false;
	unsigned ncores = $(cpu, nCores);
#ifndef __APPLE__
	for (unsigned core = 0; core < ncores; core++) {
#else
	unsigned core = 0; {
#endif
		unsigned L1i = $(cpu, levelNCacheSize, core, 1, false);
		unsigned L1d = $(cpu, levelNCacheSize, core, 1, true);
		unsigned L2 = $(cpu, levelNCacheSize, core, 2, false);
		unsigned L3 = $(cpu, levelNCacheSize, core, 3, false);
		float maxSpeed = $(cpu, maximumSpeed, core);
		float minSpeed = $(cpu, minimumSpeed, core);

		bool have_cache_info = L1i > 0 || L1d > 0 || L2 > 0 || L3 > 0;
		bool have_speed_info = maxSpeed > 0.f;
		bool have_any_core_info = have_cache_info || have_speed_info;

		if (have_any_core_info) {
			printed_core_info = true;
#ifndef __APPLE__
			$(console, printf, "CPU core %u: ", core);
#else
			$(console, printf, "CPU current core: ");
#endif

			if (have_speed_info) {
				$(console, printf, "max %.2g GHz", maxSpeed);
				if (minSpeed) {
					$(console, printf, ", min %.2g GHz", minSpeed);
				}
				if (have_cache_info) {
					$(console, printf, "; ");
				}
			}
			if (have_cache_info) {
				$(console, printf, "caches: L1 instruction %ukB, L1 data %ukB", L1i, L1d);
				if (L2 > 0) {
					if (L2 >= 1024 && ((L2 & 1023)==0)) {
						$(console, printf, ", L2 %uMB", L2 >> 10);
					}
					else if (L2 >= 1024) {
						$(console, printf, ", L2 %f.02MB", (float)L2/1024.f);
					} else {
						$(console, printf, ", L2 %ukB", L2);
					}
				}

				if (L3 > 0) {
					if (L3 >= 1024 && ((L3 & 1023)==0)) {
						$(console, printf, ", L3 %uMB", L3 >> 10);
					}
					else if (L3 >= 1024) {
						$(console, printf, ", L3 %.02fMB", (float)L3/1024.f);
					} else {
						$(console, printf, ", L3 %ukB", L3);
					}
				}
			}
			$(console, newline);
		}
	}

	if (!printed_core_info) {
		$(console, printf, "CPU cores: %u\n", ncores);
	}

	size_t pagesize = $(os, memoryPageSize);
	$(console, printf, "Memory page size: %lu bytes\n", pagesize);

	unsigned long totalRAM = $(hardware, totalRAM);
	if (totalRAM > 0) {
		$(console, printf, "Memory available: %lu MB\n", totalRAM);
	}

	string = $(os, kernelName);
	if (string) {
		$(console, printf, "Kernel name: ");
		$(console, printObject, string);
		$(console, newline);
		$(graphTitle, appendString, string);
		$(graphTitle, appendCharacter, ' ');
		release (string);
	}

	string = $(os, kernelRelease);
	if (string) {
		$(console, printf, "Kernel release: ");
		$(console, printObject, string);
		$(console, newline);
		$(graphTitle, appendString, string);
		//$(graphTitle, appendCharacter, ' ');
		release (string);
	}

	// Print physical core temperatures.
	for (int core = 0; core < ncores; core++) {
		float temp = $(cpu, temperature, core);
		if (temp > 0.f) {
			$(console, printf, "Physical core %d temperature: %.2fC\n", core, temp);
		}
	}
	// Print thermal zone temperatures.
#define MAX_THERMAL_ZONES 128
	for (int zone = 0; zone < MAX_THERMAL_ZONES; zone++) {
		float temp = $(hardware, temperature, zone);
		if (temp > 0.f) {
			$(console, printf, "Thermal zone %d temperature: %.2fC\n", zone, temp);
		} else {
			break;
		}
	}

	String *subtitle = _String(TITLE_MEMORY_GRAPH);
	dataBegins (graphTitle, subtitle);
	$(console, newline);
	$(console, printf, "Graph title: ");
	$(console, printObject, graphTitle);
	$(console, newline);

	$(console, newline);
	$(console, puts, "Notation: B = byte, kB = 1024 B, MB = 1048576 B.");
	$(console, flush);

	unsigned long chunk_size;

	if (diagnostic_mode) {
		chunk_limit = 1777;
	}

	//------------------------------------------------------------
	// Sequential non-vector register reads.
	//
	bool supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_MAIN_REGISTER, false);
	if (supported && options.perform_read_tests) {
		$(console, newline);
#ifdef IS_64BIT
		dataBeginSection ("Sequential 64-bit reads", RGB_BLUE);
#else
		dataBeginSection ("Sequential 32-bit reads", RGB_BLUE);
#endif

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_MAIN_REGISTER, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit sequential reads using Intel SSE2 or ARM NEON.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128, false);
	if (supported && options.perform_read_tests && options.perform_128bit_tests) {
		dataBeginSection ("Sequential 128-bit reads", RGB_LIGHTBLUE);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_128, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit sequential reads using AVX.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256, false);
	if (supported && options.perform_read_tests && options.perform_256bit_tests) {
		dataBeginSection ("Sequential 256-bit reads", RGB_NAVYBLUE);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_256, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 512-bit sequential reads using AVX512.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_512, false);
	if (supported && options.perform_read_tests && options.perform_512bit_tests) {
		if ($(cpu, has512bitVectors)) {
			dataBeginSection ("Sequential 512-bit reads (dashed)", RGB_NAVYBLUE | DASHED);

			$(console, newline);

			i = 0;
			while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
				long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_512, false);
				dataAddDatum (chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// 32/64-bit sequential nontemporal reads.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_MAIN_REGISTER_NONTEMPORAL, false);
	if (supported && options.perform_read_tests && options.perform_mainregister_nontemporal_tests) {
#ifdef IS_64BIT
		dataBeginSection ("Sequential 64-bit nontemporal reads", RGB_STEELBLUE);
#else
		dataBeginSection ("Sequential 32-bit nontemporal reads", RGB_STEELBLUE);
#endif

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_MAIN_REGISTER_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit nontemporal sequential reads using SSE4.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128_NONTEMPORAL, false);
	if (supported && options.perform_read_tests && options.perform_128bit_nontemporal_tests) {
		dataBeginSection ("Sequential 128-bit nontemporal reads", RGB_LIGHTTEAL);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_128_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit nontemporal sequential reads using AVX512.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256_NONTEMPORAL, false);
	if (supported && options.perform_read_tests && options.perform_256bit_nontemporal_tests) {
		dataBeginSection ("Sequential 256-bit nontemporal reads", RGB_DARKCYAN);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_256_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 512-bit nontemporal sequential reads using AVX512.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_512_NONTEMPORAL, false);
	if (supported && options.perform_read_tests && options.perform_512bit_nontemporal_tests) {
		dataBeginSection ("Sequential 512-bit nontemporal reads (dashed)", RGB_DARKCYAN | DASHED);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_512_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// Sequential non-vector register writes.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_MAIN_REGISTER, false);
	if (supported && options.perform_write_tests) {
#ifdef IS_64BIT
		dataBeginSection ("Sequential 64-bit writes", RGB_MEDIUMGREEN);
#else
		dataBeginSection ("Sequential 32-bit writes", RGB_MEDIUMGREEN);
#endif

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_MAIN_REGISTER, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit sequential writes using SSE2 or NEON 128.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128, false);
	if (supported && options.perform_write_tests && options.perform_128bit_tests) {
		dataBeginSection ("Sequential 128-bit writes", RGB_LIGHTGREEN);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_128, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit sequential writes using AVX.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256, false);
	if (supported && options.perform_write_tests && options.perform_256bit_tests) {
		dataBeginSection ("Sequential 256-bit writes", RGB_DARKGREEN);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_256, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 512-bit sequential writes using AVX512.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_512, false);
	if (supported && options.perform_write_tests && options.perform_512bit_tests) {
		if ($(cpu, has512bitVectors)) {
			dataBeginSection ("Sequential 512-bit writes (dashed)", RGB_DARKGREEN | DASHED);

			$(console, newline);

			i = 0;
			while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
				long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_512, false);
				dataAddDatum (chunk_size, amount);
			}
		}
	}

	//------------------------------------------------------------
	// 32/64-bit sequential nontemporal writes.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_MAIN_REGISTER_NONTEMPORAL, false);
	if (supported && options.perform_write_tests && options.perform_mainregister_nontemporal_tests) {
#ifdef IS_64BIT
		dataBeginSection ("Sequential 64-bit nontemporal writes", RGB_BROWN);
#else
		dataBeginSection ("Sequential 32-bit nontemporal writes", RGB_BROWN);
#endif

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_MAIN_REGISTER_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit sequential nontemporal writes using SSE4.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128_NONTEMPORAL, false);
	if (supported && options.perform_write_tests && options.perform_128bit_nontemporal_tests) {
		dataBeginSection ("Sequential 128-bit nontemporal writes", RGB_LIGHTBROWN);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_128_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit sequential writes with nontemporal hint.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256_NONTEMPORAL, false);
	if (supported && options.perform_write_tests && options.perform_256bit_nontemporal_tests) {
		dataBeginSection ("Sequential 256-bit nontemporal writes", RGB_DARKBROWN);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_256_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 512-bit sequential nontemporal writes using AVX-512.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_512_NONTEMPORAL, false);
	if (supported && options.perform_write_tests && options.perform_512bit_nontemporal_tests) {
		dataBeginSection ("Sequential 512-bit nontemporal writes (dashed)", RGB_DARKBROWN | DASHED);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_512_NONTEMPORAL, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// Random non-vector register reads.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_MAIN_REGISTER, true);
	if (supported && options.perform_read_tests && options.perform_random_tests) {
		$(console, newline);
#ifdef IS_64BIT
		dataBeginSection ("Random 64-bit reads", RGB_RED);
#else
		dataBeginSection ("Random 32-bit reads", RGB_RED);
#endif
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_MAIN_REGISTER, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit random reads using NEON or SSE2.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128, true);
	if (supported && options.perform_read_tests && options.perform_random_tests && options.perform_128bit_tests) {
		dataBeginSection ("Random 128-bit reads", RGB_LIGHTRED);

		$(console, newline);
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_128, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit random reads using AVX.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256, true);
	if (supported && options.perform_read_tests && options.perform_random_tests && options.perform_256bit_tests) {
		dataBeginSection ("Random 256-bit reads", 0xc00000);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_256, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit random nontemporal reads, using SSE4.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128_NONTEMPORAL, true);
	if (supported && options.perform_read_tests && options.perform_random_tests && options.perform_128bit_nontemporal_tests) {
		dataBeginSection ("Random 128-bit nontemporal reads", RGB_PURPLE);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_128_NONTEMPORAL, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit random nontemporal reads, using AVX.
	//
	supported = TEST_SUPPORTED == $(benchmarks, read, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256_NONTEMPORAL, true);
	if (supported && options.perform_read_tests && options.perform_random_tests && options.perform_256bit_nontemporal_tests) {
		dataBeginSection ("Random 256-bit nontemporal reads", RGB_LIGHTPURPLE);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, read, chunk_size, SIZE_VECTOR_256_NONTEMPORAL, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// Random non-vector register writes.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_MAIN_REGISTER, true);
	if (supported && options.perform_write_tests && options.perform_random_tests) {
#ifdef IS_64BIT
		dataBeginSection ("Random 64-bit writes", RGB_ORANGE);
#else
		dataBeginSection ("Random 32-bit writes", RGB_ORANGE);
#endif

		$(console, newline);
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_MAIN_REGISTER, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit random writes using SSE2 or NEON 128-bit.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128, true);
	if (supported && options.perform_write_tests && options.perform_random_tests && options.perform_128bit_tests) {
		dataBeginSection ("Random 128-bit writes", RGB_LIGHTORANGE);

		$(console, newline);
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_128, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-byte randomized writes using AVX.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256, true);
	if (supported && options.perform_write_tests && options.perform_random_tests && options.perform_256bit_tests) {
		dataBeginSection ("Random 256-bit writes", RGB_DARKORANGE);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_256, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit random nontemporal writes, using SSE4.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128_NONTEMPORAL, true);
	if (supported && options.perform_write_tests && options.perform_random_tests && options.perform_128bit_nontemporal_tests) {
		dataBeginSection ("Random 128-bit nontemporal writes", RGB_DARKPURPLE);

		$(console, newline);
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_128_NONTEMPORAL, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit random nontemporal writes, using SSE4.
	//
	supported = TEST_SUPPORTED == $(benchmarks, write, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256_NONTEMPORAL, true);
	if (supported && options.perform_write_tests && options.perform_random_tests && options.perform_256bit_nontemporal_tests) {
		dataBeginSection ("Random 256-bit nontemporal writes", RGB_PINK);

		$(console, newline);
		srand (time (NULL));

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, write, chunk_size, SIZE_VECTOR_256_NONTEMPORAL, true);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 32/64-bit sequential copy using main registers.
	//
	supported = TEST_SUPPORTED == $(benchmarks, copy, CHECK_WHETHER_SUPPORTED, SIZE_MAIN_REGISTER, false);
	if (supported && options.perform_copy_tests) {
#ifdef IS_64BIT
		dataBeginSection ("Sequential 64-bit copy", RGB_BLACK);
#else
		dataBeginSection ("Sequential 32-bit copy", RGB_BLACK);
#endif

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, copy, chunk_size, SIZE_MAIN_REGISTER, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 128-bit sequential copy using SSE2.
	//
	supported = TEST_SUPPORTED == $(benchmarks, copy, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_128, false);
	if (supported && options.perform_copy_tests && options.perform_128bit_tests)

	{
		dataBeginSection ("Sequential 128-bit copy", RGB_DARKGRAY);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, copy, chunk_size, SIZE_VECTOR_128, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 256-bit sequential copy using AVX.
	//
	supported = TEST_SUPPORTED == $(benchmarks, copy, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_256, false);
	if (supported && options.perform_copy_tests && options.perform_256bit_tests) {
		dataBeginSection ("Sequential 256-bit copy", RGB_GRAY);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, copy, chunk_size, SIZE_VECTOR_256, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// 512-bit sequential copy using AVX512.
	//
	supported = TEST_SUPPORTED == $(benchmarks, copy, CHECK_WHETHER_SUPPORTED, SIZE_VECTOR_512, false);
	if (supported && options.perform_copy_tests && options.perform_512bit_tests) {
		dataBeginSection ("Sequential 512-bit copy (dashed)", RGB_GRAY | DASHED);

		$(console, newline);

		i = 0;
		while ((chunk_size = chunk_sizes [i++]) && chunk_size <= chunk_limit) {
			long amount = $(benchmarks, copy, chunk_size, SIZE_VECTOR_512, false);
			dataAddDatum (chunk_size, amount);
		}
	}

	//------------------------------------------------------------
	// Perform register and memoryh copy/increment tests.
	//
	if (options.perform_register_and_stack_tests) {
		//------------------------------------------------------------
		// Register to register.
		//
		$(console, newline);
		$(benchmarks, registerToRegisterTest);

		if (options.perform_128bit_tests) {
			$(benchmarks, vectorToRegisterTest);
			$(benchmarks, vectorToRegister8);
			$(benchmarks, vectorToRegister16);
			$(benchmarks, vectorToRegister32);
			$(benchmarks, registerToVectorTest);
			$(benchmarks, registerToVector8);
			$(benchmarks, registerToVector16);
			$(benchmarks, registerToVector32);
			$(benchmarks, vectorToVectorTest128);
		}
		if (options.perform_256bit_tests) {
			$(benchmarks, vectorToVectorTest256);
		}
		if (options.perform_512bit_tests) {
			$(benchmarks, vectorToVectorTest512);
		}

		//------------------------------------------------------------
		// Stack to/from register.
		//
		$(benchmarks, stackRead);
		$(benchmarks, stackWrite);

		//------------------------------------------------------------
		// Register vs stack.
		//
		$(benchmarks, incrementRegisters);
		$(benchmarks, incrementStack);
	}

	$(console, flush);
	$(console, newline);
	dataEnds (RESULTS_IMAGE_FILENAME);

	$(console, newline);
	$(console, puts, "Done.");

	release (graphTitle);
	release (graph);
	release (benchmarks);
	release (console);
	release (cpu);
	release (hardware);
	release (os);
	os = NULL;

	deallocateClasses();

	if (g_totalObjectAllocations == g_totalObjectDeallocations) {
		puts ("All objects that were allocated were deallocated.");
	} else {
		puts ("Not all objects that were allocated were deallocated.");
		printf ("Remaining = %ld.\n", (long)g_totalObjectAllocations-(long)g_totalObjectDeallocations);
	}

	if (options.launch_viewer) {
#ifdef __APPLE__
		system ("open "RESULTS_IMAGE_FILENAME);
#endif
#if defined(__linux__) && !defined(__ANDROID__)
		static const char *linux_image_viewers[] = {
			"ffplay",
			"gthumb",
			"eog",
			"okular",
			"geeqie",
			"gwenview",
			"xv"
		};
		for (int i=0; i < sizeof(linux_image_viewers)/sizeof(char*); i++) {
			char command[256];
			snprintf (command, sizeof(command), "%s %s 2>/dev/null", linux_image_viewers[i], RESULTS_IMAGE_FILENAME);
			if (!system (command)) {
				break;
			}
		}
#endif
#if defined(__WIN32__) || defined(__WIN64__)
		system ("explorer "RESULTS_IMAGE_FILENAME);
#endif
	}

	return 0;
}

int bandwidth_cpux(struct BandwidthData *bwd)
{
	CPU *cpu = new(CPU);
	MutableArray *features = $(cpu, features);
	unsigned long chunk_size;

#if defined(x86)
	TestingX86 *benchmarks = new(TestingX86);
#elif defined(__arm__) || defined(__aarch64__)
	TestingARM *benchmarks = new(TestingARM);
#elif defined(RISCV64)
	TestingRISCV *benchmarks = new(TestingRISCV);
#endif

	usec_per_test = 5000;

#if defined(__x86_64__) || defined(__i386__)
	String *sse4String = _String("sse41");
	if ($(features, contains, sse4String)) {
		benchmarks->use_sse4 = true;
	}
	release(sse4String);

	String *string;
#ifdef IS_64BIT
	string = _String("movdir64b");
#else
	string = _String("movdiri");
#endif
	if ($(features, contains, string)) {
		benchmarks->use_direct_transfers = true;
	}

	benchmarks->use_sse2 = true;
	benchmarks->use_avx = true;
	benchmarks->use_avx512 = true;
#endif

	/* Needed by CPU-X */
	int count                            = 0;
	uint8_t cache_level                  = 0;
	double total_amount                  = 0;
	char test_name[TEST_NAME_BUFFER_LEN] = "";
	pthread_mutex_lock(&bwd->mutex);

#define TESTING_FUNC(func) (long (*)(void*, unsigned long, int, bool)) benchmarks->is_a->func
	const struct Tests tests[] =
	{
		/* Test          Name                                         Support instructions      Function             Testing mode                    Rand */
		// Sequential reads
		{ SEQ_WS_R,      N_("Sequential %i-bit reads"),               true,                     TESTING_FUNC(read),  SIZE_MAIN_REGISTER,             false },
		{ SEQ_128_R,     N_("Sequential 128-bit reads"),              $(cpu, has128bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_128,                false },
		{ SEQ_256_R,     N_("Sequential 256-bit reads"),              $(cpu, has256bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_256,                false },
		{ SEQ_512_R,     N_("Sequential 512-bit reads"),              $(cpu, has512bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_512,                false },
		// Sequential nontemporal reads
		{ SEQ_WS_NT_R,   N_("Sequential %i-bit nontemporal reads"),   true,                     TESTING_FUNC(read),  SIZE_MAIN_REGISTER_NONTEMPORAL, false },
		{ SEQ_128_NT_R,  N_("Sequential 128-bit nontemporal reads"),  $(cpu, has128bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_128_NONTEMPORAL,    false },
		{ SEQ_256_NT_R,  N_("Sequential 256-bit nontemporal reads"),  $(cpu, has256bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_256_NONTEMPORAL,    false },
		{ SEQ_512_NT_R,  N_("Sequential 512-bit nontemporal reads"),  $(cpu, has512bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_512_NONTEMPORAL,    false },
		// Sequential writes
		{ SEQ_WS_W,      N_("Sequential %i-bit writes"),              true,                     TESTING_FUNC(write), SIZE_MAIN_REGISTER,             false },
		{ SEQ_128_W,     N_("Sequential 128-bit writes"),             $(cpu, has128bitVectors), TESTING_FUNC(write), SIZE_VECTOR_128,                false },
		{ SEQ_256_W,     N_("Sequential 256-bit writes"),             $(cpu, has256bitVectors), TESTING_FUNC(write), SIZE_VECTOR_256,                false },
		{ SEQ_512_W,     N_("Sequential 512-bit writes"),             $(cpu, has512bitVectors), TESTING_FUNC(write), SIZE_VECTOR_512,                false },
		// Sequential nontemporal writes
		{ SEQ_WS_NT_W,   N_("Sequential %i-bit nontemporal writes"),  true,                     TESTING_FUNC(write), SIZE_MAIN_REGISTER_NONTEMPORAL, false },
		{ SEQ_128_NT_W,  N_("Sequential 128-bit nontemporal writes"), $(cpu, has128bitVectors), TESTING_FUNC(write), SIZE_VECTOR_128_NONTEMPORAL,    false },
		{ SEQ_256_NT_W,  N_("Sequential 256-bit nontemporal writes"), $(cpu, has256bitVectors), TESTING_FUNC(write), SIZE_VECTOR_256_NONTEMPORAL,    false },
		{ SEQ_512_NT_W,  N_("Sequential 512-bit nontemporal writes"), $(cpu, has512bitVectors), TESTING_FUNC(write), SIZE_VECTOR_512_NONTEMPORAL,    false },
		// Random reads
		{ RAND_WS_R,     N_("Random %i-bit reads"),                   true,                     TESTING_FUNC(read),  SIZE_MAIN_REGISTER,             true  },
		{ RAND_128_R,    N_("Random 128-bit reads"),                  $(cpu, has128bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_128,                true  },
		{ RAND_256_R,    N_("Random 256-bit reads"),                  $(cpu, has256bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_256,                true  },
		// Random nontemporal reads
		{ RAND_128_NT_R, N_("Random 128-bit nontemporal reads"),      $(cpu, has128bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_128_NONTEMPORAL,    true  },
		{ RAND_256_NT_R, N_("Random 256-bit nontemporal reads"),      $(cpu, has256bitVectors), TESTING_FUNC(read),  SIZE_VECTOR_256_NONTEMPORAL,    true  },
		// Random writes
		{ RAND_WS_W,     N_("Random %i-bit writes"),                  true,                     TESTING_FUNC(write), SIZE_MAIN_REGISTER,             true  },
		{ RAND_128_W,    N_("Random 128-bit writes"),                 $(cpu, has128bitVectors), TESTING_FUNC(write), SIZE_VECTOR_128,                true  },
		{ RAND_256_W,    N_("Random 256-bit writes"),                 $(cpu, has256bitVectors), TESTING_FUNC(write), SIZE_VECTOR_256,                true  },
		// Random nontemporal writes
		{ RAND_128_NT_W, N_("Random 128-bit nontemporal writes"),     $(cpu, has128bitVectors), TESTING_FUNC(write), SIZE_VECTOR_128_NONTEMPORAL,    true  },
		{ RAND_256_NT_W, N_("Random 256-bit nontemporal writes"),     $(cpu, has256bitVectors), TESTING_FUNC(write), SIZE_VECTOR_256_NONTEMPORAL,    true  },
		// Sequential copy
		{ SEQ_WS_C,      N_("Sequential %i-bit copy"),                true,                     TESTING_FUNC(copy),  SIZE_MAIN_REGISTER,             false },
		{ SEQ_128_C,     N_("Sequential 128-bit copy"),               $(cpu, has128bitVectors), TESTING_FUNC(copy),  SIZE_VECTOR_128,                false },
		{ SEQ_256_C,     N_("Sequential 256-bit copy"),               $(cpu, has256bitVectors), TESTING_FUNC(copy),  SIZE_VECTOR_256,                false },
		{ SEQ_512_C,     N_("Sequential 512-bit copy"),               $(cpu, has512bitVectors), TESTING_FUNC(copy),  SIZE_VECTOR_512,                false },
		// Sentinel value
		{ -1,            NULL,                                        false,                    NULL,                0,                              false }
	};
#undef TESTING_FUNC

	/* Set test names and quit */
	if(bwd->test_name == NULL)
	{
		bwd->test_supported = malloc(BANDWIDTH_LAST_TEST * sizeof(bool));
		bwd->test_name      = malloc(BANDWIDTH_LAST_TEST * sizeof(char *));
		for(int i = 0; tests[i].name != NULL; i++)
		{
			bwd->test_name[i]      = NULL;
			bwd->test_supported[i] = tests[i].support_instr;
			bwd->test_supported[i] = bwd->test_supported[i] && (TEST_SUPPORTED == (*tests[i].func_ptr)(benchmarks, CHECK_WHETHER_SUPPORTED, tests[i].testing_mode, tests[i].random));
			if(strchr(tests[i].name, '%') != NULL)
				snprintf(test_name, TEST_NAME_BUFFER_LEN, tests[i].name, __WORDSIZE);
			else
				strncpy(test_name, tests[i].name, TEST_NAME_BUFFER_LEN);
			asprintf(&bwd->test_name[i], G_("#%2i: %s %s"), i, test_name, bwd->test_supported[i] ? "" : _("(unavailable)"));
		}
		goto clean;
	}

	/* Run test if supported */
	if(bwd->test_supported[bwd->selected_test])
	{
		if(tests[bwd->selected_test].random)
			srand(time(NULL));

		int i = 0;
		while((chunk_size = chunk_sizes[i++]))
		{
			if(chunk_size > bwd->cache_size[cache_level] * 1024)
			{
				bwd->cache_speed[cache_level] = total_amount / count;
				count        = 0;
				total_amount = 0;
				cache_level++;

				if(bwd->cache_size[cache_level] == 0)
					goto clean;
			}

			int amount    = (*tests[bwd->selected_test].func_ptr)(benchmarks, chunk_size, tests[bwd->selected_test].testing_mode, tests[bwd->selected_test].random);
			total_amount += amount;
			count++;
		}
	}
	else
	{
		for(int i = 0; i < BANDWIDTH_MAX_CACHE_LEVEL; i++)
			bwd->cache_speed[i] = 0;
	}
clean:
	pthread_mutex_unlock(&bwd->mutex);

	return 0;
}
