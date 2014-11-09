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
	GtkWidget *labprgver;
	GtkWidget *closebutton;

	/* Tab CPU */
	GtkWidget *logocpu;
	GtkWidget *gtktabcpu[2][LASTCPU];

	/* Tab Mainboard */
	GtkWidget *gtktabmb[2][LASTMB];

	/* Tab System */
	GtkWidget *gtktabsys[2][LASTMB];

	/* Tab About */
	GtkWidget *logoprg;
	GtkWidget *aboutprgver;

} GtkLabels; /* Useful GtkWidgets */

typedef struct
{
	GThread *thrdrefr;
	GtkLabels *glab;
	Labels *data;
} GThrd; /* Used to refresh GUI */

/********************************** GUI  **********************************/

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* Refresh non-static values */
gpointer grefresh(GThrd *refr);

/* Show vendor logo (Intel/AMD) */
void set_vendorlogo(GtkLabels *glab, Labels *data);

/* Build tab 'CPU' thanks to GtkBuilder */
void get_labels(GtkBuilder *builder, GtkLabels *glab);

/* Set values in labels */
void set_labels(GtkLabels *glab, Labels *data);


#endif /* _CPUX_GTK_H_ */
