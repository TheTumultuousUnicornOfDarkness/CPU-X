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
* FILE gui_gtk_id.h
*/

#ifndef _GUI_GTK_ID_H_
#define _GUI_GTK_ID_H_


static GSettings *settings = NULL;

/* Translation */
static const char *trad[LASTOBJ] =
{
	"cpulabel", "cacheslabel", "motherboardlabel", "ramlabel", "systemlabel", "graphicslabel", "benchlabel", "aboutlabel",
	"proc_lab", "clock_lab", "cache_lab",
	"l1cache_lab", "l2cache_lab", "l3cache_lab", "l4cache_lab", "test_lab",
	"motherboard_lab", "bios_lab", "chip_lab",
	"bank0_lab", "bank1_lab", "bank2_lab", "bank3_lab", "bank4_lab", "bank5_lab", "bank6_lab", "bank7_lab",
	"os_lab", "mem_lab",
	"card0_lab", "card1_lab", "card2_lab", "card3_lab",
	"primeslow_lab", "primefast_lab", "param_lab",
	"about_lab", "license_lab"
};

/* Tab CPU */
static const char *objectcpu[LASTCPU] =
{
	"proc_vendor", "proc_cdename", "proc_pkg", "proc_tech", "proc_volt", "proc_spec", "proc_fam", "proc_extfam", "proc_mod", "proc_extmod", "proc_temp", "proc_step", "proc_instr",
	"clock_core", "clock_mult", "clock_bus", "clock_usage",
	"cache_l1d", "cache_l1i", "cache_l2", "cache_l3",
	"trg_sock", "trg_core", "trg_thrd"
};

/* Tab Caches */
static const char *objectcache[LASTCACHES] =
{
	"l1cache_size", "l1cache_speed",
	"l2cache_size", "l2cache_speed",
	"l3cache_size", "l3cache_speed",
	"l4cache_size", "l4cache_speed"
};

/* Tab Motherboard */
static const char *objectmb[LASTMOTHERBOARD] =
{
	"motherboard_manu", "motherboard_mod", "motherboard_rev",
	"bios_brand", "bios_vers", "bios_date", "bios_rom",
	"chip_vend", "chip_name"
};

/* Tab RAM */
static const char *objectram[LASTMEMORY] =
{
	"bank0_ref", "bank1_ref", "bank2_ref", "bank3_ref",
	"bank4_ref", "bank5_ref", "bank6_ref", "bank7_ref"
};

/* Tab System */
static const char *objectsys[LASTSYSTEM] =
{
	"os_kern", "os_distro", "os_host", "os_uptime", "os_comp",
	"mem_used", "mem_buff", "mem_cache", "mem_free", "mem_swap"
};

static const char *objectsys_bar[LASTBAR] =
{
	"mem_barused", "mem_barbuff", "mem_barcache", "mem_barfree", "mem_barswap"
};

/* Tab Graphics */
static const char *objectgpu[LASTGRAPHICS] =
{
	"card0_vend", "card0_mod", "card0_temp", "card0_usage", "card0_gclk", "card0_mclk",
	"card1_vend", "card1_mod", "card1_temp", "card1_usage", "card1_gclk", "card1_mclk",
	"card2_vend", "card2_mod", "card2_temp", "card2_usage", "card2_gclk", "card2_mclk",
	"card3_vend", "card3_mod", "card3_temp", "card3_usage", "card3_gclk", "card3_mclk"
};

/* Tab Bench */
static const char *objectbench[LASTBENCH] =
{
	"primeslow_score", "primeslow_run",
	"primefast_score", "primefast_run",
	"param_duration",  "param_threads"
};

/* Tab About */
static const char *objectabout[LASTABOUT] =
{
	"about_descr",
	"about_version", "about_author", "about_site",
	"license_labcopyright", "license_lablicense", "license_nowarranty"
};

static const char *nicktab[NO_ABOUT + 1] =
{
	"cpu", "caches", "motherboad", "memory", "system", "graphics", "bench", "about"
};


#endif /* _GUI_GTK_ID_H_ */
