/*============================================================================
  CPU, an object-oriented C CPU information class.
  Copyright (C) 2019, 2023 by Zack T Smith.

  Object-Oriented C is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  Object-Oriented C is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public License
  along with this software.  If not, see <http://www.gnu.org/licenses/>.

  The author may be reached at 1 at zsmith.co.
 *===========================================================================*/

#include "CPU.h"
#include "Utility.h"

#include <sys/types.h>
#include <dirent.h> // opendir
#include <ctype.h> // isdigit
#include <unistd.h>
#include <stdbool.h>

#if defined(__linux__) || defined(__CYGWIN__)
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#if defined(__x86_64__) || defined(__i386__) 
#include "utility-x86.h"
#endif

CPUClass *_CPUClass = NULL;

String* CPU_family (CPU *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	const char *result = NULL;

#if defined(__arm__) || defined(__aarch64__)
  #if defined(__APPLE__)
	result = "AppleSilicon";
  #elif defined(__ANDROID__)
	result = execute_and_return_first_line ("cat /sys/devices/soc0/family");
	if (!result) {
		result = "ARM";
	}
  #else
	result = "ARM";
  #endif
#endif

#if defined(__x86_64__) || defined(__WIN64__) || defined(__i386__) || defined(_WIN32) || defined(__WIN32__) || defined(__MINGW32__) || defined(__i386)
	result = "X86";
#endif

#if defined(__riscv) || defined(__riscv__)
	result = "RISC-V";
#endif

	if (!result) {
		return NULL;
	}
	return String_withCString (result);
}

String* CPU_make (CPU *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,CPU);

#ifdef __ANDROID__
	char *android_result = execute_and_return_first_line ("cat /sys/devices/soc0/vendor");
	if (!android_result) {
		android_result = "ARM";
	}
	return _String(android_result);
#endif

#if defined(__APPLE__)
   #if defined(__arm__) || defined(__aarch64__)
	return String_withCString ("Apple");
   #elif defined(__POWERPC__)
	return String_withCString ("PowerPC"); 
   #endif
#endif

#if defined(__x86_64__) || defined(__WIN64__)
	char family[17];
	memset (family, 0, sizeof(family));
	uint32_t *ptr = (uint32_t*) family;
	ptr[0] = get_cpuid_family1();
	ptr[1] = get_cpuid_family2();
	ptr[2] = get_cpuid_family3();
	if (!strcmp (family, "GenuineIntel")) {
		self->is_intel = true;
	}
	return String_withCString (family);
#endif

#if defined(__i386__) || defined(_WIN32) || defined(__WIN32__) || defined(__MINGW32__) || defined(__i386)
	char family[17];
	memset (family, 0, sizeof(family));
	get_cpuid_family (family);
	if (!strcmp (family, "GenuineIntel")) {
		self->is_intel = true;
	}
	return String_withCString (family);
#endif

#if defined(__linux__) || defined(__CYGWIN__) 
	char *result = execute_and_return_first_line ("grep -i '^vendor_id.*:' /proc/cpuinfo | sed 's/^.*: //' | tail -1");
	if (result) {
		return String_withCString (result);
	}
#endif

	return NULL;
}

#ifdef NOT_USEFUL
static String* CPU_implementer (CPU* self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	char *result = NULL;

#if defined(__aarch64__) && defined(__linux__)
	unsigned long midr_el1 = 0;
	if (read_hex_from_file ("/sys/devices/system/cpu/cpu0/regs/identification/midr_el1", &midr_el1)) {
		unsigned long implementer = (midr_el1 >> 24) & 0xff;
		switch (implementer) {
		case 0x41: result = "Arm Limited"; break;
		case 0x42: result = "Broadcom"; break;
		case 0x43: result = "Cavium"; break;
		case 0x44: result = "Digital Equipment Corporation"; break;
		case 0x46: result = "Fujitsu"; break;
		case 0x49: result = "Infineon Technologies"; break;
		case 0x4d: result = "Motorola/Freescale"; break;
		case 0x4e: result = "Nvidia"; break;
		case 0x50: result = "Applied Micro Circuits"; break;
		case 0x51: result = "Qualcomm"; break;
		case 0x56: result = "Marvell"; break;
		case 0x69: result = "Intel"; break;
		case 0xc0: result = "Ampere"; break;
		}
	}
    #endif
} 
#endif

String* CPU_model (CPU *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,CPU);

#ifdef __ANDROID__
	char *android_result = execute_and_return_first_line ("getprop ro.hardware.chipname");
	if (android_result) {
		return _String(android_result);
	}
#endif

#if defined(__linux__) || defined(__CYGWIN__)
    #if defined(__i386__) || defined(__x86_64__) 
	char temp[49];
	get_cpuid_model (temp);
	temp[48] = 0;
	return _String(temp);

    #elif defined(__aarch64__) || defined(__arm__)
	char *result = execute_and_return_first_line ("grep -i '^Hardware.*:' /proc/cpuinfo | sed 's/^.*: //' | tail -1");
	if (result) {
		return String_withCString (result);
	}
    #elif defined(__riscv) || defined(__riscv__)
	char *result = execute_and_return_first_line ("grep uarch /proc/cpuinfo | sed 's/^.*://'");
	if (result) {
		return String_withCString (result);
	}
    #endif
#endif

#ifdef __APPLE__
	char name[48];
	size_t size = sizeof(name);
	if (!sysctlbyname("machdep.cpu.brand_string", name, &size, NULL, 0)) {
		if (name[0]) {
			name[size] = 0;
			return String_withCString (name);
		}
	}
#endif

	return NULL;
}

unsigned CPU_registerSize (CPU *self) 
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	// Size in bits.
	return 8 * sizeof(long);
}

String* CPU_instructionSet (CPU *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,CPU);

#if defined(__aarch64__) || defined(__arm__) 
	// In 32-bit Raspberry Pi OS, uname returns aarch64 because the kernel is 64-bit
	// even though all the executables are 32-bit. So here, we have to test
	// the size of a long to see which ISA is in use.
	if (sizeof(long) == 8) {
		return String_withCString ("aarch64");
	} else {
		return String_withCString ("aarch32");
	}
#endif
#if defined(__x86_64__)
	return String_withCString ("x86_64");
#endif
#if defined(__i386__) || defined(_WIN32) || defined(__WIN32__) || defined(__MINGW32__) || defined(__i386) 
	return String_withCString ("i386");
#endif
#if defined(__riscv) || defined(__riscv__)
	return String_withCString ("riscv64");
#endif

	return NULL;
}

String* CPU_osType (CPU *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	char *type = NULL;
#ifdef __CYGWIN__
	type = "Windows/Cygwin";
#elif defined(__linux__)
	type = "Linux";
#elif defined(__APPLE__)
	type = "MacOS";
#elif defined(__WIN32__) || defined(__WIN64__)
	type = "Windows";
#endif

	return type ? String_withCString (type) : NULL;
}

String* CPU_osRelease (CPU *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,CPU);

#ifdef __linux__
	struct utsname uts;
	uname (&uts);
	const char *cstring = uts.release;

	// Truncate to leave out extraneous build info.
	static char truncated_str [512];
	strncpy (truncated_str, cstring, sizeof(truncated_str)-1);
	char *s = strchr (truncated_str, '(');
	if (s) *s = 0;
	s = strchr (truncated_str, '-');
	if (s) *s = 0;

	return strlen(cstring) ? String_withCString (truncated_str) : NULL;
#endif

#if defined(__APPLE__)
	char *result = execute_and_return_first_line ("system_profiler SPSoftwareDataType | grep 'CPU Version:' | sed 's/^.*CPU Version: //' | sed 's/(.*//' | sed 's/macOS //'");
	if (result) {
		return String_withCString (result);
	}
#endif

#if defined(__WIN32__) || defined(__WIN64__)
	// If have windows.h, call GetVersionEx() or VerifyVersionInfo()
	// If have uname() (missing for me), just use Linux code.
#endif

	return NULL;
}

void CPU_describe (CPU* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,CPU);
	
	fprintf (file ?: stdout, "%s\n", $(self, className));
}

void CPU_print (CPU *self, FILE* file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (!file)
		file = stdout;
}

bool CPU_equals (CPU *self, void *other_) 
{ 
	if (!self || !other_) 
		return false;
	verifyCorrectClassOrSubclass(self,CPU);
	CPU *other = (CPU*) other_;
	verifyCorrectClassOrSubclass(other,CPU);

	return false;
}

void CPU_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,CPU);

	CPU *self = self_;

	if (self->features) {
		release(self->features);
		self->features = NULL;
	}
        
        if (self->level1d_sizes) {
        	free(self->level1d_sizes);
        	self->level1d_sizes = NULL;
	}
        if (self->level1i_sizes) {
        	free (self->level1i_sizes);
        	self->level1i_sizes = NULL;
	}
        if (self->level2_sizes) {
        	free (self->level2_sizes);
        	self->level2_sizes = NULL;
	}
        if (self->level3_sizes) {
        	free (self->level3_sizes);
        	self->level3_sizes = NULL;
	}

	Object_destroy (self);
}

CPU* CPU_init (CPU* self)
{
	ENSURE_CLASS_READY(CPU);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _CPUClass;
		(void) $(self, features);
	}
	return self;
}

static MutableArray* CPU_features (CPU *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (self->features) {
		return self->features;
	}

	MutableArray *mut = new(MutableArray);

#if defined(__aarch64__) || defined(__arm__)
   #if defined(__ARM_NEON__) || defined(__ARM_NEON) || defined(WITH_SIMD)
	$(mut, append, _String("neon"));
	self->has128bitVectors = true;
   #else
	char *result = execute_and_return_first_line ("grep -e neon -e asimd /proc/cpuinfo | head -1");
	if (result && *result) {
		if (strstr (result, "neon")) {
			$(mut, append, _String("neon"));
			self->has128bitVectors = true;
		}
		else if (strstr (result, "asimd")) {
			$(mut, append, _String("asimd"));
			self->has128bitVectors = true;
		}
	}
   #endif
#endif

#if defined(__x86_64__) || defined(__i386__)
	uint32_t ebx, ecx, edx;

	//------------
	// CPUID eax=1
	//
	bool have_sse = false;
	bool have_avx = false;
	ecx = get_cpuid1_ecx ();
	if (ecx & CPUID1_ECX_F16C) $(mut, append, _String("f16c"));
	if (ecx & CPUID1_ECX_SSE3) $(mut, append, _String("sse3"));
	if (ecx & CPUID1_ECX_SSSE3) $(mut, append, _String("ssse3"));
	if (ecx & CPUID1_ECX_SSE41) {
		$(mut, append, _String("sse41"));
	}
	if (ecx & CPUID1_ECX_SSE42) $(mut, append, _String("sse42"));
	if (ecx & CPUID1_ECX_AES) $(mut, append, _String("aes"));
	if (ecx & CPUID1_ECX_AVX) {
		have_avx = true;
		$(mut, append, _String("avx"));
	}
	if (ecx & CPUID1_ECX_HYPER_GUEST) $(mut, append, _String("hyper"));
	if (ecx & CPUID1_ECX_POPCNT) $(mut, append, _String("popcnt"));
	if (ecx & CPUID1_ECX_TM2) $(mut, append, _String("tm2"));

	edx = get_cpuid1_edx ();
	if (edx & CPUID1_EDX_TM) $(mut, append, _String("tm"));
	if (edx & CPUID1_EDX_CMOV) $(mut, append, _String("cmov"));
	if (edx & CPUID1_EDX_ACPI) $(mut, append, _String("acpi"));
	if (edx & CPUID1_EDX_HTT) $(mut, append, _String("htt"));
	if (edx & CPUID1_EDX_SSE2) $(mut, append, _String("sse2"));
	if (edx & CPUID1_EDX_MMX) $(mut, append, _String("mmx"));
	if (edx & CPUID1_EDX_SSE) {
		have_sse = true;
		$(mut, append, _String("sse"));
	}

	//-------------------
	// CPUID eax=7, ecx=0
	//
	ebx = get_cpuid7_ebx (); 
	if (ebx & CPUID7_EBX_ADX) $(mut, append, _String("adx"));
	if (ebx & CPUID7_EBX_BMI1) $(mut, append, _String("bmi1"));
	if (ebx & CPUID7_EBX_BMI2) $(mut, append, _String("bmi2"));
	if (ebx & CPUID7_EBX_MPX) $(mut, append, _String("mpx"));
	if (ebx & CPUID7_EBX_RDSEED) $(mut, append, _String("rdseed"));
	if (ebx & CPUID7_EBX_SHA) $(mut, append, _String("sha"));
	if (ebx & CPUID7_EBX_SGX) $(mut, append, _String("sgx"));
	bool have_avx512 = false;
	if (have_avx) {
		if (ebx & CPUID7_EBX_AVX2) $(mut, append, _String("avx2"));
		if (ebx & CPUID7_EBX_AVX512_VL) $(mut, append, _String("avx512vl"));
		if (ebx & CPUID7_EBX_AVX512_F) $(mut, append, _String("avx512f"));
		if (ebx & CPUID7_EBX_AVX512_DQ) $(mut, append, _String("avx512dq"));
		if (ebx & CPUID7_EBX_AVX512_IFMA) $(mut, append, _String("avx512ifma"));
		if (ebx & CPUID7_EBX_AVX512_PF) $(mut, append, _String("avx512pf"));
		if (ebx & CPUID7_EBX_AVX512_ER) $(mut, append, _String("avx512er"));
		if (ebx & CPUID7_EBX_AVX512_CD) $(mut, append, _String("avx512cd"));
		if (ebx & CPUID7_EBX_AVX512_BW) $(mut, append, _String("avx512bw"));

		if ((ebx & CPUID7_EBX_AVX512_F) && (ebx & CPUID7_EBX_AVX512_DQ)) {
			have_avx512 = true;
		}
	}

	ecx = get_cpuid7_ecx (); 
	edx = get_cpuid7_edx ();
	if (ecx & CPUID7_ECX_CET) $(mut, append, _String("cet"));
	if (ecx & CPUID7_ECX_VAES) $(mut, append, _String("vaes"));
	if (have_avx512) {
		if (ecx & CPUID7_ECX_AVX512_VNNI) $(mut, append, _String("avx512vnni"));
		if (ecx & CPUID7_ECX_AVX512_VBMI) $(mut, append, _String("avx512vbmi"));
		if (ecx & CPUID7_ECX_AVX512_VBMI2) $(mut, append, _String("avx512vbmi2"));
		if (ecx & CPUID7_ECX_AVX512_BITALG) $(mut, append, _String("avx512bitalg"));
		if (ecx & CPUID7_ECX_AVX512_VPOPCNTDQ) $(mut, append, _String("avx512vpopcntdq"));
		if (edx & CPUID7_EDX_AVX512_FP16) $(mut, append, _String("avx512fp16"));
		if (edx & CPUID7_EDX_AVX512_VP2INTERSECT) $(mut, append, _String("avx512vp2intersect"));
		if (edx & CPUID7_EDX_AVX512_4VNNIW) $(mut, append, _String("avx512_4vnniw"));
		if (edx & CPUID7_EDX_AVX512_4FMAPS) $(mut, append, _String("avx512_4fmaps"));
	}
	if (ecx & CPUID7_ECX_MOVDIRI) $(mut, append, _String("movdiri"));
	if (ecx & CPUID7_ECX_MOVDIR64B) $(mut, append, _String("movdir64b"));

	if (edx & CPUID7_EDX_HYBRID) $(mut, append, _String("hybrid"));
	if (edx & CPUID7_EDX_AMX_BF16) $(mut, append, _String("amx_bf16"));
	if (edx & CPUID7_EDX_AMX_TILE) $(mut, append, _String("amx_tile"));
	if (edx & CPUID7_EDX_AMX_INT8) $(mut, append, _String("amx_int8"));

	//-------------------
	// CPUID eax=80000001
	//
	uint32_t ecx2 = get_cpuid_80000001_ecx ();
	if (ecx2 & CPUID80000001_ECX_LZCNT) $(mut, append, _String("lzcnt"));
	if (ecx2 & CPUID80000001_ECX_PREFETCHW) $(mut, append, _String("prefetchw"));

	uint32_t edx2 = get_cpuid_80000001_edx ();
	if (edx2 & CPUID80000001_EDX_NX) $(mut, append, _String("nx"));
	if (edx2 & CPUID80000001_EDX_MMXEXT) $(mut, append, _String("mmxext"));
	if (edx2 & CPUID80000001_EDX_INTEL64) $(mut, append, _String("intel64"));

	self->has128bitVectors = have_sse;
	self->has256bitVectors = have_avx;
	self->has512bitVectors = have_avx512;

	$(mut, quicksort);
#endif

	self->features = retain(mut);

	return mut;
}

static bool CPU_has128bitVectors (CPU *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	return self->has128bitVectors;
}

static bool CPU_has256bitVectors (CPU *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	return self->has256bitVectors;
}

static bool CPU_has512bitVectors (CPU *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	return self->has512bitVectors;
}

static bool CPU_hasVectorUnit (CPU *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (!self->features) {
		(void) $(self,features);
	}

	// Ignore 64-bit vectors, they generally aren't very useful.

	return self->has128bitVectors 
	    || self->has256bitVectors
	    || self->has512bitVectors;
}

static unsigned CPU_nCores (CPU *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

#if defined(__linux__) || defined(__CYGWIN__)
	if (!self->nCores) {
		self->nCores = get_nprocs_conf();
	}
	return self->nCores;
#endif

#ifdef __APPLE__
	int mib[2];
	int value = 0;
	size_t len = sizeof(int);
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	if (!sysctl (mib, 2, &value, (void*)&len, NULL, 0)) {
		if (value > 0) {
			self->nCores = value;
			return self->nCores;
		}
	}

	char temp[16];
	size_t size = sizeof(temp);
	if (!sysctlbyname("machdep.cpu.core_count", temp, &size, NULL, 0)) {
		int ncores = atoi(temp);
		if (ncores > 0) {
			self->nCores = ncores;
			return self->nCores;
		}
	}
#endif

	return 0;
}

static void allocate_cache_arrays (CPU *self)
{
	if (!self->level2_sizes) {
		unsigned ncores = CPU_nCores(self);

		size_t nBytes = sizeof(unsigned) * ncores;
		self->level1d_sizes = (unsigned *) malloc (nBytes);
		self->level1i_sizes = (unsigned *) malloc (nBytes);
		self->level2_sizes = (unsigned *) malloc (nBytes);
		self->level3_sizes = (unsigned *) malloc (nBytes);
		
		memset (self->level1d_sizes, 0, nBytes);
		memset (self->level1i_sizes, 0, nBytes);
		memset (self->level2_sizes, 0, nBytes);
		memset (self->level3_sizes, 0, nBytes);
	}
}

#if defined(__x86_64__)
/* Only (newer) Intel CPUs support cache info lookup via CPUID with EAX=4.
 */
static bool intel_x86_64_fetch_cache_info (CPU *self)
{
	if (!self || !self->is_intel) {
		return false;
	}

	allocate_cache_arrays (self);

	if (self->level1d_sizes[0]) {
		return true;
	}

	unsigned ncores = CPU_nCores(self);
	uint32_t cache_info[4];
	int index = 0;
	while (1) {
		get_cpuid_cache_info (cache_info, index);

		int level = (cache_info[0] >> 5) & 7;
		// Level 1..3
		
		int type = cache_info[0] & 31;
		// Type 1: data cache
		// Type 2: instruction cache
		// Type 3: unified cache
		
		if (!type) {
			break;
		}

		uint32_t n_ways = 1 + (cache_info[1] >> 22);
		uint32_t line_size = 1 + (cache_info[1] & 2047);
		uint32_t n_sets = 1 + cache_info[2];
		unsigned size = (n_ways * line_size * n_sets) >> 10;

		switch (level) {
		case 1:
			if (type == 1) {
				for (unsigned core = 0; core < ncores; core++) {
					self->level1d_sizes[core] = size;
				}
			} 
			else if (type == 2) {
				for (unsigned core = 0; core < ncores; core++) {
					self->level1i_sizes[core] = size;
				}
			}
			break;
		case 2:
			for (unsigned core = 0; core < ncores; core++) {
				self->level2_sizes[core] = size;
			}
			break;
		case 3:
			for (unsigned core = 0; core < ncores; core++) {
				self->level3_sizes[core] = size;
			}
			break;
		}

		index++;
	} 

	return index > 0;
}
#endif

#if defined(__linux__) 
static bool linux_fetch_cache_info (CPU *self)
{
	allocate_cache_arrays (self);

	if (self->level1d_sizes[0]) {
		return true;
	}

	char path[PATH_MAX];

	bool got_data = false;

	unsigned ncores = CPU_nCores(self);
	for (unsigned core = 0; core < ncores; core++) {
		int index = 0;
		do {
			snprintf (path, sizeof(path), "/sys/devices/system/cpu/cpu%u/cache/index%d/type", core, index);
	
			char cacheType[64] = {0};
			if (!read_trimmed_line_from_file (path, cacheType, sizeof(cacheType))) {
				break;
			}

			unsigned size;
			snprintf (path, sizeof(path), "/sys/devices/system/cpu/cpu%u/cache/index%d/size", core, index);
			bool got_size = read_unsigned_from_file (path, &size);
			if (!got_size) {
				break;
			}

			unsigned level;
			snprintf (path, sizeof(path), "/sys/devices/system/cpu/cpu%u/cache/index%d/level", core, index);
			bool got_level = read_unsigned_from_file (path, &level);
			if (!got_level) {
				break;
			}

			switch (level) {
			case 1:
				if (!strcasecmp ("instruction", cacheType)) {
					self->level1i_sizes[core] = size;
				}
				else if (!strcasecmp ("data", cacheType)) {
					self->level1d_sizes[core] = size;
				}
				got_data = true;
				break;

			case 2:
				self->level2_sizes[core] = size;
				got_data = true;
				break;

			case 3:
				self->level3_sizes[core] = size;
				got_data = true;
				break;

			default: break;
			}

			index++;
		} while(true);
	}

	return got_data;
}
#endif

static float CPU_maximumSpeed (CPU *self, unsigned core)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (core >= CPU_nCores(self)) {
		return 0;
	}

#if defined(__linux__)
	char path[PATH_MAX];
	snprintf (path, sizeof(path), "/sys/devices/system/cpu/cpu%u/cpufreq/cpuinfo_max_freq", core);
	unsigned value = 0;
	if (read_unsigned_from_file (path, &value)) {
		float f = (float) value;
		f /= 1000000.f; // Convert kHz to GHz
		return f;
	}
#endif

	return 0.f;
}

static float CPU_minimumSpeed (CPU *self, unsigned core)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (core >= CPU_nCores(self)) {
		return 0;
	}

#if defined(__linux__)
	char path[PATH_MAX];
	snprintf (path, sizeof(path), "/sys/devices/system/cpu/cpu%u/cpufreq/cpuinfo_min_freq", core);
	unsigned value = 0;
	if (read_unsigned_from_file (path, &value)) {
		float f = (float) value;
		f /= 1000000.f; // Convert kHz to GHz
		return f;
	}
#endif

	return 0.f;
}

static float CPU_currentSpeed (CPU *self, unsigned core)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (core >= CPU_nCores(self)) {
		return 0;
	}

#if defined(__linux__)
	char path[PATH_MAX];
	snprintf (path, sizeof(path), "/sys/devices/system/cpu/cpu%u/cpufreq/cpuinfo_cur_freq", core);
	unsigned value = 0;
	if (read_unsigned_from_file (path, &value)) {
		float f = (float) value;
		f /= 1000000.f; // Convert kHz to GHz
		return f;
	}
#endif

	return 0.f;
}

static unsigned CPU_levelNCacheSize (CPU *self, unsigned core, unsigned level, bool is_data) 
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (core >= CPU_nCores(self)) {
		return 0;
	}

	unsigned size = 0;

	allocate_cache_arrays (self);

#if defined(__x86_64__) || defined(__linux__)
	if (!size) {
		bool got_data = false;
#if defined(__x86_64__)
		got_data = intel_x86_64_fetch_cache_info (self);
#endif
#if defined(__linux__)
		if (!got_data) {
			got_data = linux_fetch_cache_info (self);
		}
#endif
		if (!got_data) {
			return 0;
		}
		switch (level) {
		case 1:
			if (is_data) {
				size = self->level1d_sizes[core];
			} else {
				size = self->level1i_sizes[core];
			}
			break;
		case 2:
			size = self->level2_sizes[core];
			break;
		case 3:
			size = self->level3_sizes[core];
			break;
		}
	}
#endif

#if defined(__APPLE__)
	if (!size) {
		uint64_t value = 0;
		size_t len = sizeof(value);
		const char *name = NULL;
		switch (level) {
		case 1:
			name = is_data ? "hw.l1dcachesize" : "hw.l1icachesize";
			break;
		case 2:
			name = "hw.l2cachesize";
			break;
		case 3:
			name = "hw.l3cachesize";
			break;
		}
		if (!sysctlbyname (name, &value, &len, NULL, 0)) {
			unsigned long nbytes = value;
			if (nbytes > 0) {
				size = nbytes >> 10;
			}
		}
	}
#endif 

	return size;
}

/* Get the CPU temperature by physical core.
 * If hyperthreading is enabled, divide the core number by 2.
 */
static float CPU_temperature (CPU *self, unsigned sought_core) 
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,CPU);

	if (sought_core >= CPU_nCores(self)) {
		return 0.f;
	}

#if defined(__linux__) 
	/* For reference, my BASH function.
	function t3 {
		  pushd /sys/bus/platform/devices > /dev/null
			  cd coretemp.0/hwmon/hwmon?
			  for a in temp?_label; do
				I=`echo $a | sed "s/label/input/"`
				L=`cat $a`
				T1000=`cat $I`
				T=$(($T1000/1000))
				echo $L: $T"°C"
			  done
		  popd > /dev/null
	}
	*/

	// NOTE: We have to examine the labels to find the
	// correct file with the core number that we want.

	char hwmon_path[PATH_MAX];
	snprintf (hwmon_path, sizeof(hwmon_path), "/sys/bus/platform/devices/coretemp.0/hwmon");
	DIR *dir = opendir (hwmon_path);
	if (!dir) {
		return 0.;
	}
	// Just get the first directory.
	struct dirent *de;
	bool found = false;
	while ((de = readdir(dir))) {
		if (has_prefix (de->d_name, "hwmon")) {
			if (isdigit(de->d_name[5])) {
				strncat (hwmon_path, "/", sizeof(hwmon_path)-1);
				strncat (hwmon_path, de->d_name, sizeof(hwmon_path)-1);
				strncat (hwmon_path, "/", sizeof(hwmon_path)-1);
				found = true;
				break;
			}
		}
	}
	closedir (dir);
	if (!found) {
		return false;
	}
	int temperature_index = 1;
	int pathlen = strlen (hwmon_path);
	while (temperature_index < 128) {
		snprintf (hwmon_path+pathlen, sizeof(hwmon_path) - pathlen, "temp%d_label", temperature_index);
		char line [256];
		unsigned len = read_trimmed_line_from_file (hwmon_path, line, 256);
		if (!len) {
			break;
		}
		if (has_prefix (line, "Core ")) {
			int found_core = atoi (line + 5);
			if (found_core == sought_core) {
				hwmon_path[pathlen] = 0;
				snprintf (hwmon_path+pathlen, sizeof(hwmon_path) - pathlen, "temp%d_input", temperature_index);
				len = read_trimmed_line_from_file (hwmon_path, line, 256);
				if (len) {
					int value = atoi (line);
					return (float)value / 1000.f;
				}
			}
		}
		temperature_index++;
	}
#endif

	return 0.f;
}

CPUClass* CPUClass_init (CPUClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(CPU,describe);
        SET_OVERRIDDEN_METHOD_POINTER(CPU,destroy);
        SET_OVERRIDDEN_METHOD_POINTER(CPU,equals);

	SET_METHOD_POINTER(CPU,family);
	SET_METHOD_POINTER(CPU,make);
	SET_METHOD_POINTER(CPU,model);
	SET_METHOD_POINTER(CPU,nCores);
	SET_METHOD_POINTER(CPU,features);
	SET_METHOD_POINTER(CPU,registerSize);
	SET_METHOD_POINTER(CPU,instructionSet);
	SET_METHOD_POINTER(CPU,levelNCacheSize);
	SET_METHOD_POINTER(CPU,hasVectorUnit);
	SET_METHOD_POINTER(CPU,has128bitVectors);
	SET_METHOD_POINTER(CPU,has256bitVectors);
	SET_METHOD_POINTER(CPU,has512bitVectors);
	SET_METHOD_POINTER(CPU,maximumSpeed);
	SET_METHOD_POINTER(CPU,minimumSpeed);
	SET_METHOD_POINTER(CPU,currentSpeed);
	SET_METHOD_POINTER(CPU,temperature);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

