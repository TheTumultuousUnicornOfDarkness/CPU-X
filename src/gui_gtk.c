/****************************************************************************
*    Copyright © 2014-2021 Xorg
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
* FILE gui_gtk.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libintl.h>
#include <sys/wait.h>
#include "cpu-x.h"
#include "ipc.h"
#include "gui_gtk.h"
#include "gui_gtk_id.h"


/************************* Public function *************************/

/* Load and apply settings from GSettings */
void load_settings()
{
	settings            = g_settings_new(PRGNAME_LOW);
	theme               = g_settings_get_enum(settings, "gui-theme");
	opts->refr_time     = g_settings_get_uint(settings, "refresh-time");
	opts->selected_page = g_settings_get_enum(settings, "default-tab");
	opts->selected_core = g_settings_get_uint(settings, "default-cpu-core");
	opts->bw_test       = g_settings_get_uint(settings, "default-cache-test");
	opts->selected_gpu  = g_settings_get_uint(settings, "default-active-card");
	opts->cpuid_decimal = g_settings_get_boolean(settings, "print-cpuid-decimal");
	opts->with_daemon   = g_settings_get_boolean(settings, "always-start-daemon");
	g_settings_delay(settings);
}

/* Start CPU-X in GTK mode */
void start_gui_gtk(int *argc, char **argv[], Labels *data)
{
	int i;
	GtkLabels  glab;
	GThrd      refr     = { &glab, data };
	GtkBuilder *builder = gtk_builder_new();
	const gchar *ui_files[] = { "cpu-x-gtk-3.12.ui", NULL };

	MSG_VERBOSE("%s", _("Starting GTK GUI…"));
	gtk_init(argc, argv);
	g_set_prgname(PRGNAME_LOW);

	/* Build UI from Glade file */
	for(i = 0; (ui_files[i] != NULL) && (!gtk_builder_add_from_file(builder, data_path(ui_files[i]), NULL)); i++);
	if(ui_files[i] == NULL)
	{
		MSG_ERROR("%s", _("failed to import UI in GtkBuilder"));
		exit(EXIT_FAILURE);
	}

	/* Set widgets */
	get_widgets(builder, &glab);
	g_object_unref(G_OBJECT(builder));
	set_colors (&glab);
	set_logos  (&glab, data);
	set_labels (&glab, data);
	set_signals(&glab, data, &refr);
	labels_free(data);

	/* Bind settings to get_widgets */
	g_settings_bind(settings, "refresh-time",        glab.refreshtime,      "value",     G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "gui-theme",           glab.theme,            "active-id", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "default-tab",         glab.defaulttab,       "active-id", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "default-cpu-core",    glab.defaultcore,      "active",    G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "default-cache-test",  glab.defaultcachetest, "active",    G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "default-active-card", glab.defaultcard,      "active",    G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "print-cpuid-decimal", glab.cpuiddecimal,     "active",    G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "always-start-daemon", glab.startdaemon,      "active",    G_SETTINGS_BIND_DEFAULT);

	g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, opts->refr_time, (gpointer)grefresh, &refr, modify_refresh_time);
	gtk_main();

	if(data->reload)
		execvp((*argv)[0], *argv);
}


/************************* Private functions *************************/

/* Refresh dynamic values */
static gboolean grefresh(GThrd *refr)
{
	int i;
	gboolean ret = true;
	Labels    *(data) = refr->data;
	GtkLabels *(glab) = refr->glab;
	uint16_t new_refr_time = g_settings_get_uint(settings, "refresh-time");
	static uint16_t old_refr_time = 0;

	opts->selected_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(glab->notebook));
	do_refresh(data);

	switch(opts->selected_page)
	{
		case NO_CPU:
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][VOLTAGE]),      data->tab_cpu[VALUE][VOLTAGE]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][TEMPERATURE]),  data->tab_cpu[VALUE][TEMPERATURE]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][MULTIPLIER]),   data->tab_cpu[VALUE][MULTIPLIER]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][CORESPEED]),    data->tab_cpu[VALUE][CORESPEED]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][USAGE]),        data->tab_cpu[VALUE][USAGE]);
			break;
		case NO_CACHES:
			for(i = L1SPEED; i < data->cache_count * CACHEFIELDS; i += CACHEFIELDS)
				gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[VALUE][i]), data->tab_caches[VALUE][i]);
			break;
		case NO_SYSTEM:
			gtk_label_set_text(GTK_LABEL(glab->gtktab_system[VALUE][UPTIME]), data->tab_system[VALUE][UPTIME]);
			for(i = USED; i < LASTSYSTEM; i++)
				gtk_label_set_text(GTK_LABEL(glab->gtktab_system[VALUE][i]), data->tab_system[VALUE][i]);
			break;
		case NO_GRAPHICS:
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1PCIE]),        data->tab_graphics[VALUE][GPU1PCIE        + opts->selected_gpu * GPUFIELDS]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1TEMPERATURE]), data->tab_graphics[VALUE][GPU1TEMPERATURE + opts->selected_gpu * GPUFIELDS]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1USAGE]),       data->tab_graphics[VALUE][GPU1USAGE       + opts->selected_gpu * GPUFIELDS]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1VOLTAGE]),     data->tab_graphics[VALUE][GPU1VOLTAGE     + opts->selected_gpu * GPUFIELDS]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1POWERAVG]),    data->tab_graphics[VALUE][GPU1POWERAVG    + opts->selected_gpu * GPUFIELDS]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1CORECLOCK]),   data->tab_graphics[VALUE][GPU1CORECLOCK   + opts->selected_gpu * GPUFIELDS]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1MEMCLOCK]),    data->tab_graphics[VALUE][GPU1MEMCLOCK    + opts->selected_gpu * GPUFIELDS]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1MEMUSED]),     data->tab_graphics[VALUE][GPU1MEMUSED     + opts->selected_gpu * GPUFIELDS]);
			break;
		case NO_BENCH:
			for(i = PRIMESLOWSCORE; i <= PRIMEFASTSCORE; i += BENCHFIELDS)
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR(glab->gtktab_bench[VALUE][i]), data->tab_bench[VALUE][i]);
			change_benchsensitive(glab, data);
			break;
		default:
			break;
	}

	/* Destroy current timeout when refresh time is updated
	Note: modify_refresh_time() will create a new timeout */
	if(old_refr_time == 0)
		old_refr_time = new_refr_time;
	ret = (old_refr_time == new_refr_time);
	old_refr_time = new_refr_time;
	return ret;
}

/* Create new timeout when old one is destroyed */
static void modify_refresh_time(gpointer data)
{
	opts->refr_time = g_settings_get_uint(settings, "refresh-time");
	g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, opts->refr_time, (gpointer)grefresh, data, modify_refresh_time);
}

/* Show settings window */
static void open_settings_window(GtkWidget *__button, GtkLabels *glab)
{
	UNUSED(__button);
	gtk_widget_show(GTK_WIDGET(glab->settingswindow));
}

/* Hide settings window and revert changes */
static gboolean hide_settings_window(GtkWidget *widget, GdkEvent *event, GtkLabels *glab)
{
	UNUSED(event);
	UNUSED(glab);
	g_settings_revert(settings);
	gtk_widget_hide(widget);
	return TRUE;
}

/* Hide settings window and revert changes */
static void close_settings_window(GtkWidget *__button, GtkLabels *glab)
{
	UNUSED(__button);
	g_settings_revert(settings);
	gtk_widget_hide(GTK_WIDGET(glab->settingswindow));
}

/* Hide settings window and apply changes */
static void save_settings(GtkWidget *__button, GtkLabels *glab)
{
	UNUSED(__button);
	theme = g_settings_get_enum(settings, "gui-theme");
	set_colors(glab);
	g_settings_apply(settings);
	gtk_widget_hide(GTK_WIDGET(glab->settingswindow));
}

/* Start daemon and reload CPU-X */
static void reload_with_daemon(GtkWidget *button, GThrd *refr)
{
	Labels    *(data) = refr->data;
	GtkLabels *(glab) = refr->glab;

	gtk_widget_set_sensitive(button, false);
	const char *msg = start_daemon(true);

	if(msg == NULL)
	{
		data->reload = true;
		gtk_main_quit();
	}
	else
	{
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(glab->mainwindow),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_CLOSE,
			"%s", _(msg));
		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_CLOSE)
			gtk_widget_destroy(GTK_WIDGET(dialog));
		gtk_widget_set_sensitive(button, true);
	}
}

/* Event in CPU tab when Core number is changed */
static void change_activecore(GtkComboBox *box, Labels *data)
{
	const gint core = gtk_combo_box_get_active(GTK_COMBO_BOX(box));

	if(0 <= core && core < data->cpu_count)
		opts->selected_core = core;
}

/* Event in Caches tab when Test number is changed */
static void change_activetest(GtkComboBox *box, Labels *data)
{
	const gint test = gtk_combo_box_get_active(GTK_COMBO_BOX(box));

	if(0 <= test && test < data->w_data->test_count)
		opts->bw_test = test;
}

/* Event in Graphics tab when graphic card is changed */
static void change_activecard(GtkComboBox *box, GThrd *refr)
{
	int i, j;
	Labels    *(data) = refr->data;
	GtkLabels *(glab) = refr->glab;
	const gint gpu = gtk_combo_box_get_active(GTK_COMBO_BOX(box));

	if(0 <= gpu && gpu < data->gpu_count)
	{
		opts->selected_gpu = gpu;
		gtk_label_set_text(GTK_LABEL(glab->gtktrad[FRAMGPU1]), data->objects[FRAMGPU1 + opts->selected_gpu]);
		for(i = opts->selected_gpu * GPUFIELDS, j = 0; j < GPUFIELDS; i++, j++)
			gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][j]), data->tab_graphics[VALUE][i]);
	}
}

/* Events in Bench tab when a benchmark start/stop */
static void start_benchmark_bg(GtkSwitch *gswitch, GdkEvent *__event, GThrd *refr)
{
	UNUSED(__event);
	Labels *data = refr->data;

	if(!data->b_data->run)
	{
		change_benchsensitive(refr->glab, data);
		data->b_data->fast_mode = !strcmp(gtk_widget_get_name(GTK_WIDGET(gswitch)), objectbench[PRIMEFASTRUN]);
		start_benchmarks(data);
	}
	else
		data->b_data->run = false;
}

/* Events in Bench tab when Duration/Threads SpinButtons are changed */
static void change_benchparam(GtkSpinButton *spinbutton, Labels *data)
{
	const gint val = gtk_spin_button_get_value_as_int(spinbutton);

	if(!g_strcmp0(gtk_widget_get_name     (GTK_WIDGET(spinbutton)), objectbench[PARAMDURATION]))
		data->b_data->duration = val;
	else if(!g_strcmp0(gtk_widget_get_name(GTK_WIDGET(spinbutton)), objectbench[PARAMTHREADS]))
		data->b_data->threads = val;

	gtk_spin_button_update(spinbutton);
}

/* Set/Unset widgets sensitive when a benchmark start/stop */
static void change_benchsensitive(GtkLabels *glab, Labels *data)
{
	static gboolean skip = false;
	const enum EnTabBench indP = data->b_data->fast_mode ? PRIMEFASTSCORE : PRIMESLOWSCORE;
	const enum EnTabBench indS = data->b_data->fast_mode ? PRIMESLOWRUN   : PRIMEFASTRUN;
	const enum EnTabBench indA = data->b_data->fast_mode ? PRIMEFASTRUN   : PRIMESLOWRUN;

	if(data->b_data->run)
	{
		skip = false;
#if GTK_CHECK_VERSION(3, 15, 0)
		gtk_switch_set_state(GTK_SWITCH(glab->gtktab_bench[VALUE][indA]), true);
#endif /* GTK_CHECK_VERSION(3, 15, 0) */
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(glab->gtktab_bench[VALUE][indP]),
			(double) data->b_data->elapsed / (data->b_data->duration * 60));
		gtk_widget_set_sensitive(glab->gtktab_bench[VALUE][indS],         false);
		gtk_widget_set_sensitive(glab->gtktab_bench[VALUE][PARAMTHREADS], false);
	}
	else if(!data->b_data->run && !skip)
	{
		skip = true;
#if GTK_CHECK_VERSION(3, 15, 0)
		gtk_switch_set_state(GTK_SWITCH(glab->gtktab_bench[VALUE][indA]),  false);
#endif /* GTK_CHECK_VERSION(3, 15, 0) */
		gtk_switch_set_active(GTK_SWITCH(glab->gtktab_bench[VALUE][indA]), false);
		gtk_widget_set_sensitive(glab->gtktab_bench[VALUE][indS],          true);
		gtk_widget_set_sensitive(glab->gtktab_bench[VALUE][PARAMTHREADS],  true);
	}
}

/* Get label ID ('type' must be "lab" or "val") */
static gchar *get_id(const gchar *objectstr, gchar *type)
{
	static gchar *buff = NULL;
	gchar **split;

	g_free(buff);
	split = g_strsplit(objectstr, "_", 2);
	buff  = g_strconcat(split[0], "_", type, split[1], NULL);
	g_strfreev(split);

	return buff;
}

/* Search file location in standard paths */
static gchar *data_path(const gchar *file)
{
	int i;
	gboolean file_exists = false;
	static gchar *path = NULL;
	const gchar *prgname = g_get_prgname();
	const gchar *const *paths = g_get_system_data_dirs();

	for(i = 0; (!file_exists) && (paths[i] != NULL); i++)
	{
		g_free(path);
		path = g_build_filename(paths[i], prgname, file, NULL);
		file_exists = g_file_test(path, G_FILE_TEST_EXISTS);
	}

	return path;
}

/* Retrieve widgets from GtkBuilder */
static void get_widgets(GtkBuilder *builder, GtkLabels *glab)
{
	int i;

	glab->mainwindow     = GTK_WIDGET(gtk_builder_get_object(builder, "mainwindow"));
	glab->settingsbutton = GTK_WIDGET(gtk_builder_get_object(builder, "settingsbutton"));
	glab->daemonbutton   = GTK_WIDGET(gtk_builder_get_object(builder, "daemonbutton"));
	glab->labprgver	     = GTK_WIDGET(gtk_builder_get_object(builder, "labprgver"));
	glab->footer         = GTK_WIDGET(gtk_builder_get_object(builder, "footer_box"));
	glab->notebook       = GTK_WIDGET(gtk_builder_get_object(builder, "header_notebook"));
	glab->logocpu        = GTK_WIDGET(gtk_builder_get_object(builder, "proc_logocpu"));
	glab->activecore     = GTK_WIDGET(gtk_builder_get_object(builder, "trg_activecore"));
	glab->activetest     = GTK_WIDGET(gtk_builder_get_object(builder, "test_activetest"));
	glab->activecard     = GTK_WIDGET(gtk_builder_get_object(builder, "cards_activecard"));
	glab->logoprg        = GTK_WIDGET(gtk_builder_get_object(builder, "about_logoprg"));
	glab->butcol         = GTK_WIDGET(gtk_builder_get_object(builder, "colorbutton"));

	glab->settingswindow   = GTK_WIDGET(gtk_builder_get_object(builder, "settingswindow"));
	glab->validatebutton   = GTK_WIDGET(gtk_builder_get_object(builder, "validatebutton"));
	glab->cancelbutton     = GTK_WIDGET(gtk_builder_get_object(builder, "cancelbutton"));
	glab->refreshtime      = GTK_WIDGET(gtk_builder_get_object(builder, "refreshtime_val"));
	glab->theme            = GTK_WIDGET(gtk_builder_get_object(builder, "theme_val"));
	glab->defaulttab       = GTK_WIDGET(gtk_builder_get_object(builder, "defaulttab_val"));
	glab->defaultcore      = GTK_WIDGET(gtk_builder_get_object(builder, "defaultcore_val"));
	glab->defaultcachetest = GTK_WIDGET(gtk_builder_get_object(builder, "defaultcachetest_val"));
	glab->defaultcard      = GTK_WIDGET(gtk_builder_get_object(builder, "defaultcard_val"));
	glab->cpuiddecimal     = GTK_WIDGET(gtk_builder_get_object(builder, "cpuiddecimal"));
	glab->startdaemon      = GTK_WIDGET(gtk_builder_get_object(builder, "startdaemon"));

	gtk_widget_set_name(glab->footer, "footer_box");
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glab->notebook), opts->selected_page);

	/* Various labels to translate */
	for(i = TABCPU; i < LASTOBJ; i++)
		glab->gtktrad[i] = GTK_WIDGET(gtk_builder_get_object(builder, trad[i]));

	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		glab->gtktab_cpu[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcpu[i], "lab")));
		glab->gtktab_cpu[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcpu[i], "val")));
		gtk_widget_set_name(glab->gtktab_cpu[VALUE][i], "value");
	}

	/* Tab Caches */
	for(i = L1SIZE; i < LASTCACHES; i++)
	{
		glab->gtktab_caches[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcache[i], "lab")));
		glab->gtktab_caches[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcache[i], "val")));
		gtk_widget_set_name(glab->gtktab_caches[VALUE][i], "value");
	}
	glab->gridcaches = GTK_WIDGET(gtk_builder_get_object(builder, "caches_grid"));

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
	{
		glab->gtktab_motherboard[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectmb[i], "lab")));
		glab->gtktab_motherboard[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectmb[i], "val")));
		gtk_widget_set_name(glab->gtktab_motherboard[VALUE][i], "value");
	}

	/* Tab RAM */
	for(i = BANK0; i < LASTMEMORY; i++)
	{
		glab->gtktab_memory[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectram[i], "lab")));
		glab->gtktab_memory[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectram[i], "val")));
		gtk_widget_set_name(glab->gtktab_memory[VALUE][i], "value");
	}
	glab->gridbanks   = GTK_WIDGET(gtk_builder_get_object(builder, "memory_grid"));
	glab->scrollbanks = GTK_WIDGET(gtk_builder_get_object(builder, "memory_scrolledwindow"));

	/* Tab System */
	for(i = KERNEL; i < LASTSYSTEM; i++)
	{
		glab->gtktab_system[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectsys[i], "lab")));
		glab->gtktab_system[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectsys[i], "val")));
		gtk_widget_set_name(glab->gtktab_system[VALUE][i], "value");
	}
	for(i = BARUSED; i < LASTBAR; i++)
	{
		glab->bar[i] = GTK_WIDGET(gtk_builder_get_object(builder, objectsys_bar[i]));
		gtk_widget_set_name(GTK_WIDGET(glab->bar[i]), format("%i", i));
	}

	/* Tab Graphics */
	for(i = GPU1VENDOR; i < GPUFIELDS; i++)
	{
		glab->gtktab_graphics[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectgpu[i], "lab")));
		glab->gtktab_graphics[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectgpu[i], "val")));
		gtk_widget_set_name(glab->gtktab_graphics[VALUE][i], "value");
	}
	glab->gridcards = GTK_WIDGET(gtk_builder_get_object(builder, "graphics_box"));

	/* Tab Bench */
	for(i = PRIMESLOWSCORE; i < LASTBENCH; i++)
	{
		glab->gtktab_bench[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectbench[i], "lab")));
		glab->gtktab_bench[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectbench[i], "val")));
		gtk_widget_set_name(glab->gtktab_bench[VALUE][i], objectbench[i]);
	}

	/* Tab About */
	for(i = DESCRIPTION; i < LASTABOUT; i++)
		glab->gtktab_about[i] = GTK_WIDGET(gtk_builder_get_object(builder, objectabout[i]));
}

static gboolean is_dark_theme(GtkLabels *glab)
{
	gdouble contrast;
	GdkRGBA *fg, *bg;
	GtkStyleContext *context;

	if (theme != AUTO)
		return (theme == DARK);

	context = gtk_widget_get_style_context(GTK_WIDGET(glab->mainwindow));
	gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL,
		GTK_STYLE_PROPERTY_COLOR, &fg,
		GTK_STYLE_PROPERTY_BACKGROUND_COLOR, &bg,
		NULL);
	contrast = bg->red - fg->red + bg->green - fg->green + bg->blue - fg->blue;
	gdk_rgba_free(fg);
	gdk_rgba_free(bg);

	return (contrast < -1);
}

/* Set custom GTK theme */
static void set_colors(GtkLabels *glab)
{
	gchar *filename = NULL;
	GtkCssProvider *provider = gtk_css_provider_new();

	if(gtk_check_version(3, 19, 2) == NULL) // GTK 3.20 or newer
		filename = is_dark_theme(glab) ? g_strdup("cpu-x-gtk-3.20-dark.css") : g_strdup("cpu-x-gtk-3.20.css");
	else // GTK 3.12 to 3.18
		filename = is_dark_theme(glab) ? g_strdup("cpu-x-gtk-3.12-dark.css") : g_strdup("cpu-x-gtk-3.12.css");

	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_css_provider_load_from_path(provider, data_path(filename), NULL);

	g_free(filename);
	g_object_unref(provider);
}

/* Set CPU vendor logo and program logo */
static void set_logos(GtkLabels *glab, Labels *data)
{
	int width, height, prg_size = 72;
	GdkPixbuf *cpu_pixbuf = NULL, *unknown_pixbuf = NULL, *prg_pixbuf = NULL;
	GError *error = NULL;

	width  = gtk_widget_get_allocated_width(glab->gtktab_cpu[VALUE][SPECIFICATION]) -
	         gtk_widget_get_allocated_width(glab->gtktab_cpu[VALUE][VENDOR]) - 6;
	height = (gtk_widget_get_allocated_height(glab->gtktab_cpu[VALUE][VENDOR]) + 4) * 4;

	cpu_pixbuf     = gdk_pixbuf_new_from_file_at_scale(data_path(format("%s.png", data->tab_cpu[VALUE][VENDOR])), width, height, TRUE, &error);
	unknown_pixbuf = gdk_pixbuf_new_from_file_at_scale(data_path("Unknown.png"), width, height, TRUE, NULL);
	prg_pixbuf     = gdk_pixbuf_new_from_file_at_scale(data_path("CPU-X.png"), prg_size, prg_size, TRUE, NULL);

	gtk_window_set_icon(GTK_WINDOW(glab->mainwindow),   prg_pixbuf);
	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), cpu_pixbuf);
	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logoprg), prg_pixbuf);
	g_object_unref(cpu_pixbuf);
	g_object_unref(prg_pixbuf);

	if(error != NULL)
		gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), unknown_pixbuf);
	g_object_unref(unknown_pixbuf);
}

/* Filling all labels */
static void set_labels(GtkLabels *glab, Labels *data)
{
	int i, j;
	const int pkcheck = system(format("pkcheck --action-id org.freedesktop.policykit.exec --process %u > /dev/null 2>&1", getpid()));
	const gint width1 = gtk_widget_get_allocated_width(glab->gtktab_system[VALUE][COMPILER]);
	const gint width2 = width1 - gtk_widget_get_allocated_width(glab->gtktab_system[VALUE][USED]) - 6;
	GtkRequisition requisition;

	/* Footer label */
	gtk_label_set_text(GTK_LABEL(glab->labprgver), data->tab_about[VERSIONSTR]);
	gtk_widget_set_sensitive(glab->daemonbutton, false);
	if(DAEMON_UP)
		gtk_widget_set_tooltip_text(glab->daemonbutton, _("Connected to daemon"));
	else if(WEXITSTATUS(pkcheck) > 2)
		gtk_widget_set_tooltip_text(glab->daemonbutton, _("No polkit authentication agent found"));
	else
	{
		gtk_widget_set_sensitive(glab->daemonbutton, true);
		gtk_widget_set_tooltip_text(glab->daemonbutton, _("Ask password to start daemon in background"));
	}

	/* Various labels to translate */
	for(i = TABCPU; i < LASTOBJ; i++)
		gtk_label_set_text(GTK_LABEL(glab->gtktrad[i]), data->objects[i]);
	gtk_button_set_label(GTK_BUTTON(glab->daemonbutton), _("Start daemon"));

	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[NAME][i]),  data->tab_cpu[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][i]), data->tab_cpu[VALUE][i]);
	}
	for(i = 0; i < data->cpu_count; i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->activecore), format(_("Core #%i"), i));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->activecore), opts->selected_core);

	/* Tab Caches */
	for(i = L1SIZE; i < LASTCACHES; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[NAME][i]),  data->tab_caches[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[VALUE][i]), data->tab_caches[VALUE][i]);
	}
	for(i = LASTCACHES / CACHEFIELDS; i > data->cache_count; i--)
		gtk_grid_remove_row(GTK_GRID(glab->gridcaches), i - 1);
	for(i = 0; i < data->w_data->test_count; i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->activetest), data->w_data->test_name[i]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->activetest), opts->bw_test);
	if(!data->cache_count)
		gtk_widget_hide(GTK_WIDGET(glab->gridcaches));

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_motherboard[NAME][i]),  data->tab_motherboard[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_motherboard[VALUE][i]), data->tab_motherboard[VALUE][i]);
	}

	/* Tab RAM */
	for(i = BANK0; i < data->dimm_count; i++)
	{
		g_strdelimit(data->tab_memory[VALUE][i], ",", '\n');
		gtk_label_set_text(GTK_LABEL(glab->gtktab_memory[NAME][i]),  data->tab_memory[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_memory[VALUE][i]), data->tab_memory[VALUE][i]);
	}
	for(i = BANK7; i >= data->dimm_count; i--)
		gtk_grid_remove_row(GTK_GRID(glab->gridbanks), i);
	if(!data->dimm_count)
		gtk_widget_hide(GTK_WIDGET(glab->scrollbanks));

	/* Tab System */
	for(i = KERNEL; i < LASTSYSTEM; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_system[NAME][i]),  data->tab_system[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_system[VALUE][i]), data->tab_system[VALUE][i]);
	}
	for(i = BARUSED; i < LASTBAR; i++)
	{
		gtk_widget_get_preferred_size(glab->gtktab_system[VALUE][USED], NULL, &requisition);
		gtk_widget_set_size_request(glab->bar[i], width2, requisition.height);
	}

	/* Tab Graphics */
	for(i = opts->selected_gpu * GPUFIELDS, j = 0; j < GPUFIELDS; i++, j++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[NAME][j]),  data->tab_graphics[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][j]), data->tab_graphics[VALUE][i]);
	}
	gtk_label_set_text(GTK_LABEL(glab->gtktrad[FRAMGPU1]), data->objects[FRAMGPU1 + opts->selected_gpu]);
	if(data->gpu_count == 0)
		gtk_widget_hide(GTK_WIDGET(glab->gridcards));
	for(i = 0; i < data->gpu_count; i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->activecard), format("#%i: %s", i, data->tab_graphics[VALUE][GPU1MODEL + i * GPUFIELDS]));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->activecard), opts->selected_gpu);

	/* Tab Bench */
	for(i = PRIMESLOWSCORE; i < LASTBENCH; i++)
		gtk_label_set_text(GTK_LABEL(glab->gtktab_bench[NAME][i]), data->tab_bench[NAME][i]);
	for(i = PRIMESLOWSCORE; i <= PRIMEFASTSCORE; i += BENCHFIELDS)
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(glab->gtktab_bench[VALUE][i]), data->tab_bench[VALUE][i]);
		gtk_widget_set_size_request(glab->gtktab_bench[VALUE][i], width1, -1);
	}
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(glab->gtktab_bench[VALUE][PARAMDURATION]), 1, 60);
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(glab->gtktab_bench[VALUE][PARAMTHREADS]),  1, 1);
	gtk_spin_button_set_range     (GTK_SPIN_BUTTON(glab->gtktab_bench[VALUE][PARAMDURATION]), 1, 60 * 24);
	gtk_spin_button_set_range     (GTK_SPIN_BUTTON(glab->gtktab_bench[VALUE][PARAMTHREADS]),  1, data->cpu_count);

	/* Tab About */
	for(i = DESCRIPTION; i < LASTABOUT; i++)
		gtk_label_set_text(GTK_LABEL(glab->gtktab_about[i]), data->tab_about[i]);

	/* Settings window */
	gtk_spin_button_set_range     (GTK_SPIN_BUTTON(glab->refreshtime),  1, G_MAXUSHORT);
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(glab->refreshtime),  1, 60);
	for (i = NO_CPU; i <= NO_ABOUT; i++)
		gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(glab->defaulttab), i, nicktab[i], format(_("%s"), data->objects[i]));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->defaulttab), opts->selected_page);
	for(i = 0; i < data->cpu_count; i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->defaultcore), format(_("Core #%i"), i));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->defaultcore), opts->selected_core);
	for(i = 0; i < data->w_data->test_count; i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->defaultcachetest), data->w_data->test_name[i]);
	for(i = 0; i < data->gpu_count; i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->defaultcard), format("#%i: %s", i, data->tab_graphics[VALUE][GPU1MODEL + i * GPUFIELDS]));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->defaultcachetest), opts->bw_test);
	gtk_widget_set_sensitive(GTK_WIDGET(glab->defaultcachetest), data->cache_count > 0);
}

/* Call defined functions on signals */
static void set_signals(GtkLabels *glab, Labels *data, GThrd *refr)
{
	int i;

	g_signal_connect(glab->mainwindow,     "destroy",      G_CALLBACK(gtk_main_quit),        NULL);
	g_signal_connect(glab->settingswindow, "delete-event", G_CALLBACK(hide_settings_window), NULL);
	g_signal_connect(glab->settingsbutton, "clicked",      G_CALLBACK(open_settings_window), glab);
	g_signal_connect(glab->daemonbutton,   "clicked",      G_CALLBACK(reload_with_daemon),   refr);
	g_signal_connect(glab->activecore,     "changed",      G_CALLBACK(change_activecore),    data);
	g_signal_connect(glab->activetest,     "changed",      G_CALLBACK(change_activetest),    data);
	g_signal_connect(glab->activecard,     "changed",      G_CALLBACK(change_activecard),    refr);

	/* Tab Bench */
	g_signal_connect(glab->gtktab_bench[VALUE][PRIMESLOWRUN],  "button-press-event", G_CALLBACK(start_benchmark_bg), refr);
	g_signal_connect(glab->gtktab_bench[VALUE][PRIMEFASTRUN],  "button-press-event", G_CALLBACK(start_benchmark_bg), refr);
	g_signal_connect(glab->gtktab_bench[VALUE][PARAMDURATION], "value-changed",      G_CALLBACK(change_benchparam),  data);
	g_signal_connect(glab->gtktab_bench[VALUE][PARAMTHREADS],  "value-changed",      G_CALLBACK(change_benchparam),  data);

	/* Tab System */
	for(i = BARUSED; i < LASTBAR; i++)
		g_signal_connect(G_OBJECT(glab->bar[i]),  "draw", G_CALLBACK(fill_frame), refr);

	/* Settings window */
	g_signal_connect(glab->validatebutton, "clicked", G_CALLBACK(save_settings), glab);
	g_signal_connect(glab->cancelbutton,   "clicked", G_CALLBACK(close_settings_window), glab);
}

/* Draw bars in Memory tab */
void fill_frame(GtkWidget *widget, cairo_t *cr, GThrd *refr)
{
	int i = USED, page;
	guint width, height;
	double before = 0, percent = 0;
	const gchar *widget_name;
	cairo_pattern_t *pat;
	PangoLayout *reflayout, *newlayout;

	Labels *(data)    = refr->data;
	GtkLabels *(glab) = refr->glab;

	widget_name = gtk_widget_get_name(widget);
	width       = gtk_widget_get_allocated_width(widget);
	height      = gtk_widget_get_allocated_height(widget);
	page        = atoi(widget_name);
	reflayout   = gtk_label_get_layout(GTK_LABEL(glab->gtktab_system[VALUE][page + USED]));
	newlayout   = pango_layout_copy(reflayout);

	if((page == BARSWAP && data->m_data->swap_total == 0) || (page != BARSWAP && data->m_data->mem_total == 0))
		return;

	for(i = BARUSED; (i <= page) && (page != BARSWAP); i++) /* Get value to start */
	{
		before += percent;
		percent = (double) data->m_data->mem_usage[i] / data->m_data->mem_total * 100;
	}

	if(page == BARSWAP)
		percent = (double) data->m_data->mem_usage[BARSWAP] / data->m_data->swap_total * 100;

	pat = cairo_pattern_create_linear(before / 100 * width, 0, percent / 100 * width, height);

	switch(page) /* Set differents level bar color */
	{
		case BARUSED: // yellow
			pat = is_dark_theme(glab) ? cairo_pattern_create_rgba(0.01, 0.02, 0.37, 1.00) : cairo_pattern_create_rgba(1.00, 0.92, 0.23, 1.00);
			break;
		case BARBUFFERS: // blue
			pat = is_dark_theme(glab) ? cairo_pattern_create_rgba(0.01, 0.24, 0.54, 1.00) : cairo_pattern_create_rgba(0.13, 0.59, 0.95, 1.00);
			break;
		case BARCACHED: // purple
			pat = is_dark_theme(glab) ? cairo_pattern_create_rgba(0.00, 0.47, 0.71, 1.00) : cairo_pattern_create_rgba(0.61, 0.15, 0.69, 1.00);
			break;
		case BARFREE: // green
			pat = is_dark_theme(glab) ? cairo_pattern_create_rgba(0.00, 0.59, 0.78, 1.00) : cairo_pattern_create_rgba(0.30, 0.69, 0.31, 1.00);
			break;
		case BARSWAP: // red
			pat = is_dark_theme(glab) ? cairo_pattern_create_rgba(0.00, 0.71, 0.85, 1.00) : cairo_pattern_create_rgba(0.96, 0.26, 0.21, 1.00);
			break;
	}

	cairo_rectangle(cr, before / 100 * width, 0, percent / 100 * width, height); /* Print a colored rectangle */
	cairo_set_source(cr, pat);
	cairo_fill(cr);
	cairo_pattern_destroy(pat);

	/* Print percentage */
	if(is_dark_theme(glab))
		cairo_set_source_rgb(cr, 0.8080, 0.8080, 0.0);
	else
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.8080);

	cairo_move_to(cr, -40, 0);
	pango_layout_set_text(newlayout, format("%.2f%%", percent), -1);
	pango_cairo_show_layout(cr, newlayout);
	cairo_fill(cr);
	g_object_unref(newlayout);
}
