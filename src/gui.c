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
* gui.c
*/

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "cpu-x.h"

#ifdef EMBED
# include "../embed/AMD.png.h"
# include "../embed/Intel.png.h"
# include "../embed/CPU-X.png.h"
#endif


gpointer refresh(Gwid *cpu) {
	Lscpu inforefr;
	Dmi extrainforefr;
	char multsyntrefr[Q];

	ext_lscpu(&inforefr);
	if(!getuid()) {
		ext_dmidecode(&extrainforefr);
		mult(extrainforefr.bus, inforefr.mhz, inforefr.mhzmin, inforefr.mhzmax, multsyntrefr);
		gtk_label_set_text(GTK_LABEL(cpu->clock_vbus), extrainforefr.bus);
	}
	gtk_label_set_text(GTK_LABEL(cpu->clock_vcore), inforefr.mhz);
	gtk_label_set_text(GTK_LABEL(cpu->clock_vmult), multsyntrefr);

	return NULL;
}

gpointer boucle(Gwid *cpu) {
	while(42) {
		refresh(cpu);
		sleep(1);
	}
	return NULL;
}

void empty_labels(Libcpuid *data, Lscpu *info, Dmi *extrainfo) {
	data->vendor[0] = '\0';
	data->name[0] = '\0';
	data->arch[0] = '\0';
	data->spec[0] = '\0';
	data->fam[0] = '\0';
	data->mod[0] = '\0';
	data->step[0] = '\0';
	data->extfam[0] = '\0';
	data->extmod[0] = '\0';
	data->instr[0] = '\0';
	data->l1d[0] = '\0';
	data->l1i[0] = '\0';
	data->l2[0] = '\0';
	data->l3[0] = '\0';
	data->l1dw[0] = '\0';
	data->l1iw[0] = '\0';
	data->l2w[0] = '\0';
	data->l3w[0] = '\0';
	data->soc[0] = '\0';
	data->core[0] = '\0';
	data->thrd[0] = '\0';

	info->arch[0] = '\0';
	info->endian[0] = '\0';
	info->mhz[0] = '\0';
	info->mhzmin[0] = '\0';
	info->mhzmax[0] = '\0';
	info->mips[0] = '\0';
	info->virtu[0] = '\0';

	extrainfo->vendor[0] = '\0';
	extrainfo->socket[0] = '\0';
	extrainfo->bus[0] = '\0';
	extrainfo->manu[0] = '\0';
	extrainfo->model[0] = '\0';
	extrainfo->rev[0] = '\0';
	extrainfo->brand[0] = '\0';
	extrainfo->version[0] = '\0';
	extrainfo->date[0] = '\0';
	extrainfo->rom[0] = '\0';
	
}

void set_colors(Gwid *cpu) {
	GdkRGBA window_colors, notebook_colors;

	window_colors.red	= 0.3;
	window_colors.green	= 0.6;
	window_colors.blue	= 0.9;
	window_colors.alpha	= 0.95;
	gtk_widget_override_background_color(cpu->window, GTK_STATE_FLAG_NORMAL, &window_colors);

	notebook_colors.red	= 0.95;
	notebook_colors.green	= 0.98;
	notebook_colors.blue	= 0.98;
	notebook_colors.alpha	= 1.0;
	gtk_widget_override_background_color(cpu->notebook1, GTK_STATE_FLAG_NORMAL, &notebook_colors);
}

void set_vendorlogo(Gwid *cpu, Libcpuid *data) {
#ifdef EMBED
	GdkPixbuf *pixbuf_AMD	= gdk_pixbuf_new_from_inline (-1, AMD, FALSE, NULL);
	GdkPixbuf *pixbuf_Intel = gdk_pixbuf_new_from_inline (-1, Intel, FALSE, NULL);
	GdkPixbuf *pixbuf_CPU_X = gdk_pixbuf_new_from_inline (-1, CPU_X, FALSE, NULL);

	gtk_window_set_icon(GTK_WINDOW(cpu->window), pixbuf_CPU_X);
	if(!strcmp(data->vendor, "GenuineIntel"))
		gtk_image_set_from_pixbuf(GTK_IMAGE(cpu->proc_logo), pixbuf_Intel);
	else if(!strcmp(data->vendor, "AuthenticAMD"))
		gtk_image_set_from_pixbuf(GTK_IMAGE(cpu->proc_logo), pixbuf_AMD);
	else
		gtk_image_set_from_pixbuf(GTK_IMAGE(cpu->proc_logo), pixbuf_CPU_X);
#else
	gtk_window_set_icon_from_file(GTK_WINDOW(cpu->window), DATADIR("CPU-X.png"), NULL);
	if(!strcmp(data->vendor, "GenuineIntel"))
		gtk_image_set_from_file(GTK_IMAGE(cpu->proc_logo), DATADIR("Intel.png"));
	else if(!strcmp(data->vendor, "AuthenticAMD"))
		gtk_image_set_from_file(GTK_IMAGE(cpu->proc_logo), DATADIR("AMD.png"));
	else
		gtk_image_set_from_file(GTK_IMAGE(cpu->proc_logo), DATADIR("CPU-X.png"));
#endif
}

void build_tab_cpu(GtkBuilder *builder, Gwid *cpu){
	cpu->proc_logo		= GTK_WIDGET(gtk_builder_get_object(builder, "logo"));
	cpu->proc_vvendor	= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vvendor"));
	cpu->proc_vname		= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vname"));
	cpu->proc_vpkg		= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vpkg"));
	cpu->proc_varch		= GTK_WIDGET(gtk_builder_get_object(builder, "proc_varch"));
	cpu->proc_vspec		= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vspec"));
	cpu->proc_vfam		= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vfam"));
	cpu->proc_vmod		= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vmod"));
	cpu->proc_vextfam	= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vextfam"));
	cpu->proc_vextmod	= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vextmod"));
	cpu->proc_vstep		= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vstep"));
	cpu->proc_vinstr	= GTK_WIDGET(gtk_builder_get_object(builder, "proc_vinstr"));
	cpu->clock_vcore	= GTK_WIDGET(gtk_builder_get_object(builder, "clock_vcore"));
	cpu->clock_vmult	= GTK_WIDGET(gtk_builder_get_object(builder, "clock_vmult"));
	cpu->clock_vbus		= GTK_WIDGET(gtk_builder_get_object(builder, "clock_vbus"));
	cpu->clock_vmips	= GTK_WIDGET(gtk_builder_get_object(builder, "clock_vmips"));
	cpu->cache_vl1d		= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl1d"));
	cpu->cache_vl1i		= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl1i"));
	cpu->cache_vl2		= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl2"));
	cpu->cache_vl3		= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl3"));
	cpu->cache_vl1dway	= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl1dway"));
	cpu->cache_vl1iway	= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl1iway"));
	cpu->cache_vl2way	= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl2way"));
	cpu->cache_vl3way	= GTK_WIDGET(gtk_builder_get_object(builder, "cache_vl3way"));
	cpu->trg_vsoc		= GTK_WIDGET(gtk_builder_get_object(builder, "trg_vsoc"));
	cpu->trg_vcore		= GTK_WIDGET(gtk_builder_get_object(builder, "trg_vcore"));
	cpu->trg_vthrd		= GTK_WIDGET(gtk_builder_get_object(builder, "trg_vthrd"));
	cpu->mb_vmanu		= GTK_WIDGET(gtk_builder_get_object(builder, "mb_vmanu"));
	cpu->mb_vmodel		= GTK_WIDGET(gtk_builder_get_object(builder, "mb_vmodel"));
	cpu->mb_vrev		= GTK_WIDGET(gtk_builder_get_object(builder, "mb_vrev"));
	cpu->bios_vbrand	= GTK_WIDGET(gtk_builder_get_object(builder, "bios_vbrand"));
	cpu->bios_vversion	= GTK_WIDGET(gtk_builder_get_object(builder, "bios_vversion"));
	cpu->bios_vdate		= GTK_WIDGET(gtk_builder_get_object(builder, "bios_vdate"));
	cpu->bios_vroms		= GTK_WIDGET(gtk_builder_get_object(builder, "bios_vroms"));
}

void set_labels(Gwid *cpu, Libcpuid *data, Lscpu *info, Dmi *extrainfo) {
	char clock_multsynt[Q], proc_instr[S];

	mult(extrainfo->bus, info->mhz, info->mhzmin, info->mhzmax, clock_multsynt);
	instructions(info, data, proc_instr);

	gtk_label_set_text(GTK_LABEL(cpu->lprgver),	 "Version " PRGVER);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vvendor), data->vendor);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vname),	 data->name);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vpkg),	 extrainfo->socket);
	gtk_label_set_text(GTK_LABEL(cpu->proc_varch),	 info->arch);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vspec),	 data->spec);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vfam),	 data->fam);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vmod),	 data->mod);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vextfam), data->extfam);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vextmod), data->extmod);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vstep),	 data->step);
	gtk_label_set_text(GTK_LABEL(cpu->proc_vinstr),	 proc_instr);
	gtk_label_set_text(GTK_LABEL(cpu->clock_vcore),	 info->mhz);
	gtk_label_set_text(GTK_LABEL(cpu->clock_vmult),	 clock_multsynt);
	gtk_label_set_text(GTK_LABEL(cpu->clock_vbus),	 extrainfo->bus);
	gtk_label_set_text(GTK_LABEL(cpu->clock_vmips),	 info->mips);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl1d),	 data->l1d);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl1i),	 data->l1i);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl2),	 data->l2);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl3),	 data->l3);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl1dway), data->l1dw);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl1iway), data->l1iw);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl2way), data->l2w);
	gtk_label_set_text(GTK_LABEL(cpu->cache_vl3way), data->l3w);
	gtk_label_set_text(GTK_LABEL(cpu->trg_vsoc),	 data->soc);
	gtk_label_set_text(GTK_LABEL(cpu->trg_vcore),	 data->core);
	gtk_label_set_text(GTK_LABEL(cpu->trg_vthrd),	 data->thrd);
	gtk_label_set_text(GTK_LABEL(cpu->mb_vmanu),	 extrainfo->manu);
	gtk_label_set_text(GTK_LABEL(cpu->mb_vmodel),	 extrainfo->model);
	gtk_label_set_text(GTK_LABEL(cpu->mb_vrev),	 extrainfo->rev);
	gtk_label_set_text(GTK_LABEL(cpu->bios_vbrand),	 extrainfo->brand);
	gtk_label_set_text(GTK_LABEL(cpu->bios_vversion), extrainfo->version);
	gtk_label_set_text(GTK_LABEL(cpu->bios_vdate),	extrainfo->date);
	gtk_label_set_text(GTK_LABEL(cpu->bios_vroms),	extrainfo->rom);
}
