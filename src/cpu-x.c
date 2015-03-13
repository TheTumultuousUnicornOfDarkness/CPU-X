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
#include <math.h>
#include <limits.h>
#include <sys/utsname.h>
#include <locale.h>
#include <libintl.h>
#include "cpu-x.h"

#if HAS_GTK
# include "gui_gtk.h"
#endif

#if HAS_NCURSES
# include "tui_ncurses.h"
#endif

#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
#endif

#if HAS_LIBDMI
# include "dmidecode/libdmi.h"
#endif

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


int main(int argc, char *argv[]) {
	char option;
	option = menu(argc, argv);

	Labels data;
	MSGVERB("Set locale");
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	MSGVERB("Initialize main array");
	labels_setempty(&data);
	labels_setname(&data);
	bogomips(data.tabcpu[VALUE][BOGOMIPS]);
	tabsystem(&data);

	if(HAS_LIBCPUID)
	{
		if(libcpuid(&data))
			MSGERR("libcpuid failed.");
		else
		{
			cpuvendor(data.tabcpu[VALUE][VENDOR]);
			instructions(data.tabcpu[VALUE][ARCHITECTURE], data.tabcpu[VALUE][INSTRUCTIONS]);
		}
	}

	if(!getuid() && HAS_LIBDMI)
	{
		if(libdmidecode(&data))
			MSGERR("libdmidecode failed");
	}
	else
	{
		if(libdmi_fallback(&data))
			MSGERR("libdmi_fallback failed");
	}

	cpufreq(data.tabcpu[VALUE][BUSSPEED], data.tabcpu[VALUE][CORESPEED], data.tabcpu[VALUE][MULTIPLIER]);

	/* Start GUI */
	if(HAS_GTK && option == 'G') /* Start with GTK3 */
		start_gui_gtk(&argc, &argv, &data);
	else if(HAS_NCURSES && option == 'n') /* Start with NCurses */
		start_tui_ncurses(&data);
	else if(HAS_LIBDMI && option == 'D') /* Just run command dmidecode */
		libdmi(NULL, option);
	else if(option == 'd') /* Just dump datas */
		dump_data(&data);

	/* If compiled without GUI */
	if(!HAS_GTK && !HAS_NCURSES && option != 'D')
	{
		fprintf(stderr, "%s is compiled without GUI support. Dumping data...\n\n", PRGNAME);
		dump_data(&data);
	}

	return EXIT_SUCCESS;
}

/* Set empty labels */
void labels_setempty(Labels *data)
{
	int i;

	MSGVERB(_("Put null value in labels"));

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

	/* Tab RAM */
	for(i = BANK0_0; i < LASTRAM; i++)
	{
		memset(data->tabram[NAME][i], 0, MAXSTR);
		memset(data->tabram[VALUE][i], 0, MAXSTR);
	}

	/* Tab System */
	for(i = KERNEL; i < LASTSYS; i++)
	{
		memset(data->tabsys[NAME][i], 0, MAXSTR);
		memset(data->tabsys[VALUE][i], 0, MAXSTR);
	}
}

/* Set labels name */
void labels_setname(Labels *data)
{
	MSGVERB(_("Set labels name"));

	/* Various objects*/
	snprintf(data->objects[TABCPU],			MAXSTR, _("CPU"));
	snprintf(data->objects[TABMB],			MAXSTR, _("Mainboard"));
	snprintf(data->objects[TABRAM],			MAXSTR, _("RAM"));
	snprintf(data->objects[TABSYS],			MAXSTR, _("System"));
	snprintf(data->objects[TABABOUT],		MAXSTR, _("About"));
	snprintf(data->objects[FRAMPROCESSOR],		MAXSTR, _("Processor"));
	snprintf(data->objects[FRAMCLOCKS],		MAXSTR, _("Clocks"));
	snprintf(data->objects[FRAMCACHE],		MAXSTR, _("Cache"));
	snprintf(data->objects[FRAMMOBO],		MAXSTR, _("Motherboard"));
	snprintf(data->objects[FRAMBIOS],		MAXSTR, _("BIOS"));
	snprintf(data->objects[FRAMBANKS],		MAXSTR, _("Banks"));
	snprintf(data->objects[FRAMOS],			MAXSTR, _("Operating System"));
	snprintf(data->objects[FRAMMEMORY],		MAXSTR, _("Memory"));
	snprintf(data->objects[FRAMABOUT],		MAXSTR, _("About"));
	snprintf(data->objects[FRAMLICENSE],		MAXSTR, _("License"));
	snprintf(data->objects[LABVERSION],		MAXSTR, _("Version %s"), PRGVER);
	snprintf(data->objects[LABDESCRIPTION],		MAXSTR + 40, _(
		"%s is a Free software that gathers information\n"
		"on CPU, motherboard and more."), PRGNAME);
	snprintf(data->objects[LABAUTHOR],		MAXSTR, _("Author : %s"), PRGAUTH);
	snprintf(data->objects[LABCOPYRIGHT],		MAXSTR, "%s", PRGCPYR);
	snprintf(data->objects[LABLICENSE],		MAXSTR + 40, _(
		"This program comes with ABSOLUTELY NO WARRANTY"));

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

	/* Tab RAM */
	snprintf(data->tabram[NAME][BANK0_0],		MAXSTR, _("Bank 0 Ref."));
	snprintf(data->tabram[NAME][BANK0_1],		MAXSTR, _("Bank 0 Type"));
	snprintf(data->tabram[NAME][BANK1_0],		MAXSTR, _("Bank 1 Ref."));
	snprintf(data->tabram[NAME][BANK1_1],		MAXSTR, _("Bank 1 Type"));
	snprintf(data->tabram[NAME][BANK2_0],		MAXSTR, _("Bank 2 Ref."));
	snprintf(data->tabram[NAME][BANK2_1],		MAXSTR, _("Bank 2 Type"));
	snprintf(data->tabram[NAME][BANK3_0],		MAXSTR, _("Bank 3 Ref."));
	snprintf(data->tabram[NAME][BANK3_1],		MAXSTR, _("Bank 3 Type"));
	snprintf(data->tabram[NAME][BANK4_0],		MAXSTR, _("Bank 4 Ref."));
	snprintf(data->tabram[NAME][BANK4_1],		MAXSTR, _("Bank 4 Type"));
	snprintf(data->tabram[NAME][BANK5_0],		MAXSTR, _("Bank 5 Ref."));
	snprintf(data->tabram[NAME][BANK5_1],		MAXSTR, _("Bank 5 Type"));
	snprintf(data->tabram[NAME][BANK6_0],		MAXSTR, _("Bank 6 Ref."));
	snprintf(data->tabram[NAME][BANK6_1],		MAXSTR, _("Bank 6 Type"));
	snprintf(data->tabram[NAME][BANK7_0],		MAXSTR, _("Bank 7 Ref."));
	snprintf(data->tabram[NAME][BANK7_1],		MAXSTR, _("Bank 7 Type"));

	/* Tab System */
	snprintf(data->tabsys[NAME][KERNEL],		MAXSTR, _("Kernel"));
	snprintf(data->tabsys[NAME][DISTRIBUTION],	MAXSTR, _("Distribution"));
	snprintf(data->tabsys[NAME][HOSTNAME],		MAXSTR, _("Hostname"));
	snprintf(data->tabsys[NAME][UPTIME],		MAXSTR, _("Uptime"));
	snprintf(data->tabsys[NAME][COMPILER],		MAXSTR, _("Compiler"));

	snprintf(data->tabsys[NAME][USED],		MAXSTR, _("Used"));
	snprintf(data->tabsys[NAME][BUFFERS],		MAXSTR, _("Buffers"));
	snprintf(data->tabsys[NAME][CACHED],		MAXSTR, _("Cached"));
	snprintf(data->tabsys[NAME][FREE],		MAXSTR, _("Free"));
	snprintf(data->tabsys[NAME][SWAP],		MAXSTR, _("Swap"));
}

#if HAS_LIBCPUID
/* Elements provided by libcpuid library */
int libcpuid(Labels *data)
{
	int err = 0;
	char tmp[MAXSTR];
	struct cpu_raw_data_t raw;
	struct cpu_id_t datanr;

	MSGVERB("Call Libcpuid");
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

	MSGVERB(_("Fill array with values provided by Libcpuid"));
	if(datanr.l1_data_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL1D],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l1_data_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, ", %2d-way", datanr.l1_assoc);
			strncat(data->tabcpu[VALUE][LEVEL1D], tmp, MAXSTR);
		}
	}

	if(datanr.l1_instruction_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL1I],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l1_instruction_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, ", %2d-way", datanr.l1_assoc);
			strncat(data->tabcpu[VALUE][LEVEL1I], tmp, MAXSTR);
		}
	}

	if(datanr.l2_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL2],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l2_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, ", %2d-way", datanr.l2_assoc);
			strncat(data->tabcpu[VALUE][LEVEL2], tmp, MAXSTR);
		}
	}

	if(datanr.l3_cache > 0)
	{
		snprintf(data->tabcpu[VALUE][LEVEL3],	MAXSTR, "%d x %4d KB", datanr.num_cores, datanr.l3_cache);
		if(datanr.l1_assoc > 0)
		{
			snprintf(tmp, MAXSTR, ", %2d-way", datanr.l3_assoc);
			strncat(data->tabcpu[VALUE][LEVEL3], tmp, MAXSTR);
		}
	}

	if(datanr.num_cores > 0) /* Avoid divide by 0 */
		snprintf(data->tabcpu[VALUE][SOCKETS],	MAXSTR, "%d", datanr.total_logical_cpus / datanr.num_cores);

	snprintf(data->tabcpu[VALUE][CORES],		MAXSTR, "%d", datanr.num_cores);
	snprintf(data->tabcpu[VALUE][THREADS],		MAXSTR, "%d", datanr.num_logical_cpus);

	clean_specification(data->tabcpu[VALUE][SPECIFICATION]);

	return err;
}
#endif /* HAS_LIBCPUID */

#if HAS_LIBDMI
/* Elements provided by libdmi library (need root privileges) */
int libdmidecode(Labels *data)
{
	int i, err = 0;
	static int nodyn = 0;
	char datanr[LASTRAM][MAXSTR] = { { '\0' } };

	MSGVERB("Call Libdmi");
	err += libdmi(datanr, 'c');

	/* Tab CPU */
	MSGVERB(_("Fill array with values provided by Libdmi"));
	strncpy(data->tabcpu[VALUE][PACKAGE],  datanr[PROC_PACKAGE], MAXSTR);
	strncpy(data->tabcpu[VALUE][BUSSPEED], datanr[PROC_BUS], MAXSTR);

	/* Skip this part on refresh */
	if(!nodyn)
	{
		/* Tab Mainboard */
		err += libdmi(datanr, 'm');
		for(i = MANUFACTURER; i < LASTMB; i++)
			strncpy(data->tabmb[VALUE][i], datanr[i], MAXSTR);

		/* Tab RAM */
		err += libdmi(datanr, 'r');
		for(i = BANK0_0; i < LASTRAM; i++)
			strncpy(data->tabram[VALUE][i], datanr[i], MAXSTR);

		nodyn++;
	}

	return err;
}
#endif /* HAS_LIBDMI */

/* Alternative for libdmidecode (Linux only) */
int libdmi_fallback(Labels *data)
{
	int i, err = 0;

	MSGVERB("Call Libdmi (fallback mode)");

#ifdef __linux__
	char path[PATH_MAX];
	const char *id[LASTMB] = { "board_vendor", "board_name", "board_version", "bios_vendor", "bios_version", "bios_date" };
	FILE *mb[LASTMB] = { NULL };

	/* Tab Mainboard */
	for(i = MANUFACTURER; i < ROMSIZE; i++)
	{
		snprintf(path, PATH_MAX, "%s/%s", SYS_DMI, id[i]);
		mb[i] = fopen(path, "r");
		if(mb[i] != NULL)
		{
			fgets(data->tabmb[VALUE][i], MAXSTR, mb[i]);
			data->tabmb[VALUE][i][ strlen(data->tabmb[VALUE][i]) - 1 ] = '\0';
			fclose(mb[i]);
		}
		else
			err++;
	}
#endif /* __linux__ */

	return err;
}

#if HAS_LIBCPUID
/* Pretty label CPU Vendor */
void cpuvendor(char *vendor)
{
	/* This use Libcpuid. See here: https://github.com/anrieff/libcpuid/blob/master/libcpuid/cpuid_main.c#L233 */

	MSGVERB(_("Improve CPU Vendor label"));
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

/* Remove unwanted spaces in value Specification */
void clean_specification(char *spec)
{
	int i = 0, j = 0, skip = 0;
	char tmp[MAXSTR];

	MSGVERB(_("Remove unnecessary spaces in value Specification"));
	while(spec[i] != '\0')
	{
		if(isspace(spec[i]))
			skip = 1;
		else
		{
			if(skip)
			{
				tmp[j] = ' ';
				skip = 0;
				j++;
			}

			tmp[j] = spec[i];
			j++;
		}
		i++;
	}
	tmp[j] = '\0';
	strcpy(spec, tmp);
}
#endif /* HAS_LIBCPUID */

/* Get CPU frequencies (current - min - max) */
void cpufreq(char *busfreq, char *clock, char *mults)
{
	static int error = 0;
	char multmin[S] = { "0" }, multmax[S] = { "0" };
	FILE *fmin = NULL, *fmax = NULL;

	MSGVERB("Get CPU frequency");
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

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char *c) {
	int i = 0, j = 0;
	char read[20];
	char *mips = NULL;
	FILE *cpuinfo = NULL;

	MSGVERB(_("Read value BogoMIPS"));

#ifdef __linux__
	cpuinfo = fopen("/proc/cpuinfo", "r");
	if(cpuinfo == NULL) {
		MSGERR("failed to open '/proc/cpuinfo'.");
	}
	else
	{
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
	}
#else
	sprintf(c, "%s", "- - - -");
#endif /* __linux__ */
}

/* Determine CPU multiplicator from base clock */
void mult(char *busfreq, char *cpufreq, char *multmin, char *multmax, char multsynt[MAXSTR])
{
	int i, fcpu, fbus, cur, min, max;
	char ncpu[S] = "", nbus[S] = "";

	MSGVERB(_("Estimate CPU multiplicateurs (current - minimum - maximum)"));
	for(i = 0; isdigit(cpufreq[i]); i++)
		ncpu[i] = cpufreq[i];
	fcpu = atoi(ncpu);

	for(i = 0; isdigit(busfreq[i]); i++)
		nbus[i] = busfreq[i];
	nbus[i] = '\0';
	fbus = atoi(nbus);
	cur = round((double) fcpu / fbus);
	min = atoi(multmin);
	max = atoi(multmax);

	if(fbus > 0)
	{
		if(min >= 10000)
			min /= (fbus * 1000);
		if(max >= 10000 && fbus > 0)
			max /= (fbus * 1000);
		sprintf(multsynt, "x %i (%i-%i)", cur, min, max);
	}
}

#if HAS_LIBCPUID
/* Show some instructions supported by CPU */
void instructions(char arch[MAXSTR], char instr[MAXSTR])
{
	struct cpu_raw_data_t raw;
	struct cpu_id_t id;

	MSGVERB(_("Find CPU instructions"));
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
#endif /* HAS_LIBCPUID */

/* Get system informations */
void tabsystem(Labels *data)
{
	MSGVERB(_("Fill System tab"));
	static int called = 0;
	long int duptime, huptime, muptime, suptime = 0, memtot;
	struct utsname name;
	FILE *cc;
	uname(&name);

#ifdef __linux__
	const int div = 1000;
	char *filestr = NULL, *distro = NULL;
	FILE *osrel = NULL;

	snprintf(data->tabsys[VALUE][KERNEL], MAXSTR, "%s %s", name.sysname, name.release); /* Label Kernel */
	suptime = uptime(NULL, NULL); /* Label Uptime */

	osrel = fopen("/etc/os-release", "r"); /* Label Distribution */
	if(osrel == NULL && !called)
		MSGERR("can't open file '/etc/os-release'.");
	else if(!called)
	{
		filestr = malloc(500 * (sizeof(char)));
		if(filestr == NULL)
			MSGERR("malloc failed.");
		else
		{
			fread(filestr, sizeof(char), 500, osrel);
			distro = strstr(filestr, "PRETTY_NAME=");
			if(distro == NULL)
				snprintf(data->tabsys[VALUE][DISTRIBUTION], MAXSTR, _("Unknown distro"));
			else
			snprintf(data->tabsys[VALUE][DISTRIBUTION], MAXSTR, "%s", strtok(strchr(distro, '"') + 1, "\""));
			fclose(osrel);
			free(filestr);
		}
	}
	called = 1;

# if HAS_LIBPROCPS
	meminfo(); /* Memory labels */
	memtot = kb_main_total / div;

	snprintf(data->tabsys[VALUE][USED], MAXSTR, "%5ld MB / %5ld MB", kb_main_used / div, memtot);
	snprintf(data->tabsys[VALUE][BUFFERS], MAXSTR, "%5ld MB / %5ld MB", kb_main_buffers / div, memtot);
	snprintf(data->tabsys[VALUE][CACHED], MAXSTR, "%5ld MB / %5ld MB", kb_main_cached / div, memtot);
	snprintf(data->tabsys[VALUE][FREE], MAXSTR, "%5ld MB / %5ld MB", kb_main_free / div, memtot);
	snprintf(data->tabsys[VALUE][SWAP], MAXSTR, "%5ld MB / %5ld MB", kb_swap_used / div, kb_swap_total / div);
# endif /* HAS_LIBPROCPS */

#else /* __ linux__ */
	char os[MAXSTR];
	size_t len = sizeof(os);
	const int div = 1000000;

	sysctlbyname("kern.osrelease", &os, &len, NULL, 0); /* Label Kernel */
	stpncpy(data->tabsys[VALUE][KERNEL], os, MAXSTR);

	sysctlbyname("kern.ostype", &os, &len, NULL, 0); /* Label Distribution */
	stpncpy(data->tabsys[VALUE][DISTRIBUTION], os, MAXSTR);

# if HAS_LIBSTATGRAB
	sg_mem_stats *mem; /* Memory labels */
	sg_swap_stats *swap;

	if(!called)
	{
		sg_init(0);
		called = 1;
	}

	mem  = sg_get_mem_stats(NULL);
	swap = sg_get_swap_stats(NULL);

	memtot = mem->total / div;
	snprintf(data->tabsys[VALUE][USED], MAXSTR, "%5llu MB / %5ld MB", mem->used / div, memtot);
	snprintf(data->tabsys[VALUE][BUFFERS], MAXSTR, "%5u MB / %5ld MB", 0, memtot);
	snprintf(data->tabsys[VALUE][CACHED], MAXSTR, "%5llu MB / %5ld MB", mem->cache / div, memtot);
	snprintf(data->tabsys[VALUE][FREE], MAXSTR, "%5llu MB / %5ld MB", mem->free / div, memtot);
	snprintf(data->tabsys[VALUE][SWAP], MAXSTR, "%5llu MB / %5llu MB", swap->used / div, swap->total / div);
# endif /* HAS_LIBSTATGRAB */

#endif /* __linux__ */

#if defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	struct timespec tsp;

	clock_gettime(CLOCK_MONOTONIC, &tsp); /* Label Uptime */
	suptime = tsp.tv_sec;
#endif /* BSD */

#ifdef __MACH__
	clock_serv_t cclock;
	mach_timespec_t mts;

	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock); /* Label Uptime */
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	suptime = mts.tv_sec;
#endif /* __MACH__ */

#ifdef __APPLE__
	char *tmp;
	tmp = strdup(data->tabsys[VALUE][KERNEL]);

	snprintf(data->tabsys[VALUE][KERNEL], MAXSTR, "%s %s", data->tabsys[VALUE][DISTRIBUTION], tmp); /* Label Kernel */

	cc = popen("echo $(sw_vers -productName ; sw_vers -productVersion)", "r"); /* Label Distribution */
	if(cc != NULL)
	{
		fgets(data->tabsys[VALUE][DISTRIBUTION], MAXSTR, cc);
		pclose(cc);
	}
#endif /* __APPLE__ */

	if(suptime > 0)
	{
		duptime = suptime / (24 * 60 * 60); suptime -= duptime * (24 * 60 * 60); /* Label Uptime */
		huptime = suptime / (60 * 60); suptime -= huptime * (60 * 60);
		muptime = suptime / 60; suptime -= muptime * 60;
		snprintf(data->tabsys[VALUE][UPTIME], MAXSTR, _("%ld days, %2ld hours, %2ld minutes, %2ld seconds"), duptime, huptime, muptime, suptime);
	}

	snprintf(data->tabsys[VALUE][HOSTNAME],	MAXSTR, "%s", name.nodename); /* Label Hostname */

	cc = popen("cc --version", "r"); /* Label Compiler */
	if(cc != NULL)
	{
		fgets(data->tabsys[VALUE][COMPILER], MAXSTR, cc);
		data->tabsys[VALUE][COMPILER][ strlen(data->tabsys[VALUE][COMPILER]) - 1] = '\0';
		pclose(cc);
	}
}

/* Find the number of existing banks */
int last_bank(Labels *data)
{
	int i, cpt = LASTRAM;

	for(i = BANK7_0; i >= BANK0_0; i -= 2)
	{
		if(data->tabram[VALUE][i][0] == '\0')
			cpt -= 2;
	}

	return cpt;
}

/* Dump all data in stdout */
void dump_data(Labels *data)
{
	int i;

	if(getuid())
	{
		fprintf(stderr, "\n\t\t\t\033[1;33m");
		fprintf(stderr, MSGROOT, PRGNAME);
		fprintf(stderr, "\n\n\033[0m");
	}

	/* Tab CPU */
	printf(" ***** %s *****\n\n", data->objects[TABCPU]);
	printf("\t*** %s ***\n", data->objects[FRAMPROCESSOR]);
	for(i = VENDOR; i < LASTCPU; i++)
	{
		if(i == CORESPEED)
			printf("\n\t*** %s ***\n", data->objects[FRAMCLOCKS]);
		else if(i == LEVEL1D)
			printf("\n\t*** %s ***\n", data->objects[FRAMCACHE]);
		else if(i == SOCKETS)
			printf("\n\t***  ***\n");
		printf("%16s: %s\n", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);
	}

	/* Tab Mainboard */

	printf("\n\n ***** %s *****\n", data->objects[TABMB]);
	printf("\n\t*** %s ***\n", data->objects[FRAMMOBO]);
	for(i = MANUFACTURER; i < LASTMB; i++)
	{
		if(i == BRAND)
			printf("\n\t*** %s ***\n", data->objects[FRAMBIOS]);
		printf("%16s: %s\n", data->tabmb[NAME][i], data->tabmb[VALUE][i]);
	}

	/* Tab RAM */
	printf("\n\n ***** %s *****\n", data->objects[TABRAM]);
	printf("\n\t*** %s ***\n", data->objects[FRAMBANKS]);
	for(i = BANK0_0; i < last_bank(data); i++)
		printf("%16s: %s\n", data->tabram[NAME][i], data->tabram[VALUE][i]);

	/* Tab System */
	printf("\n\n ***** %s *****\n", data->objects[TABSYS]);
	printf("\n\t*** %s ***\n", data->objects[FRAMOS]);
	for(i = KERNEL; i < LASTSYS; i++)
	{
		if(i == USED)
			printf("\n\t*** %s ***\n", data->objects[FRAMMEMORY]);
		printf("%16s: %s\n", data->tabsys[NAME][i], data->tabsys[VALUE][i]);
	}
}
