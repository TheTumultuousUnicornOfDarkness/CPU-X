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
* FILE tui_ncurses.h
*/

#ifndef _TUI_NCURSES_HPP_
#define _TUI_NCURSES_HPP_


#include <ncurses.h>
#include <vector>

#define DEFAULT_TERM "xterm"

/* Convert keys when an alternative mapping is used */
#define ALT_CODE 27
#define ALT(x)   (x & ALT_CODE)

/* glibc's term.h pulls in sys/ttydefaults.h which has it, but musl's does not. */
#ifndef CTRL
#define CTRL(x)  (x & 037)
#endif

/* Tab key */
#define KEY_TAB   '\t'
#define SHIFT_TAB 353

enum Lines
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


/* Pairs class */

class Pairs
{
public:
	enum Colors
	{
		DEFAULT_COLOR = 1,
		TITLE_COLOR,
		ACTIVE_TAB_COLOR,
		INACTIVE_TAB_COLOR,
		LABEL_NAME_COLOR,
		LABEL_VALUE_COLOR,
		YELLOW_BAR_COLOR,
		BLUE_BAR_COLOR,
		MAGENTA_BAR_COLOR,
		GREEN_BAR_COLOR,
		RED_BAR_COLOR,
		LAST_COLOR
	};

	struct Definition
	{
		const short pair, foreground, background;
		const int attrs;

		Definition(const short pair, const short foreground, const short background, const int attrs);
	};

	static void init();
	static int get_attrs(const Colors pair);

private:
	static const inline std::vector<Definition> colors =
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
};


/* SizeInfo class */

struct SizeInfo
{
	static const inline int height  = LINE_COUNT;
	static const inline int width   = 76;
	static const inline int start   = 1;
	static const inline int middle  = width / 2;
	static const inline int quarter = (middle - 3) / 2;

	static const inline int tb      = 2;
	static const inline int tm      = 26;
	static const inline int te      = 48;
};


#endif /* _TUI_NCURSES_HPP_ */
