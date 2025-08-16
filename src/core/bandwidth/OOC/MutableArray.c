/*============================================================================
  MutableArray, an object-oriented C mutable array class.
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

MutableArrayClass *_MutableArrayClass = NULL;

void MutableArray_destroy (Any *self)
{
	DEBUG_DESTROY;
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);
	Array_destroy (self);
}
	
static void MutableArray_describe (MutableArray* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self, MutableArray);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%s", $(self, className));
	fprintf (outputFile, "(%lu)", (unsigned long) self->total);
}

static void MutableArray_removeAt (MutableArray* self, unsigned index) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	if (!self->array)
		return;

	size_t count = self->total;
	if (index >= count)
		return;

	if (index == count-1) {
		Object *object = self->array [count-1];
		release(object);
		self->array [count-1] = NULL;
		self->total = count-1;
		return;
	}

	Object *object = self->array [index];
	if (object)
		release(object);

	int i = index;
	while (i <= count-2) {
		self->array [i] = self->array [i+1];
		i++;
	}
	self->array [i+1] = NULL;
	self->total = count-1;
}

static void MutableArray_remove (MutableArray* self, Any *object_) 
{ 
	if (!self) {
		return;
	}
	if (!object_)
		return;
	verifyCorrectClassOrSubclass(self,MutableArray);

	if (!isObject(object_)) 
		error_not_an_object(__FUNCTION__);

	Object *object = object_;

	size_t count = self->total;
	if (!count)
		return;

	size_t where = 0;
	bool found = false;

	for (int index=0; index < count; index++) {
		if (object == self->array[index]) {
			where = index;
			found = true;
			break;
		}
	}
	if (!found) {
		warning (__FUNCTION__, "OBJECT NOT IN ARRAY\n");
		return;
	}

	MutableArray_removeAt (self, where);
}

static void MutableArray_removeFirst (MutableArray* self) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	MutableArray_removeAt (self, 0);
}

static void MutableArray_removeLast (MutableArray* self) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	size_t count = self->total;
	if (!count)
		return;
	
	MutableArray_removeAt (self, count-1);
}

static int quicksort_compare(const void* a, const void* b)
{
	Object **ptr1 = (Object**) a;
	Object **ptr2 = (Object**) b;
	Object *obj1 = (Object*) *ptr1;
	Object *obj2 = (Object*) *ptr2;
	return $(obj1, compare, (Any*) obj2);
}

static void MutableArray_quicksort (MutableArray* self) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	size_t count = self->total;
	if (count <= 1) {
		return;
	}

	qsort (self->array, count, sizeof(Object*), quicksort_compare);
}

static void MutableArray_reverse (MutableArray* self) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	size_t count = self->total;
	if (count <= 1) {
		return;
	}

	size_t i = 0;
	size_t j = count-1;
	while (i < j) {
		void *object1 = self->array[i];
		void *object2 = self->array[j];
		self->array[i++] = object2;
		self->array[j--] = object1;
	}
}

static void MutableArray_randomize (MutableArray* self) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	size_t count = self->total;
	if (count <= 1) {
		return;
	}

	for (size_t i = 0; i < count; i++) {
		size_t j = rand() % count;
		if (i != j) {
			void *object1 = self->array[i];
			void *object2 = self->array[j];
			self->array[i] = object2;
			self->array[j] = object1;
		}
	}
}

static void MutableArray_removeAll (MutableArray* self) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	Object **array = self->array;
	if (!self->array) {
		return;
	}

	size_t count = self->total;
	if (count) {
		for (unsigned i = 0; i < count; i++) {
			Object *object = array[i];
			release(object);
			array[i] = NULL;
		}
	}
	self->total = 0; 

	size_t nBytes = sizeof(Object*) * self->size;
	if (nBytes) {
		ooc_bzero ((void*) self->array, nBytes);
	}
}

#if 0
// TODO
static void MutableArray_printJSON (MutableArray *self, FILE *outputFile)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	if (!outputFile) {
		outputFile = stdout;
	}

	if (!self->array) {
		return;
	}

	fputc ('[', outputFile);
	for (size_t index = 0; index < self->total; index++) {
		Object *object = self->array[index];
		if (!object) {
        		fprintf (outputFile, "null");
		}
		else {
			$(object, print, outputFile);
		}
		
		if (index != self->total-1) {
			fprintf (outputFile, ", ");
		}
	}
	fputc (']', outputFile);
}
#endif

static void MutableArray_append (MutableArray* self, Any *object_)
{
	if (!self) {
		return;
	}
	if (!object_)
		return;
	verifyCorrectClassOrSubclass(self,MutableArray);

	Object *object = object_;

	if (!isObject(object)) {
$(object, print,0);
		error_not_an_object(__FUNCTION__);
	}

	retain(object);

	size_t count = self->total;
	if (count >= self->size) {
		self->size *= 2;
		self->array = realloc (self->array, self->size * sizeof(Object*));
	}

	self->array [count++] = object;
	self->total = count;
}

static void allocateInitialArray (MutableArray* self)
{
	if (!self || self->array)
		return;

#define DEFAULT_ARRAY_SIZE (32)
	self->size = DEFAULT_ARRAY_SIZE;
	self->total = 0;
	size_t nBytes = sizeof(Object*) * self->size;
	self->array = (Object**) ooc_alloc_memory (nBytes);
	if (self->array == NULL) {
		self->size = 0;
	}
}

static void MutableArray_insertAt (MutableArray* self, Any* object_, unsigned index) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	// XX Maybe allow NULLs within array later.
	if (!object_)
		error_null_parameter (__FUNCTION__);

	if (!isObject(object_))
		error_not_an_object(__FUNCTION__);

	Object *object = object_;

	size_t count = self->total;
	if (index >= count) {
		MutableArray_append (self, object);
		return;
	} 
	if (index < 0) {
		index = 0;
	}

	retain(object);

	if (1+count >= self->size) {
		self->size *= 2;
		self->array = realloc (self->array, self->size * sizeof(Object*));
	}

	for (unsigned i = count; i > index; i--)
		self->array[i] = self->array[i-1];

	self->array[index] = object;
	self->total = count + 1;
}

static void MutableArray_prepend (MutableArray* self, Any* object) 
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,MutableArray);

	MutableArray_insertAt (self, object, 0);
}

MutableArrayClass* MutableArrayClass_init (MutableArrayClass *class)
{
        SET_SUPERCLASS(Array);

        SET_OVERRIDDEN_METHOD_POINTER(MutableArray,describe);
        SET_OVERRIDDEN_METHOD_POINTER(MutableArray,destroy);

	SET_METHOD_POINTER(MutableArray,append);
	SET_METHOD_POINTER(MutableArray,prepend);
	SET_METHOD_POINTER(MutableArray,remove);
	SET_METHOD_POINTER(MutableArray,removeAt);
	SET_METHOD_POINTER(MutableArray,removeAll);
	SET_METHOD_POINTER(MutableArray,removeFirst);
	SET_METHOD_POINTER(MutableArray,removeLast);
	SET_METHOD_POINTER(MutableArray,insertAt);
	SET_METHOD_POINTER(MutableArray,reverse);
	SET_METHOD_POINTER(MutableArray,randomize);
	SET_METHOD_POINTER(MutableArray,quicksort);

	VALIDATE_CLASS_STRUCT(class);
        return class;
}

MutableArray *MutableArray_init (MutableArray* self) 
{
	ENSURE_CLASS_READY(MutableArray);

	if (!self) {
		return NULL;
	}

	Array_init ((Array*) self);
	self->is_a = _MutableArrayClass;

	allocateInitialArray(self);

	return self;
}

MutableArray *MutableArray_new ()
{
        return new(MutableArray);
}

