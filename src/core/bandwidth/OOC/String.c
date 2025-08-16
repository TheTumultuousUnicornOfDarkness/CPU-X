/*============================================================================
  String, an object-oriented C string class.
  Copyright (C) 2019, 2023-2024 by Zack T Smith.

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

#include "String.h"
#include "MutableString.h"
#include "MutableArray.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h> 
#include <ctype.h> // isalnum

StringClass *_StringClass = NULL;

#define kDefaultStringSize (32)

int String_parseInt (String* self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);

	wchar_t *string = self->chars;
	return string? wcstol (string, NULL, 10) : 0;
}

double String_parseDouble (String* self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);

	wchar_t *string = self->chars;
	return string? wcstod (string, NULL) : 0.0;
}

unsigned long String_parseHex (String* self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);

	wchar_t *string = self->chars;
	if (!string) {
		return 0;
	}
	if (*string == '0' && string[1] == 'x') {
		string += 2;
	}
	return wcstod (string, NULL);
}

unsigned long String_length (String* self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);

	if (!self->chars)
		return 0;
	return wcslen(self->chars);
}

void String_describe (String* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,String);
	
	fprintf (file ?: stdout, "%s(%S)\n", $(self, className), self->chars != NULL ? self->chars : L"");
}

void String_print (String *self, FILE* file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,String);

	if (!file) {
		file = stdout;
	}

	if (self->chars != NULL) {
		fprintf (file, "%ls", self->chars);
	}
}

bool String_equals (String *self, void *other_) 
{ 
	if (!self || !other_) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,String);
	String *other = (String*) other_;
	verifyCorrectClassOrSubclass(other,String);

	if (self->chars == NULL && other->chars == NULL) {
		return true;
	}
	if (self->chars == NULL || other->chars == NULL) {
		return false;
	}
	size_t lenSelf = wcslen(self->chars);
	size_t lenOther = wcslen(other->chars);
	if (!lenSelf && !lenOther)
		return true;
	if (lenSelf != lenOther)
		return false;
	if (wcscmp (self->chars, other->chars))
		return false;

	return true;
}

static unsigned String_totalLines (String *self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);

	unsigned nLines = 1;
	wchar_t *ptr = self->chars;
	while (*ptr) {
		if (*ptr == '\n')
			nLines++;
		ptr++;
	}

	return nLines;
}

void String_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,String);

	String *self = self_;

	if (self->chars) {
		ooc_free (self->chars);
		self->chars = NULL;
	}

	Object_destroy (self);
}

String* String_init (String* self)
{
	ENSURE_CLASS_READY(String);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _StringClass;

		self->chars = NULL;
	}
	return self;
}

String* String_initWithCString (String* self, const char *cstring)
{
	ENSURE_CLASS_READY(String);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _StringClass;

		if (cstring) {
			unsigned length = ooc_strlen (cstring);
			if (length > 0) {
				self->chars = ooc_alloc_memory(sizeof(wchar_t) * (length+1));
				if (self->chars != NULL) {
					uint8_t *bytes = (uint8_t*)cstring;

					for (int i = 0; i < length; i++)
						self->chars[i] = bytes[i];

					self->chars[length] = 0;
				}
			}
		}
	}
	return self;
}

String* String_initWithWideStringAndLength (String* self, const wchar_t *string, size_t length)
{
	ENSURE_CLASS_READY(String);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _StringClass;

		if (string && length) {
			void *newChars = ooc_alloc_memory (sizeof(wchar_t) * (length+1));
			if (newChars != NULL) {
				self->chars = newChars;
				memcpy (newChars, string, length * sizeof(wchar_t));
				self->chars[length] = 0;
			}
		}
	}
	return self;
}

String* String_initWithWide (String* self, const wchar_t *string)
{
	if (string)
		return String_initWithWideStringAndLength (self, string, wcslen(string));
	else
		return String_init(self);
}

String* String_withWide (const wchar_t *wide)
{
	String *string = allocate(String);
	return String_initWithWide (string, wide);
}

static String* String_withWideStringAndLength (const wchar_t *wide, size_t length)
{
	String *string = allocate(String);
	return String_initWithWideStringAndLength (string, wide, length);
}

static bool String_hasSuffix (String *self, void *other_)
{
	if (!self || !other_)
		return false;

	verifyCorrectClassOrSubclass(self,String);

	wchar_t *selfPtr = self->chars;
	size_t selfLength = selfPtr ? wcslen(selfPtr) : 0;

	if (isObject(other_)) {
		String *other = (String*)other_;
		verifyCorrectClassOrSubclass(other,String);

		size_t otherLength = $(other, length);
		if (otherLength > selfLength)
			return false;

		size_t selfIndex = selfLength - otherLength;
		size_t otherIndex = 0;
		wchar_t *otherPtr = other->chars;
		while (selfIndex < selfLength) {
			wchar_t selfChar = selfPtr[selfIndex];
			if (selfChar != otherPtr[otherIndex])
				return false;
			selfIndex++;
			otherIndex++;
		}
	}
	else {
		uint8_t *otherPtr = (uint8_t*)other_;
		size_t otherLength = strlen ((char*)other_);
		if (otherLength > selfLength)
			return false;

		size_t selfIndex = selfLength - otherLength;
		size_t otherIndex = 0;
		
		while (selfIndex < selfLength) {
			wchar_t selfChar = selfPtr[selfIndex];
			if (selfChar != (wchar_t) otherPtr[otherIndex])
				return false;
			selfIndex++;
			otherIndex++;
		}
	}

	return true;
}

static bool String_hasPrefix (String *self, Any *other_)
{
	if (!self || !other_) {
		return false;
	}

	verifyCorrectClassOrSubclass(self,String);
	verifyCorrectClassOrSubclass(other_,String);
	String *other = other_;

	wchar_t *selfPtr = self->chars;
	size_t selfLength = selfPtr ? wcslen(selfPtr) : 0;

	size_t otherLength = $(other, length);
	if (otherLength > selfLength) {
		return false;
	}

	size_t index = 0;
	wchar_t *otherPtr = other->chars;
	while (index < otherLength) {
		wchar_t selfChar = selfPtr[index];
		if (selfChar != otherPtr[index]) {
			return false;
		}
		index++;
	}
	return true;
}

wchar_t* String_characters (String *self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,String);

	return self->chars;
}

wchar_t String_at (String *self, int index)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);

	if (self->chars == NULL)
		return 0;

	int length = wcslen(self->chars);
	if (index < length)
		return self->chars[index];

	return 0;
}

Array *String_explode (String *self, Any *needle_)
{
	if (!self || !needle_)
		return NULL;
	verifyCorrectClassOrSubclass(self,String);
	wchar_t *haystackChars = self->chars;
	if (!haystackChars || !*haystackChars)
		return NULL;
	verifyCorrectClassOrSubclass(needle_,String);

	String *needle = needle_;
	if (!needle->chars)
		return NULL;
	wchar_t *needleChars = needle->chars;
	size_t needleLength = wcslen(needleChars);
	if (!needleLength)
		return NULL;
	size_t haystackLength = wcslen(haystackChars);
	if (needleLength > haystackLength)
		return NULL;

	MutableArray *mut = new(MutableArray);

	size_t index = 0;
	wchar_t *ptr = haystackChars;
	while (index < haystackLength) {
		// Find next needle.
		wchar_t *foundString = wcsstr (ptr, needleChars);
		if (!foundString)
			break;

		size_t foundLength = foundString - ptr;
		String *substring = NULL;
		if (foundLength > 0) {
			// String before need was != ""
			substring = String_withWideStringAndLength (ptr, foundLength);
		} else {
			// String before need was == ""
			substring = new(String);
		}
		$(mut, append, substring);

		size_t nCharsToSkip = foundLength + needleLength;
		ptr += nCharsToSkip;
		index += nCharsToSkip;
	}
	// Trailing substring
	if (index < haystackLength) {
		String *substring = String_withWideStringAndLength (ptr, haystackLength - index);
		$(mut, append, substring);
	}
	// String ended with needle.
	else if (index == haystackLength) {
		$(mut, append, new(String));
	}

	Array *result = Array_withArray (mut);
	release(mut);

	return result;
}

unsigned String_hash (String *self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);

	unsigned long sum = 0;
	wchar_t *ptr = self->chars;

	while (*ptr) {
		sum <<= 1;
		sum += *ptr++;
	}

	return sum;
}

int String_compare (String *self, Any *other_)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,String);
	verifyCorrectClassOrSubclass(other_,String);

	String *other = other_;

	wchar_t *a = self->chars;
	wchar_t *b = other->chars;
	if (!a && b) {
		return -1;
	}
	if (a && !b) {
		return 1;
	}
	if (!a && !b) {
		return 0;
	}
	return wcscmp (a, b);
}

const char *String_asUTF8 (String *self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,String);

	wchar_t *source = self->chars;
	if (!source || !*source) {
		return NULL;
	}
	size_t bytesNeeded = wcstombs (NULL, source, 0) + 1;
	char *destination = ooc_alloc_memory (bytesNeeded);
	size_t result = wcstombs (destination, source, bytesNeeded);
	if (result == (size_t)-1) {
		ooc_free (destination);
		return NULL;
	}
	return destination;
}

bool String_containsCString (String* self, const char *cstring)
{
	if (!self || !cstring || !*cstring) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,String);

	wchar_t *source = self->chars;
	if (!source || !*source) {
		return false;
	}

	size_t len = wcslen (source);
	size_t cslen = strlen (cstring);

	for (int i=0; i <= len-cslen; i++) {
		if (source[i] == (wchar_t) cstring[0]) {
			bool match = true;
			for (int j=1; j < cslen; j++) {
				char ch = cstring[j];
				if (!ch) {
					return true;
				}
				if (ch != source[i+j]) {
					match = false;
					break;
				}
				if (match) {
					return true;
				}
			}
		}
	}
	return false;
}

String* String_withCString (const char* str)
{
	return String_initWithCString (allocate(String), str);
}

StringClass* StringClass_init (StringClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(String,describe);
        SET_OVERRIDDEN_METHOD_POINTER(String,destroy);
        SET_OVERRIDDEN_METHOD_POINTER(String,equals);
        SET_OVERRIDDEN_METHOD_POINTER(String,compare);
        SET_OVERRIDDEN_METHOD_POINTER(String,hash);

        SET_METHOD_POINTER(String,length);
        SET_METHOD_POINTER(String,print);
	SET_METHOD_POINTER(String,at);
	SET_METHOD_POINTER(String,hasSuffix);
	SET_METHOD_POINTER(String,hasPrefix);
        SET_METHOD_POINTER(String,characters);
        SET_METHOD_POINTER(String,totalLines);
        SET_METHOD_POINTER(String,explode);
        SET_METHOD_POINTER(String,asUTF8);
	SET_METHOD_POINTER(String,parseInt);
	SET_METHOD_POINTER(String,parseDouble);
	SET_METHOD_POINTER(String,parseHex);
	SET_METHOD_POINTER(String,containsCString);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

