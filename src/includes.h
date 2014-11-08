/****************************************************************************
*    Copyright © 2014 Xorg
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
* includes.h
*/

#ifndef _INCLUDES_H_
#define _INCLUDES_H_


#ifdef GTK
# define HAS_GTK 1
# include "gui_gtk.h"
#else
# define HAS_GTK 0
#endif

#ifdef NCURSES
# define HAS_NCURSES 1
# include "gui_ncurses.h"
#else
# define HAS_NCURSES 0
#endif

#ifdef LIBCPUID
# define HAS_LIBCPUID 1
# include <libcpuid/libcpuid.h>
#else
# define HAS_LIBCPUID 0
#endif

#ifdef LIBDMI
# define HAS_LIBDMI 1
# include "dmidecode/libdmi.h"
#else
# define HAS_LIBDMI 0
#endif


#endif /*_INCLUDES_H_*/
