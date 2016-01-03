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

	if(gtk_builder_add_from_resource(builder, "/cpu-x/ui/cpux-gtk-3.14.ui", NULL))
		goto open_ok;
	if(gtk_builder_add_from_resource(builder, "/cpu-x/ui/cpux-gtk-3.8.ui", NULL))
		goto open_ok;
#else
	if(gtk_builder_add_from_file(builder, data_path("cpux-gtk-3.14.ui"), NULL))
		goto open_ok;
	if(gtk_builder_add_from_file(builder, data_path("cpux-gtk-3.8.ui"), NULL))
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

	g_signal_connect(glab.mainwindow,  "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(glab.closebutton, "clicked", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(glab.butcol, "color-set", G_CALLBACK(change_color), &glab);

#if HAS_LIBPROCPS || HAS_LIBSTATGRAB
	g_signal_connect(G_OBJECT(glab.barused),  "draw", G_CALLBACK(fill_frame), data); /* Level bars */
	g_signal_connect(G_OBJECT(glab.barbuff),  "draw", G_CALLBACK(fill_frame), data);
	g_signal_connect(G_OBJECT(glab.barcache), "draw", G_CALLBACK(fill_frame), data);
	g_signal_connect(G_OBJECT(glab.barfree),  "draw", G_CALLBACK(fill_frame), data);
	g_signal_connect(G_OBJECT(glab.barswap),  "draw", G_CALLBACK(fill_frame), data);
#endif /* HAS_LIBPROCPS || HAS_LIBSTATGRAB */

	set_colors(&glab);
	g_timeout_add_seconds(opts->refr_time, (gpointer)grefresh, &refr);
	gtk_main();
}

void warning_window(GtkWidget *mainwindow)
{
	GtkWidget *dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(mainwindow),
		GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_NONE,
		"\n\t\t\t<span font_weight='heavy' font_size='x-large'>%s</span>\n\n%s",
		_("WARNING"),
		_("root privileges are required to work properly"));

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	gtk_window_set_title(GTK_WINDOW(dialog), PRGNAME);

	if(system("pkexec --version > /dev/null"))
	{
		gtk_dialog_add_buttons(GTK_DIALOG (dialog), _("Ignore"), GTK_RESPONSE_REJECT, NULL);
		gtk_dialog_run(GTK_DIALOG(dialog));
	}
	else
	{
		gtk_dialog_add_buttons(GTK_DIALOG (dialog), _("Run as root"), GTK_RESPONSE_ACCEPT, _("Ignore"), GTK_RESPONSE_REJECT, NULL);

		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			system("cpu-x_polkit &");
			exit(EXIT_SUCCESS);
		}
	}

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

void set_colors(GtkLabels *glab)
{
	GdkRGBA window_colors;

	window_colors.red	= 0.3;
	window_colors.green	= 0.6;
	window_colors.blue	= 0.9;
	window_colors.alpha	= 0.95;
	gtk_widget_override_background_color(glab->mainwindow, GTK_STATE_FLAG_NORMAL, &window_colors);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(glab->butcol), &window_colors);
}

void change_color(GtkWidget *button, GtkLabels *glab)
{
	GdkRGBA color;

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &color);
	gtk_widget_override_background_color(glab->mainwindow, GTK_STATE_FLAG_NORMAL, &color);
}

void set_logos(GtkLabels *glab, Labels *data)
{
	char tmp[MAXSTR];
	const gchar *icon_name[MAXSTR];
#if PORTABLE_BINARY
	sprintf(tmp, "/cpu-x/pictures/%s.png", data->tab_cpu[VALUE][VENDOR]);

	gtk_image_set_from_resource(GTK_IMAGE(glab->logoprg), "/cpu-x/pictures/CPU-X.png"); /* Program icon in About */
	gtk_image_set_from_resource(GTK_IMAGE(glab->logocpu), tmp); /* CPU vendor icon */

	gtk_image_get_icon_name(GTK_IMAGE(glab->logocpu), icon_name, NULL);
	if(icon_name[0] != NULL) /* If no icon is set, apply "novendor.png" */
		gtk_image_set_from_resource(GTK_IMAGE(glab->logocpu), "/cpu-x/pictures/novendor.png");
#else
	sprintf(tmp, "%s.png", data->tab_cpu[VALUE][VENDOR]);

	gtk_image_set_from_file(GTK_IMAGE(glab->logoprg), data_path("CPU-X.png")); /* Program icon in About */
	gtk_image_set_from_file(GTK_IMAGE(glab->logocpu), data_path(tmp)); /* CPU vendor icon */

	gtk_image_get_icon_name(GTK_IMAGE(glab->logocpu), icon_name, NULL);
	if(icon_name[0] != NULL) /* If no icon is set, apply "novendor.png" */
		gtk_image_set_from_file(GTK_IMAGE(glab->logocpu), data_path("novendor.png"));
#endif
}

void get_labels(GtkBuilder *builder, GtkLabels *glab)
{
	int i;

	glab->notebook = GTK_WIDGET(gtk_builder_get_object(builder, "header_notebook"));
	glab->logocpu = GTK_WIDGET(gtk_builder_get_object(builder, "proc_logocpu"));
	glab->logoprg = GTK_WIDGET(gtk_builder_get_object(builder, "about_logoprg"));
	glab->butcol = GTK_WIDGET(gtk_builder_get_object(builder, "colorbutton"));

	/* Various labels to translate */
	for(i = TABCPU; i < LASTOBJ; i++)
		glab->gtktrad[i] = GTK_WIDGET(gtk_builder_get_object(builder, trad[i]));

	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		glab->gtktab_cpu[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectcpu[NAME][i]));
		glab->gtktab_cpu[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectcpu[VALUE][i]));
	}

	/* Tab Caches */
	for(i = L1SIZE; i < LASTCACHES; i++)
	{
		glab->gtktab_caches[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectcache[NAME][i]));
		glab->gtktab_caches[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectcache[VALUE][i]));
	}

	/* Tab Motherboard */
	for(i = MANUFACTURER; i < LASTMOTHERBOARD; i++)
	{
		glab->gtktab_motherboard[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectmb[NAME][i]));
		glab->gtktab_motherboard[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectmb[VALUE][i]));
	}

	/* Tab RAM */
	for(i = BANK0_0; i < LASTMEMORY; i++)
	{
		glab->gtktab_memory[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectram[NAME][i]));
		glab->gtktab_memory[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectram[VALUE][i]));
	}
	glab->gridbanks = GTK_WIDGET(gtk_builder_get_object(builder, "banks_grid"));

	/* Tab System */
	for(i = KERNEL; i < LASTSYSTEM; i++)
	{
		glab->gtktab_system[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectsys[NAME][i]));
		glab->gtktab_system[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectsys[VALUE][i]));
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
		glab->gtktab_graphics[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectgpu[NAME][i]));
		glab->gtktab_graphics[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectgpu[VALUE][i]));
	}
	glab->gridcards = GTK_WIDGET(gtk_builder_get_object(builder, "graphics_box"));
}

void set_labels(GtkLabels *glab, Labels *data)
{
	int i;

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

	/* Tab Caches */
	for(i = L1SIZE; i < LASTCACHES; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[NAME][i]), data->tab_caches[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktab_caches[VALUE][i]), data->tab_caches[VALUE][i]);
	}

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
void fill_frame(GtkWidget *widget, cairo_t *cr, Labels *data)
{
	int i = USED, n;
	guint width, height;
	double before = 0, percent;
	char text[MAXSTR];
	const char *widget_name;
	cairo_pattern_t *pat;
	GSettings *setting = g_settings_new("org.gnome.desktop.interface");
	gchar *font = g_settings_get_string(setting, "font-name");

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	widget_name = gtk_widget_get_name(widget);
	n = atoi(widget_name);

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

	snprintf(text, MAXSTR, "%.2f%%", percent); /* Percentage in level bar */
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

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.5); /* Print percentage */
	cairo_select_font_face(cr, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_move_to(cr, (width / 2) - 20, height - 6);
	cairo_set_font_size(cr, 13);
	cairo_show_text(cr, text);
	cairo_fill(cr);
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

	return NULL;
}
