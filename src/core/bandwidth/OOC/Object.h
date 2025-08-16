/*============================================================================
  Object, an object-oriented C base object class.
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

#ifndef _OOC_OBJECT_H
#define _OOC_OBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "ObjectOriented.h"

struct object;

#ifdef __clang__
	#if __has_extension(blocks)
		// Future support for blocks
	#endif
#endif

extern uint64_t g_totalObjectAllocations;
extern uint64_t g_totalObjectDeallocations;

#define DECLARE_OBJECT_INSTANCE_VARS(TYPE_POINTER) \
	unsigned magic : 31; \
	unsigned temporary : 1; \
	int32_t retainCount; 

#define DECLARE_OBJECT_METHODS(TYPE_POINTER) \
	void *(*class) (TYPE_POINTER); \
	const char *(*className) (TYPE_POINTER); \
	void (*destroy) (Any*); \
	unsigned (*hash) (TYPE_POINTER); \
	void (*describe) (TYPE_POINTER, FILE*); \
	bool (*equals) (TYPE_POINTER, Any*); \
	void (*print) (TYPE_POINTER, FILE*); \
	int (*compare) (TYPE_POINTER, Any*); \
	long (*message) (TYPE_POINTER, long message, Any *sender, long, long);

#define DECLARE_OBJECT_CLASS_VARS DECLARE_EMPTY_CLASS_VARS

typedef struct object_class {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct object*)
} ObjectClass;

extern ObjectClass *_ObjectClass;
extern ObjectClass* ObjectClass_init (ObjectClass*);

typedef struct object {
	ObjectClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct object*)
} Object;

extern Object* Object_init (Object *object);
extern void Object_destroy (Any *object);
extern void Object_print (Object* self, FILE *file);
extern bool Object_equals (Object *self, Any *other);

#endif
