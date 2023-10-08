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
* FILE util.hpp
*/

#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <unistd.h>
#include <cstdint>
#include <string>
#include "logger.hpp"

#if HAS_GETTEXT
# include <libintl.h>
#endif

/* Software definition */
#define PRGNAME               "CPU-X"
#define PRGNAME_LOW           "cpu-x"
#define PRGAUTH               "The Tumultuous Unicorn Of Darkness"
#define PRGUSERNAME           "thetumultuousunicornofdarkness"
#define PRGURL                "https://" PRGUSERNAME ".github.io/" PRGNAME
#define ISSUEURL              "https://github.com/" PRGUSERNAME "/" PRGNAME "/issues/new?template=bug_report.md"
#define PRGCPRGHT             "Copyright © 2014-2023 " PRGAUTH
#define PRGLCNS               "GNU GPL v3"
#define LCNSURL               "https://www.gnu.org/licenses/gpl.txt"

/* Colors definition */
#define DEFAULT               "\x1b[0m"
#define BOLD_RED              "\x1b[1;31m"
#define BOLD_GREEN            "\x1b[1;32m"
#define BOLD_YELLOW           "\x1b[1;33m"
#define BOLD_BLUE             "\x1b[1;34m"
#define BOLD_MAGENTA          "\x1b[1;35m"

/* Utilities macro */
#define GOTO_ERROR(str)       { error_str = str; goto error; }
#define IS_ROOT               (getuid() == 0)
#define PRGINFO(out)          fprintf(out, "%s %s%s (%s %s, %s %s, %s %s)\n", PRGNAME, PRGVER, GITREV, __DATE__, __TIME__, SYSTEM_NAME, SYSTEM_PROCESSOR, C_COMPILER_ID, __VERSION__)

/* Formatted messages definition */
#define MSG_DEBUG(fmt, ...)   Logger::log(LOG_DEBUG,    false, string_format(fmt, __VA_ARGS__))
#define MSG_VERBOSE(fmt, ...) Logger::log(LOG_VERBOSE,  false, string_format(fmt, __VA_ARGS__))
#define MSG_STDOUT(fmt, ...)  Logger::log(LOG_STANDARD, false, string_format(fmt, __VA_ARGS__))
#define MSG_WARNING(fmt, ...) Logger::log(LOG_WARNING,  false, string_format(fmt, __VA_ARGS__))
#define MSG_ERROR(fmt, ...)   Logger::log(LOG_ERROR,    false, string_format(fmt, __VA_ARGS__))
#define MSG_ERRNO(fmt, ...)   Logger::log(LOG_ERROR,    true,  string_format(fmt, __VA_ARGS__))
#if HAS_GETTEXT
# define _(msg)               gettext(msg)
#else
# define _(msg)               msg
#endif /* HAS_GETTEXT */
#define N_(msg)               msg

/* Arrays definition */
#define MAXSTR                80       /* Max string */

#define UNIT_B                _("bytes")
/* SI unit prefixes */
// TRANSLATORS: initials for kilobyte (10³): please put initials (keep it short)
#define UNIT_KB               _("kB")
// TRANSLATORS: initials for megabyte (10⁶): please put initials (keep it short)
#define UNIT_MB               _("MB")
// TRANSLATORS: initials for gigabyte (10⁹): please put initials (keep it short)
#define UNIT_GB               _("GB")
// TRANSLATORS: initials for terabyte (10¹²): please put initials (keep it short)
#define UNIT_TB               _("TB")
/* Binary unit prefixes  */
// TRANSLATORS: initials for kibibyte (2¹⁰): please put initials (keep it short)
#define UNIT_KIB               _("KiB")
// TRANSLATORS: initials for mebibyte (2²⁰): please put initials (keep it short)
#define UNIT_MIB               _("MiB")
// TRANSLATORS: initials for gibibyte (2³⁰): please put initials (keep it short)
#define UNIT_GIB               _("GiB")
// TRANSLATORS: initials for tebibyte (2⁴⁰): please put initials (keep it short)
#define UNIT_TIB               _("TiB")

/* Linux-specific paths definition */
#define SYS_DMI               "/sys/devices/virtual/dmi/id"
#define SYS_CPU               "/sys/devices/system/cpu/cpu"
#define SYS_DRM               "/sys/class/drm/card"
#define SYS_HWMON             "/sys/class/hwmon"
#define SYS_DEBUG             "/sys/kernel/debug"
#define SYS_DEBUG_DRI         SYS_DEBUG"/dri"

/* FreeBSD-specific paths definition */
#define DEV_PCI               "/dev/pci"

/* Devices vendor ID */
#define DEV_VENDOR_ID_AMD    0x1002
#define DEV_VENDOR_ID_INTEL  0x8086
#define DEV_VENDOR_ID_NVIDIA 0x10DE


/* PrefixUnit class */

class PrefixUnit
{
public:
	enum Multipliers
	{
		MULT_NONE = 0,
		MULT_K    = 1,
		MULT_M    = 2,
		MULT_G    = 3,
		MULT_T    = 4,
		MULT_LAST = 5
	};

	bool       init;
	const char *prefix;
	uint64_t   divisor;

	PrefixUnit();
	void find_best_si_prefix(uint64_t value, Multipliers multiplier);
	void find_best_binary_prefix(uint64_t value, Multipliers multiplier);

private:
	void find_best_prefix(uint64_t value, Multipliers multiplier, bool use_si_prefixes);
};


/* Functions */

/* Return a formatted C++ string */
std::string string_format(const char *str, ...);

/* Trim a C++ string from both ends */
void string_trim(std::string &str);

/* Duplicate a string and set data size unit */
std::string string_set_size_unit(char *str_src);

/* Format temperature with proper unit */
std::string string_format_temperature_unit(const char *fmt, const double temp_celsius);

/* Open a file and put its content in a variable ('str' accept printf-like format) */
int fopen_to_str(std::string &out, const char *str, ...);

/* Run a command and put output in a variable ('str' accept printf-like format) */
int popen_to_str(std::string &out, const char *str, ...);

/* Run a command and ignore output ('str' accept printf-like format) */
int run_command(const char *str, ...);

/* Check if a command exists */
bool command_exists(const char *cmd);

/* Load a kernel module (return 0 on success) */
int load_module(const char *module, int *fd);

/* Change CPU affinity */
bool set_cpu_affinity(uint16_t logical_cpu);

/* Find system path for device to drm */
std::string get_device_path_drm(std::string device_path);

/* Find system path for device to hwmon */
std::string get_device_path_hwmon(std::string device_path);

/* Find sensor path for CPU temperature depending on kernel driver */
std::string get_sensor_path_cpu_temperature_driver(uint16_t current_core_id);

/* Find sensor path for generic CPU temperature */
std::string get_sensor_path_cpu_temperature_generic();

/* Find sensor path for CPU voltage */
std::string get_sensor_path_cpu_voltage(uint16_t current_core_id);


/* Templates */

// https://stackoverflow.com/a/47072631
template <std::size_t N>
int execvp_cpp(const char* file, const char* const (&argv)[N])
{
	assert((N > 0) && (argv[N - 1] == nullptr));
	std::string args;
	for(int i = 0; const_cast<char*>(argv[i]) != nullptr; i++)
		args += std::string(const_cast<char*>(argv[i])) + " ";
	MSG_DEBUG("execvp: %s", args.c_str());
	return execvp(file, const_cast<char* const*>(argv));
}


#endif /* _UTIL_HPP_ */
