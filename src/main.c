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
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <locale.h>
#include <libintl.h>
#include "core.h"
#include "options.h"

#ifdef EMBED
# include <sys/stat.h>
# ifdef GETTEXT
#  include "../po/mo.h"
# endif
#endif

/* Options are global */
unsigned int flags;


int main(int argc, char *argv[])
{
	/* Parse options */
	Labels data;
	flags = 0;
	data.refr_time = menu(argc, argv);

	/* If option --dmidecode is passed, start dmidecode and exit */
	if(HAS_LIBDMI && !getuid() && (flags & OPT_DMIDECODE))
		return libdmi('D');

#if defined(EMBED) && defined (GETTEXT)
	int i;
	char *path;
	FILE *mofile;

	/* Write .mo files in temporary directory */
	MSGVERB("Extract translations");
	asprintf(&path, "%s", LOCALEDIR);
	mkdir(path, 0777);

	for(i = 0; ptrlen[i] != NULL; i++)
	{
		asprintf(&path, "%s/%s", LOCALEDIR, lang[i]);
		mkdir(path, 0777);

		asprintf(&path, "%s/%s/LC_MESSAGES", LOCALEDIR, lang[i]);
		mkdir(path, 0777);

		asprintf(&path, "%s/%s/LC_MESSAGES/%s.mo", LOCALEDIR, lang[i], GETTEXT_PACKAGE);

		mofile = fopen(path, "w");
		if(mofile != NULL)
		{
			fwrite(ptrlang[i], sizeof(unsigned char), *(ptrlen)[i], mofile);
			fclose(mofile);
		}
	}
#endif /* EMBED && GETTEXT */

	/* Start collecting data */
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	MSGVERB(_("Setting locale done"));

	labels_setnull(&data);
	labels_setname(&data);
	bogomips(&data.tabcpu[VALUE][BOGOMIPS]);
	tabsystem(&data);

	if(HAS_LIBCPUID)
	{
		if(libcpuid(&data))
			MSGSERR(_("libcpuid failed"));
		else
		{
			cpuvendor(data.tabcpu[VALUE][VENDOR]);
			instructions(&data.tabcpu[VALUE][ARCHITECTURE], &data.tabcpu[VALUE][INSTRUCTIONS]);

			if(strcmp(data.tabcpu[VALUE][CORES], data.tabcpu[VALUE][THREADS]))
				strcat(data.tabcpu[VALUE][INSTRUCTIONS], ", HT");

			if(HAS_LIBBDWT)
			{
				if(bandwidth(&data))
					MSGSERR(_("bandwidth failed"));
			}
		}
	}

	if(HAS_LIBDMI && !getuid())
	{
		if(libdmidecode(&data))
			MSGSERR(_("libdmidecode failed"));
	}
	else
	{
		if(libdmi_fallback(&data))
			MSGSERR(_("libdmi_fallback failed"));
	}

	if(HAS_LIBPCI)
		pcidev(&data);

	cpufreq(&data);
	labels_delnull(&data);

	/* Show data */
	if(HAS_GTK && (flags & OPT_GTK)) /* Start GTK3 GUI */
		start_gui_gtk(&argc, &argv, &data);
	else if(HAS_NCURSES && (flags & OPT_NCURSES)) /* Start NCurses TUI */
		start_tui_ncurses(&data);
	else if(flags & OPT_DUMP) /* Just dump data and exit */
		dump_data(&data);

	/* If compiled without UI */
	if(!HAS_GTK && !HAS_NCURSES && !(flags & OPT_DUMP))
	{
		fprintf(stderr, "%s is compiled without GUI support. Dumping data...\n\n", PRGNAME);
		dump_data(&data);
	}

#ifdef EMBED
	update_prg(argv[0]);
#endif /* EMBED */

	return EXIT_SUCCESS;
}

const char *optstring[] =
{
	"gtk",
	"ncurses",
	"dump",
	"refresh",
#if HAS_LIBDMI
	"dmidecode",
#endif /* HAS_LIBDMI */
	"verbose",
	"help",
	"version"
};

/* This is help display with option --help */
void help(FILE *out, char *argv[])
{
	int o = 0;

	fprintf(out, _("Usage: %s [OPTION]\n\n"), argv[0]);
	fprintf(out, _("Available OPTION:\n"));
	fprintf(out, _("  -g, --%-10s Start graphical user interface (GUI) (default)\n"), optstring[o]); o++;
	fprintf(out, _("  -n, --%-10s Start text-based user interface (TUI)\n"), optstring[o]); o++;
	fprintf(out, _("  -d, --%-10s Dump all data on standard output and exit\n"), optstring[o]); o++;
	fprintf(out, _("  -r, --%-10s Set custom time between two refreshes (in seconds)\n"), optstring[o]); o++;
#if HAS_LIBDMI
	fprintf(out, _("  -D, --%-10s Run embedded command dmidecode and exit\n"), optstring[o]); o++;
#endif /* HAS_LIBDMI */
	fprintf(out, _("  -v, --%-10s Verbose output\n"), optstring[o]); o++;
	fprintf(out, _("  -h, --%-10s Print help and exit\n"), optstring[o]); o++;
	fprintf(out, _("  -V, --%-10s Print version and exit\n"), optstring[o]); o++;
}

/* This is the --version option */
void version(void)
{
	char *strver, *newver = check_lastver();

	if(newver[0] == 'f')
		asprintf(&strver, _("(up-to-date)"));
	else
		asprintf(&strver, _("(version %s is available)"), newver);

	printf(_("%s %s %s\n"
	"%s\n\n"
	"This is free software: you are free to change and redistribute it.\n"
	"This program comes with ABSOLUTELY NO WARRANTY\n"
	"See the GPLv3 license: <http://www.gnu.org/licenses/gpl.txt>\n\n"
	"Compiled on %s, %s, with compiler version %s.\n"),
	PRGNAME, PRGVER, strver, PRGCPYR, __DATE__, __TIME__, __VERSION__);
}

/* Parse options given in arg */
int menu(int argc, char *argv[])
{
	int c, tmp_refr = -1;

	const struct option longopts[] =
	{
		{optstring[0],	no_argument, 0, 'g'}, /* Arg gtk */
		{optstring[1],	no_argument, 0, 'n'}, /* Arg ncurses */
		{optstring[2],	no_argument, 0, 'd'}, /* Arg dump */
		{optstring[3],	required_argument, 0, 'r'}, /* Arg refresh */
#if HAS_LIBDMI
		{optstring[4],	no_argument, 0, 'D'}, /* Arg Dmidecode */
#endif /* HAS_LIBDMI */
		{optstring[5],	no_argument, 0, 'v'}, /* Arg verbose */
		{optstring[6],	no_argument, 0, 'h'}, /* Arg help */
		{optstring[7],	no_argument, 0, 'V'}, /* Arg version */
		{0,		0,	     0,  0}
	};

	while((c = getopt_long(argc, argv, ":gndr:DvhV", longopts, NULL)) != -1)
	{
		switch(c)
		{
			case 'g':
				flags |= OPT_GTK;
				break;
			case 'n':
				flags |= OPT_NCURSES;
				break;
			case 'd':
				flags |= OPT_DUMP;
				break;
			case 'r':
				tmp_refr = atoi(optarg);
				break;
#if HAS_LIBDMI
			case 'D':
				flags |= OPT_DMIDECODE;
				break;
#endif /* HAS_LIBDMI */
			case 'v':
				flags |= OPT_VERBOSE;
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

	if(!((flags & OPT_NCURSES) || (flags & OPT_DUMP)))
		 flags |= OPT_GTK;

	return (tmp_refr == -1) ? 1 : tmp_refr;
}

/* Print a formatted message */
void msg(char type, char *msg, char *prgname, char *basefile, int line)
{
	const char *reset = "\033[0m";
	const char *boldred = "\033[1;31m";
	const char *boldgre = "\033[1;32m";

	if(type == 'p')
	{
		fprintf(stderr, "%s%s:%s:%i: ", boldred, prgname, basefile, line);
		perror(msg);
		fprintf(stderr, "%s\n", reset);
	}

	else if(type == 'e')
		fprintf(stderr, "%s%s:%s:%i: %s%s\n", boldred, prgname, basefile, line, msg, reset);

	else if(type == 'v' && (flags & OPT_VERBOSE))
		printf("%s%s%s\n", boldgre, msg, reset);
}

/* Duplicate a not null string */
char *strdupnullok(const char *s)
{
	return (s != NULL) ? strdup(s) : NULL;
}

/* Initialize all labels pointers to null */
void labels_setnull(Labels *data)
{
	int i;

	MSGVERB(_("Setting label pointers"));
	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
		data->tabcpu[VALUE][i] = NULL;

	/* Tab Cache */
	for(i = L1SIZE; i < LASTCACHE; i++)
		data->tabcache[VALUE][i] = NULL;

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMB; i++)
		data->tabmb[VALUE][i] = NULL;

	/* Tab RAM */
	for(i = BANK0_0; i < LASTRAM; i++)
		data->tabram[VALUE][i] = NULL;

	/* Tab System */
	for(i = KERNEL; i < LASTSYS; i++)
		data->tabsys[VALUE][i] = NULL;

	/* Tab Graphics */
	for(i = GPUVENDOR1; i < LASTGPU; i++)
		data->tabgpu[VALUE][i] = NULL;
}

/* Set labels name */
void labels_setname(Labels *data)
{
	int i;

	MSGVERB(_("Setting label names"));
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
	asprintf(&data->tabcpu[NAME][ARCHITECTURE],	_("Architecture"));
	asprintf(&data->tabcpu[NAME][SPECIFICATION],	_("Specification"));
	asprintf(&data->tabcpu[NAME][FAMILY],		_("Family"));
	asprintf(&data->tabcpu[NAME][EXTFAMILY],	_("Ext. Family"));
	asprintf(&data->tabcpu[NAME][MODEL],		_("Model"));
	asprintf(&data->tabcpu[NAME][EXTMODEL],		_("Ext. Model"));
	asprintf(&data->tabcpu[NAME][STEPPING],		_("Stepping"));
	asprintf(&data->tabcpu[NAME][INSTRUCTIONS],	_("Instructions"));

	asprintf(&data->tabcpu[NAME][CORESPEED],	_("Core Speed"));
	asprintf(&data->tabcpu[NAME][MULTIPLIER],	_("Multiplier"));
	asprintf(&data->tabcpu[NAME][BUSSPEED],		_("Bus Speed"));
	asprintf(&data->tabcpu[NAME][BOGOMIPS],		_("BogoMIPS"));

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
		asprintf(&data->tabgpu[NAME][GPUDRIVER1 + i],	_("Driver"));
	}
}

/* Replace null pointers by character '\0' */
void labels_delnull(Labels *data)
{
	int i;

	MSGVERB(_("Replace undefined label by empty string"));
	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		if(data->tabcpu[VALUE][i] == NULL)
		{
			data->tabcpu[VALUE][i] = malloc(1 * sizeof(char));
			data->tabcpu[VALUE][i][0] = '\0';
		}
	}

	/* Tab Cache */
	for(i = L1SIZE; i < LASTCACHE; i++)
	{
		if(data->tabcache[VALUE][i] == NULL)
		{
			data->tabcache[VALUE][i] = malloc(1 * sizeof(char));
			data->tabcache[VALUE][i][0] = '\0';
		}
	}

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMB; i++)
	{
		if(data->tabmb[VALUE][i] == NULL)
		{
			data->tabmb[VALUE][i] = malloc(1 * sizeof(char));
			data->tabmb[VALUE][i][0] = '\0';
		}
	}

	/* Tab RAM */
	for(i = BANK0_0; i < LASTRAM; i++)
	{
		if(data->tabram[VALUE][i] == NULL)
		{
			data->tabram[VALUE][i] = malloc(1 * sizeof(char));
			data->tabram[VALUE][i][0] = '\0';
		}
	}

	/* Tab System */
	for(i = KERNEL; i < LASTSYS; i++)
	{
		if(data->tabsys[VALUE][i] == NULL)
		{
			data->tabsys[VALUE][i] = malloc(1 * sizeof(char));
			data->tabsys[VALUE][i][0] = '\0';
		}
	}

	/* Tab Graphics */
	for(i = GPUVENDOR1; i < LASTGPU; i++)
	{
		if(data->tabgpu[VALUE][i] == NULL)
		{
			data->tabgpu[VALUE][i] = malloc(1 * sizeof(char));
			data->tabgpu[VALUE][i][0] = '\0';
		}
	}
}

/* Free memory after display labels */
void labels_free(Labels *data)
{
	int i;

	MSGVERB(_("Freeing memory"));
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

/* Dump all data in stdout */
void dump_data(Labels *data)
{
	int i;

	MSGVERB(_("Dumping data..."));
	if(getuid())
		fprintf(stderr, "\n\t\t\t\033[1;33m%s\033[0m\n", MSGROOT);

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

	/* Tab Graphics */
	printf("\n\n ***** %s *****\n", data->objects[TABGPU]);
	printf("\n\t*** %s ***\n", data->objects[FRAMGPU1]);
	for(i = GPUVENDOR1; i < last_gpu(data); i++)
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

/* Check if running version is latest */
char *check_lastver(void)
{
	char newver[S];
	static char *ret = NULL;
	FILE *page = NULL;

	if(ret != NULL)
		return ret;

	MSGVERB(_("Check for a new version..."));
	page = popen("curl -s https://api.github.com/repos/X0rg/CPU-X/releases/latest | grep 'tag_name' | awk -F '\"' '{ print $4 }' | cut -d'v' -f2", "r");

	/* Open file descriptor and put version number in variable */
	if(page == NULL)
	{
		MSGSERR(_("Failed to check on Internet."));
		ret = strdup("f");
		return ret;
	}

	fgets(newver, S - 1, page);
	pclose(page);
	newver[strlen(newver) - 1] = '\0';

	if(!strcmp(PRGVER, newver))
	{
		MSGVERB(_("No new version available."));
		ret = strdup("f");
	}
	else
	{
		MSGVERB(_("A new version is available!"));
		ret = strdup(newver);
	}

	return ret;
}

/* Apply new portable version if available */
int update_prg(char *executable)
{
#ifdef EMBED
	int err = 0, i = 0;
	char *newver, *opt, *portype, *tgzname, *cmd, *bin, *tmp;
	const char *ext[] = { "bsd32", "linux32", "linux64", "" };

	opt = (flags & OPT_VERBOSE) ? strdup("") : strdup("s");
	newver = check_lastver();
	if(newver[0] == 'f')
		return 1;

	/* Find what archive we need to download */
	if(HAS_GTK)
		portype = strdup("portable");
	else
		portype = strdup("portable_noGTK");

	/* Download archive */
	MSGVERB(_("Downloading new version..."));
	asprintf(&tgzname, "%s_v%s_%s.tar.gz", PRGNAME, newver, portype);
	asprintf(&cmd, "curl -L%s https://github.com/%s/%s/releases/download/v%s/%s -o %s", opt, PRGAUTH, PRGNAME, newver, tgzname, tgzname);
	system(cmd);

	/* Extract archive */
	opt = (flags & OPT_VERBOSE) ? strdup("v") : strdup("");
	asprintf(&cmd, "tar -zx%sf %s", opt, tgzname);
	system(cmd);

	free(opt);
	free(cmd);

#if defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	asprintf(&bin, "%s_v%s_%s.%s", PRGNAME, newver, portype, "bsd32");
#elif defined (__linux__) && defined (__LP64__)
	asprintf(&bin, "%s_v%s_%s.%s", PRGNAME, newver, portype, "linux64");
#elif defined (__linux__) && !defined (__LP64__)
	asprintf(&bin, "%s_v%s_%s.%s", PRGNAME, newver, portype, "linux32");
#endif /* OS */

	/* Rename new binary */
	MSGVERB(_("Applying new version..."));
	err = rename(bin, executable);
	if(err)
		MSGVERB(_("Error when updating."));
	else
		MSGVERB(_("Update successful!"));

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
		MSGVERB(_("Error when deleting temporary files."));

	free(newver);
	free(portype);
	free(tgzname);
	free(bin);
	free(tmp);

#endif /* EMBED */
	return 0;
}
