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
* ncurses.c
*/

#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <libintl.h>
#include "cpu-x.h"
#include "includes.h"

static int loop = 0;


void start_gui_ncurses(Labels *data)
{
	int startx, starty, width, height, ch, current_tab = 0;
	WINDOW *tab;
	pthread_t thrdrefr;
	NThrd refr;

	if(getuid())
	{
		fprintf(stderr, MSGROOT, PRGNAME);
		fprintf(stderr, _("Start in 3 seconds...\n"));
		sleep(3);
	}

	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

	height = 25;
	width = 68;
	starty = (LINES - height) / 2; /* Calculating for a center placement of the window */
	startx = (COLS - width) / 2;

	printw("Press 'q' to exit");
	refresh();
	main_win(height, width, starty, startx, current_tab, data);
	tab = tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data);

	refr.win = tab;
	refr.data = data;
	pthread_create(&thrdrefr, NULL, nrefresh, &refr);

	while((ch = getch()) != 'q')
	{	
		switch(ch)
		{	case KEY_LEFT:
				if(current_tab > 0)
				{
					current_tab--;
					pthread_cancel(thrdrefr);
					destroy_win(tab);
					main_win(height, width, starty, startx, current_tab, data);
					tab = select_tab(height, width, starty, startx, current_tab, data);
				}
				break;
			case KEY_RIGHT:
				if(current_tab < 4)
				{
					current_tab++;
					pthread_cancel(thrdrefr);
					destroy_win(tab);
					main_win(height, width, starty, startx, current_tab, data);
					tab = select_tab(height, width, starty, startx, current_tab, data);
				}
				break;
		}

		loop = current_tab;

		if(current_tab == 0 || current_tab == 3)
		{
			refr.win = tab;
			pthread_create(&thrdrefr, NULL, nrefresh, &refr);
		}
	}

	endwin();
}

void *nrefresh(void *ptr)
{
	NThrd *refr = (NThrd *) ptr;

	/* Refresh tab CPU */
	while(loop == 0)
	{
		cpufreq(refr->data->tabcpu[VALUE][BUSSPEED], refr->data->tabcpu[VALUE][CORESPEED], refr->data->tabcpu[VALUE][MULTIPLIER]);
		if(HAS_LIBDMI && !getuid())
		{
			libdmidecode(refr->data);
			mvwprintw(refr->win, 13, 2, "%13s: %s", refr->data->tabcpu[NAME][MULTIPLIER], refr->data->tabcpu[VALUE][MULTIPLIER]);
		}
		mvwprintw(refr->win, 12, 2, "%13s: %s", refr->data->tabcpu[NAME][CORESPEED], refr->data->tabcpu[VALUE][CORESPEED]);
		wrefresh(refr->win);
		sleep(refreshtime);
	}

	/* Refresh tab System */
	while(loop == 3)
	{
		tabsystem(refr->data);
		mvwprintw(refr->win, 5,   2, "%13s: %s", refr->data->tabsys[NAME][UPTIME],	refr->data->tabsys[VALUE][UPTIME]);
		mvwprintw(refr->win, 9,   2, "%13s: %s", refr->data->tabsys[NAME][USED],	refr->data->tabsys[VALUE][USED]);
		mvwprintw(refr->win, 10,  2, "%13s: %s", refr->data->tabsys[NAME][BUFFERS],	refr->data->tabsys[VALUE][BUFFERS]);
		mvwprintw(refr->win, 11,  2, "%13s: %s", refr->data->tabsys[NAME][CACHED],	refr->data->tabsys[VALUE][CACHED]);
		mvwprintw(refr->win, 12,  2, "%13s: %s", refr->data->tabsys[NAME][FREE],	refr->data->tabsys[VALUE][FREE]);
		mvwprintw(refr->win, 13,  2, "%13s: %s", refr->data->tabsys[NAME][SWAP],	refr->data->tabsys[VALUE][SWAP]);
		wrefresh(refr->win);
		sleep(refreshtime);
	}

	return NULL;
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
		case 0:
			return tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data);
		case 1:
			return tab_mainboard(height - 4, width - 2, starty + 2, startx + 1, data);
		case 2:
			return tab_ram(height - 4, width - 2, starty + 2, startx + 1, data);
		case 3:
			return tab_system(height - 4, width - 2, starty + 2, startx + 1, data);
		case 4:
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
	frame(local_win, 1, 1, 19, width - 1, data->objects[FRAMBANKS]);

	/* Banks frame */
	for(i = BANK0_0; i < LASTRAM; i++)
		mvwprintw(local_win, i + 2,  2, "%13s: %s", data->tabram[NAME][i], data->tabram[VALUE][i]);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_system(int height, int width, int starty, int startx, Labels *data)
{
	int i;WINDOW *local_win;

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
		mvwprintw(local_win, i + 4,  2, "%13s: %s", data->tabsys[NAME][i], data->tabsys[VALUE][i]);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_about(int height, int width, int starty, int startx, Labels *data)
{
	char *part2 = strstr(data->objects[LABDESCRIPTION], "\n");
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in About tab */
	frame(local_win, 1, 1, 7, width - 1, "");
	frame(local_win, 7, 1, 12, width - 1, data->objects[FRAMABOUT]);
	frame(local_win, 12, 1, 18, width - 1, data->objects[FRAMLICENSE]);

	/* About CPU-X frame */
	strcpy(part2, "\0");
	mvwprintw(local_win, 3, 4, "%s", data->objects[LABDESCRIPTION]);
	mvwprintw(local_win, 4, 4, "%s", part2 + 1);
	mvwprintw(local_win, 8, 20, "%s", data->objects[LABVERSION]);
	mvwprintw(local_win, 9, 20, "%s", data->objects[LABAUTHOR]);
	mvwaddstr(local_win, 10, 20, "GitHub : https://github.com/X0rg");
	mvwaddstr(local_win, 13, 20, "Copyright © 2014 Xorg");
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
		if(i < startx + 2 || i > strlen(label) + startx + 1)
			mvwprintw(local_win, starty, i, "-");
		if(i == startx + 2)
			mvwprintw(local_win, starty, i, "%s", label);
		mvwprintw(local_win, endy - 1, i, "-");
	}
}

void destroy_win(WINDOW *local_win)
{	
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(local_win);
	delwin(local_win);
}
