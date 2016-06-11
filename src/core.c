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
#include <pthread.h>
#include <dirent.h>
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
# include "dmidecode/dmiopt.h"
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
	int err = 0;

	if(HAS_DMIDECODE)    err += call_dmidecode(data);
	if(HAS_LIBCPUID)     err += call_libcpuid_static(data);
	if(HAS_LIBCPUID)     err += call_libcpuid_dynamic(data);
	if(HAS_LIBPROCPS)    err += system_dynamic(data);
	if(HAS_LIBSTATGRAB)  err += system_dynamic(data);
	if(HAS_BANDWIDTH)    err += call_bandwidth(data);
	if(HAS_LIBPCI)       find_devices(data);
	err += gpu_temperature(data);
	err += system_static(data);
	cpu_usage(data, -1);
	benchmark_status(data);
	err += fallback_mode_static(data);
	err += fallback_mode_dynamic(data);

	return err;
}

/* Refresh some labels */
int do_refresh(Labels *data, enum EnTabNumber page)
{
	int err = 0;

	switch(page)
	{
		case NO_CPU:
			if(HAS_LIBCPUID)     err += call_libcpuid_dynamic(data);
			cpu_usage(data, -1);
			err += fallback_mode_dynamic(data);
			break;
		case NO_CACHES:
			if(HAS_BANDWIDTH)    err = call_bandwidth(data);
			break;
		case NO_SYSTEM:
			if(HAS_LIBPROCPS)    err = system_dynamic(data);
			if(HAS_LIBSTATGRAB)  err = system_dynamic(data);
			break;
		case NO_GRAPHICS:
			err = gpu_temperature(data);
			break;
		case NO_BENCH:
			benchmark_status(data);
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
	int i = -1;
	char *msg;
	bool found = false;
	struct Technology { const int32_t cpu_model, cpu_ext_model, cpu_ext_family; const int process; } *vendor = NULL;

	MSG_VERBOSE(_("Finding CPU technology"));
	/* Intel CPUs */
	struct Technology intel[] =
	{
		/* https://raw.githubusercontent.com/anrieff/libcpuid/master/libcpuid/recog_intel.c */
		//Model        E. Model     E. Family   Process
		{  0,           0,          -1,         180 }, // P4 Willamette
		{  1,           1,           6,         350 }, // Pentium Pro
		{  1,           1,          15,         180 }, // P4 Willamette
		{  2,           2,          -1,         130 }, // P4 Northwood / Gallatin
		{  3,           3,           5,         350 }, // PII Overdrive
		{  3,           3,           6,         350 }, // PII Klamath
		{  3,           3,          15,          90 }, // P4 Prescott
		{  4,           4,          -1,          90 }, // P4 Prescott/Irwindale / PD Smithfield
		{  5,           5,           6,         250 }, // PII Deschutes / Tonga / Xeon Drake / Celeron Covington
		{  5,          37,          -1,          32 }, // Westmere
		{  5,          53,          -1,          32 }, // Atom Cloverview
		{  5,          69,          -1,          22 }, // Haswell
		{  6,           6,           6,         250 }, // PII Dixon / Celeron Mendocino
		{  6,           6,          15,          65 }, // P4 Cedar Mill / PD Presler
		{  6,          22,          -1,          65 }, // C2 Conroe-L
		{  6,          54,          -1,          32 }, // Atom Cedarview
		{  7,           7,          -1,         250 }, // PIII Katmai
		{  7,          23,          -1,          45 }, // C2 Wolfdale / Yorkfield / Penryn
		{  7,          55,          -1,          22 }, // Atom Bay Trail
		{  7,          71,          -1,          14 }, // Broadwell
		{  8,           0,           0,         180 }, // PIII Coppermine-T
		{  8,           8,          -1,         180 }, // PIII Coppermine
		{  9,           9,          -1,         130 }, // Pentium M Banias
		{ 10,          26,          -1,          45 }, // Nehalem
		{ 10,          30,          -1,          45 }, // Nehalem
		{ 10,          42,          -1,          32 }, // Sandy Bridge
		{ 10,          58,          -1,          22 }, // Ivy Bridge
		{ 11,          11,          -1,         130 }, // PIII Tualatine
		{ 12,          28,          -1,          45 }, // Atom Diamondville / Pineview / Silverthorne
		{ 12,          44,          -1,          32 }, // Westmere
		{ 12,          60,          -1,          22 }, // Haswell
		{ 12,          76,          -1,          14 }, // Atom Cherry Trail
		{ 13,          13,          -1,          90 }, // Pentium M Dothan
		{ 13,          45,          -1,          32 }, // Sandy Bridge-E
		{ 13,          61,          -1,          14 }, // Broadwell
		{ 14,          14,          -1,          65 }, // Yonah (Core Solo)
		{ 14,          62,          -1,          22 }, // Ivy Bridge-E
		{ 14,          94,          -1,          14 }, // Skylake
		{ 15,          15,          -1,          65 }, // C2 Conroe / Allendale / Kentsfield / Merom
		{ 15,          63,          -1,          22 }, // Haswell-E
		{ -1,          -1,          -1,           0 }
	};

	/* AMD CPUs */
	struct Technology amd[] =
	{
		/* https://raw.githubusercontent.com/anrieff/libcpuid/master/libcpuid/recog_amd.c */
		//Model        E. Model     E. Family   Process
		{  0,           0,          -1,          28 }, // Jaguar (Kabini)
		{  0,          10,          -1,          32 }, // Piledriver (Trinity)
		{  0,          -1,          21,          28 }, // Steamroller (Kaveri)
		{  0,          -1,          22,          28 }, // Puma (Mullins)
		{  1,           1,          -1,          32 }, // K10 (Llano)
		{  1,          60,          -1,          28 }, // Excavator (Carrizo)
		{  1,          -1,          20,          40 }, // Bobcat
		{  2,          -1,          20,          40 }, // Bobcat
		{  3,          13,          -1,          32 }, // Piledriver (Richland)
		{ -1,          -1,          -1,           0 }
	};

	switch(data->cpu_vendor_id)
	{
		case VENDOR_INTEL:
			vendor = intel;
			break;
		case VENDOR_AMD:
			vendor = amd;
			break;
		default:
			goto skip_vendor;
	}

	while(vendor[++i].cpu_model != -1)
	{
		found = vendor[i].cpu_model == data->cpu_model;
		found = found && (vendor[i].cpu_ext_model  < 0 || vendor[i].cpu_ext_model  == data->cpu_ext_model);
		found = found && (vendor[i].cpu_ext_family < 0 || vendor[i].cpu_ext_family == data->cpu_ext_family);

		if(found)
			return vendor[i].process;
	}

skip_vendor:
	asprintf(&msg, _("your CPU does not belong in database\nCPU: %s, model: %i, ext. model: %i, ext. family: %i"),
	         data->tab_cpu[VALUE][SPECIFICATION], data->cpu_model, data->cpu_ext_model, data->cpu_ext_family);
	MSG_ERROR(msg);

	return 0;
}

/* If value is > 9, print both in decimal and hexadecimal */
static void print_hex(char **string, int32_t value)
{
	if(value > 9)
		asprintf(string, "%d (%X)", value, value);
	else
		asprintf(string, "%d", value);
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
	data->cpu_count      = datanr.num_logical_cpus;
	data->cpu_model      = datanr.model;
	data->cpu_ext_model  = datanr.ext_model;
	data->cpu_ext_family = datanr.ext_family;
	if(opts->selected_core >= data->cpu_count)
		opts->selected_core = 0;

	/* Basically fill CPU tab */
	iasprintf(&data->tab_cpu[VALUE][CODENAME],      datanr.cpu_codename);
	iasprintf(&data->tab_cpu[VALUE][TECHNOLOGY],    "%i nm", cpu_technology(data));
	iasprintf(&data->tab_cpu[VALUE][SPECIFICATION], datanr.brand_str);
	print_hex(&data->tab_cpu[VALUE][FAMILY],        datanr.family);
	print_hex(&data->tab_cpu[VALUE][EXTFAMILY],     datanr.ext_family);
	print_hex(&data->tab_cpu[VALUE][MODEL],         datanr.model);
	print_hex(&data->tab_cpu[VALUE][EXTMODEL],      datanr.ext_model);
	print_hex(&data->tab_cpu[VALUE][STEPPING],      datanr.stepping);
	iasprintf(&data->tab_cpu[VALUE][CORES],         "%d", datanr.num_cores);
	iasprintf(&data->tab_cpu[VALUE][THREADS],       "%d", datanr.num_logical_cpus);

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
		{ datanr.vendor_str, datanr.vendor_str,     VENDOR_UNKNOWN   }
	};
	for(i = 0; strcmp(cpuvendors[i].standard, datanr.vendor_str); i++);
	iasprintf(&data->tab_cpu[VALUE][VENDOR], cpuvendors[i].improved);
	data->cpu_vendor_id = cpuvendors[i].id;

	/* Remove training spaces in Specification label */
	for(i = 1; datanr.brand_str[i] != '\0'; i++)
	{
		if(!(isspace(datanr.brand_str[i]) && isspace(datanr.brand_str[i - 1])))
			data->tab_cpu[VALUE][SPECIFICATION][++j] = datanr.brand_str[i];
	}
	data->tab_cpu[VALUE][SPECIFICATION][++j] = '\0';

	/* Cache level 1 (data) */
	if(datanr.l1_data_cache > 0)
	{
		iasprintf(&data->tab_cpu[VALUE][LEVEL1D], "%d x %4d KB", datanr.num_cores, datanr.l1_data_cache);
		iasprintf(&data->tab_cpu[VALUE][LEVEL1D], "%s, %2d-way", data->tab_cpu[VALUE][LEVEL1D], datanr.l1_assoc);
	}

	/* Cache level 1 (instruction) */
	if(datanr.l1_instruction_cache > 0)
	{
		data->w_data->l1_size = datanr.l1_instruction_cache;
		iasprintf(&data->tab_cpu[VALUE][LEVEL1I], "%d x %4d KB, %2d-way", datanr.num_cores, datanr.l1_instruction_cache, datanr.l1_assoc);
		iasprintf(&data->tab_caches[VALUE][L1SIZE], data->tab_cpu[VALUE][LEVEL1I]);
		iasprintf(&data->tab_caches[VALUE][L1DESCRIPTOR], fmt, datanr.l1_assoc, datanr.l1_cacheline);
	}

	/* Cache level 2 */
	if(datanr.l2_cache > 0)
	{
		data->w_data->l2_size = datanr.l2_cache;
		iasprintf(&data->tab_cpu[VALUE][LEVEL2], "%d x %4d KB, %2d-way", datanr.num_cores, datanr.l2_cache, datanr.l2_assoc);
		iasprintf(&data->tab_caches[VALUE][L2SIZE], data->tab_cpu[VALUE][LEVEL2]);
		iasprintf(&data->tab_caches[VALUE][L2DESCRIPTOR], fmt, datanr.l2_assoc, datanr.l2_cacheline);
	}

	/* Cache level 3 */
	if(datanr.l3_cache > 0)
	{
		data->w_data->l3_size = datanr.l3_cache;
		iasprintf(&data->tab_cpu[VALUE][LEVEL3], "%9d KB, %2d-way", datanr.l3_cache, datanr.l3_assoc);
		iasprintf(&data->tab_caches[VALUE][L3SIZE], data->tab_cpu[VALUE][LEVEL3]);
		iasprintf(&data->tab_caches[VALUE][L3DESCRIPTOR], fmt, datanr.l3_assoc, datanr.l3_cacheline);
	}

	if(datanr.num_logical_cpus > 0) /* Avoid divide by 0 */
		iasprintf(&data->tab_cpu[VALUE][SOCKETS], "%d", datanr.total_logical_cpus / datanr.num_logical_cpus);

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
		if(datanr.flags[intructions[i].flag])
			iasprintf(&data->tab_cpu[VALUE][INSTRUCTIONS], "%s%s", data->tab_cpu[VALUE][INSTRUCTIONS], intructions[i].intrstr);
	}

	/* Add string "HT" in CPU Intructions label (if enabled) */
	if(datanr.num_cores < datanr.num_logical_cpus)
		iasprintf(&data->tab_cpu[VALUE][INSTRUCTIONS], "%s, HT", data->tab_cpu[VALUE][INSTRUCTIONS]);

	/* Add string "64-bit" in CPU Intructions label (if supported) */
	if(datanr.flags[CPU_FEATURE_LM])
	{
		switch(data->cpu_vendor_id)
		{
			case VENDOR_INTEL:
				iasprintf(&data->tab_cpu[VALUE][INSTRUCTIONS], "%s, Intel 64", data->tab_cpu[VALUE][INSTRUCTIONS]);
				break;
			case VENDOR_AMD:
				iasprintf(&data->tab_cpu[VALUE][INSTRUCTIONS], "%s, AMD64", data->tab_cpu[VALUE][INSTRUCTIONS]);
				break;
			default:
				iasprintf(&data->tab_cpu[VALUE][INSTRUCTIONS], "%s, 64-bit", data->tab_cpu[VALUE][INSTRUCTIONS]);
		}
	}

	return 0;
}

/* Load CPU MSR kernel module */
static bool load_msr_driver(void)
{
	static bool loaded = false;
#ifdef __linux__
	if(!loaded && !getuid())
	{
		MSG_VERBOSE(_("Loading CPU MSR kernel module"));
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
	/* CPU frequency */
	MSG_VERBOSE(_("Calling libcpuid (retrieve dynamic data)"));
	data->cpu_freq = cpu_clock();
	iasprintf(&data->tab_cpu[VALUE][CORESPEED], "%d MHz", data->cpu_freq);

#ifdef HAVE_LIBCPUID_0_2_2
	int voltage, temp, bclk;
	static bool skip = false;
	static struct msr_driver_t *msr = NULL;

	if(skip)
		return 1;

	if(getuid())
	{
		MSG_WARNING(_("Skip opening of CPU MSR (need to be root)"));
		skip = true;
		return 1;
	}
	skip = !load_msr_driver();

	/* MSR stuff */
	MSG_VERBOSE(_("Opening CPU Model-specific register (MSR)"));
	if(msr == NULL)
		msr = cpu_msr_driver_open_core(opts->selected_core);
	if(msr == NULL)
	{
		MSG_ERROR(_("failed to open CPU MSR"));
		skip = true;
		return 2;
	}

	/* Get values from MSR */
	voltage = cpu_msrinfo(msr, INFO_VOLTAGE);
	temp    = cpu_msrinfo(msr, INFO_TEMPERATURE);
	bclk    = cpu_msrinfo(msr, INFO_BCLK);

	/* CPU Voltage */
	if(voltage != CPU_INVALID_VALUE)
		iasprintf(&data->tab_cpu[VALUE][VOLTAGE],     "%.3f V", (double) voltage / 100);

	/* CPU Temperature */
	if(temp != CPU_INVALID_VALUE)
		iasprintf(&data->tab_cpu[VALUE][TEMPERATURE], "%i°C", temp);

	/* Base clock */
	if(bclk != CPU_INVALID_VALUE)
	{
		data->bus_freq = (double) bclk / 100;
		iasprintf(&data->tab_cpu[VALUE][BUSSPEED],    "%.2f MHz", data->bus_freq);
	}

#ifdef HAVE_LIBCPUID_0_3_0
	int min_mult, max_mult;

	min_mult = cpu_msrinfo(msr, INFO_MIN_MULTIPLIER);
	max_mult = cpu_msrinfo(msr, INFO_MAX_MULTIPLIER);

	/* Multipliers (min-max) */
	if(min_mult != CPU_INVALID_VALUE && max_mult != CPU_INVALID_VALUE)
		iasprintf(&data->tab_cpu[VALUE][MULTIPLIER], "x%.1f (%.0f-%.0f)",
		         data->cpu_freq / data->bus_freq,
			 (double) min_mult / 100, (double) max_mult / 100);
#endif /* HAVE_LIBCPUID_0_3_0 */
#endif /* HAVE_LIBCPUID_0_2_2 */

	return 0;
}
#endif /* HAS_LIBCPUID */

#if HAS_DMIDECODE
/* Call Dmidecode through CPU-X but do nothing else */
int run_dmidecode(void)
{
	opt.type  = NULL;
	opt.flags = (opts->verbose) ? 0 : FLAG_QUIET;
	return dmidecode();
}

/* Elements provided by dmidecode (need root privileges) */
static int call_dmidecode(Labels *data)
{
	int i, err = 0;
	/* Dmidecode options */
	opt.type  = NULL;
	opt.flags = (opts->verbose) ? FLAG_CPU_X : FLAG_CPU_X | FLAG_QUIET;

	if(getuid())
	{
		MSG_WARNING(_("Skip dmidecode (need to be root)"));
		return 1;
	}

	MSG_VERBOSE(_("Calling dmidecode"));
	opt.type = calloc(256, sizeof(uint8_t));
	if(opt.type == NULL)
	{
		MSG_ERROR_ERRNO(_("failed to allocate memory for dmidecode"));
		return 2;
	}

	/* Tab CPU */
	dmidata[PROC_PACKAGE] = &data->tab_cpu[VALUE][PACKAGE];
	dmidata[PROC_BUS]     = &data->tab_cpu[VALUE][BUSSPEED];
	opt.type[4]           = 1;
	err                  += dmidecode();

	/* Tab Motherboard */

	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
		dmidata[i]    = &data->tab_motherboard[VALUE][i];
	opt.type[0]           = 1;
	opt.type[2]           = 1;
	err                  += dmidecode();

	/* Tab RAM */
	for(i = BANK0_0; i < LASTMEMORY; i++)
		dmidata[i]    = &data->tab_memory[VALUE][i];
	opt.type[17]          = 1;
	err                  += dmidecode();

	while(data->tab_memory[VALUE][data->dimms_count] != NULL)
		data->dimms_count++;

	if(err)
		MSG_ERROR(_("failed to call dmidecode"));

	free(opt.type);
	return err;
}
#endif /* HAS_DMIDECODE */

static long **allocate_2d_array(int rows, int columns)
{
	int i;
	long **array = NULL;

	array = calloc(rows, sizeof(long *));
	if(array == NULL)
	{
		MSG_ERROR_ERRNO(_("failed to allocate memory for CPU usage calculation (rows)"));
		return NULL;
	}
	for(i = 0; i < rows; i++)
	{
		array[i] = calloc(columns, sizeof(long));
		if(array[i] == NULL)
		{
			MSG_ERROR_ERRNO(_("failed to allocate memory for CPU usage calculation (columns)"));
			return NULL;
		}
	}

	return array;
}

/* Calculate CPU usage (total if core < 0, else per given core) */
static void cpu_usage(Labels *data, int core)
{
	enum StatType { USER, NICE, SYSTEM, INTR, IDLE, LASTSTAT };
	int i, ind;
	static long **pre = NULL;
	long **new;
	double loadavg;
	FILE *fp;

	MSG_VERBOSE(_("Calculating CPU usage"));
	if(pre == NULL)
		pre = allocate_2d_array(data->cpu_count + 1, LASTSTAT);
	new = allocate_2d_array(data->cpu_count + 1, LASTSTAT);

	if(new == NULL || pre == NULL)
		return;

#ifdef __linux__
	ind = (core < 0) ? 0 : core + 1;
	fp = fopen("/proc/stat","r");
	for(i = 0; i <= data->cpu_count; i++)
		fscanf(fp,"%*s %li %li %li %li %*s %*s %*s %*s %*s %*s", &new[i][USER], &new[i][NICE], &new[i][SYSTEM], &new[i][IDLE]);
	fclose(fp);
#else
	long cp_time[LASTSTAT * 8];
	size_t len = sizeof(cp_time);
	const char *sysctlvarname = (core < 0) ? "kern.cp_time" : "kern.cp_times";

	ind = (core < 0) ? 0 : core;
	if(sysctlbyname(sysctlvarname, &cp_time, &len, NULL, 0))
		return;
	for(i = 0; i <= data->cpu_count * LASTSTAT; i++)
		new[i / LASTSTAT][i % LASTSTAT] = cp_time[i];
#endif /* __linux__ */
	loadavg = (double)((new[ind][USER] + new[ind][NICE] + new[ind][SYSTEM] + new[ind][INTR]) -
	                   (pre[ind][USER] + pre[ind][NICE] + pre[ind][SYSTEM] + pre[ind][INTR])) /
	                  ((new[ind][USER] + new[ind][NICE] + new[ind][SYSTEM] + new[ind][INTR] + new[ind][IDLE]) -
	                   (pre[ind][USER] + pre[ind][NICE] + pre[ind][SYSTEM] + pre[ind][INTR] + pre[ind][IDLE]));

	if(loadavg > 0.0 && ind == 0)
		asprintf(&data->tab_cpu[VALUE][USAGE], "%6.2f %%", loadavg * 100);

	memcpy(pre, new, (data->cpu_count + 1) * LASTSTAT * sizeof(long));
	free(new);
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
		asprintf(&vendor,  "%s", pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id));
		asprintf(&product, "%s", pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id));

		/* Looking for chipset */
		if(dev->device_class == PCI_CLASS_BRIDGE_ISA)
		{
			iasprintf(&data->tab_motherboard[VALUE][CHIPVENDOR], vendor);
			iasprintf(&data->tab_motherboard[VALUE][CHIPMODEL],   product);
		}

		/* Looking for GPU */
		if(nbgpu < LASTGRAPHICS / GPUFIELDS &&
		  (dev->device_class == PCI_BASE_CLASS_DISPLAY	||
		  dev->device_class == PCI_CLASS_DISPLAY_VGA	||
		  dev->device_class == PCI_CLASS_DISPLAY_XGA	||
		  dev->device_class == PCI_CLASS_DISPLAY_3D	||
		  dev->device_class == PCI_CLASS_DISPLAY_OTHER))
		{
			for(i = 0; gpu_vendors[i] != NULL && strstr(vendor, gpu_vendors[i]) == NULL; i++);
			drivername = find_driver(dev, namebuf);
			iasprintf(&driverstr, _("(%s driver)"), drivername);
			iasprintf(&data->tab_graphics[VALUE][GPU1VENDOR	+ nbgpu * GPUFIELDS], "%s %s", (gpu_vendors[i] == NULL) ? vendor : gpu_vendors[i], driverstr);
			iasprintf(&data->tab_graphics[VALUE][GPU1MODEL	+ nbgpu * GPUFIELDS], "%s", product);
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
	char *buff, *drm_hwmon, *drm_temp = NULL;
	DIR *dp = NULL;
	struct dirent *dir;

	MSG_VERBOSE(_("Retrieve GPU temperature"));
	if(!popen_to_str("nvidia-settings -q GPUCoreTemp", &buff) || /* NVIDIA closed source driver */
	   !popen_to_str("aticonfig --odgt | grep Sensor | awk '{ print $5 }'", &buff)) /* AMD closed source driver */
		temp = atof(buff);
	else /* Open source drivers */
	{
		asprintf(&drm_hwmon, "%s%i/device/hwmon/", SYS_DRM, 0);
		dp = opendir(drm_hwmon);
		if(dp)
		{
			while((dir = readdir(dp)) != NULL)
			{
				asprintf(&drm_temp, "%s%i/device/hwmon/%s/temp1_input", SYS_DRM, 0, dir->d_name);
				if(!access(drm_temp, R_OK) && !fopen_to_str(drm_temp, &buff))
				{
					temp = atof(buff) / 1000.0;
					break;
				}
			}
			closedir(dp);
		}
	}

	if(!err && !temp)
	{
		MSG_ERROR(_("failed to retrieve GPU temperature"));
		err++;
		return 1;
	}

	iasprintf(&data->tab_graphics[VALUE][GPU1TEMPERATURE], "%.2f°C", temp);
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
		iasprintf(&data->tab_system[VALUE][KERNEL],   "%s %s", name.sysname, name.release); /* Kernel label */
		iasprintf(&data->tab_system[VALUE][HOSTNAME], "%s",    name.nodename); /* Hostname label */
	}

	/* Compiler label */
	err += popen_to_str("cc --version", &buff);
	iasprintf(&data->tab_system[VALUE][COMPILER], buff);

#ifdef __linux__
	/* Distribution label */
	err += popen_to_str("grep PRETTY_NAME= /etc/os-release | awk -F '\"|\"' '{print $2}'", &buff);
	iasprintf(&data->tab_system[VALUE][DISTRIBUTION], buff);
#else
	char tmp[MAXSTR];
	size_t len = sizeof(tmp);

	/* Overwrite Kernel label */
	err += sysctlbyname("kern.osrelease", &tmp, &len, NULL, 0);
	iasprintf(&data->tab_system[VALUE][KERNEL], tmp);

	/* Distribution label */
	err += sysctlbyname("kern.ostype", &tmp, &len, NULL, 0);
	iasprintf(&data->tab_system[VALUE][DISTRIBUTION], tmp);
#endif /* __linux__ */

	return err;
}

/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
static int system_dynamic(Labels *data)
{
	int err = 0, i;
	time_t uptime_s;
	struct tm *tm;
	enum EnTabSystem ind = USED - USED;
	MemoryData *m_data = data->m_data;

#if HAS_LIBPROCPS
	const int div = 1e3;

	MSG_VERBOSE(_("Calling libprocps"));
	/* System uptime */
	uptime_s = (time_t) uptime(NULL, NULL);

	/* Memory variables */
	meminfo();
	m_data->mem_usage[ind++] = kb_main_used    / div;
	m_data->mem_usage[ind++] = kb_main_buffers / div;
	m_data->mem_usage[ind++] = kb_main_cached  / div;
	m_data->mem_usage[ind++] = kb_main_free    / div;
	m_data->mem_usage[ind]   = kb_swap_used    / div;
	m_data->mem_total        = kb_main_total   / div;
	m_data->swap_total       = kb_swap_total   / div;
#endif /* HAS_LIBPROCPS */

#if HAS_LIBSTATGRAB
	static bool called = false;
	const int div = 1e6;
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

	/* Memory variables */
	m_data->mem_usage[ind++] = mem->used   / div;
	m_data->mem_usage[ind++] = 0;
	m_data->mem_usage[ind++] = mem->cache  / div;
	m_data->mem_usage[ind++] = mem->free   / div;
	m_data->mem_usage[ind]   = swap->used  / div;
	m_data->mem_total        = mem->total  / div;
	m_data->swap_total       = swap->total / div;
#endif /* HAS_LIBSTATGRAB */
	/* Memory labels */
	for(i = USED; i < SWAP; i++)
		asprintf(&data->tab_system[VALUE][i], "%5u MB / %5u MB", m_data->mem_usage[i - USED], m_data->mem_total);
	asprintf(&data->tab_system[VALUE][SWAP], "%5u MB / %5u MB", m_data->mem_usage[USED - SWAP], m_data->swap_total);

	/* Uptime label */
	tm = gmtime(&uptime_s);
	asprintf(&data->tab_system[VALUE][UPTIME], _("%i days, %i hours, %i minutes, %i seconds"),
	          tm->tm_yday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return err;
}

/* Compute all prime numbers in 'duration' seconds */
static void *primes_bench(void *p_data)
{
	uint64_t  i, num, sup;
	Labels    *data   = p_data;
	BenchData *b_data = data->b_data;

	while(b_data->elapsed < b_data->duration * 60 && b_data->run)
	{
		/* b_data->num is shared by all threads */
		pthread_mutex_lock(&b_data->mutex_num);
		b_data->num++;
		num = b_data->num;
		pthread_mutex_unlock(&b_data->mutex_num);

		/* Slow mode: loop from i to num, prime if num == i
		   Fast mode: loop from i to sqrt(num), prime if num mod i != 0 */
		sup = b_data->fast_mode ? sqrt(num) : num;
		for(i = 2; (i < sup) && (num % i != 0); i++);

		if((b_data->fast_mode && num % i) || (!b_data->fast_mode && num == i))
		{
			pthread_mutex_lock(&b_data->mutex_primes);
			b_data->primes++;
			pthread_mutex_unlock(&b_data->mutex_primes);
		}

		/* Only the first thread compute elapsed time */
		if(b_data->first_thread == pthread_self())
			b_data->elapsed = (clock() - b_data->start) / CLOCKS_PER_SEC / b_data->threads;
	}

	if(b_data->first_thread == pthread_self())
	{
		b_data->run = false;
		pthread_mutex_destroy(&b_data->mutex_num);
		pthread_mutex_destroy(&b_data->mutex_primes);
	}

	return NULL;
}

/* Report score of benchmarks */
static void benchmark_status(Labels *data)
{
	char *buff;
	BenchData *b_data   = data->b_data;
	enum EnTabBench ind = b_data->fast_mode ? PRIMEFASTSCORE : PRIMESLOWSCORE;

	asprintf(&data->tab_bench[VALUE][PARAMDURATION], _("%u mins"), data->b_data->duration);
	asprintf(&data->tab_bench[VALUE][PARAMTHREADS],    "%u",       data->b_data->threads);
	asprintf(&data->tab_bench[VALUE][PRIMESLOWRUN],  _("Inactive"));
	asprintf(&data->tab_bench[VALUE][PRIMEFASTRUN],  _("Inactive"));

	if(b_data->primes == 0)
	{
		asprintf(&data->tab_bench[VALUE][PRIMESLOWSCORE], _("Not started"));
		asprintf(&data->tab_bench[VALUE][PRIMEFASTSCORE], _("Not started"));
		return;
	}

	if(b_data->run)
		asprintf(&data->tab_bench[VALUE][ind + 1], _("Active"));

	if(b_data->run && b_data->duration * 60 - b_data->elapsed >= 60)
		asprintf(&buff, _("(%li minutes left)"), b_data->duration - b_data->elapsed / 60);
	else if(b_data->run)
		asprintf(&buff, _("(%li seconds left)"), b_data->duration * 60 - b_data->elapsed);
	else if(!b_data->run && b_data->elapsed >= 60)
		asprintf(&buff, _("in %li minutes"), b_data->elapsed / 60);
	else
		asprintf(&buff, _("in %li seconds"), b_data->elapsed);

	asprintf(&data->tab_bench[VALUE][ind], _("%'u prime numbers calculated %s"), b_data->primes, buff);
}

/* Perform a multithreaded benchmark (compute prime numbers) */
void start_benchmarks(Labels *data)
{
	unsigned i;
	pthread_t *t_id;
	BenchData *b_data = data->b_data;

	b_data->run     = true;
	b_data->elapsed = 0;
	b_data->num     = 2;
	b_data->primes  = 1;
	b_data->start   = clock();
	t_id            = malloc(sizeof(pthread_t) * b_data->threads);

	pthread_mutex_init(&b_data->mutex_num,    NULL);
	pthread_mutex_init(&b_data->mutex_primes, NULL);

	for(i = 0; i < b_data->threads; i++)
		pthread_create(&t_id[i], NULL, primes_bench, data);

	b_data->first_thread = t_id[0];
	free(t_id);
}


/************************* Fallback functions *************************/

/* If dmidecode fails to find CPU package, check in database */
static int cpu_package_fallback(Labels *data)
{
	int i;
	char *msg;

	MSG_VERBOSE(_("Finding CPU package in fallback mode"));
	const struct Package { char *name, *socket; } package[] =
	{
		{ "Pentium D (SmithField)",         "LGA775" },
		{ "Pentium D (Presler)",            "LGA775" },
		{ "Atom (Diamondville)",            "BGA437" },
		{ NULL,                             ""       }
	};

	if(data->tab_cpu[VALUE][CODENAME] == NULL)
		return 1;
	for(i = 0; package[i].name != NULL && strcmp(package[i].name, data->tab_cpu[VALUE][CODENAME]); i++);

	if(package[i].name != NULL)
	{
		iasprintf(&data->tab_cpu[VALUE][PACKAGE], package[i].socket);
		return 0;
	}
	else
	{
		asprintf(&msg, _("your CPU socket does not belong in database\nCPU: %s"), data->tab_cpu[VALUE][SPECIFICATION]);
		MSG_ERROR(msg);
		return 2;
	}
}

/* Retrieve CPU temperature if run as regular user */
static int cputab_temp_fallback(Labels *data)
{
	double val = 0.0;
	char *command, *buff;

	MSG_VERBOSE(_("Retrieve CPU temperature"));
	setlocale(LC_ALL, "C");
	asprintf(&command, "sensors | grep -i 'Core[[:space:]]*%u' | awk -F '[+°]' '{ print $2 }'", opts->selected_core);
	if(!popen_to_str(command, &buff))
		val = atof(buff);
	setlocale(LC_ALL, "");

	if(val > 0)
	{
		iasprintf(&data->tab_cpu[VALUE][TEMPERATURE], "%.2f°C", val);
		return 0;
	}
	else
	{
		MSG_ERROR(_("failed to retrieve CPU temperature (fallback mode)"));
		return 1;
	}
}

/* Retrieve CPU voltage if run as regular user */
static int cputab_volt_fallback(Labels *data)
{
	double val = 0.0;
	char *command, *buff;

	MSG_VERBOSE(_("Retrieve CPU voltage"));
	setlocale(LC_ALL, "C");
	asprintf(&command, "sensors | grep -i 'VCore' | awk -F '[+V]' '{ print $3 }'");
	if(!popen_to_str(command, &buff))
		val = atof(buff);
	setlocale(LC_ALL, "");

	if(val > 0)
	{
		iasprintf(&data->tab_cpu[VALUE][VOLTAGE], "%.3f V", val);
		return 0;
	}
	else
	{
		MSG_ERROR(_("failed to retrieve CPU voltage (fallback mode)"));
		return 1;
	}
}

/* Get CPU multipliers ("x current (min-max)" label) */
static int cpu_multipliers_fallback(Labels *data)
{
	static int err = 0;
	static double min_mult = 0, max_mult = 0;

	if(data->cpu_freq <= 0 || data->bus_freq <= 0 || err > 0)
	{
		err = 2;
		return err;
	}
#ifdef __linux__
	static bool init = false;
	char *min_freq_str, *max_freq_str;
	char *cpuinfo_min_file, *cpuinfo_max_file;
	double min_freq, max_freq;

	MSG_VERBOSE(_("Getting CPU multipliers"));
	if(!init)
	{
		/* Open files */
		asprintf(&cpuinfo_min_file, "%s%i/cpufreq/cpuinfo_min_freq", SYS_CPU, opts->selected_core);
		asprintf(&cpuinfo_max_file, "%s%i/cpufreq/cpuinfo_max_freq", SYS_CPU, opts->selected_core);
		fopen_to_str(cpuinfo_min_file, &min_freq_str);
		fopen_to_str(cpuinfo_max_file, &max_freq_str);

		/* Convert to get min and max values */
		min_freq = strtod(min_freq_str, NULL) / 1000;
		max_freq = strtod(max_freq_str, NULL) / 1000;
		min_mult = round(min_freq / data->bus_freq);
		max_mult = round(max_freq / data->bus_freq);
		init     = true;
	}
#endif /* __linux__ */
	if(min_mult <= 0 || max_mult <= 0)
	{
		asprintf(&data->tab_cpu[VALUE][MULTIPLIER], "x %.2f", data->cpu_freq / data->bus_freq);
		if(!err)
			MSG_WARNING(_("Cannot get minimum and maximum CPU multipliers"));
		err = 1;
	}
	else
		asprintf(&data->tab_cpu[VALUE][MULTIPLIER], "x%.1f (%.0f-%.0f)", data->cpu_freq / data->bus_freq, min_mult, max_mult);

	return err;
}

/* Retrieve missing Motherboard data if run as regular user */
static int motherboardtab_fallback(Labels *data)
{
	int err = 0;
#ifdef __linux__
	int i;
	char *file, *buff;
	const char *id[] = { "board_vendor", "board_name", "board_version", "bios_vendor", "bios_version", "bios_date", NULL };

	MSG_VERBOSE(_("Filling labels in fallback mode"));
	/* Tab Motherboard */
	for(i = 0; id[i] != NULL; i++)
	{
		asprintf(&file, "%s/%s", SYS_DMI, id[i]);
		err += fopen_to_str(file, &buff);
		iasprintf(&data->tab_motherboard[VALUE][i], buff);
	}
#endif /* __linux__ */
	if(err)
		MSG_ERROR(_("an error occurred while retrieving motherboard informations (fallback mode)"));

	return err;
}

/* Retrieve static data if other functions failed */
int fallback_mode_static(Labels *data)
{
	int err = 0;

	if(data->tab_cpu[VALUE][PACKAGE]                           == NULL ||
	   strstr(data->tab_cpu[VALUE][PACKAGE], "CPU")            != NULL ||
	   strstr(data->tab_cpu[VALUE][PACKAGE], "Microprocessor") != NULL)
		err += cpu_package_fallback(data);

	if(data->tab_motherboard[VALUE][MANUFACTURER] == NULL ||
	   data->tab_motherboard[VALUE][MBMODEL]      == NULL ||
	   data->tab_motherboard[VALUE][REVISION]     == NULL ||
	   data->tab_motherboard[VALUE][BRAND]        == NULL ||
	   data->tab_motherboard[VALUE][BIOSVERSION]  == NULL ||
	   data->tab_motherboard[VALUE][DATE]         == NULL)
		err += motherboardtab_fallback(data);

	return err;
}

/* Retrieve dynamic data if other functions failed */
int fallback_mode_dynamic(Labels *data)
{
	int err = 0;

	if(data->tab_cpu[VALUE][TEMPERATURE] == NULL)
		err += cputab_temp_fallback(data);

	if(data->tab_cpu[VALUE][VOLTAGE] == NULL)
		err += cputab_volt_fallback(data);

	if(data->tab_cpu[VALUE][MULTIPLIER] == NULL)
		err += cpu_multipliers_fallback(data);

	return err;
}
