/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2021 Zack T Smith.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * The author may be reached at 1 at zsmith dot co.
 *=======================================================================*/

#ifndef _UTILITY_H
#define _UTILITY_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

extern bool file_exists (const char *path);
extern long long get_file_size (const char *path);
extern long get_file_line_count (const char *path);
extern bool read_float_from_file (const char *path, float *);
extern bool read_hex_from_file (const char *path, unsigned long*);
extern bool read_unsigned_from_file (const char *path, unsigned *);
extern bool read_longlong_from_file (const char *path, long long *);
extern unsigned read_trimmed_line_from_file (const char *path, char *result, unsigned maxlen);
extern bool has_prefix (const char *string, const char *prefix);
extern bool has_suffix (const char *string, const char *suffix);
extern char *execute_and_return_first_line (char *cmd);

#endif
