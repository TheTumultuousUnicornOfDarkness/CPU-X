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
* FILE daemon_server.hpp
*/

#ifndef _DAEMON_SERVER_HPP_
#define _DAEMON_SERVER_HPP_

#include <thread>
#include <mutex>
#include <vector>

#define POLL_TIMEOUT (5 * 1000) // 5 seconds
#define NFDS         1
#define LOG_FILE     "/tmp/cpu-x-daemon.log"


struct ThreadsInfo
{
	uint16_t running_threads = 0;
	std::mutex mutex;
	std::vector<std::thread> threads {};
};


#endif /* _DAEMON_SERVER_HPP_ */
