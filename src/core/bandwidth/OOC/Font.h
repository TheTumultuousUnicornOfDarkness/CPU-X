/*============================================================================
  Font, an object-oriented C font base class.
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

#ifndef _OOC_FONT_H
#define _OOC_FONT_H

#include <stdbool.h>
#include <math.h>
#include <wchar.h>

#include "Object.h"
#include "String.h"
#include "MutableString.h"

#define MAXFONTNAMELEN (64)
#define MAXCHARS 256
#define MAXCHARCODE (MAXCHARS-1)

#define kDefaultIntercharacterSpace (2) 

typedef enum {
        RowUnitByte = 0,
        RowUnitWord = 1,
        RowUnitDword = 2,
} RowUnit;

#define kMaxFontNameSize (32)

#define DECLARE_FONT_INSTANCE_VARS(FOO) \
	short dpi; 	\
	short ascent; 	\
	short descent; 	\
	short height; 	\
	float pointSize; 	\
	char name[kMaxFontNameSize]; 	\
	char family[kMaxFontNameSize]; \
	char fullName[kMaxFontNameSize]; \
	wchar_t firstCharacter; 	\
	wchar_t lastCharacter; 	\
	wchar_t totalCharacters;	\
	short spaceWidth; 	\
	char fixedWidth; 	\
	bool isMonochrome;	\
	bool isFixedWidth; 	\
	bool italic;	\
	bool bold;	\
	unsigned short weight;	\
	unsigned char bytesPerRow[MAXCHARS];	\
	unsigned short widths[MAXCHARS];	\
	unsigned short bitsHigh[MAXCHARS];	\
	unsigned short bitsWide[MAXCHARS]; /* 8, 16, 32, or 64 */	\
	short descents[MAXCHARS];	\
	short xoffsets[MAXCHARS];	\
	uint8_t *bitmaps[MAXCHARS];	\
	uint8_t *bitmapBuffer;	\
	short underlinePosition;	\
	short underlineThickness;	\
	char rowUnit;	

#define DECLARE_FONT_METHODS(TYPE_POINTER) \
	float (*pointSize) (TYPE_POINTER);	\
	short (*ascent) (TYPE_POINTER);	\
	short (*descent) (TYPE_POINTER);	\
	short (*spaceWidth) (TYPE_POINTER);	\
	short (*height) (TYPE_POINTER);	\
	wchar_t (*firstCharacter) (TYPE_POINTER);	\
	wchar_t (*lastCharacter) (TYPE_POINTER);	\
	long (*totalCharacters) (TYPE_POINTER);\
	void* (*bitmapForCharacter) (TYPE_POINTER, wchar_t characterCode, unsigned* width, unsigned* bytesPerRow, unsigned* bitsWide, unsigned* bitsHigh, int* xoffset, int* descent); 	\
	void (*sizeOfString) (TYPE_POINTER, String *, int* w, int* a, int* d); \
	void (*sizeOfWideString) (TYPE_POINTER, wchar_t *, unsigned length, int* w, int* a, int* d); \
	int (*stringWidth) (TYPE_POINTER, Any *string);	

struct font;

typedef struct fontclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct font*)
        DECLARE_FONT_METHODS(struct font*)
} FontClass;

extern FontClass *_FontClass;

typedef struct font {
        FontClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct font*)
	DECLARE_FONT_INSTANCE_VARS(struct font*)
} Font;

Font *Font_new (void);
Font *Font_init (Font *self);
void Font_destroy (Any *self);
Font* Font_with (const char* name, int size, bool bold, bool italic);

// Used by subclasses.
FontClass* FontClass_init (FontClass*);

#endif
