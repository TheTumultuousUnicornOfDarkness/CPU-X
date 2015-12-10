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


Options *opts;

/************************* Arrays management functions *************************/

/* Set labels name */
static void labels_setname(Labels *data)
{
	int i;

	MSG_VERBOSE(_("Setting label names"));
	/* Various objects*/
	asprintf(&data->objects[TABCPU],		_("CPU"));
	asprintf(&data->objects[TABCACHE],		_("Caches"));
	asprintf(&data->objects[TABMB],			_("Motherboard"));
	asprintf(&data->objects[TABRAM],		_("RAM"));
	asprintf(&data->objects[TABSYS],		_("System"));
	asprintf(&data->objects[TABGPU],		_("Graphics"));
	asprintf(&data->objects[TABABOUT],		_("About"));
	asprintf(&data->objects[FRAMPROCESSOR],		_("Processor"));
	asprintf(&data->objects[FRAMCLOCKS],		_("Clocks"));
	asprintf(&data->objects[FRAMCACHE],		_("Cache"));
	asprintf(&data->objects[FRAMCACHEL1],		_("L1 Cache"));
	asprintf(&data->objects[FRAMCACHEL2],		_("L2 Cache"));
	asprintf(&data->objects[FRAMCACHEL3],		_("L3 Cache"));
	asprintf(&data->objects[FRAMMOBO],		_("Motherboard"));
	asprintf(&data->objects[FRAMBIOS],		_("BIOS"));
	asprintf(&data->objects[FRAMCHIP],		_("Chipset"));
	for(i = 0; i < LASTGPU / GPUFIELDS; i ++)
		asprintf(&data->objects[FRAMGPU1 + i],	_("Card %i"), i);
	asprintf(&data->objects[FRAMBANKS],		_("Banks"));
	asprintf(&data->objects[FRAMOS],		_("Operating System"));
	asprintf(&data->objects[FRAMMEMORY],		_("Memory"));
	asprintf(&data->objects[FRAMABOUT],		_("About"));
	asprintf(&data->objects[FRAMLICENSE],		_("License"));
	asprintf(&data->objects[LABVERSION],		_("Version %s"), PRGVER);
	asprintf(&data->objects[LABDESCRIPTION],	_(
		"%s is a Free software that gathers information\n"
		"on CPU, motherboard and more."), PRGNAME);
	asprintf(&data->objects[LABAUTHOR],		_("Author : %s"), PRGAUTH);
	asprintf(&data->objects[LABCOPYRIGHT],		"%s", PRGCPYR);
	asprintf(&data->objects[LABLICENSE],		_(
		"This program comes with ABSOLUTELY NO WARRANTY"));

	/* Tab CPU */
	asprintf(&data->tabcpu[NAME][VENDOR],		_("Vendor"));
	asprintf(&data->tabcpu[NAME][CODENAME],		_("Code Name"));
	asprintf(&data->tabcpu[NAME][PACKAGE],		_("Package"));
	asprintf(&data->tabcpu[NAME][TECHNOLOGY],	_("Technology"));
	asprintf(&data->tabcpu[NAME][VOLTAGE],		_("Voltage"));
	asprintf(&data->tabcpu[NAME][SPECIFICATION],	_("Specification"));
	asprintf(&data->tabcpu[NAME][FAMILY],		_("Family"));
	asprintf(&data->tabcpu[NAME][EXTFAMILY],	_("Ext. Family"));
	asprintf(&data->tabcpu[NAME][MODEL],		_("Model"));
	asprintf(&data->tabcpu[NAME][EXTMODEL],		_("Ext. Model"));
	asprintf(&data->tabcpu[NAME][TEMPERATURE],	_("Temp."));
	asprintf(&data->tabcpu[NAME][STEPPING],		_("Stepping"));
	asprintf(&data->tabcpu[NAME][INSTRUCTIONS],	_("Instructions"));

	asprintf(&data->tabcpu[NAME][CORESPEED],	_("Core Speed"));
	asprintf(&data->tabcpu[NAME][MULTIPLIER],	_("Multiplier"));
	asprintf(&data->tabcpu[NAME][BUSSPEED],		_("Bus Speed"));
	asprintf(&data->tabcpu[NAME][USAGE],		_("Usage"));

	asprintf(&data->tabcpu[NAME][LEVEL1D],		_("L1 Data"));
	asprintf(&data->tabcpu[NAME][LEVEL1I],		_("L1 Inst."));
	asprintf(&data->tabcpu[NAME][LEVEL2],		_("Level 2"));
	asprintf(&data->tabcpu[NAME][LEVEL3],		_("Level 3"));

	asprintf(&data->tabcpu[NAME][SOCKETS],		_("Socket(s)"));
	asprintf(&data->tabcpu[NAME][CORES],		_("Core(s)"));
	asprintf(&data->tabcpu[NAME][THREADS],		_("Thread(s)"));

	/* Tab Cache */
	for(i = 0; i < LASTCACHE / CACHEFIELDS + 1; i++)
	{
		asprintf(&data->tabcache[NAME][i * CACHEFIELDS],	_("Size"));
		asprintf(&data->tabcache[NAME][i * CACHEFIELDS + 1],	_("Descriptor"));
		asprintf(&data->tabcache[NAME][i * CACHEFIELDS + 2],	_("Speed"));
	}

	/* Tab Motherboard */
	asprintf(&data->tabmb[NAME][MANUFACTURER],	_("Manufacturer"));
	asprintf(&data->tabmb[NAME][MBMODEL],		_("Model"));
	asprintf(&data->tabmb[NAME][REVISION],		_("Revision"));

	asprintf(&data->tabmb[NAME][BRAND],		_("Brand"));
	asprintf(&data->tabmb[NAME][BIOSVER],		_("Version"));
	asprintf(&data->tabmb[NAME][DATE],		_("Date"));
	asprintf(&data->tabmb[NAME][ROMSIZE],		_("ROM Size"));

	asprintf(&data->tabmb[NAME][CHIPVENDOR],	_("Vendor"));
	asprintf(&data->tabmb[NAME][CHIPNAME],		_("Model"));

	/* Tab RAM */
	for(i = 0; i < BANK7_1 / RAMFIELDS + 1; i++)
	{
		asprintf(&data->tabram[NAME][i * RAMFIELDS],	 _("Bank %i Ref."), i);
		asprintf(&data->tabram[NAME][i * RAMFIELDS + 1], _("Bank %i Type"), i);
	}

	/* Tab System */
	asprintf(&data->tabsys[NAME][KERNEL],		_("Kernel"));
	asprintf(&data->tabsys[NAME][DISTRIBUTION],	_("Distribution"));
	asprintf(&data->tabsys[NAME][HOSTNAME],		_("Hostname"));
	asprintf(&data->tabsys[NAME][UPTIME],		_("Uptime"));
	asprintf(&data->tabsys[NAME][COMPILER],		_("Compiler"));

	asprintf(&data->tabsys[NAME][USED],		_("Used"));
	asprintf(&data->tabsys[NAME][BUFFERS],		_("Buffers"));
	asprintf(&data->tabsys[NAME][CACHED],		_("Cached"));
	asprintf(&data->tabsys[NAME][FREE],		_("Free"));
	asprintf(&data->tabsys[NAME][SWAP],		_("Swap"));

	/* Tab Graphics */
	for(i = 0; i < LASTGPU; i += GPUFIELDS)
	{
		asprintf(&data->tabgpu[NAME][GPUVENDOR1 + i],	_("Vendor"));
		asprintf(&data->tabgpu[NAME][GPUNAME1 + i],	_("Model"));
		asprintf(&data->tabgpu[NAME][GPUTEMP1 + i],	_("Temperature"));
	}
}

/* Replace null pointers by character '\0' */
static int remove_null_ptr(Labels *data)
{
	int i, j, cpt = 0, ret = 0;
	char *msg;
	const struct Arrays { char **array; const int last; } a[] =
	{
		{ data->tabcpu[VALUE],   LASTCPU   },
		{ data->tabcache[VALUE], LASTCACHE },
		{ data->tabmb[VALUE],    LASTMB    },
		{ data->tabram[VALUE],   LASTRAM   },
		{ data->tabsys[VALUE],   LASTSYS   },
		{ data->tabgpu[VALUE],   LASTGPU   },
		{ NULL,                  0         }
	};

	MSG_VERBOSE(_("Replacing undefined labels by an empty string"));
	for(i = 0; a[i].array != NULL; i++)
	{
		for(j = 0; j < a[i].last; j++)
		{
			cpt++;
			if(a[i].array[j] == NULL)
				ret += !iasprintf(&a[i].array[j], NULL);
		}
	}

	asprintf(&msg, _("\tThere is %i/%i empty strings"), ret, cpt);
	MSG_VERBOSE(msg);
	return ret;
}

/* Dump all data in stdout */
static void dump_data(Labels *data)
{
	int i;

	MSG_VERBOSE(_("Dumping data..."));
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

	/* Tab Cache */
	printf("\n\n ***** %s *****\n", data->objects[TABCACHE]);
	printf("\t*** %s ***\n", data->objects[FRAMCACHEL1]);
	for(i = L1SIZE; i < LASTCACHE; i++)
	{
		if(i == L2SIZE)
			printf("\n\t*** %s ***\n", data->objects[FRAMCACHEL2]);
		else if(i == L3SIZE)
			printf("\n\t*** %s ***\n", data->objects[FRAMCACHEL3]);
		printf("%16s: %s\n", data->tabcache[NAME][i], data->tabcache[VALUE][i]);
	}

	/* Tab Motherboard */
	printf("\n\n ***** %s *****\n", data->objects[TABMB]);
	printf("\n\t*** %s ***\n", data->objects[FRAMMOBO]);
	for(i = MANUFACTURER; i < LASTMB; i++)
	{
		if(i == BRAND)
			printf("\n\t*** %s ***\n", data->objects[FRAMBIOS]);
		else if(i == CHIPVENDOR)
			printf("\n\t*** %s ***\n", data->objects[FRAMCHIP]);
		printf("%16s: %s\n", data->tabmb[NAME][i], data->tabmb[VALUE][i]);
	}

	/* Tab RAM */
	printf("\n\n ***** %s *****\n", data->objects[TABRAM]);
	printf("\n\t*** %s ***\n", data->objects[FRAMBANKS]);
	for(i = BANK0_0; i < data->dimms_count; i++)
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

	/* Tab Graphics */
	printf("\n\n ***** %s *****\n", data->objects[TABGPU]);
	printf("\n\t*** %s ***\n", data->objects[FRAMGPU1]);
	for(i = GPUVENDOR1; i < data->gpu_count; i++)
	{
		if(i == GPUVENDOR2)
			printf("\n\t*** %s ***\n", data->objects[FRAMGPU2]);
		else if(i == GPUVENDOR3)
			printf("\n\t*** %s ***\n", data->objects[FRAMGPU3]);
		else if(i == GPUVENDOR4)
			printf("\n\t*** %s ***\n", data->objects[FRAMGPU4]);
		printf("%16s: %s\n", data->tabgpu[NAME][i], data->tabgpu[VALUE][i]);
	}

	labels_free(data);
}


/************************* Update-related functions *************************/

/* Check if running version is latest */
static char *check_lastver(void)
{
	char newver[10];
	static char *ret = NULL;
	FILE *page = NULL;

	if(!command_exists("curl"))
	{
		ret = strdup("f");
		return ret;
	}

	if(ret != NULL)
		return ret;

	MSG_VERBOSE(_("Check for a new version..."));
	page = popen("curl -s https://api.github.com/repos/X0rg/CPU-X/releases/latest | grep 'tag_name' | awk -F '\"' '{ print $4 }' | cut -d'v' -f2", "r");

	/* Open file descriptor and put version number in variable */
	if(page == NULL)
	{
		MSG_ERROR(_("Failed to check on Internet."));
		ret = strdup("f");
		return ret;
	}

	fgets(newver, 9, page);
	pclose(page);
	newver[strlen(newver) - 1] = '\0';

	if(!strcmp(PRGVER, newver))
	{
		MSG_VERBOSE(_("No new version available."));
		ret = strdup("f");
	}
	else
	{
		MSG_VERBOSE(_("A new version is available!"));
		ret = strdup(newver);
	}

	return ret;
}

/* Apply new portable version if available */
static int update_prg(char *executable, Options *opts)
{
	int err = 0;
#if PORTABLE_BINARY
	int i = 0;
	char *newver, *opt, *portype, *tgzname, *cmd, *bin, *tmp;
	const char *ext[] = { "bsd32", "linux32", "linux64", "" };

	opt = opts->verbose ? strdup("") : strdup("s");
	newver = check_lastver();
	if(newver[0] == 'f')
		return 1;

	/* Find what archive we need to download */
	if(HAS_GTK)
		portype = strdup("portable");
	else
		portype = strdup("portable_noGTK");

	/* Download archive */
	MSG_VERBOSE(_("Downloading new version..."));
	asprintf(&tgzname, "%s_v%s_%s.tar.gz", PRGNAME, newver, portype);
	asprintf(&cmd, "curl -L%s https://github.com/%s/%s/releases/download/v%s/%s -o %s", opt, PRGAUTH, PRGNAME, newver, tgzname, tgzname);
	system(cmd);

	/* Extract archive */
	opt = opts->verbose ? strdup("v") : strdup("");
	asprintf(&cmd, "tar -zx%sf %s", opt, tgzname);
	system(cmd);

	free(opt);
	free(cmd);

# if defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	asprintf(&bin, "%s_v%s_%s.%s", PRGNAME, newver, portype, "bsd32");
# elif defined (__linux__) && defined (__LP64__)
	asprintf(&bin, "%s_v%s_%s.%s", PRGNAME, newver, portype, "linux64");
# elif defined (__linux__) && !defined (__LP64__)
	asprintf(&bin, "%s_v%s_%s.%s", PRGNAME, newver, portype, "linux32");
# endif /* OS */

	/* Rename new binary */
	MSG_VERBOSE(_("Applying new version..."));
	err = rename(bin, executable);
	if(err)
		MSG_VERBOSE(_("Error when updating."));
	else
		MSG_VERBOSE(_("Update successful!"));

	/* Delete temporary files */
	err = remove(tgzname);
	while(strcmp(ext[i], ""))
	{
		asprintf(&tmp, "%s_v%s_%s.%s", PRGNAME, newver, portype, ext[i]);
		if(strcmp(bin, tmp))
		{
			err += remove(tmp);
		}
		i++;
	}

	if(err)
		MSG_VERBOSE(_("Error when deleting temporary files."));

	free(newver);
	free(portype);
	free(tgzname);
	free(bin);
	free(tmp);
#endif /* PORTABLE_BINARY */
	return err;
}


/************************* Options-related functions *************************/

static const struct AvailableOpts
{
	const bool has_mod; const char short_opt; const char *long_opt; const int  need_arg;
} o[] =
{
	{ HAS_GTK,       'g', "gtk",       no_argument       },
	{ HAS_NCURSES,   'n', "ncurses",   no_argument       },
	{ true,          'd', "dump",      no_argument       },
	{ true,          'r', "refresh",   required_argument },
	{ HAS_DMIDECODE, 'D', "dmidecode", no_argument       },
	{ true,          'c', "color",     no_argument       },
	{ true,          'v', "verbose",   no_argument       },
	{ true,          'h', "help",      no_argument       },
	{ true,          'V', "version",   no_argument       },
	{ true,          '0', NULL,        0                 }
};

/* This is help display with --help option */
static void help(FILE *out, char *argv[], int exit_status)
{
	int i;
	const char *description[] =
	{
		_("Start graphical user interface (GUI) (default)"),
		_("Start text-based user interface (TUI)"),
		_("Dump all data on standard output and exit"),
		_("Set custom time between two refreshes (in seconds)"),
		_("Run embedded command dmidecode and exit"),
		_("Disable colored output"),
		_("Verbose output"),
		_("Print help and exit"),
		_("Print version and exit")
	};

	fprintf(out, _("Usage: %s [OPTION]\n\n"), argv[0]);
	fprintf(out, _("Available OPTION:\n"));
	for(i = 0; o[i].long_opt != NULL; i++)
	{
		if(o[i].has_mod)
			fprintf(out, "  -%c, --%-10s %s\n", o[i].short_opt, o[i].long_opt, description[i]);
	}

	exit(exit_status);
}

/* This is the --version option */
static void version(void)
{
	int i;
	char *strver, *newver = check_lastver();
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

	if(newver[0] == 'f')
		asprintf(&strver, _("(up-to-date)"));
	else
		asprintf(&strver, _("(version %s is available)"), newver);

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
			printf(_("-- %12s version: %s\n"), v[i].lib, v[i].version);
	}

	exit(EXIT_SUCCESS);
}

/* Parse options given in arg */
static void menu(int argc, char *argv[])
{
	int i, j = 0, c, tmp_refr = -1;
	struct option longopts[9];

	/* Filling longopts structure */
	for(i = 0; o[i].long_opt != NULL; i++)
	{
		if(o[i].has_mod)
		{
			longopts[j].name    = o[i].long_opt;
			longopts[j].has_arg = o[i].has_mod;
			longopts[j].flag    = 0;
			longopts[j].val     = o[i].short_opt;
			j++;
		}
	}

	/* Set the default mode */
	if(HAS_GTK)
		c = 'g';
	else if(HAS_NCURSES)
		c = 'n';
	else
		c = 'd';

	/* Parse options */
	do
	{
		switch(c)
		{
			case 'g':
				if(HAS_GTK)
					opts->output_type = OUT_GTK;
				else
					help(stderr, argv, EXIT_FAILURE);
				break;
			case 'n':
				if(HAS_NCURSES)
					opts->output_type = OUT_NCURSES;
				else
					help(stderr, argv, EXIT_FAILURE);
				break;
			case 'd':
				opts->output_type = OUT_DUMP;
				break;
			case 'r':
				tmp_refr = atoi(optarg);
				if(tmp_refr > 1)
					opts->refr_time = tmp_refr;
				break;
			case 'D':
				if(HAS_DMIDECODE)
					opts->output_type = OUT_DMIDECODE;
				else
					help(stderr, argv, EXIT_FAILURE);
				break;
			case 'c':
				opts->color = false;
				break;
			case 'v':
				opts->verbose = true;
				break;
			case 'h':
				help(stdout, argv, EXIT_SUCCESS);
			case 'V':
				version();
			case '?':
			default:
				help(stderr, argv, EXIT_FAILURE);
		}
	} while((c = getopt_long(argc, argv, ":gndr:DcvhV", longopts, NULL)) != -1);
}


/************************* Main-related functions *************************/

/* Extract locales in /tmp/.cpu-x */
static int extract_locales(void)
{
	int err = 0;
#if PORTABLE_BINARY && HAS_GETTEXT
	int i;
	char *path;
	FILE *mofile;

	/* Write .mo files in temporary directory */
	MSG_VERBOSE("Extracting translations in temporary directory");
	asprintf(&path, "%s", LOCALEDIR);
	err = mkdir(path, 0777);

	for(i = 0; ptrlen[i] != NULL; i++)
	{
		asprintf(&path, "%s/%s", LOCALEDIR, lang[i]);
		err += mkdir(path, 0777);

		asprintf(&path, "%s/%s/LC_MESSAGES", LOCALEDIR, lang[i]);
		err += mkdir(path, 0777);

		asprintf(&path, "%s/%s/LC_MESSAGES/%s.mo", LOCALEDIR, lang[i], GETTEXT_PACKAGE);

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
	return err;

}

 /* Enable internationalization support */
static int set_locales(void)
{
	int i;
	char *out[3] = { NULL };

	extract_locales();
	MSG_VERBOSE("Setting locale");
	/* Apply locale */
	setlocale(LC_ALL, "");
	out[0] = bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	out[1] = bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	out[2] = textdomain(GETTEXT_PACKAGE);

	/* Check if something is wrong */
	for(i = 0; i < 3 && out[i] != NULL; i++);
	if(out[i] == NULL)
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
	Labels data = { NULL };
	opts = &(Options) { .output_type = 0, .refr_time = 1, .verbose = false, .color = true };
	menu(argc, argv);

	/* If option --dmidecode is passed, start dmidecode and exit */
	if(HAS_DMIDECODE && !getuid() && (opts->output_type & OUT_DMIDECODE))
		return libdmi('D');

	set_locales();
	labels_setname(&data);
	fill_labels(&data);
	remove_null_ptr(&data);

	if(getuid())
		MSG_WARNING("WARNING: root privileges are required to work properly\n");

	/* Show data */
	switch(opts->output_type)
	{
		case OUT_GTK:
			if(HAS_GTK)	start_gui_gtk(&argc, &argv, &data);
			break;
		case OUT_NCURSES:
			if(HAS_NCURSES)	start_tui_ncurses(&data);
			break;
		case OUT_DUMP:
			dump_data(&data);
			break;
	}

	if(PORTABLE_BINARY)
		update_prg(argv[0], opts);

	return EXIT_SUCCESS;
}


/************************* Public functions *************************/

/* Print a formatted message */
int message(char type, char *msg, char *basefile, int line)
{
	switch(type)
	{
		case 'v': /* Verbose message */
			return opts->verbose ? fprintf(stdout, "%s%s\n" RESET, opts->color ? BOLD_GREEN : "", msg) : -1;
		case 'w': /* Warning message */
			return fprintf(stdout, "%s%s\n" RESET, opts->color ? BOLD_YELLOW : "", msg);
		case 'e': /* Error message */
			return fprintf(stderr, "%s%s:%s:%i: %s\n" RESET, opts->color ? BOLD_RED : "", PRGNAME, basefile, line, msg);
		case 'n': /* Error message with errno */
			return fprintf(stderr, "%s%s:%s:%i: %s (%s)\n" RESET, opts->color ? BOLD_RED : "", PRGNAME, basefile, line, msg, strerror(errno));
	}

	return -1;
}

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
	int arg_int, i, ret;
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
		return asprintf(str, fmt);

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
				case 'd':
				case 'i':
					is_format = false;
					arg_int = va_arg(aptr, int);
					if(arg_int > 0)
						ret = asprintf(str, tmp_fmt, *str, arg_int);
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

/* Open a file or a pipe and put its content in buffer */
int xopen_to_str(char *file, char **buffer, char type)
{
	char *test_command;
	FILE *f = NULL;

	/* Allocate buffer */
	if((*buffer = malloc(MAXSTR * sizeof(char))) == NULL)
	{
		MSG_ERROR_ERRNO(_("xopen_to_str(): malloc() failed"));
		return 1;
	}


	if(type == 'f')
	{
		/* Open file */
		if((f = fopen(file, "r")) == NULL)
		{
			MSG_ERROR_ERRNO("xopen_to_str(): fopen() failed");
			return 2;
		}
	}
	else if(type == 'p')
	{
		/* Open pipe */
		asprintf(&test_command, file);
		if(!command_exists(strtok(test_command, " ")))
			return 2;

		if((f = popen(file, "r")) == NULL)
		{
			MSG_ERROR_ERRNO(_("xopen_to_str(): popen() failed"));
			return 3;
		}
	}
	else
	{
		MSG_ERROR("(internal) bad use of xopen_to_str() function");
		return -1;
	}

	/* Get string from file descriptor */
	if(fgets(*buffer, MAXSTR, f) == NULL)
	{
		MSG_ERROR_ERRNO(_("xopen_to_str(): fgets() failed"));
		fclose(f);
		return 4;
	}

	(*buffer)[strlen(*buffer) - 1] = '\0';
	fclose(f);

	return 0;
}

/* Free memory after display labels */
void labels_free(Labels *data)
{
	int i;

	MSG_VERBOSE(_("Freeing memory"));
	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		free(data->tabcpu[NAME][i]);
		data->tabcpu[NAME][i] = NULL;

		if(i != MULTIPLIER && i != LEVEL1I && i != LEVEL2 && i != LEVEL3)
		{
			free(data->tabcpu[VALUE][i]);
			data->tabcpu[VALUE][i] = NULL;
		}
	}

	/* Tab Cache */
	for(i = L1SIZE; i < LASTCACHE; i++)
	{
		free(data->tabcache[NAME][i]);
		data->tabcache[NAME][i] = NULL;

		free(data->tabcache[VALUE][i]);
		data->tabcache[VALUE][i] = NULL;
	}

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMB; i++)
	{
		free(data->tabmb[NAME][i]);
		data->tabmb[NAME][i] = NULL;

		free(data->tabmb[VALUE][i]);
		data->tabmb[VALUE][i] = NULL;
	}

	/* Tab RAM */
	for(i = BANK0_0; i < LASTRAM; i++)
	{
		free(data->tabram[NAME][i]);
		data->tabram[NAME][i] = NULL;

		free(data->tabram[VALUE][i]);
		data->tabram[VALUE][i] = NULL;
	}

	/* Tab System */
	for(i = KERNEL; i < LASTSYS; i++)
	{
		free(data->tabsys[NAME][i]);
		data->tabsys[NAME][i] = NULL;

		if(i != USED && i != BUFFERS && i != CACHED && i != FREE && i != SWAP)
		{
			free(data->tabsys[VALUE][i]);
			data->tabsys[VALUE][i] = NULL;
		}
	}

	/* Tab Graphics */
	for(i = GPUVENDOR1; i < LASTGPU; i++)
	{
		free(data->tabgpu[NAME][i]);
		data->tabgpu[NAME][i] = NULL;

		free(data->tabgpu[VALUE][i]);
		data->tabgpu[VALUE][i] = NULL;
	}
}
