/*============================================================================
  FontFreetype, an object-oriented C class to provide TTF fonts.
  This code originates from my FrugalWidgets project.
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

#ifndef _OOC_FONTFREETYPE_H
#define _OOC_FONTFREETYPE_H

#include "Font.h"

#define DECLARE_FONTFREETYPE_INSTANCE_VARS(TYPE_POINTER) \
	
#define DECLARE_FONTFREETYPE_METHODS(TYPE_POINTER) \

struct fontfreetype;

typedef struct fontfreetypeclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct fontfreetype*)
        DECLARE_FONT_METHODS(struct fontfreetype*)
        DECLARE_FONTFREETYPE_METHODS(struct fontfreetype*)
} FontFreetypeClass;

extern FontFreetypeClass *_FontFreetypeClass;

typedef struct fontfreetype {
        FontFreetypeClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct fontfreetype*)
	DECLARE_FONT_INSTANCE_VARS(struct fontfreetype*)
	DECLARE_FONTFREETYPE_INSTANCE_VARS(struct fontfreetype*)
} FontFreetype;

extern FontFreetypeClass *FontFreetypeClass_init (FontFreetypeClass *class);

extern bool FontFreetype_isAvailable (void);
extern FontFreetype *FontFreetype_new (void);
extern FontFreetype *FontFreetype_init (FontFreetype *self);
extern FontFreetype *FontFreetype_fromFile (const char* path, int size);
// extern FontFreetype *FontFreetype_newWith (const char* name, int size, bool bold, bool italic);

#endif
