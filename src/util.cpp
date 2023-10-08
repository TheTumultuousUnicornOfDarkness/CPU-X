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
* FILE util.cpp
*/

#include <unistd.h>
#include <cstring>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <filesystem>
#include <regex>
#include <optional>
#include "options.hpp"
#include "util.hpp"
#include "daemon.h"

namespace fs = std::filesystem;


/* PrefixUnit class */

PrefixUnit::PrefixUnit() : init(false)
{
}

void PrefixUnit::find_best_prefix(uint64_t value, PrefixUnit::Multipliers multiplier, bool use_si_prefixes)
{
	unsigned prefix_index, divisor_index;
	struct Table { PrefixUnit::Multipliers multiplier; char *prefix; uint64_t divisor; };

	const std::array<struct Table, PrefixUnit::Multipliers::MULT_LAST> si_prefixes
	{{
		{ MULT_NONE,  UNIT_B,  static_cast<uint64_t>(1)    },
		{ MULT_K,     UNIT_KB, static_cast<uint64_t>(1e3)  },
		{ MULT_M,     UNIT_MB, static_cast<uint64_t>(1e6)  },
		{ MULT_G,     UNIT_GB, static_cast<uint64_t>(1e9)  },
		{ MULT_T,     UNIT_TB, static_cast<uint64_t>(1e12) },
	}};
	const std::array<struct Table, PrefixUnit::Multipliers::MULT_LAST> binary_prefixes
	{{
		{ MULT_NONE,  UNIT_B,  static_cast<uint64_t>(1)    },
		{ MULT_K,     UNIT_KB, static_cast<uint64_t>(1e3)  },
		{ MULT_M,     UNIT_MB, static_cast<uint64_t>(1e6)  },
		{ MULT_G,     UNIT_GB, static_cast<uint64_t>(1e9)  },
		{ MULT_T,     UNIT_TB, static_cast<uint64_t>(1e12) },
	}};
	const std::array prefixes = use_si_prefixes ? si_prefixes : binary_prefixes;

	/* Find current multiplier */
	for(prefix_index = 0; (prefixes[prefix_index].multiplier != multiplier) && (prefix_index < prefixes.size()); prefix_index++);
	if(prefix_index >= prefixes.size())
	{
		/* Due to the loop over an enum, this case is not possible */
		MSG_ERROR("multiplier=%i, value=%llu", multiplier, value);
		return;
	}
	this->init    = true;
	this->prefix  = prefixes[prefix_index].prefix;
	this->divisor = prefixes[0].divisor;

	/* Find new prefix and new divisor */
	for(prefix_index = prefix_index + 1, divisor_index = 1; (prefix_index < prefixes.size()) && (divisor_index < prefixes.size()) && ((value / prefixes[divisor_index].divisor) > 0); prefix_index++, divisor_index++)
	{
		this->prefix  = prefixes[prefix_index].prefix;
		this->divisor = prefixes[divisor_index].divisor;
	}
}

void PrefixUnit::find_best_si_prefix(uint64_t value, Multipliers multiplier)
{
	find_best_prefix(value, multiplier, true);
}

void PrefixUnit::find_best_binary_prefix(uint64_t value, Multipliers multiplier)
{
	find_best_prefix(value, multiplier, false);
}


/* Functions */

/* Return a formatted C++ string */
std::string string_format(const char *str, ...)
{
	char *buff = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&buff, str, aptr);
	va_end(aptr);
	std::string ret(buff);
	free(buff);

	return ret;
}

/* Trim a C++ string from start */
static inline void string_ltrim(std::string &str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch)
	{
		return !std::isspace(ch);
	}));
}

/* Trim a C++ string from end */
static inline void string_rtrim(std::string &str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch)
	{
		return !std::isspace(ch);
	}).base(), str.end());
}

/* Trim a C++ string from both ends */
void string_trim(std::string &str)
{
	string_rtrim(str);
	string_ltrim(str);
}

#define TOKEN_LEN 4
/* Duplicate a string and set data size unit */
std::string string_set_size_unit(char *str_src)
{
	if(str_src == NULL)
		return "";

	ssize_t i = 0;
	const ssize_t len = std::strlen(str_src);
	std::string str_dst = "";

	while(i < len)
	{
		if((str_src[i] == '@') && (i + TOKEN_LEN - 1 < len) && (str_src[i + TOKEN_LEN - 1] == '@'))
		{
			std::string buff = "";
			/* Set unit in destination string */
			if(!strncmp(&str_src[i], "@0B@", TOKEN_LEN))
				buff = UNIT_B;
			else if(!strncmp(&str_src[i], "@KB@", TOKEN_LEN))
				buff = UNIT_KB;
			else if(!strncmp(&str_src[i], "@MB@", TOKEN_LEN))
				buff = UNIT_MB;
			else if(!strncmp(&str_src[i], "@GB@", TOKEN_LEN))
				buff = UNIT_GB;
			else if(!strncmp(&str_src[i], "@TB@", TOKEN_LEN))
				buff = UNIT_TB;
			else
				MSG_ERROR(_("cannot find unit in '%s' string at position %i"), str_src, i);
			str_dst += buff;
			i += TOKEN_LEN;
		}
		else
		{
			/* Copy one character */
			str_dst += str_src[i++];
		}
	}

	return str_dst;
}
#undef TOKEN_LEN

/* Format temperature with proper unit */
std::string string_format_temperature_unit(const char *fmt, const double temp_celsius)
{
	double temp;
	std::string unit;

	switch(Options::get_temp_unit())
	{
		case FAHRENHEIT:
			unit = "°F";
			temp = temp_celsius * 9/5 + 32;
			break;
		case KELVIN:
			unit = "K";
			temp = temp_celsius + 273.15;
			break;
		case RANKINE:
			unit = "Ra";
			temp = temp_celsius * 9/5 + 491.67;
			break;
		default:
			unit = "°C";
			temp = temp_celsius;
			break;
	}

	std::string ret = string_format(fmt, temp);
	ret += unit;
	MSG_DEBUG("string_format_temperature_unit: %.2f°C ==> %s", temp_celsius, ret.c_str());
	return ret;
}

/* Open a file and put its content in a variable ('str' accept printf-like format) */
int fopen_to_str(std::string &out, const char *str, ...)
{
	char *filename_str = NULL;
	std::string line;
	va_list aptr;

	out.clear();
	va_start(aptr, str);
	vasprintf(&filename_str, str, aptr);
	va_end(aptr);

	MSG_DEBUG("fopen_to_str: opening '%s'", filename_str);
	std::ifstream file(filename_str);
	if(!file.is_open())
	{
		MSG_ERROR(_("an error occurred while opening file '%s'"), filename_str);
		free(filename_str);
		return 1;
	}
	free(filename_str);

	while(std::getline(file, line))
		out += line;

	file.close();
	return 0;
}

template<typename... Args>
std::string fopen_to_str(const char *str, Args... args)
{
	std::string buff;
	fopen_to_str(buff, str, args...);
	return buff;
}

/* Run a command and put output in a variable ('str' accept printf-like format) */
int popen_to_str(std::string &out, const char *str, ...)
{
	char *cmd_str = NULL;
	char buffer[MAXSTR];
	FILE *pipe_descr = NULL;
	va_list aptr;

	out.clear();
	va_start(aptr, str);
	vasprintf(&cmd_str, str, aptr);
	va_end(aptr);

	pipe_descr = popen(cmd_str, "r");
	free(cmd_str);
	if(pipe_descr == NULL)
	{
		MSG_ERROR(_("an error occurred while running command '%s'"), cmd_str);
		return 1;
	}

	while(fgets(buffer, MAXSTR, pipe_descr) != NULL)
	{
		buffer[strcspn(buffer, "\r\n")] = '\0';
		out += buffer;
	}

	return pclose(pipe_descr);
}

/* Run a command and ignore output ('str' accept printf-like format) */
int run_command(const char *str, ...)
{
	int ret;
	char *cmd_str = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&cmd_str, str, aptr);
	va_end(aptr);

	ret = std::system(cmd_str);
	free(cmd_str);

	return ret;
}

/* Check if a command exists */
bool command_exists(const char *cmd)
{
	char buff[MAXSTR];

	snprintf(buff, MAXSTR, "command -v %s > /dev/null", cmd);

	return !std::system(buff);
}

/* Load a kernel module */
int load_module(const char *module, int *fd)
{
	int ret = -1;

	const ssize_t len = std::strlen(module) + 1;
	const DaemonCommand cmd = LOAD_MODULE;

#if defined (__linux__)
	ret = run_command("grep -wq %s /proc/modules 2> /dev/null", module);
#elif defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	ret = run_command("kldstat | grep %s > /dev/null", module);
#else
# error "Unsupported operating system"
#endif

	if((ret != 0) && (*fd >= 0))
	{
		/* Send module name to daemon */
		MSG_DEBUG("load_module: loading module %s", module);
		SEND_DATA(fd, &cmd, sizeof(DaemonCommand));
		SEND_DATA(fd, &len, sizeof(ssize_t));
		SEND_DATA(fd, module, len);

		/* Receive return value */
		RECEIVE_DATA(fd, &ret, sizeof(int));
	}

	return ret;
}

/* Change CPU affinity */
#if defined (__linux__)
# include <sched.h>
bool set_cpu_affinity(uint16_t logical_cpu)
{
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(logical_cpu, &cpuset);
	return sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0;
}
/* endif defined (__linux__) */
#elif defined (__FreeBSD__)
# include <sys/param.h>
# include <sys/cpuset.h>
bool set_cpu_affinity(uint16_t logical_cpu)
{
	cpuset_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(logical_cpu, &cpuset);
	return cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, sizeof(cpuset), &cpuset) == 0;
}
/* endif defined (__FreeBSD__) */
#elif defined (__DragonFly__)
# include <pthread.h>
# include <pthread_np.h>
bool set_cpu_affinity(uint16_t logical_cpu)
{
	cpuset_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(logical_cpu, &cpuset);
	return pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0;
}
/* endif defined (__DragonFly__) */
#elif defined (__NetBSD__)
# include <pthread.h>
# include <sched.h>
bool set_cpu_affinity(uint16_t logical_cpu)
{
	cpuset_t *cpuset;
	cpuset = cpuset_create();
	cpuset_set((cpuid_t) logical_cpu, cpuset);
	int ret = pthread_setaffinity_np(pthread_self(), cpuset_size(cpuset), cpuset);
	cpuset_destroy(cpuset);
	return ret == 0;
}
/* endif defined (__NetBSD__) */
#else
bool set_cpu_affinity(uint16_t logical_cpu)
{
	static bool warning_printed = 0;
	if(!warning_printed)
	{
		MSG_WARNING("%s", _("set_cpu_affinity() not supported on this operating system"));
		warning_printed = 1;
	}
	return false;
}
#endif /* set_cpu_affinity defined */

/* Find system path for device to drm */
std::string get_device_path_drm(std::string device_path)
{
	std::error_code fs_code;
	const fs::path drm_path(fs::path(device_path) / "drm");
	const std::regex regex_card("card[[:digit:]]");

	auto drm_path_it = fs::directory_iterator(drm_path, fs_code);
	auto find_it = std::find_if(drm_path_it, end(drm_path_it), [&regex_card](const auto& dir_entry)
	{
		return std::regex_search(dir_entry.path().string(), regex_card);
	});

	const std::string ret = (find_it != end(drm_path_it)) ? find_it->path().string() : std::string();
	MSG_DEBUG("get_device_path_drm: device_path=%s ==> %s", device_path.c_str(), ret.c_str());

	return ret;
}

/* Find system path for device to hwmon */
std::string get_device_path_hwmon(std::string device_path)
{
	std::error_code fs_code;
	const fs::path hwmon_path(fs::path(device_path) / "hwmon");
	const std::regex regex_hwmon("hwmon[[:digit:]]");

	auto hwmon_path_it = fs::directory_iterator(hwmon_path, fs_code);
	auto find_it = std::find_if(hwmon_path_it, end(hwmon_path_it), [&regex_hwmon](const auto& dir_entry)
	{
		return std::regex_search(dir_entry.path().string(), regex_hwmon);
	});

	const std::string ret = (find_it != end(hwmon_path_it)) ? find_it->path().string() : std::string();
	MSG_DEBUG("get_device_path_hwmon: device_path=%s ==> ret=%s", device_path.c_str(), ret.c_str());

	return ret;
}

/* Find sensor path for CPU temperature depending on kernel driver */
std::string get_sensor_path_cpu_temperature_driver(uint16_t current_core_id)
{
	std::error_code fs_code;
	const std::regex regex_filename_temp_in ("temp1_input");
	const std::regex regex_filename_temp_lab("temp[[:digit:]]_label");
	const std::regex regex_filename_in_in   ("in1_input");
	const std::regex regex_dirname_cardN    ("card[[:digit:]]");
	const std::regex regex_label_coreN      ("Core[[:space:]]*" + std::to_string(current_core_id), std::regex::icase);
	const std::regex regex_label_tdie       ("Tdie",                                               std::regex::icase);
	const std::unordered_multimap<std::string, std::pair<std::regex, std::optional<std::regex>>> drivers =
	{
		{ "coretemp", { regex_filename_temp_lab, regex_label_coreN } }, // "Core 0:        +33.0°C  (high = +80.0°C, crit = +98.0°C)"
		{ "k8temp",   { regex_filename_temp_lab, regex_label_coreN } }, // "Core0 Temp:    +64.0°C"
		{ "k10temp",  { regex_filename_temp_lab, regex_label_tdie  } }, // "Tdie:          +41.4°C"
		{ "k10temp",  { regex_filename_temp_in,  std::nullopt      } }, // "temp1:         +29.5°C"
		{ "zenpower", { regex_filename_temp_in,  std::nullopt      } }, // "Tdie:          +67.9°C"
	};

	/* Loop over all directories in SYS_HWMON */
	for(auto const& dir_entry : fs::directory_iterator(SYS_HWMON, fs_code))
	{
		/* Ignore drivers not in unordered_map */
		const std::string driver_name = fopen_to_str("%s/name", dir_entry.path().string().c_str());
		if(drivers.find(driver_name) == drivers.end())
			continue;

		/* Look sensor matching pattern */
		auto range = drivers.equal_range(driver_name);
		for(auto it = range.first; it != range.second; ++it)
		{
			const auto &pair                                = it->second;
			const std::regex                &regex_filename = pair.first;
			const std::optional<std::regex> &regex_label    = pair.second;
			for(auto const& dir_sub_entry : fs::directory_iterator{dir_entry.path()})
			{
				std::string ret;
				std::string sensor_filename = dir_sub_entry.path().string();

				/* Keep only files matching label filename */
				if(!std::regex_search(sensor_filename, regex_filename))
					continue;

				if(regex_label.has_value())
				{
					std::smatch match_regex_label;
					const std::string label_content = fopen_to_str("%s", dir_sub_entry.path().string().c_str());
					std::regex_search(label_content, match_regex_label, regex_label.value());
					/* Return sensor filename if label content corresponds to expected value */
					if(!match_regex_label.empty())
						ret = std::regex_replace(match_regex_label.str(), std::regex("_label"), "_input");
				}
				else
				{
					/* Return sensor filename as is */
					ret = sensor_filename;
				}

				if(!ret.empty())
				{
					MSG_DEBUG("get_sensor_path_cpu_temperature_driver: current_core_id=%u ==> ret=%s", current_core_id, ret.c_str());
					return ret;
				}
			}
		}
	}

	return std::string();
}

/* Find sensor path for generic CPU temperature */
std::string get_sensor_path_cpu_temperature_generic()
{
	std::error_code fs_code;
	const std::regex regex_filename_temp_lab("temp[[:digit:]]_label");
	const std::regex regex_label_other      ("CPU", std::regex::icase);

	/* Loop over all directories in SYS_HWMON */
	for(auto const& dir_entry : fs::directory_iterator(SYS_HWMON, fs_code))
	{
		for(auto const& dir_sub_entry : fs::directory_iterator{dir_entry.path()})
		{
			/* Keep only files matching label filename */
			std::smatch match_regex_filename;
			const std::string label_filename = dir_sub_entry.path().filename().string();
			std::regex_search(label_filename, match_regex_filename, regex_filename_temp_lab);
			if(match_regex_filename.empty())
				continue;

			std::smatch match_regex_label;
			const std::string label_content = fopen_to_str("%s", dir_sub_entry.path().string().c_str());
			std::regex_search(label_content, match_regex_label, regex_label_other);
			/* Return sensor filename if label content corresponds to expected value */
			if(!match_regex_label.empty())
			{
				std::string ret = dir_sub_entry.path().parent_path().string();
				ret            += "/" + std::regex_replace(match_regex_filename.str(), std::regex("_label"), "_input");
				MSG_DEBUG("get_sensor_path_cpu_temperature_generic: ret=%s", ret.c_str());
				return ret;
			}
		}
	}

	return std::string();
}

/* Find sensor path for CPU voltage */
std::string get_sensor_path_cpu_voltage(uint16_t current_core_id)
{
	std::error_code fs_code;
	const std::unordered_map<std::string, std::pair<std::string, std::string>> drivers =
	{
		{ "zenpower", { "in1", "SVI2_Core" } } // "SVI2_Core:     1.38 V"
	};

	/* Loop over all directories in SYS_HWMON */
	for(auto const& dir_entry : fs::directory_iterator(SYS_HWMON, fs_code))
	{
		/* Ignore drivers not in unordered_map */
		const std::string driver_name = fopen_to_str("%s/name", dir_entry.path().string().c_str());
		if(drivers.find(driver_name) == drivers.end())
			continue;

		/* Look sensor matching pattern */
		const auto &pair                  = drivers.at(driver_name);
		const std::string input_filename  = pair.first + "_input";
		const std::string label_filename  = pair.first + "_label";
		const std::string label_exp_value = pair.second;
		for(auto const& dir_sub_entry : fs::directory_iterator{dir_entry.path()})
		{
			/* Keep only files matching label filename */
			if(dir_sub_entry.path().filename() != label_filename)
				continue;

			/* Return sensor filename if label content corresponds to expected value */
			const std::string label_content = fopen_to_str("%s", dir_sub_entry.path().string().c_str());
			if(label_content == label_exp_value)
			{
				std::string ret = dir_sub_entry.path().parent_path() / input_filename;
				MSG_DEBUG("get_sensor_path_cpu_voltage: current_core_id=%u ==> ret=%s", current_core_id, ret.c_str());
				return ret;
			}
		}
	}

	return std::string();
}
