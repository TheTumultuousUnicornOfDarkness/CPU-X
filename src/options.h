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
* FILE options.h
*/

#ifndef _OPTIONS_H_
#define _OPTIONS_H_


#define OPT_GTK			(1 << 0)
#define OPT_NCURSES		(1 << 1)
#define OPT_DUMP		(1 << 2)
#define OPT_DMIDECODE		(1 << 3)
#define OPT_VERBOSE		(1 << 4)


extern unsigned int flags;


#endif /* _OPTIONS_H_ */
