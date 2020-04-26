/****************************************************************************
*    Copyright © 2014-2019 Xorg
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

typedef struct
{
	/* Common */
	GtkWidget *mainwindow;
	GtkWidget *settingsbutton;
	GtkWidget *notebook;
	GtkWidget *footer;
	GtkWidget *labprgver;
	GtkWidget *daemonbutton;
	GtkWidget *gtktrad[LASTOBJ];

	/* Settings */
	GtkWidget *settingswindow;
	GtkWidget *validatebutton;
	GtkWidget *cancelbutton;
	GtkWidget *refreshtime;
	GtkWidget *theme;
	GtkWidget *defaulttab;
	GtkWidget *defaultcore;
	GtkWidget *defaultcachetest;
	GtkWidget *startdaemon;

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
	GtkWidget *scrollbanks;
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

static enum {
	AUTO,
	LIGHT,
	DARK
} theme = AUTO;

typedef struct
{
	GtkLabels *glab;
	Labels *data;
} GThrd; /* Used to refresh GUI */

/********************************** GUI  **********************************/

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data);

/* In portable version, inform when a new version is available and ask for update */
//static void new_version_window(GtkWidget *mainwindow);

/* Refresh dynamic values */
static gboolean grefresh(GThrd *refr);

/* Create new timeout when old one is destroyed */
static void modify_refresh_time(gpointer data);

/* Show settings window */
static void open_settings_window(GtkWidget *button, GtkLabels *glab);

/* Hide settings window and revert changes */
static void close_settings_window(GtkWidget *button, GtkLabels *glab);

/* Hide settings window and apply changes */
static void save_settings(GtkWidget *button, GtkLabels *glab);

/* Start daemon and reload CPU-X */
static void reload_with_daemon(GtkWidget *button, Labels *data);

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
static gchar *get_id(const gchar *objectstr, gchar *type);

/* Search file location in standard paths */
static gchar *data_path(const gchar *file);

/* Retrieve widgets from GtkBuilder */
static void get_widgets(GtkBuilder *builder, GtkLabels *glab);

/* Set custom GTK theme */
static void set_colors(GtkLabels *glab);

/* Set CPU vendor logo and program logo */
static void set_logos(GtkLabels *glab, Labels *data);

/* Filling all labels */
static void set_labels(GtkLabels *glab, Labels *data);

/* Call defined functions on signals */
static void set_signals(GtkLabels *glab, Labels *data, GThrd *refr);

/* Draw bars in Memory tab */
void fill_frame(GtkWidget *widget, cairo_t *cr, GThrd *refr);

#endif /* _GUI_GTK_H_ */
