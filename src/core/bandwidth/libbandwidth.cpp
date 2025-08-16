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

/* Compute CPU cache speed */
int call_bandwidth([[maybe_unused]] Data &data)
{
	int err = 0;
#if HAS_LIBCPUID
	static bool first = true;
	static struct BandwidthData bwd{};

	if(data.caches.get_selected_cpu_type().caches.size() == 0)
		return 1;

	for(std::size_t i = 0; i < data.caches.get_selected_cpu_type().caches.size(); i++)
		bwd.cache_size[i] = data.caches.get_selected_cpu_type().caches[i].size_i;

	MSG_VERBOSE("%s", _("Calling bandwidth"));
	bwd.selected_test = Options::get_selected_test();
	if(first)
	{
		/* Init BandwidthData */
		bwd.test_name = NULL;
		pthread_mutex_init(&bwd.mutex, NULL);

		/* Call bandwidth */
		err   = bandwidth_cpux(&bwd);
		first = false;

		/* Copy test names */
		for(uint8_t i = 0; i < BANDWIDTH_LAST_TEST; i++)
		{
			data.caches.test.names.push_back(bwd.test_name[i]);
			free(bwd.test_name[i]);
		}
		free(bwd.test_name);
	}
	else
	{
		/* Run bandwidth in a separated thread if not running */
		if(pthread_mutex_trylock(&bwd.mutex) != EBUSY)
		{
			pthread_mutex_unlock(&bwd.mutex);
			std::thread bwt(bandwidth_cpux, &bwd);
			MSG_DEBUG("%s", "call_bandwidth: created new thread");
			bwt.detach();
		}
		else
			MSG_DEBUG("%s", "call_bandwidth: a previous thread is still running");
	}

	/* Speed labels */
	for(std::size_t i = 0; i < data.caches.get_selected_cpu_type().caches.size(); i++)
		data.caches.get_selected_cpu_type().caches[i].speed.value = string_format("%.2f %s/s", double(bwd.cache_speed[i]) / 10.0, UNIT_MB);
#endif /* HAS_LIBCPUID */

	return err;
}
