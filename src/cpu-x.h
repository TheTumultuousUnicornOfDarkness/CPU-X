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
#define MSGERR(msg, args...) fprintf(stderr, "%s:%s:%i: " msg "\n", PRGNAME, BASEFILE, __LINE__, ##args)
#define _(str) gettext(str)

#define PRGNAME "CPU-X"
#define PRGVER  "1.2.2"
#define EXIT_FNO 2	/* Exit when File Not Open */
#define SYS_DMI "/sys/devices/virtual/dmi/id"
#define NAME	0
#define VALUE	1
#define MAXSTR	60	/* Max string */
#define S 10		/* Little string */

extern int refreshtime;


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

enum EnTabSystem
{
	KERNEL, DISTRIBUTION, HOSTNAME, UPTIME, COMPILER,
	USED, BUFFERS, CACHED, FREE, SWAP,
	LASTSYS
};

typedef struct
{
	char tabcpu[2][LASTCPU][MAXSTR];
	char tabmb[2][LASTMB][MAXSTR];
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

/* Dump all datas in stdout */
void dump_data(Labels *data);

/* Get path for data files */
char *get_path(char *file);


#endif /* _CPUX_H_ */
