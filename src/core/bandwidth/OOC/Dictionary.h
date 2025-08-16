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

#ifndef _OOC_DICTIONARY_H
#define _OOC_DICTIONARY_H

#include "Object.h"
#include "String.h"

typedef struct dictionary_list_item {
	String *key;
	Object *value;
	struct dictionary_list_item *next;
} DictionaryListItem;

typedef enum {
	DictionarySizeSmall = 0, // small number of buckets
	DictionarySizeMedium = 1, // medium number of buckets
	DictionarySizeLarge = 2, // large number of buckets
} DictionarySize;

#define DECLARE_DICTIONARY_METHODS(TYPE_POINTER) \
	Object *(*object) (TYPE_POINTER, String* /* key */); \
	Array *(*keys) (TYPE_POINTER); \
	unsigned (*count) (TYPE_POINTER); 

struct dictionary;

typedef struct dictionaryclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct dictionary*)
	DECLARE_DICTIONARY_METHODS(struct dictionary*)
} DictionaryClass;

extern DictionaryClass *_DictionaryClass;
extern DictionaryClass* DictionaryClass_init (DictionaryClass*);

#define DECLARE_DICTIONARY_INSTANCE_VARS(TYPE_POINTER) \
	unsigned nBuckets; \
	DictionaryListItem **buckets;

typedef struct dictionary {
	DictionaryClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct dictionary*)
	DECLARE_DICTIONARY_INSTANCE_VARS(struct dictionary*)
} Dictionary;

extern Dictionary* Dictionary_init (Dictionary*);
extern Dictionary* Dictionary_newWith (String*, Object*); // TODO Need variadic variant.
extern void Dictionary_destroy (Any *);

#endif

