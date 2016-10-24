/****************************************************************************
*    Copyright © 2014-2016 Xorg
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
* FILE libdmi.h
*/

#ifndef _LIBDMI_H_
#define _LIBDMI_H_

#include "../cpu-x.h"

#define PROC_PACKAGE 0
#define PROC_BUS     1


enum EnDmidecode
{
	DMI_CPU, DMI_MB, DMI_RAM, LASTDMI
};

extern char **dmidata[LASTDMI][16];

int dmidecode(void);


#endif