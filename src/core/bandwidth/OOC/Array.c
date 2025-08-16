/*============================================================================
  Array, an object-oriented C array class.
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

#include "Array.h"
#include "MutableArray.h"

ArrayClass *_ArrayClass = NULL;

size_t Array_count (Array *self) 
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Array);
	
	return self->total; 
}

Any *Array_at (Array *self, long index) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Array);

	if (index < 0 || index >= self->total)
		return NULL;
	return self->array [index];
}

Array *Array_withArray (Any* array_)
{
	Array *self = Array_new ();

	if (array_ && isMemberOfClassOrSubclass(array_, Array)) {

		Array *array = array_;

		size_t n = self->total = self->size = Array_count (array);
		unsigned nBytes = sizeof(Any*) * self->size;
		self->array = (Object**) malloc(nBytes);
		ooc_bzero (self->array, nBytes);

		for (long i=0; i < n ; i++) {
			Object *object = Array_at (array, i);
			retain(object);
			self->array[i] = object;
		}
	}
	return self;
}

Any *Array_first (Array* self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Array);
	if (!self->total)
		return NULL;

	return self->array [0];
}

Any *Array_last (Array *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Array);

	if (!self->total)
		return NULL;
	return self->array [self->total - 1];
}

bool Array_contains (Array *self, Any *object_) 
{
	if (!self || !object_) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,Array);
	if (!isObject(object_)) {
		// XX
		return false;
	}

	size_t n = self->total;
	if (!n) {
		return false;
	}

	Object *object = object_;

	for (size_t i=0; i < n; i++) {
		if ($(object, equals, self->array [i])) {
			return true;
		}
	}
	return false;
}

void Array_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,Array);

	Array *self = self_;
	Object **array = self->array;
	if (self->size && array) {
		for (unsigned index = 0; index < self->total; index++) {
			Object *object = self->array[index];
			release (object);
		}
		ooc_free (array);
		self->array = NULL;
		self->size = 0;
		self->total = 0;
	}
}

static void Array_describe (Array* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Array);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%s", $(self, className));
	fprintf (outputFile, "(%lu)", (unsigned long) self->total);
}

void Array_print (Array* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Array);

	if (!outputFile)
		outputFile = stdout;

	for (int i=0; i < self->total; i++) {
		Object *object = Array_at (self, i);
		if (object) {
			$(object, print, outputFile);
			fputc (' ', outputFile);
		}
	}
}

bool Array_equals (Array* self, Any *other_)
{ 
	if (!self || !other_)
		return false;
	verifyCorrectClassOrSubclass(self,Array);
	Array *other = (Array*)other_;
	verifyCorrectClassOrSubclass(other,Array);

	if (self->total != other->total)
		return false;

	for (int i=0; i < self->total; i++) {
		Object *first = Array_at (self, i);
		Object *second = Array_at (other, i);

		if (!first || !second) 
			return false;
		if (first->is_a != second->is_a)
			return false;
		if (!$(first, equals, second))
			return false;
	}
	return true;
}

ArrayClass* ArrayClass_init (ArrayClass *class)
{
	SET_SUPERCLASS(Object);

	// Overridden methods
	SET_OVERRIDDEN_METHOD_POINTER(Array,describe);
	SET_OVERRIDDEN_METHOD_POINTER(Array,destroy);
	SET_OVERRIDDEN_METHOD_POINTER(Array,print);
	SET_OVERRIDDEN_METHOD_POINTER(Array,equals);

	// Array methods
	SET_METHOD_POINTER(Array,count);
	SET_METHOD_POINTER(Array,first);
	SET_METHOD_POINTER(Array,last);
	SET_METHOD_POINTER(Array,at);
	SET_METHOD_POINTER(Array,contains);

	VALIDATE_CLASS_STRUCT(class);

        return class;
}

Array* Array_init (Array *self)
{
        ENSURE_CLASS_READY(Array);

	Object_init ((Object*) self);
	self->is_a = _ArrayClass;

	self->size = 0;
	self->total = 0;
	self->array = NULL;

	return self;
}

Array *Array_new () 
{
	Array *self = new(Array);
	return self;
}

Array *Array_withObject (Any *object_) 
{
	Array *self = Array_new ();
	self->size = 1;
	self->total = 1;

	unsigned long nBytes = sizeof(Any*);
	self->array = (Object**) calloc(nBytes, 1);

	Object *object = (Object*)object_;
	if (object) {
		self->array[0] = object;
		retain(object);
	}
	return self;
}

