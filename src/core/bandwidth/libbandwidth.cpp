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
* FILE core/bandwidth/libbandwidth.cpp
*/

#include <cstdint>
#include "libbandwidth.h"
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "../internal.hpp"
#include "../../daemon/daemon.h"


/* Call Bandwidth through CPU-X but do nothing else */
int run_bandwidth(std::list<std::string> &args)
{
	char **argv = transform_string_list_to_char_array("bandwidth (built-in with CPU-X)", args);

	return bandwidth_main(args.size() + 1, argv);
}

#if HAS_LIBCPUID
static int init_bandwidth(Data &data, struct BandwidthData &bwd)
{
	int err = 0;

	/* Init BandwidthData */
	bwd.test_name = NULL;
	pthread_mutex_init(&bwd.mutex, NULL);

	/* Call bandwidth */
	err = bandwidth_cpux(&bwd);

	/* Copy test names */
	for(uint8_t i = 0; i < BANDWIDTH_LAST_TEST; i++)
	{
		data.caches.test.names.push_back(bwd.test_name[i]);
		free(bwd.test_name[i]);
	}
	free(bwd.test_name);

	return err;
}

static void call_bandwidth_in_thread(Data &data, struct BandwidthData &bwd, bool wait_thread)
{
	/* Run bandwidth in a separated thread if not running */
	if(pthread_mutex_trylock(&bwd.mutex) != EBUSY)
	{
		pthread_mutex_unlock(&bwd.mutex);
		std::thread bwt(bandwidth_cpux, &bwd);
		MSG_DEBUG("call_bandwidth: created new thread (wait_thread=%i)", wait_thread);
		wait_thread ? bwt.join() : bwt.detach();
	}
	else
		MSG_DEBUG("%s", "call_bandwidth: a previous thread is still running");

	/* Speed labels */
	for(std::size_t i = 0; i < data.caches.get_selected_cpu_type().caches.size(); i++)
		data.caches.get_selected_cpu_type().caches[i].speed.value = string_format("%.2f %s/s", double(bwd.cache_speed[i]) / 10.0, UNIT_MB);
}
#endif /* HAS_LIBCPUID */

/* Compute CPU cache speed */
int call_bandwidth([[maybe_unused]] Data &data)
{
	int err = 0;
#if HAS_LIBCPUID
	static bool first = true;
	static struct BandwidthData bwd{};

	MSG_VERBOSE("%s", _("Calling bandwidth"));
	bwd.selected_test = Options::get_selected_test();

	if(first)
	{
		first = false;

		/* Save current selected CPU type and core number */
		const auto saved_selected_type = Options::get_selected_type();
		const auto saved_selected_core = Options::get_selected_core();

		/* Set test names */
		err = init_bandwidth(data, bwd);

		for(std::size_t cpu_type = 0; cpu_type < data.caches.cpu_types.size(); cpu_type++)
		{
			Options::set_selected_type(cpu_type);

			/* Ensure cache level are initialized for the CPU type */
			if(data.caches.get_selected_cpu_type().caches.size() == 0)
				return 1;

			/* Initialize cache sizes for CPU types */
			for(std::size_t i = 0; i < data.caches.get_selected_cpu_type().caches.size(); i++)
				bwd.cache_size[i] = data.caches.get_selected_cpu_type().caches[i].size_i;

			/* Call bandwidth */
			call_bandwidth_in_thread(data, bwd, true);
		}

		/* Restore selected CPU type and core number */
		Options::set_selected_type(saved_selected_type);
		Options::set_selected_core(saved_selected_core);
	}
	else
		call_bandwidth_in_thread(data, bwd, false);
#endif /* HAS_LIBCPUID */

	return err;
}
