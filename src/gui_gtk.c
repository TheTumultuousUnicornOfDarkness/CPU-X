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
# include "../embed/AMD.png.h"
# include "../embed/Intel.png.h"
# include "../embed/CPU-X.png.h"
# include "../embed/cpu-x.ui.h"
#endif


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
	glab.aboutprgver = GTK_WIDGET(gtk_builder_get_object(builder, "about_version"));
	get_labels(builder, &glab);
	g_object_unref(G_OBJECT(builder));

	gtk_window_set_icon_from_file(GTK_WINDOW(glab.mainwindow), get_path("CPU-X.png"), NULL); /* Window icon */
	gtk_image_set_from_file(GTK_IMAGE(glab.logoprg), get_path("CPU-X.png")); /* Program icon in About */
	set_vendorlogo(&glab, data); /* Vendor icon */
	set_labels(&glab, data);

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
		if(HAS_LIBDMI && !getuid())
		{
			libdmidecode(refr->data);
			gtk_label_set_text(GTK_LABEL(refr->glab->gtktabcpu[VALUE][MULTIPLIER]), refr->data->tabcpu[VALUE][MULTIPLIER]);
		}
		gtk_label_set_text(GTK_LABEL(refr->glab->gtktabcpu[VALUE][CORESPEED]),  refr->data->tabcpu[VALUE][CORESPEED]);
		sleep(refreshtime);
	}

	return NULL;
}

void set_vendorlogo(GtkLabels *glab, Labels *data)
{
#ifdef EMBED
	GdkPixbuf *pixbuf_AMD	= gdk_pixbuf_new_from_inline (-1, AMD, FALSE, NULL);
	GdkPixbuf *pixbuf_Intel = gdk_pixbuf_new_from_inline (-1, Intel, FALSE, NULL);
	GdkPixbuf *pixbuf_CPU_X = gdk_pixbuf_new_from_inline (-1, CPU_X, FALSE, NULL);

	gtk_window_set_icon(GTK_WINDOW(glab->window), pixbuf_CPU_X);
	if(!strcmp(data->vendor, "Intel"))
		gtk_image_set_from_pixbuf(GTK_IMAGE(glab->proc_logo), pixbuf_Intel);
	else if(!strcmp(data->vendor, "AMD"))
		gtk_image_set_from_pixbuf(GTK_IMAGE(glab->proc_logo), pixbuf_AMD);
	else
		gtk_image_set_from_pixbuf(GTK_IMAGE(glab->proc_logo), pixbuf_CPU_X);
#else

	if(!strcmp(data->tabcpu[VALUE][VENDOR], "Intel"))
		gtk_image_set_from_file(GTK_IMAGE(glab->logocpu), get_path("Intel.png"));
	else if(!strcmp(data->tabcpu[VALUE][VENDOR], "AMD"))
		gtk_image_set_from_file(GTK_IMAGE(glab->logocpu), get_path("AMD.png"));
	else
		gtk_image_set_from_file(GTK_IMAGE(glab->logocpu), get_path("CPU-X.png"));
#endif
}

void get_labels(GtkBuilder *builder, GtkLabels *glab)
{
	int i;

	glab->logocpu = GTK_WIDGET(gtk_builder_get_object(builder, "proc_logocpu"));
	glab->logoprg = GTK_WIDGET(gtk_builder_get_object(builder, "about_logoprg"));

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
}

void set_labels(GtkLabels *glab, Labels *data)
{
	int i;

	gtk_label_set_text(GTK_LABEL(glab->labprgver),   "Version " PRGVER);
	gtk_label_set_text(GTK_LABEL(glab->aboutprgver), "Version " PRGVER);

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
}
