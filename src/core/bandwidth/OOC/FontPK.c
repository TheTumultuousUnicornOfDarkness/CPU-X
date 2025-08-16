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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>

#include "Font.h"
#include "FontPK.h"
#include "Log.h"

FontPKClass *_FontPKClass = NULL;

extern FontClass *_FontClass;

extern const PKChar *get_cmr5_char (int);
extern const PKChar *get_cmr6_char (int);
extern const PKChar *get_cmr9_char (int);
extern const PKChar *get_cmr12_char (int);

void FontPK_destroy (Any *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self, FontPK);

	Font_destroy (self);
}

FontPK *FontPK_init (FontPK *self)
{
	ENSURE_CLASS_READY(FontPK);
	if (!self) { 
		return NULL;
	}

	Font_init ((Font*) self);
	self->is_a = _FontPKClass;

	self->firstCharacter = 0;
	self->lastCharacter = 127;

	return self;
}

FontPK* FontPK_with (const char *name, int size)
{
	FontPK *self = new(FontPK);

	if (name == NULL)
		name = "cmr";
	strncpy (self->name, name, kMaxFontNameSize);

	if (!strcmp (name, "cmr")) {
		strncpy (self->family, "ComputerModernRoman", kMaxFontNameSize);

		switch (size) {
			default:
			case 14:
				self->ascent = 11;
				self->descent = 4;
				self->pointSize = 14;
				break;
			case 18:
				self->ascent = 14;
				self->descent = 4;
				self->pointSize = 18;
				break;
			case 26:
				self->ascent = 20;
				self->descent = 6;
				self->pointSize = 26;
				break;
			case 35:
				self->ascent = 27;
				self->descent = 8;
				self->pointSize = 35;
				break;
		}

		self->height = 1 + self->ascent + self->descent;
	}

	if (!self->ascent) {
		Log_warning_printf (__FUNCTION__, "Cannot find font of requested size %d", size);
		return NULL;
	}

	int index=0;
	while (true) {
		const PKChar *glyph = NULL;
		if (!strcmp (name, "cmr")) {
			switch (size) {
			default:
			case 14: 
				// 240px cmr5 = height 15, 11 ascent, 4 descender
				glyph = get_cmr5_char (index); 
				break;
			case 18: 
				// 240px cmr6 = height 18, 14 ascent, 4 descender
				glyph = get_cmr6_char (index); 
				break;
			case 26: 
				// 240px cmr9 = height 26, 20 ascent, 6 descender
				glyph = get_cmr9_char (index); 
				break;
			case 35: 
				// 240px cmr12 = height 35, 27 ascent, 8 descender
				glyph = get_cmr12_char (index); 
				break;
			}
		} 

		index++;

		if (glyph == NULL) {
			break;
		}
		if (glyph->height == 0 || glyph->width == 0) {
			continue;
		}

		int encoding = glyph->encoding;
		if (encoding < 0)
			break;

		if (encoding > self->lastCharacter)
			self->lastCharacter = encoding;

		encoding -= self->firstCharacter;
		if (encoding < 0)
			break;

		if (self->bitmaps [encoding]){
			printf ("Duplicate PK encoding %i\n",encoding);
			continue;
		}

		int bytesPerRow = (glyph->width + 7) >> 3;
		if (bytesPerRow == 3)
			bytesPerRow = 4;
		else if (bytesPerRow > 4)
			bytesPerRow = 8;

		self->bytesPerRow[encoding] = bytesPerRow;

		self->widths[encoding] = glyph->width;
		self->bitsHigh[encoding] = glyph->height;
		self->bitsWide[encoding] = glyph->width;
		self->xoffsets[encoding] = -glyph->xoffset;
		self->descents[encoding] = glyph->height - glyph->yoffset;

		int bytesNeeded = bytesPerRow * glyph->height;
		uint8_t *resultingBitmap = (uint8_t*) ooc_alloc_memory (bytesNeeded);
		if (!resultingBitmap) {
			self->bitmaps[encoding] = NULL;
			continue;
		} else {
			self->bitmaps[encoding] = resultingBitmap;
		}

		// Pattern is one string for all pixels.
		char *pattern = (char*) glyph->pattern;

		bool done = false;
		for (int y=0; !done && y < glyph->height; y++) {
			uint64_t bit = 1LU;
			bit <<= 63;
			uint64_t bits = 0;

			for (int x=0; x < glyph->width; x++) {
				int ch = *pattern++;
				if (!ch) {
					done = true;
					break;
				}
				else {
					if (ch == '*') {
						bits |= bit;
					}
				}
				bit >>= 1;
			}

			switch (bytesPerRow) {
			case 1:
				resultingBitmap[y] = (uint8_t) (bits >> 56);
				break;
			case 2: {
				uint16_t *resultingBitmap16 = (uint16_t*) resultingBitmap;
				resultingBitmap16[y] = (uint16_t) (bits >> 48);
				break;
			}
			case 4: {
				uint32_t *resultingBitmap32 = (uint32_t*) resultingBitmap;
				resultingBitmap32[y] = (bits >> 32);
				break;
			 }
			case 8: {
				uint64_t *resultingBitmap64 = (uint64_t*) resultingBitmap;
				resultingBitmap64[y] = bits;
				break;
			 }
			}
		}
	}

	self->spaceWidth = self->widths [' ' - self->firstCharacter];
	if (!self->spaceWidth)
		self->spaceWidth = self->widths ['X' - self->firstCharacter];

	self->totalCharacters = self->lastCharacter - self->firstCharacter + 1;
	return self;
}

static void FontPK_describe (FontPK* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self, FontPK);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%s", $(self, className));
	fprintf (outputFile, "(%s %dpt)", self->name, (int)self->pointSize);
}

FontPKClass *FontPKClass_init (FontPKClass *class)
{
	SET_SUPERCLASS(Font);

        SET_OVERRIDDEN_METHOD_POINTER(FontPK,describe);
        SET_OVERRIDDEN_METHOD_POINTER(FontPK,destroy);

	VALIDATE_CLASS_STRUCT(class);
	return class;
}

