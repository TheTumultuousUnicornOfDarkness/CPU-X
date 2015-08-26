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
* cpu-x.c
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
#include "cpu-x.h"

#if EMBED
# include <sys/stat.h>
# if GETTEXT
#  include "../po/mo.h"
# endif
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


int main(int argc, char *argv[])
{
	/* Parse options */
	char option;
	option = menu(argc, argv);

	/* If option --dmidecode is passed, start dmidecode and exit */
	if(HAS_LIBDMI && option == 'D')
		return libdmi(option);

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
	Labels data;
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
	if(HAS_GTK && option == 'G') /* Start GTK3 GUI */
		start_gui_gtk(&argc, &argv, &data);
	else if(HAS_NCURSES && option == 'n') /* Start NCurses TUI */
		start_tui_ncurses(&data);
	else if(option == 'd') /* Just dump data and exit */
		dump_data(&data);

	/* If compiled without UI */
	if(!HAS_GTK && !HAS_NCURSES && option != 'd')
	{
		fprintf(stderr, "%s is compiled without GUI support. Dumping data...\n\n", PRGNAME);
		dump_data(&data);
	}

#ifdef EMBED
	update_prg(argv[0]);
#endif /* EMBED */

	return EXIT_SUCCESS;
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

	else if(type == 'v' && (verbose == 1 || verbose == 3))
		printf("%s%s%s\n", boldgre, msg, reset);
}

/* Initialize all labels pointers to null */
void labels_setnull(Labels *data)
{
	int i;

	MSGVERB(_("Setting label pointers"));
	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
		data->tabcpu[VALUE][i] = NULL;

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
	asprintf(&data->objects[TABMB],			_("Motherboard"));
	asprintf(&data->objects[TABRAM],		_("RAM"));
	asprintf(&data->objects[TABSYS],		_("System"));
	asprintf(&data->objects[TABGPU],		_("Graphics"));
	asprintf(&data->objects[TABABOUT],		_("About"));
	asprintf(&data->objects[FRAMPROCESSOR],		_("Processor"));
	asprintf(&data->objects[FRAMCLOCKS],		_("Clocks"));
	asprintf(&data->objects[FRAMCACHE],		_("Cache"));
	asprintf(&data->objects[FRAMMOBO],		_("Motherboard"));
	asprintf(&data->objects[FRAMBIOS],		_("BIOS"));
	asprintf(&data->objects[FRAMCHIP],		_("Chipset"));
	asprintf(&data->objects[FRAMGPU1],		_("Card 0"));
	asprintf(&data->objects[FRAMGPU2],		_("Card 1"));
	asprintf(&data->objects[FRAMGPU3],		_("Card 2"));
	asprintf(&data->objects[FRAMGPU4],		_("Card 3"));
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

	asprintf(&data->tabcpu[NAME][SOCKETS],		_("Sockets(s)"));
	asprintf(&data->tabcpu[NAME][CORES],		_("Core(s)"));
	asprintf(&data->tabcpu[NAME][THREADS],		_("Thread(s)"));

	/* Tab Motherboard */
	asprintf(&data->tabmb[NAME][MANUFACTURER],	_("Manufacturer"));
	asprintf(&data->tabmb[NAME][MBMODEL],		_("Model"));
	asprintf(&data->tabmb[NAME][REVISION],		_("Revision"));

	asprintf(&data->tabmb[NAME][BRAND],		_("Brand"));
	asprintf(&data->tabmb[NAME][VERSION],		_("Version"));
	asprintf(&data->tabmb[NAME][DATE],		_("Date"));
	asprintf(&data->tabmb[NAME][ROMSIZE],		_("ROM Size"));

	asprintf(&data->tabmb[NAME][CHIPVENDOR],	_("Vendor"));
	asprintf(&data->tabmb[NAME][CHIPNAME],		_("Model"));


	/* Tab RAM */
	asprintf(&data->tabram[NAME][BANK0_0],		_("Bank 0 Ref."));
	asprintf(&data->tabram[NAME][BANK0_1],		_("Bank 0 Type"));
	asprintf(&data->tabram[NAME][BANK1_0],		_("Bank 1 Ref."));
	asprintf(&data->tabram[NAME][BANK1_1],		_("Bank 1 Type"));
	asprintf(&data->tabram[NAME][BANK2_0],		_("Bank 2 Ref."));
	asprintf(&data->tabram[NAME][BANK2_1],		_("Bank 2 Type"));
	asprintf(&data->tabram[NAME][BANK3_0],		_("Bank 3 Ref."));
	asprintf(&data->tabram[NAME][BANK3_1],		_("Bank 3 Type"));
	asprintf(&data->tabram[NAME][BANK4_0],		_("Bank 4 Ref."));
	asprintf(&data->tabram[NAME][BANK4_1],		_("Bank 4 Type"));
	asprintf(&data->tabram[NAME][BANK5_0],		_("Bank 5 Ref."));
	asprintf(&data->tabram[NAME][BANK5_1],		_("Bank 5 Type"));
	asprintf(&data->tabram[NAME][BANK6_0],		_("Bank 6 Ref."));
	asprintf(&data->tabram[NAME][BANK6_1],		_("Bank 6 Type"));
	asprintf(&data->tabram[NAME][BANK7_0],		_("Bank 7 Ref."));
	asprintf(&data->tabram[NAME][BANK7_1],		_("Bank 7 Type"));

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

		if(i != MULTIPLIER)
		{
			free(data->tabcpu[VALUE][i]);
			data->tabcpu[VALUE][i] = NULL;
		}
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

#if HAS_LIBCPUID
/* Elements provided by libcpuid library */
int libcpuid(Labels *data)
{
	int err = 0;
	char *tmp;
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

	if(datanr.l1_data_cache > 0)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL1D],	"%d x %4d KB", datanr.num_cores, datanr.l1_data_cache);
		if(datanr.l1_assoc > 0)
		{
			asprintf(&tmp, ", %2d-way", datanr.l1_assoc);
			data->tabcpu[VALUE][LEVEL1D] = realloc(data->tabcpu[VALUE][LEVEL1D],
				(strlen(data->tabcpu[VALUE][LEVEL1D]) + strlen(tmp) + 1) * sizeof(char));
			strcat(data->tabcpu[VALUE][LEVEL1D], tmp);
			free(tmp);
		}
	}

	if(datanr.l1_instruction_cache > 0)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL1I],	"%d x %4d KB", datanr.num_cores, datanr.l1_instruction_cache);
		if(datanr.l1_assoc > 0)
		{
			asprintf(&tmp, ", %2d-way", datanr.l1_assoc);
			data->tabcpu[VALUE][LEVEL1I] = realloc(data->tabcpu[VALUE][LEVEL1I],
				(strlen(data->tabcpu[VALUE][LEVEL1I]) + strlen(tmp) + 1) * sizeof(char));
			strcat(data->tabcpu[VALUE][LEVEL1I], tmp);
			free(tmp);
		}
	}

	if(datanr.l2_cache > 0)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL2],	"%d x %4d KB", datanr.num_cores, datanr.l2_cache);
		if(datanr.l1_assoc > 0)
		{
			asprintf(&tmp, ", %2d-way", datanr.l2_assoc);
			data->tabcpu[VALUE][LEVEL2] = realloc(data->tabcpu[VALUE][LEVEL2],
				(strlen(data->tabcpu[VALUE][LEVEL2]) + strlen(tmp) + 1) * sizeof(char));
			strcat(data->tabcpu[VALUE][LEVEL2], tmp);
			free(tmp);
		}
	}

	if(datanr.l3_cache > 0)
	{
		asprintf(&data->tabcpu[VALUE][LEVEL3],	"%d x %4d KB", datanr.num_cores, datanr.l3_cache);
		if(datanr.l1_assoc > 0)
		{
			asprintf(&tmp, ", %2d-way", datanr.l3_assoc);
			data->tabcpu[VALUE][LEVEL3] = realloc(data->tabcpu[VALUE][LEVEL3],
				(strlen(data->tabcpu[VALUE][LEVEL3]) + strlen(tmp) + 1) * sizeof(char));
			strcat(data->tabcpu[VALUE][LEVEL3], tmp);
			free(tmp);
		}
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
void instructions(char **arch, char **instr)
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

		if(id.flags[CPU_FEATURE_LM])		*arch = strdupnullok("x86_64 (64-bit)");
		else					*arch = strdupnullok("ix86 (32-bit)");
	}
	else
		MSGSERR(_("libcpuid failed"));
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
	int i;
	char path[PATH_MAX], buff[MAXSTR];
	const char *id[LASTMB] = { "board_vendor", "board_name", "board_version", "bios_vendor", "bios_version", "bios_date" };
	FILE *mb[LASTMB] = { NULL };

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < ROMSIZE; i++)
	{
		snprintf(path, PATH_MAX, "%s/%s", SYS_DMI, id[i]);
		mb[i] = fopen(path, "r");
		if(mb[i] != NULL)
		{
			fgets(buff, MAXSTR, mb[i]);
			data->tabmb[VALUE][i] = strdupnullok(buff);
			data->tabmb[VALUE][i][ strlen(data->tabmb[VALUE][i]) - 1 ] = '\0';
			fclose(mb[i]);
		}
		else
			err++;
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
	struct pci_access *pacc;
	struct pci_dev *dev;
	char namebuf[MAXSTR], *vendor, *product, *driver;

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
			driver = find_driver(dev, namebuf);
			data->tabgpu[VALUE][GPUVENDOR1  + nbgpu * GPUFIELDS] = strdupnullok(vendor);
			data->tabgpu[VALUE][GPUNAME1	+ nbgpu * GPUFIELDS] = strdupnullok(product);
			data->tabgpu[VALUE][GPUDRIVER1  + nbgpu * GPUFIELDS] = strdupnullok(driver);
			nbgpu++;
		}
	}

	pci_cleanup(pacc);	/* Close everything */
	free(vendor);
	free(product);
}
#endif /* HAS_LIBPCI */

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

/* Duplicate a not null string */
char *strdupnullok(const char *s)
{
	return (s != NULL) ? strdup(s) : NULL;
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

	opt = verbose ? strdup("") : strdup("s");
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
	opt = verbose ? strdup("v") : strdup("");
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
