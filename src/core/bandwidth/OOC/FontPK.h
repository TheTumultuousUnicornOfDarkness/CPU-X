/*============================================================================
  FontPK, an object-oriented C font class for PK (TeX) fonts.
  Copyright (C) 2018, 2021 by Zack T Smith.

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

#ifndef _OOC_FONTPK_H
#define _OOC_FONTPK_H

#include "Font.h"
#include "PKChar.h"

#define DECLARE_FONTPK_INSTANCE_VARS(TYPE_POINTER) 

#define DECLARE_FONTPK_METHODS(TYPE_POINTER) 

struct fontpk;

typedef struct fontpkclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct fontpk*)
        DECLARE_FONT_METHODS(struct fontpk*)
        DECLARE_FONTPK_METHODS(struct fontpk*)
} FontPKClass;

extern FontPKClass *_FontPKClass;
extern FontPKClass* FontPKClass_init (FontPKClass*);

typedef struct fontpk {
        FontPKClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct fontpk*)
	DECLARE_FONT_INSTANCE_VARS(struct fontpk*)
	DECLARE_FONTPK_INSTANCE_VARS(struct fontpk*)
} FontPK;

extern FontPK *FontPK_new ();
extern void FontPK_destroy (Any *);
extern FontPK *FontPK_init (FontPK *self);
extern FontPK *FontPK_with (const char *, int size);

#endif
