/****************************************************************************
*    Copyright © 2014 Xorg
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
* cpu-x.h
*/

#ifndef _CPUX_H_
#define _CPUX_H_


#define HAVE_STDINT_H	/* Skip conflicts with <libcpuid/libcpuid_types.h> */
#define BASEFILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) /* Don't show full path of file */
#define MSGVERB(msg) if(verbose == 1 || verbose == 3) printf("\033[1;32m%s\033[0m\n", msg)
#define MSGERR(msg) fprintf(stderr, "\033[1;32m%s:%s:%i: %s\033[0m\n", PRGNAME, BASEFILE, __LINE__, _(msg))
#define MSGROOT _("\n\t\t\tWARNING:\n%s requires root privileges to run correctly.\n")
#define _(str) gettext(str)

#define PRGNAME "CPU-X"
#define PRGVER  "2.0.2"
#define PRGAUTH "X0rg"
#define PRGCPYR "Copyright © 2014 Xorg"
#define EXIT_FNO 2	/* Exit when File Not Open */
#define SYS_DMI "/sys/devices/virtual/dmi/id"
#define NAME	0
#define VALUE	1
#define MAXSTR	60	/* Max string */
#define S 10		/* Little string */

#ifdef GTK
# define HAS_GTK 1
#else
# define HAS_GTK 0
#endif

#ifdef NCURSES
# define HAS_NCURSES 1
#else
# define HAS_NCURSES 0
#endif

#ifdef LIBCPUID
# define HAS_LIBCPUID 1
#else
# define HAS_LIBCPUID 0
#endif

#ifdef LIBDMI
# define HAS_LIBDMI 1
#else
# define HAS_LIBDMI 0
#endif

#ifdef LIBPROCPS
# define HAS_LIBPROCPS 1
#else
# define HAS_LIBPROCPS 0
#endif

extern int refreshtime;
extern int verbose;


enum EnTabNumber
{
	NB_TAB_CPU, NB_TAB_MB, NB_TAB_RAM, NB_TAB_SYS, NB_TAB_ABOUT
};

enum EnObjects
{
	TABCPU, TABMB, TABRAM, TABSYS, TABABOUT,
	FRAMPROCESSOR, FRAMCLOCKS, FRAMCACHE, FRAMMOBO, FRAMBIOS, FRAMBANKS, FRAMOS, FRAMMEMORY, FRAMABOUT, FRAMLICENSE,
	LABVERSION, LABDESCRIPTION, LABAUTHOR, LABCOPYRIGHT, LABLICENSE,
	LASTOBJ
};

enum EnTabCpu
{
	VENDOR, CODENAME, PACKAGE, ARCHITECTURE, SPECIFICATION, FAMILY, EXTFAMILY, MODEL, EXTMODEL, STEPPING, INSTRUCTIONS,
	CORESPEED, MULTIPLIER, BUSSPEED, BOGOMIPS,
	LEVEL1D, LEVEL1I, LEVEL2, LEVEL3,
	SOCKETS, CORES, THREADS,
	LASTCPU
};

enum EnTabMainboard
{
	MANUFACTURER, MBMODEL, REVISION,
	BRAND, VERSION, DATE, ROMSIZE,
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

typedef struct
{
	char objects[LASTOBJ][MAXSTR + 40];
	char tabcpu[2][LASTCPU][MAXSTR];
	char tabmb[2][LASTMB][MAXSTR];
	char tabram[2][LASTRAM][MAXSTR];
	char tabsys[2][LASTSYS][MAXSTR];
} Labels;


/********************************** Core **********************************/

/* Get options */
char menu(int argc, char *argv[]);

/* Set empty labels */
void labels_setempty(Labels *data);

/* Set labels name */
void labels_setname(Labels *data);

/* Elements provided by libcpuid library */
int libcpuid(Labels *data);

/* Elements provided by libdmi library (need root privileges) */
int libdmidecode(Labels *data);

/* Alternative for libdmidecode (Linux only) */
int libdmi_fallback(Labels *data);

/* Pretty label CPU Vendor */
void cpuvendor(char *vendor);

/* Remove unwanted spaces in value Specification */
void clean_specification(char *spec);

/* Get CPU frequencies (current - min - max) */
void cpufreq(char *busfreq, char *clock, char *mults);

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char *c);

/* If 'dmidecode' can be called, return CPU multipliers (actual, min and max) */
void mult(char *busfreq, char *cpufreq, char *multmin, char *multmax, char multsynt[15]);

/* Print some instruction sets */
void instructions(char arch[MAXSTR], char instr[MAXSTR]);

/* Get system informations */
void tabsystem(Labels *data);

/* Find the number of existing banks */
int last_bank(Labels *data);

/* Dump all data in stdout */
void dump_data(Labels *data);

/* Get path for data files */
char *get_path(char *file);


#endif /* _CPUX_H_ */
