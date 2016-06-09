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

#if defined(__clang__)
# define CC "Clang"
#elif defined(__GNUC__) || defined(__GNUG__)
# define CC "GCC"
#else
# define CC "Unknown"
#endif

#if defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
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
	int i;

	MSG_VERBOSE(_("Setting label names"));
	/* Various objects */
	asprintf(&data->objects[TABCPU],                _("CPU"));
	asprintf(&data->objects[TABCACHES],             _("Caches"));
	asprintf(&data->objects[TABMOTHERBOARD],        _("Motherboard"));
	asprintf(&data->objects[TABMEMORY],             _("Memory"));
	asprintf(&data->objects[TABSYSTEM],             _("System"));
	asprintf(&data->objects[TABGRAPHICS],           _("Graphics"));
	asprintf(&data->objects[TABABOUT],              _("About"));
	asprintf(&data->objects[FRAMPROCESSOR],         _("Processor"));
	asprintf(&data->objects[FRAMCLOCKS],            _("Clocks"));
	asprintf(&data->objects[FRAMCACHE],             _("Cache"));
	asprintf(&data->objects[FRAML1CACHE],           _("L1 Cache"));
	asprintf(&data->objects[FRAML2CACHE],           _("L2 Cache"));
	asprintf(&data->objects[FRAML3CACHE],           _("L3 Cache"));
	asprintf(&data->objects[FRAMTEST],              _("Test"));
	asprintf(&data->objects[FRAMMOTHERBOARD],       _("Motherboard"));
	asprintf(&data->objects[FRAMBIOS],              _("BIOS"));
	asprintf(&data->objects[FRAMCHIPSET],           _("Chipset"));
	for(i = 0; i < LASTGRAPHICS / GPUFIELDS; i ++)
		asprintf(&data->objects[FRAMGPU1 + i],  _("Card %i"), i);
	asprintf(&data->objects[FRAMBANKS],             _("Banks"));
	asprintf(&data->objects[FRAMOPERATINGSYSTEM],   _("Operating System"));
	asprintf(&data->objects[FRAMMEMORY],            _("Memory"));
	asprintf(&data->objects[FRAMPRIMESLOW],         _("Prime numbers (slow)"));
	asprintf(&data->objects[FRAMPRIMEFAST],         _("Prime numbers (fast)"));
	asprintf(&data->objects[FRAMPARAM],             _("Parameters"));
	asprintf(&data->objects[FRAMABOUT],             _("About"));
	asprintf(&data->objects[FRAMLICENSE],           _("License"));
	asprintf(&data->objects[LABVERSION],            _("Version %s"), PRGVER);
	asprintf(&data->objects[LABDESCRIPTION],        _(
		"%s is a Free software that gathers information\n"
		"on CPU, motherboard and more."), PRGNAME);
	asprintf(&data->objects[LABAUTHOR],             _("Author : %s"), PRGAUTH);
	asprintf(&data->objects[LABCOPYRIGHT],          "%s", PRGCPYR);
	asprintf(&data->objects[LABLICENSE],            _(
		"This program comes with ABSOLUTELY NO WARRANTY"));

	/* CPU tab */
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

	asprintf(&data->tab_cpu[NAME][CORESPEED],       _("Core Speed"));
	asprintf(&data->tab_cpu[NAME][MULTIPLIER],      _("Multiplier"));
	asprintf(&data->tab_cpu[NAME][BUSSPEED],        _("Bus Speed"));
	asprintf(&data->tab_cpu[NAME][USAGE],           _("Usage"));

	asprintf(&data->tab_cpu[NAME][LEVEL1D],         _("L1 Data"));
	asprintf(&data->tab_cpu[NAME][LEVEL1I],         _("L1 Inst."));
	asprintf(&data->tab_cpu[NAME][LEVEL2],          _("Level 2"));
	asprintf(&data->tab_cpu[NAME][LEVEL3],          _("Level 3"));

	asprintf(&data->tab_cpu[NAME][SOCKETS],         _("Socket(s)"));
	asprintf(&data->tab_cpu[NAME][CORES],           _("Core(s)"));
	asprintf(&data->tab_cpu[NAME][THREADS],         _("Thread(s)"));

	/* Caches tab */
	for(i = 0; i < LASTCACHES / CACHEFIELDS + 1; i++)
	{
		asprintf(&data->tab_caches[NAME][i * CACHEFIELDS],     _("Size"));
		asprintf(&data->tab_caches[NAME][i * CACHEFIELDS + 1], _("Descriptor"));
		asprintf(&data->tab_caches[NAME][i * CACHEFIELDS + 2], _("Speed"));
	}

	/* Motherboard tab */
	asprintf(&data->tab_motherboard[NAME][MANUFACTURER],  _("Manufacturer"));
	asprintf(&data->tab_motherboard[NAME][MBMODEL],       _("Model"));
	asprintf(&data->tab_motherboard[NAME][REVISION],      _("Revision"));

	asprintf(&data->tab_motherboard[NAME][BRAND],         _("Brand"));
	asprintf(&data->tab_motherboard[NAME][BIOSVERSION],   _("Version"));
	asprintf(&data->tab_motherboard[NAME][DATE],          _("Date"));
	asprintf(&data->tab_motherboard[NAME][ROMSIZE],       _("ROM Size"));

	asprintf(&data->tab_motherboard[NAME][CHIPVENDOR],    _("Vendor"));
	asprintf(&data->tab_motherboard[NAME][CHIPMODEL],     _("Model"));

	/* Memory tab */
	for(i = 0; i < BANK7_1 / RAMFIELDS + 1; i++)
	{
		asprintf(&data->tab_memory[NAME][i * RAMFIELDS],     _("Bank %i Ref."), i);
		asprintf(&data->tab_memory[NAME][i * RAMFIELDS + 1], _("Bank %i Type"), i);
	}

	/* System tab */
	asprintf(&data->tab_system[NAME][KERNEL],             _("Kernel"));
	asprintf(&data->tab_system[NAME][DISTRIBUTION],       _("Distribution"));
	asprintf(&data->tab_system[NAME][HOSTNAME],           _("Hostname"));
	asprintf(&data->tab_system[NAME][UPTIME],             _("Uptime"));
	asprintf(&data->tab_system[NAME][COMPILER],           _("Compiler"));

	asprintf(&data->tab_system[NAME][USED],               _("Used"));
	asprintf(&data->tab_system[NAME][BUFFERS],            _("Buffers"));
	asprintf(&data->tab_system[NAME][CACHED],             _("Cached"));
	asprintf(&data->tab_system[NAME][FREE],               _("Free"));
	asprintf(&data->tab_system[NAME][SWAP],               _("Swap"));

	/* Graphics tab */
	for(i = 0; i < LASTGRAPHICS; i += GPUFIELDS)
	{
		asprintf(&data->tab_graphics[NAME][GPU1VENDOR + i],      _("Vendor"));
		asprintf(&data->tab_graphics[NAME][GPU1MODEL + i],       _("Model"));
		asprintf(&data->tab_graphics[NAME][GPU1TEMPERATURE + i], _("Temperature"));
	}

	/* Bench tab */
	for(i = 0; i < PARAMDURATION; i += 2)
	{
		asprintf(&data->tab_bench[NAME][PRIMESLOWSCORE  + i], _("Score"));
		asprintf(&data->tab_bench[VALUE][PRIMESLOWSCORE + i], _("Not started"));
		asprintf(&data->tab_bench[NAME][PRIMESLOWRUN    + i], _("Run"));
	}
	asprintf(&data->tab_bench[NAME][PARAMDURATION], _("Duration"));
	asprintf(&data->tab_bench[NAME][PARAMTHREADS],  _("Threads"));
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
				ret += iasprintf(&a[i].array[j], NULL);
		}
	}

	return ret;
}

/* Dump all data in stdout */
static void dump_data(Labels *data)
{
	int i;

	MSG_VERBOSE(_("Dumping data..."));
	/* CPU tab */
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
		printf("%16s: %s\n", data->tab_cpu[NAME][i], data->tab_cpu[VALUE][i]);
	}

	/* Caches tab */
	printf("\n\n ***** %s *****\n", data->objects[TABCACHES]);
	printf("\t*** %s ***\n", data->objects[FRAML1CACHE]);
	for(i = L1SIZE; i < LASTCACHES; i++)
	{
		if(i == L2SIZE)
			printf("\n\t*** %s ***\n", data->objects[FRAML2CACHE]);
		else if(i == L3SIZE)
			printf("\n\t*** %s ***\n", data->objects[FRAML3CACHE]);
		printf("%16s: %s\n", data->tab_caches[NAME][i], data->tab_caches[VALUE][i]);
	}

	/* Motherboard tab */
	printf("\n\n ***** %s *****\n", data->objects[TABMOTHERBOARD]);
	printf("\n\t*** %s ***\n", data->objects[FRAMMOTHERBOARD]);
	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
	{
		if(i == BRAND)
			printf("\n\t*** %s ***\n", data->objects[FRAMBIOS]);
		else if(i == CHIPVENDOR)
			printf("\n\t*** %s ***\n", data->objects[FRAMCHIPSET]);
		printf("%16s: %s\n", data->tab_motherboard[NAME][i], data->tab_motherboard[VALUE][i]);
	}

	/* Memory tab */
	printf("\n\n ***** %s *****\n", data->objects[TABMEMORY]);
	printf("\n\t*** %s ***\n", data->objects[FRAMBANKS]);
	for(i = BANK0_0; i < data->dimms_count; i++)
		printf("%16s: %s\n", data->tab_memory[NAME][i], data->tab_memory[VALUE][i]);

	/* System tab */
	printf("\n\n ***** %s *****\n", data->objects[TABSYSTEM]);
	printf("\n\t*** %s ***\n", data->objects[FRAMOPERATINGSYSTEM]);
	for(i = KERNEL; i < LASTSYSTEM; i++)
	{
		if(i == USED)
			printf("\n\t*** %s ***\n", data->objects[FRAMMEMORY]);
		printf("%16s: %s\n", data->tab_system[NAME][i], data->tab_system[VALUE][i]);
	}

	/* Graphics tab */
	printf("\n\n ***** %s *****\n", data->objects[TABGRAPHICS]);
	printf("\n\t*** %s ***\n", data->objects[FRAMGPU1]);
	for(i = GPU1VENDOR; i < data->gpu_count; i++)
	{
		if(i == GPU2VENDOR)
			printf("\n\t*** %s ***\n", data->objects[FRAMGPU2]);
		else if(i == GPU2VENDOR)
			printf("\n\t*** %s ***\n", data->objects[FRAMGPU3]);
		else if(i == GPU2VENDOR)
			printf("\n\t*** %s ***\n", data->objects[FRAMGPU4]);
		printf("%16s: %s\n", data->tab_graphics[NAME][i], data->tab_graphics[VALUE][i]);
	}

	labels_free(data);
}


/************************* Update-related functions *************************/

/* Check if running version is latest */
static bool check_new_version(void)
{
	MSG_VERBOSE(_("Checking on Internet for a new version..."));
	if(!command_exists("curl"))
	{
		MSG_WARNING(_("curl is missing on your system, can't check for a new version"));
		return false;
	}

	/* Retrieve the last tag on Git repo */
	popen_to_str("curl --max-time 1 -s https://api.github.com/repos/X0rg/CPU-X/releases/latest | grep 'tag_name' | awk -F '\"' '{ print $4 }' | cut -d'v' -f2",
	             &new_version);

	/* Compare Git tag with running version */
	if(strverscmp(new_version, PRGVER) > 0)
	{
		MSG_VERBOSE(_("A new version is available!"));
		return true;
	}
	else
	{
		MSG_VERBOSE(_("No new version available."));
		free(new_version);
		new_version = NULL;
		return false;
	}
}

/* Apply new portable version if available */
static int update_prg(void)
{
	int err = 0;
#if PORTABLE_BINARY
	int i;
	char *file, *tmp, *opt;
	const char *ext[] = { "bsd32", "linux32", "linux64", NULL };

	if(new_version == NULL)
	{
		MSG_ERROR(_("No new version available."));
		return 1;
	}

	/* Find what archive we need to download */
	if(HAS_GTK)
		asprintf(&file, "%s_v%s_portable",       PRGNAME, new_version);
	else
		asprintf(&file, "%s_v%s_portable_noGTK", PRGNAME, new_version);

	/* Download archive */
	MSG_VERBOSE(_("Downloading new version..."));
	opt = opts->verbose ? strdup("") : strdup("s");
	asprintf(&tmp, "curl -L%s https://github.com/%s/%s/releases/download/v%s/%s.tar.gz -o %s.tar.gz ",
	         opt, PRGAUTH, PRGNAME, new_version, file, file);
	system(tmp);
	free(new_version);

	/* Extract archive */
	MSG_VERBOSE(_("Extracting new version..."));
	opt = opts->verbose ? strdup("v") : strdup("");
	asprintf(&tmp, "tar -zx%sf %s.tar.gz", opt, file);
	system(tmp);
	free(opt);

	/* Rename new binary */
	MSG_VERBOSE(_("Applying new version..."));
	asprintf(&tmp, "%s.%s", file, OS);
	err = rename(tmp, binary_name);
	if(err)
		MSG_VERBOSE(_("Error when updating."));
	else
		MSG_VERBOSE(_("Update successful!"));

	/* Delete temporary files */
	asprintf(&tmp, "%s.tar.gz", file);
	err = remove(tmp);
	for(i = 0; ext[i] != NULL; i++)
	{
		asprintf(&tmp, "%s.%s", file, ext[i]);
		err += remove(tmp);
	}

	if(err > 1)
		MSG_VERBOSE(_("Error when deleting temporary files."));

	free(file);
	free(tmp);
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
static void help(FILE *out, char *argv[])
{
	int i = -1;

	fprintf(out, _("Usage: %s [OPTION]\n\n"), argv[0]);
	fprintf(out, _("Available OPTION:\n"));
	while(o[++i].long_opt != NULL)
	{
		if(o[i].has_mod)
			fprintf(out, "  -%c, --%-10s %s\n", o[i].short_opt, o[i].long_opt, _(o[i].description));
	}
}

/* This is the --version option */
static void version(void)
{
	int i, count = 0;
	char *strver, *newver = NULL;
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

	if(new_version != NULL)
		asprintf(&strver, _("(version %s is available)"), newver);
	else
		asprintf(&strver, _("(up-to-date)"));

	printf("%s %s %s\n%s\n\n", PRGNAME, PRGVER, strver, PRGCPYR);
	printf(_(""
	"This is free software: you are free to change and redistribute it.\n"
	"This program comes with ABSOLUTELY NO WARRANTY\n"
	"See the GPLv3 license: <http://www.gnu.org/licenses/gpl.txt>\n\n"
	"Built on %s, %s (with %s %s).\n"),
	__DATE__, __TIME__, CC, __VERSION__);

	/* Print features version */
	for(i = 0; v[i].lib != NULL; i++)
	{
		if(v[i].has_mod)
		{
			printf(_("-- %-9s version: %s\n"), v[i].lib, v[i].version);
			count++;
		}
	}
	printf(_("=> %i/%i dependencies are enabled.\n"), count, i - 1);
}

/* Parse options given in arg */
static void menu(int argc, char *argv[])
{
	int i = -1, j = 0, c, tmp_arg = -1;
	char *shortopts = { "" };
	struct option longopts[sizeof(o)/sizeof(o[0]) - 1];

	/* Filling longopts structure */
	while(o[++i].long_opt != NULL)
	{
		while(!o[i].has_mod)
			i++;
		longopts[j] = (struct option) { .name = o[i].long_opt, .has_arg = o[i].need_arg, .flag = 0, .val = o[i].short_opt };
		asprintf(&shortopts, "%s%c%s", shortopts, o[i].short_opt, o[i].need_arg ? ":" : "" );
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
				if(tmp_arg > 1)
					opts->refr_time = tmp_arg;
				break;
			case 't':
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
				help(stdout, argv);
				exit(EXIT_SUCCESS);
			case 'V':
				version();
				exit(EXIT_SUCCESS);
			case '?':
			default:
				help(stderr, argv);
				exit(EXIT_FAILURE);
		}
	}
}


/************************* Main-related functions *************************/

/* Action on SIGSEV/SIGFPE */
void sighandler(int signum)
{
	int bt_size, i;
	char **bt_syms, *cmd, *buff = NULL;
	void *bt[1 << 4];

	/* Get the backtrace */
	bt_size = backtrace(bt, 1 << 4);
	bt_syms = backtrace_symbols(bt, bt_size);

	/* Print the backtrace */
	fprintf(stderr, "%s", strsignal(signum));
	fprintf(stderr, _("\n%sOops, something was wrong! %s got signal %d and has crashed.%s\n\n"), BOLD_RED, PRGNAME, signum, RESET);
	fprintf(stderr, "======= Backtrace: =========\n");
        for(i = 1; i < bt_size; i++)
	{
		fprintf(stderr, "#%2i %s", i, bt_syms[i]);
		asprintf(&cmd, "addr2line %s -e /usr/bin/cpu-x", strtok(strrchr(bt_syms[i], '[') + 1, "]"));
		popen_to_str(cmd, &buff);
		if(strstr(buff, "??") == NULL)
			fprintf(stderr, " ==> %s", strrchr(buff, '/') + 1);
		fprintf(stderr, "\n");
        }
	fprintf(stderr, "======= End Backtrace ======\n\n");
	fprintf(stderr, _("You can paste this backtrace by opening a new issue here:\n"));
	fprintf(stderr, "https://github.com/X0rg/CPU-X/issues/new\n\n");

	/* Stop program */
	free(bt_syms);
	free(cmd);
	free(buff);
	signal(signum, SIG_DFL);
	kill(getpid(), signum);
}

 /* Enable internationalization support */
static int set_locales(void)
{
	int err = 0;

#if PORTABLE_BINARY && HAS_GETTEXT
	int i;
	char *command, *path;
	FILE *mofile;

	/* Write .mo files in temporary directory */
	for(i = 0; ptrlen[i] != NULL; i++)
	{
		asprintf(&command, "mkdir --parents %s/%s/LC_MESSAGES/", LOCALEDIR, lang[i]);
		asprintf(&path, "%s/%s/LC_MESSAGES/%s.mo", LOCALEDIR, lang[i], GETTEXT_PACKAGE);
		err    = system(command);
		mofile = NULL;
		mofile = fopen(path, "w");

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
	{
		MSG_VERBOSE(_("Setting locale done"));
		return 0;
	}
}

int main(int argc, char *argv[])
{
	/* Parse options */
	binary_name = argv[0];
	Labels *data = &(Labels) {
	                .tab_cpu    = {{ NULL }}, .tab_caches     = {{ NULL }}, .tab_motherboard = {{ NULL }},
	                .tab_memory = {{ NULL }}, .tab_system     = {{ NULL }}, .tab_graphics    = {{ NULL }},
	                .cpu_freq   = 0,          .cpu_vendor_id  = 0,          .bus_freq        = 0.0,
	                .cpu_count  = 0,          .gpu_count      = 0,          .dimms_count     = 0,
	                .l1_size    = 0,          .l2_size        = 0,          .l3_size         = 0 };

	data->b_data = &(BenchData) { .run = false, .duration = 1, .threads = 1, .primes = 0 };

	opts = &(Options) { .output_type = 0,     .selected_core  = 0,          .refr_time       = 1,
	                    .bw_test     = 0,     .verbose        = false,      .color           = true,
	                    .update      = false, .cpu_temp_msr   = false,      .cpu_volt_msr    = false };

	set_locales();
	signal(SIGSEGV, sighandler);
	signal(SIGFPE,  sighandler);
	menu(argc, argv);

	if(getuid())
		MSG_WARNING(_("WARNING: root privileges are required to work properly\n"));

	labels_setname (data);
	fill_labels    (data);
	remove_null_ptr(data);

	if(!getenv("CPUX_NETWORK"))
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

/* The improved asprintf:
 * - allocate an empty string if input string is null
 * - only call asprintf if there is no format in input string
 * - print "valid" args if input string is formatted, or skip them until next arg
     E.g.: iasprintf(&buff, "%i nm", 32) will allocate "32 nm" string
           iasprintf(&buff, "%i nm", 0) will allocate an empty string
	   iasprintf(&buff, "foo %s %s", NULL, "bar") will allocate "foo bar" */
int iasprintf(char **str, const char *fmt, ...)
{
	bool is_format = false, print = true;
	int arg_int, i, ret = 0;
	unsigned int arg_uint;
	double arg_double;
	char *arg_string, *tmp_fmt;
	va_list aptr;

	/* Allocate an empty string */
	*str    = malloc(1 * sizeof(char));
	*str[0] = '\0';

	/* Exit if input is null or without format */
	if(fmt == NULL)
		return 0;
	else if(strchr(fmt, '%') == NULL)
		return asprintf(str, "%s", fmt);

	/* Read format, character by character */
	va_start(aptr, fmt);
	for(i = 0; fmt[i] != '\0'; i++)
	{
		is_format = fmt[i] == '%' || is_format;
		if(is_format)
		{
			print = true;

			/* Construct the new temporary format */
			if(fmt[i] == '%')
				asprintf(&tmp_fmt, "%%s%%");
			else
				asprintf(&tmp_fmt, "%s%c", tmp_fmt, fmt[i]);

			/* Extract arg */
			switch(fmt[i])
			{
				case '%':
					break;
				case '.':
				case '0' ... '9':
				case 'l':
				case 'L':
					break;
				case 'x':
				case 'X':
				case 'd':
				case 'i':
					is_format = false;
					arg_int = va_arg(aptr, int);
					if(arg_int > 0)
						ret = asprintf(str, tmp_fmt, *str, arg_int);
					else
						print = false;
					break;
				case 'u':
					is_format = false;
					arg_uint = va_arg(aptr, unsigned int);
					if(arg_uint > 0)
						ret = asprintf(str, tmp_fmt, *str, arg_uint);
					else
						print = false;
					break;
				case 'f':
					is_format = false;
					arg_double = va_arg(aptr, double);
					if(arg_double > 0.0)
						ret = asprintf(str, tmp_fmt, *str,  arg_double);
					else
						print = false;
					break;
				case 's':
					is_format = false;
					arg_string = va_arg(aptr, char *);
					if(arg_string != NULL)
						ret = asprintf(str, tmp_fmt, *str, arg_string);
					else
						print = false;
					break;
				default:
					is_format = false;
					asprintf(&tmp_fmt, "(internal) format not implemented: %%%c\n", fmt[i]);
					MSG_ERROR(tmp_fmt);
					break;
			}
		}
		else if(print)
			ret = asprintf(str, "%s%c", *str, fmt[i]);
	}
	va_end(aptr);

	return ret;
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
	char *msg;
	FILE *f = NULL;

	if((*buffer = malloc(MAXSTR * sizeof(char))) == NULL)
		goto error;

	if((f = fopen(file, "r")) == NULL)
		goto error;

	if(fgets(*buffer, MAXSTR, f) == NULL)
		goto error;

	(*buffer)[strlen(*buffer) - 1] = '\0';
	return fclose(f);

error:
	asprintf(&msg, _("fopen_to_str(): an error was ocured (file: %s)"), file);
	MSG_ERROR_ERRNO(msg);
	return (f == NULL) ? 1 : 2 + fclose(f);
}

/* Open a pipe and put its content in buffer */
int popen_to_str(char *command, char **buffer)
{
	char *test_command, *msg;
	FILE *p = NULL;

	if((*buffer = malloc(MAXSTR * sizeof(char))) == NULL)
		goto error;

	asprintf(&test_command, "%s", command);
	if(!command_exists(strtok(test_command, " ")))
		return -1;

	if((p = popen(command, "r")) == NULL)
		goto error;

	if(fgets(*buffer, MAXSTR, p) == NULL)
		goto error;

	(*buffer)[strlen(*buffer) - 1] = '\0';
	return pclose(p);

error:
	asprintf(&msg, _("popen_to_str(): an error was ocured (command: %s)"), command);
	MSG_ERROR_ERRNO(msg);
	return (p == NULL) ? 1 : 2 + pclose(p);
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
}
