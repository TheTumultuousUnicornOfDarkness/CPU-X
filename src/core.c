/****************************************************************************
*    Copyright © 2014-2017 Xorg
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
# include <sys/sysctl.h>
#endif

#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
# include "databases.h"
#endif

#if HAS_DMIDECODE
# include "dmidecode/libdmi.h"
# include "dmidecode/dmiopt.h"
#endif

#if HAS_BANDWIDTH
# include "bandwidth/libbandwidth.h"
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
	int i, err = 0;
	const uint8_t selected_page = opts->selected_page;

	if(HAS_LIBCPUID)  err += call_libcpuid_static    (data);
	if(HAS_LIBCPUID)  err += call_libcpuid_msr_static(data);
	if(HAS_DMIDECODE) err += call_dmidecode          (data);
	if(HAS_LIBPCI)    err += find_devices            (data);
	casprintf(&data->tab_cpu[VALUE][BUSSPEED], true, "%.2f MHz", data->bus_freq);

	err += system_static       (data);
	err += fallback_mode_static(data);

	/* Call do_refresh() once to get dynamic values */
	for(i = NO_CPU; i < NO_ABOUT; i++)
	{
		opts->selected_page = i;
		err += do_refresh(data);
	}
	opts->selected_page = selected_page;

	return err;
}

/* Refresh some labels */
int do_refresh(Labels *data)
{
	int err = 0;

	switch(opts->selected_page)
	{
		case NO_CPU:
			if(HAS_LIBCPUID) err += err_func(call_libcpuid_dynamic,     data);
			if(HAS_LIBCPUID) err += err_func(call_libcpuid_msr_dynamic, data);
			err += err_func(cpu_usage, data);
			err += fallback_mode_dynamic(data);
			err += err_func(cputab_fill_multipliers, data);
			break;
		case NO_CACHES:
			if(HAS_BANDWIDTH) err += err_func(call_bandwidth, data);
			break;
		case NO_SYSTEM:
			if(HAS_LIBSYSTEM) err += err_func(system_dynamic, data);
			break;
		case NO_GRAPHICS:
			err += err_func(gpu_temperature, data);
			break;
		case NO_BENCH:
			err += err_func(benchmark_status, data);
			break;
	}

	return err;
}


/************************* Private functions *************************/

/* Avoid to re-run a function if an error was occurred in previous call */
static int err_func(int (*func)(Labels *), Labels *data)
{
	unsigned i = 0;
	static unsigned last = 0;
	struct Functions { void *func; int skip_func; } *tmp = NULL;
	static struct Functions *registered = NULL;

	for(i = 0; (i < last) && (func != registered[i].func); i++);

	if(i == last)
	{
		if((tmp = realloc(registered, sizeof(struct Functions) * (i + 1))) == NULL)
		{
			MSG_ERROR(_("could not reallocate memory, exiting %s"), PRGNAME);
			exit(255);
		}
		registered                 = tmp;
		registered[last].func      = func;
		registered[last].skip_func = 0;
		last++;
	}

	if(!registered[i].skip_func)
		registered[i].skip_func = func(data);

	return registered[i].skip_func;
}

#if HAS_LIBCPUID
/* Get CPU technology, in nanometre (nm) */
static int cpu_technology(Labels *data)
{
	int i = -1;
	const Technology_DB *db;
	LibcpuidData *l_data = data->l_data;

	if(l_data->cpu_vendor_id < 0 || l_data->cpu_model < 0 || l_data->cpu_ext_model < 0 || l_data->cpu_ext_family < 0)
		return 1;

	MSG_VERBOSE(_("Finding CPU technology"));
	if(l_data->cpu_vendor_id == VENDOR_INTEL)
		db = technology_intel;
	else if(l_data->cpu_vendor_id == VENDOR_AMD)
		db = technology_amd;
	else
		db = technology_unknown;

	while(db[++i].cpu_model != -1)
	{
		if((db[i].cpu_model == l_data->cpu_model) &&
		  ((db[i].cpu_ext_model  < 0) || (db[i].cpu_ext_model  == l_data->cpu_ext_model)) &&
		  ((db[i].cpu_ext_family < 0) || (db[i].cpu_ext_family == l_data->cpu_ext_family)))
		{
			casprintf(&data->tab_cpu[VALUE][TECHNOLOGY], false, "%i nm", db[i].process);
			return 0;
		}
	}

	MSG_WARNING(_("Your CPU does not belong in database ==> %s, model: %i, ext. model: %i, ext. family: %i"),
	            data->tab_cpu[VALUE][SPECIFICATION], l_data->cpu_model, l_data->cpu_ext_model, l_data->cpu_ext_family);
	return 2;
}

/* Static elements provided by libcpuid */
static int call_libcpuid_static(Labels *data)
{
	int err, i, j = 0;
	char tmp[MAXSTR] = "";
	const char *fmt_cache_kb = _("%d x %d KB, %d-way");
	const char *fmt_cache_mb = _("%d MB, %d-way");
	const char *fmt_lines    = _("%s associative, %d-byte line size");
	struct cpu_raw_data_t raw;
	struct cpu_id_t datanr;

	/* Call libcpuid */
	MSG_VERBOSE(_("Calling libcpuid for retrieving static data"));
	if(data->l_data->cpuid_raw_file == NULL)
		err = cpuid_get_raw_data(&raw);
	else
		err = cpuid_deserialize_raw_data(&raw, data->l_data->cpuid_raw_file);

	if(err || cpu_identify(&raw, &datanr))
	{
		MSG_ERROR(_("failed to call libcpuid"));
		return 1;
	}

	/* Some prerequisites */
	data->cpu_count              = datanr.num_logical_cpus;
	data->l_data->cpu_model      = datanr.model;
	data->l_data->cpu_ext_model  = datanr.ext_model;
	data->l_data->cpu_ext_family = datanr.ext_family;
	if(opts->selected_core >= data->cpu_count)
		opts->selected_core = 0;

	/* Basically fill CPU tab */
	casprintf(&data->tab_cpu[VALUE][CODENAME],      false, "%s",  datanr.cpu_codename);
	casprintf(&data->tab_cpu[VALUE][SPECIFICATION], false, "%s",  datanr.brand_str);
	casprintf(&data->tab_cpu[VALUE][FAMILY],        false, "%#X", datanr.family);
	casprintf(&data->tab_cpu[VALUE][EXTFAMILY],     false, "%#X", datanr.ext_family);
	casprintf(&data->tab_cpu[VALUE][MODEL],         false, "%#X", datanr.model);
	casprintf(&data->tab_cpu[VALUE][EXTMODEL],      false, "%#X", datanr.ext_model);
	casprintf(&data->tab_cpu[VALUE][STEPPING],      false, "%d",  datanr.stepping);
	casprintf(&data->tab_cpu[VALUE][CORES],         true,  "%d",  datanr.num_cores);
	casprintf(&data->tab_cpu[VALUE][THREADS],       true,  "%d",  datanr.num_logical_cpus);

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
	casprintf(&data->tab_cpu[VALUE][VENDOR], false, cpuvendors[i].improved);
	data->l_data->cpu_vendor_id = cpuvendors[i].id;

	/* Search in DB for CPU technology (depends on data->l_data->cpu_vendor_id) */
	err = cpu_technology(data);

	/* Remove training spaces in Specification label */
	for(i = 1; datanr.brand_str[i] != '\0'; i++)
	{
		if(!(isspace(datanr.brand_str[i]) && isspace(datanr.brand_str[i - 1])))
			data->tab_cpu[VALUE][SPECIFICATION][++j] = datanr.brand_str[i];
	}
	data->tab_cpu[VALUE][SPECIFICATION][++j] = '\0';

	/* Cache level 1 (data) */
	if(datanr.l1_data_cache > 0)
		casprintf(&data->tab_cpu[VALUE][LEVEL1D], true, fmt_cache_kb, datanr.num_cores, datanr.l1_data_cache, datanr.l1_assoc);

	/* Cache level 1 (instruction) */
	if(datanr.l1_instruction_cache > 0)
	{
		data->cache_count++;
		data->w_data->size[0] = datanr.l1_instruction_cache;
		casprintf(&data->tab_cpu[VALUE][LEVEL1I], true, fmt_cache_kb, datanr.num_cores, datanr.l1_instruction_cache, datanr.l1_assoc);
		casprintf(&data->tab_caches[VALUE][L1SIZE], true, fmt_lines, data->tab_cpu[VALUE][LEVEL1I], datanr.l1_cacheline);
	}

	/* Cache level 2 */
	if(datanr.l2_cache > 0)
	{
		data->cache_count++;
		data->w_data->size[1] = datanr.l2_cache;
		casprintf(&data->tab_cpu[VALUE][LEVEL2], true, fmt_cache_kb, datanr.num_cores, datanr.l2_cache, datanr.l2_assoc);
		casprintf(&data->tab_caches[VALUE][L2SIZE], true, fmt_lines, data->tab_cpu[VALUE][LEVEL2], datanr.l2_cacheline);
	}

	/* Cache level 3 */
	if(datanr.l3_cache > 0)
	{
		data->cache_count++;
		data->w_data->size[2] = datanr.l3_cache;
		casprintf(&data->tab_cpu[VALUE][LEVEL3], true, fmt_cache_mb, datanr.l3_cache >> 10, datanr.l3_assoc);
		casprintf(&data->tab_caches[VALUE][L3SIZE], true, fmt_lines, data->tab_cpu[VALUE][LEVEL3], datanr.l3_cacheline);
	}

	/* Cache level 4 */
	if(datanr.l4_cache > 0)
	{
		data->cache_count++;
		data->w_data->size[3] = datanr.l4_cache;
		snprintf(tmp, MAXSTR, fmt_cache_mb, datanr.l4_cache >> 10, datanr.l4_assoc);
		casprintf(&data->tab_caches[VALUE][L4SIZE], true, fmt_lines, tmp, datanr.l4_cacheline);
		memset(tmp, 0, MAXSTR);
	}

	if(datanr.total_logical_cpus < datanr.num_logical_cpus)
		casprintf(&data->tab_cpu[VALUE][SOCKETS], true, "%d", 1);
	else if(datanr.num_logical_cpus > 0) /* Avoid divide by 0 */
		casprintf(&data->tab_cpu[VALUE][SOCKETS], true, "%d", datanr.total_logical_cpus / datanr.num_logical_cpus);

	/* Fill CPU Intructions label */
	const struct CpuFlags { const cpu_feature_t flag; const char *intrstr; } intructions[] =
	{
		{ CPU_FEATURE_MMX,      "MMX"      },
		{ CPU_FEATURE_MMXEXT,   "(+)"      },
		{ CPU_FEATURE_3DNOW,    ", 3DNOW!" },
		{ CPU_FEATURE_3DNOWEXT, "(+)"      },

		{ CPU_FEATURE_SSE,      ", SSE (1" },
		{ CPU_FEATURE_SSE2,     ", 2"      },
		{ CPU_FEATURE_PNI,      ", 3"      },
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
			strncat(tmp, intructions[i].intrstr, MAXSTR - strlen(tmp));
	}

	/* Add string "HT" in CPU Intructions label (if enabled) */
	if(datanr.num_cores < datanr.num_logical_cpus)
		strncat(tmp, ", HT", MAXSTR - strlen(tmp));

	/* Add string "64-bit" in CPU Intructions label (if supported) */
	if(datanr.flags[CPU_FEATURE_LM])
	{
		switch(data->l_data->cpu_vendor_id)
		{
			case VENDOR_INTEL:
				strncat(tmp, ", Intel64", MAXSTR - strlen(tmp));
				break;
			case VENDOR_AMD:
				strncat(tmp, ", AMD64",   MAXSTR - strlen(tmp));
				break;
			default:
				strncat(tmp, ", 64-bit",  MAXSTR - strlen(tmp));
		}
	}
	casprintf(&data->tab_cpu[VALUE][INSTRUCTIONS], false, tmp);

	if(opts->output_type == OUT_DUMP && opts->verbose)
		err += cpuid_serialize_raw_data(&raw, "");

	return err;
}

/* Dynamic elements provided by libcpuid */
static int call_libcpuid_dynamic(Labels *data)
{
	/* CPU frequency */
	MSG_VERBOSE(_("Calling libcpuid for retrieving dynamic data"));
	data->cpu_freq = cpu_clock();
	casprintf(&data->tab_cpu[VALUE][CORESPEED], true, "%d MHz", data->cpu_freq);

	return (data->cpu_freq <= 0);
}

/* Try to open a CPU MSR */
static int libcpuid_init_msr(struct msr_driver_t **msr)
{
	if(getuid())
	{
		MSG_WARNING(_("Skip CPU MSR opening (need to be root)"));
		return 1;
	}

	*msr = cpu_msr_driver_open_core(opts->selected_core);
	if(*msr == NULL)
	{
		MSG_ERROR(_("failed to open CPU MSR"));
		return 2;
	}

	return 0;
}

/* MSRs static values provided by libcpuid */
static int call_libcpuid_msr_static(Labels *data)
{
	int min_mult, max_mult, bclk;
	struct msr_driver_t *msr = NULL;

	if(libcpuid_init_msr(&msr))
		return 1;

	MSG_VERBOSE(_("Calling libcpuid for retrieving CPU MSR static values"));
	/* CPU Multipliers (minimum & maximum) */
	min_mult = cpu_msrinfo(msr, INFO_MIN_MULTIPLIER);
	max_mult = cpu_msrinfo(msr, INFO_MAX_MULTIPLIER);
	if(min_mult != CPU_INVALID_VALUE && max_mult != CPU_INVALID_VALUE)
	{
		data->cpu_min_mult = (double) min_mult / 100;
		data->cpu_max_mult = (double) max_mult / 100;
	}

	/* Base clock */
	bclk = cpu_msrinfo(msr, INFO_BCLK);
	if(bclk != CPU_INVALID_VALUE && data->bus_freq == 0.0)
		data->bus_freq = (double) bclk / 100;

	return cpu_msr_driver_close(msr);
}

/* MSRs dynamic values provided by libcpuid */
static int call_libcpuid_msr_dynamic(Labels *data)
{
	int voltage, temp;
	struct msr_driver_t *msr = NULL;

	if(libcpuid_init_msr(&msr))
		return 1;

	MSG_VERBOSE(_("Calling libcpuid for retrieving CPU MSR dynamic values"));
	/* CPU Voltage */
	voltage = cpu_msrinfo(msr, INFO_VOLTAGE);
	if(voltage != CPU_INVALID_VALUE)
		casprintf(&data->tab_cpu[VALUE][VOLTAGE], true, "%.3f V", (double) voltage / 100);

	/* CPU Temperature */
	temp = cpu_msrinfo(msr, INFO_TEMPERATURE);
	if(temp != CPU_INVALID_VALUE)
		casprintf(&data->tab_cpu[VALUE][TEMPERATURE], true, "%i°C", temp);

	return cpu_msr_driver_close(msr);
}
#endif /* HAS_LIBCPUID */

/* Fill the Multiplier label with the most appropriate format */
static int cputab_fill_multipliers(Labels *data)
{
	if(data->cpu_freq <= 0 || data->bus_freq <= 0.0)
		return 1;

	MSG_VERBOSE(_("Calculating CPU multipliers"));
	const int    fmt      = (data->cpu_max_mult < 10) ? 1 : 0;
	const double cur_mult = (double) data->cpu_freq / data->bus_freq;

	if(data->cpu_min_mult <= 0.0 && data->cpu_max_mult <= 0.0)
		casprintf(&data->tab_cpu[VALUE][MULTIPLIER], false, "x %.2f", cur_mult);
	else if(data->cpu_min_mult <= 0.0 && data->cpu_max_mult > 0.0)
		casprintf(&data->tab_cpu[VALUE][MULTIPLIER], false, "x%.1f (?-%.*f)",
		          cur_mult, fmt, data->cpu_max_mult);
	else if(data->cpu_min_mult > 0.0 && data->cpu_max_mult <= 0.0)
		casprintf(&data->tab_cpu[VALUE][MULTIPLIER], false, "x%.1f (%.*f-?)",
		          cur_mult, fmt, data->cpu_min_mult);
	else
		casprintf(&data->tab_cpu[VALUE][MULTIPLIER], false, "x%.1f (%.*f-%.*f)",
		          cur_mult, fmt, data->cpu_min_mult, fmt, data->cpu_max_mult);

	return 0;
}

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
	int i, err;

	/* Dmidecode options */
	opt.type  = NULL;
	opt.flags = FLAG_CPU_X | FLAG_QUIET;

	if(getuid())
	{
		MSG_WARNING(_("Skip call to dmidecode (need to be root)"));
		return 1;
	}

	MSG_VERBOSE(_("Calling dmidecode"));
	opt.type = calloc(256, sizeof(uint8_t));
	if(opt.type == NULL)
	{
		MSG_ERROR(_("failed to allocate memory for dmidecode"));
		return 2;
	}

	/* Tab CPU */
	dmidata[DMI_CPU][0] = &data->tab_cpu[VALUE][PACKAGE];
	ext_clk = &data->bus_freq;
	opt.type[4] = 1;

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
		dmidata[DMI_MB][i] = &data->tab_motherboard[VALUE][i];
	opt.type[0] = 1;
	opt.type[2] = 1;

	/* Tab RAM */
	for(i = BANK0; i < LASTMEMORY; i++)
		dmidata[DMI_RAM][i] = &data->tab_memory[VALUE][i];
	bank = &data->dimm_count;
	opt.type[17] = 1;

	/* Call built-in dmidecode in CPU-X mode */
	if((err = dmidecode()))
		MSG_ERROR(_("failed to call dmidecode"));

	free(opt.type);
	return err;
}
#endif /* HAS_DMIDECODE */

/* Calculate total CPU usage */
static int cpu_usage(Labels *data)
{
	static long *pre = NULL;
	long        *new = NULL;
	double loadavg;
	enum StatType { USER, NICE, SYSTEM, INTR, IDLE, LASTSTAT };

	MSG_VERBOSE(_("Calculating CPU usage"));
	if(pre == NULL)
		pre = calloc(LASTSTAT, sizeof(long));
	new = calloc(LASTSTAT, sizeof(long));

	if(new == NULL || pre == NULL)
		return 1;

#ifdef __linux__
	FILE *fp;

	fp = fopen("/proc/stat","r");
	fscanf(fp,"%*s %li %li %li %li %*s %*s %*s %*s %*s %*s", &new[USER], &new[NICE], &new[SYSTEM], &new[IDLE]);
	fclose(fp);
#else
	size_t len = sizeof(new) * LASTSTAT;

	if(sysctlbyname("kern.cp_time", new, &len, NULL, 0))
		return 1;
#endif /* __linux__ */
	loadavg = (double)((new[USER] + new[NICE] + new[SYSTEM] + new[INTR]) -
	                   (pre[USER] + pre[NICE] + pre[SYSTEM] + pre[INTR])) /
	                  ((new[USER] + new[NICE] + new[SYSTEM] + new[INTR] + new[IDLE]) -
	                   (pre[USER] + pre[NICE] + pre[SYSTEM] + pre[INTR] + pre[IDLE]));

	casprintf(&data->tab_cpu[VALUE][USAGE], false, "%6.2f %%", loadavg * 100);
	memcpy(pre, new, LASTSTAT * sizeof(long));
	free(new);

	return 0;
}

#if HAS_BANDWIDTH
/* Call Bandwidth through CPU-X but do nothing else */
int run_bandwidth(void)
{
	return bandwidth(NULL);
}

/* Compute CPU cache speed */
static int call_bandwidth(Labels *data)
{
	static bool first = true;
	int i, err;
	pthread_t tid;

	if(data->w_data->size[0] < 1)
		return 1;

	MSG_VERBOSE(_("Calling bandwidth"));
	/* Run bandwidth in a separated thread */
	err = pthread_create(&tid, NULL, (void *)bandwidth, data);
	if(first)
	{
		err += pthread_join(tid, NULL);
		first = false;
	}
	else
		err += pthread_detach(tid);

	/* Speed labels */
	for(i = 0; i < LASTCACHES / CACHEFIELDS; i++)
		casprintf(&data->tab_caches[VALUE][i * CACHEFIELDS + L1SPEED], true, "%.2f MB/s", (double) data->w_data->speed[i] / 10);

	return err;
}
#endif /* HAS_BANDWIDTH */

#if HAS_LIBPCI
/* Find driver name for a device */
static int find_driver(struct pci_dev *dev, char *buff)
{
	/* Taken from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/ls-kernel.c */
	int n;
	char name[MAXSTR];
	char *base = NULL, *drv = NULL;

	MSG_VERBOSE(_("Finding graphic card driver"));
	if(dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
		goto error;

	base = pci_get_param(dev->access, "sysfs.path");
	if(base == NULL)
		goto error;

	snprintf(name, MAXSTR, "%s/devices/%04x:%02x:%02x.%d/driver",
		base, dev->domain, dev->bus, dev->dev, dev->func);

	n = readlink(name, buff, MAXSTR);
	if(n <= 0)
		goto error;

	if(n >= MAXSTR)
		n = MAXSTR - 1;
	buff[n] = '\0';

	if((drv = strrchr(buff, '/')))
		strcpy(buff, drv + 1);
	snprintf(name, MAXSTR, _("(%s driver)"), buff);
	strcpy(buff, name);

	return 0;

error:
	MSG_ERROR(_("failed to find graphic card driver"));
	return 1;
}

#define DEVICE_VENDOR  pci_lookup_name(pacc, buff, MAXSTR, PCI_LOOKUP_VENDOR, dev->vendor_id, dev->device_id)
#define DEVICE_PRODUCT pci_lookup_name(pacc, buff, MAXSTR, PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id)
#define PCI_CLASS_DISP (dev->device_class == PCI_BASE_CLASS_DISPLAY) || (PCI_CLASS_DISPLAY_VGA <= dev->device_class && dev->device_class <= PCI_CLASS_DISPLAY_OTHER)
/* Find some PCI devices, like chipset and GPU */
static int find_devices(Labels *data)
{
	/* Adapted from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/example.c */
	bool chipset_found = false;
	int i;
	char buff[MAXSTR] = "";
	struct pci_access *pacc;
	struct pci_dev *dev;
	enum Vendors { CURRENT = 3, LASTVENDOR };
	char *gpu_vendors[LASTVENDOR] = { "AMD", "Intel", "NVIDIA", NULL };

	MSG_VERBOSE(_("Finding devices"));
	pacc = pci_alloc(); /* Get the pci_access structure */
#ifdef __FreeBSD__
	if(access(pci_get_param(pacc, "fbsd.path"), W_OK))
	{
		MSG_WARNING(_("Skip devices search (need to be root)"));
		return 1;
	}
#endif /* __FreeBSD__ */
	pci_init(pacc);	    /* Initialize the PCI library */
	pci_scan_bus(pacc); /* We want to get the list of devices */

	/* Iterate over all devices */
	for(dev = pacc->devices; dev; dev = dev->next)
	{
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);

		/* Looking for chipset */
		if(!chipset_found && dev->device_class == PCI_CLASS_BRIDGE_ISA)
		{
			casprintf(&data->tab_motherboard[VALUE][CHIPVENDOR], false, DEVICE_VENDOR);
			casprintf(&data->tab_motherboard[VALUE][CHIPMODEL],  false, DEVICE_PRODUCT);
			chipset_found = true;
		}

		/* Looking for GPU */
		if((data->gpu_count < LASTGRAPHICS / GPUFIELDS) && (PCI_CLASS_DISP))
		{
			asprintf(&gpu_vendors[CURRENT], DEVICE_VENDOR);
			find_driver(dev, buff);
			for(i = 0; (i < CURRENT) && (strstr(gpu_vendors[CURRENT], gpu_vendors[i]) == NULL); i++);

			casprintf(&data->tab_graphics[VALUE][GPU1VENDOR + data->gpu_count * GPUFIELDS], false, "%s %s", gpu_vendors[i], buff);
			casprintf(&data->tab_graphics[VALUE][GPU1MODEL +  data->gpu_count * GPUFIELDS], false, DEVICE_PRODUCT);
			data->gpu_count++;
			free(gpu_vendors[CURRENT]);
		}
	}

	pci_cleanup(pacc);
	if(!chipset_found)
		MSG_ERROR(_("failed to find chipset vendor and model"));
	if(!data->gpu_count)
		MSG_ERROR(_("failed to find graphic card vendor and model"));

	return !chipset_found + !data->gpu_count;
}
#endif /* HAS_LIBPCI */

/* Retrieve GPU temperature */
static int gpu_temperature(Labels *data)
{
	int ret = -1;
	double temp = 0.0;
	char *buff = NULL;
	DIR *dp = NULL;
	struct dirent *dir;

	MSG_VERBOSE(_("Retrieving GPU temperature"));
	if(!popen_to_str(&buff, "nvidia-settings -q GPUCoreTemp -t") || /* NVIDIA closed source driver */
	   !popen_to_str(&buff, "aticonfig --odgt | grep Sensor | awk '{ print $5 }'")) /* AMD closed source driver */
		temp = atof(buff);
	else /* Open source drivers */
	{
		if((dp = opendir(format("%s%i/device/hwmon/", SYS_DRM, 0))))
		{
			while(((dir = readdir(dp)) != NULL) && (ret))
			{
				if(!(ret = fopen_to_str(&buff, "%s%i/device/hwmon/%s/temp1_input", SYS_DRM, 0, dir->d_name)))
					temp = atof(buff) / 1000.0;
			}
			closedir(dp);
		}
	}
	free(buff);

	if(temp)
	{
		casprintf(&data->tab_graphics[VALUE][GPU1TEMPERATURE], true, "%.2f°C", temp);
		return 0;
	}
	else
	{
		MSG_ERROR(_("failed to retrieve GPU temperature"));
		return 1;
	}
}

/* Satic elements for System tab, OS specific */
static int system_static(Labels *data)
{
	int err = 0;
	struct utsname name;

	MSG_VERBOSE(_("Identifying running system"));
	err = uname(&name);
	if(err)
		MSG_ERROR(_("failed to identify running system"));
	else
	{
		casprintf(&data->tab_system[VALUE][KERNEL],   false, "%s %s", name.sysname, name.release); /* Kernel label */
		casprintf(&data->tab_system[VALUE][HOSTNAME], false, "%s",    name.nodename); /* Hostname label */
	}

	/* Compiler label */
	err += popen_to_str(&data->tab_system[VALUE][COMPILER], "cc --version");

#ifdef __linux__
	/* Distribution label */
	err += popen_to_str(&data->tab_system[VALUE][DISTRIBUTION], "grep PRETTY_NAME= /etc/os-release | awk -F '\"|\"' '{print $2}'");

#else
	char tmp[MAXSTR];
	size_t len = sizeof(tmp);

	/* Overwrite Kernel label */
	err += sysctlbyname("kern.osrelease", &tmp, &len, NULL, 0);
	casprintf(&data->tab_system[VALUE][KERNEL], false, tmp);

	/* Distribution label */
	err += sysctlbyname("kern.ostype", &tmp, &len, NULL, 0);
	casprintf(&data->tab_system[VALUE][DISTRIBUTION], false, tmp);
#endif /* __linux__ */

	return err;
}

/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
static int system_dynamic(Labels *data)
{
	int err = 0, i, j = 0;
	time_t uptime_s = 0;
	struct tm *tm;
	MemoryData *m_data = data->m_data;

#if HAS_LIBPROCPS
	const int div = 1e3;

	MSG_VERBOSE(_("Calling libprocps"));
	/* System uptime */
	uptime_s = (time_t) uptime(NULL, NULL);

	/* Memory variables */
	meminfo();
	m_data->mem_usage[BARUSED]    = kb_main_used    / div;
	m_data->mem_usage[BARBUFFERS] = kb_main_buffers / div;
	m_data->mem_usage[BARCACHED]  = kb_main_cached  / div;
	m_data->mem_usage[BARFREE]    = kb_main_free    / div;
	m_data->mem_usage[BARSWAP]    = kb_swap_used    / div;
	m_data->mem_total             = kb_main_total   / div;
	m_data->swap_total            = kb_swap_total   / div;
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
	m_data->mem_usage[BARUSED]    = mem->used   / div;
	m_data->mem_usage[BARBUFFERS] = 0;
	m_data->mem_usage[BARCACHED]  = mem->cache  / div;
	m_data->mem_usage[BARFREE]    = mem->free   / div;
	m_data->mem_usage[BARSWAP]    = swap->used  / div;
	m_data->mem_total             = mem->total  / div;
	m_data->swap_total            = swap->total / div;
#endif /* HAS_LIBSTATGRAB */
	/* Memory labels */
	for(i = USED; i < SWAP; i++)
		casprintf(&data->tab_system[VALUE][i], false, "%5u MB / %5u MB", m_data->mem_usage[j++], m_data->mem_total);
	casprintf(&data->tab_system[VALUE][SWAP], false, "%5u MB / %5u MB", m_data->mem_usage[j], m_data->swap_total);

	/* Uptime label */
	tm = gmtime(&uptime_s);
	casprintf(&data->tab_system[VALUE][UPTIME], false, _("%i days, %i hours, %i minutes, %i seconds"),
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
static int benchmark_status(Labels *data)
{
	char *buff = NULL;
	BenchData *b_data   = data->b_data;
	enum EnTabBench ind = b_data->fast_mode ? PRIMEFASTSCORE : PRIMESLOWSCORE;

	MSG_VERBOSE(_("Updating benchmark status"));
	asprintf(&data->tab_bench[VALUE][PARAMDURATION], _("%u mins"), data->b_data->duration);
	asprintf(&data->tab_bench[VALUE][PARAMTHREADS],    "%u",       data->b_data->threads);
	asprintf(&data->tab_bench[VALUE][PRIMESLOWRUN],  _("Inactive"));
	asprintf(&data->tab_bench[VALUE][PRIMEFASTRUN],  _("Inactive"));

	if(b_data->primes == 0)
	{
		asprintf(&data->tab_bench[VALUE][PRIMESLOWSCORE], _("Not started"));
		asprintf(&data->tab_bench[VALUE][PRIMEFASTSCORE], _("Not started"));
		return 0;
	}

	if(b_data->run)
		asprintf(&data->tab_bench[VALUE][ind + 1], _("Active"));

	if(b_data->run)
	{
		if(b_data->duration * 60 - b_data->elapsed > 60 * 59)
			asprintf(&buff, _("(%u hours left)"), (b_data->duration - b_data->elapsed / 60) / 60);
		else if(b_data->duration * 60 - b_data->elapsed >= 60)
			asprintf(&buff, _("(%u minutes left)"), b_data->duration - b_data->elapsed / 60);
		else
			asprintf(&buff, _("(%u seconds left)"), b_data->duration * 60 - b_data->elapsed);
	}
	else
	{
		if(b_data->elapsed >= 60 * 60)
			asprintf(&buff, _("in %u hours"),   b_data->elapsed / 60 / 60);
		else if(b_data->elapsed >= 60)
			asprintf(&buff, _("in %u minutes"), b_data->elapsed / 60);
		else
			asprintf(&buff, _("in %u seconds"), b_data->elapsed);
	}

	asprintf(&data->tab_bench[VALUE][ind], "%'u %s", b_data->primes, buff);
	free(buff);
	return 0;
}

/* Perform a multithreaded benchmark (compute prime numbers) */
void start_benchmarks(Labels *data)
{
	int err = 0;
	unsigned i;
	pthread_t *t_id = NULL;
	BenchData *b_data = data->b_data;

	MSG_VERBOSE(_("Starting benchmark"));
	b_data->run     = true;
	b_data->elapsed = 0;
	b_data->num     = 2;
	b_data->primes  = 1;
	b_data->start   = clock();
	t_id            = malloc(sizeof(pthread_t) * b_data->threads);
	if(t_id == NULL)
	{
		MSG_ERROR(_("failed to allocate memory for benchmark"));
		return;
	}

	err += pthread_mutex_init(&b_data->mutex_num,    NULL);
	err += pthread_mutex_init(&b_data->mutex_primes, NULL);

	for(i = 0; i < b_data->threads; i++)
		err += pthread_create(&t_id[i], NULL, primes_bench, data);

	b_data->first_thread = t_id[0];
	free(t_id);

	if(err)
		MSG_ERROR(_("an error occurred while starting benchmark"));
}


/************************* Fallback functions (static) *************************/

#if HAS_LIBCPUID
/* If dmidecode fails to find CPU package, check in database */
static int cputab_package_fallback(Labels *data)
{
	int i = -1;
	const Package_DB *db;
	LibcpuidData *l_data = data->l_data;

	if(l_data->cpu_vendor_id < 0 || data->tab_cpu[VALUE][CODENAME] == NULL || data->tab_cpu[VALUE][SPECIFICATION] == NULL)
		return 1;

	MSG_VERBOSE(_("Finding CPU package in fallback mode"));
	if(l_data->cpu_vendor_id == VENDOR_INTEL)
		db = package_intel;
	else if(l_data->cpu_vendor_id == VENDOR_AMD)
		db = package_amd;
	else
		db = package_unknown;

	while(db[++i].codename != NULL)
	{
		if((strstr(data->tab_cpu[VALUE][CODENAME], db[i].codename) != NULL) &&
		  ((db[i].model == NULL) || (strstr(data->tab_cpu[VALUE][SPECIFICATION], db[i].model) != NULL)))
		{
			casprintf(&data->tab_cpu[VALUE][PACKAGE], false, db[i].socket);
			return 0;
		}
	}

	MSG_WARNING(_("Your CPU socket does not belong in database ==> %s, codename: %s"),
		    data->tab_cpu[VALUE][SPECIFICATION], data->tab_cpu[VALUE][CODENAME]);
	data->tab_cpu[VALUE][PACKAGE][0] = '\0';
	return 2;
}
#endif /* HAS_LIBCPUID */

/* Get minimum and maximum CPU multipliers */
static int cputab_multipliers_fallback(Labels *data)
{
	if(data->bus_freq <= 0)
		return 1;

#ifdef __linux__
	char *min_freq_str = NULL;
	char *max_freq_str = NULL;

	MSG_VERBOSE(_("Calculating CPU multipliers in fallback mode"));
	/* Minimum multiplier */
	if(!fopen_to_str(&min_freq_str, "%s%i/cpufreq/cpuinfo_min_freq", SYS_CPU, opts->selected_core))
		data->cpu_min_mult = round((strtod(min_freq_str, NULL) / 1000) / data->bus_freq);
	free(min_freq_str);

	/* Maximum multiplier */
	if(!fopen_to_str(&max_freq_str, "%s%i/cpufreq/cpuinfo_max_freq", SYS_CPU, opts->selected_core))
		data->cpu_max_mult = round((strtod(max_freq_str, NULL) / 1000) / data->bus_freq);
	free(max_freq_str);
#endif /* __linux__ */

	return 0;
}

/* Retrieve missing Motherboard data if run as regular user */
static int motherboardtab_fallback(Labels *data)
{
	int err = 0;
#ifdef __linux__
	int i;
	const char *id[] = { "board_vendor", "board_name", "board_version", "bios_vendor", "bios_version", "bios_date", NULL };

	MSG_VERBOSE(_("Retrieving motherboard informations in fallback mode"));
	/* Tab Motherboard */
	for(i = 0; id[i] != NULL; i++)
		err += fopen_to_str(&data->tab_motherboard[VALUE][i], "%s/%s", SYS_DMI, id[i]);

#endif /* __linux__ */
	if(err)
		MSG_ERROR(_("failed to retrieve motherboard informations (fallback mode)"));

	return err;
}

/* Check is string is empty (e.g. contains only non printable characters) */
static bool string_is_empty(char *str)
{
	int i;

	if(str == NULL)
		return true;

	for(i = 0; (!isalnum(str[i])) && (str[i] != '\0'); i++);

	return (str[i] == '\0');
}

/* Retrieve static data if other functions failed */
static int fallback_mode_static(Labels *data)
{
	int err = 0;

	if(HAS_LIBCPUID &&
	   (string_is_empty(data->tab_cpu[VALUE][PACKAGE])                  ||
	    strstr(data->tab_cpu[VALUE][PACKAGE], "CPU")            != NULL ||
	    strstr(data->tab_cpu[VALUE][PACKAGE], "Microprocessor") != NULL))
		err += cputab_package_fallback(data);

	if(data->cpu_min_mult <= 0.0 || data->cpu_max_mult <= 0.0)
		err += cputab_multipliers_fallback(data);

	if(string_is_empty(data->tab_motherboard[VALUE][MANUFACTURER]) ||
	   string_is_empty(data->tab_motherboard[VALUE][MBMODEL])      ||
	   string_is_empty(data->tab_motherboard[VALUE][REVISION])     ||
	   string_is_empty(data->tab_motherboard[VALUE][BRAND])        ||
	   string_is_empty(data->tab_motherboard[VALUE][BIOSVERSION])  ||
	   string_is_empty(data->tab_motherboard[VALUE][DATE]))
		err += motherboardtab_fallback(data);

	return err;
}


/************************* Fallback functions (dynamic) *************************/

/* Retrieve CPU temperature if run as regular user */
static int cputab_temp_fallback(Labels *data)
{
	static bool use_sysfs = false;
	double val = 0.0;
	char *buff = NULL;

	MSG_VERBOSE(_("Retrieving CPU temperature in fallback mode"));

	/* First, try by reading 'sensors' output */
	if(!use_sysfs)
	{
		setlocale(LC_ALL, "C");
		if(!popen_to_str(&buff, "sensors | grep -i 'Core[[:space:]]*%u' | awk -F '[+°]' '{ print $2 }'", opts->selected_core))
			val = atof(buff);
		setlocale(LC_ALL, "");

		if(val <= 0)
			use_sysfs = true;
	}

#if HAS_LIBCPUID
	bool module_loaded = false;
	int  file_error = -1;

	/* If 'sensors' is not configured, try by using sysfs */
	if(use_sysfs)
	{
		switch(data->l_data->cpu_vendor_id)
		{
			case VENDOR_INTEL:
				module_loaded = load_module("coretemp");
				file_error    = fopen_to_str(&buff, "%s/temp%i_input", SYS_TEMP_INTEL, opts->selected_core + 1);
				break;
			case VENDOR_AMD:
				module_loaded = load_module("k8temp") | load_module("k10temp");
				file_error    = fopen_to_str(&buff, "%s/temp%i_input", SYS_TEMP_AMD, opts->selected_core + 1);
				break;
			default:
				return 0;
		}

		if(module_loaded && !file_error)
			val = atof(buff) / 1000;
	}
#endif /* HAS_LIBCPUID */
	free(buff);

	if(val > 0)
	{
		casprintf(&data->tab_cpu[VALUE][TEMPERATURE], false, "%.2f°C", val);
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
	char *buff = NULL;

	MSG_VERBOSE(_("Retrieving CPU voltage in fallback mode"));
	setlocale(LC_ALL, "C");
	if(!popen_to_str(&buff, "sensors | grep -i 'VCore' | awk -F '[+V]' '{ print $3 }'"))
		val = atof(buff);
	free(buff);
	setlocale(LC_ALL, "");

	if(val > 0)
	{
		casprintf(&data->tab_cpu[VALUE][VOLTAGE], false, "%.3f V", val);
		return 0;
	}
	else
	{
		MSG_ERROR(_("failed to retrieve CPU voltage (fallback mode)"));
		return 1;
	}
}

/* Retrieve dynamic data if other functions failed */
static int fallback_mode_dynamic(Labels *data)
{
	enum FallbackDynamic { TEMP, VOLT, LASTFALLBACK };
	static bool use_fallback[LASTFALLBACK] = { false };
	int err = 0;

	if(string_is_empty(data->tab_cpu[VALUE][TEMPERATURE]) || use_fallback[TEMP])
	{
		use_fallback[TEMP] = true;
		err += err_func(cputab_temp_fallback, data);
	}

	if(string_is_empty(data->tab_cpu[VALUE][VOLTAGE])     || use_fallback[VOLT])
	{
		use_fallback[VOLT] = true;
		err += err_func(cputab_volt_fallback, data);
	}

	return err;
}
