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
* FILE gui_gtk.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libintl.h>
#include "cpu-x.h"
#include "gui_gtk.h"
#include "gui_gtk_id.h"

#if PORTABLE_BINARY
# include "gtk-resources.h"
#endif


void start_gui_gtk(int *argc, char **argv[], Labels *data)
{
	GtkBuilder *builder;
	GtkLabels glab;
	GThrd refr;

	MSG_VERBOSE(_("Starting GTK GUI..."));
	gtk_init(argc, argv);
	builder = gtk_builder_new();
	refr.glab = &glab;
	refr.data = data;

	/* Build UI from Glade file */
#if PORTABLE_BINARY
	g_resources_register(cpu_x_get_resource());

	if(gtk_builder_add_from_resource(builder, "/cpu-x/ui/cpu-x-gtk-3.16.ui", NULL))
		goto open_ok;
	if(gtk_builder_add_from_resource(builder, "/cpu-x/ui/cpu-x-gtk-3.8.ui", NULL))
		goto open_ok;
#else
	if(gtk_builder_add_from_file(builder, data_path("cpu-x-gtk-3.16.ui"), NULL))
		goto open_ok;
	if(gtk_builder_add_from_file(builder, data_path("cpu-x-gtk-3.8.ui"), NULL))
		goto open_ok;
#endif
	MSG_ERROR_ERRNO(_("Import UI in GtkBuilder failed"));
	exit(EXIT_FAILURE);

	open_ok:
	g_set_prgname(g_ascii_strdown(PRGNAME, -1));
	glab.mainwindow	 = GTK_WIDGET(gtk_builder_get_object(builder, "mainwindow"));
	glab.closebutton = GTK_WIDGET(gtk_builder_get_object(builder, "closebutton"));
	glab.labprgver	 = GTK_WIDGET(gtk_builder_get_object(builder, "labprgver"));
	get_labels(builder, &glab);
	g_object_unref(G_OBJECT(builder));

	set_logos(&glab, data); /* Vendor icon */
	set_labels(&glab, data);
	labels_free(data);

	if(getuid()) /* Show warning if not root */
		warning_window(glab.mainwindow);

	if(PORTABLE_BINARY && new_version != NULL)
		new_version_window(glab.mainwindow);

	g_signal_connect(glab.mainwindow,  "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(glab.closebutton, "clicked", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(glab.activecore, "changed", G_CALLBACK(change_activecore), data);
	g_signal_connect(glab.activetest, "changed", G_CALLBACK(change_activetest), data);
	if(gtk_check_version(3, 15, 0) != NULL) // Only for GTK 3.14 or older
		g_signal_connect(glab.butcol, "color-set", G_CALLBACK(change_color), &glab);

#if HAS_LIBPROCPS || HAS_LIBSTATGRAB
	g_signal_connect(G_OBJECT(glab.barused),  "draw", G_CALLBACK(fill_frame), &refr); /* Level bars */
	g_signal_connect(G_OBJECT(glab.barbuff),  "draw", G_CALLBACK(fill_frame), &refr);
	g_signal_connect(G_OBJECT(glab.barcache), "draw", G_CALLBACK(fill_frame), &refr);
	g_signal_connect(G_OBJECT(glab.barfree),  "draw", G_CALLBACK(fill_frame), &refr);
	g_signal_connect(G_OBJECT(glab.barswap),  "draw", G_CALLBACK(fill_frame), &refr);
#endif /* HAS_LIBPROCPS || HAS_LIBSTATGRAB */

	set_colors(&glab);
	g_timeout_add_seconds(opts->refr_time, (gpointer)grefresh, &refr);
	gtk_main();
}

void warning_window(GtkWidget *mainwindow)
{
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(mainwindow),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_NONE,
		_("Root privileges are required to work properly"));

	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
		_("Some informations will not be readable, some labels will be empty."));

	gtk_dialog_add_buttons(GTK_DIALOG(dialog), _("Ignore"), GTK_RESPONSE_REJECT, NULL);
	if(command_exists("pkexec") && command_exists("cpu-x_polkit"))
		gtk_dialog_add_buttons(GTK_DIALOG(dialog), _("Run as root"), GTK_RESPONSE_ACCEPT, NULL);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		system("cpu-x_polkit &");
		exit(EXIT_SUCCESS);
	}

	gtk_widget_destroy(dialog);
}

void new_version_window(GtkWidget *mainwindow)
{
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(mainwindow),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_NONE,
		_("A new version of %s is available!"), PRGNAME);

	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
		_("Do you want to update %s to version %s after exit?\n"
		"It will erase this binary file (%s) by the new version."),
		PRGNAME, new_version, binary_name);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), _("Not now"), GTK_RESPONSE_REJECT, _("Update"), GTK_RESPONSE_ACCEPT, NULL);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		opts->update = true;

	gtk_widget_destroy(dialog);
}

gboolean grefresh(GThrd *refr)
{
	int i;
	enum EnTabNumber page;
	Labels *(data) = refr->data;
	GtkLabels *(glab) = refr->glab;

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(glab->notebook));
	do_refresh(data, page);

	switch(page)
	{
		case NO_CPU:
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][VOLTAGE]),     data->tab_cpu[VALUE][VOLTAGE]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][TEMPERATURE]), data->tab_cpu[VALUE][TEMPERATURE]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][MULTIPLIER]),  data->tab_cpu[VALUE][MULTIPLIER]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][CORESPEED]),   data->tab_cpu[VALUE][CORESPEED]);
			gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][USAGE]),       data->tab_cpu[VALUE][USAGE]);
			break;
		case NO_CACHES:
			for(i = L1SPEED; i < LASTCACHES; i += CACHEFIELDS)
				gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[VALUE][i]), data->tab_caches[VALUE][i]);
			break;
		case NO_SYSTEM:
			gtk_label_set_text(GTK_LABEL(glab->gtktab_system[VALUE][UPTIME]),      data->tab_system[VALUE][UPTIME]);
			for(i = USED; i < LASTSYSTEM; i++)
				gtk_label_set_text(GTK_LABEL(glab->gtktab_system[VALUE][i]),   data->tab_system[VALUE][i]);
			break;
		case NO_GRAPHICS:
			for(i = 0; i < data->gpu_count; i += GPUFIELDS)
				gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][GPU1TEMPERATURE + i]), data->tab_graphics[VALUE][GPU1TEMPERATURE + i]);
			break;
		default:
			break;
	}

	return G_SOURCE_CONTINUE;
}

void change_activecore(GtkComboBox *box, Labels *data)
{
	const gint core = gtk_combo_box_get_active(GTK_COMBO_BOX(box));

	if(0 <= core && core < data->cpu_count)
		opts->selected_core = core;
}

void change_activetest(GtkComboBox *box, Labels *data)
{
	const gint test = gtk_combo_box_get_active(GTK_COMBO_BOX(box));

	if(0 <= test && test < bandwidth_last_test())
		opts->bw_test = test;
}

void set_colors(GtkLabels *glab)
{
	if(gtk_check_version(3, 15, 0) == NULL) // GTK 3.16 or newer
	{
		char *filename;
		GtkCssProvider *provider;

		if(gtk_check_version(3, 19, 2) == NULL) // GTK 3.20 or newer
			filename = g_strdup("cpu-x-gtk-3.20.css");
		else // GTK 3.16 or 3.18
			filename = g_strdup("cpu-x-gtk-3.16.css");

		provider = gtk_css_provider_new();
		gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
		gtk_css_provider_load_from_path(provider, data_path(filename), NULL);

		g_object_unref(provider);
	}
#if PORTABLE_BINARY || !GTK_CHECK_VERSION(3, 15, 0)
	else
	{
		GdkRGBA window_colors;

		window_colors.red	= 0.3;
		window_colors.green	= 0.6;
		window_colors.blue	= 0.9;
		window_colors.alpha	= 0.95;
		gtk_widget_override_background_color(glab->mainwindow, GTK_STATE_FLAG_NORMAL, &window_colors);
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(glab->butcol), &window_colors);
	}
#endif /* PORTABLE_BINARY || !GTK_CHECK_VERSION(3, 15, 0) */
}

void change_color(GtkWidget *button, GtkLabels *glab)
{
#if PORTABLE_BINARY || !GTK_CHECK_VERSION(3, 15, 0)
	GdkRGBA color;

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &color);
	gtk_widget_override_background_color(glab->mainwindow, GTK_STATE_FLAG_NORMAL, &color);
#endif /* PORTABLE_BINARY || !GTK_CHECK_VERSION(3, 15, 0) */
}

void set_logos(GtkLabels *glab, Labels *data)
{
	const int width = 105, height = 92, prg_size = 92;
	char *name;
	GdkPixbuf *cpu_pixbuf, *prg_pixbuf;
	GError *error = NULL;

#if PORTABLE_BINARY
	/* CPU-X logo in About tab */
	prg_pixbuf = gdk_pixbuf_new_from_resource_at_scale(RESOURCE_PICTURES "CPU-X.png", prg_size, prg_size, TRUE, NULL);
	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logoprg), prg_pixbuf);

	/* CPU logo in CPU tab */
	iasprintf(&name, RESOURCE_PICTURES "%s.png", data->tab_cpu[VALUE][VENDOR]);
	cpu_pixbuf = gdk_pixbuf_new_from_resource_at_scale(name, width, height, TRUE, &error);
	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), cpu_pixbuf);

	/* Unknown CPU logo */
	if(error != NULL)
	{
		cpu_pixbuf = gdk_pixbuf_new_from_resource_at_scale(RESOURCE_PICTURES UNKNOWN_VENDOR, width, height, TRUE, NULL);
		gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), cpu_pixbuf);
	}
#else
	/* CPU-X logo in About tab */
	prg_pixbuf = gdk_pixbuf_new_from_file_at_scale(data_path("CPU-X.png"), prg_size, prg_size, TRUE, NULL);
	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logoprg), prg_pixbuf);

	/* CPU logo in CPU tab */
	iasprintf(&name, "%s.png", data->tab_cpu[VALUE][VENDOR]);
	cpu_pixbuf = gdk_pixbuf_new_from_file_at_scale(data_path(name), width, height, TRUE, &error);
	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), cpu_pixbuf);

	/* Unknown CPU logo */
	if(error != NULL)
	{
		cpu_pixbuf = gdk_pixbuf_new_from_file_at_scale(data_path(UNKNOWN_VENDOR), width, height, TRUE, NULL);
		gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), cpu_pixbuf);
	}
#endif
}

char *get_id(const char *objectstr, char *type)
{
	static char *buff;
	gchar **split;

	split = g_strsplit(objectstr, "_", 2);
	buff  = g_strconcat(split[0], "_", type, split[1], NULL);
	g_strfreev(split);

	return buff;
}

void get_labels(GtkBuilder *builder, GtkLabels *glab)
{
	int i;

	glab->notebook = GTK_WIDGET(gtk_builder_get_object(builder, "header_notebook"));
	glab->logocpu = GTK_WIDGET(gtk_builder_get_object(builder, "proc_logocpu"));
	glab->activecore = GTK_WIDGET(gtk_builder_get_object(builder, "trg_activecore"));
	glab->activetest = GTK_WIDGET(gtk_builder_get_object(builder, "test_activetest"));
	glab->logoprg = GTK_WIDGET(gtk_builder_get_object(builder, "about_logoprg"));
	glab->butcol = GTK_WIDGET(gtk_builder_get_object(builder, "colorbutton"));

	/* Various labels to translate */
	for(i = TABCPU; i < LASTOBJ; i++)
		glab->gtktrad[i] = GTK_WIDGET(gtk_builder_get_object(builder, trad[i]));

	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		glab->gtktab_cpu[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcpu[i], "lab")));
		glab->gtktab_cpu[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcpu[i], "val")));
	}

	/* Tab Caches */
	for(i = L1SIZE; i < LASTCACHES; i++)
	{
		glab->gtktab_caches[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcache[i], "lab")));
		glab->gtktab_caches[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectcache[i], "val")));
	}

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
	{
		glab->gtktab_motherboard[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectmb[i], "lab")));
		glab->gtktab_motherboard[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectmb[i], "val")));
	}

	/* Tab RAM */
	for(i = BANK0_0; i < LASTMEMORY; i++)
	{
		glab->gtktab_memory[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectram[i], "lab")));
		glab->gtktab_memory[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectram[i], "val")));
	}
	glab->gridbanks = GTK_WIDGET(gtk_builder_get_object(builder, "banks_grid"));

	/* Tab System */
	for(i = KERNEL; i < LASTSYSTEM; i++)
	{
		glab->gtktab_system[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectsys[i], "lab")));
		glab->gtktab_system[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectsys[i], "val")));
	}
	glab->barused  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barused"));
	glab->barbuff  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barbuff"));
	glab->barcache = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barcache"));
	glab->barfree  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barfree"));
	glab->barswap  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barswap"));
	gtk_widget_set_name(glab->barused,  g_strdup_printf("%i", USED));
	gtk_widget_set_name(glab->barbuff,  g_strdup_printf("%i", BUFFERS));
	gtk_widget_set_name(glab->barcache, g_strdup_printf("%i", CACHED));
	gtk_widget_set_name(glab->barfree,  g_strdup_printf("%i", FREE));
	gtk_widget_set_name(glab->barswap,  g_strdup_printf("%i", SWAP));

	/* Tab Graphics */
	for(i = GPU1VENDOR; i < LASTGRAPHICS; i++)
	{
		glab->gtktab_graphics[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectgpu[i], "lab")));
		glab->gtktab_graphics[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, get_id(objectgpu[i], "val")));
	}
	glab->gridcards = GTK_WIDGET(gtk_builder_get_object(builder, "graphics_box"));
}

void set_labels(GtkLabels *glab, Labels *data)
{
	int i;
	gchar buff[9];

	gtk_label_set_text(GTK_LABEL(glab->labprgver), data->objects[LABVERSION]); /* Footer label */

	/* Various labels to translate */
	for(i = TABCPU; i < LASTOBJ; i++)
		gtk_label_set_text(GTK_LABEL(glab->gtktrad[i]), data->objects[i]);

	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[NAME][i]), data->tab_cpu[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_cpu[VALUE][i]), data->tab_cpu[VALUE][i]);
	}
	for(i = 0; i < data->cpu_count; i++)
	{
		sprintf(buff, _("Core #%i"), i);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->activecore), buff);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->activecore), opts->selected_core);

	/* Tab Caches */
	for(i = L1SIZE; i < LASTCACHES; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[NAME][i]), data->tab_caches[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[VALUE][i]), data->tab_caches[VALUE][i]);
	}
	for(i = 0; i < bandwidth_last_test(); i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(glab->activetest), bandwidth_test_name(i));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glab->activetest), opts->bw_test);

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_motherboard[NAME][i]), data->tab_motherboard[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_motherboard[VALUE][i]), data->tab_motherboard[VALUE][i]);
	}

	/* Tab RAM */
	for(i = BANK0_0; i < data->dimms_count; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_memory[NAME][i]), data->tab_memory[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_memory[VALUE][i]), data->tab_memory[VALUE][i]);
	}
	for(i = BANK7_1; i >= data->dimms_count; i--)
		gtk_grid_remove_row(GTK_GRID(glab->gridbanks), i);

	/* Tab System */
	for(i = KERNEL; i < LASTSYSTEM; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_system[NAME][i]), data->tab_system[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_system[VALUE][i]), data->tab_system[VALUE][i]);
	}

	/* Tab Graphics */
	for(i = GPU1VENDOR; i < LASTGRAPHICS; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[NAME][i]), data->tab_graphics[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_graphics[VALUE][i]), data->tab_graphics[VALUE][i]);
	}
	for(i = LASTGRAPHICS; i >= data->gpu_count; i -= GPUFIELDS)
		gtk_grid_remove_row(GTK_GRID(glab->gridcards), i / GPUFIELDS);
}

#if HAS_LIBPROCPS || HAS_LIBSTATGRAB
void fill_frame(GtkWidget *widget, cairo_t *cr, GThrd *refr)
{
	int i = USED, n;
	guint width, height;
	double before = 0, percent;
	const char *widget_name;
	cairo_pattern_t *pat;
	PangoLayout *reflayout, *newlayout;
	Labels *(data) = refr->data;
	GtkLabels *(glab) = refr->glab;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	widget_name = gtk_widget_get_name(widget);
	n = atoi(widget_name);
	reflayout = gtk_label_get_layout(GTK_LABEL(glab->gtktab_system[VALUE][n]));
	newlayout = pango_layout_copy(reflayout);

	while(i < n) /* Get value to start */
	{
		before += (double) strtol(data->tab_system[VALUE][i], NULL, 10) /
			strtol(strstr(data->tab_system[VALUE][i], "/ ") + 2, NULL, 10) * 100;
		i++;
	}
	percent = (double) strtol(data->tab_system[VALUE][n], NULL, 10) /
		strtol(strstr(data->tab_system[VALUE][n], "/ ") + 2, NULL, 10) * 100;

	if(isnan(percent))
		percent = 0.00;

	pat = cairo_pattern_create_linear(before / 100 * width, 0, percent / 100 * width, height);

	switch(n) /* Set differents level bar color */
	{
		case USED:
			cairo_pattern_add_color_stop_rgba (pat, 0, 1.00, 1.00, 0.15, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1, 1.00, 0.75, 0.15, 1);
			break;
		case BUFFERS:
			cairo_pattern_add_color_stop_rgba (pat, 0, 0.00, 0.30, 0.75, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1, 0.25, 0.55, 1.00, 1);
			break;
		case CACHED:
			cairo_pattern_add_color_stop_rgba (pat, 0, 1.00, 0.35, 0.15, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1, 0.75, 0.15, 0.00, 1);
			break;
		case FREE:
			cairo_pattern_add_color_stop_rgba (pat, 0, 0.20, 1.00, 0.25, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1, 0.00, 0.75, 0.05, 1);
			break;
		case SWAP:
			cairo_pattern_add_color_stop_rgba (pat, 0, 1.00, 0.25, 0.90, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1, 0.75, 0.00, 0.65, 1);
			break;
	}

	cairo_rectangle(cr, before / 100 * width, 0, percent / 100 * width, height); /* Print a colored rectangle */
	cairo_set_source (cr, pat);
	cairo_fill(cr);
	cairo_pattern_destroy(pat);

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.8081); /* Print percentage */
	cairo_move_to(cr, -40, 0);
	pango_layout_set_text(newlayout, g_strdup_printf("%.2f%%", percent), -1);
	pango_cairo_show_layout(cr, newlayout);
	cairo_fill(cr);
	g_object_unref(newlayout);
}
#endif /* HAS_LIBPROCPS || HAS_LIBSTATGRAB */

/* Search file location to avoid hardcode them */
char *data_path(char *file)
{
	int i = 0;
	const char *prgname = g_get_prgname();
	const gchar *const *paths = g_get_system_data_dirs();
	static char *buffer;

	while(paths[i] != NULL)
	{
		gchar *path = g_build_filename(paths[i], prgname, file, NULL);
		if(g_file_test(path, G_FILE_TEST_EXISTS))
		{
			buffer = g_strdup(path);
			return buffer;
		}
		i++;
	}

	buffer = strdup(" ");
	return buffer;
}
