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
* FILE core.h
*/

#ifndef _CPUX_H_
#define _CPUX_H_

#include <stdint.h>
#include <stdbool.h>
#define HAVE_STDINT_H	/* Skip conflicts with <libcpuid/libcpuid_types.h> */

#define BOLD_RED	"\x1b[1;31m"
#define BOLD_GREEN	"\x1b[1;32m"
#define BOLD_YELLOW	"\x1b[1;33m"
#define RESET		"\x1b[0m"

#define BASEFILE		(strrchr(__FILE__, '/') ? \
					strrchr(__FILE__, '/') + 1 : __FILE__) /* Don't show full path of file */
#define MSG_VERBOSE(str)	message('v', str, BASEFILE, __LINE__)
#define MSG_WARNING(str)	message('w', str, BASEFILE, __LINE__)
#define MSG_ERROR(str)		message('e', str, BASEFILE, __LINE__)
#define MSG_ERROR_ERRNO(str)	message('n', str, BASEFILE, __LINE__)
#define _(str) gettext(str)

#define OUT_GTK			(1 << 0)
#define OUT_NCURSES		(1 << 1)
#define OUT_DUMP		(1 << 2)
#define OUT_DMIDECODE		(1 << 3)

#define PRGNAME "CPU-X"
#define PRGAUTH "X0rg"
#define PRGCPYR "Copyright © 2014-2015 Xorg"
#define EXIT_FNO 2	/* Exit when File Not Open */
#define SYS_DMI "/sys/devices/virtual/dmi/id"
#define SYS_CPU "/sys/devices/system/cpu/cpu"
#define SYS_DRM "/sys/class/drm/card"
#define NAME	0
#define VALUE	1
#define MAXSTR	60	/* Max string */
#define S 10		/* Little string */
#define CACHEFIELDS 3	/* Nb of fields by cache frame */
#define RAMFIELDS 2	/* Nb of fields by bank */
#define GPUFIELDS 3	/* Nb of fields by GPU frame */


enum EnTabNumber
{
	NB_TAB_CPU, NB_TAB_CACHE, NB_TAB_MB, NB_TAB_RAM, NB_TAB_SYS, NB_TAB_GPU, NB_TAB_ABOUT
};

enum EnObjects
{
	TABCPU, TABCACHE, TABMB, TABRAM, TABSYS, TABGPU, TABABOUT,
	FRAMPROCESSOR, FRAMCLOCKS, FRAMCACHE,
	FRAMCACHEL1, FRAMCACHEL2, FRAMCACHEL3,
	FRAMMOBO, FRAMBIOS, FRAMCHIP,
	FRAMBANKS,
	FRAMOS, FRAMMEMORY,
	FRAMGPU1, FRAMGPU2, FRAMGPU3, FRAMGPU4,
	FRAMABOUT, FRAMLICENSE,	LABVERSION, LABDESCRIPTION, LABAUTHOR, LABCOPYRIGHT, LABLICENSE,
	LASTOBJ
};

enum EnTabCpu
{
	VENDOR, CODENAME, PACKAGE, TECHNOLOGY, VOLTAGE, SPECIFICATION, FAMILY, EXTFAMILY, MODEL, EXTMODEL, TEMPERATURE, STEPPING, INSTRUCTIONS,
	CORESPEED, MULTIPLIER, BUSSPEED, BOGOMIPS,
	LEVEL1D, LEVEL1I, LEVEL2, LEVEL3,
	SOCKETS, CORES, THREADS,
	LASTCPU
};

enum EnTabCache
{
	L1SIZE, L1DESCR, L1SPEED,
	L2SIZE, L2DESCR, L2SPEED,
	L3SIZE, L3DESCR, L3SPEED,
	LASTCACHE
};

enum EnTabMainboard
{
	MANUFACTURER, MBMODEL, REVISION,
	BRAND, BIOSVER, DATE, ROMSIZE,
	CHIPVENDOR, CHIPNAME,
	LASTMB
};

enum EnTabRam
{
	BANK0_0, BANK0_1, BANK1_0, BANK1_1, BANK2_0, BANK2_1, BANK3_0, BANK3_1,
	BANK4_0, BANK4_1, BANK5_0, BANK5_1, BANK6_0, BANK6_1, BANK7_0, BANK7_1,
	LASTRAM
};

enum EnTabSystem
{
	KERNEL, DISTRIBUTION, HOSTNAME, UPTIME, COMPILER,
	USED, BUFFERS, CACHED, FREE, SWAP,
	LASTSYS
};

enum EnTabGraphics
{
	GPUVENDOR1, GPUNAME1, GPUTEMP1,
	GPUVENDOR2, GPUNAME2, GPUTEMP2,
	GPUVENDOR3, GPUNAME3, GPUTEMP3,
	GPUVENDOR4, GPUNAME4, GPUTEMP4,
	LASTGPU
};

typedef struct
{
	char *objects[LASTOBJ];
	char *tabcpu[2][LASTCPU];
	char *tabcache[2][LASTCACHE];
	char *tabmb[2][LASTMB];
	char *tabram[2][LASTRAM];
	char *tabsys[2][LASTSYS];
	char *tabgpu[2][LASTGPU];

	int cpu_freq;
	int8_t cpu_vendor_id;
	uint8_t selected_core, gpu_count, dimms_count;
	int32_t cpu_model, cpu_ext_model, cpu_ext_family;
	uint32_t l1_size, l2_size, l3_size;
	double bus_freq;
} Labels;

typedef struct
{
	unsigned int output_type;
	unsigned int refr_time;
	bool verbose;
} Options;

extern Options *opts;


/***************************** Defined in main.c *****************************/

/* Enable internationalization support */
int set_locales(void);

/* Extract locales in /tmp/.cpu-x */
int extract_locales(void);

/* Apply new portable version if available */
int update_prg(char *executable, Options *opts);

/* Get options */
void menu(int argc, char *argv[]);

/* Print a formatted message */
int message(char type, char *msg, char *basefile, int line);

/* Duplicate a not null string */
char *strdupnullok(const char *s);

/* The improved asprintf, which allocate a empty string if string is null */
int iasprintf(char **str, const char *fmt, ...);

/* Check if a command exists */
bool command_exists(char *in);

/* Open a file or a pipe and put its content in buffer */
int xopen_to_str(char *file, char **buffer, char type);

/* Set labels name */
void labels_setname(Labels *data);

/* Replace null pointers by character '\0' */
void labels_delnull(Labels *data);

/* Free memory after display labels */
void labels_free(Labels *data);

/* Dump all data in stdout */
void dump_data(Labels *data);

/* Check if running version is latest */
char *check_lastver(void);


/***************************** External headers *****************************/

/* Fill labels by calling core functions */
int fill_labels(Labels *data);

/* Call dmidecode library */
int libdmi(char c);

/* Call bandwidth library */
int bandwidth(Labels *data);

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* Start CPU-X in NCurses mode */
void start_tui_ncurses(Labels *data);


#endif /* _CPUX_H_ */
