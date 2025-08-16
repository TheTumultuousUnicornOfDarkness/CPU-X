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

#ifndef _OOC_DOUBLE_H
#define _OOC_DOUBLE_H

#include <stdbool.h>
#include <math.h>

#include "Object.h"

#define DECLARE_DOUBLE_INSTANCE_VARS(FOO) \
	double floatingPoint;

#define DECLARE_DOUBLE_METHODS(TYPE_POINTER) \
	long long (*asLongLong) (TYPE_POINTER); \
	bool (*asBool) (TYPE_POINTER); \
	double (*asDouble) (TYPE_POINTER); 

struct floatingpoint;

typedef struct floatingpointclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct floatingpoint*)
        DECLARE_DOUBLE_METHODS(struct floatingpoint*)
} DoubleClass;

extern DoubleClass *_DoubleClass;
extern DoubleClass* DoubleClass_init (DoubleClass*);

typedef struct floatingpoint {
        DoubleClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct floatingpoint*)
	DECLARE_DOUBLE_INSTANCE_VARS(struct floatingpoint*)
} Double;

extern Double *Double_new ();
extern Double *Double_init (Double *self);
extern void Double_destroy (Any *);
extern Double *Double_withLongLong (long long value);
extern Double *Double_withDouble (double value);
extern Double *Double_withUTF8String (const char *string);

// Convenience instantiator macro
#define _Double(VALUE) Double_withDouble((double)VALUE)

#endif
