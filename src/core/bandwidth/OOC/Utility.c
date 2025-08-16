/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2021, 2023 Zack T Smith.
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

#include "ObjectOriented.h"
#include "Utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h> // isspace

//----------------------------------------------------------------------------
// Name:	get_file_size
// Purpose:	Obtain the size of a file.
// Returns:	Size in bytes or -1 on error.
//----------------------------------------------------------------------------
long long get_file_size (const char *path)
{
	struct stat st;
	if (stat (path, &st)) {
		perror ("stat");
		return -1;
	}
	return (long long) st.st_size;
}

//----------------------------------------------------------------------------
// Name:	file_exists
// Purpose:	Determine if a file exists at specified path.
// Returns:	True if it exists, false if not.
//----------------------------------------------------------------------------
bool file_exists (const char *path)
{
	struct stat st;
	if (!stat (path, &st)) {
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
// Name:read_trimmed_line_from_file
// Purpose:	Reads first line of a file, removes whitespace from start+end.
// Returns:	Line length.
//----------------------------------------------------------------------------
unsigned read_trimmed_line_from_file (const char *path, char *result, unsigned maxlen)
{
	if (!path || !result || !maxlen) {
		return 0;
	}

	FILE *f = fopen (path, "r");
	if (!f) {
		return 0;
	}

	int ch;
	unsigned index = 0;
	bool found_nonwhitespace = false;
	while (EOF != (ch = fgetc(f))) {
		if (ch == '\n')
			break;

		// Trim the start.
		if (!found_nonwhitespace && isspace(ch))
			continue;

		found_nonwhitespace = true;
		result[index++] = ch;

		if (index == maxlen-1)
			break;
	}
	result[index] = 0;

	// Trim the end.
	unsigned length = index;
	while (index > 0) {
		index--;
		uint8_t *result_unsigned = (uint8_t*) result;
		uint8_t ch = result_unsigned[index];
		if (isspace((int) ch)) {
			result[index] = 0;
			length--;
		}
		else {
			break;
		}
	}

	fclose (f);
	return length;
}

//----------------------------------------------------------------------------
// Name:	read_float_from_file
// Purpose:	Reads a single floating-point number from a file.
// Returns:	Stores float in result, returns true if success, else false.
//----------------------------------------------------------------------------
bool read_float_from_file (const char *path, float *return_value)
{
	if (!path || !return_value) {
		return false;
	}
	FILE *f = fopen (path, "r");
	if (!f) {
		return false;
	}

	int count = fscanf (f, "%f", return_value);
	fclose (f);
	return count == 1;
}

//----------------------------------------------------------------------------
// Name:	read_hex_from_file
// Purpose:	Reads a single hexadecimal number from a file.
// Returns:	Stores hex in result, returns true if success, else false.
//----------------------------------------------------------------------------
bool read_hex_from_file (const char *path, unsigned long *return_value)
{
	if (!path || !return_value) {
		return false;
	}
	FILE *f = fopen (path, "r");
	if (!f) {
		return false;
	}

	int count = fscanf (f, "0x%lx", return_value);
	fclose (f);
	return count == 1;
}

//----------------------------------------------------------------------------
// Name:	read_unsigned_from_file
// Purpose:	Reads a single unsigned int from a file.
// Returns:	Stores long in result, returns true if success, else false.
//----------------------------------------------------------------------------
bool read_unsigned_from_file (const char *path, unsigned *return_value)
{
	if (!path || !return_value) {
		return false;
	}
	FILE *f = fopen (path, "r");
	if (!f) {
		return false;
	}

	int count = fscanf (f, "%u", return_value);
	fclose (f);

	return count == 1;
}

//----------------------------------------------------------------------------
// Name:	read_longlong_from_file
// Purpose:	Reads a single long long int from a file.
// Returns:	Stores long in result, returns true if success, else false.
//----------------------------------------------------------------------------
bool read_longlong_from_file (const char *path, long long *return_value)
{
	if (!path || !return_value) {
		return false;
	}
	FILE *f = fopen (path, "r");
	if (!f) {
		return false;
	}

	int count = fscanf (f, "%lld", return_value);
	fclose (f);

	return count == 1;
}

//----------------------------------------------------------------------------
// Name:	get_file_line_count
// Purpose:	Counts the number of text lines in a text file.
// Returns:	Count, or -1 on error.
//----------------------------------------------------------------------------
long get_file_line_count (const char *path)
{
	FILE *f = fopen (path, "r");
	if (!f) {
		return -1;
	}

	long lineCount = 0;

#define BUFSIZE 4096
	char buffer[BUFSIZE];
	char lastChar = 0;
	while (!feof (f)) {
		long len = fread (buffer, 1, BUFSIZE, f);
		if (len <= 0)
			break;
		for (long i=0; i < len; i++) {
			lastChar = buffer[i];
			if (lastChar == '\n')
				lineCount++;
		}
	}
	if (lastChar != '\n')
		lineCount++; // Partial last line.

	return lineCount;
}

//----------------------------------------------------------------------------
// Name:	has_prefix
// Purpose:	Check whether C string has specified prefix, case-sensitive.
// Returns:	True if it has the prefix.
//----------------------------------------------------------------------------
bool has_prefix (const char *string, const char *prefix)
{
	if (!string || !prefix)
		return false;

	size_t stringLength = strlen(string);
	size_t prefixLength = strlen(prefix);
	if (prefixLength > stringLength)
		return false;

	return 0 == strncmp (string, prefix, prefixLength);
}

//----------------------------------------------------------------------------
// Name:	has_suffix
// Purpose:	Check whether C string has specified suffix, case-sensitive.
// Returns:	True if it has the suffix.
//----------------------------------------------------------------------------
bool has_suffix (const char *string, const char *suffix)
{
	if (!string || !suffix)
		return false;

	size_t stringLength = strlen(string);
	size_t suffixLength = strlen(suffix);
	if (suffixLength > stringLength)
		return false;

	char *ending = (char*)string + stringLength - suffixLength;
	return 0 == strcmp (ending, suffix);
}

char *execute_and_return_first_line (char *cmd)
{
#define SYSTEM_TEMP_FILE TMPDIR"/.ooc-system.dat"
#define MAX_EXECUTED_RESULT_LINE_LEN 1024
	static char result [MAX_EXECUTED_RESULT_LINE_LEN];
	result[0] = 0;
	char *filename = SYSTEM_TEMP_FILE;
	char full_cmd [1024];
	snprintf (full_cmd, sizeof(full_cmd), "%s > %s 2> /dev/null", cmd, filename);
	int retval = system(full_cmd);
	bool success = false;
	if (!retval) {
		if (0 < read_trimmed_line_from_file (filename, result, MAX_EXECUTED_RESULT_LINE_LEN)) {
			success = true;
		}
	}
	unlink (filename);
	return success ? result : NULL;
}
