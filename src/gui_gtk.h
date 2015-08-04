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
* gui_gtk.h
*/

#ifndef _GUI_GTK_H_
#define _GUI_GTK_H_


#include <gtk/gtk.h>
#include <cairo.h>
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

	/* Tab Graphics */
	GtkWidget *gridcards;
	GtkWidget *gtktabgpu[2][LASTGPU];

	/* Tab About */
	GtkWidget *logoprg;
	GtkWidget *butcol;

} GtkLabels; /* Useful GtkWidgets */

typedef struct
{
	GtkLabels *glab;
	Labels *data;
} GThrd; /* Used to refresh GUI */


/********************************** GUI  **********************************/

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* Show a warning if launched as regulat user */
void warning_window(GtkWidget *mainwindow);

/* Refresh non-static values */
gboolean grefresh(GThrd *refr);

/* Set default background color in GUI */
void set_colors(GtkLabels *glab);

/* Change UI color by using GtkColorButton */
void change_color(GtkWidget *button, GtkLabels *glab);

/* Set logos (Window, CPU vendor, tab About) */
void set_logos(GtkLabels *glab, Labels *data);

/* Build tab 'CPU' thanks to GtkBuilder */
void get_labels(GtkBuilder *builder, GtkLabels *glab);

/* Set values in labels */
void set_labels(GtkLabels *glab, Labels *data);

/* Set Memory bar in tab System */
void fill_frame(GtkWidget *widget, cairo_t *cr, Labels *data, int n);
void setbar_used(GtkWidget *widget, cairo_t *cr, Labels *data);
void setbar_buff(GtkWidget *widget, cairo_t *cr, Labels *data);
void setbar_cache(GtkWidget *widget, cairo_t *cr, Labels *data);
void setbar_free(GtkWidget *widget, cairo_t *cr, Labels *data);
void setbar_swap(GtkWidget *widget, cairo_t *cr, Labels *data);

/* Get path for data files thanks to GLib */
char *data_path(char *file);


#endif /* _GUI_GTK_H_ */
