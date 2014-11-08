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
#include <locale.h>
#include <libintl.h>
#include "cpu-x.h"
#include "includes.h"


int main(int argc, char *argv[]) {
	char option;
	option = menu(argc, argv);

#ifdef NEXT
	Labels data;
	setlocale(LC_ALL, "");
	bindtextdomain("cpux", get_path("locale"));
	free(get_path("locale"));
	textdomain("cpux");

	labels_setempty(&data);
	labels_setname(&data);
	bogomips(data.tabcpu[VALUE][BOGOMIPS]);

	if(HAS_LIBCPUID) {
		if(libcpuid(&data))
			MSGERR("libcpuid failed.");
		else {
			cpuvendor(data.tabcpu[VALUE][VENDOR]);
			instructions(data.tabcpu[VALUE][ARCHITECTURE], data.tabcpu[VALUE][INSTRUCTIONS]);
		}
	}

	if(HAS_LIBDMI && !getuid()) {
		if(libdmidecode(&data))
			MSGERR("libdmidecode failed");
	}
	cpufreq(data.tabcpu[VALUE][BUSSPEED], data.tabcpu[VALUE][CORESPEED], data.tabcpu[VALUE][MULTIPLIER]);
	//dump_data(&data);

	/* Start GUI */
	if(HAS_GTK && option == 'G') /* Start with GTK3 */
		start_gui_gtk(&argc, &argv, &data);
	else if(HAS_NCURSES && option == 'N') /* Start with NCurses */
		start_gui_ncurses(&data);
#else
	setenv("LC_ALL", "", 1);
	Libcpuid data;
	Dmi extrainfo;
	Internal global;

	/* Populate structures */
	empty_labels(&data, &extrainfo, &global);
	cpufreq(&global, extrainfo.bus);
	bogomips(global.mips);

	if(HAS_LIBCPUID) {
		if(libcpuid(&data))
			MSGERR("libcpuid failed.");
		else {
			cpuvendor(data.vendor, global.prettyvendor);
			instructions(&data, global.instr);
		}
	}

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
#endif
	return EXIT_SUCCESS;
}

/* Set empty labels */
#ifdef NEXT
void labels_setempty(Labels *data)
{
	int i;

	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		memset(data->tabcpu[NAME][i], 0, MAXSTR);
		memset(data->tabcpu[VALUE][i], 0, MAXSTR);
	}

	/* Tab Mainboard */
	for(i = MANUFACTURER; i < LASTMB; i++)
	{
		memset(data->tabmb[NAME][i], 0, MAXSTR);
		memset(data->tabmb[VALUE][i], 0, MAXSTR);
	}
}
#else
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

	extrainfo->socket[0] = '\0';
	extrainfo->bus[0] = '\0';
	extrainfo->manu[0] = '\0';
	extrainfo->model[0] = '\0';
	extrainfo->rev[0] = '\0';
	extrainfo->brand[0] = '\0';
	extrainfo->version[0] = '\0';
	extrainfo->date[0] = '\0';
	extrainfo->rom[0] = '\0';

	global->prettyvendor[0] = '\0';
	global->clock[0] = '\0';
	global->mults[0] = '\0';
	global->mips[0] = '\0';
	global->instr[0] = '\0';
}
#endif

#ifdef NEXT
/* Set label name */
void labels_setname(Labels *data)
{
	/* Tab CPU */
	snprintf(data->tabcpu[NAME][VENDOR],		MAXSTR, _("Vendor"));
	snprintf(data->tabcpu[NAME][CODENAME],		MAXSTR, _("Code Name"));
	snprintf(data->tabcpu[NAME][PACKAGE],		MAXSTR, _("Package"));
	snprintf(data->tabcpu[NAME][ARCHITECTURE],	MAXSTR, _("Architecture"));
	snprintf(data->tabcpu[NAME][SPECIFICATION],	MAXSTR, _("Specification"));
	snprintf(data->tabcpu[NAME][FAMILY],		MAXSTR, _("Family"));
	snprintf(data->tabcpu[NAME][EXTFAMILY],		MAXSTR, _("Ext. Family"));
	snprintf(data->tabcpu[NAME][MODEL],		MAXSTR, _("Model"));
	snprintf(data->tabcpu[NAME][EXTMODEL],		MAXSTR, _("Ext. Model"));
	snprintf(data->tabcpu[NAME][STEPPING],		MAXSTR, _("Stepping"));
	snprintf(data->tabcpu[NAME][INSTRUCTIONS],	MAXSTR, _("Instructions"));

	snprintf(data->tabcpu[NAME][CORESPEED],		MAXSTR, _("Core Speed"));
	snprintf(data->tabcpu[NAME][MULTIPLIER],	MAXSTR, _("Multiplier"));
	snprintf(data->tabcpu[NAME][BUSSPEED],		MAXSTR, _("Bus Speed"));
	snprintf(data->tabcpu[NAME][BOGOMIPS],		MAXSTR, _("BogoMIPS"));

	snprintf(data->tabcpu[NAME][LEVEL1D],		MAXSTR, _("L1 Data"));
	snprintf(data->tabcpu[NAME][LEVEL1I],		MAXSTR, _("L1 Inst."));
	snprintf(data->tabcpu[NAME][LEVEL2],		MAXSTR, _("Level 2"));
	snprintf(data->tabcpu[NAME][LEVEL3],		MAXSTR, _("Level 3"));

	snprintf(data->tabcpu[NAME][SOCKETS],		MAXSTR, _("Sockets(s)"));
	snprintf(data->tabcpu[NAME][CORES],		MAXSTR, _("Core(s)"));
	snprintf(data->tabcpu[NAME][THREADS],		MAXSTR, _("Thread(s)"));

	/* Tab Mainboard */
	snprintf(data->tabmb[NAME][MANUFACTURER],	MAXSTR, _("Manufacturer"));
	snprintf(data->tabmb[NAME][MBMODEL],		MAXSTR, _("Model"));
	snprintf(data->tabmb[NAME][REVISION],		MAXSTR, _("Revision"));

	snprintf(data->tabmb[NAME][BRAND],		MAXSTR, _("Brand"));
	snprintf(data->tabmb[NAME][VERSION],		MAXSTR, _("Version"));
	snprintf(data->tabmb[NAME][DATE],		MAXSTR, _("Date"));
	snprintf(data->tabmb[NAME][ROMSIZE],		MAXSTR, _("ROM Size"));
}
#endif

#if HAS_LIBCPUID
/* Elements provided by libcpuid library */
# ifdef NEXT
int libcpuid(Labels *data)
{
	int err = 0;
	char tmp[MAXSTR];
	struct cpu_raw_data_t raw;
	struct cpu_id_t datanr;

	err += cpuid_get_raw_data(&raw);
	err += cpu_identify(&raw, &datanr);

	/* Tab CPU */
	snprintf(data->tabcpu[VALUE][VENDOR],		MAXSTR, "%s", datanr.vendor_str);
	snprintf(data->tabcpu[VALUE][CODENAME],		MAXSTR, "%s", datanr.cpu_codename);
	snprintf(data->tabcpu[VALUE][SPECIFICATION],	MAXSTR, "%s", datanr.brand_str);
	snprintf(data->tabcpu[VALUE][FAMILY],		MAXSTR, "%d", datanr.family);
	snprintf(data->tabcpu[VALUE][EXTFAMILY],	MAXSTR, "%d", datanr.ext_family);
	snprintf(data->tabcpu[VALUE][MODEL],		MAXSTR, "%d", datanr.model);
	snprintf(data->tabcpu[VALUE][EXTMODEL],		MAXSTR, "%d", datanr.ext_model);
	snprintf(data->tabcpu[VALUE][STEPPING],		MAXSTR, "%d", datanr.stepping);

	if(datanr.l1_data_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL1D],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l1_data_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, " , %2d-way", datanr.l1_assoc);
			strncat(data->tabcpu[VALUE][LEVEL1D], tmp, MAXSTR);
		}
	}

	if(datanr.l1_instruction_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL1I],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l1_instruction_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, " , %2d-way", datanr.l1_assoc);
			strncat(data->tabcpu[VALUE][LEVEL1I], tmp, MAXSTR);
		}
	}

	if(datanr.l2_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL2],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l2_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, " , %2d-way", datanr.l2_assoc);
			strncat(data->tabcpu[VALUE][LEVEL2], tmp, MAXSTR);
		}
	}

	if(datanr.l3_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL3],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l3_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, " , %2d-way", datanr.l3_assoc);
			strncat(data->tabcpu[VALUE][LEVEL3], tmp, MAXSTR);
		}
	}

	if(datanr.num_cores > 0) /* Avoid divide by 0 */
		snprintf(data->tabcpu[VALUE][SOCKETS],	MAXSTR, "%d", datanr.total_logical_cpus / datanr.num_cores);

	snprintf(data->tabcpu[VALUE][CORES],		MAXSTR, "%d", datanr.num_cores);
	snprintf(data->tabcpu[VALUE][THREADS],		MAXSTR, "%d", datanr.num_logical_cpus);

	return err;
}
# else
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
	if(datanr.l1_data_cache > 0)
		sprintf(data->l1d,	"%d x %d KB",	datanr.num_cores, datanr.l1_data_cache);
	if(datanr.l1_instruction_cache > 0)
		sprintf(data->l1i,	"%d x %d KB",	datanr.num_cores, datanr.l1_instruction_cache);
	if(datanr.l2_cache > 0)
		sprintf(data->l2,	"%d x %d KB",	datanr.num_cores, datanr.l2_cache);
	if(datanr.l3_cache > 0)
		sprintf(data->l3,	"%d KB",	datanr.l3_cache);
	if(datanr.l1_assoc > 0) {
		sprintf(data->l1dw,	"%d-way",	datanr.l1_assoc);
		sprintf(data->l1iw,	"%d-way",	datanr.l1_assoc);
	}
	if(datanr.l2_assoc > 0)
		sprintf(data->l2w,	"%d-way",	datanr.l2_assoc);
	if(datanr.l3_assoc > 0)
		sprintf(data->l3w,	"%d-way",	datanr.l3_assoc);
	if(datanr.num_cores > 0) /* Avoid divide by 0 */
		sprintf(data->soc,	"%d",		datanr.total_logical_cpus / datanr.num_cores);
	sprintf(data->core,	"%d",		datanr.num_cores);
	sprintf(data->thrd,	"%d",		datanr.num_logical_cpus);

	return err;
}
# endif
#endif /* HAS_LIBCPUID */

#if HAS_LIBDMI
/* Elements provided by libdmi library (need root privileges) */
# ifdef NEXT
int libdmidecode(Labels *data)
{
	int err = 0;
	char datanr[L][C] = { { '\0' } };

	err += libdmi(datanr);

	/* Tab CPU */
	snprintf(data->tabcpu[VALUE][PACKAGE],		MAXSTR, "%s", datanr[PROCESSOR_SOCKET]);
	snprintf(data->tabcpu[VALUE][BUSSPEED],		MAXSTR, "%s", datanr[PROCESSOR_CLOCK]);

	/* Tab Mainboard */
	snprintf(data->tabmb[VALUE][MANUFACTURER],	MAXSTR, "%s", datanr[BASEBOARD_MANUFACTURER]);
	snprintf(data->tabmb[VALUE][MBMODEL],		MAXSTR, "%s", datanr[BASEBOARD_PRODUCT_NAME]);
	snprintf(data->tabmb[VALUE][REVISION],		MAXSTR, "%s", datanr[BASEBOARD_VERSION]);

	snprintf(data->tabmb[VALUE][BRAND],		MAXSTR, "%s", datanr[BIOS_VENDOR]);
	snprintf(data->tabmb[VALUE][VERSION],		MAXSTR, "%s", datanr[BIOS_VERSION]);
	snprintf(data->tabmb[VALUE][DATE],		MAXSTR, "%s", datanr[BIOS_RELEASE_DATE]);
	snprintf(data->tabmb[VALUE][ROMSIZE],		MAXSTR, "%s", datanr[BIOS_ROM_SIZE]);

	return err;
}
# else
int libdmidecode(Dmi *data) {
	int err = 0;
	char datanr[L][C] = { { '\0' } };

	err += libdmi(datanr);

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
# endif
#endif /* HAS_LIBDMI */

#if HAS_LIBCPUID
/* Pretty label CPU Vendor */
# ifdef NEXT
void cpuvendor(char *vendor)
{
	/* This use Libcpuid. See here: https://github.com/anrieff/libcpuid/blob/master/libcpuid/cpuid_main.c#L233 */

	if(!strcmp(vendor, "GenuineIntel"))	 /* Intel */
		strcpy(vendor, "Intel");
	else if(!strcmp(vendor, "AuthenticAMD")) /* AMD */
		strcpy(vendor, "AMD");
	else if(!strcmp(vendor, "CyrixInstead")) /* Cyrix */
		strcpy(vendor, "Cyrix");
	else if(!strcmp(vendor, "NexGenDriven")) /* NexGen */
		strcpy(vendor, "NexGen");
	else if(!strcmp(vendor, "GenuineTMx86")) /* Transmeta */
		strcpy(vendor, "Transmeta");
	else if(!strcmp(vendor, "UMC UMC UMC ")) /* UMC */
		strcpy(vendor, "UMC");
	else if(!strcmp(vendor, "CentaurHauls")) /* Centaur */
		strcpy(vendor, "Centaur");
	else if(!strcmp(vendor, "RiseRiseRise")) /* Rise */
		strcpy(vendor, "Rise");
	else if(!strcmp(vendor, "SiS SiS SiS ")) /* SiS */
		strcpy(vendor, "SiS");
	else if(!strcmp(vendor, "Geode by NSC")) /* National Semiconductor */
		strcpy(vendor, "National Semiconductor");
	else
		strcpy(vendor, "Unknown");
}
# else
void cpuvendor(char *vendor, char *prettyvendor) {
	/* This use Libcpuid. See here: https://github.com/anrieff/libcpuid/blob/master/libcpuid/cpuid_main.c#L233 */

	if(!strcmp(vendor, "GenuineIntel"))	 /* Intel */
		strcpy(prettyvendor, "Intel");
	else if(!strcmp(vendor, "AuthenticAMD")) /* AMD */
		strcpy(prettyvendor, "AMD");
	else if(!strcmp(vendor, "CyrixInstead")) /* Cyrix */
		strcpy(prettyvendor, "Cyrix");
	else if(!strcmp(vendor, "NexGenDriven")) /* NexGen */
		strcpy(prettyvendor, "NexGen");
	else if(!strcmp(vendor, "GenuineTMx86")) /* Transmeta */
		strcpy(prettyvendor, "Transmeta");
	else if(!strcmp(vendor, "UMC UMC UMC ")) /* UMC */
		strcpy(prettyvendor, "UMC");
	else if(!strcmp(vendor, "CentaurHauls")) /* Centaur */
		strcpy(prettyvendor, "Centaur");
	else if(!strcmp(vendor, "RiseRiseRise")) /* Rise */
		strcpy(prettyvendor, "Rise");
	else if(!strcmp(vendor, "SiS SiS SiS ")) /* SiS */
		strcpy(prettyvendor, "SiS");
	else if(!strcmp(vendor, "Geode by NSC")) /* National Semiconductor */
		strcpy(prettyvendor, "National Semiconductor");
	else
		strcpy(prettyvendor, "Unknown");

}
# endif
#endif /* HAS_LIBCPUID */

/* Get CPU frequencies (current - min - max) */
#ifdef NEXT
void cpufreq(char *busfreq, char *clock, char *mults)
{
	static int error = 0;
	char multmin[P] = { "0" }, multmax[P] = { "0" };
	FILE *fmin = NULL, *fmax = NULL;

	if(HAS_LIBCPUID)
		snprintf(clock, MAXSTR, "%d MHz", cpu_clock());

	/* Can't get base clock without root rights, skip multiplicators calculation */
	if(!getuid())
	{
#ifdef __linux__
		if(error != 1 && error != 3) {
			fmin = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq", "r");
			if(fmin == NULL) {
				MSGERR("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq'.");
				error = 1;
			}
			else {
				fgets(multmin, 9, fmin);
				fclose(fmin);
			}
		}

		if(error != 2 && error != 3) {
			fmax = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
			if(fmax == NULL) {
				MSGERR("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq'.");
				error = (error == 1) ? 3 : 2;
			}
			else {
				fgets(multmax, 9, fmax);
				fclose(fmax);
			}
		}
#endif /* __linux__ */
		mult(busfreq, clock, multmin, multmax, mults);
	}
}
#else
void cpufreq(Internal *global, char *busfreq) {
	static int error = 0;
	char multmin[P] = { "0" }, multmax[P] = { "0" };
	FILE *fmin = NULL, *fmax = NULL;

	if(HAS_LIBCPUID)
		sprintf(global->clock, "%d MHz", cpu_clock());

	/* Can't get base clock without root rights, skip multiplicators calculation */
	if(!getuid()) {
#ifdef __linux__
		if(error != 1 && error != 3) {
			fmin = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq", "r");
			if(fmin == NULL) {
				MSGERR("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq'.");
				error = 1;
			}
			else {
				fgets(multmin, 9, fmin);
				fclose(fmin);
			}
		}

		if(error != 2 && error != 3) {
			fmax = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
			if(fmax == NULL) {
				MSGERR("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq'.");
				error = (error == 1) ? 3 : 2;
			}
			else {
				fgets(multmax, 9, fmax);
				fclose(fmax);
			}
		}
#endif /* __linux__ */
		mult(busfreq, global->clock, multmin, multmax, global->mults);
	}
}
#endif

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char *c) {
	int i = 0, j = 0;
	char read[20];
	char *mips = NULL;
	FILE *cpuinfo = NULL;

#ifdef __linux__
	cpuinfo = fopen("/proc/cpuinfo", "r");
	if(cpuinfo == NULL) {
		MSGERR("failed to open '/proc/cpuinfo'.");
		return;
	}

	while(fgets(read, sizeof(read), cpuinfo) != NULL) {
		mips = strstr(read, "bogomips");
		if(mips != NULL)
			break;
	}

	while(mips[i] != '\n') {
		if(isdigit(mips[i]) || mips[i] == '.') {
			c[j] = mips[i];
			j++;
		}
		i++;
	}
	c[j] = '\0';
#else
	sprintf(c, "%s", "Unavailable");
#endif /* __linux__ */
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
# ifdef NEXT
void instructions(char arch[MAXSTR], char instr[MAXSTR])
{
	struct cpu_raw_data_t raw;
	struct cpu_id_t id;

	if (!cpuid_get_raw_data(&raw) && !cpu_identify(&raw, &id))
	{
		if(id.flags[CPU_FEATURE_MMX])
		{
			strcpy(instr, "MMX");
			if(id.flags[CPU_FEATURE_MMXEXT])
				strcat(instr, "(+)");
		}
		if(id.flags[CPU_FEATURE_3DNOW])
		{
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
			strcpy(arch, "x86_64 (64-bit)");
		else
			strcpy(arch, "ix86 (32-bit)");
	}
	else
		MSGERR("failed to call 'libcpuid'.");
}
# else
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
# endif
#endif /* HAS_LIBCPUID */

#ifdef NEXT
/* Dump all datas in stdout */
void dump_data(Labels *data)
{
	int i;

	/* Tab CPU */
	printf("\t***** CPU *****\n");
	for(i = VENDOR; i < LASTCPU; i++)
		printf("%16s: %s\n", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);

	/* Tab Mainboard */
	printf("\n\t***** Mainboard *****\n");
	for(i = MANUFACTURER; i < LASTMB; i++)
		printf("%16s: %s\n", data->tabmb[NAME][i], data->tabmb[VALUE][i]);
}
#endif

/* Search file location */
#ifdef NEXT
char *get_path(char *file)
{
	/* Taken from http://www.advancedlinuxprogramming.com/listings/chapter-7/get-exe-path.c
	See this file for more informations */
	char *path_end;
	char *buffer = malloc(PATH_MAX);
	size_t len = PATH_MAX;

	if(readlink ("/proc/self/exe", buffer, len) <= 0)
		return NULL;

	path_end = strrchr(buffer, '/');
	if(path_end == NULL)
		return NULL;

	path_end++;
	*path_end = '\0';

	if(!strcmp(file, "locale"))
		sprintf(buffer, "%s../share/%s", buffer, file);
	else
		sprintf(buffer, "%s../share/cpu-x/%s", buffer, file);

	return buffer;
}
#else
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
#endif
