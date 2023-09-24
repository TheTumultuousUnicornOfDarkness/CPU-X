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

#define DAEMON_UP (data.socket_fd >= 0)

#ifndef MAXSTR
# define MAXSTR 80 /* Max string */
#endif

#define SEND_DATA(pfd, pdata, size)              \
	if(write(*pfd, pdata, size) == size)         \
	{                                            \
		MSG_DEBUG("daemon: writing %luB", size); \
	}                                            \
	else                                         \
	{                                            \
		MSG_ERRNO("%s", "daemon: write error");  \
		close(*pfd);                             \
		*pfd = -1;                               \
		return 1;                                \
	}
#define RECEIVE_DATA(pfd, pdata, size)           \
	if(read(*pfd, pdata, size) == size)          \
	{                                            \
		MSG_DEBUG("daemon: reading %luB", size); \
	}                                            \
	else                                         \
	{                                            \
		MSG_ERRNO("%s", "daemon: read error");   \
		close(*pfd);                             \
		*pfd = -1;                               \
		return 1;                                \
	}

typedef enum
{
	LIBCPUID_MSR_DEBUG,
	LIBCPUID_MSR_STATIC,
	LIBCPUID_MSR_DYNAMIC,
	DMIDECODE,
	ACCESS_DEV_PCI,
	ACCESS_SYS_DEBUG,
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

typedef struct
{
	char brand[MAXSTR], version[MAXSTR], date[MAXSTR], romsize[MAXSTR];
} DmidecodeBiosData;

typedef struct
{
	char manufacturer[MAXSTR], model[MAXSTR], revision[MAXSTR];
} DmidecodeMBData;

typedef struct
{
	double bus_freq;
	char cpu_package[MAXSTR];
} DmidecodeCPUData;

typedef struct
{
	char manufacturer[MAXSTR];
	char part_number[MAXSTR];
	char type[MAXSTR];
	char type_detail[MAXSTR];
	char device_locator[MAXSTR];
	char bank_locator[MAXSTR];
	char size[MAXSTR];
	char rank[MAXSTR];
	char speed_maximum[MAXSTR];
	char speed_configured[MAXSTR];
	char voltage_minimum[MAXSTR];
	char voltage_maximum[MAXSTR];
	char voltage_configured[MAXSTR];
} DmidecodeMemoryData;

typedef struct
{
	int ret;
	uint8_t stick_count;
	DmidecodeBiosData bios;
	DmidecodeMBData mb;
	DmidecodeCPUData processor;
	DmidecodeMemoryData *memory;
} DmidecodeData;


#endif /* _DAEMON_H_ */
