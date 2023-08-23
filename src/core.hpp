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
* FILE core.hpp
*/

#ifndef _CORE_HPP_
#define _CORE_HPP_

#include "data.hpp"


/***************************** External headers *****************************/

/* Load and apply settings from GSettings */
void load_settings(void);

/* Start CPU-X in GTK mode */
int start_gui_gtk(Data &data);

/* Start CPU-X in NCurses mode */
void start_tui_ncurses(Data &data);


/************************* Public functions *************************/

/* Fill labels by calling core functions */
int fill_labels(Data &data);

/* Refresh some labels */
int do_refresh(Data &data, TabNumber tab_number);

/* Call Dmidecode through CPU-X but do nothing else */
int run_dmidecode(void);

/* Call Bandwidth through CPU-X but do nothing else */
int run_bandwidth(void);

/* Perform a multithreaded benchmark (compute prime numbers) */
void start_benchmarks(Data &data);


#endif /* _CORE_HPP_ */
