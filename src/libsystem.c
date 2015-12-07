/****************************************************************************
*    Copyright © 2014-2015 Xorg
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
* FILE libsystem.c
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>
#include <locale.h>
#include <libintl.h>
#include "cpu-x.h"

#if HAS_LIBPROCPS
# include <proc/sysinfo.h>
#endif

#if HAS_LIBSTATGRAB
# include <statgrab.h>
#endif

#ifndef __linux__
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

#ifdef __MACH__
# include <mach/clock.h>
# include <mach/mach.h>
#endif

static int os_specific(Labels *data)
{
#ifdef __linux__
	char *buff = NULL;

	xopen_to_str("grep PRETTY_NAME= /etc/os-release | awk -F '\"|\"' '{print $2}'", &buff, 'p'); /* Label Distribution */
	iasprintf(&data->tabsys[VALUE][DISTRIBUTION], buff);
#else
	char buff[MAXSTR];
	size_t len = sizeof(buff);

	sysctlbyname("kern.osrelease", &buff, &len, NULL, 0); /* Label Kernel */
	iasprintf(&data->tabsys[VALUE][KERNEL], buff);

	sysctlbyname("kern.ostype", &buff, &len, NULL, 0); /* Label Distribution */
	iasprintf(&data->tabsys[VALUE][DISTRIBUTION], buff);
#endif /* __linux__ */
	return 0;
}

static int library_specific(Labels *data, time_t *time)
{
	long int memtot = 0;
# if HAS_LIBPROCPS
	const int div = 1000;

	MSG_VERBOSE(_("Filling labels (libprocps step)"));
	*time = (time_t) uptime(NULL, NULL); /* Label Uptime */

	meminfo(); /* Memory labels */
	memtot = kb_main_total / div;

	asprintf(&data->tabsys[VALUE][USED],    "%5ld MB / %5ld MB", kb_main_used    / div, memtot);
	asprintf(&data->tabsys[VALUE][BUFFERS], "%5ld MB / %5ld MB", kb_main_buffers / div, memtot);
	asprintf(&data->tabsys[VALUE][CACHED],  "%5ld MB / %5ld MB", kb_main_cached  / div, memtot);
	asprintf(&data->tabsys[VALUE][FREE],    "%5ld MB / %5ld MB", kb_main_free    / div, memtot);
	asprintf(&data->tabsys[VALUE][SWAP],    "%5ld MB / %5ld MB", kb_swap_used    / div, kb_swap_total / div);
# endif /* HAS_LIBPROCPS */

# if HAS_LIBSTATGRAB
	static int called = 0;
	const int div = 1000000;
	sg_mem_stats *mem; /* Memory labels */
	sg_swap_stats *swap;
	sg_host_info *info;

	MSG_VERBOSE(_("Filling labels (libstatgrab step)"));
	if(!called)
	{
		sg_init(0);
		called = 1;
	}

	mem  = sg_get_mem_stats(NULL);
	swap = sg_get_swap_stats(NULL);
	info = sg_get_host_info(NULL);
	*time = info->uptime;

	memtot = mem->total / div;
	asprintf(&data->tabsys[VALUE][USED],    "%5llu MB / %5ld MB",  mem->used   / div, memtot);
	asprintf(&data->tabsys[VALUE][BUFFERS], "  %3d MB / %5ld MB",  0                , memtot);
	asprintf(&data->tabsys[VALUE][CACHED],  "%5llu MB / %5ld MB",  mem->cache  / div, memtot);
	asprintf(&data->tabsys[VALUE][FREE],    "%5llu MB / %5ld MB",  mem->free   / div, memtot);
	asprintf(&data->tabsys[VALUE][SWAP],    "%5llu MB / %5llu MB", swap->used  / div, swap->total / div);
# endif /* HAS_LIBSTATGRAB */
	return 0;
}

void system_macos(Labels *data, long int *suptime)
{
#ifdef __MACH__
	clock_serv_t cclock;
	mach_timespec_t mts;

	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock); /* Label Uptime */
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	*suptime = mts.tv_sec;
#endif /* __MACH__ */

#ifdef __APPLE__
	char *tmp, buff[MAXSTR];
	FILE *cc;

	asprintf(&tmp, data->tabsys[VALUE][KERNEL]);
	asprintf(&data->tabsys[VALUE][KERNEL], "%s %s", data->tabsys[VALUE][DISTRIBUTION], tmp); /* Label Kernel */

	cc = popen("echo $(sw_vers -productName ; sw_vers -productVersion)", "r"); /* Label Distribution */
	if(cc != NULL)
	{
		fgets(buff, MAXSTR, cc);
		asprintf(&data->tabsys[VALUE][DISTRIBUTION], buff);
		pclose(cc);
	}
#endif /* __APPLE__ */
}

/* Get system informations */
int tabsystem(Labels *data)
{
	int err;
	char *buff;
	time_t uptime;
	struct utsname name;
	struct tm *tm;

	MSG_VERBOSE(_("Filling labels (libsystem step)"));
	if((err = uname(&name)))
		MSG_ERROR_ERRNO("");
	err += xopen_to_str("cc --version", &buff, 'p');

	iasprintf(&data->tabsys[VALUE][KERNEL],   "%s %s", name.sysname, name.release); /* Label Kernel */
	iasprintf(&data->tabsys[VALUE][HOSTNAME], "%s", name.nodename); /* Label Hostname */
	iasprintf(&data->tabsys[VALUE][COMPILER], buff); /* Label Compiler */

	os_specific(data);
	library_specific(data, &uptime);

	tm = gmtime(&uptime);
	iasprintf(&data->tabsys[VALUE][UPTIME], _("%i days, %i hours, %i minutes, %i seconds"),
	          tm->tm_yday, tm->tm_hour, tm->tm_min, tm->tm_sec); /* Label Uptime */

	return err;
}
