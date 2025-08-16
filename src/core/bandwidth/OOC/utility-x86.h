
#ifndef _UTILITY_x86_H
#define _UTILITY_x86_H

//----------------------------------------
// utility-x86*.asm routines and constants
//
extern unsigned get_cpuid_cache_info (uint32_t *, unsigned);

extern void get_cpuid_family (char *family_return); // for 32bit
extern unsigned get_cpuid_family1 (); // for 64-bit
extern unsigned get_cpuid_family2 (); // for 64-bit
extern unsigned get_cpuid_family3 (); // for 64-bit

extern void get_cpuid_model (char *model_return); // returns 48 bytes

extern unsigned get_cpuid1_ecx ();
extern unsigned get_cpuid1_edx ();
extern unsigned get_cpuid7_ebx ();
extern unsigned get_cpuid7_ecx ();
extern unsigned get_cpuid7_edx ();
extern unsigned get_cpuid8_ecx ();
extern unsigned get_cpuid_80000001_ecx ();
extern unsigned get_cpuid_80000001_edx ();

#define CPUID80000001_ECX_SSE4A (1<<6)
#define CPUID80000001_ECX_LZCNT (1<<5)
#define CPUID80000001_ECX_PREFETCHW (1<<8)
#define CPUID80000001_EDX_INTEL64 (1<<29)	// "Long Mode" on AMD.
#define CPUID80000001_EDX_NX (1<<20)
#define CPUID80000001_EDX_MMXEXT (1<<22)

#define CPUID1_EDX_CMOV (1<<15)
#define CPUID1_EDX_ACPI (1<<22)
#define CPUID1_EDX_MMX (1<<23)
#define CPUID1_EDX_SSE (1<<25)
#define CPUID1_EDX_SSE2 (1<<26)
#define CPUID1_EDX_HTT (1<<28)
#define CPUID1_EDX_TM (1<<29)

#define CPUID1_ECX_SSE3 (1)
#define CPUID1_ECX_TM2 (1<<8)
#define CPUID1_ECX_SSSE3 (1<<9)
#define CPUID1_ECX_SSE41 (1<<19)
#define CPUID1_ECX_SSE42 (1<<20)
#define CPUID1_ECX_POPCNT (1<<23)
#define CPUID1_ECX_AES (1<<25)	// Encryption.
#define CPUID1_ECX_AVX (1<<28)	// 256-bit YMM registers.
#define CPUID1_ECX_F16C (1<<29)
#define CPUID1_ECX_HYPER_GUEST (1<<31)

#define CPUID7_EBX_SGX (1<<2)
#define CPUID7_EBX_BMI1 (1<<3)
#define CPUID7_EBX_HLE (1<<4)
#define CPUID7_EBX_AVX2 (1<<5)
#define CPUID7_EBX_BMI2 (1<<8)
#define CPUID7_EBX_MPX (1<<14)
#define CPUID7_EBX_AVX512_F (1<<16)
#define CPUID7_EBX_AVX512_DQ (1<<17)
#define CPUID7_EBX_RDSEED (1<<18)
#define CPUID7_EBX_ADX (1<<19)
#define CPUID7_EBX_AVX512_IFMA (1<<21)
#define CPUID7_EBX_AVX512_PF (1<<26)
#define CPUID7_EBX_AVX512_ER (1<<27)
#define CPUID7_EBX_AVX512_CD (1<<28)
#define CPUID7_EBX_SHA (1<<29)
#define CPUID7_EBX_AVX512_BW (1<<30)
#define CPUID7_EBX_AVX512_VL (1<<31)

#define CPUID7_ECX_AVX512_VBMI (1<<1)
#define CPUID7_ECX_AVX512_VBMI2 (1<<6)
#define CPUID7_ECX_CET (1<<7)
#define CPUID7_ECX_VAES (1<<9)
#define CPUID7_ECX_AVX512_VNNI (1<<11)
#define CPUID7_ECX_AVX512_BITALG (1<<12)
#define CPUID7_ECX_AVX512_VPOPCNTDQ (1<<14)
#define CPUID7_ECX_MOVDIRI (1<<27)
#define CPUID7_ECX_MOVDIR64B (1<<28)

#define CPUID7_ECX1_EAX_SHA512 (1)
#define CPUID7_ECX1_EAX_AMX_FP16 (1<<21)

#define CPUID7_EDX_AVX512_4VNNIW (1<<2)
#define CPUID7_EDX_AVX512_4FMAPS (1<<3)
#define CPUID7_EDX_AVX512_VP2INTERSECT (1<<8)
#define CPUID7_EDX_HYBRID (1<<15)
#define CPUID7_EDX_AMX_BF16 (1<<22)
#define CPUID7_EDX_AMX_TILE (1<<24)
#define CPUID7_EDX_AMX_INT8 (1<<25)
#define CPUID7_EDX_AVX512_FP16 (1<<23)

#endif

