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
* gui_gtk.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "cpu-x.h"
#include "gui_gtk.h"

#ifdef EMBED
# include "../embed/CPU-X.png.h"
# include "../embed/NoVendor.png.h"
# include "../embed/AMD.png.h"
# include "../embed/Centaur.png.h"
# include "../embed/Cyrix.png.h"
# include "../embed/Intel.png.h"
# include "../embed/NexGen.png.h"
# include "../embed/NSC.png.h"
# include "../embed/Rise.png.h"
# include "../embed/SiS.png.h"
# include "../embed/Transmeta.png.h"
# include "../embed/UMC.png.h"
# include "../embed/cpu-x.ui.h"
#endif


/* Objects' ID for traduction */
static const char *trad[LASTOBJ] =
{
	"cpulabel", "mainboardlabel", "ramlabel", "systemlabel", "aboutlabel",
	"proc_lab", "clock_lab", "cache_lab", "motherboard_lab", "bios_lab", "banks_lab", "os_lab", "mem_lab", "about_lab", "license_lab",
	"about_version", "about_descr", "about_author", "license_labcopyright", "license_lablicense"
};

/* Objects' ID in tab CPU */
static const char *objectcpu[2][LASTCPU] =
{
	{ "proc_labvendor", "proc_labcdename", "proc_labpkg", "proc_labarch", "proc_labspec", "proc_labfam", "proc_labextfam", "proc_labmod", "proc_labextmod", "proc_labstep", "proc_labinstr",
		"clock_labcore", "clock_labmult", "clock_labbus", "clock_labmips",
		"cache_labl1d", "cache_labl1i", "cache_labl2", "cache_labl3",
		"trg_labsock", "trg_labcore", "trg_labthrd"
	},
	{ "proc_valvendor", "proc_valcdename", "proc_valpkg", "proc_valarch", "proc_valspec", "proc_valfam", "proc_valextfam", "proc_valmod", "proc_valextmod", "proc_valstep", "proc_valinstr",
		"clock_valcore", "clock_valmult", "clock_valbus", "clock_valmips",
		"cache_vall1d", "cache_vall1i", "cache_vall2", "cache_vall3",
		"trg_valsock", "trg_valcore", "trg_valthrd"
	}
};

/* Objects' ID in tab Mainboard */
static const char *objectmb[2][LASTMB] =
{
	{ "motherboard_labmanu", "motherboard_labmod", "motherboard_labrev",
		"bios_labbrand", "bios_labvers", "bios_labdate", "bios_labrom"
	},
	{ "motherboard_valmanu", "motherboard_valmod", "motherboard_valrev",
		"bios_valbrand", "bios_valvers", "bios_valdate", "bios_valrom"
	}
};

/* Objects' ID in tab RAM */
static const char *objectram[2][LASTRAM] =
{
	{ "banks_labbank0_0", "banks_labbank0_1", "banks_labbank1_0", "banks_labbank1_1", "banks_labbank2_0", "banks_labbank2_1", "banks_labbank3_0", "banks_labbank3_1",
		"banks_labbank4_0", "banks_labbank4_1", "banks_labbank5_0", "banks_labbank5_1", "banks_labbank6_0", "banks_labbank6_1", "banks_labbank7_0", "banks_labbank7_1"
	},
	{ "banks_valbank0_0", "banks_valbank0_1", "banks_valbank1_0", "banks_valbank1_1", "banks_valbank2_0", "banks_valbank2_1", "banks_valbank3_0", "banks_valbank3_1",
		"banks_valbank4_0", "banks_valbank4_1", "banks_valbank5_0", "banks_valbank5_1", "banks_valbank6_0", "banks_valbank6_1", "banks_valbank7_0", "banks_valbank7_1"
	}
};

/* Objects' ID in tab System */
static const char *objectsys[2][LASTSYS] =
{
	{ "os_labkern", "os_labdistro", "os_labhost", "os_labuptime", "os_labcomp",
		"mem_labused", "mem_labbuff", "mem_labcache", "mem_labfree", "mem_labswap",
	},
	{ "os_valkern", "os_valdistro", "os_valhost", "os_valuptime", "os_valcomp",
		"mem_valused", "mem_valbuff", "mem_valcache", "mem_valfree", "mem_valswap"
	}
};


void warning_window(GtkWidget *mainwindow)
{
	char markup[MAXSTR*2];

	sprintf(markup, MSGROOT);

	GtkWidget *dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(mainwindow),
		GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_NONE,
		"\n\t\t\t<span font_weight='heavy' font_size='x-large'>%s</span>\n\n%s",
		strtok(markup, ":"),
		strstr(MSGROOT, "\n"));

	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	gtk_dialog_add_buttons(GTK_DIALOG (dialog), _("Run as root"), GTK_RESPONSE_ACCEPT, _("Ignore"), GTK_RESPONSE_REJECT, NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), PRGNAME);

	if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		system("cpu-x_polkit &");
		exit(EXIT_SUCCESS);
	}

	gtk_widget_destroy(dialog);
}

void start_gui_gtk(int *argc, char **argv[], Labels *data)
{
	GtkBuilder *builder;
	GtkLabels glab;
	GThrd refr;

	MSGVERB(_("Starting GTK GUI..."));
	gtk_init(argc, argv);
	builder = gtk_builder_new();
	refr.glab = &glab;
	refr.data = data;

	/* Build UI from Glade file */
#ifdef EMBED
	if(!gtk_builder_add_from_string(builder, cpux_glade, -1, NULL))
	{
		MSGPERR(_("gtk_builder_add_from_string failed"));
		exit(EXIT_FAILURE);
	}
#else
	if(!gtk_builder_add_from_file(builder, data_path("cpux-gtk-3.8.ui"), NULL))
	{
		MSGPERR(_("gtk_builder_add_from_file failed"));
		exit(EXIT_FAILURE);
	}
#endif
	g_set_prgname(g_ascii_strdown(PRGNAME, -1));
	glab.mainwindow	 = GTK_WIDGET(gtk_builder_get_object(builder, "mainwindow"));
	glab.closebutton = GTK_WIDGET(gtk_builder_get_object(builder, "closebutton"));
	glab.labprgver	 = GTK_WIDGET(gtk_builder_get_object(builder, "labprgver"));
	set_colors(&glab);
	get_labels(builder, &glab);
	g_object_unref(G_OBJECT(builder));

	set_logos(&glab, data); /* Vendor icon */
	set_labels(&glab, data);

	if(getuid()) /* Show warning if not root */
		warning_window(glab.mainwindow);

	g_signal_connect(glab.mainwindow,  "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(glab.closebutton, "clicked", G_CALLBACK(gtk_main_quit), NULL);

#if HAS_LIBPROCPS || HAS_LIBSTATGRAB
	g_signal_connect(G_OBJECT(glab.barused), "draw", G_CALLBACK(setbar_used), data); /* Level bars */
	g_signal_connect(G_OBJECT(glab.barbuff), "draw", G_CALLBACK(setbar_buff), data);
	g_signal_connect(G_OBJECT(glab.barcache), "draw", G_CALLBACK(setbar_cache), data);
	g_signal_connect(G_OBJECT(glab.barfree), "draw", G_CALLBACK(setbar_free), data);
	g_signal_connect(G_OBJECT(glab.barswap), "draw", G_CALLBACK(setbar_swap), data);
#endif /* HAS_LIBPROCPS || HAS_LIBSTATGRAB */

	g_timeout_add_seconds(refreshtime, (gpointer)grefresh, &refr);
	gtk_main();
}

gboolean grefresh(GThrd *refr)
{
	int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(refr->glab->notebook));

	/* Refresh tab CPU */
	if(page == NB_TAB_CPU)
	{
		cpufreq(refr->data->tabcpu[VALUE][BUSSPEED], refr->data->tabcpu[VALUE][CORESPEED], refr->data->tabcpu[VALUE][MULTIPLIER]);
		if(HAS_LIBDMI && !getuid())
		{
			libdmidecode(refr->data);
			gtk_label_set_text(GTK_LABEL(refr->glab->gtktabcpu[VALUE][MULTIPLIER]), refr->data->tabcpu[VALUE][MULTIPLIER]);
		}
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabcpu[VALUE][CORESPEED]),  refr->data->tabcpu[VALUE][CORESPEED]);
	}

	/* Refresh tab System */
	else if(page == NB_TAB_SYS)
	{
		tabsystem(refr->data);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][UPTIME]),	refr->data->tabsys[VALUE][UPTIME]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][USED]), refr->data->tabsys[VALUE][USED]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][BUFFERS]), refr->data->tabsys[VALUE][BUFFERS]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][CACHED]), refr->data->tabsys[VALUE][CACHED]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][FREE]), refr->data->tabsys[VALUE][FREE]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][SWAP]), refr->data->tabsys[VALUE][SWAP]);
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
}

void set_logos(GtkLabels *glab, Labels *data)
{
#ifdef EMBED
	GdkPixbuf *pixbuf_CPU_X = gdk_pixbuf_new_from_inline	(-1, CPU_X, FALSE, NULL);
	GdkPixbuf *pixbuf_NOVENDOR = gdk_pixbuf_new_from_inline (-1, NOVENDOR, FALSE, NULL);
	GdkPixbuf *pixbuf_vendor = NULL;

	gtk_window_set_icon(GTK_WINDOW(glab->mainwindow), pixbuf_CPU_X); /* Window icon */
	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logoprg), pixbuf_CPU_X); /* Program icon in About */

	if(!strcmp(data->tabcpu[VALUE][VENDOR], "Intel"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, Intel, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "AMD"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, AMD, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "Cyrix"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, Cyrix, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "NexGen"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, NexGen, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "Transmeta"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, Transmeta, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "UMC"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, UMC, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "Centaur"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, Centaur, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "Rise"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, Rise, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "SiS"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, SiS, FALSE, NULL);
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "National Semiconductor"))
		pixbuf_vendor = gdk_pixbuf_new_from_inline (-1, National_Semiconductor, FALSE, NULL);

	gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), pixbuf_vendor); /* CPU vendor icon */
	if(gtk_image_get_pixbuf(GTK_IMAGE(glab->logocpu)) == NULL) /* If no icon is set, apply "novendor.png" */
		gtk_image_set_from_pixbuf(GTK_IMAGE(glab->logocpu), pixbuf_NOVENDOR);

#else
	char tmp[MAXSTR];
	const gchar *icon_name[MAXSTR];
	sprintf(tmp, "%s.png", data->tabcpu[VALUE][VENDOR]);

	gtk_window_set_icon_from_file(GTK_WINDOW(glab->mainwindow), data_path("CPU-X.png"), NULL); /* Window icon */
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

	/* Various labels to translate */
	for(i = TABCPU; i < LASTOBJ; i++)
		glab->gtktrad[i] = GTK_WIDGET(gtk_builder_get_object(builder, trad[i]));

	/* Tab CPU */
	for(i = VENDOR; i < LASTCPU; i++)
	{
		glab->gtktabcpu[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectcpu[NAME][i]));
		glab->gtktabcpu[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectcpu[VALUE][i]));
	}

	/* Tab Mainboard */
	for(i = MANUFACTURER; i < LASTMB; i++)
	{
		glab->gtktabmb[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectmb[NAME][i]));
		glab->gtktabmb[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectmb[VALUE][i]));
	}

	/* Tab RAM */
	for(i = BANK0_0; i < LASTRAM; i++)
	{
		glab->gtktabram[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectram[NAME][i]));
		glab->gtktabram[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectram[VALUE][i]));
	}
	glab->gridbanks = GTK_WIDGET(gtk_builder_get_object(builder, "banks_grid"));

	/* Tab System */
	for(i = KERNEL; i < LASTSYS; i++)
	{
		glab->gtktabsys[NAME][i]  = GTK_WIDGET(gtk_builder_get_object(builder, objectsys[NAME][i]));
		glab->gtktabsys[VALUE][i] = GTK_WIDGET(gtk_builder_get_object(builder, objectsys[VALUE][i]));
	}
	glab->barused  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barused"));
	glab->barbuff  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barbuff"));
	glab->barcache = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barcache"));
	glab->barfree  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barfree"));
	glab->barswap  = GTK_WIDGET(gtk_builder_get_object(builder, "mem_barswap"));
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
		gtk_label_set_text(GTK_LABEL(glab->gtktabcpu[NAME][i]), data->tabcpu[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktabcpu[VALUE][i]), data->tabcpu[VALUE][i]);
	}

	/* Tab Mainboard */
	for(i = MANUFACTURER; i < LASTMB; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktabmb[NAME][i]), data->tabmb[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktabmb[VALUE][i]), data->tabmb[VALUE][i]);
	}

	/* Tab RAM */
	for(i = BANK0_0; i < last_bank(data); i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktabram[NAME][i]), data->tabram[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktabram[VALUE][i]), data->tabram[VALUE][i]);
	}
	for(i = BANK7_1; i >= last_bank(data); i--)
		gtk_grid_remove_row(GTK_GRID(glab->gridbanks), i);

	/* Tab System */
	for(i = KERNEL; i < LASTSYS; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktabsys[NAME][i]), data->tabsys[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktabsys[VALUE][i]), data->tabsys[VALUE][i]);
	}
}

#if HAS_LIBPROCPS || HAS_LIBSTATGRAB
void fill_frame(GtkWidget *widget, cairo_t *cr, double before, double val)
{
	guint width, height;
	char text[MAXSTR];

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	snprintf(text, MAXSTR, "%.2f%%", val);

	cairo_rectangle(cr, before / 100 * width, 0, val / 100 * width, 14);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.5);
	cairo_move_to(cr, (width / 2) - 20, height - 2);
	cairo_set_font_size(cr, 14);
	cairo_show_text(cr, text);
	cairo_fill(cr);
}

void setbar_used(GtkWidget *widget, cairo_t *cr, Labels *data)
{
	double percent;

	percent = (double) strtol(data->tabsys[VALUE][USED], NULL, 10) /
		strtol(strstr(data->tabsys[VALUE][USED], "/ ") + 2, NULL, 10) * 100;

	cairo_set_source_rgb(cr, 255.0 / 255.0, 215.0 / 255.0, 40.0 / 255.0);
	fill_frame(widget, cr, 0, percent);
}

void setbar_buff(GtkWidget *widget, cairo_t *cr, Labels *data)
{
	double before, percent;

	before = (double) strtol(data->tabsys[VALUE][USED], NULL, 10) /
		strtol(strstr(data->tabsys[VALUE][USED], "/ ") + 2, NULL, 10) * 100;
	percent = (double) strtol(data->tabsys[VALUE][BUFFERS], NULL, 10) /
		strtol(strstr(data->tabsys[VALUE][BUFFERS], "/ ") + 2, NULL, 10) * 100;

	cairo_set_source_rgb(cr, 65.0 / 255.0, 155.0 / 255.0, 240.0 / 255.0);
	fill_frame(widget, cr, before, percent);
}

void setbar_cache(GtkWidget *widget, cairo_t *cr, Labels *data)
{
	double before, percent;

	before = (double) ( strtol(data->tabsys[VALUE][USED], NULL, 10) +
		strtol(data->tabsys[VALUE][BUFFERS], NULL, 10) ) /
		strtol(strstr(data->tabsys[VALUE][USED], "/ ") + 2, NULL, 10) * 100;

	percent = (double) strtol(data->tabsys[VALUE][CACHED], NULL, 10) /
		strtol(strstr(data->tabsys[VALUE][CACHED], "/ ") + 2, NULL, 10) * 100;

	cairo_set_source_rgb(cr, 250.0 / 255.0, 90.0 / 255.0, 35.0 / 255.0);
	fill_frame(widget, cr, before, percent);
}

void setbar_free(GtkWidget *widget, cairo_t *cr, Labels *data)
{
	double before, percent;

	before = (double) ( strtol(data->tabsys[VALUE][USED], NULL, 10) +
		strtol(data->tabsys[VALUE][BUFFERS], NULL, 10) +
		strtol(data->tabsys[VALUE][CACHED], NULL, 10) ) /
		strtol(strstr(data->tabsys[VALUE][USED], "/ ") + 2, NULL, 10) * 100;
	percent = (double) strtol(data->tabsys[VALUE][FREE], NULL, 10) /
		strtol(strstr(data->tabsys[VALUE][FREE], "/ ") + 2, NULL, 10) * 100;

	cairo_set_source_rgb(cr, 48.0 / 255.0, 225.0 / 255.0, 58.0 / 255.0);
	fill_frame(widget, cr, before, percent);
}

void setbar_swap(GtkWidget *widget, cairo_t *cr, Labels *data)
{
	double percent;

	percent = (double) strtol(data->tabsys[VALUE][SWAP], NULL, 10) /
		strtol(strstr(data->tabsys[VALUE][SWAP], "/ ") + 2, NULL, 10) * 100;

	cairo_set_source_rgb(cr, 250.0 / 255.0, 60.0 / 255.0, 225.0 / 255.0);
	fill_frame(widget, cr, 0, percent);
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
