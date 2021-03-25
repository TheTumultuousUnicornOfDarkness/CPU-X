/****************************************************************************
*    Copyright © 2014-2021 Xorg
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
* FILE main.c
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>
#include <getopt.h>
#include <wordexp.h>
#include <locale.h>
#include "cpu-x.h"

#if HAS_GETTEXT
# include <libintl.h>
#endif


#define LOG_FILE "/tmp/cpu-x.log"

Options *opts;


/************************* Arrays management functions *************************/

/* Set labels name */
static void labels_setname(Labels *data)
{
	int i, j;

	MSG_VERBOSE("%s", _("Setting label names"));
	/* CPU tab */
	asprintf(&data->objects[TABCPU],                _("CPU")); // Tab label
	asprintf(&data->objects[FRAMPROCESSOR],         _("Processor")); // Frame label
	asprintf(&data->tab_cpu[NAME][VENDOR],          _("Vendor"));
	asprintf(&data->tab_cpu[NAME][CODENAME],        _("Code Name"));
	asprintf(&data->tab_cpu[NAME][PACKAGE],         _("Package"));
	asprintf(&data->tab_cpu[NAME][TECHNOLOGY],      _("Technology"));
	asprintf(&data->tab_cpu[NAME][VOLTAGE],         _("Voltage"));
	asprintf(&data->tab_cpu[NAME][SPECIFICATION],   _("Specification"));
	asprintf(&data->tab_cpu[NAME][FAMILY],          _("Family"));
	asprintf(&data->tab_cpu[NAME][EXTFAMILY],       _("Ext. Family"));
	asprintf(&data->tab_cpu[NAME][MODEL],           _("Model"));
	asprintf(&data->tab_cpu[NAME][EXTMODEL],        _("Ext. Model"));
	asprintf(&data->tab_cpu[NAME][TEMPERATURE],     _("Temp."));
	asprintf(&data->tab_cpu[NAME][STEPPING],        _("Stepping"));
	asprintf(&data->tab_cpu[NAME][INSTRUCTIONS],    _("Instructions"));

	asprintf(&data->objects[FRAMCLOCKS],            _("Clocks")); // Frame label
	asprintf(&data->tab_cpu[NAME][CORESPEED],       _("Core Speed"));
	asprintf(&data->tab_cpu[NAME][MULTIPLIER],      _("Multiplier"));
	asprintf(&data->tab_cpu[NAME][BUSSPEED],        _("Bus Speed"));
	asprintf(&data->tab_cpu[NAME][USAGE],           _("Usage"));

	asprintf(&data->objects[FRAMCACHE],             _("Cache")); // Frame label
	asprintf(&data->tab_cpu[NAME][LEVEL1D],         _("L1 Data"));
	asprintf(&data->tab_cpu[NAME][LEVEL1I],         _("L1 Inst."));
	asprintf(&data->tab_cpu[NAME][LEVEL2],          _("Level 2"));
	asprintf(&data->tab_cpu[NAME][LEVEL3],          _("Level 3"));

	asprintf(&data->tab_cpu[NAME][SOCKETS],         _("Socket(s)"));
	asprintf(&data->tab_cpu[NAME][CORES],           _("Core(s)"));
	asprintf(&data->tab_cpu[NAME][THREADS],         _("Thread(s)"));

	/* Caches tab */
	asprintf(&data->objects[TABCACHES], _("Caches")); // Tab label
	for(i = L1SIZE; i < LASTCACHES; i += CACHEFIELDS)
	{
		j = i / CACHEFIELDS;
		asprintf(&data->objects[FRAML1CACHE + j],      _("L%i Cache"), j + 1); // Frame label
		asprintf(&data->tab_caches[NAME][L1SIZE  + i], _("Size"));
		asprintf(&data->tab_caches[NAME][L1SPEED + i], _("Speed"));
	}
	asprintf(&data->objects[FRAMTEST], _("Test"));

	/* Motherboard tab */
	asprintf(&data->objects[TABMOTHERBOARD],              _("Motherboard")); // Tab label
	asprintf(&data->objects[FRAMMOTHERBOARD],             _("Motherboard")); // Frame label
	asprintf(&data->tab_motherboard[NAME][MANUFACTURER],  _("Manufacturer"));
	asprintf(&data->tab_motherboard[NAME][MBMODEL],       _("Model"));
	asprintf(&data->tab_motherboard[NAME][REVISION],      _("Revision"));

	asprintf(&data->objects[FRAMBIOS],                    _("BIOS")); // Frame label
	asprintf(&data->tab_motherboard[NAME][BRAND],         _("Brand"));
	asprintf(&data->tab_motherboard[NAME][BIOSVERSION],   _("Version"));
	asprintf(&data->tab_motherboard[NAME][DATE],          _("Date"));
	asprintf(&data->tab_motherboard[NAME][ROMSIZE],       _("ROM Size"));

	asprintf(&data->objects[FRAMCHIPSET],                 _("Chipset")); // Frame label
	asprintf(&data->tab_motherboard[NAME][CHIPVENDOR],    _("Vendor"));
	asprintf(&data->tab_motherboard[NAME][CHIPMODEL],     _("Model"));

	/* Memory tab */
	asprintf(&data->objects[TABMEMORY], _("Memory")); // Tab label
	for(i = BANK0; i < LASTMEMORY; i++)
	{
		asprintf(&data->objects[FRAMBANK0 + i], _("Bank %i"), i);
		asprintf(&data->tab_memory[NAME][i],    _("Reference"));
	}

	/* System tab */
	asprintf(&data->objects[TABSYSTEM],                   _("System")); // Tab label
	asprintf(&data->objects[FRAMOPERATINGSYSTEM],         _("Operating System")); // Frame label
	asprintf(&data->tab_system[NAME][KERNEL],             _("Kernel"));
	asprintf(&data->tab_system[NAME][DISTRIBUTION],       _("Distribution"));
	asprintf(&data->tab_system[NAME][HOSTNAME],           _("Hostname"));
	asprintf(&data->tab_system[NAME][UPTIME],             _("Uptime"));
	asprintf(&data->tab_system[NAME][COMPILER],           _("Compiler"));

	asprintf(&data->objects[FRAMMEMORY],                  _("Memory")); // Frame label
	asprintf(&data->tab_system[NAME][USED],               _("Used"));
	asprintf(&data->tab_system[NAME][BUFFERS],            _("Buffers"));
	asprintf(&data->tab_system[NAME][CACHED],             _("Cached"));
	asprintf(&data->tab_system[NAME][FREE],               _("Free"));
	asprintf(&data->tab_system[NAME][SWAP],               _("Swap"));

	/* Graphics tab */
	asprintf(&data->objects[TABGRAPHICS], _("Graphics")); // Tab label
	for(i = GPU1VENDOR; i < LASTGRAPHICS; i += GPUFIELDS)
	{
		j = i / GPUFIELDS;
		asprintf(&data->objects[FRAMGPU1 + j],                   _("Card %i"), j); // Frame label
		asprintf(&data->tab_graphics[NAME][GPU1VENDOR      + i], _("Vendor"));
		asprintf(&data->tab_graphics[NAME][GPU1DRIVER      + i], _("Driver"));
		asprintf(&data->tab_graphics[NAME][GPU1UMD         + i], _("UMD Version"));
		asprintf(&data->tab_graphics[NAME][GPU1MODEL       + i], _("Model"));
		asprintf(&data->tab_graphics[NAME][GPU1DIDRID      + i], _("DeviceID"));
		asprintf(&data->tab_graphics[NAME][GPU1PCIE        + i], _("Interface"));
		asprintf(&data->tab_graphics[NAME][GPU1TEMPERATURE + i], _("Temperature"));
		asprintf(&data->tab_graphics[NAME][GPU1USAGE       + i], _("Usage"));
		asprintf(&data->tab_graphics[NAME][GPU1VOLTAGE     + i], _("Core Voltage"));
		asprintf(&data->tab_graphics[NAME][GPU1POWERAVG    + i], _("Power Avg"));
		asprintf(&data->tab_graphics[NAME][GPU1CORECLOCK   + i], _("GPU clock"));
		asprintf(&data->tab_graphics[NAME][GPU1MEMCLOCK    + i], _("Memory clock"));
		asprintf(&data->tab_graphics[NAME][GPU1MEMUSED     + i], _("Memory Used"));
	}
	asprintf(&data->objects[FRAMCARDS], _("Cards")); // Frame label

	/* Bench tab */
	asprintf(&data->objects[TABBENCH],              _("Bench")); // Tab label
	asprintf(&data->objects[FRAMPRIMESLOW],         _("Prime numbers (slow)")); // Frame label
	asprintf(&data->objects[FRAMPRIMEFAST],         _("Prime numbers (fast)")); // Frame label
	for(i = PRIMESLOWSCORE; i < PARAMDURATION; i += BENCHFIELDS)
	{
		asprintf(&data->tab_bench[NAME][PRIMESLOWSCORE + i], _("Score"));
		asprintf(&data->tab_bench[NAME][PRIMESLOWRUN   + i], _("Run"));
	}

	asprintf(&data->objects[FRAMPARAM],             _("Parameters")); // Frame label
	asprintf(&data->tab_bench[NAME][PARAMDURATION], _("Duration"));
	asprintf(&data->tab_bench[NAME][PARAMTHREADS],  _("Threads"));

	/* About tab */
	asprintf(&data->objects[TABABOUT],              _("About")); // Tab label
	asprintf(&data->tab_about[DESCRIPTION],         _(
		"%s is a Free software that gathers information\n"
		"on CPU, motherboard and more."), PRGNAME);

	asprintf(&data->objects[FRAMABOUT],             _("About")); // Frame label
	asprintf(&data->tab_about[VERSIONSTR],          _("Version %s"), PRGVER);
	asprintf(&data->tab_about[AUTHOR],              _("Author: %s"), PRGAUTH);
	asprintf(&data->tab_about[SITE],                _("Site: %s"),   PRGURL);

	asprintf(&data->objects[FRAMLICENSE],           _("License")); // Frame label
	asprintf(&data->tab_about[COPYRIGHT],           PRGCPRGHT);
	asprintf(&data->tab_about[LICENSE],             _(
		"This software is distributed under the terms of %s"), PRGLCNS);
	asprintf(&data->tab_about[NOWARRANTY],          _(
		"This program comes with ABSOLUTELY NO WARRANTY"));

	/* Initialize all values */
	Arrays arrays[] =
	{
		{ data->tab_cpu[NAME],         data->tab_cpu[VALUE],         LASTCPU         },
		{ data->tab_caches[NAME],      data->tab_caches[VALUE],      LASTCACHES      },
		{ data->tab_motherboard[NAME], data->tab_motherboard[VALUE], LASTMOTHERBOARD },
		{ data->tab_memory[NAME],      data->tab_memory[VALUE],      LASTMEMORY      },
		{ data->tab_system[NAME],      data->tab_system[VALUE],      LASTSYSTEM      },
		{ data->tab_graphics[NAME],    data->tab_graphics[VALUE],    LASTGRAPHICS    },
		{ data->tab_bench[NAME],       data->tab_bench[VALUE],       LASTBENCH       },
		{ NULL,                        NULL,                         0               }
	};

	for(i = 0; arrays[i].dim_names != NULL; i++)
	{
		for(j = 0; j < arrays[i].last; j++)
			casprintf(&arrays[i].dim_values[j], false, "%c", '\0');
	}
}

/* Dump all data in stdout */
static void dump_data(Labels *data)
{
	int i, j, k = 0;
	const char *col_start = opts->color ? BOLD_BLUE : "";
	const char *col_end   = opts->color ? DEFAULT   : "";
	const Arrays arrays[] =
	{
		{ data->tab_cpu[NAME],         data->tab_cpu[VALUE],         LASTCPU                                 },
		{ data->tab_caches[NAME],      data->tab_caches[VALUE],      data->cache_count * CACHEFIELDS         },
		{ data->tab_motherboard[NAME], data->tab_motherboard[VALUE], LASTMOTHERBOARD                         },
		{ data->tab_memory[NAME],      data->tab_memory[VALUE],      data->dimm_count                        },
		{ data->tab_system[NAME],      data->tab_system[VALUE],      LASTSYSTEM                              },
		{ data->tab_graphics[NAME],    data->tab_graphics[VALUE],    data->gpu_count * GPUFIELDS             },
		{ NULL,                        NULL,                         0                                       }
	};
	const struct { int tab_nb, lab_nb, frame_nb; } frames[] =
	{
		{ NO_CPU,         VENDOR,       FRAMPROCESSOR       },
		{ NO_CPU,         CORESPEED,    FRAMCLOCKS          },
		{ NO_CPU,         LEVEL1D,      FRAMCACHE           },
		{ NO_CPU,         SOCKETS,      -1                  },
		{ NO_CACHES,      L1SIZE,       FRAML1CACHE         },
		{ NO_CACHES,      L2SIZE,       FRAML2CACHE         },
		{ NO_CACHES,      L3SIZE,       FRAML3CACHE         },
		{ NO_CACHES,      L4SIZE,       FRAML4CACHE         },
		{ NO_MOTHERBOARD, MANUFACTURER, FRAMMOTHERBOARD     },
		{ NO_MOTHERBOARD, BRAND,        FRAMBIOS            },
		{ NO_MOTHERBOARD, CHIPVENDOR,   FRAMCHIPSET         },
		{ NO_MEMORY,      BANK0,        FRAMBANK0           },
		{ NO_MEMORY,      BANK1,        FRAMBANK1           },
		{ NO_MEMORY,      BANK2,        FRAMBANK2           },
		{ NO_MEMORY,      BANK3,        FRAMBANK3           },
		{ NO_MEMORY,      BANK4,        FRAMBANK4           },
		{ NO_MEMORY,      BANK5,        FRAMBANK5           },
		{ NO_MEMORY,      BANK6,        FRAMBANK6           },
		{ NO_MEMORY,      BANK7,        FRAMBANK7           },
		{ NO_SYSTEM,      KERNEL,       FRAMOPERATINGSYSTEM },
		{ NO_SYSTEM,      USED,         FRAMMEMORY          },
		{ NO_GRAPHICS,    GPU1VENDOR,   FRAMGPU1            },
		{ NO_GRAPHICS,    GPU2VENDOR,   FRAMGPU2            },
		{ NO_GRAPHICS,    GPU3VENDOR,   FRAMGPU3            },
		{ NO_GRAPHICS,    GPU4VENDOR,   FRAMGPU4            },
		{ NO_GRAPHICS,    GPU5VENDOR,   FRAMGPU5            },
		{ NO_GRAPHICS,    GPU6VENDOR,   FRAMGPU6            },
		{ NO_GRAPHICS,    GPU7VENDOR,   FRAMGPU7            },
		{ NO_GRAPHICS,    GPU8VENDOR,   FRAMGPU8            }
	};

	MSG_VERBOSE("%s", _("Dumping data…"));
	for(i = 0; arrays[i].dim_names != NULL; i++)
	{
		MSG_STDOUT("  %s>>>>>>>>>> %s <<<<<<<<<<%s", col_start, data->objects[i], col_end);
		while(frames[k].tab_nb != i)
			k++;

		for(j = 0; j < arrays[i].last; j++)
		{
			if(frames[k].tab_nb == i && frames[k].lab_nb == j)
			{
				MSG_STDOUT("\n\t%s***** %s *****%s", col_start,
					(frames[k].frame_nb >= 0) ? data->objects[frames[k].frame_nb] : "*", col_end);
				k++;
			}
			MSG_STDOUT("%16s: %s", arrays[i].dim_names[j], arrays[i].dim_values[j]);
		}
		MSG_STDOUT("%s", "\n");
	}

	labels_free(data);
}

/* Free memory after display labels */
void labels_free(Labels *data)
{
	/* Note: data->objects and data->tab_graphics[VALUE] are not freed:
	 * values inside are still read when user change active graphic card  */
	int i, j;
	Arrays arrays[] =
	{
		{ data->tab_cpu[NAME],         data->tab_cpu[VALUE],         LASTCPU                         },
		{ data->tab_caches[NAME],      data->tab_caches[VALUE],      data->cache_count * CACHEFIELDS },
		{ data->w_data->test_name,     NULL,                         data->w_data->test_count        },
		{ data->tab_motherboard[NAME], data->tab_motherboard[VALUE], LASTMOTHERBOARD                 },
		{ data->tab_memory[NAME],      data->tab_memory[VALUE],      data->dimm_count                },
		{ data->tab_system[NAME],      data->tab_system[VALUE],      LASTSYSTEM                      },
		{ data->tab_graphics[NAME],    NULL,                         data->gpu_count * GPUFIELDS     },
		{ data->tab_bench[NAME],       data->tab_bench[VALUE],       LASTBENCH                       },
		{ data->tab_about,             NULL,                         LASTABOUT                       },
		{ NULL,                        NULL,                         0                               }
	};

	MSG_VERBOSE("%s", _("Freeing memory"));
	for(i = 0; arrays[i].dim_names != NULL; i++)
	{
		for(j = 0; j < arrays[i].last; j++)
		{
			free(arrays[i].dim_names[j]);
			arrays[i].dim_names[j] = NULL;
			if(arrays[i].dim_values != NULL)
			{
				free(arrays[i].dim_values[j]);
				arrays[i].dim_values[j] = NULL;
			}
		}
	}
}


/************************* Options-related functions *************************/

static const struct
{
	const bool has_mod;
	const char short_opt;
	const char *long_opt;
	const int  need_arg;
	char       *description;
} cpux_options[] =
{
	{ HAS_GTK,         'G', "gtk",           no_argument,       N_("Start graphical user interface (GUI) (default)")           },
	{ HAS_NCURSES,     'N', "ncurses",       no_argument,       N_("Start text-based user interface (TUI)")                    },
	{ true,            'D', "dump",          no_argument,       N_("Dump all data on standard output and exit")                },
	{ HAS_DMIDECODE,   'M', "dmidecode",     no_argument,       N_("Run embedded command dmidecode and exit")                  },
	{ HAS_BANDWIDTH,   'B', "bandwidth",     no_argument,       N_("Run embedded command bandwidth and exit")                  },
	{ true,            'r', "refresh",       required_argument, N_("Set custom time between two refreshes (in seconds)")       },
	{ true,            't', "tab",           required_argument, N_("Set default tab (integer)")                                },
	{ HAS_LIBCPUID,    'c', "core",          required_argument, N_("Select CPU core to monitor (integer)")                     },
	{ HAS_BANDWIDTH,   'b', "cachetest",     required_argument, N_("Set custom bandwidth test for CPU caches speed (integer)") },
	{ HAS_LIBPCI,      'g', "gpu",           required_argument, N_("Select default graphic card (integer)")                    },
	{ true,            'd', "daemon",        no_argument,       N_("Start and connect to daemon")                              },
	{ true,            'v', "verbose",       no_argument,       N_("Verbose output")                                           },
	{ true,            'h', "help",          no_argument,       N_("Print help and exit")                                      },
	{ true,            'V', "version",       no_argument,       N_("Print version and exit")                                   },
	{ HAS_LIBCPUID,      0, "cpuid-decimal", no_argument,       N_("Print CPUID values in decimal (default is hexadeximal)")   },
	{ true,              0, "nocolor",       no_argument,       N_("Disable colored output")                                   },
	{ true,              0, "debug",         no_argument,       N_("Print information for debugging")                          },
	{ true,              0, "issue-fmt",     no_argument,       N_("Print required information to paste in an issue")          },
	{ HAS_NCURSES,       0, "keymap",        required_argument, N_("Set key mapping for NCurses mode (a[rrows]|e[macs]|i[nverted-T]|v[im])") },
	{ true,              0, NULL,            0,                 NULL                                                           }
};

static const struct
{
	const bool has_mod;
	const char *var_name;
	char       *description;
} cpux_env_vars[] =
{
	{ true,            "CPUX_ARGS",                N_("Add default command line arguments")                      },
	{ true,            "CPUX_BCLK",                N_("Enforce the bus clock")                                   },
	{ true,            "CPUX_FORCE_FREQ_FALLBACK", N_("Ignore CPU frequency reported by libcpuid") },
	{ HAS_LIBCPUID,    "CPUX_CPUID_RAW",           N_("Read CPUID raw data from a given file")                   },
	{ HAS_LIBCPUID,    "CPUX_DEBUG_DATABASE",      N_("Only print a message if CPU is not present in the database") },
	{ true,            NULL,                       NULL                                                          }
};

/* Check for influenceable environment variables */
static void check_environment_variables(Labels *data)
{
	if(getenv("CPUX_BCLK"))
		data->bus_freq = atof(getenv("CPUX_BCLK"));
	if(getenv("CPUX_FORCE_FREQ_FALLBACK"))
		opts->freq_fallback = ((atoi(getenv("CPUX_FORCE_FREQ_FALLBACK"))) > 0);
	if(getenv("CPUX_CPUID_RAW"))
		data->l_data->cpuid_raw_file = getenv("CPUX_CPUID_RAW");
	if(getenv("CPUX_DEBUG_DATABASE"))
		opts->debug_database = ((atoi(getenv("CPUX_DEBUG_DATABASE"))) > 0);
}

/* This is help display with --help option */
static void help(char *binary_name)
{
	int i;
	bool options_header = false;
	char buff[MAXSTR];

	MSG_STDOUT(_("Usage: %s DISPLAY [OPTIONS]\n"), binary_name);
	MSG_STDOUT("%s", _("Available DISPLAY:"));
	for(i = 0; cpux_options[i].long_opt != NULL; i++)
	{
		if(!cpux_options[i].has_mod)
			continue;

		if(!options_header && islower(cpux_options[i].short_opt))
		{
			options_header = true;
			MSG_STDOUT("\n%s", _("Available OPTIONS:"));
		}

		if(cpux_options[i].short_opt) snprintf(buff, MAXSTR, "  -%c,", cpux_options[i].short_opt);
		else                          snprintf(buff, MAXSTR, "     ");
		MSG_STDOUT("%s --%-14s %s", buff, cpux_options[i].long_opt, _(cpux_options[i].description));
	}

	MSG_STDOUT("\n%s", _("Influenceable environment variables:"));
	for(i = 0; cpux_env_vars[i].var_name != NULL; i++)
	{
		if(!cpux_env_vars[i].has_mod)
			continue;
		MSG_STDOUT("  %-25s %s", cpux_env_vars[i].var_name, _(cpux_env_vars[i].description));
	}
}

/* This is the --version option */
static void version(bool full_header)
{
	int i;
	const struct { const bool has_mod; const char *lib, *version; } libs_ver[] =
	{
		{ HAS_GTK,         "GTK",         GTK_VERSION         },
		{ HAS_NCURSES,     "NCURSES",     NCURSES_VERSION     },
		{ HAS_LIBCPUID,    "LIBCPUID",    LIBCPUID_VERSION    },
		{ HAS_LIBPCI,      "LIBPCI",      LIBPCI_VERSION      },
		{ HAS_LIBGLFW,     "LIBGLFW",     LIBGLFW_VERSION     },
		{ HAS_LIBPROCPS,   "LIBPROCPS",   LIBPROCPS_VERSION   },
		{ HAS_LIBSTATGRAB, "LIBSTATGRAB", LIBSTATGRAB_VERSION },
		{ HAS_DMIDECODE,   "DMIDECODE",   DMIDECODE_VERSION   },
		{ HAS_BANDWIDTH,   "BANDWIDTH",   BANDWIDTH_VERSION   },
		{ false,           NULL,          NULL                }
	};

	PRGINFO(stdout);
	if(full_header)
	{
		MSG_STDOUT("%s\n", PRGCPRGHT);
		MSG_STDOUT("%s", _("This is free software: you are free to change and redistribute it."));
		MSG_STDOUT("%s", _("This program comes with ABSOLUTELY NO WARRANTY"));
		MSG_STDOUT(_("See the %s license: <%s>\n"), PRGLCNS, LCNSURL);
	}

	/* Print features version */
	for(i = 0; libs_ver[i].lib != NULL; i++)
	{
		if(libs_ver[i].has_mod)
			MSG_STDOUT(_("-- %-9s version: %s"), libs_ver[i].lib, libs_ver[i].version);
	}
}

/* Add arguments from environment variable CPUX_ARGS */
static void environment_to_arguments(int *argc, char ***argv)
{
	int err = 0;
	const char *args = getenv("CPUX_ARGS");
	wordexp_t we;

	if(args == NULL)
		return;

	if((err = wordexp(args, &we, 0)) != 0)
	{
		MSG_ERROR(_("failed to call wordexp (%i)"), err);
		return;
	}

	*argc = we.we_wordc;
	*argv = malloc((we.we_wordc + 1) * sizeof(char *));
	for(size_t i = 0; i < we.we_wordc; i++)
	{
		const size_t length = strlen(we.we_wordv[i]) + 1;
		(*argv)[i + 1] = malloc(length * sizeof(char));
		memcpy((*argv)[i + 1], we.we_wordv[i], length);
	}
	wordfree(&we);
}

/* Parse arguments and set some flags */
#define OPTIONS_COUNT   (sizeof(cpux_options) / sizeof(cpux_options[0]))
#define SHORT_OPT_SIZE  3
#define SHORT_OPTS_SIZE (OPTIONS_COUNT * 2)
static void parse_arguments(int argc_orig, char *argv_orig[])
{
	int i, j = 0, c, argc = 0, longindex, tmp_arg = -1;
	char shortopt[SHORT_OPT_SIZE], shortopts[SHORT_OPTS_SIZE] = "";
	char **argv = NULL, **tmp = NULL;
	struct option longopts[OPTIONS_COUNT];

	/* Inject arguments from environment */
	environment_to_arguments(&argc, &argv);
	c     = argc;
	argc += argc_orig;
	tmp   = realloc(argv, argc * sizeof(char *));
	ALLOC_CHECK(tmp);
	argv  = tmp;
	for(i = 0; i < argc_orig; i++)
	{
		j = (i == 0) ? 0 : c + i; // Preserve argv[0], then append original argv at end of new array
		const size_t length = strlen(argv_orig[i]) + 1;
		argv[j] = malloc(length * sizeof(char));
		memcpy(argv[j], argv_orig[i], length);
	}

	/* Fill longopts structure */
	j = 0;
	for(i = 0; cpux_options[i].long_opt != NULL; i++)
	{
		while(!cpux_options[i].has_mod)
			i++;
		longopts[j++] = (struct option) { .name = cpux_options[i].long_opt, .has_arg = cpux_options[i].need_arg, .flag = 0, .val = cpux_options[i].short_opt };
		snprintf(shortopt, SHORT_OPT_SIZE, "%c%c", cpux_options[i].short_opt, cpux_options[i].need_arg ? ':' : '\0');
		strncat(shortopts, shortopt, SHORT_OPTS_SIZE - 1);
	}
	longopts[j] = (struct option) { 0, 0, 0, 0 };

	/* Set the default mode */
	if(HAS_GTK && (getenv("DISPLAY") != NULL || getenv("WAYLAND_DISPLAY") != NULL))
		opts->output_type = OUT_GTK;
	else if(HAS_NCURSES)
		opts->output_type = OUT_NCURSES;
	else
		opts->output_type = OUT_DUMP;

	/* Parse options */
	while((c = getopt_long(argc, argv, shortopts, longopts, &longindex)) != -1)
	{
		switch(c)
		{
			case 'G':
				opts->output_type = OUT_GTK;
				break;
			case 'N':
				opts->output_type = OUT_NCURSES;
				break;
			case 'D':
				opts->output_type = OUT_DUMP;
				break;
			case 'M':
				opts->output_type = OUT_DMIDECODE;
				break;
			case 'B':
				opts->output_type = OUT_BANDWIDTH;
				break;
			case 'r':
				tmp_arg = atoi(optarg);
				if(tmp_arg >= 1)
					opts->refr_time = tmp_arg;
				break;
			case 't':
				tmp_arg = atoi(optarg);
				if(NO_CPU < tmp_arg && tmp_arg <= NO_ABOUT)
					opts->selected_page = tmp_arg;
				break;
			case 'c':
				tmp_arg = atoi(optarg);
				if(tmp_arg >= 0)
					opts->selected_core = tmp_arg;
				break;
			case 'b':
				tmp_arg = atoi(optarg);
				if(tmp_arg >= 0)
					opts->bw_test = atoi(optarg);
				break;
			case 'g':
				tmp_arg = atoi(optarg);
				if(tmp_arg >= 0)
					opts->selected_gpu = tmp_arg;
				break;
			case 'd':
				opts->with_daemon = true;
				break;
			case 'v':
				opts->verbose = true;
				break;
			case 'h':
				help(argv[0]);
				exit(EXIT_SUCCESS);
				break;
			case 'V':
				version(true);
				exit(EXIT_SUCCESS);
				break;
			case 0:
				if(!strcmp(longopts[longindex].name, "cpuid-decimal"))
				{
					opts->cpuid_decimal = true;
					break;
				}
				else if(!strcmp(longopts[longindex].name, "nocolor"))
				{
					opts->color = false;
					break;
				}
				else if(!strcmp(longopts[longindex].name, "debug"))
				{
					opts->debug = true;
					break;
				}
				else if(!strcmp(longopts[longindex].name, "issue-fmt"))
				{
					opts->color       = false;
					opts->verbose     = true;
					opts->debug       = true;
					opts->issue       = true;
					opts->output_type = OUT_DUMP;
					setlocale(LC_ALL, "C");
					setenv("CPUX_DAEMON_DEBUG", "1", 1);
					unlink(LOG_FILE);
					freopen(LOG_FILE, "a", stdout);
					setvbuf(stdout, NULL, _IONBF, 0);
					dup2(STDOUT_FILENO, STDERR_FILENO);
					version(false);
					break;
				}
				else if(!strcmp(longopts[longindex].name, "keymap"))
				{
					switch(optarg[0])
					{
						case 'a': opts->keymap = ARROWS;     break;
						case 'e': opts->keymap = EMACS;      break;
						case 'i': opts->keymap = INVERTED_T; break;
						case 'v': opts->keymap = VIM;        break;
						default: help(argv[0]); exit(EXIT_FAILURE);
					}
					break;
				}
				/* Fall through */
			case '?':
			default:
				help(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	for(i = 0; i < argc; i++)
		free(argv[i]);
	free(argv);
}
#undef OPTIONS_COUNT
#undef SHORT_OPT_SIZE
#undef SHORT_OPTS_SIZE


/************************* Main-related functions *************************/

/* Action on SIGSEV/SIGFPE */
static void sighandler(int signum)
{
	int bt_size, i;
	char **bt_syms, *buff = NULL;
	void *bt[16];

	/* Get the backtrace */
	bt_size = backtrace(bt, 16);
	bt_syms = backtrace_symbols(bt, bt_size);

	/* Print the backtrace */
	MSG_STDERR(_("\n%sOops, something was wrong! %s has received signal %d (%s) and has crashed.%s"),
	           BOLD_RED, PRGNAME, signum, strsignal(signum), DEFAULT);
	MSG_STDERR("%s", "========================= Backtrace =========================");
	PRGINFO(stderr);
	for(i = 1; i < bt_size; i++)
	{
		char *address = strtok(strrchr(strdup(bt_syms[i]), '[') + 1, "]");
		popen_to_str(&buff, "addr2line %s -e /proc/%d/exe", address, getpid());
		if(strstr(buff, "??") == NULL)
			MSG_STDERR("#%2i %s %s", i, strrchr(buff, '/') + 1, bt_syms[i]);
		else
			MSG_STDERR("#%2i %s", i, bt_syms[i]);
		free(buff);
	}
	MSG_STDERR("%s", "======================== End Backtrace =======================\n");
	MSG_STDERR("%s", _("You can paste this backtrace by opening a new issue here:"));
	MSG_STDERR("%s", "https://github.com/X0rg/CPU-X/issues/new\n");

	/* Stop program */
	free(bt_syms);
	signal(signum, SIG_DFL);
	kill(getpid(), signum);
}

 /* Enable internationalization support */
#if HAS_GETTEXT
static int set_locales(void)
{
	int err;

	char *TEXTDOMAINDIR = getenv("TEXTDOMAINDIR");
	if(TEXTDOMAINDIR == NULL || TEXTDOMAINDIR[0] == '\0')
		TEXTDOMAINDIR = LOCALEDIR;

	/* Apply locale */
	setlocale(LC_ALL, "");
	err  = bindtextdomain(GETTEXT_PACKAGE, TEXTDOMAINDIR)    ? 0 : 1;
	err += bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8") ? 0 : 1;
	err += textdomain(GETTEXT_PACKAGE)                       ? 0 : 1;

	/* Check if something is wrong */
	if(err)
	{
		MSG_ERROR("%s", _("an error occurred while setting locale"));
		return 1;
	}
	else
		return 0;
}
#endif /* HAS_GETTEXT */

int main(int argc, char *argv[])
{
	/* Init variables */
	Labels *data = &(Labels)
	{
		.tab_cpu         = { { NULL } },
		.tab_caches      = { { NULL } },
		.tab_motherboard = { { NULL } },
		.tab_memory      = { { NULL } },
		.tab_system      = { { NULL } },
		.tab_graphics    = { { NULL } },
		.tab_bench       = { { NULL } },
		.cpu_freq        = 0,
		.socket_fd       = -1,
		.cpu_count       = 0,
		.cache_count     = 0,
		.dimm_count      = 0,
		.gpu_count       = 0,
		.bus_freq        = 0.0,
		.cpu_min_mult    = 0.0,
		.cpu_max_mult    = 0.0,
		.reload          = false
	};
	data->l_data = &(LibcpuidData)
	{
		.cpu_vendor_id  = -1,
		.cpu_model      = -1,
		.cpu_ext_model  = -1,
		.cpu_ext_family = -1,
		.cpuid_raw_file = NULL
	};
	data->w_data = &(BandwidthData)
	{
		.test_count = 0,
		.size       = { 0 },
		.speed      = { 0 },
		.test_name  = NULL
	};
	data->m_data = &(MemoryData)
	{
		.mem_usage  = { 0 },
		.mem_total  = 0,
		.swap_total = 0
	};
	data->g_data = &(GraphicsData)
	{
		.gpu_driver  = { GPUDRV_UNKNOWN },
		.device_path = { NULL },
	};
	data->b_data = &(BenchData)
	{
		.run      = false,
		.duration = 1,
		.threads  = 1,
		.primes   = 0,
		.start    = 0,
		.elapsed  = 0,
		.num      = 0
	};
	opts = &(Options)
	{
		.cpuid_decimal  = false,
		.color          = true,
		.verbose        = false,
		.debug          = false,
		.issue          = false,
		.with_daemon    = false,
		.debug_database = false,
		.freq_fallback  = false,
		.selected_page  = 0,
		.selected_core  = 0,
		.bw_test        = 0,
		.selected_gpu   = 0,
		.refr_time      = 1,
		.keymap         = ARROWS
	};

#if HAS_GETTEXT
	set_locales();
#endif /* HAS_GETTEXT */
	signal(SIGSEGV, sighandler);
	signal(SIGFPE,  sighandler);
	signal(SIGABRT, sighandler);

	/* Parse options */
#if HAS_GTK
	load_settings();
#endif /* HAS_GTK */
	parse_arguments(argc, argv);
	check_environment_variables(data);
	if(opts->output_type > OUT_NO_CPUX)
		goto skip_init;

	/* Connect to daemon */
	if(IS_ROOT || opts->with_daemon)
		start_daemon(false);
	if(daemon_is_alive())
		connect_to_daemon(data);

	/* Retrieve data */
	labels_setname(data);
	fill_labels   (data);

	/* Show data */
	if(HAS_GTK && (opts->output_type == OUT_GTK))
		start_gui_gtk(&argc, &argv, data);
	if(HAS_NCURSES && (opts->output_type == OUT_NCURSES))
		start_tui_ncurses(data);
	if(opts->output_type == OUT_DUMP)
		dump_data(data);
skip_init:
	if(HAS_DMIDECODE && (opts->output_type == OUT_DMIDECODE))
		return run_dmidecode();
	if(HAS_BANDWIDTH && (opts->output_type == OUT_BANDWIDTH))
		return run_bandwidth();

	return EXIT_SUCCESS;
}
