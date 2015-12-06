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
* FILE tui_ncurses.c
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <libintl.h>
#include "cpu-x.h"
#include "tui_ncurses.h"

static int loop = NB_TAB_CPU;


void start_tui_ncurses(Labels *data)
{
	int startx, starty, width, height, ch = 0, current_tab = 0;
	WINDOW *tab;
	NThrd refr;

	MSG_VERBOSE(_("Starting NCurses TUI..."));
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	halfdelay(0);
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);

	height = 25;
	width = 68;
	starty = (LINES - height) / 2; /* Calculating for a center placement of the window */
	startx = (COLS - width) / 2;

	printw("Press 'q' to exit; use right/left key to change tab");
	refresh();
	main_win(height, width, starty, startx, current_tab, data);
	tab = tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data);

	refr.win = tab;
	refr.data = data;
	timeout(opts->refr_time * 1000);

	while(ch != 'q')
	{
		ch = getch();
		switch(ch)
		{	case KEY_LEFT:
				/* Switch to left tab */
				if(current_tab > NB_TAB_CPU)
				{
					current_tab--;
					main_win(height, width, starty, startx, current_tab, data);
					tab = select_tab(height, width, starty, startx, current_tab, data);
				}
				break;
			case KEY_RIGHT:
				/* Switch to right tab */
				if(current_tab < NB_TAB_ABOUT)
				{
					current_tab++;
					main_win(height, width, starty, startx, current_tab, data);
					tab = select_tab(height, width, starty, startx, current_tab, data);
				}
				break;
			case ERR:
				/* Refresh labels if needed */
				if(current_tab == NB_TAB_CPU || current_tab == NB_TAB_CACHE || current_tab == NB_TAB_SYS || current_tab == NB_TAB_GPU)
				{
					refr.win = tab;
					nrefresh(&refr);
				}
				break;
			case KEY_RESIZE:
				erase() ;
				starty = (LINES - height) / 2;
				startx = (COLS - width) / 2;
				printw("Press 'q' to exit; use right/left key to change tab");
				refresh();
				main_win(height, width, starty, startx, current_tab, data);
				tab = tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data);
				break;
		}

		loop = current_tab;
	}

	endwin();
	labels_free(data);
}

void nrefresh(NThrd *refr)
{
#if 0
	int i, j = 2;

	/* Refresh tab CPU */
	if(loop == NB_TAB_CPU)
	{
		cpufreq(refr->data);
		if(HAS_LIBCPUID && !getuid())
		{
			libcpuid(refr->data);
			mvwprintw(refr->win, 5, 22, "%13s: %s", refr->data->tabcpu[NAME][VOLTAGE], refr->data->tabcpu[VALUE][VOLTAGE]);
			mvwprintw(refr->win, 7, 38, "%9s: %s", refr->data->tabcpu[NAME][TEMPERATURE], refr->data->tabcpu[VALUE][TEMPERATURE]);
		}
		if(HAS_DMIDECODE && !getuid())
		{
			//libdmidecode(refr->data);
			mvwprintw(refr->win, 13, 2, "%13s: %s", refr->data->tabcpu[NAME][MULTIPLIER], refr->data->tabcpu[VALUE][MULTIPLIER]);
		}
		mvwprintw(refr->win, 12, 2, "%13s: %s", refr->data->tabcpu[NAME][CORESPEED], refr->data->tabcpu[VALUE][CORESPEED]);
		wrefresh(refr->win);
	}

	/* Refresh tab Caches */
	else if(HAS_LIBCPUID && HAS_BANDWIDTH && loop == NB_TAB_CACHE)
	{
		bandwidth(refr->data);
		for(i = L1SPEED; i < LASTCACHE; i += CACHEFIELDS)
		{
			mvwprintw(refr->win, i + j,  2, "%13s: %s", refr->data->tabcache[NAME][i], refr->data->tabcache[VALUE][i]);
			j += 2;
		}
		wrefresh(refr->win);
	}

	/* Refresh tab System */
	else if(loop == NB_TAB_SYS)
	{
		tabsystem(refr->data);
		for(i = USED; i < LASTSYS; i++)
		{
			mvwprintw(refr->win, i + 4,  2, "%13s: %s", refr->data->tabsys[NAME][i], refr->data->tabsys[VALUE][i]);
			clear_bar(refr->win, i);
			draw_bar(refr->win, refr->data, i);
		}
		wrefresh(refr->win);
	}

	/* Refresh tab GPU */
	else if(HAS_LIBPCI && loop == NB_TAB_GPU)
	{
		j = GPUTEMP1 + 2;
		pcidev(refr->data);
		for(i = 0; i < last_gpu(refr->data); i += GPUFIELDS)
		{
			mvwprintw(refr->win, j,  2, "%13s: %s", refr->data->tabgpu[NAME][GPUTEMP1 + i], refr->data->tabgpu[VALUE][GPUTEMP1 + i]);
			j += GPUFIELDS + 2;
		}
		wrefresh(refr->win);
	}
#endif
}

void main_win(int height, int width, int starty, int startx, int tab, Labels *data)
{
	int i, cpt = 2;
	char buff[MAXSTR];
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* General stuff */
	for(i = NB_TAB_CPU; i <= NB_TAB_ABOUT; i++)
	{
		if(i == tab)
		{
			sprintf(buff, "(%s)", data->objects[i]);
			mvwaddstr(local_win, 1, cpt, buff);
		}
		else
			mvwaddstr(local_win, 1, cpt, data->objects[i]);

		cpt += strlen(data->objects[i]) + 2;
	}

	mvwprintw(local_win, height - 2, 2, PRGNAME);
	mvwprintw(local_win, height - 2, width / 2, data->objects[LABVERSION]);

	wrefresh(local_win);
}

WINDOW *select_tab(int height, int width, int starty, int startx, int num, Labels *data)
{
	switch(num)
	{
		case NB_TAB_CPU:
			return tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_CACHE:
			return tab_cache(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_MB:
			return tab_motherboard(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_RAM:
			return tab_ram(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_SYS:
			return tab_system(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_GPU:
			return tab_graphics(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_ABOUT:
			return tab_about(height - 4, width - 2, starty + 2, startx + 1, data);
		default:
			return tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data); /* If problem */
	}
}

WINDOW *tab_cpu(int height, int width, int starty, int startx, Labels *data)
{
	int i, j, middle;
	WINDOW *local_win;

	middle = (strlen(data->tabcpu[VALUE][MULTIPLIER]) == 0) ? 15 + strlen(data->tabcpu[VALUE][CORESPEED]) + 4 :
								  15 + strlen(data->tabcpu[VALUE][MULTIPLIER]) + 4;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in CPU tab */
	frame(local_win, 1, 1, 11, width - 1, data->objects[FRAMPROCESSOR]);
	frame(local_win, 11, 1, 17, middle, data->objects[FRAMCLOCKS]);
	frame(local_win, 11, middle, 17, width - 1, data->objects[FRAMCACHE]);
	frame(local_win, 17, 1, 20, width - 1, "");

	/* Processor frame */
	j = VENDOR + 2;
	for(i = VENDOR; i < CORESPEED; i++)
	{
		mvwprintw(local_win, j, 2, "%13s: %s", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);
		switch(i)
		{
			case VOLTAGE:
				mvwprintw(local_win, j - 1, 22, "%13s: %s", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);
				break;
			case MODEL:
				mvwprintw(local_win, j - 2, 22, "%11s: %2s", data->tabcpu[NAME][MODEL], data->tabcpu[VALUE][MODEL]);
				break;
			case EXTMODEL:
				mvwprintw(local_win, j - 1, 22, "%11s: %2s", data->tabcpu[NAME][EXTMODEL], data->tabcpu[VALUE][EXTMODEL]);
				break;
			case TEMPERATURE:
				mvwprintw(local_win, j - 2, 38, "%9s: %s", data->tabcpu[NAME][TEMPERATURE], data->tabcpu[VALUE][TEMPERATURE]);
				break;
			case STEPPING:
				mvwprintw(local_win, j - 1, 38, "%9s: %s", data->tabcpu[NAME][STEPPING], data->tabcpu[VALUE][STEPPING]);
				break;
			default:
				j++;
		}
	}

	/* Clocks frame */
	for(i = CORESPEED; i < LEVEL1D; i++)
		mvwprintw(local_win, i - 1, 2, "%13s: %s", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);

	/* Cache frame */
	for(i = LEVEL1D; i < SOCKETS; i++)
		mvwprintw(local_win, i - 5, middle + 1, "%10s: %20s", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);

	/* Last frame */
	mvwprintw(local_win, 18, 4, "%s: %2s", data->tabcpu[NAME][SOCKETS], data->tabcpu[VALUE][SOCKETS]);
	mvwprintw(local_win, 18, 23, "%s: %2s", data->tabcpu[NAME][CORES], data->tabcpu[VALUE][CORES]);
	mvwprintw(local_win, 18, 39, "%s: %2s", data->tabcpu[NAME][THREADS], data->tabcpu[VALUE][THREADS]);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_cache(int height, int width, int starty, int startx, Labels *data)
{
	int i;
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in Caches tab */
	frame(local_win, 1, 1, 6, width - 1, data->objects[FRAMCACHEL1]);
	frame(local_win, 6, 1, 11, width - 1, data->objects[FRAMCACHEL2]);
	frame(local_win, 11, 1, 16, width - 1, data->objects[FRAMCACHEL3]);

	/* L1 Cache frame */
	for(i = L1SIZE; i < L2SIZE; i++)
		mvwprintw(local_win, i + 2,  2, "%13s: %s", data->tabcache[NAME][i], data->tabcache[VALUE][i]);

	/* L2 Cache frame */
	for(i = L2SIZE; i < L3SIZE; i++)
		mvwprintw(local_win, i + 4,  2, "%13s: %s", data->tabcache[NAME][i], data->tabcache[VALUE][i]);

	/* L3 Cache frame */
	for(i = L3SIZE; i < LASTCACHE; i++)
		mvwprintw(local_win, i + 6,  2, "%13s: %s", data->tabcache[NAME][i], data->tabcache[VALUE][i]);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_motherboard(int height, int width, int starty, int startx, Labels *data)
{
	int i;
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in Motherboard tab */
	frame(local_win, 1, 1, 6, width - 1, data->objects[FRAMMOBO]);
	frame(local_win, 6, 1, 12, width - 1, data->objects[FRAMBIOS]);
	frame(local_win, 12, 1, 16, width - 1, data->objects[FRAMCHIP]);

	/* Motherboard frame */
	for(i = MANUFACTURER; i < BRAND; i++)
		mvwprintw(local_win, i + 2,  2, "%13s: %s", data->tabmb[NAME][i], data->tabmb[VALUE][i]);

	/* BIOS frame */
	for(i = BRAND; i < CHIPVENDOR; i++)
		mvwprintw(local_win, i + 4,  2, "%13s: %s", data->tabmb[NAME][i], data->tabmb[VALUE][i]);

	/* Chipset frame */
	for(i = CHIPVENDOR; i < LASTMB; i++)
		mvwprintw(local_win, i + 6,  2, "%13s: %s", data->tabmb[NAME][i], data->tabmb[VALUE][i]);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_ram(int height, int width, int starty, int startx, Labels *data)
{
	int i;
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in RAM tab */
	frame(local_win, 1, 1, data->dimms_count + 3, width - 1, data->objects[FRAMBANKS]);

	/* Banks frame */
	for(i = BANK0_0; i < data->gpu_count; i++)
		mvwprintw(local_win, i + 2,  2, "%13s: %s", data->tabram[NAME][i], data->tabram[VALUE][i]);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_system(int height, int width, int starty, int startx, Labels *data)
{
	int i;
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in System tab */
	frame(local_win, 1, 1, 8, width - 1, data->objects[FRAMOS]);
	frame(local_win, 8, 1, 15, width - 1, data->objects[FRAMMEMORY]);

	/* OS frame */
	for(i = KERNEL; i < USED; i++)
		mvwprintw(local_win, i + 2,  2, "%13s: %s", data->tabsys[NAME][i], data->tabsys[VALUE][i]);

	/* Memory frame */
	for(i = USED; i < LASTSYS; i++)
	{
		mvwprintw(local_win, i + 4,  2, "%13s: %s", data->tabsys[NAME][i], data->tabsys[VALUE][i]);
		draw_bar(local_win, data, i);
	}

	wrefresh(local_win);

	return local_win;
}

void draw_bar(WINDOW *win, Labels *data, int bar)
{
	int i;
	const int val = 38, start = 45, end = 62;
	static double before;
	double percent;

	before = (bar == USED) ? 0 : before;
	percent = (double) strtol(data->tabsys[VALUE][bar], NULL, 10) /
		strtol(strstr(data->tabsys[VALUE][bar], "/ ") + 2, NULL, 10) * 100;

	if(isnan(percent))
		percent = 0.00;

	mvwprintw(win, bar + 4, val, "%.2f%%", percent);
	mvwprintw(win, bar + 4, start, "[");

	for(i = 0; i < (percent / 100) * (end - start); i++)
		mvwprintw(win, bar + 4, start + 1 + (before / 100) * (end - start) + i, "|");

	mvwprintw(win, bar + 4, end, "]");
	before += percent;
}

void clear_bar(WINDOW *win, int bar)
{
	int i;
	const int start = 45, end = 62;

	for(i = 0; i < (end - start); i++)
		mvwprintw(win, bar + 4, start + 1 + i, " ");
}

WINDOW *tab_graphics(int height, int width, int starty, int startx, Labels *data)
{
	int i, start = 1, end = 6, space = 0;
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Card frames */
	for(i = GPUVENDOR1; i < data->gpu_count; i++)
	{
		if(i % GPUFIELDS == 0)
		{
			frame(local_win, start, 1, end, width - 1, data->objects[FRAMGPU1 + i / GPUFIELDS]);
			start = end;
			end += 5;
			space += 2;
		}

		mvwprintw(local_win, i + space,  2, "%13s: %s", data->tabgpu[NAME][i], data->tabgpu[VALUE][i]);
	}

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_about(int height, int width, int starty, int startx, Labels *data)
{
	char *part2 = strdup(data->objects[LABDESCRIPTION]);
	const char *part1 = strsep(&part2, "\n");
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in About tab */
	frame(local_win, 1, 1, 7, width - 1, "");
	frame(local_win, 7, 1, 12, width - 1, data->objects[FRAMABOUT]);
	frame(local_win, 12, 1, 18, width - 1, data->objects[FRAMLICENSE]);

	/* About CPU-X frame */
	mvwprintw(local_win, 3, 4, "%s", part1);
	mvwprintw(local_win, 4, 4, "%s", part2);
	mvwprintw(local_win, 8, 20, "%s", data->objects[LABVERSION]);
	mvwprintw(local_win, 9, 20, "%s", data->objects[LABAUTHOR]);
	mvwaddstr(local_win, 10, 20, "GitHub : https://github.com/X0rg");
	mvwprintw(local_win, 13, 20, "%s", PRGCPYR);
	mvwprintw(local_win, 15, 4, "%s", data->objects[LABLICENSE]);
	mvwaddstr(local_win, 16, 20, "\tGPLv3");

	wrefresh(local_win);

	return local_win;
}

void frame(WINDOW *local_win, int starty, int startx, int endy, int endx, char *label)
{
	int i;

	/* 4 corners */
	mvwprintw(local_win, starty, startx, "+");
	mvwprintw(local_win, endy - 1, startx, "+");
	mvwprintw(local_win, starty, endx - 1, "+");
	mvwprintw(local_win, endy - 1, endx - 1, "+");

	/* Sides */
	for (i = starty + 1; i < (endy - 1); i++)
	{
		mvwprintw(local_win, i, startx, "|");
		mvwprintw(local_win, i, endx - 1, "|");
	}

	/* Top and bottom */
	for (i = startx + 1; i < (endx - 1); i++)
	{
		if(i < startx + 2 || i > (int) strlen(label) + startx + 1)
			mvwprintw(local_win, starty, i, "-");
		if(i == startx + 2)
			mvwprintw(local_win, starty, i, "%s", label);
		mvwprintw(local_win, endy - 1, i, "-");
	}
}
