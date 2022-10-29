/****************************************************************************
*    Copyright © 2014-2022 The Tumultuous Unicorn Of Darkness
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
* FILE core.h
*/

#ifndef _CORE_H_
#define _CORE_H_

#include "cpu-x.h"
#if HAS_LIBPCI
# include "pci/pci.h"
#endif /* HAS_LIBPCI */


/* Avoid to re-run a function if an error was occurred in previous call */
static int err_func(int (*func)(Labels *), Labels *data);

#if HAS_LIBCPUID
/* Static elements provided by libcpuid */
static int call_libcpuid_static(Labels *data);
/* Required: HAS_LIBCPUID */

/* Dynamic elements provided by libcpuid */
static int call_libcpuid_dynamic(Labels *data);
/* Required: HAS_LIBCPUID */

/* MSRs raw values provided by libcpuid */
static int call_libcpuid_msr_debug(Labels *data, uint16_t all_cpu_count);
/* Required: HAS_LIBCPUID && DAEMON_UP */

/* MSRs static values provided by libcpuid */
static int call_libcpuid_msr_static(Labels *data);
/* Required: HAS_LIBCPUID && DAEMON_UP */

/* MSRs dynamic values provided by libcpuid */
static int call_libcpuid_msr_dynamic(Labels *data);
/* Required: HAS_LIBCPUID && DAEMON_UP */
#endif /* HAS_LIBCPUID */

/* Fill the Multiplier label with the most appropriate format */
static int cputab_fill_multipliers(Labels *data);
/* Required: none
Both normal and fallback mode provide CPU multipliers, need to be call after */

#if HAS_DMIDECODE
/* Elements provided by dmidecode */
static int call_dmidecode(Labels *data);
/* Required: HAS_DMIDECODE && DAEMON_UP */
#endif /* HAS_DMIDECODE */

#if HAS_BANDWIDTH
/* Compute CPU cache speed */
static int call_bandwidth(Labels *data);
/* Required: HAS_BANDWIDTH */
#endif /* HAS_BANDWIDTH */

/* Calculate total CPU usage */
static int cpu_usage(Labels *data);
/* Required: none */

#if HAS_LIBPCI
/* Find some PCI devices, like chipset and GPU */
static int find_devices(Labels *data);
/* Required: HAS_LIBPCI */

/* Retrieve GPU temperature and clocks */
static int gpu_monitoring(Labels *data);
/* Required: HAS_LIBPCI */
#endif /* HAS_LIBPCI */

/* Satic elements for System tab, OS specific */
static int system_static(Labels *data);
/* Required: none */

#if (HAS_LIBSYSTEM)
/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
static int system_dynamic(Labels *data);
/* Required: HAS_LIBPROC2 || HAS_LIBPROCPS || HAS_LIBSTATGRAB */
#endif /* HAS_LIBSYSTEM */

/* Report score of benchmarks */
static int benchmark_status(Labels *data);
/* Required: none */

/* Retrieve static data if other functions failed */
static int fallback_mode_static(Labels *data);

/* Retrieve dynamic data if other functions failed */
static int fallback_mode_dynamic(Labels *data);


#endif /* _CORE_H_ */
