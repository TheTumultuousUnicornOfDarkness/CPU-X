/****************************************************************************
*    Copyright © 2014-2015 Xorg
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
* FILE libdmi.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "libdmi.h"
#include "types.h"
#include "dmiopt.h"


/* Options are global */
struct opt opt;
char **dmidata[LASTRAM];


static u8 *dmiparse(u8 *p, int l)
{
	/* Allocate memory on first call only */
	if (p == NULL)
	{
		p = (u8 *)calloc(256, sizeof(u8));
		if (p == NULL)
		{
			perror("calloc");
			return NULL;
		}
	}

	p[l] = 1;
	return p;
}

int libdmi(char c)
{
	int err = 0;

	/* Dmidecode options */
	opt.flags = 0;
	opt.type = NULL;
	if(!opts->verbose)
		opt.flags |= FLAG_QUIET;

	switch(c)
	{
		case 'c':
			opt.flags |= FLAG_CPU_X;
			opt.type = dmiparse(opt.type, 4);
			break;
		case 'm':
			opt.flags |= FLAG_CPU_X;
			opt.type = dmiparse(opt.type, 0);
			opt.type = dmiparse(opt.type, 2);
			break;
		case 'r':
			opt.flags |= FLAG_CPU_X;
			opt.type = dmiparse(opt.type, 17);
			break;
		case 'D':
			break;
		default:
			return -1;
	}
	err = maindmi();

	return err;
}
