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
#define PRGVER  "1.2.1"
#define EXIT_FNO 2	/* Exit when File Not Open */

#define S 80		/* Big	  char* */
#define P 5		/* Little char* */
#define Q 3*P		/* Medium char* */

extern int refreshtime;


typedef struct {
	char vendor[S];
	char name[S];
	char arch[S];
	char spec[S];
	char fam[S];
	char mod[S];
	char step[S];
	char extfam[S];
	char extmod[S];
	char instr[S];
	char l1d[S];
	char l1i[S];
	char l2[S];
	char l3[S];
	char l1dw[S];
	char l1iw[S];
	char l2w[S];
	char l3w[S];
	char soc[S];
	char core[S];
	char thrd[S];
	} Libcpuid;	/* Designed for libcpuid */

typedef struct {
	char vendor[S];
	char socket[S];
	char bus[S];
	char manu[S];
	char model[S];
	char rev[S];
	char brand[S];
	char version[S];
	char date[S];
	char rom[S];
	} Dmi;		/* Designed for dmidecode */

typedef struct {
	char clock[Q];
	char mults[Q];
	char mips[Q];
	char instr[S];
	} Internal;	/* Used to call core functions */


/********************************** Core **********************************/

/* Get options */
char menu(int argc, char *argv[]);

/* Set empty labels */
void empty_labels(Libcpuid *data, Dmi *extrainfo, Internal *global);

/* Use 'libcpuid' to build 'data' */
int libcpuid(Libcpuid *data);

/* Use 'libdmi' to build 'extrainfo' (replace ext_dmidecode) */
int libdmidecode(Dmi *data);

/* Get CPU frequencies (current - min - max) */
void cpufreq(Internal *global, char *busfreq);

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char *c);

/* If 'dmidecode' can be called, return CPU multipliers (actual, min and max) */
void mult(char *busfreq, char *cpufreq, char *multmin, char *multmax, char multsynt[15]);

/* Print some instruction sets */ 
void instructions(Libcpuid *data, char instr[S]);

/* Get path for data files */
size_t get_path(char* buffer, char *file);


#endif /* _CPUX_H_ */
