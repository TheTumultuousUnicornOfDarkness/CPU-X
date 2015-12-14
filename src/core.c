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
#include <time.h>
#include <math.h>
#include <locale.h>
#include <libintl.h>
#include <sys/utsname.h>
#include "core.h"
#include "cpu-x.h"

#ifndef __linux__
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
#endif

#if HAS_DMIDECODE
# include "dmidecode/libdmi.h"
#endif

#if HAS_LIBPCI
# include "pci/pci.h"
#endif

#if HAS_LIBPROCPS
# include <proc/sysinfo.h>
#endif

#if HAS_LIBSTATGRAB
# include <statgrab.h>
#endif


/************************* Public functions *************************/

/* Fill labels by calling below functions */
int fill_labels(Labels *data)
{
	int fallback = 0, err = 0;

	if(HAS_LIBCPUID)	err += call_libcpuid_static(data);
	if(HAS_LIBCPUID)	err += call_libcpuid_dynamic(data);
	if(HAS_DMIDECODE)	err += fallback = call_dmidecode(data);
	if(HAS_LIBCPUID)	err += cpu_multipliers(data);
	if(HAS_LIBPROCPS)	err += system_dynamic(data);
	if(HAS_LIBSTATGRAB)	err += system_dynamic(data);
	if(HAS_BANDWIDTH)	err += bandwidth(data);
	if(HAS_LIBPCI)		find_devices(data);
	err += gpu_temperature(data);
	err += system_static(data);
	cpu_usage(data);

	if(fallback)
		fallback_mode(data);

	return err;
}

/* Refresh some labels */
int do_refresh(Labels *data, enum EnTabNumber page)
{
	int err;

	switch(page)
	{
		case NB_TAB_CPU:
			if(HAS_LIBCPUID)	err = call_libcpuid_dynamic(data);
			if(HAS_LIBCPUID)	err += cpu_multipliers(data);
			cpu_usage(data);
			break;
		case NB_TAB_CACHE:
			if(HAS_BANDWIDTH)	err = bandwidth(data);
			break;
		case NB_TAB_SYS:
			if(HAS_LIBPROCPS)	err = system_dynamic(data);
			if(HAS_LIBSTATGRAB)	err = system_dynamic(data);
			break;
		case NB_TAB_GPU:
			err = gpu_temperature(data);
			break;
		default:
			err = -1;
	}

	return err;
}


/************************* Private functions *************************/

#if HAS_LIBCPUID
/* Get CPU technology, in nanometre (nm) */
static int cpu_technology(Labels *data)
{
	char *msg;

	MSG_VERBOSE(_("Finding CPU technology"));
	if(data->cpu_vendor_id == VENDOR_INTEL)
	{
		/* https://raw.githubusercontent.com/anrieff/libcpuid/master/libcpuid/recog_intel.c */
		switch(data->cpu_model)
		{
			case 5:
				if(data->cpu_ext_model == 37) return 32; // Westmere
				if(data->cpu_ext_model == 69) return 22; // Haswell
			case 7:
				if(data->cpu_ext_model == 23) return 45;
				if(data->cpu_ext_model == 71) return 14; // Broadwell
			case 10:
				if(data->cpu_ext_model == 26 || data->cpu_ext_model == 30) return 45; // Nehalem
				if(data->cpu_ext_model == 42) return 32; // Sandy Bridge
				if(data->cpu_ext_model == 58) return 22; // Ivy Bridge
			case 12:
				if(data->cpu_ext_model == 44) return 32; // Westmere
				if(data->cpu_ext_model == 60) return 22; // Haswell
			case 13:
				if(data->cpu_ext_model == 45) return 32; // Sandy Bridge-E
				if(data->cpu_ext_model == 61) return 14; // Broadwell
			case 14:
				if(data->cpu_ext_model == 62) return 22; // Ivy Bridge-E
				if(data->cpu_ext_model == 94) return 14; // Skylake
			case 15:
				if(data->cpu_ext_model == 63) return 22; // Haswell-E
		}
	}
	else if(data->cpu_vendor_id == VENDOR_AMD)
	{
		/* https://raw.githubusercontent.com/anrieff/libcpuid/master/libcpuid/recog_amd.c */
		switch(data->cpu_model)
		{
			case 0:
				if(data->cpu_ext_model  == 0)  return 28; // Jaguar (Kabini)
				if(data->cpu_ext_model  == 10) return 32; // Piledriver (Trinity)
				if(data->cpu_ext_family == 21) return 28; // Steamroller (Kaveri)
				if(data->cpu_ext_family == 22) return 28; // Puma (Mullins)
			case 1:
				if(data->cpu_ext_model  == 1)  return 32; // K10 (Llano)
				if(data->cpu_ext_family == 20) return 40; // Bobcat
				if(data->cpu_ext_model  == 60) return 28; // Excavator (Carrizo)
			case 2:
				if(data->cpu_ext_family == 20) return 40; // Bobcat
			case 3:
				if(data->cpu_ext_model  == 13) return 32; // Piledriver (Richland)
		}
	}

	asprintf(&msg, _("your CPU does not belong in database\nCPU: %s, model: %i, ext. model: %i, ext. family: %i"),
	         data->tabcpu[VALUE][SPECIFICATION], data->cpu_model, data->cpu_ext_model, data->cpu_ext_family);
	MSG_ERROR(msg);

	return 0;
}

/* Static elements provided by libcpuid */
static int call_libcpuid_static(Labels *data)
{
	int i, j = 0;
	const char *fmt = { _("%2d-way set associative, %2d-byte line size") };
	struct cpu_raw_data_t raw;
	struct cpu_id_t datanr;

	/* Call libcpuid */
	MSG_VERBOSE(_("Calling libcpuid (retrieve static data)"));
	if(cpuid_get_raw_data(&raw) || cpu_identify(&raw, &datanr))
	{
		MSG_ERROR(_("failed to call libcpuid"));
		return 1;
	}

	/* Some prerequisites */
	data->cpu_count = datanr.num_logical_cpus;
	data->cpu_model = datanr.model;
	data->cpu_ext_model = datanr.ext_model;
	data->cpu_ext_family = datanr.ext_family;

	/* Basically fill CPU tab */
	iasprintf(&data->tabcpu[VALUE][CODENAME],	datanr.cpu_codename);
	iasprintf(&data->tabcpu[VALUE][SPECIFICATION],	datanr.brand_str);
	iasprintf(&data->tabcpu[VALUE][FAMILY],		"%d", datanr.family);
	iasprintf(&data->tabcpu[VALUE][EXTFAMILY],	"%d", datanr.ext_family);
	iasprintf(&data->tabcpu[VALUE][MODEL],		"%d", datanr.model);
	iasprintf(&data->tabcpu[VALUE][EXTMODEL],	"%d", datanr.ext_model);
	iasprintf(&data->tabcpu[VALUE][STEPPING],	"%d", datanr.stepping);
	iasprintf(&data->tabcpu[VALUE][TECHNOLOGY],	"%i nm", cpu_technology(data));
	iasprintf(&data->tabcpu[VALUE][CORES],		"%d", datanr.num_cores);
	iasprintf(&data->tabcpu[VALUE][THREADS],	"%d", datanr.num_logical_cpus);

	/* Improve the CPU Vendor label */
	const struct CpuVendor { char *standard; char *improved; cpu_vendor_t id; } cpuvendors[] =
	{
		{ "GenuineIntel", "Intel",                  VENDOR_INTEL     },
		{ "AuthenticAMD", "AMD",                    VENDOR_AMD       },
		{ "CyrixInstead", "Cyrix",                  VENDOR_CYRIX     },
		{ "NexGenDriven", "NexGen",                 VENDOR_NEXGEN    },
		{ "GenuineTMx86", "Transmeta",              VENDOR_TRANSMETA },
		{ "UMC UMC UMC ", "UMC",                    VENDOR_UMC       },
		{ "CentaurHauls", "Centaur",                VENDOR_CENTAUR   },
		{ "RiseRiseRise", "Rise",                   VENDOR_RISE      },
		{ "SiS SiS SiS ", "SiS",                    VENDOR_SIS       },
		{ "Geode by NSC", "National Semiconductor", VENDOR_NSC       },
		{ datanr.vendor_str, datanr.vendor_str,     VENDOR_UNKNOWN    }
	};
	for(i = 0; strcmp(cpuvendors[i].standard, datanr.vendor_str); i++);
	iasprintf(&data->tabcpu[VALUE][VENDOR], cpuvendors[i].improved);
	data->cpu_vendor_id = cpuvendors[i].id;

	/* Remove training spaces in Specification label */
	for(i = 1; datanr.brand_str[i] != '\0'; i++)
	{
		if(!(isspace(datanr.brand_str[i]) && isspace(datanr.brand_str[i - 1])))
			data->tabcpu[VALUE][SPECIFICATION][++j] = datanr.brand_str[i];
	}
	data->tabcpu[VALUE][SPECIFICATION][++j] = '\0';

	/* Cache level 1 (data) */
	if(datanr.l1_data_cache > 0)
	{
		iasprintf(&data->tabcpu[VALUE][LEVEL1D],	"%d x %4d KB", datanr.num_cores, datanr.l1_data_cache);
		iasprintf(&data->tabcpu[VALUE][LEVEL1D],	  "%s, %2d-way", data->tabcpu[VALUE][LEVEL1D], datanr.l1_assoc);
	}

	/* Cache level 1 (instruction) */
	if(datanr.l1_instruction_cache > 0)
	{
		data->l1_size = datanr.l1_instruction_cache;
		iasprintf(&data->tabcpu[VALUE][LEVEL1I],	"%d x %4d KB, %2d-way", datanr.num_cores, datanr.l1_instruction_cache, datanr.l1_assoc);
		iasprintf(&data->tabcache[VALUE][L1SIZE], data->tabcpu[VALUE][LEVEL1I]);
		iasprintf(&data->tabcache[VALUE][L1DESCR], fmt, datanr.l1_assoc, datanr.l1_cacheline);
	}

	/* Cache level 2 */
	if(datanr.l2_cache > 0)
	{
		data->l2_size = datanr.l2_cache;
		iasprintf(&data->tabcpu[VALUE][LEVEL2], "%d x %4d KB, %2d-way", datanr.num_cores, datanr.l2_cache, datanr.l2_assoc);
		iasprintf(&data->tabcache[VALUE][L2SIZE], data->tabcpu[VALUE][LEVEL2]);
		iasprintf(&data->tabcache[VALUE][L2DESCR], fmt, datanr.l2_assoc, datanr.l2_cacheline);
	}

	/* Cache level 3 */
	if(datanr.l3_cache > 0)
	{
		data->l3_size = datanr.l3_cache;
		iasprintf(&data->tabcpu[VALUE][LEVEL3], "%9d KB, %2d-way", datanr.l3_cache, datanr.l3_assoc);
		iasprintf(&data->tabcache[VALUE][L3SIZE], data->tabcpu[VALUE][LEVEL3]);
		iasprintf(&data->tabcache[VALUE][L3DESCR], fmt, datanr.l3_assoc, datanr.l3_cacheline);
	}

	if(datanr.num_cores > 0) /* Avoid divide by 0 */
		iasprintf(&data->tabcpu[VALUE][SOCKETS],	"%d", datanr.total_logical_cpus / datanr.num_logical_cpus);

	/* Fill CPU Intructions label */
	const struct CpuFlags { const cpu_feature_t flag; const char *intrstr; } intructions[] =
	{
		{ CPU_FEATURE_MMX,      "MMX"      },
		{ CPU_FEATURE_MMXEXT,   "(+)"      },
		{ CPU_FEATURE_3DNOW,    ", 3DNOW!" },
		{ CPU_FEATURE_3DNOWEXT, "(+)"      },

		{ CPU_FEATURE_SSE,      ", SSE (1" },
		{ CPU_FEATURE_SSE2,     ", 2"      },
		{ CPU_FEATURE_SSSE3,    ", 3S"     },
		{ CPU_FEATURE_SSE4_1,   ", 4.1"    },
		{ CPU_FEATURE_SSE4_2,   ", 4.2"    },
		{ CPU_FEATURE_SSE4A,    ", 4A"     },
		{ CPU_FEATURE_SSE,      ")"        },

		{ CPU_FEATURE_AES,      ", AES"    },
		{ CPU_FEATURE_AVX,      ", AVX"    },
		{ CPU_FEATURE_VMX,      ", VT-x"   },
		{ CPU_FEATURE_SVM,      ", AMD-V"  },
		{ NUM_CPU_FEATURES,	NULL       }
	};
	for(i = 0; intructions[i].flag != NUM_CPU_FEATURES; i++)
	{
		if(datanr.flags[intructions[i].flag] && data->tabcpu[VALUE][INSTRUCTIONS] == NULL)
			iasprintf(&data->tabcpu[VALUE][INSTRUCTIONS], intructions[i].intrstr);
		else if(datanr.flags[intructions[i].flag])
			iasprintf(&data->tabcpu[VALUE][INSTRUCTIONS], "%s%s", data->tabcpu[VALUE][INSTRUCTIONS], intructions[i].intrstr);
	}

	/* Add string "HT" in CPU Intructions label (if enabled) */
	if(strcmp(data->tabcpu[VALUE][CORES], data->tabcpu[VALUE][THREADS]))
		iasprintf(&data->tabcpu[VALUE][INSTRUCTIONS], "%s, HT", data->tabcpu[VALUE][INSTRUCTIONS]);

	/* Add string "64-bit" in CPU Intructions label (if supported) */
	if(datanr.flags[CPU_FEATURE_LM])
	{
		switch(data->cpu_vendor_id)
		{
			case VENDOR_INTEL:
				iasprintf(&data->tabcpu[VALUE][INSTRUCTIONS], "%s, Intel 64", data->tabcpu[VALUE][INSTRUCTIONS]);
				break;
			case VENDOR_AMD:
				iasprintf(&data->tabcpu[VALUE][INSTRUCTIONS], "%s, AMD64", data->tabcpu[VALUE][INSTRUCTIONS]);
				break;
			default:
				iasprintf(&data->tabcpu[VALUE][INSTRUCTIONS], "%s, 64-bit", data->tabcpu[VALUE][INSTRUCTIONS]);
		}
	}

	return 0;
}

/* Load CPU MSR kernel module */
static bool load_msr_driver(void)
{
	static bool loaded = false;
#ifdef __linux__
	if(!loaded)
	{
		MSG_VERBOSE(_("Loading CPU MSR kernel module"));
		if(getuid())
			MSG_WARNING(_("Load a kernel module as regular user should fail"));

		loaded = !system("modprobe msr 2> /dev/null");
		if(!loaded)
			MSG_ERROR(_("failed to load CPU MSR kernel module"));
	}
#endif /* __linux__ */
	return loaded;
}

/* Dynamic elements provided by libcpuid */
static int call_libcpuid_dynamic(Labels *data)
{
	static bool skip = false;
	int voltage, temp, bclk;
	struct msr_driver_t *msr = NULL;

	/* CPU frequency */
	MSG_VERBOSE(_("Calling libcpuid (retrieve dynamic data)"));
	data->cpu_freq = cpu_clock();
	iasprintf(&data->tabcpu[VALUE][CORESPEED], "%d MHz", data->cpu_freq);

#ifdef HAVE_LIBCPUID_0_2_2
	/* MSR stuff */
	if(skip || (skip = !load_msr_driver()))
		return 1;
	MSG_VERBOSE(_("Opening CPU Model-specific register (MSR)"));
	if(getuid())
		MSG_WARNING(_("Open CPU MSR as regular user should fail"));
	msr = cpu_msr_driver_open_core(data->selected_core);
	if(msr == NULL)
	{
		MSG_ERROR(_("failed to open CPU MSR"));
		skip = true;
		return 1;
	}

	/* Get values from MSR */
	voltage = cpu_msrinfo(msr, INFO_VOLTAGE);
	temp = cpu_msrinfo(msr, INFO_TEMPERATURE);
	bclk = cpu_msrinfo(msr, INFO_BCLK);

	/* CPU Voltage */
	if(voltage != CPU_INVALID_VALUE)
		iasprintf(&data->tabcpu[VALUE][VOLTAGE],     "%.3f V", (double) voltage / 100);

	/* CPU Temperature */
	if(temp != CPU_INVALID_VALUE)
		iasprintf(&data->tabcpu[VALUE][TEMPERATURE], "%i°C", temp);

	/* Base clock */
	if(bclk != CPU_INVALID_VALUE)
	{
		data->bus_freq = (double) bclk / 100;
		iasprintf(&data->tabcpu[VALUE][BUSSPEED],    "%.2f MHz", data->bus_freq);
	}
#endif /* HAVE_LIBCPUID_0_2_2 */

	return 0;
}
#endif /* HAS_LIBCPUID */

#if HAS_DMIDECODE
/* Elements provided by dmidecode (need root privileges) */
static int call_dmidecode(Labels *data)
{
	int i, err = 0;

	MSG_VERBOSE(_("Calling dmidecode"));
	if(getuid())
		MSG_WARNING(_("Call dmidecode as regular user should fail"));
	/* Tab CPU */
	dmidata[PROC_PACKAGE] = &data->tabcpu[VALUE][PACKAGE];
	dmidata[PROC_BUS]     = &data->tabcpu[VALUE][BUSSPEED];
	err += libdmi('c');

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMB; i++)
		dmidata[i]    = &data->tabmb[VALUE][i];
	err += libdmi('m');

	/* Tab RAM */
	for(i = BANK0_0; i < LASTRAM; i++)
		dmidata[i]    = &data->tabram[VALUE][i];
	err += libdmi('r');
	while(data->tabram[VALUE][++data->dimms_count] != NULL);

	if(err)
		MSG_ERROR(_("failed to call dmidecode"));

	return err;
}
#endif /* HAS_DMIDECODE */

/* Alternative function if started as regular user (Linux only) */
static int fallback_mode(Labels *data)
{
#ifdef __linux__
	int i;
	char *file, *buff;
	const char *id[] = { "board_vendor", "board_name", "board_version", "bios_vendor", "bios_version", "bios_date", NULL };

	MSG_VERBOSE(_("Filling labels in fallback mode"));
	/* Tab Motherboard */
	for(i = 0; id[i] != NULL; i++)
	{
		asprintf(&file, "%s/%s", SYS_DMI, id[i]);
		xopen_to_str(file, &buff, 'f');
		iasprintf(&data->tabmb[VALUE][i], buff);
	}
#endif /* __linux__ */
	return 0;
}

/* Get CPU multipliers ("x current (min-max)" label) */
static int cpu_multipliers(Labels *data)
{
	MSG_VERBOSE(_("Getting CPU multipliers"));
#ifdef __linux__
	char *min_freq_str, *max_freq_str;
	char *cpuinfo_min_file, *cpuinfo_max_file;
	double min_freq, max_freq;
	double min_mult, cur_mult, max_mult;

	if(data->cpu_freq <= 0 || data->bus_freq <= 0)
	{
		MSG_ERROR(_("failed to get CPU multipliers"));
		return 1;
	}

	cur_mult = data->cpu_freq / data->bus_freq;
	asprintf(&cpuinfo_min_file, "%s%i/cpufreq/cpuinfo_min_freq", SYS_CPU, data->selected_core);
	asprintf(&cpuinfo_max_file, "%s%i/cpufreq/cpuinfo_max_freq", SYS_CPU, data->selected_core);

	if(xopen_to_str(cpuinfo_min_file, &min_freq_str, 'f') || xopen_to_str(cpuinfo_max_file, &max_freq_str, 'f'))
	{
		asprintf(&data->tabcpu[VALUE][MULTIPLIER], "x %.2f", cur_mult);
		MSG_WARNING(_("Cannot get minimum and maximum CPU multiplierss"));
	}
	else
	{
		min_freq = strtod(min_freq_str, NULL) / 1000;
		max_freq = strtod(max_freq_str, NULL) / 1000;

		min_mult = min_freq / data->bus_freq;
		max_mult = max_freq / data->bus_freq;

		asprintf(&data->tabcpu[VALUE][MULTIPLIER], "x %.0f (%.0f-%.0f)", round(cur_mult), round(min_mult), round(max_mult));
	}
#endif /* __linux__ */
	return 0;
}

/* Calculate CPU usage (total and by core) */
static void cpu_usage(Labels *data)
{
#ifdef __linux__
	enum StatType { USER, NICE, SYSTEM, IDLE, LASTSTAT };
	int i;
	static bool init = false;
	static long double pre[8][LASTSTAT];
	long double        new[8][LASTSTAT], loadavg;
	FILE *fp;

	MSG_VERBOSE(_("Calculating CPU usage"));
	if(!init)
	{
		fp = fopen("/proc/stat", "r");
		for(i = 0; i <= data->cpu_count; i++)
			fscanf(fp,"%*s %Lf %Lf %Lf %Lf %*s %*s %*s %*s %*s %*s", &pre[i][USER], &pre[i][NICE], &pre[i][SYSTEM], &pre[i][IDLE]);
		fclose(fp);
		sleep(1);
		init = true;
	}

	fp = fopen("/proc/stat","r");
	for(i = 0; i <= data->cpu_count; i++)
		fscanf(fp,"%*s %Lf %Lf %Lf %Lf %*s %*s %*s %*s %*s %*s", &new[i][USER], &new[i][NICE], &new[i][SYSTEM], &new[i][IDLE]);
	fclose(fp);

	for(i = 0; i <= data->cpu_count; i++)
	{
		loadavg = ((new[i][USER]+new[i][NICE]+new[i][SYSTEM]) - (pre[i][USER]+pre[i][NICE]+pre[i][SYSTEM])) /
		          ((new[i][USER]+new[i][NICE]+new[i][SYSTEM]+new[i][IDLE]) - (pre[i][USER]+pre[i][NICE]+pre[i][SYSTEM]+pre[i][IDLE]));
		if(i == 0)
			asprintf(&data->tabcpu[VALUE][USAGE], "%6.2Lf %%", loadavg * 100);
		memcpy(pre[i], new[i], 4 * sizeof(long double));
	}
#endif /* __linux__ */
}

#if HAS_LIBPCI
/* Find driver name for a device */
static char *find_driver(struct pci_dev *dev, char *buff)
{
	/* Taken from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/ls-kernel.c */
	int n;
	char name[MAXSTR], *drv, *base;

	MSG_VERBOSE(_("Finding graphic card driver"));
	if(dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
	{
		MSG_ERROR(_("failed to find graphic card driver (access is not by PCI bus)"));
		return NULL;
	}

	base = pci_get_param(dev->access, "sysfs.path");
	if(!base || !base[0])
	{
		MSG_ERROR(_("failed to find graphic card driver (failed to get param)"));
		return NULL;
	}

	n = snprintf(name, sizeof(name), "%s/devices/%04x:%02x:%02x.%d/driver",
		base, dev->domain, dev->bus, dev->dev, dev->func);

	n = readlink(name, buff, MAXSTR);
	if(n < 0)
	{
		MSG_ERROR(_("failed to find graphic card driver (driver name seems to be empty)"));
		return NULL;
	}
	else if(n >= MAXSTR)
		buff[MAXSTR - 1] = '\0';
	else
		buff[n] = '\0';

	if((drv = strrchr(buff, '/')))
		return drv+1;
	else
		return buff;
}

/* Find some PCI devices, like chipset and GPU */
static void find_devices(Labels *data)
{
	/* Adapted from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/example.c */
	int i, nbgpu = 0;
	struct pci_access *pacc;
	struct pci_dev *dev;
	char namebuf[MAXSTR], *vendor, *product, *drivername, *driverstr;
	const char *gpu_vendors[] = { "AMD", "Intel", "NVIDIA", NULL };

	MSG_VERBOSE(_("Finding devices"));
	pacc = pci_alloc(); /* Get the pci_access structure */
	pci_init(pacc);	    /* Initialize the PCI library */
	pci_scan_bus(pacc); /* We want to get the list of devices */

	/* Iterate over all devices */
	for(dev = pacc->devices; dev; dev = dev->next)
	{
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
		asprintf(&vendor,  pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id));
		asprintf(&product, pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id));

		/* Looking for chipset */
		if(dev->device_class == PCI_CLASS_BRIDGE_ISA)
		{
			iasprintf(&data->tabmb[VALUE][CHIPVENDOR], vendor);
			iasprintf(&data->tabmb[VALUE][CHIPNAME],   product);
		}

		/* Looking for GPU */
		if(nbgpu < LASTGPU / GPUFIELDS &&
		  (dev->device_class == PCI_BASE_CLASS_DISPLAY	||
		  dev->device_class == PCI_CLASS_DISPLAY_VGA	||
		  dev->device_class == PCI_CLASS_DISPLAY_XGA	||
		  dev->device_class == PCI_CLASS_DISPLAY_3D	||
		  dev->device_class == PCI_CLASS_DISPLAY_OTHER))
		{
			for(i = 0; gpu_vendors[i] != NULL && strstr(vendor, gpu_vendors[i]) == NULL; i++);
			drivername = find_driver(dev, namebuf);
			iasprintf(&driverstr, _("(%s driver)"), drivername);
			iasprintf(&data->tabgpu[VALUE][GPUVENDOR1	+ nbgpu * GPUFIELDS], "%s %s", (gpu_vendors[i] == NULL) ? vendor : gpu_vendors[i], driverstr);
			iasprintf(&data->tabgpu[VALUE][GPUNAME1		+ nbgpu * GPUFIELDS], "%s", product);
			nbgpu++;
		}
	}

	/* Close everything */
	data->gpu_count = nbgpu;
	pci_cleanup(pacc);
	free(vendor);
	free(product);
	if(driverstr != NULL)
		free(driverstr);
}
#endif /* HAS_LIBPCI */

/* Retrieve GPU temperature */
static int gpu_temperature(Labels *data)
{
	static int err = 0;
	double temp = 0.0;
	char *buff, *drm;

	MSG_VERBOSE(_("Retrieve GPU temperature"));
	asprintf(&drm, "%s%i/device/hwmon/hwmon0/temp1_input", SYS_DRM, 0);
	if(!xopen_to_str("nvidia-settings -q GPUCoreTemp", &buff, 'p') ||
	   !xopen_to_str("aticonfig --odgt | grep Sensor | awk '{ print $5 }'", &buff, 'p'))
		temp = atof(buff);
	else if(!xopen_to_str(drm, &buff, 'f'))
		temp = atof(buff) / 1000.0;

	if(!err && !temp)
	{
		MSG_ERROR(_("failed to retrieve GPU temperature"));
		err++;
		return 1;
	}

	iasprintf(&data->tabgpu[VALUE][GPUTEMP1], "%.2f°C", temp);
	return 0;
}

/* Satic elements for System tab, OS specific */
static int system_static(Labels *data)
{
	int err = 0;
	char *buff = NULL;
	struct utsname name;

	MSG_VERBOSE(_("Identifying system"));
	err = uname(&name);
	if(err)
		MSG_ERROR_ERRNO(_("failed to identify system"));
	else
	{
		iasprintf(&data->tabsys[VALUE][KERNEL],   "%s %s", name.sysname, name.release); /* Kernel label */
		iasprintf(&data->tabsys[VALUE][HOSTNAME], "%s",    name.nodename); /* Hostname label */
	}

	/* Compiler label */
	err += xopen_to_str("cc --version", &buff, 'p');
	iasprintf(&data->tabsys[VALUE][COMPILER], buff);

#ifdef __linux__
	/* Distribution label */
	err += xopen_to_str("grep PRETTY_NAME= /etc/os-release | awk -F '\"|\"' '{print $2}'", &buff, 'p');
	iasprintf(&data->tabsys[VALUE][DISTRIBUTION], buff);
#else
	char tmp[MAXSTR];
	size_t len = sizeof(tmp);

	/* Overwrite Kernel label */
	err += sysctlbyname("kern.osrelease", &tmp, &len, NULL, 0);
	iasprintf(&data->tabsys[VALUE][KERNEL], tmp);

	/* Distribution label */
	err += sysctlbyname("kern.ostype", &tmp, &len, NULL, 0);
	iasprintf(&data->tabsys[VALUE][DISTRIBUTION], tmp);
#endif /* __linux__ */

	return err;
}

/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
static int system_dynamic(Labels *data)
{
	int err = 0;
	time_t uptime_s;
	struct tm *tm;

#if HAS_LIBPROCPS
	const int div = 1000;
	long int total_memory = 0;

	MSG_VERBOSE(_("Calling libprocps"));
	/* System uptime */
	uptime_s = (time_t) uptime(NULL, NULL);

	/* Memory labels */
	meminfo();
	total_memory = kb_main_total / div;
	asprintf(&data->tabsys[VALUE][USED],    "%5ld MB / %5ld MB", kb_main_used    / div, total_memory);
	asprintf(&data->tabsys[VALUE][BUFFERS], "%5ld MB / %5ld MB", kb_main_buffers / div, total_memory);
	asprintf(&data->tabsys[VALUE][CACHED],  "%5ld MB / %5ld MB", kb_main_cached  / div, total_memory);
	asprintf(&data->tabsys[VALUE][FREE],    "%5ld MB / %5ld MB", kb_main_free    / div, total_memory);
	asprintf(&data->tabsys[VALUE][SWAP],    "%5ld MB / %5ld MB", kb_swap_used    / div, kb_swap_total / div);
#endif /* HAS_LIBPROCPS */

#if HAS_LIBSTATGRAB
	static bool called = false;
	const int div = 1000000;
	long int total_memory = 0;
	sg_mem_stats *mem; /* Memory labels */
	sg_swap_stats *swap;
	sg_host_info *info;

	MSG_VERBOSE(_("Calling libstatgrab"));
	/* Libstatgrab initialization */
	if(!called)
	{
		err += sg_init(0);
		called = true;
	}
	mem  = sg_get_mem_stats(NULL);
	swap = sg_get_swap_stats(NULL);
	info = sg_get_host_info(NULL);

	/* System uptime */
	uptime_s = info->uptime;

	/* Memory labels */
	total_memory = mem->total / div;
	asprintf(&data->tabsys[VALUE][USED],    "%5llu MB / %5ld MB",  mem->used   / div, total_memory);
	asprintf(&data->tabsys[VALUE][BUFFERS], "  %3d MB / %5ld MB",  0                , total_memory);
	asprintf(&data->tabsys[VALUE][CACHED],  "%5llu MB / %5ld MB",  mem->cache  / div, total_memory);
	asprintf(&data->tabsys[VALUE][FREE],    "%5llu MB / %5ld MB",  mem->free   / div, total_memory);
	asprintf(&data->tabsys[VALUE][SWAP],    "%5llu MB / %5llu MB", swap->used  / div, swap->total / div);
#endif /* HAS_LIBSTATGRAB */

	/* Uptime label */
	tm = gmtime(&uptime_s);
	asprintf(&data->tabsys[VALUE][UPTIME], _("%i days, %i hours, %i minutes, %i seconds"),
	          tm->tm_yday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return err;
}
