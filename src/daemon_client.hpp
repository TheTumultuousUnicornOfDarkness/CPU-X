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
* FILE daemon_client.hpp
*/

#ifndef _DAEMON_CLIENT_HPP_
#define _DAEMON_CLIENT_HPP_


/* Start daemon in background */
const char *start_daemon(bool graphical);

/* Check if daemon is running */
bool daemon_is_alive(void);

/* Establish connection to daemon */
int connect_to_daemon(int &socket_fd);


#endif /* _DAEMON_CLIENT_HPP_ */
