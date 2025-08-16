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

#define StackStructType EVALUATING_CONCAT(StackType,_s)
#define StackClassType EVALUATING_CONCAT(StackType,Class)
#define StackClassStructType EVALUATING_CONCAT(StackType,Class_s)

#include <stdint.h>

#define STACK_DEFAULT_COUNT (16)

#include "ObjectOriented.h"
#include "Object.h"

#define DECLARE_STACKTEMPLATE_INSTANCE_VARS(TYPE_POINTER) \
	ValueType *array; \
	size_t allocatedSize; \
	size_t count;

#define DECLARE_STACKTEMPLATE_METHODS(TYPE_POINTER) \
	bool (*isEmpty) (TYPE_POINTER); \
	void (*push) (TYPE_POINTER, ValueType); \
	ValueType (*tos) (TYPE_POINTER); /* top of stack item */ \
	ValueType (*pop) (TYPE_POINTER); \
	void (*swap) (TYPE_POINTER); \
	void (*empty) (TYPE_POINTER); 

struct StackStructType;

typedef struct StackClassStructType {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct StackStructType*)
        DECLARE_STACKTEMPLATE_METHODS(struct StackStructType*)
} StackClassType;

typedef struct StackStructType {
	StackClassType *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct StackStructType*)
	DECLARE_STACKTEMPLATE_INSTANCE_VARS(struct StackStructType*)
} StackType;

extern StackClassType * EVALUATING_CONCAT(_,StackClassType) ;
extern StackClassType * EVALUATING_CONCAT(StackClassType,_init) (StackClassType*) ;

extern StackType * EVALUATING_CONCAT(StackType,_new) ();
extern StackType * EVALUATING_CONCAT(StackType,_init) (StackType*);
extern StackType * EVALUATING_CONCAT(StackType,_initWithCount) (StackType*, size_t);
extern StackType * EVALUATING_CONCAT(StackType,_newWithCount) (size_t);
extern void EVALUATING_CONCAT(StackType,_destroy) (Any *);

// No subclassing allowed.
#undef DECLARE_STACKTEMPLATE_INSTANCE_VARS
#undef DECLARE_STACKTEMPLATE_METHODS
#undef StackClassStructType
#undef StackClassType
#undef StackStructType
#undef StackType
#undef ValueType

#endif
