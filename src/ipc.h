/****************************************************************************
*    Copyright © 2014-2019 Xorg
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
* FILE ipc.h
*/

#ifndef _IPC_H_
#define _IPC_H_

#define SOCKET_NAME "/tmp/cpu-x.sock"
#define DAEMON_UP   (data->socket_fd >= 0)

#define SEND_DATA(pfd, pdata, size)    if(write(*pfd, pdata, size) != size) { MSG_ERRNO("write"); close(*pfd); *pfd = -1; return 1; }
#define RECEIVE_DATA(pfd, pdata, size) if(read (*pfd, pdata, size) != size) { MSG_ERRNO("read");  close(*pfd); *pfd = -1; return 1; }

typedef enum
{
	LIBCPUID_MSR_STATIC,
	LIBCPUID_MSR_DYNAMIC,
	DMIDECODE,
	POPEN_TO_STR,
	LOAD_MODULE,
} DaemonCommand;

typedef struct
{
	int min_mult, max_mult, bclk;
} MsrStaticData;

typedef struct
{
	int voltage, temp;
} MsrDynamicData;

#endif /* _IPC_H_ */
