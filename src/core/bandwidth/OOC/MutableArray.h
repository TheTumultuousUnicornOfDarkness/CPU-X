/*============================================================================
  MutableArray, an object-oriented C mutable array class.
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

#ifndef _OOC_MUTABLEARRAY_H
#define _OOC_MUTABLEARRAY_H

#include "Object.h"
#include "Array.h"

#define DECLARE_MUTABLE_ARRAY_INSTANCE_VARS(TYPE_POINTER)  /* Nothing additional required */

#define DECLARE_MUTABLE_ARRAY_METHODS(TYPE_POINTER) \
	void (*append) (TYPE_POINTER, Any *object); \
	void (*prepend) (TYPE_POINTER, Any *object); \
	void (*insertAt) (TYPE_POINTER, Any*, unsigned index); \
	void (*remove) (TYPE_POINTER, Any*); \
	void (*removeAt) (TYPE_POINTER, unsigned index); \
	void (*removeFirst) (TYPE_POINTER); \
	void (*removeLast) (TYPE_POINTER); \
	void (*removeAll) (TYPE_POINTER); \
	void (*reverse) (TYPE_POINTER); \
	void (*quicksort) (TYPE_POINTER); \
	void (*randomize) (TYPE_POINTER); 

struct mutablearray;

typedef struct mutablearrayclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct mutablearray*)
        DECLARE_ARRAY_METHODS(struct mutablearray*)
        DECLARE_MUTABLE_ARRAY_METHODS(struct mutablearray*)
} MutableArrayClass;

extern MutableArrayClass *_MutableArrayClass;
extern MutableArrayClass* MutableArrayClass_init (MutableArrayClass*);

typedef struct mutablearray {
	MutableArrayClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct mutablearray*)
	DECLARE_ARRAY_INSTANCE_VARS(struct mutablearray*)
	DECLARE_MUTABLE_ARRAY_INSTANCE_VARS(struct mutablearray*)
} MutableArray;

extern MutableArray *MutableArray_new ();
extern void MutableArray_destroy (Any *);
extern MutableArray *MutableArray_init (MutableArray* self);
extern MutableArray* MutableArray_withObject (Any *object);
extern MutableArray* MutableArray_withArray (MutableArray *arrayToCopy);

#endif
