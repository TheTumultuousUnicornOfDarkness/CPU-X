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
* FILE logger.hpp
*/

#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <string>

#define DEFAULT               "\x1b[0m"
#define BOLD_RED              "\x1b[1;31m"
#define BOLD_GREEN            "\x1b[1;32m"
#define BOLD_YELLOW           "\x1b[1;33m"
#define BOLD_BLUE             "\x1b[1;34m"
#define BOLD_MAGENTA          "\x1b[1;35m"


enum LogPriority
{
	LOG_DEBUG,
	LOG_VERBOSE,
	LOG_STANDARD,
	LOG_WARNING,
	LOG_ERROR,
};

class Logger
{
public:
	static void set_verbosity(LogPriority priority);
	static LogPriority get_verbosity();
	static void log(LogPriority priority, bool print_errno, std::string message);

private:
	static inline LogPriority verbosity = LOG_STANDARD;
	Logger() = delete;
	~Logger() = delete;
};


#endif /* _LOGGER_HPP_ */
