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
* FILE core/benchmarks.cpp
*/

#include <cmath>
#include <thread>
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"

#ifndef __linux__
# include <pthread_np.h>
# include <sys/cpuset.h>
#endif


/* Compute all prime numbers in 'duration' seconds */
static void primes_bench(Data *data)
{
	uint_fast64_t i, number;
	Data::Bench::PrimeNumbers *bench = data->bench.fast_mode ? static_cast<Data::Bench::PrimeNumbers*>(&data->bench.prime_fast) : static_cast<Data::Bench::PrimeNumbers*>(&data->bench.prime_slow);

	while(data->bench.is_running)
	{
		/* data->bench.number is shared by all threads */
		number = (++bench->number);

		/* Slow mode: loop from i to number, prime if number == i
		   Fast mode: loop from i to sqrt(number), prime if number mod i != 0 */
		const uint_fast64_t sup = data->bench.fast_mode ? sqrt(number) : number;
		for(i = 2; (i < sup) && (number % i != 0); i++);

		if((data->bench.fast_mode && number % i) || (!data->bench.fast_mode && number == i))
			bench->primes++;
	}
}

/* Stop all threads running benchmark */
static void stop_benchmarks(Data *data)
{
	std::time_t start, end;

	/* Wait until the time is up or until user stops benchmark */
	std::time(&start);
	while((data->bench.parameters.elapsed_i < (data->bench.parameters.duration_i * 60)) && data->bench.is_running)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::time(&end);
		data->bench.parameters.elapsed_i = end - start;
	}

	if(data->bench.parameters.elapsed_i >= (data->bench.parameters.duration_i * 60))
		data->bench.is_completed = true;

	/* Exit all threads */
	data->bench.is_running = false;
	for(uint_fast16_t i = 0; i < data->bench.compute_threads.size(); i++)
	{
		data->bench.compute_threads[i].join();
		MSG_DEBUG("stop_benchmarks: stopped thread #%u", i);
	}
	data->bench.compute_threads.clear();
}

/* Report score of benchmarks */
int benchmark_status(Data &data)
{
	if(!data.bench.did_run)
		return 0;

	Data::Bench::PrimeNumbers *bench = data.bench.fast_mode ? static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_fast) : static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_slow);

	MSG_VERBOSE("%s", _("Updating benchmark status"));
	if(data.bench.is_running)
	{
		bench->state.value = _("Active");
		bench->score.value = std::to_string(bench->primes) + " ";
		if(data.bench.parameters.duration_i * 60 - data.bench.parameters.elapsed_i > 60 * 59)
			bench->score.value += string_format(_("(%u hours left)"), (data.bench.parameters.duration_i - data.bench.parameters.elapsed_i / 60) / 60);
		else if(data.bench.parameters.duration_i * 60 - data.bench.parameters.elapsed_i >= 60)
			bench->score.value += string_format(_("(%u minutes left)"), data.bench.parameters.duration_i - data.bench.parameters.elapsed_i / 60);
		else
			bench->score.value += string_format(_("(%u seconds left)"), data.bench.parameters.duration_i * 60 - data.bench.parameters.elapsed_i);
	}
	else
	{
		bench->state.value = _("Inactive");
		bench->score.value = std::to_string(bench->primes) + " ";
		if(data.bench.parameters.elapsed_i >= 60 * 60)
			bench->score.value += string_format(_("in %u hours"),   data.bench.parameters.elapsed_i / 60 / 60);
		else if(data.bench.parameters.elapsed_i >= 60)
			bench->score.value += string_format(_("in %u minutes"), data.bench.parameters.elapsed_i / 60);
		else
			bench->score.value += string_format(_("in %u seconds"), data.bench.parameters.elapsed_i);
	}

	return 0;
}

/* Perform a multithreaded benchmark (compute prime numbers) */
void start_benchmarks(Data &data)
{
	int err = 0;
	Data::Bench::PrimeNumbers *bench = data.bench.fast_mode ? static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_fast) : static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_slow);
#ifdef __FreeBSD__
	cpuset_t cpu_set;
#else
	cpu_set_t cpu_set;
#endif /* __FreeBSD__ */

	MSG_VERBOSE(_("Starting benchmark with %u threads"), data.bench.parameters.threads_i);
	data.bench.did_run              = true;
	data.bench.is_running           = true;
	data.bench.is_completed         = false;
	data.bench.parameters.elapsed_i = 0;
	bench->number                   = 2;
	bench->primes                   = 1;

	/* Start one thread per logical CPU */
	for(uint_fast16_t i = 0; i < data.bench.parameters.threads_i; i++)
	{
		CPU_ZERO(&cpu_set);
		CPU_SET(i, &cpu_set);
		std::thread bench_thread(primes_bench, &data);
		MSG_DEBUG("start_benchmarks: started thread #%u", i);
		err += pthread_setaffinity_np(bench_thread.native_handle(), sizeof(cpu_set), &cpu_set);
		data.bench.compute_threads.push_back(std::move(bench_thread));
	}

	std::thread timer_thread(stop_benchmarks, &data);
	timer_thread.detach();

	if(err)
		MSG_ERROR("%s", _("an error occurred while starting benchmark"));
}

/* Set initial values for benchmarks */
void init_benchmarks(Data &data)
{
	data.bench.parameters.set_threads(data.bench.parameters.threads_i);
	data.bench.parameters.set_duration(data.bench.parameters.duration_i);
	data.bench.prime_slow.state.value    = _("Inactive");
	data.bench.prime_fast.state.value    = _("Inactive");
}
