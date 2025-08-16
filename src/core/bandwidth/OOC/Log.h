/*============================================================================
  Log, a simple logging facility.
  Copyright (C) 2018, 2021 by Zack T Smith.

  Object-Oriented C is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  Object-Oriented C is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public License
  along with this software.  If not, see <http://www.gnu.org/licenses/>.

  The author may be reached at 1@zsmith.co.
 *===========================================================================*/

#ifndef _LOG_H
#define _LOG_H

#include "GraphicsTypes.h"

#include <stdbool.h>

extern bool Log_show_debug_output;

extern bool Log_reopen (const char *path);
extern void Log_close();

extern void Log_print (const char *message);
extern void Log_println (const char *message);

extern void Log_error (const char *whereabouts, const char *message);
extern void Log_warning (const char *whereabouts, const char *message);
extern void Log_debug (const char *whereabouts, const char *message);
extern void Log_errorNullParameter (const char *whereabouts);
extern void Log_perror (const char *whereabouts, const char *calledFunction);

extern void Log_error_printf (const char *funcName, const char *format, ...);
extern void Log_warning_printf (const char *funcName, const char *format, ...);
extern void Log_debug_printf (const char *funcName, const char *format, ...);

#endif
