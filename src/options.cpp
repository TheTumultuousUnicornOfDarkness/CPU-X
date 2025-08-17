/****************************************************************************
*    Copyright © 2014-2025 The Tumultuous Unicorn Of Darkness
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
#include "data.hpp"

#if HAS_BANDWIDTH
# include "core/bandwidth/libbandwidth.h"
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

void Options::set_fallback_cpu_mult(bool fallback_cpu_mult)
{
	Options::fallback_cpu_mult = fallback_cpu_mult;
}

bool Options::get_fallback_cpu_mult()
{
	return Options::fallback_cpu_mult;
}

void Options::init_page_visibility()
{
	Options::page_visible.fill(true);
}

void Options::set_page_visibility(TabNumber page, bool visible)
{
	Options::page_visible[page] = visible;
}

void Options::set_page_visibility_auto(Data &data)
{
	Options::set_page_visibility(TAB_CACHES,   (data.caches.get_selected_cpu_type().caches.size() > 0));
	Options::set_page_visibility(TAB_MEMORY,   (data.memory.sticks.size()                         > 0));
	Options::set_page_visibility(TAB_GRAPHICS, (data.graphics.cards.size()                        > 0));
}

bool Options::get_page_visibility(TabNumber page)
{
	return Options::page_visible[page];
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
			if(Options::get_page_visibility(selected_page))
			{
				Options::selected_page = selected_page;
				return true;
			}
			else
			{
				Options::selected_page = TAB_CPU;
				MSG_WARNING(_("Selected tab (%u) is not visible"), selected_page);
				return false;
			}
		default:
			Options::selected_page = TAB_CPU;
			MSG_WARNING(_("Selected tab (%u) is not a valid number (%u is the maximum)"), selected_page, TAB_ABOUT);
			return false;
	}
}

bool Options::set_selected_page_next()
{
	if(Options::selected_page < TabNumber(LAST_TAB_NUMBER - 1))
	{
		do
		{
			Options::selected_page = TabNumber(Options::selected_page + 1);
		} while((Options::selected_page < TabNumber(LAST_TAB_NUMBER - 1)) && !Options::get_page_visibility(Options::selected_page));
		return true;
	}
	else
		return false;
}

bool Options::set_selected_page_prev()
{
	if(Options::selected_page > TabNumber(0))
	{
		do
		{
			Options::selected_page = TabNumber(Options::selected_page - 1);
		} while((Options::selected_page > TabNumber(0)) && !Options::get_page_visibility(Options::selected_page));
		return true;
	}
	else
		return false;
}

TabNumber Options::get_selected_page()
{
	return Options::selected_page;
}

bool Options::check_selected_type_valid(uint8_t selected_type)
{
	if(selected_type < Options::selected_type.second)
		return true;
	else
	{
		Options::selected_type.first = 0;
		MSG_WARNING(_("Selected CPU type (%u) is not a valid number (%u is the maximum for this CPU)"), selected_type, Options::selected_type.second - 1);
		return false;
	}
}

void Options::set_num_types(uint8_t num_types)
{
	Options::selected_type.second = num_types;

	/* selected_core allowed values depend of cpu_type */
	Options::selected_core.second.resize(num_types);
	for(auto& cpu_type_num_cores : Options::selected_core.second)
		cpu_type_num_cores = -1;
}

bool Options::set_selected_type(uint8_t selected_type)
{
	if(Options::check_selected_type_valid(selected_type))
	{
		Options::selected_type.first = selected_type;
		Options::set_selected_core(0); // Reset selected core number to a safe value
		return true;
	}
	else
		return false;
}

uint8_t Options::get_selected_type()
{
	Options::check_selected_type_valid(Options::selected_type.first);
	return Options::selected_type.first;
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

bool Options::check_selected_stick_valid(uint8_t selected_stick)
{
	if(selected_stick < Options::selected_stick.second)
		return true;
	else
	{
		Options::selected_stick.first = 0;
		MSG_WARNING(_("Selected RAM stick (%u) is not a valid number (%u is the maximum for this system)"), selected_stick, Options::selected_stick.second - 1);
		return false;
	}
}

void Options::set_num_sticks(uint8_t num_sticks)
{
	Options::selected_stick.second = num_sticks;
}

bool Options::set_selected_stick(uint8_t selected_stick)
{
	if(Options::check_selected_stick_valid(selected_stick))
	{
		Options::selected_stick.first = selected_stick;
		return true;
	}
	else
		return false;
}

uint8_t Options::get_selected_stick()
{
	Options::check_selected_stick_valid(Options::selected_stick.first);
	return Options::selected_stick.first;
}

bool Options::check_selected_gpu_valid(uint8_t selected_gpu)
{
	if(selected_gpu < Options::selected_gpu.second)
		return true;
	else
	{
		Options::selected_gpu.first = 0;
		MSG_WARNING(_("Selected graphic card (%u) is not a valid number (%u is the maximum for this system)"), selected_gpu, Options::selected_gpu.second - 1);
		return false;
	}
}

void Options::set_num_gpus(uint8_t num_gpus)
{
	Options::selected_gpu.second = num_gpus;
}

bool Options::set_selected_gpu(uint8_t selected_gpu)
{
	if(Options::check_selected_gpu_valid(selected_gpu))
	{
		Options::selected_gpu.first = selected_gpu;
		return true;
	}
	else
		return false;
}

uint8_t Options::get_selected_gpu()
{
	Options::check_selected_gpu_valid(Options::selected_gpu.first);
	return Options::selected_gpu.first;
}

bool Options::check_selected_core_valid(uint16_t selected_core)
{
	/* During early load (e.g. load_settings()), the vector is not yet initialized, but we need to set the value anyway. */
	if(Options::selected_core.second.size() == 0)
		return true;
	else if(selected_core < Options::selected_core.second.at(Options::get_selected_type()))
		return true;
	else
	{
		Options::selected_core.first = 0;
		MSG_WARNING(_("Selected CPU core (%u) is not a valid number (%u is the maximum for this type of core)"), selected_core, Options::selected_core.second.at(Options::get_selected_type()) - 1);
		return false;
	}
}

uint16_t Options::get_selected_core_id()
{
	uint16_t core_offset = 0;

	/* Do nothing if selected_core vector is not yet initialized */
	if(Options::selected_core.second.size() == 0)
		return 0;

	/* Calculate offset for CPU core ID */
	for(auto core_type = 0; core_type < Options::get_selected_type(); core_type++)
		core_offset += Options::selected_core.second.at(core_type);

	return core_offset + Options::get_selected_core();
}

void Options::set_num_cores(uint8_t selected_type, uint16_t num_cores)
{
	if(Options::selected_core.second.size() > 0)
		Options::selected_core.second.at(selected_type) = num_cores;
}

bool Options::set_selected_core(uint16_t selected_core)
{
	if(Options::check_selected_core_valid(selected_core))
	{
		Options::selected_core.first = selected_core;
		if(!set_cpu_affinity(Options::get_selected_core_id()))
			MSG_ERROR(_("failed to change CPU affinitiy to core %u"), Options::get_selected_core_id());
		return true;
	}
	else
		return false;
}

uint16_t Options::get_selected_core()
{
	Options::check_selected_core_valid(Options::selected_core.first);
	return Options::selected_core.first;
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
