/****************************************************************************
*    Copyright © 2014-2019 Xorg
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
* FILE libbandwidth.h
*/

#ifndef _LIBBANDWIDTH_H_
#define _LIBBANDWIDTH_H_

#define BANDWIDTH_MODE (opts->output_type == OUT_BANDWIDTH)


enum EnTests
{
	SEQ_128_R,
	SEQ_256_R,
	RAND_128_R,
	SEQ_128_CACHE_W,
	SEQ_256_CACHE_W,
	RAND_128_CACHE_W,
	SEQ_128_BYPASS_R,
	RAND_128_BYPASS_R,
	SEQ_128_BYPASS_W,
	SEQ_256_BYPASS_W,
	RAND_128_BYPASS_W,
	SEQ_128_C,
	SEQ_256_C,
	SEQ_32_LR,
	SEQ_16_LR,
	SEQ_8_LR,
#ifdef __x86_64__
	SEQ_64_R,
	RAND_64_R,
	SEQ_64_W,
	RAND_64_W,
	SEQ_64_LR,
	RAND_256_R,
	RAND_256_W,
#else
	SEQ_32_R,
	RAND_32_R,
	SEQ_32_W,
	RAND_32_W,
#endif
	LASTTEST
};

struct Tests
{
	enum EnTests test;
	char     *name;
	uint32_t color;
	bool     need_flag;
	bool     need_mask;
	int      (*func_ptr)(unsigned long, int, bool);
	int      mode;
	bool     random;
};

int bandwidth_main(int argc, char **argv);
int bandwidth_cpux(void *p_data);


#endif
