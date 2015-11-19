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
* FILE core.c
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <libintl.h>
#include "core.h"

#if EMBED
# include <sys/stat.h>
#endif

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

#if HAS_LIBPCI
# include "pci/pci.h"
#endif


#if HAS_LIBCPUID
/* Elements provided by libcpuid library */
int libcpuid(Labels *data)
{
	int err = 0, tech, temp;
	double volt;
	struct cpu_raw_data_t raw;
	struct cpu_id_t datanr;

	MSGVERB(_("Filling labels (libcpuid step)"));
	err += cpuid_get_raw_data(&raw);
	err += cpu_identify(&raw, &datanr);

	/* Tab CPU */
	asprintf(&data->tabcpu[VALUE][VENDOR],		"%s", datanr.vendor_str);
	asprintf(&data->tabcpu[VALUE][CODENAME],	"%s", datanr.cpu_codename);
	asprintf(&data->tabcpu[VALUE][SPECIFICATION],	"%s", datanr.brand_str);
	asprintf(&data->tabcpu[VALUE][FAMILY],		"%d", datanr.family);
	asprintf(&data->tabcpu[VALUE][EXTFAMILY],	"%d", datanr.ext_family);
	asprintf(&data->tabcpu[VALUE][MODEL],		"%d", datanr.model);
	asprintf(&data->tabcpu[VALUE][EXTMODEL],	"%d", datanr.ext_model);
	asprintf(&data->tabcpu[VALUE][STEPPING],	"%d", datanr.stepping);

	tech = cpu_technology(datanr.model, datanr.ext_family, datanr.ext_model);
	if(tech)
		asprintf(&data->tabcpu[VALUE][TECHNOLOGY], "%i nm", tech);

	volt = cpu_voltage(0);
	if(volt)
		asprintf(&data->tabcpu[VALUE][VOLTAGE], "%.3f V", volt);

	temp = cpu_temperature(0);
	if(temp)
		asprintf(&data->tabcpu[VALUE][TEMPERATURE], "%i°C", temp);

	/* Cache : level 1 (data) */
	if(datanr.l1_data_cache)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL1D],	"%d x %4d KB", datanr.num_cores, datanr.l1_data_cache);

		if(datanr.l1_assoc > 0)
			asprintf(&data->tabcpu[VALUE][LEVEL1D],	  "%s, %2d-way", data->tabcpu[VALUE][LEVEL1D], datanr.l1_assoc);
	}

	/* Cache : level 1 (instruction) */
	if(datanr.l1_instruction_cache > 0)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL1I],	"%d x %4d KB", datanr.num_cores, datanr.l1_instruction_cache);
		data->tabcache[VALUE][L1SIZE] = strdup(data->tabcpu[VALUE][LEVEL1I]);

		if(datanr.l1_assoc > 0)
		{
			asprintf(&data->tabcpu[VALUE][LEVEL1I],	  "%s, %2d-way", data->tabcpu[VALUE][LEVEL1I], datanr.l1_assoc);
			asprintf(&data->tabcache[VALUE][L1DESCR], "%2d-way set associative", datanr.l1_assoc);
		}
		if(datanr.l1_cacheline > 0)
			asprintf(&data->tabcache[VALUE][L1DESCR], "%s, %2d-byte line size", data->tabcache[VALUE][L1DESCR], datanr.l1_cacheline);
	}

	/* Cache : level 2 */
	if(datanr.l2_cache > 0)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL2], "%d x %4d KB", datanr.num_cores, datanr.l2_cache);
		data->tabcache[VALUE][L2SIZE] = strdup(data->tabcpu[VALUE][LEVEL2]);

		if(datanr.l2_assoc > 0)
		{
			asprintf(&data->tabcpu[VALUE][LEVEL2],	  "%s, %2d-way", data->tabcpu[VALUE][LEVEL2], datanr.l2_assoc);
			asprintf(&data->tabcache[VALUE][L2DESCR], "%2d-way set associative", datanr.l2_assoc);
		}
		if(datanr.l2_cacheline > 0)
			asprintf(&data->tabcache[VALUE][L2DESCR], "%s, %2d-byte line size", data->tabcache[VALUE][L2DESCR], datanr.l2_cacheline);
	}

	/* Cache : level 3 */
	if(datanr.l3_cache > 0)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL3], "%9d KB", datanr.l3_cache);
		data->tabcache[VALUE][L3SIZE] = strdup(data->tabcpu[VALUE][LEVEL3]);

		if(datanr.l3_assoc > 0)
		{
			asprintf(&data->tabcpu[VALUE][LEVEL3],	  "%s, %2d-way", data->tabcpu[VALUE][LEVEL3], datanr.l3_assoc);
			asprintf(&data->tabcache[VALUE][L3DESCR], "%2d-way set associative", datanr.l3_assoc);
		}
		if(datanr.l3_cacheline > 0)
			asprintf(&data->tabcache[VALUE][L3DESCR], "%s, %2d-byte line size", data->tabcache[VALUE][L3DESCR], datanr.l3_cacheline);
	}

	if(datanr.num_cores > 0) /* Avoid divide by 0 */
		asprintf(&data->tabcpu[VALUE][SOCKETS],	"%d", datanr.total_logical_cpus / datanr.num_logical_cpus);

	asprintf(&data->tabcpu[VALUE][CORES],		"%d", datanr.num_cores);
	asprintf(&data->tabcpu[VALUE][THREADS],		"%d", datanr.num_logical_cpus);

	clean_specification(data->tabcpu[VALUE][SPECIFICATION]);

	return err;
}

/* Pretty label CPU Vendor */
void cpuvendor(char *vendor)
{
	/* https://github.com/anrieff/libcpuid/blob/master/libcpuid/cpuid_main.c#L233 */
	MSGVERB(_("Improving CPU Vendor label"));

	if     (!strcmp(vendor, "GenuineIntel"))	strcpy(vendor, "Intel");
	else if(!strcmp(vendor, "AuthenticAMD"))	strcpy(vendor, "AMD");
	else if(!strcmp(vendor, "CyrixInstead"))	strcpy(vendor, "Cyrix");
	else if(!strcmp(vendor, "NexGenDriven"))	strcpy(vendor, "NexGen");
	else if(!strcmp(vendor, "GenuineTMx86"))	strcpy(vendor, "Transmeta");
	else if(!strcmp(vendor, "UMC UMC UMC "))	strcpy(vendor, "UMC");
	else if(!strcmp(vendor, "CentaurHauls"))	strcpy(vendor, "Centaur");
	else if(!strcmp(vendor, "RiseRiseRise"))	strcpy(vendor, "Rise");
	else if(!strcmp(vendor, "SiS SiS SiS "))	strcpy(vendor, "SiS");
	else if(!strcmp(vendor, "Geode by NSC"))	strcpy(vendor, "National Semiconductor");
	else						strcpy(vendor, "Unknown");
}

/* Remove unwanted spaces in value Specification */
void clean_specification(char *spec)
{
	int i = 0, j = 0, skip = 0;

	MSGVERB(_("Removing unnecessary spaces in label Specification"));
	while(spec[i] != '\0')
	{
		if(isspace(spec[i]) && !skip)
		{
			spec[j] = ' ';
			j++;
			skip = 1;
		}
		else if(!isspace(spec[i]))
		{
			spec[j] = spec[i];
			j++;
			skip = 0;
		}
		i++;
	}
	spec[j] = '\0';
}

void catinstr(char **str, char *in)
{
	int sep = 1;
	static int first = 1;
	char *tmp;

	if(first)
	{
		*str = strdupnullok(in);
		first = 0;
	}
	else
	{
		sep = isalnum(in[0]) ? 3 : sep;
		tmp = strdupnullok(*str);
		free(*str);
		tmp = (char *) realloc(tmp, (strlen(tmp) + strlen(in) + sep) * sizeof(char));

		if(isalnum(in[0]))
			strcat(tmp, ", ");
		strcat(tmp, in);
		*str = strdupnullok(tmp);
		free(tmp);
	}
}

/* Show some instructions supported by CPU */
void instructions(char **instr)
{
	struct cpu_raw_data_t raw;
	struct cpu_id_t id;

	MSGVERB(_("Finding CPU instructions"));
	if (!cpuid_get_raw_data(&raw) && !cpu_identify(&raw, &id))
	{
		if(id.flags[CPU_FEATURE_MMX])		catinstr(instr, "MMX");
		if(id.flags[CPU_FEATURE_MMXEXT])	catinstr(instr, "(+)");
		if(id.flags[CPU_FEATURE_3DNOW])		catinstr(instr, ", 3DNOW!");
		if(id.flags[CPU_FEATURE_3DNOWEXT])	catinstr(instr, "(+)");

		if(id.flags[CPU_FEATURE_SSE])		catinstr(instr, ", SSE (1");
		if(id.flags[CPU_FEATURE_SSE2])		catinstr(instr, ", 2");
		if(id.flags[CPU_FEATURE_SSSE3])		catinstr(instr, ", 3S");
		if(id.flags[CPU_FEATURE_SSE4_1])	catinstr(instr, ", 4.1");
		if(id.flags[CPU_FEATURE_SSE4_2])	catinstr(instr, ", 4.2");
		if(id.flags[CPU_FEATURE_SSE4A])		catinstr(instr, ", 4A");
		if(id.flags[CPU_FEATURE_SSE])		catinstr(instr, ")");

		if(id.flags[CPU_FEATURE_AES])		catinstr(instr, ", AES");
		if(id.flags[CPU_FEATURE_AVX])		catinstr(instr, ", AVX");
		if(id.flags[CPU_FEATURE_VMX])		catinstr(instr, ", VT-x");
		if(id.flags[CPU_FEATURE_SVM])		catinstr(instr, ", AMD-V");

		if(id.flags[CPU_FEATURE_LM])
		{
#ifdef HAVE_LIBCPUID_0_2_2
			switch(cpuid_get_vendor())
			{
				case VENDOR_INTEL:
					catinstr(instr, ", Intel 64");
					break;
				case VENDOR_AMD:
					catinstr(instr, ", AMD64");
					break;
				default:
					catinstr(instr, ", 64-bit");
			}
#else
			catinstr(instr, ", 64-bit");
#endif /* HAVE_LIBCPUID_0_2_2 */
		}
	}
	else
		MSGSERR(_("libcpuid failed"));
}

/* Get CPU core voltage */
double cpu_voltage(int core)
{
#ifdef HAVE_LIBCPUID_0_2_2
	int voltage = 0;
	static int err = 0;
	struct msr_driver_t *msr = NULL;

	MSGVERB(_("Finding CPU core voltage"));
	msr = cpu_msr_driver_open_core(core);
	if(msr != NULL)
	{
		voltage = cpu_msrinfo(msr, INFO_VOLTAGE);
		if(voltage != CPU_INVALID_VALUE)
			return (double) voltage / 100;
	}

	if(!err)
	{
		MSGSERR(_("error when finding CPU core voltage"));
		err++;
	}
#endif /* HAVE_LIBCPUID_0_2_2 */
	return 0.0;
}

/* Get CPU core temprature */
int cpu_temperature(int core)
{
#ifdef HAVE_LIBCPUID_0_2_2
	int temp = 0;
	static int err = 0;
	struct msr_driver_t *msr = NULL;

	MSGVERB(_("Finding CPU core temperature"));
	msr = cpu_msr_driver_open_core(core);
	if(msr != NULL)
	{
		temp = cpu_msrinfo(msr, INFO_TEMPERATURE);
		if(temp != CPU_INVALID_VALUE)
			return temp;
	}

	if(!err)
	{
		MSGSERR(_("error when finding CPU core temperature"));
		err++;
	}
#endif /* HAVE_LIBCPUID_0_2_2 */
	return 0;
}

/* Get CPU technology, in nanometre (nm) */
int cpu_technology(int32_t model, int32_t ext_family, int32_t ext_model)
{
#ifdef HAVE_LIBCPUID_0_2_2
	static int err = 0;

	MSGVERB(_("Finding CPU technology"));
	if(cpuid_get_vendor() == VENDOR_INTEL)
	{
		/* https://raw.githubusercontent.com/anrieff/libcpuid/master/libcpuid/recog_intel.c */
		switch(model)
		{
			case 5:
				if(ext_model == 37) return 32; // Westmere
				if(ext_model == 69) return 22; // Haswell
			case 7:
				if(ext_model == 23) return 45;
				if(ext_model == 71) return 14; // Broadwell
			case 10:
				if(ext_model == 26 || ext_model == 30) return 45; // Nehalem
				if(ext_model == 42) return 32; // Sandy Bridge
				if(ext_model == 58) return 22; // Ivy Bridge
			case 12:
				if(ext_model == 44) return 32; // Westmere
				if(ext_model == 60) return 22; // Haswell
			case 13:
				if(ext_model == 45) return 32; // Sandy Bridge-E
				if(ext_model == 61) return 14; // Broadwell
			case 14:
				if(ext_model == 62) return 22; // Ivy Bridge-E
				if(ext_model == 94) return 14; // Skylake
			case 15:
				if(ext_model == 63) return 22; // Haswell-E

		}
	}
	else if(cpuid_get_vendor() == VENDOR_AMD)
	{
		/* https://raw.githubusercontent.com/anrieff/libcpuid/master/libcpuid/recog_amd.c */
		switch(model)
		{
			case 0:
				if(ext_model == 0) return 28; // Jaguar (Kabini)
				if(ext_model == 10) return 32; // Piledriver (Trinity)
				if(ext_family == 21) return 28; // Steamroller (Kaveri)
				if(ext_family == 22) return 28; // Puma (Mullins)
			case 1:
				if(ext_model == 1) return 32; // K10 (Llano)
				if(ext_family == 20) return 40; // Bobcat
				if(ext_model == 60) return 28; // Excavator (Carrizo)
			case 2:
				if(ext_family == 20) return 40; // Bobcat
			case 3:
				if(ext_model == 13) return 32; // Piledriver (Richland)
		}
	}

	if(!err)
	{
		MSGSERR(_("error when finding CPU technology"));
		err++;
	}
#endif /* HAVE_LIBCPUID_0_2_2 */
	return 0;
}
#endif /* HAS_LIBCPUID */

#if HAS_LIBDMI
/* Elements provided by libdmi library (need root privileges) */
int libdmidecode(Labels *data)
{
	int i, err = 0;
	static int nodyn = 0;

	MSGVERB(_("Filling labels (libdmi step)"));
	/* Tab CPU */
	dmidata[PROC_PACKAGE]	= &data->tabcpu[VALUE][PACKAGE];
	dmidata[PROC_BUS]	= &data->tabcpu[VALUE][BUSSPEED];
	err += libdmi('c');

	/* Skip this part on refresh */
	if(!nodyn)
	{
		/* Tab Motherboard */
		for(i = MANUFACTURER; i < LASTMB; i++)
			dmidata[i] = &data->tabmb[VALUE][i];
		err += libdmi('m');

		/* Tab RAM */
		for(i = BANK0_0; i < LASTRAM; i++)
			dmidata[i] = &data->tabram[VALUE][i];
		err += libdmi('r');

		nodyn++;
	}

	return err;
}
#endif /* HAS_LIBDMI */

/* Alternative for libdmidecode (Linux only) */
int libdmi_fallback(Labels *data)
{
	int err = 0;

	MSGVERB(_("Filling labels (libdmi step, fallback mode)"));
#ifdef __linux__
	int i = 0, len;
	char path[PATH_MAX], buff[MAXSTR];
	const char *id[LASTMB] = { "board_vendor", "board_name", "board_version", "bios_vendor", "bios_version", "bios_date", NULL };
	FILE *mb[LASTMB] = { NULL };

	/* Tab Motherboard */
	while(id[i] != NULL)
	{
		snprintf(path, PATH_MAX, "%s/%s", SYS_DMI, id[i]);
		mb[i] = fopen(path, "r");
		if(mb[i] != NULL)
		{
			if(fgets(buff, MAXSTR, mb[i]) != NULL)
			{
				len = (strlen(buff) >= 1) ? strlen(buff) - 1 : 0;
				buff[len] = '\0';
				data->tabmb[VALUE][i] = strdupnullok(buff);
			}
			else
				err++;

			fclose(mb[i]);
		}
		else
			err++;

		i++;
	}
#endif /* __linux__ */

	return err;
}

/* Get CPU frequencies (current - min - max) */
void cpufreq(Labels *data)
{
	MSGVERB(_("Getting CPU frequency"));

	if(HAS_LIBCPUID)
		asprintf(&data->tabcpu[VALUE][CORESPEED], "%d MHz", cpu_clock());

#ifdef __linux__
	static int error = 0;
	char multmin[S] = { "0" }, multmax[S] = { "0" };
	FILE *fmin = NULL, *fmax = NULL;

	/* Can't get base clock without root rights, skip multiplicators calculation */
	if(!getuid())
	{
		if(error != 1 && error != 3)
		{
			fmin = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq", "r");
			if(fmin == NULL)
			{
				MSGPERR(_("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq'"));
				error = 1;
			}
			else {
				fgets(multmin, S - 1, fmin);
				fclose(fmin);
			}
		}

		if(error != 2 && error != 3)
		{
			fmax = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
			if(fmax == NULL)
			{
				MSGPERR(_("failed to open file '/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq'"));
				error = (error == 1) ? 3 : 2;
			}
			else {
				fgets(multmax, S - 1, fmax);
				fclose(fmax);
			}
		}

		if(data->tabcpu[VALUE][BUSSPEED] != NULL && data->tabcpu[VALUE][CORESPEED] != NULL)
			mult(data->tabcpu[VALUE][BUSSPEED], data->tabcpu[VALUE][CORESPEED], multmin, multmax, &data->tabcpu[VALUE][MULTIPLIER]);
	}
#endif /* __linux__ */
}

/* Determine CPU multiplicator from base clock */
void mult(char *busfreq, char *cpufreq, char *multmin, char *multmax, char **multsynt)
{
	int i, fcpu, min, max;
	static int fbus = -1;
	double cur;
	char ncpu[S] = "", nbus[S] = "";

	MSGVERB(_("Estimating CPU multipliers"));
	for(i = 0; isdigit(cpufreq[i]); i++)
		ncpu[i] = cpufreq[i];
	ncpu[i] = '\0';
	fcpu = atoi(ncpu);

	if(fbus == -1)
	{
		for(i = 0; isdigit(busfreq[i]); i++)
			nbus[i] = busfreq[i];
		nbus[i] = '\0';
		fbus = atoi(nbus);
	}

	cur = (double) fcpu / fbus;
	min = atoi(multmin);
	max = atoi(multmax);

	if(fbus > 0 && min >= 10000 && max >= 10000)
	{
		min /= (fbus * 1000);
		max /= (fbus * 1000);
		asprintf(multsynt, "x %i (%i-%i)", (int) round(cur), min, max);
	}
	else if(cur > 0)
			asprintf(multsynt, "x %.2f", cur);
}

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char **c)
{
	MSGVERB(_("Reading value BogoMIPS"));
#ifdef __linux__
	int i = 0, j = 0;
	char read[20];
	char tmp[10], *mips = NULL;
	FILE *cpuinfo = NULL;

	cpuinfo = fopen("/proc/cpuinfo", "r");
	if(cpuinfo == NULL)
	{
		MSGPERR(_("failed to open file '/proc/cpuinfo'"));
	}
	else
	{
		while(fgets(read, sizeof(read), cpuinfo) != NULL)
		{
			mips = strstr(read, "bogomips");
			if(mips != NULL)
				break;
		}

		while(mips[i] != '\n')
		{
			if(isdigit(mips[i]) || mips[i] == '.')
			{
				tmp[j] = mips[i];
				j++;
			}
			i++;
		}
		tmp[j] = '\0';
	}
	*c = strdupnullok(tmp);
#else
	*c = strdupnullok("- - - -");
#endif /* __linux__ */
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

#if HAS_LIBPCI
/* Find driver name for a device */
static char *find_driver(struct pci_dev *dev, char *buf)
{
	/* Taken from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/ls-kernel.c */
	int n;
	char name[MAXSTR], *drv, *base;

	if (dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
		return NULL;

	base = pci_get_param(dev->access, "sysfs.path");
	if (!base || !base[0])
		return NULL;

	n = snprintf(name, sizeof(name), "%s/devices/%04x:%02x:%02x.%d/driver",
		base, dev->domain, dev->bus, dev->dev, dev->func);
	if (n < 0 || n >= (int)sizeof(name))
		printf("show_driver: sysfs device name too long, why?");

	n = readlink(name, buf, MAXSTR);
	if (n < 0)
		return NULL;
	if (n >= MAXSTR)
		return "<name-too-long>";
	buf[n] = 0;

	if ((drv = strrchr(buf, '/')))
		return drv+1;
	else
		return buf;
}

/* Find some PCI devices */
void pcidev(Labels *data)
{
	/* Adapted from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/example.c */
	int nbgpu = 0;
	double temp = 0.0;
	struct pci_access *pacc;
	struct pci_dev *dev;
	char namebuf[MAXSTR], *vendor, *product, *drivername, *driverstr;

	MSGVERB(_("Filling labels (libpci step)"));
	pacc = pci_alloc();	/* Get the pci_access structure */
	pci_init(pacc);		/* Initialize the PCI library */
	pci_scan_bus(pacc);	/* We want to get the list of devices */

	for (dev=pacc->devices; dev; dev=dev->next)	/* Iterate over all devices */
	{
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
		vendor  = strdupnullok(pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id));
		product = strdupnullok(pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id));

		if(dev->device_class == PCI_CLASS_BRIDGE_ISA)	/* Looking for chipset */
		{
			data->tabmb[VALUE][CHIPVENDOR] = strdupnullok(vendor);
			data->tabmb[VALUE][CHIPNAME] = strdupnullok(product);
		}

		if(nbgpu < LASTGPU / GPUFIELDS &&
				(dev->device_class == PCI_BASE_CLASS_DISPLAY	||
				dev->device_class == PCI_CLASS_DISPLAY_VGA	||
				dev->device_class == PCI_CLASS_DISPLAY_XGA	||
				dev->device_class == PCI_CLASS_DISPLAY_3D	||
				dev->device_class == PCI_CLASS_DISPLAY_OTHER))	/* Looking for GPU */
		{
			drivername = find_driver(dev, namebuf);
			temp = gpu_temperature();
			if(drivername != NULL)
				asprintf(&driverstr, _("(%s driver)"), drivername);
			else
				driverstr = strdup("");
			asprintf(&data->tabgpu[VALUE][GPUVENDOR1	+ nbgpu * GPUFIELDS], "%s %s", clean_gpuvendor(vendor), driverstr);
			asprintf(&data->tabgpu[VALUE][GPUNAME1		+ nbgpu * GPUFIELDS], "%s", product);
			if(temp)
				asprintf(&data->tabgpu[VALUE][GPUTEMP1		+ nbgpu * GPUFIELDS], "%.2f°C", temp);
			nbgpu++;
		}
	}

	pci_cleanup(pacc);	/* Close everything */
	free(vendor);
	free(product);
}

/* Pretty label GPU Vendor */
char *clean_gpuvendor(char *str)
{
	if(strstr(str, "NVIDIA") != NULL)
		return strdup("NVIDIA");
	else if(strstr(str, "AMD") != NULL)
		return strdup("AMD");
	else if(strstr(str, "Intel") != NULL)
		return strdup("Intel");
	else
		return str;
}
#endif /* HAS_LIBPCI */

/* Get GPU tempreature */
double gpu_temperature(void)
{
	enum GPU_Driver { NVIDIA, CATALYST, MESA, UNKNOWN = -1 };
	static enum GPU_Driver driver = UNKNOWN;
	static int err = 0;
	char buff[MAXSTR];
	FILE *command = NULL;

	MSGVERB(_("Finding GPU temperature"));
	if(command_exists("nvidia-settings"))
	{
		command = popen("nvidia-settings -q GPUCoreTemp -t", "r");
		driver = NVIDIA;
	}
	else if(command_exists("aticonfig"))
	{
		command = popen("aticonfig --odgt | grep Sensor | awk '{ print $5 }'", "r");
		driver = CATALYST;
	}
	else
	{
		command = fopen("/sys/class/drm/card0/device/hwmon/hwmon0/temp1_input", "r");
		driver = MESA;
	}

	if((driver == NVIDIA || driver == CATALYST) && command != NULL)
	{
		fgets(buff, MAXSTR, command);
		return atof(buff);
	}
	else if(driver == MESA && command != NULL)
	{
		fgets(buff, MAXSTR, command);
		return ((double) atoi(buff) / 1000);
	}

	if(!err)
	{
		MSGSERR(_("error when finding GPU temperature"));
		err++;
	}
	return 0.0;
}

/* Find the number of GPU */
int last_gpu(Labels *data)
{
	int i, cpt = LASTGPU;

	for(i = GPUVENDOR4; i >= GPUVENDOR1; i -= GPUFIELDS)
	{
		if(data->tabgpu[VALUE][i][0] == '\0')
			cpt -= GPUFIELDS;
	}

	return cpt;
}

/* Check if a command exists */
int command_exists(char *in)
{
	int ret;
	char *cmd;

	asprintf(&cmd, "which %s >/dev/null 2>&1", in);
	ret = system(cmd);
	free(cmd);

	return !ret;
}
