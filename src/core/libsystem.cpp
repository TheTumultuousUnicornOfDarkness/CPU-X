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
* FILE core/libsystem.cpp
*/

#include <cmath>
#if HAS_LIBPROC2
# include <libproc2/meminfo.h>
# include <libproc2/misc.h>
#elif HAS_LIBPROCPS
# include <proc/sysinfo.h>
#elif HAS_LIBSTATGRAB
# include <statgrab.h>
#endif
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "internal.hpp"


/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
int system_dynamic(Data &data)
{
	int err = 0;
	time_t uptime_s = 0;
	struct tm *tm;
	static PrefixUnit pu_mem, pu_swap;

#if HAS_LIBPROC2
	double up_secs;
	struct meminfo_info *mem_info = NULL;

	MSG_VERBOSE("%s", _("Calling libprocps"));
	/* System uptime */
	if(procps_uptime(&up_secs, NULL) < 0)
		MSG_ERRNO("%s", _("unable to get system uptime"));
	else
		uptime_s = (time_t) up_secs;

	/* Memory variables */
	if(procps_meminfo_new(&mem_info) < 0)
		MSG_ERRNO("%s", _("unable to create meminfo structure"));
	else
	{
		if(!pu_mem.init || !pu_swap.init)
		{
			pu_mem.find_best_binary_prefix(MEMINFO_GET(mem_info, MEMINFO_MEM_TOTAL,  ul_int),  PrefixUnit::Multipliers::MULT_K);
			pu_swap.find_best_binary_prefix(MEMINFO_GET(mem_info, MEMINFO_SWAP_TOTAL, ul_int), PrefixUnit::Multipliers::MULT_K);
		}
		data.system.memory.mem_used    = MEMINFO_GET(mem_info, MEMINFO_MEM_USED,       ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_buffers = MEMINFO_GET(mem_info, MEMINFO_MEM_BUFFERS,    ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_cached  = MEMINFO_GET(mem_info, MEMINFO_MEM_CACHED_ALL, ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_free    = MEMINFO_GET(mem_info, MEMINFO_MEM_FREE,       ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_total   = MEMINFO_GET(mem_info, MEMINFO_MEM_TOTAL,      ul_int) / (long double) pu_mem.divisor;
		data.system.memory.swap_used   = MEMINFO_GET(mem_info, MEMINFO_SWAP_USED,      ul_int) / (long double) pu_swap.divisor;
		data.system.memory.swap_total  = MEMINFO_GET(mem_info, MEMINFO_SWAP_TOTAL,     ul_int) / (long double) pu_swap.divisor;
		procps_meminfo_unref(&mem_info);
	}
#endif /* HAS_LIBPROC2 */

#if HAS_LIBPROCPS
	MSG_VERBOSE("%s", _("Calling libprocps"));
	/* System uptime */
	uptime_s = (time_t) uptime(NULL, NULL);

	/* Memory variables */
	meminfo();
	if(!pu_mem.init || !pu_swap.init)
	{
		pu_mem.find_best_binary_prefix(kb_main_total,  PrefixUnit::Multipliers::MULT_K);
		pu_swap.find_best_binary_prefix(kb_swap_total, PrefixUnit::Multipliers::MULT_K);
	}
	data.system.memory.mem_used    = kb_main_used    / (long double) pu_mem.divisor;
	data.system.memory.mem_buffers = kb_main_buffers / (long double) pu_mem.divisor;
	data.system.memory.mem_cached  = kb_main_cached  / (long double) pu_mem.divisor;
	data.system.memory.mem_free    = kb_main_free    / (long double) pu_mem.divisor;
	data.system.memory.mem_total   = kb_main_total   / (long double) pu_mem.divisor;
	data.system.memory.swap_used   = kb_swap_used    / (long double) pu_swap.divisor;
	data.system.memory.swap_total  = kb_swap_total   / (long double) pu_swap.divisor;
#endif /* HAS_LIBPROCPS */

#if HAS_LIBSTATGRAB
	static bool called = false;
	sg_mem_stats *mem; /* Memory labels */
	sg_swap_stats *swap;
	sg_host_info *info;

	MSG_VERBOSE("%s", _("Calling libstatgrab"));
	/* Libstatgrab initialization */
	if(!called)
	{
		err += sg_init(0);
		called = true;
	}
	mem  = sg_get_mem_stats(NULL);
	swap = sg_get_swap_stats(NULL);
	info = sg_get_host_info(NULL);

	/* System uptime */
	uptime_s = info->uptime;

	/* Memory variables */
	if(!pu_mem.init || !pu_swap.init)
	{
		pu_mem.find_best_binary_prefix(mem->total,   PrefixUnit::Multipliers::MULT_NONE);
		pu_swap.find_best_binary_prefix(swap->total, PrefixUnit::Multipliers::MULT_NONE);
	}
	data.system.memory.mem_used    = mem->used   / (long double) pu_mem.divisor;
	data.system.memory.mem_buffers = 0;
	data.system.memory.mem_cached  = mem->cache  / (long double) pu_mem.divisor;
	data.system.memory.mem_free    = mem->free   / (long double) pu_mem.divisor;
	data.system.memory.mem_total   = mem->total  / (long double) pu_mem.divisor;
	data.system.memory.swap_used   = swap->used  / (long double) pu_swap.divisor;
	data.system.memory.swap_total  = swap->total / (long double) pu_swap.divisor;
#endif /* HAS_LIBSTATGRAB */

	/* Memory labels */
	if(data.system.memory.mem_total > 0)
	{
		const int int_digits =  std::log10(data.system.memory.mem_total) + 4;
		data.system.memory.used.value    = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_used,    pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
		data.system.memory.buffers.value = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_buffers, pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
		data.system.memory.cached.value  = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_cached,  pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
		data.system.memory.free.value    = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_free,    pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
	}
	if(data.system.memory.swap_total > 0)
	{
		const int int_digits = std::log10(data.system.memory.swap_total) + 4;
		data.system.memory.swap.value = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.swap_used, pu_swap.prefix, int_digits, data.system.memory.swap_total, pu_swap.prefix);
	}

	/* Uptime label */
	if(uptime_s > 0)
	{
		tm = gmtime(&uptime_s);
		data.system.os.uptime.value = string_format(_("%i days, %i hours, %i minutes, %i seconds"), tm->tm_yday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}

	return err;
}
