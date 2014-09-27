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
#define PRGNAME "CPU-X"
#define PRGVER  "1.0.0"
#define EXIT_FNO 2	/* Exit when File Not Open */
#define DATADIR(str) "/usr/local/share/cpu-x/"str /* Data directory | TODO: Find an alternative */

#define S 80		/* Big	  char* */
#define P 5		/* Little char* */
#define Q 3*P		/* Medium char* */


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
	char arch[S];
	char endian[S];
	char mhz[S];
	char mhzmin[S];
	char mhzmax[S];
	char mips[S];
	char virtu[S];
	} Lscpu;	/* Designed for lscpu */

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
	GtkWidget *window;
	GtkWidget *notebook1;
	GtkWidget *okbutton;
	GtkWidget *lprgname;
	GtkWidget *lprgver;
	GtkWidget *proc_logo;
	GtkWidget *proc_vvendor;
	GtkWidget *proc_vname;
	GtkWidget *proc_vpkg;
	GtkWidget *proc_varch;
	GtkWidget *proc_vspec;
	GtkWidget *proc_vfam;
	GtkWidget *proc_vmod;
	GtkWidget *proc_vextfam;
	GtkWidget *proc_vextmod;
	GtkWidget *proc_vstep;
	GtkWidget *proc_vinstr;
	GtkWidget *clock_vcore;
	GtkWidget *clock_vmult;
	GtkWidget *clock_vbus;
	GtkWidget *clock_vmips;
	GtkWidget *cache_vl1d;
	GtkWidget *cache_vl1i;
	GtkWidget *cache_vl2;
	GtkWidget *cache_vl3;
	GtkWidget *cache_vl1dway;
	GtkWidget *cache_vl1iway;
	GtkWidget *cache_vl2way;
	GtkWidget *cache_vl3way;
	GtkWidget *trg_vsoc;
	GtkWidget *trg_vcore;
	GtkWidget *trg_vthrd;
	GtkWidget *mb_vmanu;
	GtkWidget *mb_vmodel;
	GtkWidget *mb_vrev;
	GtkWidget *bios_vbrand;
	GtkWidget *bios_vversion;
	GtkWidget *bios_vdate;
	GtkWidget *bios_vroms;
	GThread *threfresh;
	} Gwid;		/* Useful GtkWidgets */


/********************************** Core **********************************/

/* Run extern command and stock the output */
int cmd_char(char *command, char *regex, char data[S]);

/* Use 'libcpuid' to build 'data' */
int libcpuid(Libcpuid *data);

/* Use extern command 'lscpu' to build 'info' */
int ext_lscpu(Lscpu *info);

/* Use 'libdmi' to build 'extrainfo' (replace ext_dmidecode) */
int libdmidecode(Dmi *data);

/* Get CPU frequencies (current - min - max) */
int cpufreq(char *curfreq, char *multmin, char *multmax);

/* Read value "bobomips" from file /proc/cpuinfo */
void bogomips(char *c);

/* If 'dmidecode' can be called, return CPU multipliers (actual, min and max) */
void mult(char *busfreq, char *cpufreq, char *multmin, char *multmax, char multsynt[15]);

/* Print some instruction sets */ 
void instructions(Lscpu *info, Libcpuid *data, char instr[S]);


/********************************** GUI  **********************************/

/* Refresh non-static values */
gpointer refresh(Gwid *cpu);

/* Infinite loop which auto-refresh GUI */
gpointer boucle(Gwid *cpu);

/* Set empty labels */
void empty_labels(Libcpuid *data, Lscpu *info, Dmi *extrainfo);

/* White was too simple... */
void set_colors(Gwid *cpu);

/* Show vendor logo (Intel/AMD) */
void set_vendorlogo(Gwid *cpu, Libcpuid *data);

/* Build tab 'CPU' thanks to GtkBuilder */
void build_tab_cpu(GtkBuilder *builder, Gwid *cpu);

/* Set values in labels */
void set_labels(Gwid *cpu, Libcpuid *data, Lscpu *info, Dmi *extrainfo);

#endif
