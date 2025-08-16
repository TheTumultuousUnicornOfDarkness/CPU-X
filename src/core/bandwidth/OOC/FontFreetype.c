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

//-----------------------------------------------------------------------------
// Name:	FontFreetype
// Responsible:	For providing glyphs of high quality TTF fonts.
// What:	High quality fonts.
// Where:	Font subsystem.
// How: 	Overrides Font methods.
// Why: 	All font types must conform the Font interface.
// When:	N/A.
//-----------------------------------------------------------------------------

#include "FontFreetype.h"
#include "Utility.h"
#include "Log.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_FREETYPE2
#include <ft2build.h>
#include FT_GLYPH_H
#include FT_TRUETYPE_DRIVER_H
#endif

FontFreetypeClass *_FontFreetypeClass = NULL;

FontFreetype *FontFreetype_init (FontFreetype *self)
{
        ENSURE_CLASS_READY(FontFreetype);

        if (!self) {
                return NULL;
        }

#ifndef HAVE_FREETYPE2
	release(self);
	return NULL;
#else
        Font_init ((Font*) self);
        self->is_a = _FontFreetypeClass;

	self->dpi = kDefaultDPI;
	self->pointSize = 0.f;
	self->ascent = 0;
	self->descent = 0;
	self->height = 0;
	self->firstCharacter = -1;
	self->lastCharacter = -1;
	return self;
#endif
}

static void FontFreetype_destroy (Any *self_) 
{
#ifdef HAVE_FREETYPE2
	if (!self_) {
		return;
	}
	verifyCorrectClass(self_, FontFreetype);

	FontFreetype *self = self_;
	Font_destroy (self);
#endif
}

FontFreetype *FontFreetype_fromFile (const char* fontpath, int size)
{
	if (!fontpath) {
		return NULL;
	}
	if (!file_exists (fontpath)) {
		return NULL;
	}
	//==========

#ifndef HAVE_FREETYPE2
	return NULL;
#else
	FontFreetype *self = new(FontFreetype);

	self->family[0] = 0;

	char *filename = strrchr (fontpath, '/');
	if (filename) {
		strncpy (self->fullName, filename+1, sizeof(self->fullName)-1);
	} else {
		// File is in current dir?
		strncpy (self->fullName, fontpath, sizeof(self->fullName)-1);
	}

	if (strstr(filename, "Regular") != NULL) {
		self->italic = false;
		self->bold = false;
	} else {
		self->italic = strstr(filename, "Italic") != NULL 
			    || strstr(filename, "Oblique") != NULL;
		self->bold = strstr(filename, "Bold") != NULL;
	}

	// TODO Get this info from FT2.
	self->isFixedWidth = false;
	if (strstr(filename, "Fixed") != NULL || strstr(filename, "Mono") || has_prefix(filename, "Courier")) {
		self->isFixedWidth = true;
	}

	FT_Bitmap *bitmap = NULL;
	FT_BitmapGlyphRec *g;
	FT_Face face;
	FT_Glyph glyph;

	int err;
	FT_Library library;
	if ((err = FT_Init_FreeType (&library))) {
		Log_warning_printf (__FUNCTION__, "Freetype init error %d", err);
		return NULL;
	}

	if ((err = FT_New_Face (library, fontpath, 0, &face))) {
		Log_warning_printf (__FUNCTION__, "Freetype load error %d", err);
		FT_Done_FreeType (library);
		release(self);
		return NULL;
	}

	FT_Set_Char_Size (face, size << 6, 0, kDefaultDPI, 0);

	self->fixedWidth = face->num_fixed_sizes > 0;
	self->ascent = face->ascender >> 7;
	self->descent = face->descender >> 6;
	self->height = face->size->metrics.height >> 6;

	int bytesPerRow = sizeof(uint32_t);

	self->firstCharacter = 32;
	self->lastCharacter = 127;
	self->totalCharacters = self->lastCharacter - self->firstCharacter + 1;

	for (int i = self->firstCharacter; i <= self->lastCharacter; i++) {
		if ((err = FT_Load_Char(face, i, FT_LOAD_TARGET_MONO))) {
			fprintf(stderr, "Error %d loading char '%c'\n", err, i);
			continue;
		}

		if ((err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO))) {
			fprintf(stderr, "Error %d rendering char '%c'\n", err, i);
			continue;
		}

		if ((err = FT_Get_Glyph(face->glyph, &glyph))) {
			fprintf(stderr, "Error %d getting glyph '%c'\n", err, i);
			continue;
		}

                bitmap = &face->glyph->bitmap;
                g = (FT_BitmapGlyphRec *)glyph;

		int index = i -self->firstCharacter;

		self->widths [index] = bitmap->width;
		self->bitsWide[index] = bitmap->width;
		self->bitsHigh[index] = bitmap->rows;
		self->xoffsets[index] = g->left;
		self->bytesPerRow[index] = bytesPerRow;
		self->descents[index] = bitmap->rows - g->top;

		size_t bytesNeeded = bytesPerRow * bitmap->rows;
		uint32_t *resultingBitmap = (uint32_t*) calloc (bytesNeeded, 1);
		if (!resultingBitmap) {
			Log_perror (__FUNCTION__, "malloc");
		}

		self->bitmaps[index] = (uint8_t*) resultingBitmap;

		for (int y=0; y < bitmap->rows; y++) {
			uint32_t row = 0;
			uint32_t bit = 1<<31;
			for (int x=0; x < bitmap->width; x++) {
				int byteOffset = x / 8;
				int mask = 0x80 >> (x & 7);
                                if (bitmap->buffer [byteOffset + y*bitmap->pitch] & mask) {
                                        row |= bit;
                                }
				bit >>= 1;
			}

			resultingBitmap[y] = row;
		}

		FT_Done_Glyph(glyph);
	}

	FT_Done_FreeType(library);

	self->spaceWidth = self->widths['x' -self->firstCharacter];
	return self;
#endif // HAVE_FREETYPE2
}

static void FontFreetype_describe (FontFreetype* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self, FontFreetype);

	if (!outputFile) {
		outputFile = stdout;
	}
	fprintf (outputFile, "%s", $(self, className));
	fprintf (outputFile, "(%s %dpt)", self->name, (int)self->pointSize);
}

FontFreetypeClass *FontFreetypeClass_init (FontFreetypeClass *class)
{
        SET_SUPERCLASS(Font);

        SET_OVERRIDDEN_METHOD_POINTER(FontFreetype,describe);
        SET_OVERRIDDEN_METHOD_POINTER(FontFreetype,destroy);

        VALIDATE_CLASS_STRUCT(class);
        return class;
}

bool FontFreetype_isAvailable ()
{
#ifdef HAVE_FREETYPE2
	return true;
#else
	return false;
#endif // HAVE_FREETYPE2
}
