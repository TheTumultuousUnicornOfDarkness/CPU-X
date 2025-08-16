/*============================================================================
  MutableData, an object-oriented C mutable byte array class.
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

#ifndef _OOC_MUTABLEDATA_H
#define _OOC_MUTABLEDATA_H

#include <stdint.h>
#include <wchar.h>

#include "Object.h"
#include "Data.h"

struct mutabledata;

#define DECLARE_MUTABLEDATA_METHODS(TYPE_POINTER) \
	void (*appendData) (TYPE_POINTER, void*); \
	void (*appendByte) (TYPE_POINTER, uint8_t); \
	void (*appendBytes) (TYPE_POINTER, uint8_t* bytes, size_t length); \
	void (*truncateAt) (TYPE_POINTER, size_t); 

typedef struct mutabledataclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct mutabledata*)
	DECLARE_DATA_METHODS(struct mutabledata*)
	DECLARE_MUTABLEDATA_METHODS(struct mutabledata*)
} MutableDataClass;

extern MutableDataClass *_MutableDataClass;
extern MutableDataClass* MutableDataClass_init (MutableDataClass*);

#define DECLARE_MUTABLEDATA_INSTANCE_VARS(TYPE_POINTER) \
	size_t allocatedSize; 

typedef struct mutabledata {
	MutableDataClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct mutabledata*)
	DECLARE_DATA_INSTANCE_VARS(struct mutabledata*)
	DECLARE_MUTABLEDATA_INSTANCE_VARS(struct mutabledata*)
} MutableData;

extern MutableData* MutableData_init (MutableData* object);
extern void MutableData_destroy (Any *);
extern MutableData* MutableData_withBytes (const uint8_t *bytes, size_t length);
extern MutableData* MutableData_withCString (const char *string);

#endif

