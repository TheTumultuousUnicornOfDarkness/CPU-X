/****************************************************************************
*    Copyright © 2014 Xorg
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
* cpu-x.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "cpu-x.h"
#include "includes.h"


int main(int argc, char *argv[]) {
	char option;
	setenv("LC_ALL", "C", 1);
	Libcpuid data;
	Dmi extrainfo;
	Internal global;

	/* Populate structures */
	option = menu(argc, argv);
	empty_labels(&data, &extrainfo, &global);
	cpufreq(&global, extrainfo.bus);
	bogomips(global.mips);

	if(HAS_LIBCPUID)
		if(libcpuid(&data))
			MSGERR("libcpuid failed.");
		else
			instructions(&data, global.instr);

	if(HAS_LIBDMI && !getuid()) {
		if(libdmidecode(&extrainfo))
			MSGERR("libdmidecode failed");
	}

	/* Start GUI */
	if(HAS_GTK && option == 'G') /* Start with GTK3 */
		start_gui_gtk(&argc, &argv, &data, &extrainfo, &global);
	else if(HAS_NCURSES && option == 'N') /* Start with NCurses */
		start_gui_ncurses(&data, &extrainfo, &global);

	/* Error when compiled without GUI */
	if(!HAS_GTK && !HAS_NCURSES) {
		fprintf(stderr, "Hey! You need to compile with GTK3+ support and/or NCurses!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/* Set empty labels */
void empty_labels(Libcpuid *data, Dmi *extrainfo, Internal *global) {
	data->vendor[0] = '\0';
	data->name[0] = '\0';
	data->arch[0] = '\0';
	data->spec[0] = '\0';
	data->fam[0] = '\0';
	data->mod[0] = '\0';
	data->step[0] = '\0';
	data->extfam[0] = '\0';
	data->extmod[0] = '\0';
	data->instr[0] = '\0';
	data->l1d[0] = '\0';
	data->l1i[0] = '\0';
	data->l2[0] = '\0';
	data->l3[0] = '\0';
	data->l1dw[0] = '\0';
	data->l1iw[0] = '\0';
	data->l2w[0] = '\0';
	data->l3w[0] = '\0';
	data->soc[0] = '\0';
	data->core[0] = '\0';
	data->thrd[0] = '\0';

	extrainfo->vendor[0] = '\0';
	extrainfo->socket[0] = '\0';
	extrainfo->bus[0] = '\0';
	extrainfo->manu[0] = '\0';
	extrainfo->model[0] = '\0';
	extrainfo->rev[0] = '\0';
	extrainfo->brand[0] = '\0';
	extrainfo->version[0] = '\0';
	extrainfo->date[0] = '\0';
	extrainfo->rom[0] = '\0';

	global->clock[0] = '\0';
	global->mults[0] = '\0';
	global->mips[0] = '\0';
	global->instr[0] = '\0';
}

#if HAS_LIBCPUID
/* Elements provided by libcpuid library */
int libcpuid(Libcpuid *data) {
	int err = 0;
	struct cpu_raw_data_t raw;
	struct cpu_id_t datanr;

	err += cpuid_get_raw_data(&raw);
	err += cpu_identify(&raw, &datanr);

	sprintf(data->vendor,	"%s",		datanr.vendor_str);
	sprintf(data->name,	"%s",		datanr.cpu_codename);
	sprintf(data->spec,	"%s",		datanr.brand_str);
	sprintf(data->fam,	"%d",		datanr.family);
	sprintf(data->mod,	"%d",		datanr.model);
	sprintf(data->step,	"%d",		datanr.stepping);
	sprintf(data->extfam,	"%d",		datanr.ext_family);
	sprintf(data->extmod,	"%d",		datanr.ext_model);
	sprintf(data->instr,	"%s",		datanr.flags);
	sprintf(data->l1d,	"%d x %d KB",	datanr.num_cores, datanr.l1_data_cache);
	sprintf(data->l1i,	"%d x %d KB",	datanr.num_cores, datanr.l1_instruction_cache);
	sprintf(data->l2,	"%d x %d KB",	datanr.num_cores, datanr.l2_cache);
	sprintf(data->l3,	"%d KB",	datanr.l3_cache);
	sprintf(data->l1dw,	"%d-way",	datanr.l1_assoc);
	sprintf(data->l1iw,	"%d-way",	datanr.l1_assoc);
	sprintf(data->l2w,	"%d-way",	datanr.l2_assoc);
	sprintf(data->l3w,	"%d-way",	datanr.l3_assoc);
	sprintf(data->soc,	"%d",		datanr.total_logical_cpus / datanr.num_cores);
	sprintf(data->core,	"%d",		datanr.num_cores);
	sprintf(data->thrd,	"%d",		datanr.num_logical_cpus);

	return err;
}
#endif /* HAS_LIBCPUID */

#if HAS_LIBDMI
/* Elements provided by libdmi library (need root privileges) */
int libdmidecode(Dmi *data) {
	int err = 0;
	char datanr[L][C] = { { '\0' } };

	err += libdmi(datanr);

	sprintf(data->vendor,	"%s", datanr[PROCESSOR_MANUFACTURER]);
	sprintf(data->socket,	"%s", datanr[PROCESSOR_SOCKET]);
	sprintf(data->bus,	"%s", datanr[PROCESSOR_CLOCK]);
	sprintf(data->manu,	"%s", datanr[BASEBOARD_MANUFACTURER]);
	sprintf(data->model,	"%s", datanr[BASEBOARD_PRODUCT_NAME]);
	sprintf(data->rev,	"%s", datanr[BASEBOARD_VERSION]);
	sprintf(data->brand,	"%s", datanr[BIOS_VENDOR]);
	sprintf(data->version,	"%s", datanr[BIOS_VERSION]);
	sprintf(data->date,	"%s", datanr[BIOS_RELEASE_DATE]);
	sprintf(data->rom,	"%s", datanr[BIOS_ROM_SIZE]);

	return err;
}
#endif /* HAS_LIBDMI */

/* Get CPU frequencies (current - min - max) */
void cpufreq(Internal *global, char *busfreq) {
	char multmin[P], multmax[P];
	FILE *fmin = NULL, *fmax = NULL;

	if(HAS_LIBCPUID)
		sprintf(global->clock, "%d MHz", cpu_clock());

	/* Can't get base clock without root rights, skip multiplicators calculation */
	if(!getuid()) {
		fmin = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq", "r");
		fmax = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
		if(fmin == NULL)
			MSGERR("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq'.");
		else {
			fgets(multmin, 9, fmin);
			fclose(fmin);
		}

		if(fmax == NULL)
			MSGERR("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq'.");
		else {
			fgets(multmax, 9, fmax);
			fclose(fmax);
		}

		mult(busfreq, global->clock, multmin, multmax, global->mults);
	}
}

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char *c) {
	char read[20];
	char *mips = NULL;
	int size = 20;
	FILE *cpuinfo = NULL;

	cpuinfo = fopen("/proc/cpuinfo", "r");
	if(cpuinfo == NULL) {
		MSGERR("failed to open '/proc/cpuinfo'.");
		return;
	}

	while(fgets(read, size, cpuinfo) != NULL) {
		mips = strstr(read, "bogomips");
		if(mips != NULL)
			break;
	}

	sprintf(c, "%s", strrchr(mips, ' '));
	c[strlen(c) - 1] = '\0';
}

/* Determine CPU multiplicator from base clock */
void mult(char *busfreq, char *cpufreq, char *multmin, char *multmax, char multsynt[Q]) {
	int i, fcpu, fbus, min, max;
	char ncpu[P] = "", nbus[P] = "";

	for(i = 0; isdigit(cpufreq[i]); i++)
		ncpu[i] = cpufreq[i];
	fcpu = atoi(ncpu);

	for(i = 0; isdigit(busfreq[i]); i++)
		nbus[i] = busfreq[i];
	nbus[i] = '\0';
	fbus = atoi(nbus);
	min = atoi(multmin);
	max = atoi(multmax);

	if(fbus > 0) {
		if(min >= 10000)
			min /= (fbus * 1000);
		if(max >= 10000 && fbus > 0)
			max /= (fbus * 1000);
		sprintf(multsynt, "x %i (%i-%i)", (fcpu / fbus), min, max);
	}
}

#if HAS_LIBCPUID
/* Show some instructions supported by CPU */
void instructions(Libcpuid *data, char instr[S]) {
	struct cpu_raw_data_t raw;
	struct cpu_id_t id;

	if (cpuid_get_raw_data(&raw) == 0 && cpu_identify(&raw, &id) == 0) {
		if(id.flags[CPU_FEATURE_MMX]) {
			strcpy(instr, "MMX");
			if(id.flags[CPU_FEATURE_MMXEXT])
				strcat(instr, "(+)");
		}
		if(id.flags[CPU_FEATURE_3DNOW]) {
			strcat(instr, ", 3DNOW!");
			if(id.flags[CPU_FEATURE_3DNOWEXT])
				strcat(instr, "(+)");
		}
		if(id.flags[CPU_FEATURE_SSE])
			strcat(instr, ", SSE (1");
		if(id.flags[CPU_FEATURE_SSE2])
			strcat(instr, ", 2");
		if(id.flags[CPU_FEATURE_SSSE3])
			strcat(instr, ", 3S");
		if(id.flags[CPU_FEATURE_SSE4_1])
			strcat(instr, ", 4.1");
		if(id.flags[CPU_FEATURE_SSE4_2])
			strcat(instr, ", 4.2");
		if(id.flags[CPU_FEATURE_SSE4A])
			strcat(instr, ", 4A");
		if(id.flags[CPU_FEATURE_SSE])
			strcat(instr, ")");
		if(id.flags[CPU_FEATURE_AES])
			strcat(instr, ", AES");
		if(id.flags[CPU_FEATURE_AVX])
			strcat(instr, ", AVX");
		if(id.flags[CPU_FEATURE_VMX])
			strcat(instr, ", VT-x");
		if(id.flags[CPU_FEATURE_SVM])
			strcat(instr, ", AMD-V");
		if(id.flags[CPU_FEATURE_LM])
			strcpy(data->arch, "x86_64 (64-bit)");
		else
			strcpy(data->arch, "ix86 (32-bit)");
	}
	else
	MSGERR("failed to call 'libcpuid'.");
}
#endif /* HAS_LIBCPUID */

/* Search file location */
size_t get_path(char* buffer, char *file) {
	/* Taken from http://www.advancedlinuxprogramming.com/listings/chapter-7/get-exe-path.c
	See this file for more informations */
	char *path_end;
	size_t len = PATH_MAX;

	if(readlink ("/proc/self/exe", buffer, len) <= 0)
		return -1;

	path_end = strrchr(buffer, '/');
	if(path_end == NULL)
		return -1;

	path_end++;
	*path_end = '\0';
	sprintf(buffer, "%s../share/cpu-x/%s", buffer, file);

	return (size_t) (path_end - buffer);
}
