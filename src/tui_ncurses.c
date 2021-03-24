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
* FILE tui_ncurses.c
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <term.h>
#include <math.h>
#include <libintl.h>
#include "cpu-x.h"
#include "tui_ncurses.h"

static void (*func_ptr[])(WINDOW*, const SizeInfo, Labels*) =
{
	ntab_cpu,
	ntab_caches,
	ntab_motherboard,
	ntab_memory,
	ntab_system,
	ntab_graphics,
	ntab_bench,
	ntab_about
};
static const Colors color[] =
{
	{ 0,                  0,        0,             0             },
	{ DEFAULT_COLOR,      COLOR_BLACK,   COLOR_WHITE,   A_NORMAL },
	{ TITLE_COLOR,        COLOR_BLUE,    COLOR_WHITE,   A_BOLD   },
	{ ACTIVE_TAB_COLOR,   COLOR_WHITE,   COLOR_BLUE,    A_BOLD   },
	{ INACTIVE_TAB_COLOR, COLOR_WHITE,   COLOR_BLUE,    A_NORMAL },
	{ LABEL_NAME_COLOR,   COLOR_BLACK,   COLOR_WHITE,   A_NORMAL },
	{ LABEL_VALUE_COLOR,  COLOR_BLUE,    COLOR_WHITE,   A_NORMAL },
	{ YELLOW_BAR_COLOR,   COLOR_YELLOW,  COLOR_YELLOW,  A_NORMAL },
	{ BLUE_BAR_COLOR,     COLOR_BLUE,    COLOR_BLUE,    A_NORMAL },
	{ MAGENTA_BAR_COLOR,  COLOR_MAGENTA, COLOR_MAGENTA, A_NORMAL },
	{ GREEN_BAR_COLOR,    COLOR_GREEN,   COLOR_GREEN,   A_NORMAL },
	{ RED_BAR_COLOR,      COLOR_RED,     COLOR_RED,     A_NORMAL }
};


/************************* Public function *************************/

/* Start CPU-X in NCurses mode */
void start_tui_ncurses(Labels *data)
{
	int err = 0, ch = 0;
	const SizeInfo info = { .height = LINE_COUNT, .width = 70, .start = 1, .tb = 2, .tm = 26, .te = 48 };
	NThrd refr = { .data = data, .info = info };
	WINDOW *win;

	MSG_VERBOSE("%s", _("Starting NCurses TUI…"));
	if(!getenv("TERMINFO"))
		setenv("TERMINFO", TERMINFODIR, 0);
	freopen("/dev/null", "a", stderr);
	if(setupterm(NULL, STDOUT_FILENO, &err))
	{
		if(err == -1)
		{
			MSG_ERROR("%s", _("FATAL ERROR: terminfo database could not be found (try to set TERMINFO environment variable)"));
			exit(255);
		}
		else
		{
			MSG_WARNING(_("Failed to set up %s terminal (err=%i); falling back to %s"), getenv("TERM"), err, DEFAULT_TERM);
			setenv("TERM", DEFAULT_TERM, 1);
			setupterm(DEFAULT_TERM, STDOUT_FILENO, &err);
		}
	}

	initscr();
	cbreak();
	noecho();
	curs_set(0);
	halfdelay(0);
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	if(opts->color)
	{
		start_color();
		opts->color = has_colors();
	}
	if(opts->color)
	{
		enum EnColors pair;
		for(pair = DEFAULT_COLOR; pair < LAST_COLOR; pair++)
			init_pair(pair, color[pair].foreground, color[pair].background);
	}

	win      = newwin(info.height, info.width, 0, 0);
	refr.win = win;

	if(resize_window(win, info))
	{
		main_win(win, info, data);
		(*func_ptr[opts->selected_page])(win, info, data);
	}

	while(ch != 'q')
	{
		ch = convert_char(getch());
		switch(ch)
		{
			case KEY_LEFT:
				/* Switch to left tab */
				if(opts->selected_page > NO_CPU)
				{
					opts->selected_page--;
					if(resize_window(win, info))
					{
						main_win(win, info, data);
						(*func_ptr[opts->selected_page])(win, info, data);
					}
				}
				break;
			case KEY_RIGHT:
				/* Switch to right tab */
				if(opts->selected_page < NO_ABOUT)
				{
					opts->selected_page++;
					if(resize_window(win, info))
					{
						main_win(win, info, data);
						(*func_ptr[opts->selected_page])(win, info, data);
					}
				}
				break;
			case KEY_DOWN:
				if(opts->selected_page == NO_CPU && opts->selected_core > 0)
				{
					opts->selected_core--;
					print_activecore(win);
				}
				else if(opts->selected_page == NO_CACHES && opts->bw_test > 0)
				{
					opts->bw_test--;
					print_activetest(win, info, data);
				}
				else if(opts->selected_page == NO_GRAPHICS && opts->selected_gpu > 0)
				{
					opts->selected_gpu--;
					print_activecard(win, info, data);
				}
				else if(opts->selected_page == NO_BENCH && data->b_data->duration > 1)
				{
					data->b_data->duration--;
					print_paramduration(win, info, data);
				}
				break;
			case KEY_NPAGE:
				if(opts->selected_page == NO_BENCH && data->b_data->threads > 1 && !data->b_data->run)
				{
					data->b_data->threads--;
					print_paramthreads(win, info, data);
				}
				break;
			case KEY_UP:
				if(opts->selected_page == NO_CPU && (int) opts->selected_core < data->cpu_count - 1)
				{
					opts->selected_core++;
					print_activecore(win);
				}
				else if(opts->selected_page == NO_CACHES && (int) opts->bw_test < data->w_data->test_count - 1)
				{
					opts->bw_test++;
					print_activetest(win, info, data);
				}
				else if(opts->selected_page == NO_GRAPHICS && (int) opts->selected_gpu < data->gpu_count - 1)
				{
					opts->selected_gpu++;
					print_activecard(win, info, data);
				}
				else if(opts->selected_page == NO_BENCH && data->b_data->duration < 60 * 24)
				{
					data->b_data->duration++;
					print_paramduration(win, info, data);
				}
				break;
			case KEY_PPAGE:
				if(opts->selected_page == NO_BENCH && data->b_data->threads < data->cpu_count && !data->b_data->run)
				{
					data->b_data->threads++;
					print_paramthreads(win, info, data);
				}
				break;
			case 'f':
				if(opts->selected_page == NO_BENCH && !data->b_data->run)
				{
					data->b_data->fast_mode = true;
					start_benchmarks(data);
				}
				else if(opts->selected_page == NO_BENCH && data->b_data->run)
					data->b_data->run = false;
				break;
			case 's':
				if(opts->selected_page == NO_BENCH && !data->b_data->run)
				{
					data->b_data->fast_mode = false;
					start_benchmarks(data);
				}
				else if(opts->selected_page == NO_BENCH && data->b_data->run)
					data->b_data->run = false;
				break;
			case 'h':
				erase();
				print_help();
				if(resize_window(win, info))
				{
					main_win(win, info, data);
					(*func_ptr[opts->selected_page])(win, info, data);
				}
				break;
			case ERR:
				/* Refresh dynamic labels */
				if(resize_window(win, info))
				{
					nrefresh(&refr);
					main_win(win, info, data);
					(*func_ptr[opts->selected_page])(win, info, data);
				}
				break;
			case KEY_RESIZE:
				/* Resize window */
				if(resize_window(win, info))
				{
					main_win(win, info, data);
					(*func_ptr[opts->selected_page])(win, info, data);
				}
				break;
			default:
				break;
		}
	}

	endwin();
	labels_free(data);
}


/************************* Private functions *************************/

/* Convert keys when an alternative mapping is used */
#define ALT_CODE 27
#define ALT(x)   (x & ALT_CODE)
static int convert_char(int ch)
{
	int i = 0;
	static int modifier = 0;
	const int ch_mod = modifier ? ch & modifier : ch;

	const int keymaps[][LASTKEYMAP] =
	{
		//Arrow       Emacs       Inverted-T  Vim
		{ KEY_LEFT,   CTRL('b'),  'j',        'h'          },
		{ KEY_RIGHT,  CTRL('f'),  'l',        'l'          },
		{ KEY_UP,     CTRL('p'),  'i',        'k'          },
		{ KEY_DOWN,   CTRL('n'),  'k',        'j'          },
		{ KEY_PPAGE,  CTRL('v'),  CTRL('b'),  CTRL('b')    },
		{ KEY_NPAGE,  ALT('v'),   CTRL('f'),  CTRL('f')    },
		{ 'h',        '?',        '?',        '?'          },
		{ ch_mod,     ch_mod,     ch_mod,     ch_mod       } // Sentinel value
	};

	modifier = (ch == ALT_CODE) ? ch : 0;
	while(keymaps[i][opts->keymap] != ch_mod)
		i++;

	return keymaps[i][0];
}
#undef ALT_CODE
#undef ALT

/* Put window in the center of the screen */
static bool resize_window(WINDOW *pwin, const SizeInfo info)
{
	static int prev_lines = -1, prev_cols = -1;

	bool ret = true;
	const int starty = (LINES - info.height) / 2;
	const int startx = (COLS  - info.width)  / 2;

	if (prev_lines >= 0 && prev_lines == LINES &&
	    prev_cols >= 0 && prev_cols == COLS)
		return true;
	prev_lines = LINES;
	prev_cols = COLS;
	erase();
	if((startx < 0) || (starty < 0))
	{
		printw("%s\n", _("Window is too small!"));
		timeout(-1);
		ret = false;
	}
	else
	{
		wresize(pwin, info.height, info.width);
		mvwin(pwin, starty, startx);
		printw("%s\n", _("Press 'h' to see help."));
		timeout(opts->refr_time * 1000);
	}
	refresh();

	return ret;
}

/* Clean window */
static void wclrscr(WINDOW *pwin)
{
	int y, x, maxy, maxx;

	getmaxyx(pwin, maxy, maxx);
	for(y = 0; y < maxy; y++)
	{
		for(x = 0; x < maxx; x++)
			mvwaddch(pwin, y, x, ' ');
	}
}

/* Similar to mvwprintw, but specify a color pair */
static int mvwprintwc(WINDOW *win, int y, int x, enum EnColors pair, const char *fmt, ...)
{
	int ret;
	va_list args;

	va_start(args, fmt);
	wmove(win, y, x);

	if(opts->color)
		wattron(win, COLOR_PAIR(pair) | color[pair].attrs);
	ret = vwprintw(win, fmt, args);
	if(opts->color)
		wattroff(win, COLOR_PAIR(pair) | color[pair].attrs);

	va_end(args);
	return ret;
}

/* Similar to mvwprintw, but print first string in black and second string in blue */
static int mvwprintw2c(WINDOW *win, int y, int x, const char *fmt, ...)
{
	int ret = 0;
	char *s1, *s2, *f1, *f2, *ptr = strdup(fmt);
	enum EnColors pair;
	va_list args;
	const size_t ptr_len = strlen(ptr) + 1;

	/* Retrive args */
	va_start(args, fmt);
	f2 = strstr(fmt, ": ") + 1;
	f1 = strncat(strtok(ptr, ": "), ":", ptr_len);
	s1 = va_arg(args, char *);
	s2 = va_arg(args, char *);

	/* Print label name */
	wmove(win, y, x);
	pair = LABEL_NAME_COLOR;
	if(opts->color)
		wattron(win, COLOR_PAIR(pair) | color[pair].attrs);
	ret += wprintw(win, f1, s1);
	free(f1);
	if(opts->color)
		wattroff(win, COLOR_PAIR(pair) | color[pair].attrs);

	/* Print label value */
	pair = LABEL_VALUE_COLOR;
	if(opts->color)
		wattron(win, COLOR_PAIR(pair) | color[pair].attrs);
	ret += wprintw(win, f2, s2);
	if(opts->color)
		wattroff(win, COLOR_PAIR(pair) | color[pair].attrs);

	va_end(args);
	return ret;
}

/* Refresh dynamic values */
static void nrefresh(NThrd *refr)
{
	int i, line = 0;
	WINDOW *(win)  = refr->win;
	Labels *(data) = refr->data;
	SizeInfo info  = refr->info;

	switch(opts->selected_page)
	{
		case NO_CPU:
			do_refresh(data);
			mvwprintw2c(win, LINE_4,  info.tm, "%11s: %s", data->tab_cpu[NAME][VOLTAGE],     data->tab_cpu[VALUE][VOLTAGE]);
			mvwprintw2c(win, LINE_6,  info.te, "%9s: %s",  data->tab_cpu[NAME][TEMPERATURE], data->tab_cpu[VALUE][TEMPERATURE]);
			mvwprintw2c(win, LINE_12, info.tb, "%14s: %s", data->tab_cpu[NAME][CORESPEED],   data->tab_cpu[VALUE][CORESPEED]);
			mvwprintw2c(win, LINE_13, info.tb, "%14s: %s", data->tab_cpu[NAME][MULTIPLIER],  data->tab_cpu[VALUE][MULTIPLIER]);
			mvwprintw2c(win, LINE_15, info.tb, "%14s: %s", data->tab_cpu[NAME][USAGE],       data->tab_cpu[VALUE][USAGE]);
			break;
		case NO_CACHES:
			do_refresh(data);
			line = LINE_2;
			for(i = L1SPEED; i < data->cache_count * CACHEFIELDS; i += CACHEFIELDS)
			{
				mvwprintw2c(win, line,  info.tb, "%13s: %s", data->tab_caches[NAME][i], data->tab_caches[VALUE][i]);
				line += CACHEFIELDS + 2;
			}
			break;
		case NO_SYSTEM:
			do_refresh(data);
			mvwprintw2c(win, LINE_4,  info.tb, "%13s: %s", data->tab_system[NAME][UPTIME],   data->tab_system[VALUE][UPTIME]);
			for(i = USED; i < LASTSYSTEM; i++)
			{
				mvwprintw2c(win, LINE_8 + line++, info.tb, "%13s: %s", data->tab_system[NAME][i], data->tab_system[VALUE][i]);
				draw_bar(win, info, data, i);
			}
			break;
		case NO_GRAPHICS:
			do_refresh(data);
			line = LINE_3;
			for(i = 0; i < data->gpu_count * GPUFIELDS; i += GPUFIELDS)
			{
				mvwprintw2c(win, line, info.tm, "%18s: %s", data->tab_graphics[NAME][GPU1DIDRID      + i], data->tab_graphics[VALUE][GPU1DIDRID     + i]);
				line++;
				mvwprintw2c(win, line, info.tb, "%13s: %s", data->tab_graphics[NAME][GPU1PCIE        + i], data->tab_graphics[VALUE][GPU1PCIE       + i]);
				line++;
				mvwprintw2c(win, line, info.tb, "%13s: %s", data->tab_graphics[NAME][GPU1TEMPERATURE + i], data->tab_graphics[VALUE][GPU1TEMPERATURE + i]);
				mvwprintw2c(win, line, info.tm, "%18s: %s", data->tab_graphics[NAME][GPU1USAGE       + i], data->tab_graphics[VALUE][GPU1USAGE       + i]);
				line++;
				mvwprintw2c(win, line, info.tm, "%18s: %s", data->tab_graphics[NAME][GPU1MEMUSED     + i], data->tab_graphics[VALUE][GPU1MEMUSED     + i]);
				line++;
				mvwprintw2c(win, line, info.tb, "%13s: %s", data->tab_graphics[NAME][GPU1CORECLOCK + i], data->tab_graphics[VALUE][GPU1CORECLOCK     + i]);
				mvwprintw2c(win, line, info.tm, "%18s: %s", data->tab_graphics[NAME][GPU1MEMCLOCK  + i], data->tab_graphics[VALUE][GPU1MEMCLOCK      + i]);
				line++;
				mvwprintw2c(win, line, info.tb, "%13s: %s", data->tab_graphics[NAME][GPU1VOLTAGE   + i], data->tab_graphics[VALUE][GPU1VOLTAGE       + i]);
				mvwprintw2c(win, line, info.tm, "%18s: %s", data->tab_graphics[NAME][GPU1POWERAVG  + i], data->tab_graphics[VALUE][GPU1POWERAVG      + i]);
				line += LINE_3 - 1;
			}
			break;
		case NO_BENCH:
			do_refresh(data);
			mvwprintw2c(win, LINE_1, info.tb, "%13s: %s", data->tab_bench[NAME][PRIMESLOWSCORE], data->tab_bench[VALUE][PRIMESLOWSCORE]);
			mvwprintw2c(win, LINE_2, info.tb, "%13s: %s", data->tab_bench[NAME][PRIMESLOWRUN],   data->tab_bench[VALUE][PRIMESLOWRUN]);
			mvwprintw2c(win, LINE_5, info.tb, "%13s: %s", data->tab_bench[NAME][PRIMEFASTSCORE], data->tab_bench[VALUE][PRIMEFASTSCORE]);
			mvwprintw2c(win, LINE_6, info.tb, "%13s: %s", data->tab_bench[NAME][PRIMEFASTRUN],   data->tab_bench[VALUE][PRIMEFASTRUN]);
			break;
		default:
			return;
	}

	wrefresh(win);
}

/* Print how to use this TUI */
static void print_help()
{
	timeout(-1);
	nodelay(stdscr, FALSE);

	printw(_("Welcome in %s NCurses help!\n"), PRGNAME);
	printw("%s\n", _("This help describes how to use this Text-based User Interface."));

	printw("\n%s\n", _("Global keys:"));
	printw("\t%s\n", _("Press 'left' key to switch in left tab."));
	printw("\t%s\n", _("Press 'right' key to switch in right tab."));
	printw("\t%s\n", _("Press 'h' key to see this help."));
	printw("\t%s\n", _("Press 'q' key to exit."));

	printw("\n%s\n", _("CPU tab:"));
	printw("\t%s\n", _("Press 'down' key to decrease core number to monitor."));
	printw("\t%s\n", _("Press 'up' key to increase core number to monitor."));

	printw("\n%s\n", _("Caches tab:"));
	printw("\t%s\n", _("Press 'down' key to switch to previous test."));
	printw("\t%s\n", _("Press 'up' key' to switch to next test."));

	printw("\n%s\n", _("Bench tab:"));
	printw("\t%s\n", _("Press 'down' key to decrement benchmark duration."));
	printw("\t%s\n", _("Press 'up' key to increment benchmark duration."));
	printw("\t%s\n", _("Press 'next page' key to decrement number of threads to use."));
	printw("\t%s\n", _("Press 'previous page' key to increment number of threads to use."));
	printw("\t%s\n", _("Press 's' key to start/stop prime numbers (slow) benchmark."));
	printw("\t%s\n", _("Press 'f' key to start/stop prime numbers (fast) benchmark."));

	printw("\n%s\n", _("Graphics tab:"));
	printw("\t%s\n", _("Press 'down' key to switch to previous graphic card."));
	printw("\t%s\n", _("Press 'up' key to switch to next graphic card."));

	printw("\n%s\n", _("Press any key to exit this help."));

	refresh();
	getch();
	nodelay(stdscr, TRUE);
	timeout(opts->refr_time * 1000);
}

/* The main window (title, tabs, footer) */
static void main_win(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, cpt = 2;

	if(opts->color)
	{
		init_pair(DEFAULT_COLOR, COLOR_BLACK, COLOR_WHITE);
		wattrset(win, COLOR_PAIR(DEFAULT_COLOR));
	}
	wclrscr(win);
	box(win, 0, 0);

	mvwprintwc(win, TITLE_LINE, info.width / 2 - strlen(PRGNAME) / 2, TITLE_COLOR, PRGNAME);
	mvwprintwc(win, HEADER_LINE, 2, DEFAULT_COLOR, PRGNAME);
	mvwprintwc(win, HEADER_LINE, info.width / 2, DEFAULT_COLOR, data->tab_about[VERSIONSTR]);

	for(i = 1; i < info.width - 1; i++)
		mvwprintwc(win, TABS_LINE, i, INACTIVE_TAB_COLOR, " ");
	for(i = NO_CPU; i <= NO_ABOUT; i++)
	{
		if(i == (int) opts->selected_page && opts->color)
			mvwprintwc(win, TABS_LINE, cpt, ACTIVE_TAB_COLOR, data->objects[i]);
		else if(i == (int) opts->selected_page && !opts->color)
			mvwprintw(win, TABS_LINE, cpt++, "[%s]", data->objects[i]);
		else
			mvwprintwc(win, TABS_LINE, cpt, INACTIVE_TAB_COLOR, data->objects[i]);

		cpt += strlen(data->objects[i]) + 2;
	}

	wrefresh(win);
}

/* Display active Core in CPU tab */
#define BUFF_SIZE 16
static void print_activecore(WINDOW *win)
{
	char buff[BUFF_SIZE];
	snprintf(buff, BUFF_SIZE, _("Core #%i"), opts->selected_core);
	mvwprintwc(win, LINE_18, 4, DEFAULT_COLOR, buff);
}
#undef BUFF_SIZE

/* CPU tab */
static void ntab_cpu(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, line;
	const int length = info.width - (info.tb + 18);
	const int middle = info.width - 38;
	int index = length - 1;

	/* Split Intructions label in two parts */
	for(i = 0; i < length; i++)
		if(data->tab_cpu[VALUE][INSTRUCTIONS][i] == ',')
			index = i;
	data->tab_cpu[VALUE][INSTRUCTIONS][index + 1] = '\0';

	/* Processor frame */
	frame(win, LINE_0, info.start , LINE_10, info.width - 1, data->objects[FRAMPROCESSOR]);
	line = LINE_1;
	for(i = VENDOR; i < CORESPEED; i++)
	{
		switch(i)
		{
			case VOLTAGE:
				mvwprintw2c(win, LINE_4, info.tm, "%11s: %s", data->tab_cpu[NAME][VOLTAGE],      data->tab_cpu[VALUE][VOLTAGE]);
				break;
			case MODEL:
				mvwprintw2c(win, LINE_6, info.tm, "%11s: %s", data->tab_cpu[NAME][MODEL],        data->tab_cpu[VALUE][MODEL]);
				break;
			case EXTMODEL:
				mvwprintw2c(win, LINE_7, info.tm, "%11s: %s", data->tab_cpu[NAME][EXTMODEL],     data->tab_cpu[VALUE][EXTMODEL]);
				break;
			case TEMPERATURE:
				mvwprintw2c(win, LINE_6, info.te, "%9s: %s",  data->tab_cpu[NAME][TEMPERATURE],  data->tab_cpu[VALUE][TEMPERATURE]);
				break;
			case STEPPING:
				mvwprintw2c(win, LINE_7, info.te, "%9s: %s",  data->tab_cpu[NAME][STEPPING],     data->tab_cpu[VALUE][STEPPING]);
				break;
			case INSTRUCTIONS:
				mvwprintw2c(win, line++, info.tb, "%14s: %s", data->tab_cpu[NAME][INSTRUCTIONS], data->tab_cpu[VALUE][INSTRUCTIONS]);
				mvwprintwc (win, line++, info.tb + 16,        LABEL_VALUE_COLOR,                 &(data->tab_cpu[VALUE][INSTRUCTIONS][index + 2]));
				break;
			default:
				mvwprintw2c(win, line++, info.tb, "%14s: %s", data->tab_cpu[NAME][i],            data->tab_cpu[VALUE][i]);
		}
	}

	/* Clocks frame */
	frame(win, LINE_11, info.start, LINE_16, middle, data->objects[FRAMCLOCKS]);
	line = LINE_12;
	for(i = CORESPEED; i < LEVEL1D; i++)
		mvwprintw2c(win, line++, info.tb, "%14s: %s", data->tab_cpu[NAME][i], data->tab_cpu[VALUE][i]);

	/* Cache frame */
	frame(win, LINE_11, middle, LINE_16, info.width - 1, data->objects[FRAMCACHE]);
	line = LINE_12;
	for(i = LEVEL1D; i < SOCKETS; i++)
		mvwprintw2c(win, line++, middle + 1, "%12s: %s", data->tab_cpu[NAME][i], data->tab_cpu[VALUE][i]);

	/* Last frame */
	frame(win, LINE_17, info.start, LINE_19, info.width - 1, "");
	print_activecore(win);
	mvwprintw2c(win, LINE_18, 18,  "%s: %2s", data->tab_cpu[NAME][SOCKETS], data->tab_cpu[VALUE][SOCKETS]);
	mvwprintw2c(win, LINE_18, 36, "%s: %2s", data->tab_cpu[NAME][CORES],   data->tab_cpu[VALUE][CORES]);
	mvwprintw2c(win, LINE_18, 54, "%s: %2s", data->tab_cpu[NAME][THREADS], data->tab_cpu[VALUE][THREADS]);

	wrefresh(win);
}

/* Display active Test in Caches tab */
static void print_activetest(WINDOW *win, const SizeInfo info, Labels *data)
{
	const int line = LINE_1 + (CACHEFIELDS + 2) * data->cache_count;

	if(!data->cache_count)
		return;

	if(HAS_BANDWIDTH)
		mvwprintwc(win, line, info.tb + 1, DEFAULT_COLOR, "%s", data->w_data->test_name[opts->bw_test]);

	wrefresh(win);
}

/* Caches tab */
static void ntab_caches(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, line = LINE_1, start = LINE_0, end = LINE_3;

	if(!data->cache_count)
		return;

	/* Cache frames */
	for(i = L1SIZE; i < data->cache_count * CACHEFIELDS; i++)
	{
		if(i % CACHEFIELDS == 0)
		{
			frame(win, start, info.start, end, info.width - 1, data->objects[FRAML1CACHE + i / CACHEFIELDS]);
			start = end + 1;
			end += 4;
			if(i > 0)
				line += 2;
		}
		mvwprintw2c(win, line++, 2, "%13s: %s", data->tab_caches[NAME][i], data->tab_caches[VALUE][i]);
	}

	/* Test frame */
	line++;
	frame(win, line, info.start , line + 2, info.width - 1, data->objects[FRAMTEST]);
	print_activetest(win, info, data);

	wrefresh(win);
}

/* Motherboard tab */
static void ntab_motherboard(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, line;

	/* Motherboard frame */
	frame(win, LINE_0, info.start , LINE_4, info.width - 1, data->objects[FRAMMOTHERBOARD]);
	line = LINE_1;
	for(i = MANUFACTURER; i < BRAND; i++)
		mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_motherboard[NAME][i], data->tab_motherboard[VALUE][i]);

	/* BIOS frame */
	frame(win, LINE_5, info.start , LINE_10, info.width - 1, data->objects[FRAMBIOS]);
	line = LINE_6;
	for(i = BRAND; i < CHIPVENDOR; i++)
		mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_motherboard[NAME][i], data->tab_motherboard[VALUE][i]);

	/* Chipset frame */
	frame(win, LINE_11, info.start , LINE_14, info.width - 1, data->objects[FRAMCHIPSET]);
	line = LINE_12;
	for(i = CHIPVENDOR; i < LASTMOTHERBOARD; i++)
		mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_motherboard[NAME][i], data->tab_motherboard[VALUE][i]);

	wrefresh(win);
}

/* Memory tab */
static void ntab_memory(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, line = LINE_0;

	if(!data->dimm_count)
		return;

	/* Banks frame */
	for(i = BANK0; i < data->dimm_count; i++)
	{
		frame(win, line, info.start, line + 2, info.width - 1, data->objects[FRAMBANK0 + i]);
		line++;
		mvwprintw2c(win, line, info.tb, "%13s: %s", data->tab_memory[NAME][i], data->tab_memory[VALUE][i]);
		line += 2;
	}

	wrefresh(win);
}

/* System tab */
static void ntab_system(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, line;

	/* OS frame */
	frame(win, LINE_0, info.start , LINE_6, info.width - 1, data->objects[FRAMOPERATINGSYSTEM]);
	line = LINE_1;
	for(i = KERNEL; i < USED; i++)
		mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_system[NAME][i], data->tab_system[VALUE][i]);

	/* Memory frame */
	frame(win, LINE_7, info.start , LINE_13, info.width - 1, data->objects[FRAMMEMORY]);
	line = LINE_8;
	for(i = USED; i < LASTSYSTEM; i++)
	{
		mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_system[NAME][i], data->tab_system[VALUE][i]);
		draw_bar(win, info, data, i);
	}

	wrefresh(win);
}

/* Draw an usage bar in System tab */
static void draw_bar(WINDOW *win, const SizeInfo info, Labels *data, int bar)
{
	int i, line, color, bar_count, adjust = 0;
	static int before = 0;
	const int val = 39, start = 46, end = info.width - 3, size = end - start;
	double percent;

	if((bar == SWAP && data->m_data->swap_total == 0) || (bar != SWAP && data->m_data->mem_total == 0))
		return;

	line      = bar - USED + LINE_8;
	color     = YELLOW_BAR_COLOR + bar - USED;
	before    = ((bar == USED) || (bar == SWAP)) ? 0 : before;
	percent   = (double) data->m_data->mem_usage[bar - USED] / ((bar == SWAP) ? data->m_data->swap_total : data->m_data->mem_total);
	bar_count = (int) roundf(percent * (size - 1));
	if(0.0 < percent && bar_count < 1)
	{
		bar_count = 1;
		adjust    = 1;
	}

	/* Write percentage + delimiters */
	mvwprintwc(win, line, val,   LABEL_VALUE_COLOR, "%.2f%%", percent * 100);
	mvwprintwc(win, line, start, DEFAULT_COLOR, "[");
	mvwprintwc(win, line, end,   DEFAULT_COLOR, "]");

	/* Draw bar */
	for(i = 0; i < bar_count; i++)
	{
		if(opts->color)
			mvwprintwc(win, line, start + 1 + before + i, color, " ");
		else
			mvwprintw(win,  line, start + 1 + before + i, "|");
	}

	before += bar_count - adjust;
}

/* Display active card in Graphics tab */
static void print_activecard(WINDOW *win, const SizeInfo info, Labels *data)
{
	if(data->gpu_count == 0)
		return;

	mvwprintwc(win, LINE_12, info.tb + 1, DEFAULT_COLOR, "#%i: %s", opts->selected_gpu, data->tab_graphics[VALUE][GPU1MODEL + opts->selected_gpu * GPUFIELDS]);
	wrefresh(win);
}

/* Graphics tab */
static void ntab_graphics(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, j, line, start = LINE_0, end = LINE_10;

	if(data->gpu_count == 0)
		return;

	/* Card frames */
	line = LINE_1;
	frame(win, start, info.start, end, info.width - 1, data->objects[FRAMGPU1 + opts->selected_gpu]);
	for(i = opts->selected_gpu * GPUFIELDS, j = 0; j < GPUFIELDS; i++, j++)
	{
		switch(j)
		{
			case GPU1DRIVER:
			case GPU1USAGE:
			case GPU1MEMCLOCK:
			case GPU1POWERAVG:
				mvwprintw2c(win, line - 1, info.tm, "%18s: %s", data->tab_graphics[NAME][i], data->tab_graphics[VALUE][i]);
				break;
			case GPU1DIDRID:
				mvwprintw2c(win, line++, info.tm, "%18s: %s", data->tab_graphics[NAME][i], data->tab_graphics[VALUE][i]);
				break;
			default:
				mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_graphics[NAME][i], data->tab_graphics[VALUE][i]);
		}
	}

	/* Cards frame */
	line++;
	frame(win, line, info.start , line + 2, info.width - 1, data->objects[FRAMCARDS]);
	print_activecard(win, info, data);

	wrefresh(win);
}

/* Display Duration parameter in Bench tab */
static void print_paramduration(WINDOW *win, const SizeInfo info, Labels *data)
{
	casprintf(&data->tab_bench[VALUE][PARAMDURATION], true, _("%u mins"), data->b_data->duration);
	mvwprintw2c(win, LINE_9, info.tb, "%13s: %s", data->tab_bench[NAME][PARAMDURATION], data->tab_bench[VALUE][PARAMDURATION]);
	wrefresh(win);
}

/* Display Threads parameter in Bench tab */
static void print_paramthreads(WINDOW *win, const SizeInfo info, Labels *data)
{
	casprintf(&data->tab_bench[VALUE][PARAMTHREADS], true, "%u", data->b_data->threads);
	mvwprintw2c(win, LINE_9, info.tm, "%13s: %s", data->tab_bench[NAME][PARAMTHREADS],  data->tab_bench[VALUE][PARAMTHREADS]);
	wrefresh(win);
}

/* Bench tab */
static void ntab_bench(WINDOW *win, const SizeInfo info, Labels *data)
{
	int i, line;

	/* Prime numbers (slow) frame */
	frame(win, LINE_0, info.start , LINE_3, info.width - 1, data->objects[FRAMPRIMESLOW]);
	line = LINE_1;
	for(i = PRIMESLOWSCORE; i < PRIMEFASTSCORE; i++)
		mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_bench[NAME][i], data->tab_bench[VALUE][i]);

	/* Prime numbers (fast) frame */
	frame(win, LINE_4, info.start , LINE_7, info.width - 1, data->objects[FRAMPRIMEFAST]);
	line = LINE_5;
	for(i = PRIMEFASTSCORE; i < PARAMDURATION; i++)
		mvwprintw2c(win, line++, info.tb, "%13s: %s", data->tab_bench[NAME][i], data->tab_bench[VALUE][i]);

	/* Parameters frame */
	frame(win, LINE_8, info.start , LINE_10, info.width - 1, data->objects[FRAMPARAM]);
	print_paramduration(win, info, data);
	print_paramthreads (win, info, data);

	wrefresh(win);
}

/* About tab */
static void ntab_about(WINDOW *win, const SizeInfo info, Labels *data)
{
	char *part2 = strdup(data->tab_about[DESCRIPTION]);
	const char *part1 = strsep(&part2, "\n");

	/* About CPU-X frame */
	frame(win, LINE_0, info.start, LINE_5, info.width - 1, "");
	mvwprintwc(win, LINE_2, 4,   DEFAULT_COLOR, "%s", part1);
	mvwprintwc(win, LINE_3, 4,   DEFAULT_COLOR, "%s", part2);

	frame(win, LINE_6, info.start, LINE_10, info.width - 1, data->objects[FRAMABOUT]);
	mvwprintwc(win, LINE_7, 20,  DEFAULT_COLOR, "%s", data->tab_about[VERSIONSTR]);
	mvwprintwc(win, LINE_8, 20,  DEFAULT_COLOR, "%s", data->tab_about[AUTHOR]);
	mvwprintwc(win, LINE_9, 20,  DEFAULT_COLOR, "%s", data->tab_about[SITE]);

	frame(win, LINE_11, info.start, LINE_16, info.width - 1, data->objects[FRAMLICENSE]);
	mvwprintwc(win, LINE_12, 20, DEFAULT_COLOR, "%s", data->tab_about[COPYRIGHT]);
	mvwprintwc(win, LINE_14, 6,  DEFAULT_COLOR, "%s", data->tab_about[LICENSE]);
	mvwprintwc(win, LINE_15, 10, DEFAULT_COLOR, "%s", data->tab_about[NOWARRANTY]);

	wrefresh(win);
}

/* Draw a frame */
static void frame(WINDOW *win, int starty, int startx, int endy, int endx, char *label)
{
	if(opts->color)
	{
		init_pair(DEFAULT_COLOR, COLOR_BLACK, COLOR_WHITE);
		wattron(win, COLOR_PAIR(DEFAULT_COLOR));
	}

	/* Horizontal lines */
	mvwhline(win, starty, startx, 0, endx - startx);
	mvwhline(win, endy, startx, 0, endx - startx);

	/* Vertical lines */
	mvwvline(win, starty, startx, 0, endy - starty);
	mvwvline(win, starty, endx - 1, 0, endy - starty);

	/* Corners */
	mvwhline(win, starty, startx, ACS_ULCORNER, 1);
	mvwhline(win, endy, startx, ACS_LLCORNER, 1);
	mvwhline(win, starty, endx - 1, ACS_URCORNER, 1);
	mvwhline(win, endy, endx - 1, ACS_LRCORNER, 1);

	/* Title */
	mvwprintwc(win, starty, startx + 2, DEFAULT_COLOR, "%s", label);

	if(opts->color)
		wattroff(win, COLOR_PAIR(DEFAULT_COLOR));
}
