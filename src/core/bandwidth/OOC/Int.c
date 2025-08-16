/*============================================================================
  Int, an object-oriented C integer class.
  Copyright (C) 2019, 2023 by Zack T Smith.

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

#include "Int.h"

#include <stdlib.h>

IntClass *_IntClass = NULL;

void Int_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,Int);
}

static void Int_print (Int* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Int);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%lld", self->integer);
}

static void Int_describe (Int* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Int);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%s", $(self, className));
}

static bool Int_equals (Int *self, void *other_) 
{ 
	if (!self || !other_) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,Int);

	Int *other = (Int*)other_;
	verifyCorrectClassOrSubclass(other,Int);

	return self->integer == other->integer;
}

unsigned Int_hash (Int *self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Int);

	return self->integer ^ 0x5a5a5a5a;
}

int Int_compare (Int *self, Any *other_)
{
	if (!self || !other_) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Int);
	verifyCorrectClassOrSubclass(other_,Int);

	Int *other = other_;

	if (self->integer == other->integer) return 0;
	if (self->integer < other->integer) return -1;
	return 1;
}

static const char* Int_asCString (Int *self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Int);

	static char buffer [64];
	snprintf (buffer, sizeof(buffer), "%lld", self->integer);
	return buffer;
}

static int Int_asInt (Int* self)
{ 
	if (!self) {
		return 0;
	}
	verifyCorrectClass(self,Int);
	return (int) self->integer;
}

static long long Int_asLongLong (Int* self)
{ 
	if (!self) {
		return 0;
	}
	verifyCorrectClass(self,Int);
	return self->integer;
}

static double Int_asDouble (Int* self)
{ 
	if (!self) {
		return 0;
	}
	verifyCorrectClass(self,Int);
	return (double) self->integer;
}

static bool Int_asBool (Int* self)
{ 
	if (!self) {
		return 0;
	}
	verifyCorrectClass(self,Int);
	return 0 != self->integer;
}

IntClass* IntClass_init (IntClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Int,describe);
	SET_OVERRIDDEN_METHOD_POINTER(Int,print);
	SET_OVERRIDDEN_METHOD_POINTER(Int,equals);
	SET_OVERRIDDEN_METHOD_POINTER(Int,compare);
	SET_OVERRIDDEN_METHOD_POINTER(Int,hash);

	SET_METHOD_POINTER(Int,asInt);
	SET_METHOD_POINTER(Int,asLongLong);
	SET_METHOD_POINTER(Int,asDouble);
	SET_METHOD_POINTER(Int,asBool);
	SET_METHOD_POINTER(Int,asCString);
	
        VALIDATE_CLASS_STRUCT(class);
	return class;
}

Int* Int_init (Int *self)
{
	ENSURE_CLASS_READY(Int);

	if (self) {
		Object_init ((Object*) self);
		self->is_a = _IntClass;

		self->integer = 0;
	}

	return self;
}

Int *Int_new () 
{
	return new(Int);
}

Int* Int_withUnsignedLong (unsigned long value) {
	Int *self = Int_new ();
	self->integer = (long long) value;
	return self;
}

Int *Int_withLongLong (long long value) {
	Int *self = Int_new ();
	self->integer = value;
	return self;
}

Int *Int_withDouble (double value) {
	Int *self = Int_new ();
	self->integer = (long long) value;
	return self;
}

Int* Int_fromCString (const char* str)
{
	if (!str)
		return NULL;
	Int *self = Int_new ();
	self->integer = atoi (str);
	return self;
}

Int* Int_fromFile (const char* path)
{
	if (!path)
		return NULL;

	FILE *f = fopen (path, "r");
	if (!f)
		return NULL;

	char firstChar = fgetc (f);
	bool negative = firstChar == '-';
	bool success = true;
	long long signed_value;
	unsigned long long unsigned_value;
	ungetc (firstChar, f);
	if (negative) {
		if (1 != fscanf (f, "%lld", &signed_value))
			success = false;
	}
	else {
		if (1 != fscanf (f, "%llu", &unsigned_value))
			success = false;
	}

	if (!success) {
		fclose (f);
		return NULL;
	}

	Int *integer = new(Int);
	if (negative) { // XX
		integer->integer = signed_value;
	} 
	else {
		integer->integer = unsigned_value;
	}
	return integer;
}
