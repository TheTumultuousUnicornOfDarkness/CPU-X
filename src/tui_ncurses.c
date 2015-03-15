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
* ncurses.c
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libintl.h>
#include "cpu-x.h"
#include "tui_ncurses.h"

static int loop = NB_TAB_CPU;


void start_tui_ncurses(Labels *data)
{
	int startx, starty, width, height, ch, current_tab = 0;
	WINDOW *tab;
	NThrd refr;

	MSGVERB(_("Starting NCurses TUI..."));
	if(getuid())
	{
		fprintf(stderr, "\n\t\t\t\033[1;33m%s\033[0m\n", MSGROOT);
		sleep(1);
	}

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
	timeout(refreshtime * 1000);

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
				if(current_tab == NB_TAB_CPU || current_tab == NB_TAB_SYS)
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
}

void nrefresh(NThrd *refr)
{
	int i;

	/* Refresh tab CPU */
	if(loop == NB_TAB_CPU)
	{
		cpufreq(refr->data->tabcpu[VALUE][BUSSPEED], refr->data->tabcpu[VALUE][CORESPEED], refr->data->tabcpu[VALUE][MULTIPLIER]);
		if(HAS_LIBDMI && !getuid())
		{
			libdmidecode(refr->data);
			mvwprintw(refr->win, 13, 2, "%13s: %s", refr->data->tabcpu[NAME][MULTIPLIER], refr->data->tabcpu[VALUE][MULTIPLIER]);
		}
		mvwprintw(refr->win, 12, 2, "%13s: %s", refr->data->tabcpu[NAME][CORESPEED], refr->data->tabcpu[VALUE][CORESPEED]);
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
}

void main_win(int height, int width, int starty, int startx, int tab, Labels *data)
{
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* General stuff */
	mvwaddstr(local_win, 1, 2, data->objects[tab]);
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
		case NB_TAB_MB:
			return tab_mainboard(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_RAM:
			return tab_ram(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_SYS:
			return tab_system(height - 4, width - 2, starty + 2, startx + 1, data);
		case NB_TAB_ABOUT:
			return tab_about(height - 4, width - 2, starty + 2, startx + 1, data);
		default:
			return tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data); /* If problem */
	}
}

WINDOW *tab_cpu(int height, int width, int starty, int startx, Labels *data)
{
	int i, middle;
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
	for(i = VENDOR; i < MODEL; i++)
		mvwprintw(local_win, i + 2, 2, "%13s: %s", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);

	mvwprintw(local_win, FAMILY + 2,	22, "%11s: %2s", data->tabcpu[NAME][MODEL], data->tabcpu[VALUE][MODEL]);
	mvwprintw(local_win, FAMILY + 2,	38, "%9s: %s", data->tabcpu[NAME][STEPPING], data->tabcpu[VALUE][STEPPING]);
	mvwprintw(local_win, EXTFAMILY + 2,	22, "%11s: %2s", data->tabcpu[NAME][EXTMODEL], data->tabcpu[VALUE][EXTMODEL]);
	mvwprintw(local_win, EXTFAMILY + 3,	2, "%13s: %2s", data->tabcpu[NAME][INSTRUCTIONS], data->tabcpu[VALUE][INSTRUCTIONS]);

	/* Clocks frame */
	for(i = CORESPEED; i < LEVEL1D; i++)
		mvwprintw(local_win, i + 1, 2, "%13s: %s", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);

	/* Cache frame */
	for(i = LEVEL1D; i < SOCKETS; i++)
		mvwprintw(local_win, i - 3, middle + 1, "%10s: %20s", data->tabcpu[NAME][i], data->tabcpu[VALUE][i]);

	/* Last frame */
	mvwprintw(local_win, 18, 4, "%s: %2s", data->tabcpu[NAME][SOCKETS], data->tabcpu[VALUE][SOCKETS]);
	mvwprintw(local_win, 18, 23, "%s: %2s", data->tabcpu[NAME][CORES], data->tabcpu[VALUE][CORES]);
	mvwprintw(local_win, 18, 39, "%s: %2s", data->tabcpu[NAME][THREADS], data->tabcpu[VALUE][THREADS]);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_mainboard(int height, int width, int starty, int startx, Labels *data)
{
	int i;
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in Mainboard tab */
	frame(local_win, 1, 1, 6, width - 1, data->objects[FRAMMOBO]);
	frame(local_win, 6, 1, 12, width - 1, data->objects[FRAMBIOS]);

	/* Motherboard frame */
	for(i = MANUFACTURER; i < BRAND; i++)
		mvwprintw(local_win, i + 2,  2, "%13s: %s", data->tabmb[NAME][i], data->tabmb[VALUE][i]);

	/* BIOS frame */
	for(i = BRAND; i < LASTMB; i++)
		mvwprintw(local_win, i + 4,  2, "%13s: %s", data->tabmb[NAME][i], data->tabmb[VALUE][i]);

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
	frame(local_win, 1, 1, last_bank(data) + 3, width - 1, data->objects[FRAMBANKS]);

	/* Banks frame */
	for(i = BANK0_0; i < last_bank(data); i++)
		mvwprintw(local_win, i + 2,  2, "%13s: %s", data->tabram[NAME][i], data->tabram[VALUE][i]);

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
