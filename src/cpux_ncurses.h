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

/* Start CPU-X in NCurses mode */
void cpux_ncurses(Libcpuid *data, Dmi *extrainfo);

/* NCurses main window */
WINDOW *main_win(int height, int width, int starty, int startx, int tab);

/* Switch to different tabs */
WINDOW *select_tab(int height, int width, int starty, int startx, int num, Libcpuid *data, Dmi *extrainfo);

/* Tab CPU */
WINDOW *tab_cpu(int height, int width, int starty, int startx, Libcpuid *data, Dmi *extrainfo);

/* Tab Mainboard */
WINDOW *tab_mainboard(int height, int width, int starty, int startx, Libcpuid *data, Dmi *extrainfo);

/* Tab About */
WINDOW *tab_about(int height, int width, int starty, int startx, Libcpuid *data, Dmi *extrainfo);

/* Draw a frame in a NCurses window */
void frame(WINDOW *local_win, int starty, int startx, int endy, int endx, char *label);

/* Destroy a window */
void destroy_win(WINDOW *local_win);

#endif
