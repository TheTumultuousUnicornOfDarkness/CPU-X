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
* FILE logger.cpp
*/

#include <cstring>
#include <cerrno>
#include <string>
#include <iostream>
#include "options.hpp"
#include "logger.hpp"


void Logger::set_verbosity(LogPriority priority)
{
	Logger::verbosity = priority;
}

LogPriority Logger::get_verbosity()
{
	return Logger::verbosity;
}

void Logger::log(LogPriority priority, bool print_errno, std::string message)
{
	std::string color;
	std::ostream *os;

	if(priority < Logger::verbosity)
		return;

	switch(priority)
	{
		case LOG_DEBUG:    os = &std::cout; color = BOLD_MAGENTA; break;
		case LOG_VERBOSE:  os = &std::cout; color = BOLD_GREEN;   break;
		case LOG_STANDARD: os = &std::cout; color = DEFAULT;      break;
		case LOG_WARNING:  os = &std::cout; color = BOLD_YELLOW;  break;
		case LOG_ERROR:    os = &std::cerr; color = BOLD_RED;     break;
	}

	if(Options::get_color())
		*os << color;
	*os << message;
	if(print_errno)
		*os << " (" << std::strerror(errno) << ")";
	if(Options::get_color())
		*os << DEFAULT;
	*os << std::endl;
}
