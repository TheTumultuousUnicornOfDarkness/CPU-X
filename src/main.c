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
* FILE main.c
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <execinfo.h>
#include <getopt.h>
#include <locale.h>
#include <libintl.h>
#include "cpu-x.h"

#if PORTABLE_BINARY
# include <sys/stat.h>
# if HAS_GETTEXT
#  include "../po/mo.h"
# endif
#endif

#if (defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)) && defined (__LP64__)
# define OS "bsd64"
#elif (defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)) && !defined (__LP64__)
# define OS "bsd32"
#elif defined (__linux__) && defined (__LP64__)
# define OS "linux64"
#elif defined (__linux__) && !defined (__LP64__)
# define OS "linux32"
#endif


char *binary_name, *new_version = NULL;
Options *opts;


/************************* Arrays management functions *************************/

/* Set labels name */
static void labels_setname(Labels *data)
{
	int i, j;

	MSG_VERBOSE(_("Setting label names"));

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
		asprintf(&data->objects[FRAML1CACHE + j],           _("L%i Cache"), j + 1); // Frame label
		asprintf(&data->tab_caches[NAME][L1SIZE       + i], _("Size"));
		asprintf(&data->tab_caches[NAME][L1DESCRIPTOR + i], _("Descriptor"));
		asprintf(&data->tab_caches[NAME][L1SPEED      + i], _("Speed"));
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
	asprintf(&data->objects[FRAMBANKS], _("Banks")); // Frame label
	for(i = BANK0_0; i < LASTMEMORY; i += RAMFIELDS)
	{
		asprintf(&data->tab_memory[NAME][BANK0_0 + i], _("Bank %i Ref."), i / RAMFIELDS);
		asprintf(&data->tab_memory[NAME][BANK0_1 + i], _("Bank %i Type"), i / RAMFIELDS);
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
		asprintf(&data->tab_graphics[NAME][GPU1MODEL       + i], _("Model"));
		asprintf(&data->tab_graphics[NAME][GPU1TEMPERATURE + i], _("Temperature"));
	}

	/* Bench tab */
	asprintf(&data->objects[TABBENCH],              _("Bench")); // Tab label
	asprintf(&data->objects[FRAMPRIMESLOW],         _("Prime numbers (slow)")); // Frame label
	asprintf(&data->objects[FRAMPRIMEFAST],         _("Prime numbers (fast)")); // Frame label
	for(i = PRIMESLOWSCORE; i < PARAMDURATION; i += BENCHFIELDS)
	{
		asprintf(&data->tab_bench[NAME][PRIMESLOWSCORE  + i], _("Score"));
		asprintf(&data->tab_bench[NAME][PRIMESLOWRUN    + i], _("Run"));
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
	asprintf(&data->tab_about[COPYRIGHT],           PRGCPYR);
	asprintf(&data->tab_about[LICENSE],             _(
		"This software is distributed under the terms of GNU GPL v3"));
	asprintf(&data->tab_about[NOWARRANTY],            _(
		"This program comes with ABSOLUTELY NO WARRANTY"));
}

/* Replace null pointers by character '\0' */
static int remove_null_ptr(Labels *data)
{
	int i, j, ret = 0;
	const struct Arrays { char **array; const int last; } a[] =
	{
		{ data->tab_cpu[VALUE],         LASTCPU         },
		{ data->tab_caches[VALUE],      LASTCACHES      },
		{ data->tab_motherboard[VALUE], LASTMOTHERBOARD },
		{ data->tab_memory[VALUE],      LASTMEMORY      },
		{ data->tab_system[VALUE],      LASTSYSTEM      },
		{ data->tab_graphics[VALUE],    LASTGRAPHICS    },
		{ data->tab_bench[VALUE],       LASTBENCH       },
		{ NULL,                         0               }
	};

	MSG_VERBOSE(_("Replacing undefined labels by an empty string"));
	for(i = 0; a[i].array != NULL; i++)
	{
		for(j = 0; j < a[i].last; j++)
		{
			if(a[i].array[j] == NULL)
				ret += casprintf(&a[i].array[j], false, " ");
		}
	}

	return ret;
}

/* Dump all data in stdout */
static void dump_data(Labels *data)
{
	int i, j, k = 0;
	const char *col_start = opts->color ? BOLD_BLUE : "";
	const char *col_end   = opts->color ? DEFAULT   : "";
	const struct Arrays { char **array_name, **array_value; int last; } a[] =
	{
		{ data->tab_cpu[NAME],         data->tab_cpu[VALUE],         LASTCPU                     },
		{ data->tab_caches[NAME],      data->tab_caches[VALUE],      LASTCACHES                  },
		{ data->tab_motherboard[NAME], data->tab_motherboard[VALUE], LASTMOTHERBOARD             },
		{ data->tab_memory[NAME],      data->tab_memory[VALUE],      data->dimms_count           },
		{ data->tab_system[NAME],      data->tab_system[VALUE],      LASTSYSTEM                  },
		{ data->tab_graphics[NAME],    data->tab_graphics[VALUE],    data->gpu_count * GPUFIELDS },
		{ NULL,                        NULL,                         0                           }
	};
	const struct Frames { int tab_nb, lab_nb, frame_nb; } f[] =
	{
		{ NO_CPU,         VENDOR,       FRAMPROCESSOR       },
		{ NO_CPU,         CORESPEED,    FRAMCLOCKS          },
		{ NO_CPU,         LEVEL1D,      FRAMCACHE           },
		{ NO_CPU,         SOCKETS,      -1                  },
		{ NO_CACHES,      L1SIZE,       FRAML1CACHE         },
		{ NO_CACHES,      L2SIZE,       FRAML2CACHE         },
		{ NO_CACHES,      L3SIZE,       FRAML3CACHE         },
		{ NO_MOTHERBOARD, MANUFACTURER, FRAMMOTHERBOARD     },
		{ NO_MOTHERBOARD, BRAND,        FRAMBIOS            },
		{ NO_MOTHERBOARD, CHIPVENDOR,   FRAMCHIPSET         },
		{ NO_MEMORY,      BANK0_0,      FRAMBANKS           },
		{ NO_SYSTEM,      KERNEL,       FRAMOPERATINGSYSTEM },
		{ NO_SYSTEM,      USED,         FRAMMEMORY          },
		{ NO_GRAPHICS,    GPU1VENDOR,   FRAMGPU1            },
		{ NO_GRAPHICS,    GPU2VENDOR,   FRAMGPU2            },
		{ NO_GRAPHICS,    GPU3VENDOR,   FRAMGPU3            },
		{ NO_GRAPHICS,    GPU4VENDOR,   FRAMGPU4            }
	};

	MSG_VERBOSE(_("Dumping data..."));
	for(i = 0; a[i].array_name != NULL; i++)
	{
		MSG_STDOUT("  %s>>>>>>>>>> %s <<<<<<<<<<%s", col_start, data->objects[i], col_end);
		for(j = 0; j < a[i].last; j++)
		{
			if(f[k].tab_nb == i && f[k].lab_nb == j)
			{
				MSG_STDOUT("\n\t%s***** %s *****%s", col_start, (f[k].frame_nb >= 0) ?
				                                 data->objects[f[k].frame_nb] : "*", col_end);
				k++;
			}
			MSG_STDOUT("%16s: %s", a[i].array_name[j], a[i].array_value[j]);
		}
		MSG_STDOUT("\n");
	}

	labels_free(data);
}


/************************* Update-related functions *************************/

/* Check if running version is latest */
static bool check_new_version(void)
{
	int err = 1;
	bool with_curl = false;
	bool with_wget = false;

	if(opts->use_network <= 0)
		return false;

	MSG_VERBOSE(_("Checking on Internet for a new version..."));
	if(command_exists("curl"))
		with_curl = true;

	if(command_exists("wget"))
		with_wget = true;

	if(!with_curl && !with_wget)
	{
		MSG_WARNING(_("Curl/Wget are missing on your system, can't check for a new version"));
		return false;
	}

	/* Retrieve the last tag on Git repo */
	if(with_curl)
		err = popen_to_str("curl --max-time 1 -s http://x0rg.github.io/CPU-X/version.txt", &new_version);

	if(err && with_wget)
	{
		opts->use_wget = true;
		err = popen_to_str("wget --timeout=1 -O- -q http://x0rg.github.io/CPU-X/version.txt", &new_version);
	}

	/* Compare Git tag with running version */
	if(err)
	{
		MSG_WARNING(_("Can not reach Internet"));
		opts->use_network = 0;
	}
	else if(strcmp(new_version, PRGVER))
	{
		MSG_VERBOSE(_("A new version of %s is available!"), PRGNAME);
		return true;
	}
	else
		MSG_VERBOSE(_("No new version available"));

	free(new_version);
	new_version = NULL;
	return false;
}

/* Apply new portable version if available */
static int update_prg(void)
{
	int err = 0;
#if PORTABLE_BINARY
	bool delete = true;
	int i;
	char *file, *tmp, *opt;
	const char *ext[] = { "bsd64", "bsd32", "linux32", "linux64", NULL };

	if(opts->use_network <= 0)
	{
		MSG_WARNING(_("Network access is disabled by environment variable"
		            " (set CPUX_NETWORK with a positive value to enable it)"));
		return 1;
	}

	if(new_version == NULL)
	{
		MSG_WARNING(_("No new version available"));
		return 2;
	}

	/* Find what archive we need to download */
	if(HAS_GTK)
		asprintf(&file, "%s_v%s_portable",       PRGNAME, new_version);
	else
		asprintf(&file, "%s_v%s_portable_noGTK", PRGNAME, new_version);

	/* Download archive */
	MSG_VERBOSE(_("Downloading new version..."));

	if(opts->use_wget)
	{
		opt = opts->verbose ? "" : "-q";
		asprintf(&tmp, "wget %s https://github.com/%s/%s/releases/download/v%s/%s.tar.gz",
			 opt, PRGAUTH, PRGNAME, new_version, file);
	}
	else
	{
		opt = opts->verbose ? "" : "s";
		asprintf(&tmp, "curl -L%s https://github.com/%s/%s/releases/download/v%s/%s.tar.gz -o %s.tar.gz",
			 opt, PRGAUTH, PRGNAME, new_version, file, file);
	}
	system(tmp);
	free(new_version);

	/* Extract archive */
	MSG_VERBOSE(_("Extracting new version..."));
	opt = opts->verbose ? strdup("v") : strdup("");
	asprintf(&tmp, "tar -zx%sf %s.tar.gz", opt, file);
	system(tmp);

	/* Rename new binary */
	MSG_VERBOSE(_("Applying new version..."));
	if(strstr(binary_name, PRGVER) != NULL)
	{
		err    = remove(binary_name);
		delete = false;
	}
	else
	{
		asprintf(&tmp, "%s.%s", file, OS);
		err = rename(tmp, binary_name);
	}

	if(err)
		MSG_ERROR(_("an error occurred while renaming files"));
	else
		MSG_VERBOSE(_("Update successful!"));

	/* Delete temporary files */
	asprintf(&tmp, "%s.tar.gz", file);
	err = remove(tmp);
	for(i = 0; ext[i] != NULL; i++)
	{
		asprintf(&tmp, "%s.%s", file, ext[i]);
		if(strcmp(ext[i], OS) != 0 || delete)
			err += remove(tmp);
	}

	if(err > 1)
		MSG_ERROR(_("an error occurred while deleting temporary files"));

	free_multi(file, tmp, opt);
#endif /* PORTABLE_BINARY */
	return err;
}


/************************* Options-related functions *************************/

#define N_(x) x
static const struct AvailableOpts
{
	const bool has_mod;
	const char short_opt;
	const char *long_opt;
	const int  need_arg;
	char       *description;
} o[] =
{
	{ HAS_GTK,         'g', "gtk",       no_argument,       N_("Start graphical user interface (GUI) (default)")           },
	{ HAS_NCURSES,     'n', "ncurses",   no_argument,       N_("Start text-based user interface (TUI)")                    },
	{ true,            'd', "dump",      no_argument,       N_("Dump all data on standard output and exit")                },
	{ true,            'c', "core",      required_argument, N_("Select CPU core to monitor (integer)")                     },
	{ true,            'r', "refresh",   required_argument, N_("Set custom time between two refreshes (in seconds)")       },
	{ HAS_BANDWIDTH,   't', "cachetest", required_argument, N_("Set custom bandwidth test for CPU caches speed (integer)") },
	{ HAS_DMIDECODE,   'D', "dmidecode", no_argument,       N_("Run embedded command dmidecode and exit")                  },
	{ HAS_BANDWIDTH,   'B', "bandwidth", no_argument,       N_("Run embedded command bandwidth and exit")                  },
	{ true,            'o', "nocolor",   no_argument,       N_("Disable colored output")                                   },
	{ true,            'v', "verbose",   no_argument,       N_("Verbose output")                                           },
	{ PORTABLE_BINARY, 'u', "update",    no_argument,       N_("Update portable version if a new version is available")    },
	{ true,            'h', "help",      no_argument,       N_("Print help and exit")                                      },
	{ true,            'V', "version",   no_argument,       N_("Print version and exit")                                   },
	{ true,            '0', NULL,        0,                 NULL                                                           }
};
#undef N_

/* This is help display with --help option */
static void help(void)
{
	int i;

	MSG_STDOUT(_("Usage: %s [OPTIONS]\n"), binary_name);
	MSG_STDOUT(_("Available OPTIONS:"));
	for(i = 0; o[i].long_opt != NULL; i++)
	{
		if(o[i].has_mod)
			MSG_STDOUT("  -%c, --%-10s %s", o[i].short_opt, o[i].long_opt, _(o[i].description));
	}
}

/* This is the --version option */
static void version(void)
{
	int i;
	char *strver;
	const struct LibsVer { const bool has_mod; const char *lib, *version; } v[] =
	{
		{ HAS_GTK,         "GTK",         GTK_VERSION         },
		{ HAS_NCURSES,     "NCURSES",     NCURSES_VERSION     },
		{ HAS_LIBCPUID,    "LIBCPUID",    LIBCPUID_VERSION    },
		{ HAS_LIBPCI,      "LIBPCI",      LIBPCI_VERSION      },
		{ HAS_LIBPROCPS,   "LIBPROCPS",   LIBPROCPS_VERSION   },
		{ HAS_LIBSTATGRAB, "LIBSTATGRAB", LIBSTATGRAB_VERSION },
		{ HAS_DMIDECODE,   "DMIDECODE",   DMIDECODE_VERSION   },
		{ HAS_BANDWIDTH,   "BANDWIDTH",   BANDWIDTH_VERSION   },
		{ false,           NULL,          NULL                }
	};

	check_new_version();
	if(opts->use_network > 0 && new_version != NULL)
		asprintf(&strver, _("(version %s is available)"), new_version);
	else if(opts->use_network > 0)
		asprintf(&strver, _("(up-to-date)"));
	else
		strver = "";

	MSG_STDOUT("%s %s %s", PRGNAME, PRGVER, strver);
	MSG_STDOUT("%s\n", PRGCPYR);
	MSG_STDOUT(_("This is free software: you are free to change and redistribute it."));
	MSG_STDOUT(_("This program comes with ABSOLUTELY NO WARRANTY"));
	MSG_STDOUT(_("See the GPLv3 license: <http://www.gnu.org/licenses/gpl.txt>\n"));
	MSG_STDOUT(_("Built on %s, %s (with %s %s on %s)."), __DATE__, __TIME__, CC, __VERSION__, OS);

	/* Print features version */
	for(i = 0; v[i].lib != NULL; i++)
	{
		if(v[i].has_mod)
			MSG_STDOUT(_("-- %-9s version: %s"), v[i].lib, v[i].version);
	}
}

/* Parse options given in arg */
static void menu(int argc, char *argv[])
{
	int i, j = 0, c, tmp_arg = -1;
	char *tmp_opt      = NULL;
	char shortopts[32] = "";
	struct option longopts[sizeof(o)/sizeof(o[0]) - 1];

	/* Filling longopts structure */
	for(i = 0; o[i].long_opt != NULL; i++)
	{
		while(!o[i].has_mod)
			i++;
		longopts[j] = (struct option) { .name = o[i].long_opt, .has_arg = o[i].need_arg, .flag = 0, .val = o[i].short_opt };
		asprintf(&tmp_opt, "%c%s", o[i].short_opt, o[i].need_arg ? ":" : "");
		strcat(shortopts, tmp_opt);
		free(tmp_opt);
		j++;
	}

	/* Set the default mode */
	if(HAS_GTK && (getenv("DISPLAY") != NULL || getenv("WAYLAND_DISPLAY") != NULL))
		opts->output_type = OUT_GTK;
	else if(HAS_NCURSES)
		opts->output_type = OUT_NCURSES;
	else
		opts->output_type = OUT_DUMP;

	/* Parse options */
	while((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
	{
		switch(c)
		{
			case 'g':
				opts->output_type = OUT_GTK;
				break;
			case 'n':
				opts->output_type = OUT_NCURSES;
				break;
			case 'd':
				opts->output_type = OUT_DUMP;
				break;
			case 'c':
				tmp_arg = atoi(optarg);
				if(tmp_arg >= 0)
					opts->selected_core = tmp_arg;
				break;
			case 'r':
				tmp_arg = atoi(optarg);
				if(tmp_arg >= 1)
					opts->refr_time = tmp_arg;
				break;
			case 't':
				tmp_arg = atoi(optarg);
				if(tmp_arg >= 0)
					opts->bw_test = atoi(optarg);
				break;
			case 'D':
				opts->output_type = OUT_DMIDECODE;
				if(HAS_DMIDECODE)
					exit(run_dmidecode());
			case 'B':
				opts->output_type = OUT_BANDWIDTH;
				if(HAS_BANDWIDTH)
					exit(run_bandwidth());
			case 'o':
				opts->color = false;
				break;
			case 'v':
				opts->verbose = true;
				break;
			case 'u':
				opts->update = true;
				break;
			case 'h':
				help();
				exit(EXIT_SUCCESS);
			case 'V':
				version();
				exit(EXIT_SUCCESS);
			case '?':
			default:
				help();
				exit(EXIT_FAILURE);
		}
	}
}


/************************* Main-related functions *************************/

/* Action on SIGSEV/SIGFPE */
void sighandler(int signum)
{
	int bt_size, i;
	char **bt_syms, *cmd = NULL, *buff = NULL;
	void *bt[16];

	/* Get the backtrace */
	bt_size = backtrace(bt, 16);
	bt_syms = backtrace_symbols(bt, bt_size);

	/* Print the backtrace */
	MSG_STDERR(_("\n%sOops, something was wrong! %s has received signal %d (%s) and has crashed.%s"), BOLD_RED, PRGNAME, signum, strsignal(signum), DEFAULT);
	MSG_STDERR("========================= Backtrace =========================");
        for(i = 1; i < bt_size; i++)
	{
		MSG_STDERR("#%2i %s", i, bt_syms[i]);
		asprintf(&cmd, "addr2line %s -e /proc/%d/exe", strtok(strrchr(bt_syms[i], '[') + 1, "]"), getpid());
		popen_to_str(cmd, &buff);
		if(strstr(buff, "??") == NULL)
			MSG_STDERR(" ==> %s", strrchr(buff, '/') + 1);
		free_multi(cmd, buff);
        }
	MSG_STDERR("======================== End Backtrace =======================\n");
	MSG_STDERR(_("You can paste this backtrace by opening a new issue here:"));
	MSG_STDERR("https://github.com/X0rg/CPU-X/issues/new\n");

	/* Stop program */
	free_multi(bt_syms, bt);
	signal(signum, SIG_DFL);
	kill(getpid(), signum);
}

 /* Enable internationalization support */
static int set_locales(void)
{
	int err = 0;

#if PORTABLE_BINARY && HAS_GETTEXT
	int i;
	char *command = NULL, *path = NULL;
	FILE *mofile;

	/* Write .mo files in temporary directory */
	for(i = 0; ptrlen[i] != NULL; i++)
	{
		asprintf(&command, "mkdir -p %s/%s/LC_MESSAGES/", LOCALEDIR, lang[i]);
		asprintf(&path, "%s/%s/LC_MESSAGES/%s.mo", LOCALEDIR, lang[i], GETTEXT_PACKAGE);
		err    = system(command);
		mofile = NULL;
		mofile = fopen(path, "w");
		free_multi(command, path);

		if(mofile != NULL)
		{
			fwrite(ptrlang[i], sizeof(unsigned char), *(ptrlen)[i], mofile);
			fclose(mofile);
		}
		else
			err++;
	}

	if(err)
		MSG_ERROR("an error occurred while extracting translations");
# endif /* PORTABLE_BINARY && HAS_GETTEXT */

	/* Apply locale */
	setlocale(LC_ALL, "");
	err  = bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR)        ? 0 : 1;
	err += bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8") ? 0 : 1;
	err += textdomain(GETTEXT_PACKAGE)                       ? 0 : 1;

	/* Check if something is wrong */
	if(err)
	{
		MSG_ERROR(_("an error occurred while setting locale"));
		return 1;
	}
	else
		return 0;
}

int main(int argc, char *argv[])
{
	/* Parse options */
	binary_name = argv[0];
	Labels *data = &(Labels) {
	                .tab_cpu    = {{ NULL }}, .tab_caches     = {{ NULL }}, .tab_motherboard = {{ NULL }},
	                .tab_memory = {{ NULL }}, .tab_system     = {{ NULL }}, .tab_graphics    = {{ NULL }},
	                .cpu_freq   = 0,          .bus_freq       = 0.0,
	                .cpu_count  = 0,          .gpu_count      = 0,          .dimms_count     = 0 };

	data->l_data = &(LibcpuidData) { .cpu_vendor_id = -1, .cpu_model = -1, .cpu_ext_model = -1, .cpu_ext_family = -1 };

	data->w_data = &(BandwidthData) { .l1_size = 0, .test_count = 0, .test_name = NULL, .speed = { 0 } };

	data->m_data = &(MemoryData) { .mem_total = 0, .swap_total = 0 };

	data->b_data = &(BenchData) { .run = false, .duration = 1, .threads = 1, .primes = 0 };

	opts = &(Options) { .output_type = 0,     .selected_core  = 0,          .refr_time       = 1,
	                    .bw_test     = 0,     .verbose        = false,      .color           = true,
	                    .update      = false, .use_network    = 1,          .use_wget        = false };

	set_locales();
	signal(SIGSEGV, sighandler);
	signal(SIGFPE,  sighandler);

	if(getenv("CPUX_NETWORK"))
		opts->use_network = atoi(getenv("CPUX_NETWORK"));

	menu(argc, argv);
	if(getuid())
	{
		MSG_WARNING(_("Root privileges are required to work properly"));
		MSG_WARNING(_("Some informations will not be retrievable"));
	}
	labels_setname (data);
	fill_labels    (data);
	remove_null_ptr(data);
	check_new_version();

	/* Show data */
	switch(opts->output_type)
	{
		case OUT_GTK:
			if(HAS_GTK)
				start_gui_gtk(&argc, &argv, data);
			break;
		case OUT_NCURSES:
			if(HAS_NCURSES)
				start_tui_ncurses(data);
			break;
		case OUT_DUMP:
			dump_data(data);
			break;
	}

	if(PORTABLE_BINARY && opts->update)
		update_prg();

	return EXIT_SUCCESS;
}


/************************* Public functions *************************/

/* Add a newline for given string (used by MSG_XXX macros) */
char *msg_newline(char *color, char *str)
{
	static char *buff = NULL;

	free(buff);
	if(opts->color)
		asprintf(&buff, "%s%s%s\n", color, str, DEFAULT);
	else
		asprintf(&buff, "%s\n",     str);

	return buff;
}

/* Add a newline and more informations for given string (used by MSG_ERROR macro) */
char *msg_error(char *color, char *file, int line, char *str)
{
	static char *buff = NULL;

	free(buff);
	if(errno)
		asprintf(&buff, "%s%s:%s:%i: %s (%s)%s\n", opts->color ? color : DEFAULT, PRGNAME, file, line, str, strerror(errno), DEFAULT);
	else
		asprintf(&buff, "%s%s:%s:%i: %s%s\n", opts->color ? color : DEFAULT, PRGNAME, file, line, str, DEFAULT);

	errno = 0;
	return buff;
}

/* An asprintf-like function, but which can clean some parts of 'str' if 'clean_str' is true
 * - It calls vasprintf if 'fmt' is a valid string
 * - If 'clean_str' is true, it removes "unvalid args" from 'str' until next "valid arg"
     E.g.: casprintf(&str, false, "%i nm", 0): str = "0 nm"
           casprintf(&str, true,  "%i nm", 0): str = ""
	   casprintf(&str, true,  "%i nm", 32): str = "32 nm"
	   casprintf(&str, true,  "%i KB %i-way", -1, 12): str = "12-way" */
int casprintf(char **str, bool clean_str, const char *fmt, ...)
{
	bool remove;
	int i, j, ret;
	va_list aptr;

	if(fmt == NULL)
		return 0;

	va_start(aptr, fmt);
	ret = vasprintf(str, fmt, aptr);
	va_end(aptr);

	if(!clean_str)
		return ret;

	remove = (((*str)[0] == '0') && (atof(*str) == 0.0)) || (atoi(*str) < 0);
	j      = remove ? -1 : 0;

	for(i = 1; (*str)[i] != '\0'; i++)
	{
		if(((((*str)[i] == '0') && (atof(*str + i) == 0.0)) || (atoi(*str + i) < 0)) && (isspace((*str)[i - 1])))
			remove = true;
		if(!remove)
			(*str)[++j] = (*str)[i];
		if((isspace((*str)[i])) && !(isdigit((*str)[i - 1])))
			remove = false;
	}

	(*str)[++j] = '\0';
	*str = realloc(*str, j + 1);

	return j;
}

/* Try to free given variables */
void free_multi(void *var, ...)
{
	void *ptr, *sentinel = NULL;
	va_list aptr;

	va_start(aptr, var);
	free(var);

	while((ptr = va_arg(aptr, void *)) != NULL && ptr != sentinel)
	{
		if(sentinel == NULL)
			sentinel = ptr;
		free(ptr);
	}

	va_end(aptr);
}

/* Check if a command exists */
bool command_exists(char *in)
{
	bool ret;
	char *cmd;

	asprintf(&cmd, "which %s >/dev/null 2>&1", in);
	ret = system(cmd);
	free(cmd);

	return !ret;
}

/* Open a file and put its content in buffer */
int fopen_to_str(char *file, char **buffer)
{
	char tmp[MAXSTR];
	FILE *f = NULL;

	if((f = fopen(file, "r")) == NULL)
		goto error;

	if(fgets(tmp, MAXSTR, f) == NULL)
		goto error;

	tmp[strlen(tmp) - 1] = '\0';
	asprintf(buffer, tmp);

	return fclose(f);

error:
	MSG_ERROR(_("an error occurred while opening file '%s'"), file);
	return (f == NULL) ? 1 : 2 + fclose(f);
}

/* Open a pipe and put its content in buffer */
int popen_to_str(char *command, char **buffer)
{
	bool command_ok;
	char tmp[MAXSTR];
	char *test_command = strdup(command);
	FILE *p = NULL;

	command_ok = command_exists(strtok(test_command, " "));
	free(test_command);
	if(!command_ok)
		return -1;

	if((p = popen(command, "r")) == NULL)
		goto error;

	if(fgets(tmp, MAXSTR, p) == NULL)
		goto error;

	tmp[strlen(tmp) - 1] = '\0';
	asprintf(buffer, tmp);

	return pclose(p);

error:
	MSG_ERROR(_("an error occurred while running command '%s'"), command);
	return (p == NULL) ? 1 : 2 + pclose(p);
}

/* Load a kernel module */
bool load_module(char *module)
{
	static bool loaded = false;
	char *cmd_check, *cmd_load = NULL;

	if(loaded)
		return true;

#if defined (__linux__)
	asprintf(&cmd_check, "lsmod | grep %s > /dev/null", module);
	if(system(cmd_check))
		asprintf(&cmd_load, "modprobe %s 2> /dev/null", module);
#elif defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	asprintf(&cmd_check, "kldstat | grep %s > /dev/null", module);
	if(system(cmd_check))
		asprintf(&cmd_load, "kldload -n %s 2> /dev/null", module);
#endif
	if(cmd_load == NULL)
		return ((loaded = true));
	else if(getuid())
		return false;
	else
		return !system(cmd_load);
}

/* Free memory after display labels */
void labels_free(Labels *data)
{
	int i, j;
	const struct Arrays { char **array_name, **array_value; const int last; } a[] =
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

	MSG_VERBOSE(_("Freeing memory"));
	for(i = 0; i < LASTOBJ; i++)
		free(data->objects[i]);

	for(i = 0; a[i].array_name != NULL; i++)
	{
		for(j = 0; j < a[i].last; j++)
		{
			free(a[i].array_name[j]);
			free(a[i].array_value[j]);
			a[i].array_name[j] = NULL;
			a[i].array_value[j] = NULL;
		}
	}

	for(i = 0; i < data->w_data->test_count; i++)
		free(data->w_data->test_name[i]);
	free(data->w_data->test_name);

	for(i = 0; i < LASTABOUT; i++)
		free(data->tab_about[i]);
}
