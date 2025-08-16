/*============================================================================
  FontBuiltin, an object-oriented C basic font.
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

//-----------------------------------------------------------------------------
// Name:	FontBuiltin
// Responsible:	For providing my built-in, handmade font.
// What:	Handmade font.
// Where:	Font subsystem.
// How: 	Piggybacks on Font methods.
//-----------------------------------------------------------------------------

#include "Font.h"
#include "FontBuiltin.h"
#include "basicfont.h" // 14pt 
#include "Log.h"

#include <string.h>

FontBuiltinClass *_FontBuiltinClass = NULL;

extern FontClass *_FontClass;

void FontBuiltin_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_)
		return;
	verifyCorrectClass(self_,FontBuiltin);

	Font_destroy (self_);
}

FontBuiltin* FontBuiltin_init (FontBuiltin *self)
{
	ENSURE_CLASS_READY(Font);

	if (self) {
		Font_init ((Font*) self);
		self->is_a = (void*) _FontClass;

		strncpy (self->family, "Builtin", kMaxFontNameSize);

		self->dpi = 120;
		self->pointSize = 0.f;
		self->ascent = 0;
		self->descent = 0;
		self->height = 0;
		self->firstCharacter = 33;
		self->lastCharacter = 'z';
	}
	return self;
}

FontBuiltin* FontBuiltin_with (const char* name, int size, bool bold, bool italic)
{
	(void)name;
	(void)bold;
	(void)italic;

	FontBuiltin* self = new(FontBuiltin);

	self->pointSize = 14;
	self->bold = false;
	self->italic = false;

	snprintf (self->fullName, kMaxFontNameSize, "Builtin %d", size);

	self->isFixedWidth = false;
	self->fixedWidth = 0;

	const char **patterns = NULL;

	if (size == 0 || size == 14) {
		self->firstCharacter = 32; 
       		self->lastCharacter = 122;
		self->totalCharacters = self->lastCharacter - self->firstCharacter + 1;
       		self->ascent = 14;
       		self->descent = 3;
		self->spaceWidth = 5;
		patterns = basicfont_chars;
	} else {
		free (self);
		return NULL;
	}

	self->height = self->ascent + self->descent;

	for (int i=0 ; i < self->totalCharacters; i++) {
		self->descents[i] = self->descent;
	}

	int characterHeight = self->ascent + self->descent;

	int maxWidth = 0;
	for (int i=0; i < self->totalCharacters; i++) {
		int maxBits = 0;
		int start = i * characterHeight;

		for (int j = 0; j < characterHeight; j++) {
			int len = ooc_strlen (patterns [start+j]);
			if (maxBits < len)
				maxBits = len;
		}

		if (maxWidth < maxBits)
			maxWidth = maxBits;

		self->widths [i] = maxBits;
		self->bitsHigh [i] = characterHeight;
	}

	int bytesPerRow = 0;
	if (maxWidth > 16) {
		self->rowUnit = RowUnitDword;
		bytesPerRow = 4;
	}
	else if (maxWidth > 8) {
		self->rowUnit = RowUnitWord;
		bytesPerRow = 2;
	}
	else {
		self->rowUnit = RowUnitByte;
		bytesPerRow = 1;
	}

	for (int i=0; i < self->totalCharacters; i++) {
		self->bytesPerRow[i] = bytesPerRow;
		self->bitsWide[i] = self->bytesPerRow[i] * 8;

		int bytesNeededForPattern = characterHeight * self->bytesPerRow[i];
		uint8_t *bitmap = (uint8_t*) malloc (bytesNeededForPattern);
		if (bitmap == NULL) {
			Log_perror (__FUNCTION__, "malloc");
			break;
		}
		self->bitmaps[i] = bitmap;

		int patternOffset = i * characterHeight;
		for (int j=0; j < characterHeight; j++) {
			uint32_t row = 0;
			const char *rowString = patterns [patternOffset + j];
			uint32_t bit = 0x80000000;
			char ch;
			while ((ch = *rowString)) {
				if (ch != ' ')
					row |= bit;
				bit >>= 1;
				rowString++;
			}

			switch (self->rowUnit) {
			case RowUnitByte: 
				row >>= 24;
				*bitmap++ = (uint8_t) row;
				break;
			case RowUnitWord: {
				uint16_t *tmp = (uint16_t*) bitmap;
				row >>= 16;
				*tmp = (uint16_t) row;
				bitmap += 2;
				break;
			 }
			case RowUnitDword: {
				uint32_t *tmp = (uint32_t*) bitmap;
				*tmp = row;
				bitmap += 4;
				break;
			 }
			}
		}
	}

	return self;
}

static void FontBuiltin_describe (FontBuiltin* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self, FontBuiltin);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%s", $(self, className));
	fprintf (outputFile, "(%s %dpt)", self->name, (int)self->pointSize);
}

FontBuiltinClass *FontBuiltinClass_init (FontBuiltinClass *class)
{
	SET_SUPERCLASS(Font);

        SET_OVERRIDDEN_METHOD_POINTER(FontBuiltin,describe);

	VALIDATE_CLASS_STRUCT(class);
	return class;
}

