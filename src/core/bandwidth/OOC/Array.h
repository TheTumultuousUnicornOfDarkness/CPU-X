/*============================================================================
  Array, an object-oriented C array class.
  Copyright (C) 2019, 2023 by Zack T Smith.

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

#ifndef _OOC_ARRAY_H
#define _OOC_ARRAY_H

#include <stdlib.h>
#include <stdbool.h>

#include "Object.h"

#define DECLARE_ARRAY_INSTANCE_VARS(FOO) \
	size_t total;\
	size_t size;\
	Object **array;

#define DECLARE_ARRAY_METHODS(TYPE_POINTER) \
	size_t (*count) (TYPE_POINTER); \
	Any* (*at) (TYPE_POINTER, long); \
	Any* (*first) (TYPE_POINTER); \
	Any* (*last) (TYPE_POINTER); \
	bool (*contains) (TYPE_POINTER, Any *object); 

struct array;

typedef struct arrayclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct array*)
        DECLARE_ARRAY_METHODS(struct array*)
} ArrayClass;

extern ArrayClass *_ArrayClass;

typedef struct array {
        ArrayClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct array*)
	DECLARE_ARRAY_INSTANCE_VARS(struct array*)
} Array;

extern ArrayClass* ArrayClass_init (ArrayClass*);
Array *Array_new (void);
Array *Array_init (Array *self);
void Array_destroy (Any *self);
Array* Array_withObject (Any *object);
Array* Array_withArray (Any *arrayToCopy);

// These have to be listed here so that MutableArray can inherit them.
size_t Array_count (Array *);
void Array_print (Array *, FILE*);
Any* Array_at (Array *, long);
bool Array_contains (Array *self, Any* object);
Any* Array_first (Array *);
Any* Array_last (Array *);
bool Array_equals (Array *self, Any* other);

#endif
