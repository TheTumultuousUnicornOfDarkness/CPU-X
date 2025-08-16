/*============================================================================
  ObjectOriented, a facility for object-oriented programming on top of C.
  Copyright (C) 2019, 2022 by Zack T Smith.

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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> // getpagesize

#if defined(__linux__) 
  #include <sys/mman.h> // mprotect, mlock
  #include <malloc.h> // memalign
#endif

#include "ObjectOriented.h"
#include "Object.h"
#include "Log.h"

uint64_t g_totalObjectAllocations = 0;
uint64_t g_totalObjectDeallocations = 0;
uint64_t g_totalObjectDeallocationsNeverRetained = 0;

// Array of all registered classes.
#define MAX_CLASSES (128)
uint64_t g_totalClassAllocations = 0;
uint64_t g_totalRegisteredClasses = 0;
EmptyClass *g_registeredClasses [MAX_CLASSES];

/*---------------------------------------------------------------------------
 * Name:	ooc_allocate_class
 * Purpose:	Allocate a class struct.
 *-------------------------------------------------------------------------*/
void *ooc_allocate_class (size_t size)
{
	void *class = NULL;
#if defined(__linux__) 
	size_t pagesize = getpagesize();
	if (size > pagesize) {
		Log_warning(__FUNCTION__, "Class struct size is larger than a memory page.");
		void* class = ooc_allocate (NULL, size);
		ooc_bzero (class, size);
		return class;
	}

	class = memalign (pagesize, pagesize);
	if (!class) {
		Log_perror (__FUNCTION__, "alloc_aligned");
		return NULL;
	}

	ooc_bzero (class, pagesize);
	return class;
#else
	class = malloc (size);
	if (class != NULL) {
		ooc_bzero (class, size);
	}
	return class;
#endif
}

/*---------------------------------------------------------------------------
 * Name:	ooc_lock_class
 * Purpose:	Lock the memory page of a class to prevent future corruption
 *		of method pointers.
 *-------------------------------------------------------------------------*/
static bool ooc_lock_class (const void* class, size_t size)
{
#if defined(__linux__) 
	if (class && size > 0 && size < getpagesize()) {
		// Make the class struct read-only.
		if (mprotect ((void*)class, size, PROT_READ)) {
			Log_perror(__FUNCTION__, "mprotect");
			return false;
		}

		// Make sure it's kept in memory.
		if (mlock ((void*)class, size)) {
			Log_perror(__FUNCTION__, "mlock");
			return false;
		}
	}
#endif
	return true;
}

/*---------------------------------------------------------------------------
 * Name:	ooc_unlock_class
 * Purpose:	Lock the memory page of a class to allow deallocation.
 *-------------------------------------------------------------------------*/
static bool ooc_unlock_class (const void* class, size_t size)
{
#if defined(__linux__) 
	if (class && size > 0 && size < getpagesize()) {
		if (mprotect ((void*)class, size, PROT_WRITE | PROT_READ)) {
			Log_perror(__FUNCTION__, "mprotect");
			return false;
		}

		if (munlock ((void*)class, size)) {
			Log_perror(__FUNCTION__, "munlock");
			return false;
		}
	}
#endif
	return true;
}

/*---------------------------------------------------------------------------
 * Name:	registerClass
 * Purpose:	Register and lock a class struct.
 *-------------------------------------------------------------------------*/
void registerClass (const void *class)
{
	if (!class) {
		return;
	}

	if (g_totalRegisteredClasses >= MAX_CLASSES) {
		Log_error (__FUNCTION__, "Maximum classes allowed were registered.");
		exit(-1);
	}

	g_registeredClasses [g_totalRegisteredClasses++] = (EmptyClass*)class;

	size_t class_struct_size = ((EmptyClass*)class)->classSize;
	if (class_struct_size) {
		ooc_lock_class (class, class_struct_size); 
	}

#ifdef GRATUITOUS_DEBUGGING
	printf ("Registered class %s\n", (((EmptyClass*) class)->_className));
#endif
}

bool isObject(void* object_)
{
	Object *object = (Object*)object_;
	return object != NULL && object->magic == OBJECT_MAGIC_NUMBER && object->is_a != NULL;
}

bool areSameClass(void* a, void* b)
{
	if (!a || !b) {
		return false;
	}
	Object *obj1 = (Object*)a;
	Object *obj2 = (Object*)b;
	if (obj1->magic != OBJECT_MAGIC_NUMBER || obj2->magic != OBJECT_MAGIC_NUMBER) {
		return false;
	}
	ObjectClass *class1 = obj1->is_a;
	ObjectClass *class2 = obj2->is_a;
	ObjectClass *super1 = class1->superClass;
	ObjectClass *super2 = class2->superClass;
	if (class1 == class2 // e.g. MutableString vs MutableString
	 || super1 == class2 // e.g. String vs MutableString
	 || class1 == super2 // e.g. MutableString vs String
	) {
		return true;
	}

	return false;
}

void ooc_free (void* pointer)
{
	Object *object = (Object*)pointer;
	if (object->is_a && object->magic == OBJECT_MAGIC_NUMBER) {
		const char *name = object->is_a->_className;
		printf ("^- Class name is %s\n", name);
	}
	fflush(NULL);
	free (pointer);
}

void deallocateClasses ()
{
	printf ("Deallocated classes ");
	for (size_t i=0; i < g_totalRegisteredClasses; i++) {
		EmptyClass *class = g_registeredClasses [i];

		if (i != g_totalRegisteredClasses-1)
			printf ("%s, ", class->_className);
		else
			printf ("%s\n", class->_className);

		size_t class_struct_size = class->classSize;
		if (class_struct_size < getpagesize()) {
			if (ooc_unlock_class (class, class_struct_size)) {
				ooc_bzero (class, class_struct_size);
				ooc_free (class);
			}
		}
		else {
			ooc_bzero (class, class_struct_size);
			ooc_free (class);
		}
	}
}

size_t ooc_strlen(const char *start)
{
	if (start == NULL) {
		return 0;
	}
	
	char *ptr = (char*) start;
	size_t len = 0;
	while (*ptr) {
		len++;
		ptr++;
	}
	return len;
}

void ooc_strncpy (const char *destination, const char *source, size_t length)
{
	if (!destination || !source || !length) {
		return;
	}
	char *dst = (char*) destination;
	char *src = (char*) source;
	for (size_t i = 0; *src && i < length; i++) {
		*dst++ = *src++;
	}
	*dst = 0;
}

void* ooc_bzero (const void *start, size_t length)
{
	if (!start || !length) {
		return NULL;
	}

	uint8_t *ptr = (uint8_t *) start;
	size_t bytesRemaining = length;

	while (bytesRemaining > 0) {
#ifdef IS_64BIT
		if (IS_EIGHT_BYTE_ALIGNED(ptr) && bytesRemaining >= 8) {
			uint64_t *ptr64 = (uint64_t*) ptr;
			while (bytesRemaining >= 8) {
				*ptr64++ = 0;
				ptr += 8;
				bytesRemaining -= 8;
			}
			continue;
		}
#else
		if (IS_FOUR_BYTE_ALIGNED(ptr) && bytesRemaining >= 4) {
			uint32_t *ptr32 = (uint32_t*) ptr;
			while (bytesRemaining >= 4) {
				*ptr32++ = 0;
				ptr += 4;
				bytesRemaining -= 4;
			}
			continue;
		}
#endif
		*ptr++ = 0;
		bytesRemaining--;
	}
	return (void*) start;
}

void ooc_abstract_method ()
{
}

void *ooc_retain (void* object_, const char *funcName)
{
	if (object_) {
		typedef struct {
			void *is_a;
			int32_t magic;
			int32_t retainCount;
		} LiteObj;
		LiteObj *object = (LiteObj*) object_;
		if (object->magic != OBJECT_MAGIC_NUMBER) {
			warning(funcName, "Attempt to retain a non-object.");
			return object_;
		}
		object->retainCount++;
		return object_;
	}
	else {
		return NULL;
	}
}

bool ooc_release (void *object_, size_t size, const char *funcName)
{
	Object *object = (Object*)object_;
	if (object) { 
		if (object->magic != OBJECT_MAGIC_NUMBER) {
			warning(funcName, "Attempt to release a non-object.");
			return false;
		}
		else if (object->retainCount < 0) {
			error_double_release(__FUNCTION__); 
			return false;
		}
		else if (object->is_a == NULL) {
			warning(funcName, "Object already release and destroyed.");
			return false;
		}

		if (object->retainCount == 0) { 
			++g_totalObjectDeallocationsNeverRetained;
#ifdef GRATUITOUS_DEBUGGING
			printf ("Object of type %s was never retained.\n",
				object->is_a->_className);
#endif
		}
		else {
			--object->retainCount;
		}

		if (object->retainCount < 0) { 
			warning (funcName, "Excessive releases of an object detected.");
			return false;
		}
		else if (object->retainCount == 0) { 
			ObjectClass *class = object->is_a;
			class->destroy((void*) object); 

			if (size > 0) {
				ooc_bzero (object_, size);
			} 
			else {
				object->magic = 0;
				object->is_a = NULL;
			}

			ooc_free (object); 
			g_totalObjectDeallocations++; 
			return false;
		}
	}

	// Tell the macro to not set the pointer to NULL.
	return true;
}

void *ooc_alloc_memory (size_t size)
{
	return ooc_allocate (NULL, size);
}

void *ooc_allocate (const char *objectType, size_t size)
{
	if (!size) {
		return NULL;
	}

//#define VALGRIND
#ifdef VALGRIND
	if (size < 10) {
		size = 10;
	}
#endif

	void *object = calloc (size, 1);
	if (!object) {
		Log_perror(__FUNCTION__, "calloc");
	}
	return object;
}

bool ooc_isMemberOfClassOrSubclass(void *self_, void *soughtClass, const char* funcName)
{
	Object *self = (Object*)self_;

	if (!self) { 
		if (funcName)
			error_null_parameter(funcName);
	}
	else if (self->magic != OBJECT_MAGIC_NUMBER) {
		if (funcName)
			warning (funcName, "Attempt to verify the class of a non-object."); 
	}
	else if (self->is_a == NULL) {
		if (funcName)
			warning (funcName, "NULL is_a pointer."); 
	}
	else {
		ObjectClass *class = (ObjectClass*) self->is_a;
		while (class) {
			if (class == (ObjectClass*) soughtClass) {
				return true;
			}

			class = class->superClass;
		}
	}
	return false;
}

