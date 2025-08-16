/*============================================================================
  ObjectOriented, a facility for object-oriented programming on top of C.
  Copyright (C) 2019, 2022, 2023 by Zack T Smith.

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

  The author may be reached at 1 at zsmith dot co.
 *===========================================================================*/

#ifndef _OBJECTORIENTED_H
#define _OBJECTORIENTED_H

#define OOC_RELEASE "0.35"

#include <stdlib.h>
#include <stdbool.h>

#include "config.h"

// TODO
// * Add JPEG writer.

#if defined(__x86_64__) || defined(_WIN64) || defined(__WIN64__) || defined(__aarch64__) || __WORDSIZE == 64
 #define IS_64BIT
 #define WORDSIZE_STR "64-bit"
#else
 #define IS_32BIT
 #define WORDSIZE_STR "32-bit"
#endif

// #define GRATUITOUS_DEBUGGING

#define OBJECT_MAGIC_NUMBER (0x7ac4c0de)

#define kDefaultDPI 100

#define ONE_MEGABYTE (1 << 20)
#define ONE_MILLION (1000000)

#ifndef MAXUNSIGNED
#define MAXUNSIGNED (~(0U))
#endif

#define require(COND) if(COND){}

#define IS_FOUR_BYTE_ALIGNED(ADDR) ((3 & (unsigned long)ADDR) == 0)
#define IS_EIGHT_BYTE_ALIGNED(ADDR) ((7 & (unsigned long)ADDR) == 0)

#ifdef __ANDROID__
  #define TMPDIR "/sdcard"
#else
  #define TMPDIR "/tmp"
#endif

extern void *ooc_bzero(const void *start, size_t length);
extern size_t ooc_strlen(const char *start);
extern void ooc_strncpy(const char *dest, const char *src, size_t length);
extern void ooc_abstract_method();
extern void *ooc_alloc_memory (size_t);
extern void *ooc_allocate_class (size_t);
extern void *ooc_retain (void* ptr, const char* funcName);
extern void *ooc_allocate (const char *objectType, size_t);
extern bool ooc_release (void *object_, size_t, const char*);
extern void ooc_free (void *object_);
extern bool ooc_isMemberOfClassOrSubclass(void *, void *, const char*);
extern bool isObject(void*);
extern bool areSameClass(void*,void*);

typedef void Any;

#ifndef __arm__
  #if defined(__i386__) || defined(__x86_64__) || defined(_WIN64) || defined(_WIN32) ||defined(__WIN32__) || defined(__WIN64__) || defined(_M_IX86) || defined(__MINGW32__) || defined(__i386) || defined(__CYGWIN__)
    #define x86
  #endif
#endif

#define STRINGIFY(a) ""#a""
#define STRINGIFY_EVALUATING(EXPR) STRINGIFY(EXPR)

#ifdef DEBUG
#define nullObjectPointerError (fprintf(stderr, "NULL OBJECT POINTER for %s IN %s\n",STRINGIFY(METHOD),__FUNCTION__) & 0)
#define nullMethodPointerError (fprintf(stderr, "NULL METHOD POINTER for %s IN %s\n",STRINGIFY(METHOD),__FUNCTION__) & 0)
#define nullClassStructError (fprintf(stderr, "MISSING IS_A POINTER for %s IN %s\n",STRINGIFY(METHOD),__FUNCTION__) & 0)
#endif

#define DECLARE_EMPTY_CLASS_VARS \
        void *superClass; \
        void *unused; \
        char *_className; \
	size_t classSize;
#define TOTAL_EMPTY_CLASS_ULONGS (4)

typedef struct {
	DECLARE_EMPTY_CLASS_VARS
} EmptyClass;

#define warning(FUNC,MESSAGE) { fprintf(stderr, "Warning (%s): %s\n",FUNC, MESSAGE); }
#define error(FUNC,MESSAGE) { fprintf(stderr, "Error (%s): %s\n",FUNC, MESSAGE); exit(1); }
#define error_null_parameter(FUNC) { fprintf(stderr, "Error (%s): NULL parameter\n",FUNC); exit(2); }
#define error_bad_class(FUNC) { fprintf(stderr, "Error (%s): Incorrect is_a pointer\n",FUNC); exit(3); }
#define error_not_an_object(FUNC) { fprintf(stderr, "Error (%s): Parameter is not an object\n",FUNC); exit(4); }
#define error_double_release(FUNC) { fprintf(stderr, "Error (%s): Detected doulbe release\n",FUNC); exit(5); }

// Note, preprocessor concatenation does not evaluate the parameters!
#define CONCAT_SYMBOLS(a,b) a##b
#define CONCAT_SYMBOLS3(a,b,c) a##b##c
#define CONCAT_SYMBOLS4(a,b,c,d) a##b##c##d
#define CONCAT_SYMBOLS5(a,b,c,d,e) a##b##c##d##e

#define EVALUATING_CONCAT(a,b) CONCAT_SYMBOLS(a,b)
#define EVALUATING_CONCAT3(a,b,c) CONCAT_SYMBOLS3(a,b,c)

extern void deallocateClasses ();
extern void registerClass (const void*);

#define classNameOf(OBJ) $(OBJ, className)

/* Ensure that the current class exists. This will invoke its *Class_init routine.
 */
#define ENSURE_CLASS_READY(CLASS) do {\
  if (! CONCAT_SYMBOLS3(_,CLASS,Class)) {\
    size_t class_struct_size = sizeof(CONCAT_SYMBOLS(CLASS,Class)); \
    CONCAT_SYMBOLS3(_,CLASS,Class)=(CONCAT_SYMBOLS(CLASS,Class)*) ooc_allocate_class(class_struct_size);\
    CONCAT_SYMBOLS3(_,CLASS,Class)->_className = STRINGIFY(CLASS); \
    CONCAT_SYMBOLS3(_,CLASS,Class)->classSize = class_struct_size; \
    CONCAT_SYMBOLS3(CLASS,Class,_init) (CONCAT_SYMBOLS3(_,CLASS,Class));\
    registerClass (CONCAT_SYMBOLS3(_,CLASS,Class)); \
  }} while(false)

/* Ensure the superclass struct exists. Called from current class's *Class_init routine.
 */
#define SET_SUPERCLASS(PARENTCLASS) do {\
  ENSURE_CLASS_READY(PARENTCLASS); \
  CONCAT_SYMBOLS3(PARENTCLASS,Class,_init) ((void*)class); \
  class->superClass = CONCAT_SYMBOLS3(_,PARENTCLASS,Class); \
  } while(false)

#ifdef DEBUG
#define $(OBJ,METHOD,...) \
 (OBJ? \
    (OBJ->is_a ? \
        (OBJ->is_a->METHOD ? 	\
            (OBJ->is_a->METHOD(OBJ, ##__VA_ARGS__) \
	    ) \
            : (typeof((OBJ->is_a->METHOD(OBJ, ##__VA_ARGS__))))0  nullMethodPointerError \
         ) \
         : nullClassStructError \
     ) \
     : nullObjectPointerError)
#else
#define $(OBJ,METHOD,...) (\
		OBJ? \
		OBJ->is_a->METHOD(OBJ, ##__VA_ARGS__)\
            : (typeof((OBJ->is_a->METHOD(OBJ, ##__VA_ARGS__))))0LU \
		)
#endif

#define VALIDATE_CLASS_STRUCT_(CLASS_PTR) {\
	void **p = (void**) CLASS_PTR; \
	size_t class_struct_size = sizeof(*CLASS_PTR); \
	int n = class_struct_size / sizeof(void*); \
	int totalNullMethodPointers = 0; \
	for (int i=4; i < n; i++) { \
		if (NULL == p[i]) { \
			totalNullMethodPointers++; \
			fprintf (stderr, "%s: Class struct has NULL method pointer at index %d.\n", __FUNCTION__, i-TOTAL_EMPTY_CLASS_ULONGS); \
		} \
	} \
	if (totalNullMethodPointers > 0) { \
		fprintf (stderr, "%s: A total of %d method pointer(s) are NULL\n", __FUNCTION__, totalNullMethodPointers); \
		exit (-1); \
	} \
}
#define VALIDATE_CLASS_STRUCT(CLASS_PTR) VALIDATE_CLASS_STRUCT_(CLASS_PTR)

// NOTE: Retain count is initially 0 i.e. not yet owned.
#ifdef GRATUITOUS_DEBUGGING
	#define allocate(CLASS) ((CLASS*)(printf("Alloc'd %s object in %s\n",STRINGIFY(CLASS), __FUNCTION__), g_totalObjectAllocations++, ooc_allocate (STRINGIFY_EVALUATING(CLASS),sizeof(CLASS)) ))
#else
	#define allocate(CLASS) ((CLASS*)(g_totalObjectAllocations++, ooc_allocate (STRINGIFY_EVALUATING(CLASS),sizeof(CLASS)) ))
#endif

#define new_(CLASS) ((CLASS*) CONCAT_SYMBOLS(CLASS,_init) (allocate(CLASS)))
#define new(CLASS) ((void*)new_(CLASS))

#define retain(OBJ) ooc_retain(OBJ, __FUNCTION__)

// Note, release() only zeroes out the pointer if the retainCount falls to zero, or was already zero.
#define release(OBJ) do { \
	if (!ooc_release((void*)OBJ, sizeof(__typeof(*OBJ)), __FUNCTION__)) \
		OBJ = (__typeof(OBJ)) NULL; \
} while(false)

#define isMemberOfClass(THIS,CLASS) ((THIS == NULL || \
				      THIS->is_a == NULL || \
				      THIS->magic != OBJECT_MAGIC_NUMBER || \
				      THIS->is_a != (void*) EVALUATING_CONCAT3(_,CLASS,Class)) ? false : true)

#define isMemberOfClassOrSubclass(THIS,CLASS) ooc_isMemberOfClassOrSubclass(THIS,CONCAT_SYMBOLS3(_,CLASS,Class),NULL)

#define verifyCorrectClassOrSubclass(THIS,CLASS) do { if (!ooc_isMemberOfClassOrSubclass(THIS,EVALUATING_CONCAT3(_,CLASS,Class),__FUNCTION__)) error_bad_class(__FUNCTION__); } while(false)

#define verifyCorrectClass(THIS,CLASS) \
	if (!THIS || !((Object*)THIS)->is_a || ((Object*)THIS)->magic != OBJECT_MAGIC_NUMBER || ((Object*)THIS)->is_a != (void*) EVALUATING_CONCAT3(_,CLASS,Class)) {\
		error_bad_class (__FUNCTION__);\
	}

#define describeObject(THIS) \
	do { \
		if (THIS) { \
			printf ("Object (%lx)'s class is %lx %s, ", (unsigned long)THIS,(unsigned long)THIS->is_a,THIS->is_a->_className); \
			printf ("superclass is %lx %s\n", (unsigned long)THIS->is_a->superClass, \
					THIS->is_a->superClass ?  ((EmptyClass*)THIS->is_a->superClass)->_className : 0); \
		} \
	while(false)

#define verifyCorrectClasses(THIS,CLASS,SUBCLASS) \
	if (!THIS->is_a || (THIS->is_a != (void*)CONCAT_SYMBOLS3(_,CLASS,Class) && THIS->is_a != (void*)CONCAT_SYMBOLS3(_,SUBCLASS,Class))) {\
		error_bad_class (__FUNCTION__);\
	}

#define SET_ABSTRACT_METHOD_POINTER(NAME) class->NAME = (void*)ooc_abstract_method

#define SET_METHOD_POINTER_(CLASS,NAME) class->NAME = CLASS##_##NAME
#define SET_METHOD_POINTER(CLASS,NAME) SET_METHOD_POINTER_(CLASS,NAME)

#define SET_OVERRIDDEN_METHOD_POINTER_(CLASS,NAME) class->NAME = CLASS##_##NAME
#define SET_OVERRIDDEN_METHOD_POINTER(CLASS,NAME) SET_OVERRIDDEN_METHOD_POINTER_(CLASS,NAME)

#define SET_INHERITED_METHOD_POINTER_(CLASS,PARENTCLASS,NAME) class->NAME = (void*)PARENTCLASS##_##NAME;
#define SET_INHERITED_METHOD_POINTER(CLASS,PARENTCLASS,NAME) SET_INHERITED_METHOD_POINTER_(CLASS,PARENTCLASS,NAME)

#ifdef GRATUITOUS_DEBUGGING
	#define DEBUG_DESTROY puts(__FUNCTION__);fflush(0)
#else
	#define DEBUG_DESTROY
#endif

#endif
