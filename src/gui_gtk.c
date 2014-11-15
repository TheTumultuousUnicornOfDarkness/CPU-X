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
#include <limits.h>
#include "cpu-x.h"
#include "includes.h"

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
	"cpulabel", "mainboardlabel", "systemlabel", "aboutlabel",
	"proc_lab", "clock_lab", "cache_lab", "motherboard_lab", "bios_lab", "os_lab", "mem_lab", "about_lab", "license_lab",
	"about_version", "about_descr", "about_author", "license_lablicense"
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


void start_gui_gtk(int *argc, char **argv[], Labels *data)
{
	GtkBuilder *builder;
	GtkLabels glab;
	GThrd refr;

	gtk_init(argc, argv);
	builder = gtk_builder_new();

	/* Build UI from Glade file */
#ifdef EMBED
	if(!gtk_builder_add_from_string(builder, cpux_glade, -1, NULL)) {
		MSGERR("gtk_builder_add_from_string failed when loading embeded UI file.");
		exit(EXIT_FAILURE);
	}
#else
	if(!gtk_builder_add_from_file(builder, get_path("cpux-gtk-3.8.ui"), NULL))
	{
		MSGERR("gtk_builder_add_from_file failed.");
		exit(EXIT_FAILURE);
	}
#endif
	g_set_prgname(PRGNAME);
	glab.mainwindow	 = GTK_WIDGET(gtk_builder_get_object(builder, "mainwindow"));
	glab.closebutton = GTK_WIDGET(gtk_builder_get_object(builder, "closebutton"));
	glab.labprgver	 = GTK_WIDGET(gtk_builder_get_object(builder, "labprgver"));
	set_colors(&glab);
	get_labels(builder, &glab);
	g_object_unref(G_OBJECT(builder));

	set_logos(&glab, data); /* Vendor icon */
	set_labels(&glab, data);
	set_membar(&glab, data);

	g_signal_connect(glab.mainwindow,  "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(glab.closebutton, "clicked", G_CALLBACK(gtk_main_quit), NULL);

	refr.glab = &glab;
	refr.data = data;
	refr.thrdrefr = g_thread_new(NULL, (gpointer)grefresh, &refr);
	gtk_main();
}


gpointer grefresh(GThrd *refr)
{
	while(42)
	{
		cpufreq(refr->data->tabcpu[VALUE][BUSSPEED], refr->data->tabcpu[VALUE][CORESPEED], refr->data->tabcpu[VALUE][MULTIPLIER]);
		tabsystem(refr->data);
		if(HAS_LIBDMI && !getuid())
		{
			libdmidecode(refr->data);
			gtk_label_set_text(GTK_LABEL(refr->glab->gtktabcpu[VALUE][MULTIPLIER]), refr->data->tabcpu[VALUE][MULTIPLIER]);
		}
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabcpu[VALUE][CORESPEED]),  refr->data->tabcpu[VALUE][CORESPEED]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][UPTIME]),	refr->data->tabsys[VALUE][UPTIME]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][USED]), refr->data->tabsys[VALUE][USED]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][BUFFERS]), refr->data->tabsys[VALUE][BUFFERS]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][CACHED]), refr->data->tabsys[VALUE][CACHED]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][FREE]), refr->data->tabsys[VALUE][FREE]);
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabsys[VALUE][SWAP]), refr->data->tabsys[VALUE][SWAP]);
		set_membar(refr->glab, refr->data);
		sleep(refreshtime);
	}

	return NULL;
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

	gtk_window_set_icon_from_file(GTK_WINDOW(glab->mainwindow), get_path("CPU-X.png"), NULL); /* Window icon */
	gtk_image_set_from_file(GTK_IMAGE(glab->logoprg), get_path("CPU-X.png")); /* Program icon in About */

	gtk_image_set_from_file(GTK_IMAGE(glab->logocpu), get_path(tmp)); /* CPU vendor icon */
	gtk_image_get_icon_name(GTK_IMAGE(glab->logocpu), icon_name, NULL);
	if(icon_name[0] != NULL) /* If no icon is set, apply "novendor.png" */
		gtk_image_set_from_file(GTK_IMAGE(glab->logocpu), get_path("novendor.png"));
#endif
}

void get_labels(GtkBuilder *builder, GtkLabels *glab)
{
	int i;

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

	/* Tab System */
	for(i = KERNEL; i < LASTSYS; i++)
	{
		gtk_label_set_text(GTK_LABEL(glab->gtktabsys[NAME][i]), data->tabsys[NAME][i]);
		gtk_label_set_text(GTK_LABEL(glab->gtktabsys[VALUE][i]), data->tabsys[VALUE][i]);
	}
}

/* Set Memory bar in tab System */
void set_membar(GtkLabels *glab, Labels *data)
{
	gtk_level_bar_set_value(GTK_LEVEL_BAR(glab->barused), (double) strtol(data->tabsys[VALUE][USED], NULL, 10)
						/ strtol(strstr(data->tabsys[VALUE][USED], "/ ") + 2, NULL, 10));
	gtk_level_bar_set_value(GTK_LEVEL_BAR(glab->barbuff), (double) strtol(data->tabsys[VALUE][BUFFERS], NULL, 10)
						/ strtol(strstr(data->tabsys[VALUE][BUFFERS], "/ ") + 2, NULL, 10));
	gtk_level_bar_set_value(GTK_LEVEL_BAR(glab->barcache), (double) strtol(data->tabsys[VALUE][CACHED], NULL, 10)
						/ strtol(strstr(data->tabsys[VALUE][CACHED], "/ ") + 2, NULL, 10));
	gtk_level_bar_set_value(GTK_LEVEL_BAR(glab->barfree), (double) strtol(data->tabsys[VALUE][FREE], NULL, 10)
						/ strtol(strstr(data->tabsys[VALUE][FREE], "/ ") + 2, NULL, 10));
	gtk_level_bar_set_value(GTK_LEVEL_BAR(glab->barswap), (double) strtol(data->tabsys[VALUE][SWAP], NULL, 10)
						/ strtol(strstr(data->tabsys[VALUE][SWAP], "/ ") + 2, NULL, 10));
}
