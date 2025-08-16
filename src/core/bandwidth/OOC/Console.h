/*============================================================================
  Console, an object-oriented C console I/O class.
  Copyright (C) 2019 by Zack T Smith.

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

#ifndef _OOC_CONSOLE_H
#define _OOC_CONSOLE_H

#include <stdbool.h>
#include <math.h>

#include "Object.h"
#include "String.h"

#define DECLARE_CONSOLE_INSTANCE_VARS(FOO) 

#define DECLARE_CONSOLE_METHODS(TYPE_POINTER) \
	void (*newline) (TYPE_POINTER); \
	void (*flush) (TYPE_POINTER); \
	void (*puts) (TYPE_POINTER, const char*); \
	void (*printInt) (TYPE_POINTER, int); \
	void (*printUnsigned) (TYPE_POINTER, unsigned); \
	void (*printf) (TYPE_POINTER, const char* fmt, ...); \
	void (*printObject) (TYPE_POINTER, Any*);

struct console;

typedef struct consoleclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct console*)
        DECLARE_CONSOLE_METHODS(struct console*)
} ConsoleClass;

extern ConsoleClass *_ConsoleClass;
extern ConsoleClass* ConsoleClass_init (ConsoleClass*);

typedef struct console {
        ConsoleClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct console*)
	DECLARE_CONSOLE_INSTANCE_VARS(struct console*)
} Console;

extern Console *Console_singleton ();
extern Console *Console_new ();
extern void Console_destroy (Any*);
extern Console *Console_init (Console *self);

#endif
