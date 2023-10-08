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
* FILE main.cpp
*/

#include <unistd.h>
#include <execinfo.h>
#include <getopt.h>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <clocale>
#include <cctype>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <list>
#include <forward_list>
#include "options.hpp"
#include "util.hpp"
#include "data.hpp"
#include "core.hpp"
#include "daemon.h"
#include "daemon_client.hpp"

#define LOG_FILE "/tmp/cpu-x.log"

bool __sigabrt_received = false;


/************************* Options-related functions *************************/

struct Getopt
{
	const bool has_mod;
	const char short_opt;
	const char *long_opt;
	const int  need_arg;
	const char *description;
};
static const std::list<struct Getopt> cpux_options =
{
	{ HAS_GTK,         'G', "gtk",           no_argument,       N_("Start graphical user interface (GUI) (default)")           },
	{ HAS_NCURSES,     'N', "ncurses",       no_argument,       N_("Start text-based user interface (TUI)")                    },
	{ true,            'D', "dump",          no_argument,       N_("Dump all data on standard output and exit")                },
	{ HAS_DMIDECODE,   'M', "dmidecode",     no_argument,       N_("Run embedded command dmidecode and exit")                  },
	{ HAS_BANDWIDTH,   'B', "bandwidth",     no_argument,       N_("Run embedded command bandwidth and exit")                  },
	{ true,            'u', "temp-unit",     required_argument, N_("Set temperature unit (c[elsius]|f[ahrenheit]|k[elvin]|r[ankine])") },
	{ true,            'r', "refresh",       required_argument, N_("Set custom time between two refreshes (in seconds)")       },
	{ true,            't', "tab",           required_argument, N_("Set default tab (integer)")                                },
	{ HAS_LIBCPUID,    'p', "type",          required_argument, N_("Select core type to monitor (integer)")                    },
	{ HAS_LIBCPUID,    'c', "core",          required_argument, N_("Select CPU core to monitor (integer)")                     },
	{ HAS_BANDWIDTH,   'b', "cachetest",     required_argument, N_("Set custom bandwidth test for CPU caches speed (integer)") },
	{ HAS_DMIDECODE,   's', "stick",         required_argument, N_("Select default memory stick (integer)")                    },
	{ HAS_LIBPCI,      'g', "gpu",           required_argument, N_("Select default graphic card (integer)")                    },
	{ true,            'd', "daemon",        no_argument,       N_("Start and connect to daemon")                              },
	{ true,            'v', "verbose",       no_argument,       N_("Verbose output")                                           },
	{ true,            'h', "help",          no_argument,       N_("Print help and exit")                                      },
	{ true,            'V', "version",       no_argument,       N_("Print version and exit")                                   },
	{ HAS_LIBCPUID,      0, "cpuid-decimal", no_argument,       N_("Print CPUID values in decimal (default is hexadeximal)")   },
	{ true,              0, "nocolor",       no_argument,       N_("Disable colored output")                                   },
	{ true,              0, "debug",         no_argument,       N_("Print information for debugging")                          },
	{ true,              0, "issue-fmt",     no_argument,       N_("Print required information to paste in an issue")          },
	{ HAS_NCURSES,       0, "keymap",        required_argument, N_("Set key mapping for NCurses mode (a[rrows]|e[macs]|i[nverted-T]|v[im])") },
};

struct EnvVar
{
	const bool has_mod;
	const char *var_name;
	const char *description;
};
static const std::list<struct EnvVar> cpux_env_vars =
{
	{ true,            "CPUX_ARGS",                N_("Add default command line arguments")                         },
	{ true,            "CPUX_BCLK",                N_("Enforce the bus clock")                                      },
	{ true,            "CPUX_FORCE_CPU_FALLBACK",  N_("Ignore CPU values reported by libcpuid (comma-separated values among 'temp', 'volt' and 'freq')") },
	{ HAS_LIBCPUID,    "CPUX_CPUID_RAW",           N_("Read CPUID raw data from a given file")                      },
	{ HAS_LIBCPUID,    "CPUX_DEBUG_DATABASE",      N_("Only print a message if CPU is not present in the database") },
};

/* Check for influenceable environment variables */
static void check_environment_variables(Data &data)
{
	if(std::getenv("CPUX_BCLK"))
		data.cpu.clocks.set_bus_freq(std::atof(std::getenv("CPUX_BCLK")));
	if(std::getenv("CPUX_FORCE_CPU_FALLBACK"))
	{
		const std::string force_cpu_fallback = std::getenv("CPUX_FORCE_CPU_FALLBACK");
		Options::set_fallback_cpu_temp(force_cpu_fallback.find("temp") != std::string::npos);
		Options::set_fallback_cpu_volt(force_cpu_fallback.find("volt") != std::string::npos);
		Options::set_fallback_cpu_freq(force_cpu_fallback.find("freq") != std::string::npos);
	}
	if(std::getenv("CPUX_CPUID_RAW"))
		data.cpu.cpuid_raw_file = std::getenv("CPUX_CPUID_RAW");
	if(std::getenv("CPUX_DEBUG_DATABASE"))
		Options::set_debug_database((std::atoi(std::getenv("CPUX_DEBUG_DATABASE"))) > 0);
}

/* This is help display with --help option */
static void help(std::string binary_name)
{
	bool options_header = false;
	char buff[MAXSTR];

	MSG_STDOUT(_("Usage: %s DISPLAY [OPTIONS]\n"), binary_name.c_str());
	MSG_STDOUT("%s", _("Available DISPLAY:"));
	for(auto& option : cpux_options)
	{
		if(option.has_mod)
		{
			if(!options_header && islower(option.short_opt))
			{
				options_header = true;
				MSG_STDOUT("\n%s", _("Available OPTIONS:"));
			}

			if(option.short_opt) snprintf(buff, MAXSTR, "  -%c,", option.short_opt);
			else                 snprintf(buff, MAXSTR, "     ");
			MSG_STDOUT("%s --%-14s %s", buff, option.long_opt, _(option.description));
		}
	}

	MSG_STDOUT("\n%s", _("Influenceable environment variables:"));
	for(auto& env_var : cpux_env_vars)
		if(env_var.has_mod)
			MSG_STDOUT("  %-25s %s", env_var.var_name, _(env_var.description));
}

/* This is the --version option */
static void version(bool full_header)
{
	int i;
	const struct { const bool has_mod; const char *lib, *version; } libs_ver[] =
	{
		{ HAS_GTK,         "GTK",         GTK_VERSION         },
		{ HAS_NCURSES,     "NCURSES",     NCURSES_VERSION     },
		{ HAS_LIBCPUID,    "LIBCPUID",    LIBCPUID_VERSION    },
		{ HAS_LIBPCI,      "LIBPCI",      LIBPCI_VERSION      },
		{ HAS_LIBGLFW,     "LIBGLFW",     LIBGLFW_VERSION     },
		{ HAS_Vulkan,      "VULKAN",      Vulkan_VERSION      },
		{ HAS_OpenCL,      "OPENCL",      OpenCL_VERSION      },
		{ HAS_LIBPROC2,    "LIBPROC2",    LIBPROC2_VERSION    },
		{ HAS_LIBPROCPS,   "LIBPROCPS",   LIBPROCPS_VERSION   },
		{ HAS_LIBSTATGRAB, "LIBSTATGRAB", LIBSTATGRAB_VERSION },
		{ HAS_DMIDECODE,   "DMIDECODE",   DMIDECODE_VERSION   },
		{ HAS_BANDWIDTH,   "BANDWIDTH",   BANDWIDTH_VERSION   },
		{ false,           NULL,          NULL                }
	};

	PRGINFO(stdout);
	if(full_header)
	{
		MSG_STDOUT("%s\n", PRGCPRGHT);
		MSG_STDOUT("%s", _("This is free software: you are free to change and redistribute it."));
		MSG_STDOUT("%s", _("This program comes with ABSOLUTELY NO WARRANTY"));
		MSG_STDOUT(_("See the %s license: <%s>\n"), PRGLCNS, LCNSURL);
	}

	/* Print features version */
	for(i = 0; libs_ver[i].lib != NULL; i++)
	{
		if(libs_ver[i].has_mod)
			MSG_STDOUT(_("-- %-9s version: %s"), libs_ver[i].lib, libs_ver[i].version);
	}
}

/* Add arguments from environment variable CPUX_ARGS */
static std::vector<std::string> get_arguments_from_environment(std::string binary_name)
{
	const char *env_args = std::getenv("CPUX_ARGS");
	std::vector<std::string> args = { binary_name };

	if(env_args != NULL)
	{
		std::string tmp;
		std::stringstream stream_env_args(env_args);

		while (stream_env_args >> tmp)
			args.push_back(tmp);
	}

	return args;
}

/* Parse arguments and set some flags */
static void parse_arguments(std::forward_list<std::string> &cmd_args)
{
	int c, longindex;
	std::string shortopts;
	std::vector<char*> cargs;
	std::vector<struct option> longopts;
	const std::string binary_name = cmd_args.front();

	/* Inject arguments from CPUX_ARGS environment variable */
	cmd_args.pop_front();
	std::vector<std::string> env_args = get_arguments_from_environment(binary_name);
	env_args.insert(env_args.end(), std::make_move_iterator(cmd_args.begin()), std::make_move_iterator(cmd_args.end()));
	std::transform(env_args.begin(), env_args.end(), std::back_inserter(cargs), [](const std::string& str)
	{
		return const_cast<char*>(str.c_str());
	});

	/* Fill longopts structure */
	for(auto& option : cpux_options)
	{
		if(option.has_mod)
		{
			longopts.push_back({ option.long_opt, option.need_arg, NULL, option.short_opt });
			if(option.short_opt)
			{
				shortopts.push_back(option.short_opt);
				if(option.need_arg)
					shortopts.push_back(':');
			}
		}
	}

	/* Set the default mode */
	if(HAS_GTK && (std::getenv("DISPLAY") != NULL || std::getenv("WAYLAND_DISPLAY") != NULL))
		Options::set_output_type(OUT_GTK);
	else if(HAS_NCURSES)
		Options::set_output_type(OUT_NCURSES);
	else
		Options::set_output_type(OUT_DUMP);

	/* Parse options */
	while((c = getopt_long(cargs.size(), cargs.data(), shortopts.c_str(), longopts.data(), &longindex)) != -1)
	{
		switch(c)
		{
			case 'G':
				Options::set_output_type(OUT_GTK);
				break;
			case 'N':
				Options::set_output_type(OUT_NCURSES);
				break;
			case 'D':
				Options::set_output_type(OUT_DUMP);
				break;
			case 'M':
				Options::set_output_type(OUT_DMIDECODE);
				break;
			case 'B':
				Options::set_output_type(OUT_BANDWIDTH);
				break;
			case 'u':
				switch(std::tolower(optarg[0]))
				{
					case 'c': Options::set_temp_unit(CELSIUS);    break;
					case 'f': Options::set_temp_unit(FAHRENHEIT); break;
					case 'k': Options::set_temp_unit(KELVIN);     break;
					case 'r': Options::set_temp_unit(RANKINE);    break;
					default: help(binary_name); exit(EXIT_FAILURE);
				}
				break;
			case 'r':
				Options::set_refr_time(std::stoul(optarg));
				break;
			case 't':
				Options::set_selected_page(static_cast<TabNumber>(std::stoul(optarg)));
				break;
			case 'p':
				Options::set_selected_type(std::stoul(optarg), -1);
				break;
			case 'c':
				Options::set_selected_core(std::stoul(optarg), -1);
				break;
			case 'b':
				Options::set_selected_test(std::stoul(optarg));
				break;
			case 's':
				Options::set_selected_stick(std::stoul(optarg), -1);
				break;
			case 'g':
				Options::set_selected_gpu(std::stoul(optarg), -1);
				break;
			case 'd':
				Options::set_with_daemon(true);
				break;
			case 'v':
				Logger::set_verbosity(LOG_VERBOSE);
				break;
			case 'h':
				help(binary_name);
				exit(EXIT_SUCCESS);
				break;
			case 'V':
				version(true);
				exit(EXIT_SUCCESS);
				break;
			case 0:
				if(!strcmp(longopts[longindex].name, "cpuid-decimal"))
				{
					Options::set_cpuid_decimal(true);
					break;
				}
				else if(!strcmp(longopts[longindex].name, "nocolor"))
				{
					Options::set_color(false);
					break;
				}
				else if(!strcmp(longopts[longindex].name, "debug"))
				{
					Logger::set_verbosity(LOG_DEBUG);
					break;
				}
				else if(!strcmp(longopts[longindex].name, "issue-fmt"))
				{
					Options::set_color(false);
					Options::set_issue(true);
					Options::set_output_type(OUT_DUMP);
					Logger::set_verbosity(LOG_DEBUG);
					std::setlocale(LC_ALL, "C");
					std::freopen(LOG_FILE, "w", stdout);
					std::setvbuf(stdout, NULL, _IONBF, 0);
					dup2(STDOUT_FILENO, STDERR_FILENO);
					version(false);
					break;
				}
				else if(!strcmp(longopts[longindex].name, "keymap"))
				{
					switch(std::tolower(optarg[0]))
					{
						case 'a': Options::set_keymap(ARROWS);     break;
						case 'e': Options::set_keymap(EMACS);      break;
						case 'i': Options::set_keymap(INVERTED_T); break;
						case 'v': Options::set_keymap(VIM);        break;
						default: help(binary_name); exit(EXIT_FAILURE);
					}
					break;
				}
				/* Fall through */
			case '?':
			default:
				help(binary_name);
				exit(EXIT_FAILURE);
		}
	}
}


/************************* Main-related functions *************************/

static void common_sighandler(int signum, bool need_stop)
{
	int bt_size, i;
	char **bt_syms;
	void *bt[16];
	std::string::size_type addr_start, addr_end;
	std::string line, address, line_number;

	/* Get the backtrace */
	bt_size = backtrace(bt, 16);
	bt_syms = backtrace_symbols(bt, bt_size);

	/* Print the backtrace */
	if(need_stop)
		MSG_ERROR(_("\nOops, something was wrong! %s has received signal %d (%s) and has crashed."), PRGNAME, signum, strsignal(signum));
	else
		MSG_ERROR(_("\nOops, something was wrong! %s has received signal %d (%s) and is trying to recover."), PRGNAME, signum, strsignal(signum));

	MSG_ERROR("%s", "========================= Backtrace =========================");
	PRGINFO(stderr);
	for(i = 1; i < bt_size; i++)
	{
		line_number.clear();
		line       = bt_syms[i];
		addr_start = line.find("[");
		addr_end   = line.find("]");
		if((addr_start != std::string::npos) && (addr_end != std::string::npos))
		{
			addr_start++;
			address = line.substr(addr_start, addr_end - addr_start);
			popen_to_str(line_number, "addr2line -s -e /proc/%d/exe %s | cut -d' ' -f1", getpid(), address.c_str());
		}
		if(!line_number.empty() && (line_number.find("??") == std::string::npos))
			MSG_ERROR("#%2i %s %s", i, line_number.c_str(), bt_syms[i]);
		else
			MSG_ERROR("#%2i %s", i, bt_syms[i]);
	}
	MSG_ERROR("%s\n", "======================== End Backtrace =======================");
	if(need_stop)
	{
		MSG_ERROR("%s", _("You can open a new issue here, by filling the template as requested:"));
		MSG_ERROR("%s\n", ISSUEURL);
	}

	/* Stop program */
	free(bt_syms);
	if(need_stop)
	{
		std::signal(signum, SIG_DFL);
		kill(getpid(), signum);
	}
}

/* Action on SIGILL/SIGSEV/SIGFPE */
static void sighandler_fatal(int signum)
{
	common_sighandler(signum, true);
}

/* Action on SIGABRT */
static void sighandler_abrt(int signum)
{
	__sigabrt_received = true;
	common_sighandler(signum, false);
}

 /* Enable internationalization support */
#if HAS_GETTEXT
static int set_locales(void)
{
	int err;
	const char *TEXTDOMAINDIR = std::getenv("TEXTDOMAINDIR") ? std::getenv("TEXTDOMAINDIR") : LOCALEDIR;

	/* Apply locale */
	std::setlocale(LC_ALL, "");
	err  = bindtextdomain(GETTEXT_PACKAGE, TEXTDOMAINDIR)    ? 0 : 1;
	err += bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8") ? 0 : 1;
	err += textdomain(GETTEXT_PACKAGE)                       ? 0 : 1;

	/* Check if something is wrong */
	if(err)
	{
		MSG_ERROR("%s", _("an error occurred while setting locale"));
		return 1;
	}
	else
		return 0;
}
#endif /* HAS_GETTEXT */

int main(int argc, char *argv[])
{
#if HAS_GETTEXT
	/* Text localization */
	set_locales();
#endif /* HAS_GETTEXT */

	/* Signal handlers */
	std::signal(SIGILL,  sighandler_fatal);
	std::signal(SIGSEGV, sighandler_fatal);
	std::signal(SIGFPE,  sighandler_fatal);
	std::signal(SIGABRT, sighandler_abrt);

	/* Init variables */
	std::forward_list<std::string> args(argv, argv + argc);
	Data data; // Note: must be done after set_locales() to translate all labels

	/* Parse options */
#if HAS_GTK
	load_settings();
#endif /* HAS_GTK */
	parse_arguments(args);
	check_environment_variables(data);
	if(Options::get_output_type() > OUT_NO_CPUX)
		goto skip_init;

	/* Connect to daemon */
	if(IS_ROOT || Options::get_with_daemon())
		start_daemon(false);
	if(daemon_is_alive())
		connect_to_daemon(data.socket_fd);

	/* Retrieve data */
	fill_labels(data);

	/* Show data */
	if(HAS_GTK && Options::output_type_is(OUT_GTK))
		start_gui_gtk(data);
	if(HAS_NCURSES && Options::output_type_is(OUT_NCURSES))
		start_tui_ncurses(data);
	if(Options::output_type_is(OUT_DUMP))
		std::cout << data;
skip_init:
	if(HAS_DMIDECODE && Options::output_type_is(OUT_DMIDECODE))
		return run_dmidecode();
	if(HAS_BANDWIDTH && Options::output_type_is(OUT_BANDWIDTH))
		return run_bandwidth();

	if(data.reload)
		execvp(argv[0], argv);

	return EXIT_SUCCESS;
}
