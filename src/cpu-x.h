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
#define PRGNAME "CPU-X"
#define PRGVER  "1.2.2"
#define EXIT_FNO 2	/* Exit when File Not Open */

#define S 80		/* Big	  char* */
#define P 5		/* Little char* */
#define Q 3*P		/* Medium char* */

extern int refreshtime;

#define _(str) gettext(str)
#define MAXSTR	60
#define NAME	0
#define VALUE	1

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

typedef struct
{
	char tabcpu[2][LASTCPU][MAXSTR];
	char tabmb[2][LASTMB][MAXSTR];
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

/* Get CPU frequencies (current - min - max) */
void cpufreq(char *busfreq, char *clock, char *mults);

/* Pretty label CPU Vendor */
void cpuvendor(char *vendor);

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char *c);

/* If 'dmidecode' can be called, return CPU multipliers (actual, min and max) */
void mult(char *busfreq, char *cpufreq, char *multmin, char *multmax, char multsynt[15]);

/* Print some instruction sets */ 
void instructions(char arch[MAXSTR], char instr[MAXSTR]);

/* Dump all datas in stdout */
void dump_data(Labels *data);

/* Get path for data files */
char *get_path(char *file);


#endif /* _CPUX_H_ */
