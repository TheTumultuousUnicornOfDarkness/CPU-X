/****************************************************************************
*    Copyright © 2014-2016 Xorg
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
* FILE gui_gtk.h
*/

#ifndef _GUI_GTK_H_
#define _GUI_GTK_H_


#include <gtk/gtk.h>
#include <cairo.h>
#include <glib.h>

#define GRESOURCE_UI(file)    g_strconcat("/cpu-x/ui/",    file, NULL)
#define GRESOURCE_CSS(file)   g_strconcat("/cpu-x/css/",   file, NULL)
#define GRESOURCE_LOGOS(file) g_strconcat("/cpu-x/logos/", file, NULL)

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
	GtkWidget *gtktab_cpu[2][LASTCPU];
	GtkWidget *activecore;

	/* Tab Caches */
	GtkWidget *gridcaches;
	GtkWidget *gtktab_caches[2][LASTCACHES];
	GtkWidget *activetest;

	/* Tab Motherboard */
	GtkWidget *gtktab_motherboard[2][LASTMOTHERBOARD];

	/* Tab RAM */
	GtkWidget *gridbanks;
	GtkWidget *gtktab_memory[2][LASTMEMORY];

	/* Tab System */
	GtkWidget *bar[LASTBAR];
	GtkWidget *gtktab_system[2][LASTSYSTEM];

	/* Tab Graphics */
	GtkWidget *gridcards;
	GtkWidget *gtktab_graphics[2][LASTGRAPHICS];

	/* Tab Bench */
	GtkWidget *gtktab_bench[2][LASTBENCH];

	/* Tab About */
	GtkWidget *logoprg;
	GtkWidget *butcol;
	GtkWidget *gtktab_about[LASTABOUT];

} GtkLabels; /* Useful GtkWidgets */

typedef struct
{
	GtkLabels *glab;
	Labels *data;
} GThrd; /* Used to refresh GUI */


/********************************** GUI  **********************************/

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* Print a window which allows to restart CPU-X as root */
static void warning_window(GtkWidget *mainwindow);

/* In portable version, inform when a new version is available and ask for update */
static void new_version_window(GtkWidget *mainwindow);

/* Refresh dynamic values */
static gboolean grefresh(GThrd *refr);

/* Event in CPU tab when Core number is changed */
static void change_activecore(GtkComboBox *box, Labels *data);

/* Event in Caches tab when Test number is changed */
static void change_activetest(GtkComboBox *box, Labels *data);

/* Events in Bench tab when a benchmark start/stop */
static void start_benchmark_bg(GtkSwitch *gswitch, GdkEvent *event, GThrd *refr);

/* Events in Bench tab when Duration/Threads SpinButtons are changed */
static void change_benchparam(GtkSpinButton *spinbutton, Labels *data);

/* Set/Unset widgets sensitive when a benchmark start/stop */
static void change_benchsensitive(GtkLabels *glab, Labels *data);

/* Get label ID ('type' must be "lab" or "val") */
static char *get_id(const char *objectstr, char *type);

/* Search file location in standard paths */
static char *data_path(const char *file);

/* Retrieve widgets from GtkBuilder */
static void get_widgets(GtkBuilder *builder, GtkLabels *glab);

/* Set custom GTK theme */
static void set_colors(GtkLabels *glab);

/* Allow user to choose a new color theme (until GTK 3.14) */
static void change_color(GtkWidget *button, GtkLabels *glab);

/* Set CPU vendor logo and program logo */
static void set_logos(GtkLabels *glab, Labels *data);

/* Filling all labels */
static void set_labels(GtkLabels *glab, Labels *data);

/* Call defined functions on signals */
static void set_signals(GtkLabels *glab, Labels *data, GThrd *refr);

/* Draw bars in Memory tab */
void fill_frame(GtkWidget *widget, cairo_t *cr, GThrd *refr);


#endif /* _GUI_GTK_H_ */
