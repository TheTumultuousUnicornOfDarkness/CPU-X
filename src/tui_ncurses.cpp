/****************************************************************************
*    Copyright © 2014-2023 The Tumultuous Unicorn Of Darkness
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
* FILE tui_ncurses.cpp
*/

#include <unistd.h>
#include <ncurses.h>
#include <term.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <array>
#include "options.hpp"
#include "data.hpp"
#include "core.hpp"
#include "tui_ncurses.hpp"

#if HAS_GETTEXT
# include <libintl.h>
#endif


/* Pairs class */

Pairs::Definition::Definition(const short pair, const short foreground, const short background, const int attrs) :
	pair(pair),
	foreground(foreground),
	background(background),
	attrs(attrs)
{
}

void Pairs::init()
{
	for(auto& color : Pairs::colors)
		init_pair(color.pair, color.foreground, color.background);
}

int Pairs::get_attrs(const Pairs::Colors pair)
{
	return ((pair >= DEFAULT_COLOR) && (pair < LAST_COLOR)) ? Pairs::colors[pair].attrs : 0;
}


/************************* Private functions *************************/

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
	while(keymaps[i][Options::get_keymap()] != ch_mod)
		i++;

	return keymaps[i][0];
}
#undef ALT_CODE
#undef ALT

/* Put window in the center of the screen */
static bool resize_window(WINDOW *pwin)
{
	static int prev_lines = -1, prev_cols = -1;

	bool ret = true;
	const int starty = (LINES - SizeInfo::height) / 2;
	const int startx = (COLS  - SizeInfo::width)  / 2;

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
		wresize(pwin, SizeInfo::height, SizeInfo::width);
		mvwin(pwin, starty, startx);
		printw("%s\n", _("Press 'h' to see help."));
		timeout(Options::get_refr_time() * 1000);
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
static int mvwprintwc(WINDOW *win, int y, int x, Pairs::Colors pair, const char *fmt, ...)
{
	int ret;
	va_list args;

	va_start(args, fmt);
	wmove(win, y, x);

	if(Options::get_color())
		wattron(win, COLOR_PAIR(pair) | Pairs::get_attrs(pair));
	ret = vwprintw(win, fmt, args);
	if(Options::get_color())
		wattroff(win, COLOR_PAIR(pair) | Pairs::get_attrs(pair));

	va_end(args);
	return ret;
}

static int mvwprintwc(WINDOW *win, int y, int x, Pairs::Colors pair, const std::string &str)
{
	return mvwprintwc(win, y, x, pair, "%s", str.c_str());
}

/* Similar to mvwprintw, but print first string in black and second string in blue */
static int mvwprintw2c(WINDOW *win, int y, int x, const char *fmt_name, const char *fmt_value, const Label &label)
{
	int ret = 0;
	Pairs::Colors pair;

	/* Print label name */
	wmove(win, y, x);
	pair = Pairs::Colors::LABEL_NAME_COLOR;
	if(Options::get_color())
		wattron(win, COLOR_PAIR(pair) | Pairs::get_attrs(pair));
	ret += wprintw(win, fmt_name, label.name.c_str());
	ret += wprintw(win, ": ");
	if(Options::get_color())
		wattroff(win, COLOR_PAIR(pair) | Pairs::get_attrs(pair));

	/* Print label value */
	pair = Pairs::Colors::LABEL_VALUE_COLOR;
	if(Options::get_color())
		wattron(win, COLOR_PAIR(pair) | Pairs::get_attrs(pair));
	ret += wprintw(win, fmt_value, label.value.c_str());
	if(Options::get_color())
		wattroff(win, COLOR_PAIR(pair) | Pairs::get_attrs(pair));

	return ret;
}

/* Draw a frame */
static void draw_frame(WINDOW *win, int starty, int startx, int endy, int endx, const std::string &frame_name)
{
	if(Options::get_color())
	{
		init_pair(Pairs::Colors::DEFAULT_COLOR, COLOR_BLACK, COLOR_WHITE);
		wattron(win, COLOR_PAIR(Pairs::Colors::DEFAULT_COLOR));
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
	mvwprintwc(win, starty, startx + 2, Pairs::Colors::DEFAULT_COLOR, frame_name);

	if(Options::get_color())
		wattroff(win, COLOR_PAIR(Pairs::Colors::DEFAULT_COLOR));
}

static void draw_frame(WINDOW *win, int starty, int startx, int endy, int endx, const Frame &frame)
{
	draw_frame(win, starty, startx, endy, endx, frame.name);
}

/* Print how to use this TUI */
static void print_help()
{
	timeout(-1);
	nodelay(stdscr, FALSE);

	printw(_("Welcome in %s NCurses help!\n"), PRGNAME);
	printw("%s\n", _("This help describes how to use this Text-based User Interface."));

	printw("\n%s\n", _("Global keys:"));
	printw("\t%s\n", _("Press 'left' key or 'tab' key to switch in left tab."));
	printw("\t%s\n", _("Press 'right' key or 'shift + tab' keys to switch in right tab."));
	printw("\t%s\n", _("Press 'h' key to see this help."));
	printw("\t%s\n", _("Press 'q' key to exit."));

	printw("\n%s\n", _("CPU tab:"));
	printw("\t%s\n", _("Press 'next page' key to decrease core type number to monitor."));
	printw("\t%s\n", _("Press 'previous page' key to increase core type number to monitor."));
	printw("\t%s\n", _("Press 'down' key to decrease CPU core number to monitor."));
	printw("\t%s\n", _("Press 'up' key to increase CPU core number to monitor."));

	printw("\n%s\n", _("Caches tab:"));
	printw("\t%s\n", _("Press 'down' key to switch to previous test."));
	printw("\t%s\n", _("Press 'up' key to switch to next test."));

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
	timeout(Options::get_refr_time() * 1000);
}

/* The main window (title, tabs, footer) */
static void main_win(WINDOW *win, Data &data)
{
	int cpt = 2;
	const std::array<Tab*, 8> tab_list =
	{
		&data.cpu,
		&data.caches,
		&data.motherboard,
		&data.memory,
		&data.system,
		&data.graphics,
		&data.bench,
		&data.about
	};

	if(Options::get_color())
	{
		init_pair(Pairs::Colors::DEFAULT_COLOR, COLOR_BLACK, COLOR_WHITE);
		wattrset(win, COLOR_PAIR(Pairs::Colors::DEFAULT_COLOR));
	}
	wclrscr(win);
	box(win, 0, 0);

	mvwprintwc(win, TITLE_LINE, SizeInfo::width / 2 - std::strlen(PRGNAME) / 2, Pairs::Colors::TITLE_COLOR, PRGNAME);
	mvwprintwc(win, HEADER_LINE, 12, Pairs::Colors::DEFAULT_COLOR, PRGNAME);
	mvwprintwc(win, HEADER_LINE, SizeInfo::width / 2, Pairs::Colors::DEFAULT_COLOR, data.about.about.version);

	for(int i = 1; i < SizeInfo::width - 1; i++)
		mvwprintwc(win, TABS_LINE, i, Pairs::Colors::INACTIVE_TAB_COLOR, " ");
	for(uint8_t i = 0; i < tab_list.size(); i++)
	{
		if(static_cast<TabNumber>(i) == Options::get_selected_page())
		{
			if(Options::get_color())
				mvwprintwc(win, TABS_LINE, cpt, Pairs::Colors::ACTIVE_TAB_COLOR, tab_list[i]->name);
			else
				mvwprintw(win, TABS_LINE, cpt++, "[%s]", tab_list[i]->name.c_str());
		}
		else
			mvwprintwc(win, TABS_LINE, cpt, Pairs::Colors::INACTIVE_TAB_COLOR, tab_list[i]->name);
		cpt += tab_list[i]->name.size() + 2;
	}

}

/* CPU tab */
static void ntab_cpu(WINDOW *win, Data &data)
{
	const unsigned length = SizeInfo::width - (SizeInfo::tb + 18);
	const auto& cpu_type = data.cpu.get_selected_cpu_type();
	std::string::size_type index_prev = 0, indes_cur = 0;
	Label instructions1 = Label(cpu_type.processor.instructions.name);
	Label instructions2 = Label(cpu_type.processor.instructions.name);

	/* Split Intructions label in two parts */
	while((indes_cur = cpu_type.processor.instructions.value.find(",", indes_cur)) != std::string::npos)
	{
		if(indes_cur >= length)
		{
			instructions1.value = cpu_type.processor.instructions.value.substr(0, index_prev);
			instructions2.value = cpu_type.processor.instructions.value.substr(index_prev + 1);
			break;
		}
		indes_cur++;
		index_prev = indes_cur;
	}

	/* Processor frame */
	draw_frame(win, LINE_0, SizeInfo::start , LINE_10, SizeInfo::width - 1, cpu_type.processor);
	mvwprintw2c(win, LINE_1, SizeInfo::tb, "%14s", "%s", cpu_type.processor.vendor);
	mvwprintw2c(win, LINE_2, SizeInfo::tb, "%14s", "%s", cpu_type.processor.codename);
	mvwprintw2c(win, LINE_3, SizeInfo::tb, "%14s", "%s", cpu_type.processor.package);
	mvwprintw2c(win, LINE_4, SizeInfo::tb, "%14s", "%s", cpu_type.processor.technology);
	mvwprintw2c(win, LINE_4, SizeInfo::tm, "%11s", "%s", cpu_type.processor.voltage);
	mvwprintw2c(win, LINE_5, SizeInfo::tb, "%14s", "%s", cpu_type.processor.specification);
	mvwprintw2c(win, LINE_6, SizeInfo::tb, "%14s", "%s", cpu_type.processor.family);
	mvwprintw2c(win, LINE_6, SizeInfo::tm, "%11s", "%s", cpu_type.processor.model);
	mvwprintw2c(win, LINE_6, SizeInfo::te, "%9s",  "%s", cpu_type.processor.temperature);
	mvwprintw2c(win, LINE_7, SizeInfo::tb, "%14s", "%s", cpu_type.processor.dispfamily);
	mvwprintw2c(win, LINE_7, SizeInfo::tm, "%11s", "%s", cpu_type.processor.dispmodel);
	mvwprintw2c(win, LINE_7, SizeInfo::te, "%9s",  "%s", cpu_type.processor.stepping);
	mvwprintw2c(win, LINE_8, SizeInfo::tb, "%14s", "%s", instructions1);
	mvwprintwc (win, LINE_9, SizeInfo::tb + 16, Pairs::Colors::LABEL_VALUE_COLOR, instructions2.value);

	/* Clocks frame */
	draw_frame(win, LINE_11, SizeInfo::start, LINE_16, SizeInfo::middle, data.cpu.clocks);
	mvwprintw2c(win, LINE_12, SizeInfo::tb, "%14s", "%s", data.cpu.clocks.core_speed);
	mvwprintw2c(win, LINE_13, SizeInfo::tb, "%14s", "%s", data.cpu.clocks.multiplier);
	mvwprintw2c(win, LINE_14, SizeInfo::tb, "%14s", "%s", data.cpu.clocks.bus_speed);
	mvwprintw2c(win, LINE_15, SizeInfo::tb, "%14s", "%s", data.cpu.clocks.usage);

	/* Cache frame */
	draw_frame(win, LINE_11, SizeInfo::middle, LINE_16, SizeInfo::width - 1, cpu_type.caches);
	mvwprintw2c(win, LINE_12, SizeInfo::middle + 1, "%12s", "%s", cpu_type.caches.level1d);
	mvwprintw2c(win, LINE_13, SizeInfo::middle + 1, "%12s", "%s", cpu_type.caches.level1i);
	mvwprintw2c(win, LINE_14, SizeInfo::middle + 1, "%12s", "%s", cpu_type.caches.level2);
	mvwprintw2c(win, LINE_15, SizeInfo::middle + 1, "%12s", "%s", cpu_type.caches.level3);

	/* Footer frame */
	draw_frame(win, LINE_17, SizeInfo::start, LINE_19, SizeInfo::width - 1, "");
	mvwprintwc(win, LINE_18, 4, Pairs::Colors::DEFAULT_COLOR, data.cpu.get_selected_cpu_type_formatted());
	mvwprintwc(win, LINE_18, SizeInfo::quarter + 6, Pairs::Colors::DEFAULT_COLOR, data.cpu.get_selected_cpu_core_formatted());
	mvwprintw2c(win, LINE_18, SizeInfo::middle,                "%s", "%2s", cpu_type.footer.cores);
	mvwprintw2c(win, LINE_18, SizeInfo::middle + SizeInfo::quarter, "%s", "%2s", cpu_type.footer.threads);
}

/* Caches tab */
static void ntab_caches(WINDOW *win, Data &data)
{
	int line = LINE_1, start = LINE_0, end = LINE_3;
	const int frame_height = end - start + 1;

	/* Cache frames */
	for(const auto& cache : data.caches.get_selected_cpu_type().caches)
	{
		draw_frame(win, start, SizeInfo::start, end, SizeInfo::width - 1, cache);
		line = start + 1;
		mvwprintw2c(win, line++, 2, "%13s", "%s", cache.size);
		mvwprintw2c(win, line++, 2, "%13s", "%s", cache.speed);
		start = end + 1;
		end  += frame_height;
	}

	/* Test frame */
	draw_frame(win, LINE_17, SizeInfo::start, LINE_19, SizeInfo::width - 1, data.caches.test);
	if(data.caches.test.names.size() > 0)
		mvwprintwc(win, LINE_18, SizeInfo::tb + 1, Pairs::Colors::DEFAULT_COLOR, data.caches.test.get_selected_test_formatted());
}

/* Motherboard tab */
static void ntab_motherboard(WINDOW *win, Data &data)
{
	/* Motherboard frame */
	draw_frame(win, LINE_0, SizeInfo::start , LINE_4, SizeInfo::width - 1, data.motherboard.board);
	mvwprintw2c(win, LINE_1, SizeInfo::tb, "%13s", "%s", data.motherboard.board.manufacturer);
	mvwprintw2c(win, LINE_2, SizeInfo::tb, "%13s", "%s", data.motherboard.board.model);
	mvwprintw2c(win, LINE_3, SizeInfo::tb, "%13s", "%s", data.motherboard.board.revision);

	/* BIOS frame */
	draw_frame(win, LINE_5, SizeInfo::start , LINE_10, SizeInfo::width - 1, data.motherboard.bios);
	mvwprintw2c(win, LINE_6, SizeInfo::tb, "%13s", "%s", data.motherboard.bios.brand);
	mvwprintw2c(win, LINE_7, SizeInfo::tb, "%13s", "%s", data.motherboard.bios.version);
	mvwprintw2c(win, LINE_8, SizeInfo::tb, "%13s", "%s", data.motherboard.bios.date);
	mvwprintw2c(win, LINE_9, SizeInfo::tb, "%13s", "%s", data.motherboard.bios.romsize);

	/* Chipset frame */
	draw_frame(win, LINE_11, SizeInfo::start , LINE_14, SizeInfo::width - 1, data.motherboard.chipset);
	mvwprintw2c(win, LINE_12, SizeInfo::tb, "%13s", "%s", data.motherboard.chipset.vendor);
	mvwprintw2c(win, LINE_13, SizeInfo::tb, "%13s", "%s", data.motherboard.chipset.model);
}

/* Memory tab */
static void ntab_memory(WINDOW *win, Data &data)
{
	if(data.memory.sticks.size() == 0)
		return;

	const auto& stick = data.memory.get_selected_stick();

	/* Card frame */
	draw_frame(win, LINE_0, SizeInfo::start, LINE_10, SizeInfo::width - 1, stick);
	mvwprintw2c(win, LINE_1, SizeInfo::tb, "%16s", "%s", stick.manufacturer);
	mvwprintw2c(win, LINE_2, SizeInfo::tb, "%16s", "%s", stick.part_number);
	mvwprintw2c(win, LINE_3, SizeInfo::tb, "%16s", "%s", stick.type);
	mvwprintw2c(win, LINE_4, SizeInfo::tb, "%16s", "%s", stick.type_detail);
	mvwprintw2c(win, LINE_5, SizeInfo::tb, "%16s", "%s", stick.device_locator);
	mvwprintw2c(win, LINE_6, SizeInfo::tb, "%16s", "%s", stick.bank_locator);
	mvwprintw2c(win, LINE_7, SizeInfo::tb, "%16s", "%s", stick.size);
	mvwprintw2c(win, LINE_7, SizeInfo::tm, "%18s", "%s", stick.rank);
	mvwprintw2c(win, LINE_8, SizeInfo::tb, "%16s", "%s", stick.speed);
	mvwprintw2c(win, LINE_9, SizeInfo::tb, "%16s", "%s", stick.voltage);

	/* Sticks frame */
	draw_frame(win, LINE_17, SizeInfo::start , LINE_19, SizeInfo::width - 1, data.memory.footer);
	mvwprintwc(win, LINE_18, 4, Pairs::Colors::DEFAULT_COLOR, data.memory.get_selected_stick_formatted());
}

/* Draw an usage bar in System tab */
static void draw_bar(WINDOW *win, int line, Pairs::Colors pair, bool start_left, long double used, long double total)
{
	int i, bar_count, adjust = 0;
	static int before = 0;
	const int val = 39, start = 46, end = SizeInfo::width - 3, size = end - start - 1;
	long double percent;

	if(total == 0.0)
		return;

	before    = start_left ? 0 : before;
	percent   = used / total;
	if(std::isnan(percent))
		return;
	bar_count = int(std::round(percent * (size - 1)));
	if(0.0 < percent && bar_count < 1)
	{
		bar_count = 1;
		adjust    = 1;
	}

	/* Write percentage + delimiters */
	mvwprintwc(win, line, val,   Pairs::Colors::LABEL_VALUE_COLOR, "%.2Lf%%", percent * 100);
	mvwprintwc(win, line, start, Pairs::Colors::DEFAULT_COLOR, "[");
	mvwprintwc(win, line, end,   Pairs::Colors::DEFAULT_COLOR, "]");

	/* Draw bar */
	for(i = 0; i < bar_count; i++)
	{
		if(Options::get_color())
			mvwprintwc(win, line, start + 1 + before + i, pair, " ");
		else
			mvwprintw(win,  line, start + 1 + before + i, "|");
	}

	before += bar_count - adjust;
}

/* System tab */
static void ntab_system(WINDOW *win, Data &data)
{
	/* OS frame */
	draw_frame(win, LINE_0, SizeInfo::start , LINE_5, SizeInfo::width - 1, data.system.os);
	mvwprintw2c(win, LINE_1, SizeInfo::tb, "%13s", "%s", data.system.os.name);
	mvwprintw2c(win, LINE_2, SizeInfo::tb, "%13s", "%s", data.system.os.kernel);
	mvwprintw2c(win, LINE_3, SizeInfo::tb, "%13s", "%s", data.system.os.hostname);
	mvwprintw2c(win, LINE_4, SizeInfo::tb, "%13s", "%s", data.system.os.uptime);

	/* Memory frame */
	draw_frame(win, LINE_6, SizeInfo::start , LINE_12, SizeInfo::width - 1, data.system.memory);
	mvwprintw2c(win, LINE_7, SizeInfo::tb, "%13s", "%s", data.system.memory.used);
	draw_bar(win,    LINE_7, Pairs::Colors::YELLOW_BAR_COLOR, true, data.system.memory.mem_used, data.system.memory.mem_total);
	mvwprintw2c(win, LINE_8, SizeInfo::tb, "%13s", "%s", data.system.memory.buffers);
	draw_bar(win,    LINE_8, Pairs::Colors::BLUE_BAR_COLOR, false, data.system.memory.mem_buffers, data.system.memory.mem_total);
	mvwprintw2c(win, LINE_9, SizeInfo::tb, "%13s", "%s", data.system.memory.cached);
	draw_bar(win,    LINE_9, Pairs::Colors::MAGENTA_BAR_COLOR, false, data.system.memory.mem_cached, data.system.memory.mem_total);
	mvwprintw2c(win, LINE_10, SizeInfo::tb, "%13s", "%s", data.system.memory.free);
	draw_bar(win,    LINE_10, Pairs::Colors::GREEN_BAR_COLOR, false, data.system.memory.mem_free, data.system.memory.mem_total);
	mvwprintw2c(win, LINE_11, SizeInfo::tb, "%13s", "%s", data.system.memory.swap);
	draw_bar(win,    LINE_11, Pairs::Colors::RED_BAR_COLOR, true, data.system.memory.swap_used, data.system.memory.swap_total);
}

/* Graphics tab */
static void ntab_graphics(WINDOW *win, Data &data)
{
	if(data.graphics.cards.size() == 0)
		return;

	const auto& card = data.graphics.get_selected_card();

	/* Card frame */
	draw_frame(win, LINE_0, SizeInfo::start, LINE_14, SizeInfo::width - 1, card);
	mvwprintw2c(win, LINE_1,  SizeInfo::tb, "%13s", "%s", card.vendor);
	mvwprintw2c(win, LINE_1,  SizeInfo::tm, "%18s", "%s", card.kernel_driver);
	mvwprintw2c(win, LINE_2,  SizeInfo::tb, "%13s", "%s", card.user_mode_driver);
	mvwprintw2c(win, LINE_3,  SizeInfo::tb, "%13s", "%s", card.model);
	mvwprintw2c(win, LINE_4,  SizeInfo::tb, "%13s", "%s", card.comp_unit);
	mvwprintw2c(win, LINE_4,  SizeInfo::tm, "%18s", "%s", card.device_id);
	mvwprintw2c(win, LINE_5,  SizeInfo::tb, "%13s", "%s", card.vbios_version);
	mvwprintw2c(win, LINE_6,  SizeInfo::tb, "%13s", "%s", card.interface);
	mvwprintw2c(win, LINE_7,  SizeInfo::tb, "%13s", "%s", card.temperature);
	mvwprintw2c(win, LINE_7,  SizeInfo::tm, "%18s", "%s", card.usage);
	mvwprintw2c(win, LINE_8,  SizeInfo::tb, "%13s", "%s", card.core_voltage);
	mvwprintw2c(win, LINE_8,  SizeInfo::tm, "%18s", "%s", card.power_avg);
	mvwprintw2c(win, LINE_9,  SizeInfo::tb, "%13s", "%s", card.core_clock);
	mvwprintw2c(win, LINE_9,  SizeInfo::tm, "%18s", "%s", card.mem_clock);
	mvwprintw2c(win, LINE_10, SizeInfo::tb, "%13s", "%s", card.mem_used);
	mvwprintw2c(win, LINE_11, SizeInfo::tb, "%13s", "%s", card.resizable_bar);
	mvwprintw2c(win, LINE_11, SizeInfo::tm, "%18s", "%s", card.vulkan_rt);
	mvwprintw2c(win, LINE_12, SizeInfo::tb, "%13s", "%s", card.opengl_version);
	mvwprintw2c(win, LINE_12, SizeInfo::tm, "%18s", "%s", card.vulkan_version);
	mvwprintw2c(win, LINE_13, SizeInfo::tb, "%13s", "%s", card.opencl_version);

	/* Cards frame */
	draw_frame(win, LINE_17, SizeInfo::start , LINE_19, SizeInfo::width - 1, data.graphics.footer);
	mvwprintwc(win, LINE_18, 4, Pairs::Colors::DEFAULT_COLOR, data.graphics.get_selected_card_formatted());
}

/* Bench tab */
static void ntab_bench(WINDOW *win, Data &data)
{
	/* Prime numbers (slow) frame */
	draw_frame(win, LINE_0, SizeInfo::start , LINE_3, SizeInfo::width - 1, data.bench.prime_slow);
	mvwprintw2c(win, LINE_1, SizeInfo::tb, "%13s", "%s", data.bench.prime_slow.score);
	mvwprintw2c(win, LINE_2, SizeInfo::tb, "%13s", "%s", data.bench.prime_slow.state);

	/* Prime numbers (fast) frame */
	draw_frame(win, LINE_4, SizeInfo::start , LINE_7, SizeInfo::width - 1, data.bench.prime_fast);
	mvwprintw2c(win, LINE_5, SizeInfo::tb, "%13s", "%s", data.bench.prime_fast.score);
	mvwprintw2c(win, LINE_6, SizeInfo::tb, "%13s", "%s", data.bench.prime_fast.state);

	/* Parameters frame */
	draw_frame(win, LINE_17, SizeInfo::start , LINE_19, SizeInfo::width - 1, data.bench.parameters);
	mvwprintw2c(win, LINE_18, SizeInfo::tb, "%13s", "%s", data.bench.parameters.duration);
	mvwprintw2c(win, LINE_18, SizeInfo::tm, "%13s", "%s", data.bench.parameters.threads);
}

/* About tab */
static void ntab_about(WINDOW *win, Data &data)
{
	const auto pos = data.about.description.text.find("\n");
	const std::string description1 = data.about.description.text.substr(0, pos);
	const std::string description2 = data.about.description.text.substr(pos + 1);

	/* About CPU-X frame */
	draw_frame(win, LINE_0, SizeInfo::start, LINE_5, SizeInfo::width - 1, "");
	mvwprintwc(win, LINE_2, 4, Pairs::Colors::DEFAULT_COLOR, description1);
	mvwprintwc(win, LINE_3, 4, Pairs::Colors::DEFAULT_COLOR, description2);

	/* About frame */
	draw_frame(win, LINE_6, SizeInfo::start, LINE_10, SizeInfo::width - 1, data.about.about);
	mvwprintwc(win, LINE_7, 4, Pairs::Colors::DEFAULT_COLOR, data.about.about.version);
	mvwprintwc(win, LINE_8, 4, Pairs::Colors::DEFAULT_COLOR, data.about.about.author);
	mvwprintwc(win, LINE_9, 4, Pairs::Colors::DEFAULT_COLOR, data.about.about.website);

	/* License frame */
	draw_frame(win, LINE_11, SizeInfo::start, LINE_16, SizeInfo::width - 1, data.about.license);
	mvwprintwc(win, LINE_12, 4, Pairs::Colors::DEFAULT_COLOR, data.about.license.copyright);
	mvwprintwc(win, LINE_14, 4, Pairs::Colors::DEFAULT_COLOR, data.about.license.name);
	mvwprintwc(win, LINE_15, 4, Pairs::Colors::DEFAULT_COLOR, data.about.license.warranty);
}

static void (*func_ptr[])(WINDOW*, Data&) =
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

/* Draw main window and tab content */
static void draw_window(WINDOW *win, Data &data)
{
	main_win(win, data);
	(*func_ptr[Options::get_selected_page()])(win, data);
	wrefresh(win);
}

/* Refresh dynamic values */
static void nrefresh(WINDOW *win, Data &data)
{
	switch(Options::get_selected_page())
	{
		case TAB_CPU:
		case TAB_CACHES:
		case TAB_SYSTEM:
		case TAB_GRAPHICS:
		case TAB_BENCH:
			do_refresh(data, Options::get_selected_page());
			draw_window(win, data);
			break;
		default:
			return;
	}
}


/************************* Public function *************************/

/* Start CPU-X in NCurses mode */
void start_tui_ncurses(Data &data)
{

	int err = 0, ch = 0;
	WINDOW *win;

	MSG_VERBOSE("%s", _("Starting NCurses TUI…"));
	if(!std::getenv("TERMINFO"))
		setenv("TERMINFO", TERMINFODIR, 0);
	std::freopen("/dev/null", "a", stderr);

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

	if(Options::get_color())
	{
		start_color();
		Options::set_color(has_colors());
	}
	if(Options::get_color())
		Pairs::init();

	win = newwin(SizeInfo::height, SizeInfo::width, 0, 0);
	if(resize_window(win))
		draw_window(win, data);

	while(ch != 'q')
	{
		ch = convert_char(getch());
		switch(ch)
		{
			case KEY_LEFT:
			case SHIFT_TAB:
				/* Switch to left tab */
				if(Options::set_selected_page_prev())
					if(resize_window(win))
						draw_window(win, data);
				break;
			case KEY_RIGHT:
			case KEY_TAB:
				/* Switch to right tab */
				if(Options::set_selected_page_next())
					if(resize_window(win))
						draw_window(win, data);
				break;
			case KEY_DOWN:
				if((Options::get_selected_page() == TAB_CPU) && (Options::get_selected_core() > 0))
				{
					Options::set_selected_core(Options::get_selected_core() - 1, data.cpu.get_selected_cpu_type().footer.num_threads);
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_CACHES) && (Options::get_selected_test() > 0))
				{
					Options::set_selected_test(Options::get_selected_test() - 1);
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_MEMORY) && (Options::get_selected_stick() > 0))
				{
					Options::set_selected_stick(Options::get_selected_stick() - 1, data.memory.sticks.size());
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_GRAPHICS) && (Options::get_selected_gpu() > 0))
				{
					Options::set_selected_gpu(Options::get_selected_gpu() - 1, data.graphics.cards.size());
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_BENCH) && data.bench.parameters.set_duration(data.bench.parameters.duration_i - 1))
					draw_window(win, data);
				break;
			case KEY_NPAGE:
				if((Options::get_selected_page() == TAB_CPU) && (Options::get_selected_type() > 0))
				{
					Options::set_selected_type(Options::get_selected_type() - 1, data.cpu.cpu_types.size());
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_BENCH) && !data.bench.is_running && (data.bench.parameters.threads_i > 0))
					if(data.bench.parameters.set_threads(data.bench.parameters.threads_i - 1))
						draw_window(win, data);
				break;
			case KEY_UP:
				if((Options::get_selected_page() == TAB_CPU) && (Options::get_selected_core() < data.cpu.get_selected_cpu_type().footer.num_threads - 1))
				{
					Options::set_selected_core(Options::get_selected_core() + 1, data.cpu.get_selected_cpu_type().footer.num_threads);
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_CACHES) && (Options::get_selected_test() < data.caches.test.names.size() - 1))
				{
					Options::set_selected_test(Options::get_selected_test() + 1);
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_MEMORY) && (Options::get_selected_stick() < data.memory.sticks.size() - 1))
				{
					Options::set_selected_stick(Options::get_selected_stick() + 1, data.memory.sticks.size());
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_GRAPHICS) && (Options::get_selected_gpu() < data.graphics.cards.size() - 1))
				{
					Options::set_selected_gpu(Options::get_selected_gpu() + 1, data.graphics.cards.size());
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_BENCH) && data.bench.parameters.set_duration(data.bench.parameters.duration_i + 1))
					draw_window(win, data);
				break;
			case KEY_PPAGE:
				if((Options::get_selected_page() == TAB_CPU) && (Options::get_selected_type() < data.cpu.cpu_types.size() - 1))
				{
					Options::set_selected_type(Options::get_selected_type() + 1, data.cpu.cpu_types.size());
					draw_window(win, data);
				}
				else if((Options::get_selected_page() == TAB_BENCH) && !data.bench.is_running && (data.bench.parameters.threads_i < std::thread::hardware_concurrency()))
					if(data.bench.parameters.set_threads(data.bench.parameters.threads_i + 1))
						draw_window(win, data);
				break;
			case 'f':
				if(Options::get_selected_page() == TAB_BENCH)
				{
					if(data.bench.is_running)
						data.bench.is_running = false;
					else
					{
						data.bench.fast_mode = true;
						start_benchmarks(data);
					}
					nrefresh(win, data);
				}
				break;
			case 's':
				if(Options::get_selected_page() == TAB_BENCH)
				{
					if(data.bench.is_running)
						data.bench.is_running = false;
					else
					{
						data.bench.fast_mode = false;
						start_benchmarks(data);
					}
					nrefresh(win, data);
				}
				break;
			case 'h':
				erase();
				print_help();
				if(resize_window(win))
					draw_window(win, data);
				break;
			case ERR:
				/* Refresh dynamic labels */
				if(resize_window(win))
					nrefresh(win, data);
				break;
			case KEY_RESIZE:
				/* Resize window */
				if(resize_window(win))
					draw_window(win, data);
				break;
			default:
				break;
		}
	}

	endwin();
}
