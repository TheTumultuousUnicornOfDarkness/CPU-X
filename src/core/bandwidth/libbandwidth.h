/****************************************************************************
*    Copyright © 2014-2025 The Tumultuous Unicorn Of Darkness
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
****************************************************************************/

/*
* PROJECT CPU-X
* FILE core/bandwidth/libbandwidth.h
*/

#ifndef _LIBBANDWIDTH_H_
#define _LIBBANDWIDTH_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BANDWIDTH_MAX_CACHE_LEVEL  4
#define TEST_NAME_BUFFER_LEN      64

enum EnTests
{
	// Sequential reads
	SEQ_WS_R,
	SEQ_128_R,
	SEQ_256_R,
	SEQ_512_R,
	// Sequential nontemporal reads
	SEQ_WS_NT_R,
	SEQ_128_NT_R,
	SEQ_256_NT_R,
	SEQ_512_NT_R,
	// Sequential writes
	SEQ_WS_W,
	SEQ_128_W,
	SEQ_256_W,
	SEQ_512_W,
	// Sequential nontemporal writes
	SEQ_WS_NT_W,
	SEQ_128_NT_W,
	SEQ_256_NT_W,
	SEQ_512_NT_W,
	// Random reads
	RAND_WS_R,
	RAND_128_R,
	RAND_256_R,
	// Random nontemporal reads
	RAND_128_NT_R,
	RAND_256_NT_R,
	// Random writes
	RAND_WS_W,
	RAND_128_W,
	RAND_256_W,
	// Random nontemporal writes
	RAND_128_NT_W,
	RAND_256_NT_W,
	// Sequential copy
	SEQ_WS_C,
	SEQ_128_C,
	SEQ_256_C,
	SEQ_512_C,
	// Sentinel value
	BANDWIDTH_LAST_TEST
};

struct Tests
{
	enum EnTests test;
	char         *name;
	bool         support_instr;
	long         (*func_ptr) (void*, unsigned long, int, bool);
	int          testing_mode;
	bool         random;
};

struct BandwidthData
{
	uint8_t  selected_test;
	uint32_t cache_size[BANDWIDTH_MAX_CACHE_LEVEL];
	uint32_t cache_speed[BANDWIDTH_MAX_CACHE_LEVEL];
	bool     *test_supported;
	char     **test_name;
	pthread_mutex_t mutex;
};


int bandwidth_main(int argc, char *const argv[]);
int bandwidth_cpux(struct BandwidthData *bwd);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
