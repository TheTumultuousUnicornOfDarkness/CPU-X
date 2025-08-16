/*============================================================================
  StackTemplate, an object-oriented C templated mutable array class.
  Copyright (C) 2022 by Zack T Smith.

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

#if defined(StackType) && defined(ValueType)

#define StackClassType EVALUATING_CONCAT(StackType,Class)

StackClassType *EVALUATING_CONCAT(_,StackClassType) = NULL;

#include "Log.h"
#include "Utility.h"
#include "GraphicsTypes.h"

#include <stdlib.h>

static void reallocateWithSize (StackType *self, unsigned newSize)
{
	if (!self) {
		return;
	}
	if (newSize <= self->allocatedSize)
		return;

	size_t nBytes = sizeof(ValueType) * newSize;
	if (self->array) {
		self->array = (ValueType*) realloc (self->array, nBytes);
	}
	else {
		self->array = (ValueType*) ooc_alloc_memory(nBytes);
	}

	if (self->array) 
		self->allocatedSize = newSize;
	else 
		self->allocatedSize = 0;
}

static void EVALUATING_CONCAT(StackType,_describe) (StackType* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self, StackType);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%s", $(self, className));
	fprintf (outputFile, "(%lu)", (unsigned long) self->count);
}

static ValueType EVALUATING_CONCAT(StackType,_tos) (StackType* self) 
{
	if (!self) {
		return (ValueType)0LU;
	}
	verifyCorrectClass(self, StackType);

	int count = self->count;
	if (count < 1)
		return (ValueType)0LU;

	return (ValueType) self->array[count-1];
}

static ValueType EVALUATING_CONCAT(StackType,_pop) (StackType* self) 
{
	if (!self) {
		return (ValueType)0LU;
	}
	verifyCorrectClass(self, StackType);

	int count = self->count;
	if (count < 1)
		return (ValueType)0LU;
	--count;
	self->count = count;
	return (ValueType) self->array[count];
}

static void EVALUATING_CONCAT(StackType,_empty) (StackType* self) 
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self, StackType);

	self->count = 0;
}

static void EVALUATING_CONCAT(StackType,_push) (StackType* self, ValueType value) 
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self, StackType);

	if (self->count >= self->allocatedSize) {
		unsigned newSize = 2 * self->allocatedSize;
		reallocateWithSize (self, newSize);
	}

	if (self->array) {
		unsigned index = self->count;
		self->array[index] = value;
		self->count = index+1;
	}
}

static bool EVALUATING_CONCAT(StackType,_isEmpty) (StackType* self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClass(self, StackType);

	return self->count == 0;
}

static void EVALUATING_CONCAT(StackType,_swap) (StackType* self) 
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self, StackType);

	size_t count = self->count;
	if (count < 2)
		return;

	unsigned top = count-1;
	unsigned next = count-2;
	ValueType value1 = self->array[top];
	ValueType value2 = self->array[next];
	self->array[top] = value2;
	self->array[next] = value1;
}

void EVALUATING_CONCAT(StackType,_destroy) (Any *self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClass(self_,StackType);

	StackType *self = self_;

	ValueType *array = self->array;
	if (array) {
		ooc_free (array);
		self->array = NULL;
		self->count = 0;
		self->allocatedSize = 0;
	}

	Object_destroy (self);
}

StackClassType* EVALUATING_CONCAT(StackClassType,_init) (StackClassType *class)
{
        SET_SUPERCLASS(Object);

        SET_OVERRIDDEN_METHOD_POINTER(StackType,describe);
        SET_OVERRIDDEN_METHOD_POINTER(StackType,destroy);
        SET_OVERRIDDEN_METHOD_POINTER(StackType,print);

	SET_INHERITED_METHOD_POINTER(StackType,Object,equals); // TO DO

	SET_METHOD_POINTER(StackType,empty);
	SET_METHOD_POINTER(StackType,isEmpty);
	SET_METHOD_POINTER(StackType,push);
	SET_METHOD_POINTER(StackType,pop);
	SET_METHOD_POINTER(StackType,tos);
	SET_METHOD_POINTER(StackType,swap);

	VALIDATE_CLASS_STRUCT(class);
        return class;
}

StackType* EVALUATING_CONCAT(StackType,_init) (StackType* self)
{
	ENSURE_CLASS_READY(StackType);

	if (!self) {
		return NULL;
	}

	Object_init ((Object*) self);
	self->is_a = EVALUATING_CONCAT(_,StackClassType);

#ifdef DEBUG
	self->count = 0;
	self->allocatedSize = 0;
	self->array = NULL;
#endif

	reallocateWithSize (self, STACK_DEFAULT_COUNT);

	return self;
}

StackType* EVALUATING_CONCAT(StackType,_new) (void)
{
        return new(StackType);
}

StackType* EVALUATING_CONCAT(StackType,_newWithCount) (size_t count) 
{
	return EVALUATING_CONCAT(StackType,_init) (allocate(StackType));
}

#endif
