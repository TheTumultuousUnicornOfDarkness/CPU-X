/*============================================================================
  MutableString, an object-oriented C string manipulation class.
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

#include "MutableString.h"
#include "Log.h"

#define _GNU_SOURCE
#include <stdio.h>

#include <stdarg.h>

MutableStringClass *_MutableStringClass = NULL;

#define kDefaultMutableStringSize (32)

void MutableString_destroy (Any* self)
{
        DEBUG_DESTROY;

	if (!self) {
		return;
	}
	verifyCorrectClass(self,MutableString);

	String_destroy (self);
}

MutableString* MutableString_init (MutableString* self)
{
	ENSURE_CLASS_READY(MutableString);

	if (!self) {
		return NULL;
	}
	
	String_init ((String*)self);
	self->is_a = _MutableStringClass;

	self->chars = ooc_alloc_memory(sizeof(wchar_t) * kDefaultMutableStringSize);
	self->_allocatedSize = kDefaultMutableStringSize;

	return self;
}

MutableString* MutableString_initWithCString (MutableString* self, const char *str)
{
	ENSURE_CLASS_READY(MutableString);

	if (self) {
		String_init ((String*)self);
		self->is_a = _MutableStringClass;

		int len = str ? ooc_strlen (str) : 0;
		self->_allocatedSize = len? len*2+1 : kDefaultMutableStringSize;
		self->chars = ooc_alloc_memory(sizeof(wchar_t) * self->_allocatedSize);
		if (str) {
			for (int i=0; i < len; i++)
				self->chars[i] = str[i];
		}
	}
	return self;
}

static void reallocIfNecessary (MutableString *self, int newLength)
{
	if (newLength >= self->_allocatedSize-1) {
		int newSize = self->_allocatedSize  * 2;
		if (newLength > newSize)
			newSize = newLength * 2;
		self->chars = realloc (self->chars, sizeof(wchar_t) * newSize);
		self->_allocatedSize = newSize;
	}
}

void MutableString_setCString (MutableString *self, const char* string)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!string || !*string) {
		if (self->chars)
			self->chars[0] = 0;
		return;
	}

	int len = ooc_strlen (string);
	reallocIfNecessary (self, len+1);
	int i;
	for (i=0; i < len; i++) {
		self->chars[i] = string[i];
	}
	self->chars[i] = 0;
}

void MutableString_setWide (MutableString *self, const wchar_t* string)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!string || !*string) {
		if (self->chars)
			self->chars[0] = 0;
		return;
	}
	int len = wcslen (string);
	reallocIfNecessary (self, len+1);
	wmemcpy (self->chars, string, len);
	self->chars[len] = 0;
}

void MutableString_setString (MutableString *self, String *other)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!other || !other->chars) {
		if (self->chars)
			self->chars[0] = 0;
		return;
	}

	int otherLength = wcslen(other->chars);
	reallocIfNecessary (self, otherLength+1);
	wmemcpy (self->chars, other->chars, otherLength);
	self->chars[otherLength] = 0;
}

void MutableString_appendCString (MutableString *self, const char *string)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!string || !*string)
		return;
	if (!self->chars) {
		MutableString_setCString (self, string);
		return;
	}

	int otherLength = ooc_strlen (string);
	int currentLength = wcslen(self->chars);
	int newLength = currentLength + otherLength;
	reallocIfNecessary (self, newLength+1);

	int i = 0;
	int j = currentLength;
	while (i < otherLength) {
		// XX Need to convert UTF8 to wchar_t.
		self->chars[j++] = (uint8_t) string[i++];
	}
	self->chars[j] = 0;
}

void MutableString_appendFormat (MutableString *self, const char* format, ...)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!format || !*format)
		return;

        va_list args;

        va_start (args, format);
        long spaceNeeded = vsnprintf (NULL, 0, format, args);
        va_end (args);

	if (spaceNeeded <= 0) {
		return;
	}

        va_start (args, format);
        char *buffer = ooc_alloc_memory(spaceNeeded + 1);
        vsnprintf (buffer, spaceNeeded, format, args);
        va_end (args);

	MutableString_appendCString (self, buffer);
	ooc_bzero (buffer, spaceNeeded);
	ooc_free (buffer);
}

void MutableString_appendWide (MutableString *self, const wchar_t *string)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!string || !*string)
		return;
	if (!self->chars) {
		MutableString_setWide (self, string);
		return;
	}

	int currentLength = wcslen(self->chars);
	int otherLength = wcslen (string);
	int newLength = currentLength + otherLength;
	reallocIfNecessary (self, newLength+1);
	wmemcpy (self->chars + currentLength, string, otherLength);
	self->chars [newLength] = 0;
}

void MutableString_appendString (MutableString *self, String *other)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!other)
		return;
	verifyCorrectClasses(other,String,MutableString);

	if (!self->chars) {
		MutableString_setString (self, other);
		return;
	}

	int currentLength = wcslen(self->chars);
	int otherLength = $(other, length);
	int newLength = currentLength + otherLength;
	reallocIfNecessary (self, newLength+1);
	wmemcpy (self->chars + currentLength, other->chars, otherLength);
	self->chars [newLength] = 0;
}

void MutableString_insertCharacterAt (MutableString *self, wchar_t ch, unsigned index)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,MutableString);

	if (!ch)
		return;

	int length = wcslen(self->chars);
	reallocIfNecessary (self, 2+length);

	if (index >= length) {
		self->chars [length++] = ch;
		self->chars [length] = 0;
	}
	else {
		for (int i = length; i > index; i--) {
			self->chars[i] = self->chars[i-1];
		}
		length++;
		self->chars[index] = ch;
		self->chars[length] = 0;
	}
}

void MutableString_appendCharacter (MutableString *self, wchar_t ch)
{
	return MutableString_insertCharacterAt (self, ch, MAXUNSIGNED);
}

void MutableString_truncateAt (MutableString *self, int index)
{
	if (!self) {
		return;
	}

	verifyCorrectClass(self,MutableString);

	if (!self->chars)
		return;

	int currentLength = wcslen (self->chars);
	if (index < currentLength) 
		self->chars[index] = 0;
}

MutableString* MutableString_withCString (const char* str)
{
	MutableString* obj = allocate(MutableString);
	return MutableString_initWithCString (obj, str);
}

MutableStringClass* MutableStringClass_init (MutableStringClass *class)
{
	SET_SUPERCLASS(String);

	SET_OVERRIDDEN_METHOD_POINTER(MutableString,destroy);

	SET_METHOD_POINTER(MutableString,setCString);
	SET_METHOD_POINTER(MutableString,setWide);
	SET_METHOD_POINTER(MutableString,setString);
	SET_METHOD_POINTER(MutableString,appendCString);
	SET_METHOD_POINTER(MutableString,appendWide);
	SET_METHOD_POINTER(MutableString,appendCharacter);
	SET_METHOD_POINTER(MutableString,appendString);
	SET_METHOD_POINTER(MutableString,insertCharacterAt);
	SET_METHOD_POINTER(MutableString,truncateAt);
	SET_METHOD_POINTER(MutableString,appendFormat);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

