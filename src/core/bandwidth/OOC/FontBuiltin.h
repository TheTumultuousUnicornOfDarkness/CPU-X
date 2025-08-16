/*============================================================================
  FontBuiltin, an object-oriented C font class for the built-in font.
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

#ifndef _OOC_FONTBUILTIN_H
#define _OOC_FONTBUILTIN_H

#include "Font.h"

#define DECLARE_FONTBUILTIN_INSTANCE_VARS(TYPE_POINTER) 

#define DECLARE_FONTBUILTIN_METHODS(TYPE_POINTER) 

struct fontbuiltin;

typedef struct fontbuiltinclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct fontbuiltin*)
        DECLARE_FONT_METHODS(struct fontbuiltin*)
        DECLARE_FONTBUILTIN_METHODS(struct fontbuiltin*)
} FontBuiltinClass;

extern FontBuiltinClass *_FontBuiltinClass;
extern FontBuiltinClass* FontBuiltinClass_init (FontBuiltinClass*);

typedef struct fontbuiltin {
        FontBuiltinClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct fontbuiltin*)
	DECLARE_FONT_INSTANCE_VARS(struct fontbuiltin*)
	DECLARE_FONTBUILTIN_INSTANCE_VARS(struct fontbuiltin*)
} FontBuiltin;

extern FontBuiltin *FontBuiltin_new ();
extern void FontBuiltin_destroy (Any *);
extern FontBuiltin *FontBuiltin_init (FontBuiltin *self);
extern FontBuiltin *FontBuiltin_with (const char* name, int size, bool bold, bool italic);

#define _FontBuiltin() FontBuiltin_with(NULL,14,false,false)

#endif
