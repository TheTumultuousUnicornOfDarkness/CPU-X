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
* FILE core.h
*/

#ifndef _CPUX_H_
#define _CPUX_H_

#include <stdint.h>
#include <stdbool.h>
#define HAVE_STDINT_H         /* Skip conflicts with <libcpuid/libcpuid_types.h> */

/* Software definition */
#define PRGNAME	              "CPU-X"
#define PRGAUTH	              "X0rg"
#define PRGCPYR               "Copyright © 2014-2016 Xorg"

/* Colors definition */
#define BOLD_RED              "\x1b[1;31m"
#define BOLD_GREEN            "\x1b[1;32m"
#define BOLD_YELLOW           "\x1b[1;33m"
#define RESET                 "\x1b[0m"

/* Formatted messages definition */
#define BASEFILE              (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define MSG_VERBOSE(str)      message('v', str, BASEFILE, __LINE__)
#define MSG_WARNING(str)      message('w', str, BASEFILE, __LINE__)
#define MSG_ERROR(str)        message('e', str, BASEFILE, __LINE__)
#define MSG_ERROR_ERRNO(str)  message('n', str, BASEFILE, __LINE__)
#define _(str)                gettext(str)

/* Options definition */
#define OUT_GTK               (1 << 0)
#define OUT_NCURSES           (1 << 1)
#define OUT_DUMP              (1 << 2)
#define OUT_DMIDECODE         (1 << 3)

/* Arrays definition */
#define NAME                  0
#define VALUE                 1
#define MAXSTR                60       /* Max string */
#define CACHEFIELDS           3        /* Nb of fields by cache frame */
#define RAMFIELDS             2        /* Nb of fields by bank */
#define GPUFIELDS             3        /* Nb of fields by GPU frame */

/* Linux-specific paths definition */
#define SYS_DMI               "/sys/devices/virtual/dmi/id"
#define SYS_CPU               "/sys/devices/system/cpu/cpu"
#define SYS_DRM               "/sys/class/drm/card"


enum EnTabNumber
{
	NO_CPU, NO_CACHES, NO_MOTHERBOARD, NO_MEMORY, NO_SYSTEM, NO_GRAPHICS, NO_ABOUT
};

enum EnObjects
{
	TABCPU, TABCACHES, TABMOTHERBOARD, TABMEMORY, TABSYSTEM, TABGRAPHICS, TABABOUT,
	FRAMPROCESSOR, FRAMCLOCKS, FRAMCACHE,
	FRAML1CACHE, FRAML2CACHE, FRAML3CACHE,
	FRAMMOTHERBOARD, FRAMBIOS, FRAMCHIPSET,
	FRAMBANKS,
	FRAMOPERATINGSYSTEM, FRAMMEMORY,
	FRAMGPU1, FRAMGPU2, FRAMGPU3, FRAMGPU4,
	FRAMABOUT, FRAMLICENSE,	LABVERSION, LABDESCRIPTION, LABAUTHOR, LABCOPYRIGHT, LABLICENSE,
	LASTOBJ
};

enum EnTabCpu
{
	VENDOR, CODENAME, PACKAGE, TECHNOLOGY, VOLTAGE, SPECIFICATION, FAMILY, EXTFAMILY, MODEL, EXTMODEL, TEMPERATURE, STEPPING, INSTRUCTIONS,
	CORESPEED, MULTIPLIER, BUSSPEED, USAGE,
	LEVEL1D, LEVEL1I, LEVEL2, LEVEL3,
	SOCKETS, CORES, THREADS,
	LASTCPU
};

enum EnTabCaches
{
	L1SIZE, L1DESCRIPTOR, L1SPEED,
	L2SIZE, L2DESCRIPTOR, L2SPEED,
	L3SIZE, L3DESCRIPTOR, L3SPEED,
	LASTCACHES
};

enum EnTabMotherboard
{
	MANUFACTURER, MBMODEL, REVISION,
	BRAND, BIOSVERSION, DATE, ROMSIZE,
	CHIPVENDOR, CHIPMODEL,
	LASTMOTHERBOARD
};

enum EnTabMemory
{
	BANK0_0, BANK0_1, BANK1_0, BANK1_1, BANK2_0, BANK2_1, BANK3_0, BANK3_1,
	BANK4_0, BANK4_1, BANK5_0, BANK5_1, BANK6_0, BANK6_1, BANK7_0, BANK7_1,
	LASTMEMORY
};

enum EnTabSystem
{
	KERNEL, DISTRIBUTION, HOSTNAME, UPTIME, COMPILER,
	USED, BUFFERS, CACHED, FREE, SWAP,
	LASTSYSTEM
};

enum EnTabGraphics
{
	GPU1VENDOR, GPU1MODEL, GPU1TEMPERATURE,
	GPU2VENDOR, GPU2MODEL, GPU2TEMPERATURE,
	GPU3VENDOR, GPU3MODEL, GPU3TEMPERATURE,
	GPU4VENDOR, GPU4MODEL, GPU4TEMPERATURE,
	LASTGRAPHICS
};

typedef struct
{
	char *objects[LASTOBJ];
	char *tab_cpu[2][LASTCPU];
	char *tab_caches[2][LASTCACHES];
	char *tab_motherboard[2][LASTMOTHERBOARD];
	char *tab_memory[2][LASTMEMORY];
	char *tab_system[2][LASTSYSTEM];
	char *tab_graphics[2][LASTGRAPHICS];

	int      cpu_freq;
	int8_t   cpu_vendor_id;
	uint8_t  selected_core, cpu_count, gpu_count, dimms_count;
	int32_t  cpu_model, cpu_ext_model, cpu_ext_family;
	uint32_t l1_size, l2_size, l3_size;
	double   bus_freq;
} Labels;

typedef struct
{
	unsigned int output_type;
	unsigned int refr_time;
	bool         verbose;
	bool         color;
} Options;

extern Options *opts;


/***************************** Defined in main.c *****************************/

/* Print a formatted message */
int message(char type, char *msg, char *basefile, int line);

/* The improved asprintf:
 * - allocate an empty string if input string is null
 * - only call asprintf if there is no format in input string
 * - print "valid" args if input string is formatted, or skip them until next arg
     E.g.: iasprintf(&buff, "%i nm", 32) will allocate "32 nm" string
           iasprintf(&buff, "%i nm", 0) will allocate an empty string
	   iasprintf(&buff, "foo %s %s", NULL, "bar") will allocate "foo bar" */
int iasprintf(char **str, const char *fmt, ...);

/* Check if a command exists */
bool command_exists(char *in);

/* Open a file or a pipe and put its content in buffer */
int xopen_to_str(char *file, char **buffer, char type);

/* Free memory after display labels */
void labels_free(Labels *data);


/***************************** External headers *****************************/

/* Fill labels by calling core functions */
int fill_labels(Labels *data);

/* Refresh some labels */
int do_refresh(Labels *data, enum EnTabNumber page);

/* Call Dmidecode through CPU-X but do nothing else */
int run_dmidecode(void);

/* Call bandwidth library */
int bandwidth(Labels *data);

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* Start CPU-X in NCurses mode */
void start_tui_ncurses(Labels *data);


#endif /* _CPUX_H_ */
