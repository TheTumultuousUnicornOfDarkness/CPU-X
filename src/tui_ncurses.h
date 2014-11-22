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
* cpux_ncurses.h
*/

#ifndef _CPUX_NCURSES_H_
#define _CPUX_NCURSES_H_


#include <ncurses.h>

typedef struct
{
	WINDOW *win;
	Labels *data;
} NThrd;


/* Start CPU-X in NCurses mode */
void start_gui_ncurses(Labels *data);

/* Refresh non-static values */
void nrefresh(NThrd *refr);

/* NCurses main window */
void main_win(int height, int width, int starty, int startx, int tab, Labels *data);

/* Switch to different tabs */
WINDOW *select_tab(int height, int width, int starty, int startx, int num, Labels *data);

/* Tab CPU */
WINDOW *tab_cpu(int height, int width, int starty, int startx, Labels *data);

/* Tab Mainboard */
WINDOW *tab_mainboard(int height, int width, int starty, int startx, Labels *data);

/* Tab RAM */
WINDOW *tab_ram(int height, int width, int starty, int startx, Labels *data);

/* Tab System */
WINDOW *tab_system(int height, int width, int starty, int startx, Labels *data);

/* Tab About */
WINDOW *tab_about(int height, int width, int starty, int startx, Labels *data);

/* Draw a frame in a NCurses window */
void frame(WINDOW *local_win, int starty, int startx, int endy, int endx, char *label);


#endif /* _CPUX_NCURSES_H_ */
