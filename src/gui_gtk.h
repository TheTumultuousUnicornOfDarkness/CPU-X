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

typedef struct
{
	/* Common */
	GtkWidget *mainwindow;
	GtkWidget *notebook;
	GtkWidget *labprgver;
	GtkWidget *closebutton;
	GtkWidget *gtktrad[LASTOBJ];

	/* Tab CPU */
	GtkWidget *logocpu;
	GtkWidget *gtktabcpu[2][LASTCPU];

	/* Tab Mainboard */
	GtkWidget *gtktabmb[2][LASTMB];

	/* Tab RAM */
	GtkWidget *gridbanks;
	GtkWidget *gtktabram[2][LASTRAM];

	/* Tab System */
	GtkWidget *barused;
	GtkWidget *barbuff;
	GtkWidget *barcache;
	GtkWidget *barfree;
	GtkWidget *barswap;
	GtkWidget *gtktabsys[2][LASTSYS];

	/* Tab About */
	GtkWidget *logoprg;

} GtkLabels; /* Useful GtkWidgets */

typedef struct
{
	GtkLabels *glab;
	Labels *data;
} GThrd; /* Used to refresh GUI */

/********************************** GUI  **********************************/

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* Refresh non-static values */
gpointer grefresh(GThrd *refr);

/* Set default background color in GUI */
void set_colors(GtkLabels *glab);

/* Set logos (Window, CPU vendor, tab About) */
void set_logos(GtkLabels *glab, Labels *data);

/* Build tab 'CPU' thanks to GtkBuilder */
void get_labels(GtkBuilder *builder, GtkLabels *glab);

/* Set values in labels */
void set_labels(GtkLabels *glab, Labels *data);

/* Set Memory bar in tab System */
void set_membar(GtkLabels *glab, Labels *data);


#endif /* _CPUX_GTK_H_ */
