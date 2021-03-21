/****************************************************************************
*    Copyright © 2014-2021 Xorg
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
* FILE tui_ncurses.h
*/

#ifndef _TUI_NCURSES_H_
#define _TUI_NCURSES_H_


#include <ncurses.h>

#define DEFAULT_TERM "xterm"

enum EnLines
{
	WINDOW_TOP_LINE,
	TITLE_LINE,
	TABS_LINE,
	LINE_0,
	LINE_1,
	LINE_2,
	LINE_3,
	LINE_4,
	LINE_5,
	LINE_6,
	LINE_7,
	LINE_8,
	LINE_9,
	LINE_10,
	LINE_11,
	LINE_12,
	LINE_13,
	LINE_14,
	LINE_15,
	LINE_16,
	LINE_17,
	LINE_18,
	LINE_19,
	HEADER_LINE,
	WINDOW_BOTTOM_LINE,
	LINE_COUNT
};

enum EnColors
{
	DEFAULT_COLOR = 1,
	TITLE_COLOR,
	ACTIVE_TAB_COLOR,
	INACTIVE_TAB_COLOR,
	LABEL_NAME_COLOR,
	LABEL_VALUE_COLOR,
	YELLOW_BAR_COLOR,
	BLUE_BAR_COLOR,
	RED_BAR_COLOR,
	GREEN_BAR_COLOR,
	MAGENTA_BAR_COLOR,
	LAST_COLOR
};

typedef struct
{
	const short pair, foreground, background;
	const int attrs;
} Colors;

typedef struct
{
	int height, width, start;
	int tb, tm, te;
} SizeInfo;

typedef struct
{
	WINDOW *win;
	Labels *data;
	SizeInfo info;
} NThrd;


/********************************** TUI  **********************************/

/* Convert keys when an alternative mapping is used */
static int convert_char(int ch);

/* Put window in the center of the screen */
static bool resize_window(WINDOW *pwin, const SizeInfo info);

/* Clean window */
static void wclrscr(WINDOW *pwin);

/* Similar to mvwprintw, but specify a color pair */
static int mvwprintwc(WINDOW *win, int y, int x, enum EnColors pair, const char *fmt, ...);

/* Similar to mvwprintw, but print first string in black and second string in blue */
static int mvwprintw2c(WINDOW *win, int y, int x, const char *fmt, ...);

/* Refresh dynamic values */
static void nrefresh(NThrd *refr);

/* Print how to use this TUI */
static void print_help(void);

/* The main window (title, tabs, footer) */
static void main_win(WINDOW *win, const SizeInfo info, Labels *data);

/* Display active Core in CPU tab */
static void print_activecore(WINDOW *win);

/* CPU tab */
static void ntab_cpu(WINDOW *win, const SizeInfo info, Labels *data);

/* Display active Test in Caches tab */
static void print_activetest(WINDOW *win, const SizeInfo info, Labels *data);

/* Caches tab */
static void ntab_caches(WINDOW *win, const SizeInfo info, Labels *data);

/* Motherboard tab */
static void ntab_motherboard(WINDOW *win, const SizeInfo info, Labels *data);

/* Memory tab */
static void ntab_memory(WINDOW *win, const SizeInfo info, Labels *data);

/* System tab */
static void ntab_system(WINDOW *win, const SizeInfo info, Labels *data);
static void draw_bar(WINDOW *win, const SizeInfo info, Labels *data, int bar);

/* Display active card in Graphics tab */
static void print_activecard(WINDOW *win, const SizeInfo info, Labels *data);

/* Graphics tab */
static void ntab_graphics(WINDOW *win, const SizeInfo info, Labels *data);

/* Display Duration parameter in Bench tab */
static void print_paramduration(WINDOW *win, const SizeInfo info, Labels *data);

/* Display Threads parameter in Bench tab */
static void print_paramthreads(WINDOW *win, const SizeInfo info, Labels *data);

/* Bench tab */
static void ntab_bench(WINDOW *win, const SizeInfo info, Labels *data);

/* About tab */
static void ntab_about(WINDOW *win, const SizeInfo info, Labels *data);

/* Draw a frame */
static void frame(WINDOW *local_win, int starty, int startx, int endy, int endx, char *label);


#endif /* _TUI_NCURSES_H_ */
