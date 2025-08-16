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

#include <stdlib.h>
#include <string.h>

#include "Font.h"
#include "FontBuiltin.h"
#include "FontPK.h"

FontClass *_FontClass = NULL;

void Font_destroy (Any *self_)
{
        DEBUG_DESTROY;

	if (!self_) {
		return;
	}
	verifyCorrectClassOrSubclass(self_,Font);

	Font *self = self_;

        if (self->bitmapBuffer) {
                free (self->bitmapBuffer);
                self->bitmapBuffer = NULL;
        } else {
                for (int i=0; i <= MAXCHARCODE; i++) {
			uint8_t *ptr = self->bitmaps[i];
			if (ptr != NULL)
                        	ooc_free (ptr);
		}
        }
}

void Font_print (Font* self, FILE *outputFile) 
{
	if (!self) { 
		return;
	}
	verifyCorrectClassOrSubclass(self,Font);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%s %gpt", self->family, self->pointSize);
}

static void Font_describe (Font* self, FILE *outputFile) 
{ 
	if (!self) { 
		return;
	}
	verifyCorrectClassOrSubclass(self,Font);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "Font name=%s\n",self->name);
	fprintf (outputFile, "Font family=%s\n",self->family);
	fprintf (outputFile, "Font fullName=%s\n",self->fullName);
	fprintf (outputFile, "Font size=%g\n",self->pointSize);

	fprintf (outputFile, "%s", $(self, className));
}

Font *Font_new () 
{
	return new(Font);
}

Font* Font_with (const char* name, int size, bool bold, bool italic)
{
	// TODO
	(void)name;
	(void)size;
	(void)bold;
	(void)italic;

	Font *self = Font_new ();
	
	return self;
}

void* Font_bitmapForCharacter (Font *self, wchar_t characterCode, 
				unsigned* width, unsigned* bytesPerRow, 
				unsigned* bitsWide, unsigned* bitsHigh, 
				int* xoffset, int* descent)  
{
	if (!self) { 
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Font);

	if (characterCode < self->firstCharacter || characterCode > self->lastCharacter) {
		return NULL;
	}

	// NOTE: Not supporting Unicode yet.
	if (characterCode > 255) {
		return NULL;
	}

	int index = characterCode - self->firstCharacter;
	if (width)
		*width = self->widths [index];
	if (bytesPerRow)
		*bytesPerRow = self->bytesPerRow [index];
	if (bitsWide)
		*bitsWide = self->bitsWide [index];
	if (bitsHigh)
		*bitsHigh = self->bitsHigh [index];
	if (xoffset)
		*xoffset = self->xoffsets [index];
	if (descent)
		*descent = self->descents [index];

	void *bitmaps = self->bitmaps[index];
	return bitmaps;
}

static void Font_sizeOfWideString (Font *self, wchar_t *string, unsigned length, int* w, int* a, int* d)      
{
	if (!self || !string || !length) {
		return;
	}
	if (!w || !a || !d) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Font);

	int totalWidth = 0;

	int i = 0;
	while (i < length) {
		wchar_t ch = string[i++];
		if (!ch)
			break;
		if (ch == ' ') {
			totalWidth += self->spaceWidth;
			continue;
		}

		int index = ch - self->firstCharacter;
		if (index >= 0 && index < self->totalCharacters) {
			totalWidth += self->widths [index]; 
			totalWidth += self->xoffsets [index];
			totalWidth += kDefaultIntercharacterSpace;
		}
	}

	if (w) *w = totalWidth;
	if (a) *a = self->ascent;
	if (d) *d = self->descent;
}

static void Font_sizeOfString (Font *self, String *string, int* w, int* a, int* d)      
{
	if (!self) { 
		return;
	}
	verifyCorrectClassOrSubclass(self,Font);
	verifyCorrectClassOrSubclass(string,String);

	wchar_t *chars = $(string, characters);
	unsigned length = $(string, length);
	Font_sizeOfWideString (self, chars, length, w, a, d);
}

static int Font_stringWidth (Font *self, Any *string_)
{
	if (!self) { 
		return 0;
	}
	if (!string_) {
		return 0;
	}

	verifyCorrectClassOrSubclass(self,Font);
	verifyCorrectClassOrSubclass(string_,String);
	String *string = string_;

	int width, ascent, descent;
	Font_sizeOfString (self, string, &width, &ascent, &descent);
	return width;
}

float Font_pointSize (Font *self)  
{
	if (!self) { 
		return 0.f;
	}
	return self->pointSize;
}

short Font_ascent (Font *self)     
{
	if (!self) { 
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Font);
	return self->ascent;
}

short Font_descent (Font *self)    
{
	if (!self) { 
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Font);
	return self->descent;
}

short Font_spaceWidth (Font *self) 
{
	if (!self) { 
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Font);
	return self->spaceWidth;
}

short Font_height (Font *self)     
{
	if (!self) { 
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Font);
	return self->height;
}

wchar_t Font_firstCharacter (Font *self)   
{
	if (!self) { 
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Font);
        return self->firstCharacter;
}

wchar_t Font_lastCharacter (Font *self)    
{
	if (!self) { 
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Font);
        return self->lastCharacter;
}

long Font_totalCharacters (Font *self)
{
	if (!self) { 
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Font);
        return self->lastCharacter - self->firstCharacter;
}

bool Font_equals (Font *self, Any *other_)
{ 
	if (!self || !other_) {
		return false;
	}
	verifyCorrectClass(self,Font);
	Font *other = (Font*)other_;
	verifyCorrectClass(other,Font);
	if (self->is_a != other->is_a) {
		return false;
	}

	if (self->pointSize == other->pointSize) {
		return false;
	}
	if (self->bold == other->bold) {
		return false;
	}
	if (self->italic == other->italic) {
		return false;
	}
	if (strcasecmp (self->name, other->name)) {
		return false;
	}
	if (strcasecmp (self->family, other->family)) {
		return false;
	}

	return true;
}

FontClass* FontClass_init (FontClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Font,describe);
	SET_OVERRIDDEN_METHOD_POINTER(Font,destroy);
	SET_OVERRIDDEN_METHOD_POINTER(Font,print);
	SET_OVERRIDDEN_METHOD_POINTER(Font,equals);

	SET_METHOD_POINTER(Font,pointSize);
	SET_METHOD_POINTER(Font,stringWidth);
	SET_METHOD_POINTER(Font,totalCharacters);
	SET_METHOD_POINTER(Font,ascent);
	SET_METHOD_POINTER(Font,descent);
	SET_METHOD_POINTER(Font,height);
	SET_METHOD_POINTER(Font,spaceWidth);
	SET_METHOD_POINTER(Font,sizeOfString);
	SET_METHOD_POINTER(Font,sizeOfWideString);
	SET_METHOD_POINTER(Font,bitmapForCharacter);
	SET_METHOD_POINTER(Font,firstCharacter);
	SET_METHOD_POINTER(Font,lastCharacter);
	
        VALIDATE_CLASS_STRUCT(class);
	return class;
}

Font* Font_init (Font *self)
{
	ENSURE_CLASS_READY(Font);

	if (self) {
		Object_init ((Object*) self);
		self->is_a = _FontClass;
		
		self->isMonochrome = true;
		self->name[0] = 0;
		self->fullName[0] = 0;
		self->family[0] = 0;

		for (int i=0; i < MAXCHARS; i++)
			self->bitmaps[i] = NULL;

		self->rowUnit = RowUnitByte;
	}

	return self;
}

