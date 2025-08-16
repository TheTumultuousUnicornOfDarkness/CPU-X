/*============================================================================
  Dictionary, an object-oriented C immutable dictionary class.
  Copyright (C) 2023 by Zack T Smith.

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

#include "Dictionary.h"
#include "MutableArray.h"
#include "Log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DictionaryClass *_DictionaryClass = NULL;

void Dictionary_print (Dictionary* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Dictionary);

	if (!file) {
		file = stdout;
	}

	for (unsigned i=0; i < self->nBuckets; i++) {
		DictionaryListItem *item = self->buckets[i];
		while (item) {
			$(item->key, print, file);
			fprintf (file, "=");
			$(item->value, print, file);
			fprintf (file, "\n");
			item = item->next;
		}
	}
}

bool Dictionary_equals (Dictionary* self, void *other_)
{
	if (!self || !other_) {
		return false;
	}
	verifyCorrectClass(self,Dictionary);
	return false;
}

void Dictionary_describe (Dictionary* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Dictionary);
	
	unsigned nPairs = $(self, count);
	fprintf (file ?: stdout, "%s(%u pairs, %u buckets)\n", $(self, className), 
		nPairs, (unsigned)self->nBuckets);
}

void Dictionary_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,Dictionary);
	
	Dictionary *self = (Dictionary*) self_;

	if (!self->buckets) {
		return;
	}

	for (unsigned i=0; i < self->nBuckets; i++) {
		DictionaryListItem *item = self->buckets[i];
		while (item) {
			DictionaryListItem *next = item->next;
			release (item->key);
			release (item->value);
			ooc_free (item);
			item = next;
		}
	}
}

unsigned Dictionary_count (Dictionary* self)
{
        DEBUG_DESTROY;

	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Dictionary);
	
	if (!self->buckets) {
		return 0;
	}

	unsigned count = 0;
	for (unsigned i=0; i < self->nBuckets; i++) {
		DictionaryListItem *item = self->buckets[i];
		while (item) {
			count++;
			item = item->next;
		}
	}

	return count;
}

Dictionary* Dictionary_init (Dictionary* self)
{
	ENSURE_CLASS_READY(Dictionary);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _DictionaryClass;
		self->nBuckets = 127; // DictionarySizeSmall
		size_t nBytes = sizeof(DictionaryListItem*) * self->nBuckets;
		self->buckets = (DictionaryListItem**) malloc(nBytes);
		ooc_bzero (self->buckets, nBytes);
	}
	return self;
}

Dictionary* Dictionary_initWith (Dictionary* self, String *key, Object *value)
{
	ENSURE_CLASS_READY(Dictionary);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _DictionaryClass;
		self->nBuckets = 127;
		size_t nBytes = sizeof(DictionaryListItem*) * self->nBuckets;
		self->buckets = (DictionaryListItem**) malloc(nBytes);
		ooc_bzero (self->buckets, nBytes);

		unsigned hash = $(key, hash);
		unsigned index = hash % self->nBuckets;
		DictionaryListItem* item = (DictionaryListItem*) malloc(sizeof(DictionaryListItem));
		item->key = retain(key);
		item->value = retain(value);
		item->next = NULL;
		self->buckets[index] = item;
	}
	return self;
}

Dictionary* Dictionary_newWith (String *key, Object *value)
{
	Dictionary *dict = allocate(Dictionary);
	return Dictionary_initWith (dict, key, value);
}

Array *Dictionary_keys (Dictionary *self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Dictionary);

	MutableArray *array = new(MutableArray);
	for (unsigned i=0; i < self->nBuckets; i++) {
		DictionaryListItem *item = self->buckets[i];
		while (item) {
			String *key = item->key;
			$(array, append, key);
			item = item->next;
		}
	}

	Array *result = Array_withArray (array);
	release (array);
	return result;
}

Object *Dictionary_object (Dictionary *self, String *key)
{
	if (!self || !key) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Dictionary);
	verifyCorrectClassOrSubclass(key,String);

	unsigned hash = $(key, hash);
	unsigned index = hash % self->nBuckets;

	DictionaryListItem *item = self->buckets[index];
	while (item) {
		if ($(key, equals, item->key)) {
			return item->value;
		}
		item = item->next;
	}

	return NULL;
}

DictionaryClass* DictionaryClass_init (DictionaryClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Dictionary,describe);
        SET_OVERRIDDEN_METHOD_POINTER(Dictionary,destroy);
        SET_OVERRIDDEN_METHOD_POINTER(Dictionary,print);

        SET_METHOD_POINTER(Dictionary,object);
	SET_METHOD_POINTER(Dictionary,keys);
	SET_METHOD_POINTER(Dictionary,count);

	VALIDATE_CLASS_STRUCT(class);
	return class;
}

