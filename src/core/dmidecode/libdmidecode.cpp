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
* FILE core/dmidecode/libdmidecode.cpp
*/

#include <cstring>
#include <list>
#include "libdmidecode.h"
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "../internal.hpp"
#include "../../daemon/daemon.h"


/* Call Dmidecode through CPU-X but do nothing else */
int run_dmidecode(std::list<std::string> &args)
{
	char **argv = transform_string_list_to_char_array("dmidecode (built-in with CPU-X)", args);

	return dmidecode(args.size() + 1, argv, Logger::get_verbosity() > LOG_VERBOSE, NULL);
}

/* Elements provided by dmidecode */
int call_dmidecode(Data &data)
{
	const DaemonCommand cmd = DMIDECODE;
	DmidecodeData msg;
	DmidecodeMemoryData msg_memory;
	bool need_sep = false;

	MSG_VERBOSE("%s", _("Calling dmidecode"));
	SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));

	RECEIVE_DATA(&data.socket_fd, &msg.ret, sizeof(int));
	if(msg.ret)
		return 1;

	/* Tab CPU */
	RECEIVE_DATA(&data.socket_fd, &msg.processor, sizeof(DmidecodeCPUData));
	for(auto& cpu_type : data.cpu.cpu_types)
		cpu_type.processor.package.value = msg.processor.cpu_package;
	if(data.cpu.clocks.bus_freq == 0.0)
		data.cpu.clocks.set_bus_freq(double(msg.processor.bus_freq));

	/* Tab Motherboard */
	RECEIVE_DATA(&data.socket_fd, &msg.mb, sizeof(DmidecodeMBData));
	data.motherboard.board.manufacturer.value = msg.mb.manufacturer;
	data.motherboard.board.model.value        = msg.mb.model;
	data.motherboard.board.revision.value     = msg.mb.revision;
	RECEIVE_DATA(&data.socket_fd, &msg.bios, sizeof(DmidecodeBiosData));
	data.motherboard.bios.brand.value         = msg.bios.brand;
	data.motherboard.bios.version.value       = msg.bios.version;
	data.motherboard.bios.date.value          = msg.bios.date;
	data.motherboard.bios.romsize.value       = string_set_size_unit(msg.bios.romsize);

	/* Tab RAM */
	RECEIVE_DATA(&data.socket_fd, &msg.stick_count, sizeof(uint8_t));
	for(uint8_t i = 0; i < msg.stick_count; i++)
	{
		RECEIVE_DATA(&data.socket_fd, &msg_memory, sizeof(DmidecodeMemoryData));
		data.memory.grow_sticks_vector();
		data.memory.sticks[i].manufacturer.value   = msg_memory.manufacturer;
		data.memory.sticks[i].part_number.value    = msg_memory.part_number;
		data.memory.sticks[i].type.value           = msg_memory.type;
		data.memory.sticks[i].type_detail.value    = msg_memory.type_detail;
		data.memory.sticks[i].device_locator.value = msg_memory.device_locator;
		data.memory.sticks[i].bank_locator.value   = msg_memory.bank_locator;
		data.memory.sticks[i].size.value           = string_set_size_unit(msg_memory.size);
		data.memory.sticks[i].rank.value           = msg_memory.rank;
		need_sep = false;
		if(std::strlen(msg_memory.speed_configured) > 0)
		{
			data.memory.sticks[i].speed.value = string_format(_("%s (configured)"), msg_memory.speed_configured);
			need_sep = true;
		}
		if(std::strlen(msg_memory.speed_maximum) > 0)
		{
			if(need_sep)
				data.memory.sticks[i].speed.value += " / ";
			data.memory.sticks[i].speed.value += string_format(_("%s (max)"), msg_memory.speed_maximum);
		}
		need_sep = false;
		if(std::strlen(msg_memory.voltage_minimum) > 0)
		{
			data.memory.sticks[i].voltage.value = string_format(_("%s (min)"), msg_memory.voltage_minimum);
			need_sep = true;
		}
		if(std::strlen(msg_memory.voltage_configured) > 0)
		{
			if(need_sep)
				data.memory.sticks[i].voltage.value += " / ";
			data.memory.sticks[i].voltage.value += string_format(_("%s (configured)"), msg_memory.voltage_configured);
			need_sep = true;
		}
		if(std::strlen(msg_memory.voltage_maximum) > 0)
		{
			if(need_sep)
				data.memory.sticks[i].voltage.value += " / ";
			data.memory.sticks[i].voltage.value += string_format(_("%s (max)"), msg_memory.voltage_maximum);
		}
	}

	return 0;
}
