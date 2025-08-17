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
* FILE core/core.cpp
*/

#include <sys/utsname.h>
#include <cmath>
#include <clocale>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <regex>
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "core/core.hpp"
#include "core/internal.hpp"
#include "daemon/daemon.h"

#ifndef __linux__
# include <sys/sysctl.h>
#endif

namespace fs = std::filesystem;


/************************* Private functions *************************/

/* Avoid to re-run a function if an error was occurred in previous call */
static int err_func(int (*func)(Data &), Data &data)
{
	static std::unordered_map<int (*)(Data &), int> registered;

	MSG_DEBUG("err_func: func=%p ==> ret=%i", func, registered[func]);
	if(registered[func] == 0)
		registered[func] = func(data);

	return registered[func];
}

/* Calculate total CPU usage */
#define BEFORE data.cpu.clocks.cpu_time_stat
static int cpu_usage(Data &data)
{
	double loadavg;
	enum StatType { USER, NICE, SYSTEM, INTR, IDLE, LASTSTAT };
	std::vector<long> after(LASTSTAT);

	MSG_VERBOSE("%s", _("Calculating CPU usage"));
	BEFORE.resize(LASTSTAT);

#ifdef __linux__
	FILE *fp;

	if((fp = fopen("/proc/stat","r")) == NULL)
		return 1;
	fscanf(fp,"%*s %li %li %li %li %*s %*s %*s %*s %*s %*s", &after[USER], &after[NICE], &after[SYSTEM], &after[IDLE]);
	fclose(fp);
#else /* __linux__ */
	size_t len = sizeof(long) * after.size();

	if(sysctlbyname("kern.cp_time", after.data(), &len, NULL, 0))
		return 1;
#endif /* __linux__ */

	loadavg = double((after[USER]  + after[NICE]  + after[SYSTEM]  + after[INTR]) -
	                 (BEFORE[USER] + BEFORE[NICE] + BEFORE[SYSTEM] + BEFORE[INTR])) /
	          double((after[USER]  + after[NICE]  + after[SYSTEM]  + after[INTR]  + after[IDLE]) -
	                 (BEFORE[USER] + BEFORE[NICE] + BEFORE[SYSTEM] + BEFORE[INTR] + BEFORE[IDLE]));
	data.cpu.clocks.usage.value = string_format("%6.2f %%", loadavg * 100);
	BEFORE.swap(after);

	return 0;
}
#undef BEFORE

/* Read EFI variables */
static int efi_readvar(Data &data)
{
	int err = 0;
	std::string pk_subject, pk_issuer;

	if(!command_exists("efi-readvar"))
		return 1;

	/* Get Platform Key (PK) X509 information */
	err += popen_to_str(pk_subject, "efi-readvar -v PK | grep -A1 Subject: | tail -n-1 | cut -d= -f2");
	err += popen_to_str(pk_issuer,  "efi-readvar -v PK | grep -A1 Issuer:  | tail -n-1 | cut -d= -f2");

	if(!err)
		data.motherboard.bios.efi_pk.value = string_format(_("%s (subject) / %s (issuer)"), pk_subject.c_str(), pk_issuer.c_str());

	return err;
}

/* Satic elements for System tab, OS specific */
static int system_static(Data &data)
{
	int err = 0;
	struct utsname name;

	MSG_VERBOSE("%s", _("Identifying running system"));
	err = uname(&name);
	if(err)
		MSG_ERRNO("%s", _("failed to identify running system"));
	else
		data.system.os.hostname.value = Options::get_issue() ? "sensitive data" : name.nodename; /* Hostname label */

#ifdef __linux__
	/* Name label */
	std::string line;
	std::ifstream stream("/etc/os-release");
	std::regex regex("^PRETTY_NAME=\"(.*?)\"$");
	std::smatch match;

	while(std::getline(stream, line))
	{
		if(std::regex_search(line, match, regex))
		{
			data.system.os.name.value = match[1].str();
			break;
		}
	}

	/* Kernel label */
	if(!err)
		data.system.os.kernel.value = string_format("%s %s", name.sysname, name.release);

#else /* __linux__ */
	size_t name_index = std::string::npos;

	if(!err)
	{
		/* Name label */
		data.system.os.name.value = string_format("%s %s", name.sysname, name.release);

		/* Kernel label */
		data.system.os.kernel.value = string_format("%s", name.version);
		name_index = data.system.os.kernel.value.find(data.system.os.name.value);
		if(name_index != std::string::npos)
			data.system.os.kernel.value.erase(name_index, data.system.os.name.value.length() + 1);
	}

#endif /* __linux__ */

	return err;
}


/************************* Fallback functions (static) *************************/

/* Get minimum and maximum CPU multipliers */
static int cputab_multipliers_fallback(Data &data) noexcept
{
	int err = 0;

	if(data.cpu.clocks.bus_freq <= 0.0)
		return 1;

#ifdef __linux__
	std::string min_freq_str, max_freq_str;
	const uint16_t current_core_id = Options::get_selected_core_id();

	MSG_VERBOSE("%s", _("Calculating CPU multipliers in fallback mode"));
	/* Minimum multiplier */
	if(!(err = fopen_to_str(min_freq_str, "%s%i/cpufreq/cpuinfo_min_freq", SYS_CPU, current_core_id)))
		data.cpu.clocks.cpu_min_mult = (std::stod(min_freq_str) / 1e3) / data.cpu.clocks.bus_freq;

	/* Maximum multiplier */
	if(!(err = fopen_to_str(max_freq_str, "%s%i/cpufreq/cpuinfo_max_freq", SYS_CPU, current_core_id)))
		data.cpu.clocks.cpu_max_mult = (std::stod(max_freq_str) / 1e3) / data.cpu.clocks.bus_freq;

#endif /* __linux__ */

	return err;
}

/* Retrieve missing Motherboard data if run as regular user */
static int motherboardtab_fallback([[maybe_unused]] Data &data)
{
	int err = 0;

#ifdef __linux__
	if(!fs::is_directory(SYS_DMI))
	{
		Options::set_page_visibility(TAB_MOTHERBOARD, false);
		return 1;
	}

	MSG_VERBOSE("%s", _("Retrieving motherboard information in fallback mode"));
	/* Frame Board */
	err += fopen_to_str(data.motherboard.board.manufacturer.value, "%s/%s", SYS_DMI, "board_vendor");
	err += fopen_to_str(data.motherboard.board.model.value,        "%s/%s", SYS_DMI, "board_name");
	err += fopen_to_str(data.motherboard.board.revision.value,     "%s/%s", SYS_DMI, "board_version");

	/* Frame Bios */
	err += fopen_to_str(data.motherboard.bios.brand.value,   "%s/%s", SYS_DMI, "bios_vendor");
	err += fopen_to_str(data.motherboard.bios.version.value, "%s/%s", SYS_DMI, "bios_version");
	err += fopen_to_str(data.motherboard.bios.date.value,    "%s/%s", SYS_DMI, "bios_date");
#else
	Options::set_page_visibility(TAB_MOTHERBOARD, false);
	return 1;
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve motherboard information (fallback mode)"));

	return err;
}

/* Retrieve static data if other functions failed */
static int fallback_mode_static(Data &data)
{
	int err = 0;

#if HAS_LIBCPUID
	const auto& cpu_type = data.cpu.get_selected_cpu_type();
	if((cpu_type.processor.package.value.empty()                                      ||
	   (cpu_type.processor.package.value.find("CPU")            != std::string::npos) ||
	   (cpu_type.processor.package.value.find("Microprocessor") != std::string::npos)))
		err += cputab_package_fallback(data);
#endif /* HAS_LIBCPUID */

	static bool use_fallback_mult = Options::get_fallback_cpu_mult();
	if((data.cpu.clocks.cpu_min_mult <= 0.0) || (data.cpu.clocks.cpu_max_mult <= 0.0) || use_fallback_mult)
		err += cputab_multipliers_fallback(data);

	if(data.motherboard.board.manufacturer.value.empty() ||
	   data.motherboard.board.model.value.empty()        ||
	   data.motherboard.board.revision.value.empty()     ||
	   data.motherboard.bios.brand.value.empty()         ||
	   data.motherboard.bios.version.value.empty()       ||
	   data.motherboard.bios.date.value.empty())
		err += motherboardtab_fallback(data);

	return err;
}


/************************* Fallback functions (dynamic) *************************/

/* Retrieve CPU temperature if run as regular user */
static int cputab_temp_fallback(Data &data)
{
	int err = 0;
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const uint16_t current_core_id = Options::get_selected_core_id();

	MSG_VERBOSE("%s", _("Retrieving CPU temperature in fallback mode"));
#ifdef __linux__
	std::string temperature;
# if HAS_LIBCPUID
	static bool module_loaded = false;
	/* Load kernel modules */
	if(!module_loaded && !data.cpu.sensors_module_name.empty())
		module_loaded = !load_module(data.cpu.sensors_module_name.c_str(), &data.socket_fd);
# endif /* HAS_LIBCPUID */

	/* Get sensor path */
	if(cpu_type.processor.path_cpu_temperature.empty())
		cpu_type.processor.path_cpu_temperature = get_sensor_path_cpu_temperature_driver(current_core_id);
	if(cpu_type.processor.path_cpu_temperature.empty())
		cpu_type.processor.path_cpu_temperature = get_sensor_path_cpu_temperature_generic();

	/* Get temperature */
	if(!cpu_type.processor.path_cpu_temperature.empty())
	{
		if(!(err = fopen_to_str(temperature, "%s", cpu_type.processor.path_cpu_temperature.c_str())))
		{
			const double tmp = std::stod(temperature);
			if(tmp > 0.0)
				cpu_type.processor.temperature.value = string_with_temperature_unit(tmp / 1e3);
		}
	}
	else if(command_exists("vcgencmd"))
	{
		if(!(err = popen_to_str(temperature, "vcgencmd measure_temp | cut -d= -f2 | cut -d\"'\" -f1")))
		{
			const double tmp = std::stod(temperature);
			if(tmp > 0.0)
				cpu_type.processor.temperature.value = string_with_temperature_unit(tmp);
		}
	}

#else /* __linux__ */
	/* Tested on FreeBSD 12: https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/issues/121#issuecomment-575985765 */
	int temperature;
	size_t len = sizeof(temperature);
	char name[MAXSTR];

	snprintf(name, MAXSTR, "dev.cpu.%i.temperature", current_core_id);
	if(!(err = sysctlbyname(name, &temperature, &len, NULL, 0)))
		cpu_type.processor.temperature.value = string_with_temperature_unit(double(temperature - 2731) / 10.0); // decikelvins
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve CPU temperature (fallback mode)"));

	return err;
}

/* Retrieve CPU voltage if run as regular user */
static int cputab_volt_fallback([[maybe_unused]] Data &data)
{
	int err = 0;

#ifdef __linux__
	std::string voltage;
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const uint16_t current_core_id = Options::get_selected_core_id();

	MSG_VERBOSE("%s", _("Retrieving CPU voltage in fallback mode"));
	/* Get sensor path */
	if(cpu_type.processor.path_cpu_voltage.empty())
		cpu_type.processor.path_cpu_voltage = get_sensor_path_cpu_voltage(current_core_id);

	/* Get voltage */
	if(!cpu_type.processor.path_cpu_voltage.empty())
	{
		if(!(err = fopen_to_str(voltage, "%s", cpu_type.processor.path_cpu_voltage.c_str())))
			cpu_type.processor.voltage.value = string_format("%.3f V", std::stod(voltage) / 1e3);
	}
	else if(command_exists("vcgencmd"))
	{
		if(!(err = popen_to_str(voltage, "vcgencmd measure_volts core | cut -d= -f2")))
			cpu_type.processor.voltage.value = voltage;
	}
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve CPU voltage (fallback mode)"));

	return err;
}

/* Retrieve CPU frequency if Libcpuid is missing */
static int cputab_freq_fallback([[maybe_unused]] Data &data)
{
	int err = 0;

#ifdef __linux__
	std::string freq;
	const uint16_t current_core_id = Options::get_selected_core_id();

	MSG_VERBOSE("%s", _("Retrieving CPU frequency in fallback mode"));
	if(!(err = fopen_to_str(freq, "%s%i/cpufreq/scaling_cur_freq", SYS_CPU, current_core_id)))
		data.cpu.clocks.set_cpu_freq(std::round(std::stod(freq) / 1e3));
	else if(command_exists("vcgencmd"))
	{
		if(!(err = popen_to_str(freq, "vcgencmd measure_clock arm | cut -d= -f2")))
			data.cpu.clocks.set_cpu_freq(std::round(std::stod(freq) / 1e6));
	}
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve CPU frequency (fallback mode)"));

	return err;
}

/* Retrieve dynamic data if other functions failed */
static int fallback_mode_dynamic(Data &data)
{
	int err = 0;
	auto& cpu_type = data.cpu.get_selected_cpu_type();

	/* CPU temperature */
	static bool use_fallback_temp = Options::get_fallback_cpu_temp();
	if((cpu_type.processor.temperature.value.empty()) || use_fallback_temp)
	{
		use_fallback_temp = true;
		err += err_func(cputab_temp_fallback, data);
	}

	/* CPU voltage */
	static bool use_fallback_volt = Options::get_fallback_cpu_volt();
	if((cpu_type.processor.voltage.value.empty()) || use_fallback_volt)
	{
		use_fallback_volt = true;
		err += err_func(cputab_volt_fallback, data);
	}

	/* CPU speed */
	static bool use_fallback_freq = Options::get_fallback_cpu_freq();
	if(data.cpu.clocks.core_speed.value.empty() || use_fallback_freq)
	{
		use_fallback_freq = true;
		err += err_func(cputab_freq_fallback, data);
	}

	return err;
}


/************************* Public functions *************************/

/* Fill labels by calling below functions */
int fill_labels(Data &data)
{
	int err = 0;

#if HAS_LIBCPUID
	err += call_libcpuid_static(data);
	if(DAEMON_UP) err += call_libcpuid_msr_static(data);
#endif

#if HAS_DMIDECODE
	if(DAEMON_UP) err += call_dmidecode(data);
#endif

#if HAS_LIBPCI
	err += find_devices(data);
#endif

	err += efi_readvar         (data);
	err += system_static       (data);
	err += fallback_mode_static(data);

	init_benchmarks(data);

	/* Call do_refresh() once to get dynamic values */
	for(TabNumber tab_number = TAB_CPU; tab_number < TAB_ABOUT; tab_number = TabNumber(tab_number + 1))
		err += do_refresh(data, tab_number);

	return err;
}

/* Refresh some labels */
int do_refresh(Data &data, TabNumber tab_number)
{
	int err = 0;

	switch(tab_number)
	{
		case TAB_CPU:
#if HAS_LIBCPUID
			err += err_func(call_libcpuid_dynamic, data);
			if(DAEMON_UP) err += err_func(call_libcpuid_msr_dynamic, data);
#endif /* HAS_LIBCPUID */
			err += err_func(cpu_usage, data);
			err += fallback_mode_dynamic(data);
			err += data.cpu.clocks.set_cpu_multiplier();
			break;
		case TAB_CACHES:
#if HAS_BANDWIDTH
			err += err_func(call_bandwidth, data);
#endif /* HAS_BANDWIDTH */
			break;
		case TAB_SYSTEM:
#if HAS_LIBSYSTEM
			err += err_func(system_dynamic, data);
#endif /* HAS_LIBSYSTEM */
			break;
		case TAB_GRAPHICS:
#if HAS_LIBPCI
			err += err_func(gpu_monitoring, data);
#endif /* HAS_LIBPCI */
			break;
		case TAB_BENCH:
			err += err_func(benchmark_status, data);
			break;
		default:
			break;
	}

	return err;
}
