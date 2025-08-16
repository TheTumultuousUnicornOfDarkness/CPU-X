#ifndef TYPES_H
#define TYPES_H

#include "config.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;
typedef unsigned long long int u64;

/*
 * You may use the following defines to adjust the type definitions
 * depending on the architecture:
 * - Define BIGENDIAN on big-endian systems.
 * - Define ALIGNMENT_WORKAROUND if your system doesn't support
 *   non-aligned memory access. In this case, we use a slower, but safer,
 *   memory access method. This should be done automatically in config.h
 *   for architectures which need it.
 */

/*
 * Per SMBIOS v2.8.0 and later, all structures assume a little-endian
 * ordering convention.
 */
#if defined(ALIGNMENT_WORKAROUND) || defined(BIGENDIAN)
#define WORD(x) (u16)((x)[0] + ((x)[1] << 8))
#define DWORD(x) (u32)((x)[0] + ((x)[1] << 8) + ((x)[2] << 16) + ((x)[3] << 24))
#define QWORD(x) (((u64)DWORD((x) + 4) << 32) + DWORD(x))
#else /* ALIGNMENT_WORKAROUND || BIGENDIAN */
#define WORD(x) (u16)(*(const u16 *)(x))
#define DWORD(x) (u32)(*(const u32 *)(x))
#define QWORD(x) (u64)(*(const u64 *)(x))
#endif /* ALIGNMENT_WORKAROUND || BIGENDIAN */

#endif
