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
#include <locale.h>
#include <libintl.h>
#include "core.h"

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

#if defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
# include <sys/timespec.h>
# include <time.h>
#endif

#ifdef __MACH__
# include <mach/clock.h>
# include <mach/mach.h>
#endif


void system_linux(Labels *data, long int *suptime)
{
#ifdef __linux__
	static int called = 0;
	char *filestr = NULL, *distro = NULL;
	FILE *osrel = NULL;

	osrel = fopen("/etc/os-release", "r"); /* Label Distribution */
	if(osrel == NULL && !called)
		MSGPERR(_("failed to open file '/etc/os-release'"));
	else if(!called)
	{
		filestr = malloc(500 * (sizeof(char)));
		if(filestr == NULL)
			MSGPERR(_("malloc failed"));
		else
		{
			fread(filestr, sizeof(char), 500, osrel);
			distro = strstr(filestr, "PRETTY_NAME=");
			if(distro == NULL)
				asprintf(&data->tabsys[VALUE][DISTRIBUTION], _("Unknown distro"));
			else
				asprintf(&data->tabsys[VALUE][DISTRIBUTION], "%s", strtok(strchr(distro, '"') + 1, "\""));
			fclose(osrel);
			free(filestr);
		}
	}
	called = 1;

# if HAS_LIBPROCPS
	long int memtot = 0;
	const int div = 1000;

	MSGVERB(_("Filling labels (libprocps step)"));
	*suptime = uptime(NULL, NULL); /* Label Uptime */

	meminfo(); /* Memory labels */
	memtot = kb_main_total / div;

	asprintf(&data->tabsys[VALUE][USED], "%5ld MB / %5ld MB", kb_main_used / div, memtot);
	asprintf(&data->tabsys[VALUE][BUFFERS], "%5ld MB / %5ld MB", kb_main_buffers / div, memtot);
	asprintf(&data->tabsys[VALUE][CACHED], "%5ld MB / %5ld MB", kb_main_cached / div, memtot);
	asprintf(&data->tabsys[VALUE][FREE], "%5ld MB / %5ld MB", kb_main_free / div, memtot);
	asprintf(&data->tabsys[VALUE][SWAP], "%5ld MB / %5ld MB", kb_swap_used / div, kb_swap_total / div);
# endif /* HAS_LIBPROCPS */

#endif /* __linux__ */
}

void system_bsd(Labels *data, long int *suptime)
{
#if defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	struct timespec tsp;

	clock_gettime(CLOCK_MONOTONIC, &tsp); /* Label Uptime */
	*suptime = tsp.tv_sec;
#endif /* BSD */
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

	tmp = strdupnullok(data->tabsys[VALUE][KERNEL]);
	asprintf(&data->tabsys[VALUE][KERNEL], "%s %s", data->tabsys[VALUE][DISTRIBUTION], tmp); /* Label Kernel */

	cc = popen("echo $(sw_vers -productName ; sw_vers -productVersion)", "r"); /* Label Distribution */
	if(cc != NULL)
	{
		fgets(buff, MAXSTR, cc);
		data->tabsys[VALUE][DISTRIBUTION] = strdupnullok(buff);
		pclose(cc);
	}
#endif /* __APPLE__ */
}

void system_nonlinux(Labels *data, long int *suptime)
{
#ifndef __linux__
	char buff[MAXSTR];
	size_t len = sizeof(buff);

	sysctlbyname("kern.osrelease", &buff, &len, NULL, 0); /* Label Kernel */
	data->tabsys[VALUE][KERNEL] = strdupnullok(buff);

	sysctlbyname("kern.ostype", &buff, &len, NULL, 0); /* Label Distribution */
	data->tabsys[VALUE][DISTRIBUTION] = strdupnullok(buff);

# if HAS_LIBSTATGRAB
	long int memtot = 0;
	static int called = 0;
	const int div = 1000000;
	sg_mem_stats *mem; /* Memory labels */
	sg_swap_stats *swap;

	MSGVERB(_("Filling labels (libstatgrab step)"));
	if(!called)
	{
		sg_init(0);
		called = 1;
	}

	mem  = sg_get_mem_stats(NULL);
	swap = sg_get_swap_stats(NULL);

	memtot = mem->total / div;
	asprintf(&data->tabsys[VALUE][USED], "%5llu MB / %5ld MB", mem->used / div, memtot);
	asprintf(&data->tabsys[VALUE][BUFFERS], "%5u MB / %5ld MB", 0, memtot);
	asprintf(&data->tabsys[VALUE][CACHED], "%5llu MB / %5ld MB", mem->cache / div, memtot);
	asprintf(&data->tabsys[VALUE][FREE], "%5llu MB / %5ld MB", mem->free / div, memtot);
	asprintf(&data->tabsys[VALUE][SWAP], "%5llu MB / %5llu MB", swap->used / div, swap->total / div);
# endif /* HAS_LIBSTATGRAB */

	system_bsd(data, suptime);
	system_macos(data, suptime);
#endif /* !__linux__ */
}

/* Get system informations */
void tabsystem(Labels *data)
{
	long int duptime, huptime, muptime, suptime = 0;
	char buff[MAXSTR];
	FILE *cc;
	struct utsname name;

	MSGVERB(_("Filling labels (libsystem step)"));
	uname(&name);
	asprintf(&data->tabsys[VALUE][KERNEL], "%s %s", name.sysname, name.release); /* Label Kernel */
	asprintf(&data->tabsys[VALUE][HOSTNAME], "%s", name.nodename); /* Label Hostname */

	cc = popen("cc --version", "r"); /* Label Compiler */
	if(cc != NULL)
	{
		fgets(buff, MAXSTR, cc);
		data->tabsys[VALUE][COMPILER] = strdupnullok(buff);
		data->tabsys[VALUE][COMPILER][ strlen(data->tabsys[VALUE][COMPILER]) - 1 ] = '\0';
		pclose(cc);
	}

	system_linux(data, &suptime);
	system_nonlinux(data, &suptime);

	if(suptime > 0)
	{
		duptime = suptime / (24 * 60 * 60); suptime -= duptime * (24 * 60 * 60); /* Label Uptime */
		huptime = suptime / (60 * 60); suptime -= huptime * (60 * 60);
		muptime = suptime / 60; suptime -= muptime * 60;
		asprintf(&data->tabsys[VALUE][UPTIME], _("%ld days, %2ld hours, %2ld minutes, %2ld seconds"), duptime, huptime, muptime, suptime);
	}
}
