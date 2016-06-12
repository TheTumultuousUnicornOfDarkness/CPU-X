/****************************************************************************
*    Copyright © 2014-2016 Xorg
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

/* Calculate cache speed for CPU-X in Caches tab */
#define CPU_X_GET_CACHE_SPEED_P1 \
if(!BANDWIDTH_MODE && chunk_size > cache_size * 1024) \
{ \
	data->w_data->speed[cache_level - LEVEL1I] = total_amount / count; \
	count        = 0; \
	total_amount = 0; \
	cache_level++; \
\
	if(cache_level > LEVEL3) \
		return 0; \
\
	cache_size = (cache_level == LEVEL2) ? data->w_data->l2_size : data->w_data->l3_size; \
	if(cache_size < 1) \
		return 1; \
}

#define CPU_X_GET_CACHE_SPEED_P2 \
if(!BANDWIDTH_MODE) \
{ \
	total_amount += amount; \
	count++; \
}


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
#ifdef __x86_64__
	SEQ_64_R,
	RAND_64_R,
	SEQ_64_W,
	RAND_64_W,
	SEQ_64_LR,
#else
	SEQ_32_R,
	RAND_32_R,
	SEQ_32_W,
	RAND_32_W,
#endif
	SEQ_128_C,
	SEQ_256_C,
	SEQ_32_LR,
	SEQ_16_LR,
	SEQ_8_LR,
	LASTTEST
};

static const struct Tests
{
	enum EnTests test;
	char *name;
} tests[] =
{
	{ SEQ_128_R,         "Sequential 128-bit reads"            },
	{ SEQ_256_R,         "Sequential 256-bit reads"            },
	{ RAND_128_R,        "Random 128-bit reads"                },
	{ SEQ_128_CACHE_W,   "Sequential 128-bit cache writes"     },
	{ SEQ_256_CACHE_W,   "Sequential 256-bit cache writes"     },
	{ RAND_128_CACHE_W,  "Random 128-bit cache writes"         },
	{ SEQ_128_BYPASS_R,  "Sequential 128-bit bypassing reads"  },
	{ RAND_128_BYPASS_R, "Random 128-bit bypassing reads"      },
	{ SEQ_128_BYPASS_W,  "Sequential 128-bit bypassing writes" },
	{ SEQ_256_BYPASS_W,  "Sequential 256-bit bypassing writes" },
	{ RAND_128_BYPASS_W, "Random 128-bit bypassing writes"     },
#ifdef __x86_64__
	{ SEQ_64_R,          "Sequential 64-bit reads"             },
	{ RAND_64_R,         "Random 64-bit reads"                 },
	{ SEQ_64_W,          "Sequential 64-bit writes"            },
	{ RAND_64_W,         "Random 64-bit writes"                },
	{ SEQ_64_LR,         "Sequential 64-bit LODSQ reads"       },
#else
	{ SEQ_32_R,          "Sequential 32-bit reads"             },
	{ RAND_32_R,         "Random 32-bit reads"                 },
	{ SEQ_32_W,          "Sequential 32-bit writes"            },
	{ RAND_32_W,         "Random 32-bit writes"                },
#endif
	{ SEQ_128_C,         "Sequential 128-bit copy"             },
	{ SEQ_256_C,         "Sequential 256-bit copy"             },
	{ SEQ_32_LR,         "Sequential 32-bit LODSD reads"       },
	{ SEQ_16_LR,         "Sequential 16-bit LODSW reads"       },
	{ SEQ_8_LR,          "Sequential 8-bit LODSB reads"        },
};

int bandwidth(void *p_data);


#endif
