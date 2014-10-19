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
* cpux_gtk.h
*/

#ifndef _CPUX_GTK_H_
#define _CPUX_GTK_H_


#include <gtk/gtk.h>
#include <glib.h>


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
	

/********************************** GUI  **********************************/

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Libcpuid *data, Dmi *extrainfo);

/* Refresh non-static values */
gpointer grefresh(Gwid *cpu);

/* White was too simple... */
void set_colors(Gwid *cpu);

/* Show vendor logo (Intel/AMD) */
void set_vendorlogo(Gwid *cpu, Libcpuid *data);

/* Build tab 'CPU' thanks to GtkBuilder */
void build_tab_cpu(GtkBuilder *builder, Gwid *cpu);

/* Set values in labels */
void set_labels(Gwid *cpu, Libcpuid *data, Dmi *extrainfo);


#endif /* _CPUX_GTK_H_ */
