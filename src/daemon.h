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
* FILE daemon.h
*/

#ifndef _DAEMON_H_
#define _DAEMON_H_

#define POLL_TIMEOUT (5 * 1000) // 5 seconds
#define NFDS         1
#define LOG_FILE     "/tmp/cpu-x-daemon.log"

typedef struct
{
	pthread_t id;
	int       fd;
} Thread;

typedef struct
{
	pthread_mutex_t mutex;
	uint8_t count;
	uint8_t allocated;
	Thread *thread;
} ThreadsInfo;

#endif /* _DAEMON_H_ */
