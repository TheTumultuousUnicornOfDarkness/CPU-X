/*============================================================================
  Data, an object-oriented C immutable byte array Data class.
  Copyright (C) 2021-2022 by Zack T Smith.

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

#ifndef _OOC_DATA_H
#define _OOC_DATA_H

#include <stdbool.h>
#include <stdint.h>

#include "Object.h"

#define DECLARE_DATA_METHODS(TYPE_POINTER) \
	uint8_t *(*bytes) (TYPE_POINTER); \
	size_t (*length) (TYPE_POINTER); \
	uint8_t (*byteAt) (TYPE_POINTER, size_t); \
	char *(*asCString) (TYPE_POINTER); /* Caller frees the string */ \
	unsigned long (*sum) (TYPE_POINTER); \
	bool (*writeToFilePath) (TYPE_POINTER, const char *path); 

struct dataobject;

typedef struct dataobjectclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct dataobject*)
	DECLARE_DATA_METHODS(struct dataobject*)
} DataClass;

extern DataClass *_DataClass;
extern DataClass* DataClass_init (DataClass*);

#define DECLARE_DATA_INSTANCE_VARS(TYPE_POINTER) \
	size_t nBytes; \
	uint8_t *bytes; \
	bool isMemoryMappedFile; 

typedef struct dataobject {
	DataClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct dataobject*)
	DECLARE_DATA_INSTANCE_VARS(struct dataobject*)
} Data;

extern Data* Data_init (Data* object);
extern Data* Data_initWithData (Data* self, void *other);
extern Data* Data_initFromFilePath (Data* self, const char *path);
extern Data* Data_initWithUTF8String (Data* self, const char *);
extern Data* Data_withData (Any *other);
extern Data* Data_withBytes (uint8_t *bytes, size_t length);
extern void Data_destroy (Any *);

// Inherited by mutable data
extern void Data_describe (Data *, FILE *);
extern void Data_print (Data* self, FILE *);
extern bool Data_equals (Data* self, void *other);
extern size_t Data_length (Data *self);
extern uint8_t *Data_bytes (Data *self);

#endif

