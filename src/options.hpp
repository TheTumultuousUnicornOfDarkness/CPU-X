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
* FILE options.hpp
*/

#ifndef _OPTIONS_HPP_
#define _OPTIONS_HPP_

#include <cstdint>

/* Options definition */
#define OUT_GTK               (1 << 0)
#define OUT_NCURSES           (1 << 1)
#define OUT_DUMP              (1 << 2)
#define OUT_NO_CPUX           (1 << 10)
#define OUT_DMIDECODE         (1 << 11)
#define OUT_BANDWIDTH         (1 << 12)

enum OptKeymap
{
	ARROWS,
	EMACS,
	INVERTED_T,
	VIM,
	LASTKEYMAP
};

enum OptTempUnit
{
	CELSIUS,
	FAHRENHEIT,
	KELVIN,
	RANKINE,
	LASTTEMPUNIT
};

enum TabNumber
{
	TAB_CPU, TAB_CACHES, TAB_MOTHERBOARD, TAB_MEMORY, TAB_SYSTEM, TAB_GRAPHICS, TAB_BENCH, TAB_ABOUT
};

class Options
{
public:
	/* Function's prototypes generated with:
	   grep -E "set_|get_" src/options.cpp | sed -e 's|^|static |g' -e 's|Options::||g' -e 's|)|);|g'
	*/
	static void set_cpuid_decimal(bool cpuid_decimal);
	static bool get_cpuid_decimal();
	static void set_color(bool color);
	static bool get_color();
	static void set_issue(bool issue);
	static bool get_issue();
	static void set_with_daemon(bool with_daemon);
	static bool get_with_daemon();
	static void set_debug_database(bool debug_database);
	static bool get_debug_database();
	static void set_fallback_cpu_temp(bool fallback_cpu_temp);
	static bool get_fallback_cpu_temp();
	static void set_fallback_cpu_volt(bool fallback_cpu_volt);
	static bool get_fallback_cpu_volt();
	static void set_fallback_cpu_freq(bool fallback_cpu_freq);
	static bool get_fallback_cpu_freq();
	static bool set_selected_page(TabNumber selected_page);
	static bool set_selected_page_next();
	static bool set_selected_page_prev();
	static TabNumber get_selected_page();
	static bool set_selected_type(uint8_t num_cpu_types, uint8_t selected_type);
	static uint8_t get_selected_type();
	static bool set_selected_test(uint8_t selected_test);
	static uint8_t get_selected_test();
	static bool set_selected_stick(uint8_t selected_stick, uint8_t num_sticks);
	static uint8_t get_selected_stick();
	static bool set_selected_gpu(uint8_t selected_gpu, uint8_t num_gpus);
	static uint8_t get_selected_gpu();
	static bool set_selected_core(uint16_t selected_core, uint16_t num_cpu_cores);
	static uint16_t get_selected_core();
	static bool set_output_type(uint16_t output_type);
	static bool output_type_is(uint16_t output_type);
	static uint16_t get_output_type();
	static bool set_refr_time(uint16_t refr_time);
	static uint16_t get_refr_time();
	static bool set_keymap(OptKeymap keymap);
	static OptKeymap get_keymap();
	static bool set_temp_unit(OptTempUnit temp_unit);
	static OptTempUnit get_temp_unit();

private:
	static inline bool cpuid_decimal     = false;
	static inline bool color             = true;
	static inline bool issue             = false;
	static inline bool with_daemon       = false;
	static inline bool debug_database    = false;
	static inline bool fallback_cpu_temp = false;
	static inline bool fallback_cpu_volt = false;
	static inline bool fallback_cpu_freq = false;

	static inline uint8_t selected_type  = 0;
	static inline uint8_t selected_test  = 0;
	static inline uint8_t selected_stick = 0;
	static inline uint8_t selected_gpu   = 0;

	static inline uint16_t selected_core = 0;
	static inline uint16_t output_type   = 0;
	static inline uint16_t refr_time     = 1;

	static inline TabNumber selected_page = TAB_CPU;
	static inline OptKeymap keymap        = ARROWS;
	static inline OptTempUnit temp_unit   = CELSIUS;
	Options() = delete;
	~Options() = delete;
};


#endif /* _OPTIONS_HPP_ */
