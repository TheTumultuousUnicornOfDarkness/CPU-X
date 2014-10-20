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
#include "cpu-x.h"
#include "includes.h"


void start_gui_ncurses(Libcpuid *data, Dmi *extrainfo, Internal *global) {
	int startx, starty, width, height, ch, current_tab = 0;
	WINDOW *master, *tab;

	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

	height = 25;
	width = 62;
	starty = (LINES - height) / 2; /* Calculating for a center placement of the window */
	startx = (COLS - width) / 2;

	printw("Press 'q' to exit");
	refresh();
	master = main_win(height, width, starty, startx, current_tab);
	tab = tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data, extrainfo, global);

	while((ch = getch()) != 'q')
	{	
		switch(ch)
		{	case KEY_LEFT:
				if(current_tab > 0) {
					current_tab--;
					destroy_win(tab);
					master = main_win(height, width, starty, startx, current_tab);
					tab = select_tab(height, width, starty, startx, current_tab, data, extrainfo, global);
				}
				break;
			case KEY_RIGHT:
				if(current_tab < 2) {
					current_tab++;
					destroy_win(tab);
					master = main_win(height, width, starty, startx, current_tab);
					tab = select_tab(height, width, starty, startx, current_tab, data, extrainfo, global);
				}
				break;	
		}
	}

	endwin();
}

WINDOW *main_win(int height, int width, int starty, int startx, int tab) {
	char tab_name[3][15] = { "CPU", "Mainboard", "About" };
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* General stuff */
	mvwaddstr(local_win, 1, 2, tab_name[tab]);
	mvwprintw(local_win, height - 2, 2, PRGNAME);
	mvwprintw(local_win, height - 2, width / 2, "Version %s", PRGVER);

	wrefresh(local_win);

	return local_win;
}

WINDOW *select_tab(int height, int width, int starty, int startx, int num, Libcpuid *data, Dmi *extrainfo, Internal *global) {
	switch(num) {
		case 0:
			return tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data, extrainfo, global);
		case 1:
			return tab_mainboard(height - 4, width - 2, starty + 2, startx + 1, data, extrainfo);
		case 2:
			return tab_about(height - 4, width - 2, starty + 2, startx + 1, data, extrainfo);
		default:
			return tab_cpu(height - 4, width - 2, starty + 2, startx + 1, data, extrainfo, global); /* If problem */
	}
}

WINDOW *tab_cpu(int height, int width, int starty, int startx, Libcpuid *data, Dmi *extrainfo, Internal *global) {
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in CPU tab */
	frame(local_win, 1, 1, 11, width - 1, "Processor");
	frame(local_win, 11, 1, 17, width / 2, "Clocks");
	frame(local_win, 11, width / 2, 17, width - 1, "Cache");
	frame(local_win, 17, 1, 20, width - 1, "");

	/* Processor frame */
	mvwprintw(local_win, 2, 2, "%13s: %s", "Vendor", extrainfo->vendor);
	mvwprintw(local_win, 3, 2, "%13s: %s", "Code Name", data->name);
	mvwprintw(local_win, 4, 2, "%13s: %s", "Package", extrainfo->socket);
	mvwprintw(local_win, 5, 2, "%13s: %s", "Architecture", data->arch);
	mvwprintw(local_win, 6, 2, "%13s: %s", "Specification", data->spec);
	mvwprintw(local_win, 7, 2, "%13s: %2s", "Family", data->fam);
	mvwprintw(local_win, 7, 22, "%11s: %2s", "Model", data->mod);
	mvwprintw(local_win, 7, 38, "%9s: %s", "Stepping", data->step);
	mvwprintw(local_win, 8, 2, "%13s: %2s", "Ext. Family", data->extfam);
	mvwprintw(local_win, 8, 22, "%11s: %2s", "Ext. Model", data->extmod);
	mvwprintw(local_win, 9, 2, "%13s: %s", "Instructions", global->instr);

	/* Clocks frame */
	mvwprintw(local_win, 12, 2, "%13s: %s", "Core Speed", global->clock);
	mvwprintw(local_win, 13, 2, "%13s: %s", "Multiplier", global->mults);
	mvwprintw(local_win, 14, 2, "%13s: %s", "Bus Speed", extrainfo->bus);
	mvwprintw(local_win, 15, 2, "%13s: %s", "BogoMIPS", global->mips);

	/* Cache frame */
	mvwprintw(local_win, 12, width / 2 + 2, "%7s: %10s %6s", "L1 Data", data->l1d, data->l1dw);
	mvwprintw(local_win, 13, width / 2 + 2, "%7s: %10s %6s", "L1 Inst", data->l1i, data->l1iw);
	mvwprintw(local_win, 14, width / 2 + 2, "%7s: %10s %6s", "Level 2", data->l2, data->l2w);
	mvwprintw(local_win, 15, width / 2 + 2, "%7s: %10s %6s", "Level 3", data->l3, data->l3w);

	/* Last frame */
	mvwprintw(local_win, 18, 4, "%s: %2s", "Sockets(s)", data->soc);
	mvwprintw(local_win, 18, 23, "%s: %2s", "Core(s)", data->core);
	mvwprintw(local_win, 18, 39, "%s: %2s", "Thread(s)", data->thrd);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_mainboard(int height, int width, int starty, int startx, Libcpuid *data, Dmi *extrainfo) {
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in Mainboard tab */
	frame(local_win, 1, 1, 6, width - 1, "Motherboard");
	frame(local_win, 6, 1, 12, width - 1, "BIOS");

	/* Motherboard frame */
	mvwprintw(local_win, 2, 2, "%13s: %s", "Manufactureur", extrainfo->manu);
	mvwprintw(local_win, 3, 2, "%13s: %s", "Model", extrainfo->model);
	mvwprintw(local_win, 4, 2, "%13s: %s", "Revision", extrainfo->rev);

	/* BIOS frame */
	mvwprintw(local_win, 7, 2, "%13s: %s", "Brand", extrainfo->brand);
	mvwprintw(local_win, 8, 2, "%13s: %s", "Version", extrainfo->version);
	mvwprintw(local_win, 9, 2, "%13s: %s", "Date", extrainfo->date);
	mvwprintw(local_win, 10, 2, "%13s: %s", "ROM Size", extrainfo->rom);

	wrefresh(local_win);

	return local_win;
}

WINDOW *tab_about(int height, int width, int starty, int startx, Libcpuid *data, Dmi *extrainfo) {
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);

	/* Frames in About tab */
	frame(local_win, 1, 1, 16, width - 1, "About CPU-X");

	/* About CPU-X frame */
	mvwaddstr(local_win, 3, 2, "\tBased on GTK3+ library");
	mvwprintw(local_win, 4, 2, "\tCompiled with NCusrses %i.%i", NCURSES_VERSION_MAJOR, NCURSES_VERSION_MINOR);
	mvwprintw(local_win, 6, 2, "\tVersion : %s", PRGVER);
	mvwaddstr(local_win, 7, 2, "\tAuthor : X0rg");
	mvwaddstr(local_win, 8, 2, "\tGitHub : https://github.com/X0rg");
	mvwaddstr(local_win, 10, 2, "\tCopyright © 2014 Xorg");
	mvwaddstr(local_win, 11, 2, "\tThis program comes with ABSOLUTELY NO WARRANTY");
	mvwaddstr(local_win, 12, 2, "\tSee the following license");
	mvwaddstr(local_win, 13, 2, "\tGPLv3");

	wrefresh(local_win);

	return local_win;
}

void frame(WINDOW *local_win, int starty, int startx, int endy, int endx, char *label) {
	int i;

	/* 4 corners */
	mvwprintw(local_win, starty, startx, "+");
	mvwprintw(local_win, endy - 1, startx, "+");
	mvwprintw(local_win, starty, endx - 1, "+");
	mvwprintw(local_win, endy - 1, endx - 1, "+");

	/* Sides */
	for (i = starty + 1; i < (endy - 1); i++) {
		mvwprintw(local_win, i, startx, "|");
		mvwprintw(local_win, i, endx - 1, "|");
	}

	/* Top and bottom */
	for (i = startx + 1; i < (endx - 1); i++) {
		if(i < startx + 2 || i > strlen(label) + startx + 1)
			mvwprintw(local_win, starty, i, "-");
		if(i == startx + 2)
			mvwprintw(local_win, starty, i, "%s", label);
		mvwprintw(local_win, endy - 1, i, "-");
	}
}

void destroy_win(WINDOW *local_win) {	
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(local_win);
	delwin(local_win);
}
