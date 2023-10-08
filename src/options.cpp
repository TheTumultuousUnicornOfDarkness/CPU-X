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
* FILE options.cpp
*/

#include <cstdint>
#include "util.hpp"
#include "options.hpp"

#if HAS_BANDWIDTH
# include "bandwidth/libbandwidth.h"
#endif


void Options::set_cpuid_decimal(bool cpuid_decimal)
{
	Options::cpuid_decimal = cpuid_decimal;
}

bool Options::get_cpuid_decimal()
{
	return Options::cpuid_decimal;
}

void Options::set_color(bool color)
{
	Options::color = color;
}

bool Options::get_color()
{
	return Options::color;
}

void Options::set_issue(bool issue)
{
	Options::issue = issue;
}

bool Options::get_issue()
{
	return Options::issue;
}

void Options::set_with_daemon(bool with_daemon)
{
	Options::with_daemon = with_daemon;
}

bool Options::get_with_daemon()
{
	return Options::with_daemon;
}

void Options::set_debug_database(bool debug_database)
{
	Options::debug_database = debug_database;
}

bool Options::get_debug_database()
{
	return Options::debug_database;
}

void Options::set_fallback_cpu_temp(bool fallback_cpu_temp)
{
	Options::fallback_cpu_temp = fallback_cpu_temp;
}

bool Options::get_fallback_cpu_temp()
{
	return Options::fallback_cpu_temp;
}

void Options::set_fallback_cpu_volt(bool fallback_cpu_volt)
{
	Options::fallback_cpu_volt = fallback_cpu_volt;
}

bool Options::get_fallback_cpu_volt()
{
	return Options::fallback_cpu_volt;
}

void Options::set_fallback_cpu_freq(bool fallback_cpu_freq)
{
	Options::fallback_cpu_freq = fallback_cpu_freq;
}

bool Options::get_fallback_cpu_freq()
{
	return Options::fallback_cpu_freq;
}

bool Options::set_selected_type(uint8_t selected_type, uint8_t num_cpu_types)
{
	Options::set_selected_core(0, -1);
	if(selected_type < num_cpu_types)
	{
		Options::selected_type = selected_type;
		return true;
	}
	else
	{
		Options::selected_type = 0;
		MSG_WARNING(_("Selected CPU type (%u) is not a valid number (%u is the maximum for this CPU)"), selected_type, num_cpu_types - 1);
		return false;
	}
}

uint8_t Options::get_selected_type()
{
	return Options::selected_type;
}

bool Options::set_selected_test([[maybe_unused]] uint8_t selected_test)
{
#if HAS_BANDWIDTH
	if(selected_test < BANDWIDTH_LAST_TEST)
	{
		Options::selected_test = selected_test;
		return true;
	}
	else
	{
		Options::selected_test = 0;
		MSG_WARNING(_("Selected bandwidth test (%u) is not a valid number (%u is the maximum for this system)"), selected_test, BANDWIDTH_LAST_TEST - 1);
		return false;
	}
#else
	return false;
#endif /* HAS_BANDWIDTH */
}

uint8_t Options::get_selected_test()
{
	return Options::selected_test;
}

bool Options::set_selected_stick(uint8_t selected_stick, uint8_t num_sticks)
{
	if(selected_stick < num_sticks)
	{
		Options::selected_stick = selected_stick;
		return true;
	}
	else
	{
		Options::selected_stick = 0;
		MSG_WARNING(_("Selected RAM stick (%u) is not a valid number (%u is the maximum for this system)"), selected_stick, num_sticks - 1);
		return false;
	}
}

uint8_t Options::get_selected_stick()
{
	return Options::selected_stick;
}

bool Options::set_selected_gpu(uint8_t selected_gpu, uint8_t num_gpus)
{
	if(selected_gpu < num_gpus)
	{
		Options::selected_gpu = selected_gpu;
		return true;
	}
	else
	{
		Options::selected_gpu = 0;
		MSG_WARNING(_("Selected graphic card (%u) is not a valid number (%u is the maximum for this system)"), selected_gpu, num_gpus - 1);
		return false;
	}
}

uint8_t Options::get_selected_gpu()
{
	return Options::selected_gpu;
}

bool Options::set_selected_core(uint16_t selected_core, uint16_t num_cpu_cores)
{
	if(selected_core < num_cpu_cores)
	{
		Options::selected_core = selected_core;
		if(!set_cpu_affinity(selected_core))
			MSG_ERROR(_("failed to change CPU affinitiy to core %u"), selected_core);
		return true;
	}
	else
	{
		Options::selected_core = 0;
		MSG_WARNING(_("Selected CPU core (%u) is not a valid number (%u is the maximum for this type of core)"), selected_core, num_cpu_cores - 1);
		return false;
	}
}

uint16_t Options::get_selected_core()
{
	return Options::selected_core;
}

bool Options::set_output_type(uint16_t output_type)
{
	switch(output_type)
	{
		case OUT_GTK:
		case OUT_NCURSES:
		case OUT_DUMP:
		case OUT_NO_CPUX:
		case OUT_DMIDECODE:
		case OUT_BANDWIDTH:
			Options::output_type = output_type;
			return true;
		default:
			Options::output_type = OUT_DUMP;
			MSG_ERROR("Options::set_output_type() called with 0x%X", output_type);
			return false;
	}
}

bool Options::output_type_is(uint16_t output_type)
{
	return Options::output_type == output_type;
}

uint16_t Options::get_output_type()
{
	return Options::output_type;
}

bool Options::set_refr_time(uint16_t refr_time)
{
	if(refr_time >= 1)
	{
		Options::refr_time = refr_time;
		return true;
	}
	else
	{
		Options::refr_time = 1;
		return false;
	}
}

uint16_t Options::get_refr_time()
{
	return Options::refr_time;
}

bool Options::set_selected_page(TabNumber selected_page)
{
	switch(selected_page)
	{
		case TAB_CPU:
		case TAB_CACHES:
		case TAB_MOTHERBOARD:
		case TAB_MEMORY:
		case TAB_SYSTEM:
		case TAB_GRAPHICS:
		case TAB_BENCH:
		case TAB_ABOUT:
			Options::selected_page = selected_page;
			return true;
		default:
			Options::selected_page = TAB_CPU;
			MSG_WARNING(_("Selected tab (%u) is not a valid number (%u is the maximum)"), selected_page, TAB_ABOUT);
			return false;
	}
}

bool Options::set_selected_page_next()
{
	if(Options::selected_page < TAB_ABOUT)
	{
		Options::selected_page = TabNumber(Options::selected_page + 1);
		return true;
	}
	else
		return false;
}

bool Options::set_selected_page_prev()
{
	if(Options::selected_page > TAB_CPU)
	{
		Options::selected_page = TabNumber(Options::selected_page - 1);
		return true;
	}
	else
		return false;
}

TabNumber Options::get_selected_page()
{
	return Options::selected_page;
}

bool Options::set_keymap(OptKeymap keymap)
{
	switch(keymap)
	{
		case ARROWS:
		case EMACS:
		case INVERTED_T:
		case VIM:
			Options::keymap = keymap;
			return true;
		default:
			Options::keymap = ARROWS;
			MSG_ERROR("Options::set_keymap() called with %i", keymap);
			return false;
	}
}

OptKeymap Options::get_keymap()
{
	return Options::keymap;
}

bool Options::set_temp_unit(OptTempUnit temp_unit)
{
	switch(temp_unit)
	{
		case CELSIUS:
		case FAHRENHEIT:
		case KELVIN:
		case RANKINE:
			Options::temp_unit = temp_unit;
			return true;
		default:
			Options::temp_unit = CELSIUS;
			MSG_ERROR("Options::set_temp_unit() called with %i", temp_unit);
			return false;
	}
}

OptTempUnit Options::get_temp_unit()
{
	return Options::temp_unit;
}
