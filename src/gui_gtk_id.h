/****************************************************************************
*    Copyright © 2014-2015 Xorg
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


/* Translation */
static const char *trad[LASTOBJ] =
{
	"cpulabel", "cacheslabel", "motherboardlabel", "ramlabel", "systemlabel", "graphicslabel", "aboutlabel",
	"proc_lab", "clock_lab", "cache_lab", "l1cache_lab", "l2cache_lab", "l3cache_lab", "motherboard_lab", "bios_lab", "chip_lab", "banks_lab", "os_lab", "mem_lab",
	"card0_lab", "card1_lab", "card2_lab", "card3_lab", "about_lab", "license_lab",
	"about_version", "about_descr", "about_author", "license_labcopyright", "license_lablicense"
};

/* Tab CPU */
static const char *objectcpu[2][LASTCPU] =
{
	{ "proc_labvendor", "proc_labcdename", "proc_labpkg", "proc_labtech", "proc_labvolt", "proc_labspec", "proc_labfam", "proc_labextfam", "proc_labmod", "proc_labextmod", "proc_labstep", "proc_labinstr",
		"clock_labcore", "clock_labmult", "clock_labbus", "clock_labmips",
		"cache_labl1d", "cache_labl1i", "cache_labl2", "cache_labl3",
		"trg_labsock", "trg_labcore", "trg_labthrd"
	},
	{ "proc_valvendor", "proc_valcdename", "proc_valpkg", "proc_valtech", "proc_valvolt", "proc_valspec", "proc_valfam", "proc_valextfam", "proc_valmod", "proc_valextmod", "proc_valstep", "proc_valinstr",
		"clock_valcore", "clock_valmult", "clock_valbus", "clock_valmips",
		"cache_vall1d", "cache_vall1i", "cache_vall2", "cache_vall3",
		"trg_valsock", "trg_valcore", "trg_valthrd"
	}
};

/* Tab Caches */
static const char *objectcache[2][LASTCACHE] =
{
	{ "l1cache_labsize", "l1cache_labdescr", "l1cache_labspeed",
		"l2cache_labsize", "l2cache_labdescr", "l2cache_labspeed",
		"l3cache_labsize", "l3cache_labdescr", "l3cache_labspeed"
	},
	{ "l1cache_valsize", "l1cache_valdescr", "l1cache_valspeed",
		"l2cache_valsize", "l2cache_valdescr", "l2cache_valspeed",
		"l3cache_valsize", "l3cache_valdescr", "l3cache_valspeed"
	}
};

/* Tab Motherboard */
static const char *objectmb[2][LASTMB] =
{
	{ "motherboard_labmanu", "motherboard_labmod", "motherboard_labrev",
		"bios_labbrand", "bios_labvers", "bios_labdate", "bios_labrom",
		"chip_labvend", "chip_labname"
	},
	{ "motherboard_valmanu", "motherboard_valmod", "motherboard_valrev",
		"bios_valbrand", "bios_valvers", "bios_valdate", "bios_valrom",
		"chip_valvend", "chip_valname"
	}
};

/* Tab RAM */
static const char *objectram[2][LASTRAM] =
{
	{ "banks_labbank0_0", "banks_labbank0_1", "banks_labbank1_0", "banks_labbank1_1", "banks_labbank2_0", "banks_labbank2_1", "banks_labbank3_0", "banks_labbank3_1",
		"banks_labbank4_0", "banks_labbank4_1", "banks_labbank5_0", "banks_labbank5_1", "banks_labbank6_0", "banks_labbank6_1", "banks_labbank7_0", "banks_labbank7_1"
	},
	{ "banks_valbank0_0", "banks_valbank0_1", "banks_valbank1_0", "banks_valbank1_1", "banks_valbank2_0", "banks_valbank2_1", "banks_valbank3_0", "banks_valbank3_1",
		"banks_valbank4_0", "banks_valbank4_1", "banks_valbank5_0", "banks_valbank5_1", "banks_valbank6_0", "banks_valbank6_1", "banks_valbank7_0", "banks_valbank7_1"
	}
};

/* Tab System */
static const char *objectsys[2][LASTSYS] =
{
	{ "os_labkern", "os_labdistro", "os_labhost", "os_labuptime", "os_labcomp",
		"mem_labused", "mem_labbuff", "mem_labcache", "mem_labfree", "mem_labswap"
	},
	{ "os_valkern", "os_valdistro", "os_valhost", "os_valuptime", "os_valcomp",
		"mem_valused", "mem_valbuff", "mem_valcache", "mem_valfree", "mem_valswap"
	}
};

/* Tab Graphics */
static const char *objectgpu[2][LASTGPU] =
{
	{ "card0_labvend", "card0_labmod", "card0_labdrv",
		"card1_labvend", "card1_labmod", "card1_labdrv",
		"card2_labvend", "card2_labmod", "card2_labdrv",
		"card3_labvend", "card3_labmod", "card3_labdrv"
	},
	{ "card0_valvend", "card0_valmod", "card0_valdrv",
		"card1_valvend", "card1_valmod", "card1_valdrv",
		"card2_valvend", "card2_valmod", "card2_valdrv",
		"card3_valvend", "card3_valmod", "card3_valdrv"
	}
};


#endif /* _GUI_GTK_ID_H_ */
