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
* FILE core.h
*/

#ifndef _CORE_H_
#define _CORE_H_

#include "cpu-x.h"
#if HAS_LIBPCI
# include "pci/pci.h"
#endif

#define RUN_IF(cond, fnc) cond ? fnc : 0;


/* Static elements provided by libcpuid */
static int call_libcpuid_static(Labels *data);
/* Required: HAS_LIBCPUID */

/* Dynamic elements provided by libcpuid */
static int call_libcpuid_dynamic(Labels *data);
/* Required: HAS_LIBCPUID && root privileges */

/* Elements provided by dmidecode (need root privileges) */
static int call_dmidecode(Labels *data);
/* Required: HAS_DMIDECODE && root privileges */

/* Compute CPU cache speed */
static int call_bandwidth(Labels *data);
/* Required: HAS_BANDWIDTH */

/* Calculate CPU usage (total if core < 0, else per given core) */
static int cpu_usage(Labels *data, int core);
/* Required: none */

/* Find some PCI devices, like chipset and GPU */
static int find_devices(Labels *data);
/* Required: HAS_LIBPCI */

/* Retrieve GPU temperature */
static int gpu_temperature(Labels *data);
/* Required: none */

/* Satic elements for System tab, OS specific */
static int system_static(Labels *data);
/* Required: none */

/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
static int system_dynamic(Labels *data);
/* Required: HAS_LIBPROCPS || HAS_LIBSTATGRAB */

/* Report score of benchmarks */
static int benchmark_status(Labels *data);
/* Required: none */

/* Retrieve static data if other functions failed */
static int fallback_mode_static(Labels *data);

/* Retrieve dynamic data if other functions failed */
static int fallback_mode_dynamic(Labels *data);


#endif /* _CORE_H_ */
