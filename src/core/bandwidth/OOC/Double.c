/*============================================================================
  Double, an object-oriented C floating point class.
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

#include "Double.h"

#include <math.h>

DoubleClass *_DoubleClass = NULL;

void Double_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,Double);
}

Double *Double_new () 
{
	return new(Double);
}

Double *Double_withLongLong (long long value) {
	Double *self = Double_new ();
	self->floatingPoint = value;
	return self;
}

Double *Double_withDouble (double value) {
	Double *self = Double_new ();
	self->floatingPoint = value;
	return self;
}

Double *Double_withUTF8String (const char *string) {
	Double *self = Double_new ();
	if (string != NULL) {
		self->floatingPoint = atof(string);
	}
	return self;
}

static void Double_print (Double* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Double);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%lf", self->floatingPoint);
}

static void Double_describe (Double* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}	
	verifyCorrectClass(self,Double);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%s", $(self, className));
}

static long long Double_asLongLong (Double* self)
{ 
	if (!self) {
		return 0;
	}
	verifyCorrectClass(self,Double);

	return (long long) floor (self->floatingPoint);
}

static double Double_asDouble (Double* self)
{ 
	if (!self) {
		return 0;
	}
	verifyCorrectClass(self,Double);

	return self->floatingPoint;
}

static bool Double_asBool (Double* self)
{ 
	if (!self) {
		return false;
	}
	verifyCorrectClass(self,Double);

	return 0. != self->floatingPoint;
}

Double* Double_init (Double *self)
{
	ENSURE_CLASS_READY(Double);

	Object_init ((Object*) self);
	self->is_a = _DoubleClass;

	self->floatingPoint = 0.;

	return self;
}

unsigned Double_hash (Double *self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Double);

	union {
		double d;
		uint64_t u;
	} u;
	u.d = self->floatingPoint;

	return u.u ^ 0x5a5a5a5a;
}

int Double_compare (Double *self, Any *other_)
{
	if (!self || !other_) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Double);
	verifyCorrectClassOrSubclass(other_,Double);

	Double *other = other_;

	if (self->floatingPoint == other->floatingPoint) return 0;
	if (self->floatingPoint < other->floatingPoint) return -1;
	return 1;
}

static bool Double_equals (Double *self, void *other_) 
{ 
	if (!self || !other_) {
		return false;
	}
	verifyCorrectClass(self,Double);
	Double *other = (Double*)other_;
	verifyCorrectClassOrSubclass(other,Double);
	if (self->is_a != other->is_a)
		return false;

	return self->floatingPoint == other->floatingPoint;
}

DoubleClass* DoubleClass_init (DoubleClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Double,describe);
	SET_OVERRIDDEN_METHOD_POINTER(Double,print);
	SET_OVERRIDDEN_METHOD_POINTER(Double,equals);
	SET_OVERRIDDEN_METHOD_POINTER(Double,compare);
	SET_OVERRIDDEN_METHOD_POINTER(Double,hash);

	SET_METHOD_POINTER(Double,asLongLong);
	SET_METHOD_POINTER(Double,asDouble);
	SET_METHOD_POINTER(Double,asBool);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

