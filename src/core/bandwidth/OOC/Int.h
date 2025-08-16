/*============================================================================
  Int, an object-oriented C integer class.
  Copyright (C) 2019, 2022 by Zack T Smith.

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

#ifndef _OOC_INT_H
#define _OOC_INT_H

#include <stdbool.h>
#include <math.h>

#include "Object.h"

#define DECLARE_INT_INSTANCE_VARS(FOO) \
	long long integer;

#define DECLARE_INT_METHODS(TYPE_POINTER) \
	int (*asInt) (TYPE_POINTER); \
	long long (*asLongLong) (TYPE_POINTER); \
	bool (*asBool) (TYPE_POINTER); \
	double (*asDouble) (TYPE_POINTER); \
	const char* (*asCString) (TYPE_POINTER);

struct integer;

typedef struct integerclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct integer*)
        DECLARE_INT_METHODS(struct integer*)
} IntClass;

extern IntClass *_IntClass;
extern IntClass* IntClass_init (IntClass*);

typedef struct integer {
        IntClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct integer*)
	DECLARE_INT_INSTANCE_VARS(struct integer*)
} Int;

extern Int *Int_new ();
extern void Int_destroy (Any *);
extern Int *Int_init (Int *self);
extern Int* Int_withBool (bool);
extern Int* Int_withUnsignedLong (unsigned long value);
extern Int* Int_withLongLong (long long value);
extern Int* Int_withDouble (double value);
extern Int* Int_fromCString (const char*);
extern Int* Int_fromFile (const char* path);

// Convenience instantiator macro
#define _Int(VALUE) Int_withLongLong((long long)VALUE)

#endif
