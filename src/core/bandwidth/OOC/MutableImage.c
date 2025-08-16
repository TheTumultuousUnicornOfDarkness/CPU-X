/*============================================================================
  MutableImage, an object-oriented C image manipulation class.
  Copyright (C) 2009-2019 by Zack T Smith.

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
#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>

#include "Log.h"
#include "MutableImage.h"
#include "minifont.h"
#include "colors.h"

// Narrowest possible numbers.
static char* narrow_nums [] = 
{
	" # ",
	"# #",
	"# #",
	"# #",
	"# #",
	"# #",
	" # ",

	" #",
	"##",
	" #",
	" #",
	" #",
	" #",
	" #",

	" # ",
	"# #",
	"  #",
	" ##",
	"#  ",
	"#  ",
	"###",

	"###",
	"  #",
	" # ",
	"## ",
	"  #",
	"# #",
	" # ",

	"# #",
	"# #",
	"# #",
	"###",
	"  #",
	"  #",
	"  #",

	"###",
	"#  ",
	"## ",
	"  #",
	"  #",
	"# #",
	" # ",


	" # ",
	"#  ",
	"#  ",
	"## ",
	"# #",
	"# #",
	" # ",

	"###",
	"  #",
	"  #",
	" # ",
	" # ",
	" # ",
	" # ",

	" # ",
	"# #",
	"# #",
	" # ",
	"# #",
	"# #",
	" # ",

	" # ",
	"# #",
	"# #",
	" ##",
	"  #",
	" # ",
	"#  ",

	" ",
	"",
	"",
	" ",
	"",
	"",
	"#",
};

MutableImageClass *_MutableImageClass = NULL;

void MutableImage_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClass(self_, MutableImage);

	MutableImage *self = self_;

	release(self->croppingStack);
	self->croppingStack = NULL;

	release(self->animations);
	self->animations = NULL;

	Image_destroy (self);
}

static void 
MutableImage_describe (MutableImage* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,MutableImage);
	
	fprintf (file ?: stdout, "%s: %dx%d\n", $(self, className), self->width, self->height);
}

MutableImage* MutableImage_init (MutableImage* self)
{
	ENSURE_CLASS_READY(MutableImage);

	if (self) {
		Image_init ((Image*) self);
		self->is_a = _MutableImageClass;

		if (self->croppingStack) {
			release(self->croppingStack);
		}

		self->croppingStack = retain(new(RectStack));

		// The default MutableImage has no pixels.
		self->width = 0;
		self->height = 0;
		self->pixels = NULL;
	}
	return self;
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_putPixel
 * Purpose:	Writes pixel into MutableImage.
 *-------------------------------------------------------------------------*/
static void
MutableImage_putPixel (MutableImage *self, int x, int y, RGB rgb)
{
	if (!self || x<0 || y<0)
		return;
	verifyCorrectClass(self,MutableImage);
	if (!self->pixels)
		return;
	if (x >= self->width || y >= self->height)
		return;

	if (!$(self->croppingStack, isEmpty)) {
		Rect cropRect = $(self->croppingStack, tos);
		bool inside = Rect_containsPoint (cropRect, Point_new (x,y));
		if (!inside)
			return;
	}

	//----------
	self->pixels[y * self->width + x] = rgb;

	if (x < self->boundsWhereModified.x0)
		self->boundsWhereModified.x0 = x;
	if (y < self->boundsWhereModified.y0)
		self->boundsWhereModified.y0 = y;
	if (x > self->boundsWhereModified.x1)
		self->boundsWhereModified.x1 = x;
	if (y > self->boundsWhereModified.y1)
		self->boundsWhereModified.y1 = y;
	self->isModified = true;
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawHorizontalLine
 * Purpose:	Draws horizontal line.
 *-------------------------------------------------------------------------*/
static void
MutableImage_drawHorizontalLine (MutableImage *self, int x0, int x1, int y, RGB rgb)
{
//printf("%s y %d color %lx\n",__FUNCTION__,y,rgb);
	verifyCorrectClass(self,MutableImage);
	if (x0 > x1) {
		int tmp = x1;
		x1 = x0;
		x0 = tmp;
	}
	
	// XX optimize
	while (x0 <= x1) {
		MutableImage_putPixel (self, x0++, y, rgb);
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_fillRectangle
 * Purpose:	Fills a rectangle with a color.
 *-------------------------------------------------------------------------*/
static void
MutableImage_fillRectangle (MutableImage *self, int x, int y, int w, int h, RGB rgb)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	if (!self->width || !self->height || !self->pixels) {
		return;
	}
	if (x >= self->width || y >= self->height) {
		return;
	}
	int x1 = x + w - 1;
	int y1 = y + h - 1;
	if (x1 < 0 || y1 < 0)
		return;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x1 >= self->width)
		x1 = self->width-1;
	if (y1 >= self->height)
		y1 = self->height-1;

	while (h > 0) {
		MutableImage_drawHorizontalLine (self, x, x1, y, rgb);
		h--;
		y++;
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawVerticalLine
 * Purpose:	Draws vertical line.
 *-------------------------------------------------------------------------*/
static void
MutableImage_drawVerticalLine (MutableImage *self, int x, int y0, int y1, RGB rgb)
{
	verifyCorrectClass(self,MutableImage);
	if (y0 > y1) {
		int tmp=y1;
		y1=y0;
		y0=tmp;
	}
	
	// XX optimize
	while (y0 <= y1) {
		MutableImage_putPixel (self, x, y0++, rgb);
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_line_core
 * Purpose:	Draws a line in an MutableImage.
 *-------------------------------------------------------------------------*/
static void
MutableImage_line_core (MutableImage *image, int x0, int y0, int x1, int y1, RGB rgb,
			int dashed)
{
	if ((rgb >> 24) == 0xff) {
		return;
	}

	int dot_counter = 0;

	if (!dashed && x0 == x1 && y0 == y1) 
		MutableImage_putPixel (image, x0, y0, rgb);
	else if (!dashed && x0 == x1)
		MutableImage_drawVerticalLine (image, x0, y0, y1, rgb);
	else if (!dashed && y0 == y1)
		MutableImage_drawHorizontalLine (image, x0, x1, y0, rgb);
	else {
		int j, x, y, dx, dy, e, xchange, s1, s2;

		// DDA, copied from my FramebufferUI project.

		x = x0;
		y = y0;
		s1 = 1;
		s2 = 1;

		dx = x1 - x0;
		if (dx < 0) {
			dx = -dx;
			s1 = -1;
		}

		dy = y1 - y0;
		if (dy < 0) {
			dy = -dy;
			s2 = -1;
		}

		xchange = 0;

		if (dy > dx) {
			int tmp = dx;
			dx = dy;
			dy = tmp;
			xchange = 1;
		}

		e = (dy<<1) - dx;
		j = 0;

		while (j <= dx) {
			j++;

			int draw = 1;
			if (dashed && (1 & (dot_counter >> 2))) 
				draw = 0;
			
			if (draw)
				MutableImage_putPixel (image, x, y, rgb);

			dot_counter++;

			if (e >= 0) {
				if (xchange)
					x += s1;
				else
					y += s2;
				e -= (dx << 1);
			}
			if (xchange) 
				y += s2;
			else
				x += s1;
			e += (dy << 1);
		}
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawLine
 * Purpose:	Draws a line in a MutableImage image.
 *-------------------------------------------------------------------------*/
static void
MutableImage_drawLine (MutableImage *self, int x0, int y0, int x1, int y1, RGB rgb)
{
	verifyCorrectClass(self,MutableImage);
	MutableImage_line_core (self, x0, y0, x1, y1, rgb, 0);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawDashedLine
 * Purpose:	Draws a dashed line in a MutableImage image.
 *-------------------------------------------------------------------------*/
static void
MutableImage_drawDashedLine (MutableImage *self, int x0, int y0, int x1, int y1, RGB rgb)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);
	MutableImage_line_core (self, x0, y0, x1, y1, rgb, 1);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawRectangle
 * Purpose:	Draws a rectangle with a color.
 *-------------------------------------------------------------------------*/
static void
MutableImage_drawRectangle (MutableImage *self, int x, int y, int w, int h, RGB rgb)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	MutableImage_drawHorizontalLine (self, x, x+w-1, y, rgb);
	MutableImage_drawHorizontalLine (self, x, x+w-1, y+h-1, rgb);
	MutableImage_drawVerticalLine (self, x, y+1, y+h-2, rgb);
	MutableImage_drawVerticalLine (self, x+w-1, y+1, y+h-2, rgb);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_fillRect
 * Purpose:	Fills a rectangle with a color.
 *-------------------------------------------------------------------------*/
static void
MutableImage_fillRect (MutableImage *self, Rect r, RGB rgb)
{
	if (!self) { 
		return;
	}
	MutableImage_fillRectangle (self, r.origin.x, r.origin.y, r.size.width, r.size.height, rgb);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_fillRectUsingVerticalGradient
 * Purpose:	Fills a rectangle with a color.
 *-------------------------------------------------------------------------*/
static void
MutableImage_fillRectUsingVerticalGradient (MutableImage *self, Rect rect, RGB upper, RGB lower)
{
	if (!self || !self->pixels) {
		return;
	}
#if WAIT
	if (self->usingDPI) {
		rect.origin = pointToPixels (rect.origin);
		rect.size = sizeToPixels (rect.size);
	}
#endif

	float initial_red = (255 & (upper>>16));
	float initial_green = (255 & (upper>>8));
	float initial_blue = (255 & upper);
	float delta_red = (255 & (lower>>16)) - initial_red;
	float delta_green = (255 & (lower>>8)) - initial_green;
	float delta_blue = (255 & lower) - initial_blue;

	int leftEdge = rect.origin.x;
	int rightEdge = leftEdge + rect.size.width - 1;

	for (int j = 0; j < rect.size.height; j++) {
		float fraction = (float)j / (float)rect.size.height;
		int red = (int) (initial_red + fraction * delta_red);
		int green = (int) (initial_green + fraction * delta_green);
		int blue = (int) (initial_blue + fraction * delta_blue);
		red &= 0xff;
		green &= 0xff;
		blue &= 0xff;
		red <<= 16;
		green <<= 8;
		RGB color = red | green | blue;

		MutableImage_drawHorizontalLine (self, leftEdge, rightEdge, j + rect.origin.y, color);
	}
	
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawRect
 * Purpose:	Draws a rectangle with a color.
 *-------------------------------------------------------------------------*/
static void
MutableImage_drawRect (MutableImage *self, Rect r, RGB rgb)
{
	MutableImage_drawRectangle (self, r.origin.x, r.origin.y, r.size.width, r.size.height, rgb);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_rotate
 * Purpose:	Rotates entire image by specified standard angle.
 *-------------------------------------------------------------------------*/
static void
MutableImage_rotate (MutableImage *self, MutableImageRotationAngle angle)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	if (!self->pixels) {
		return;
	}

	size_t limit = self->width * self->height;
	if (!limit) {
		return;
	}

	if (angle == MutableImageRotationAngle180) {
		RGB *start = self->pixels;
		RGB *end = &self->pixels[limit - 1];
		while (start < end) {
			RGB color1 = *start;
			RGB color2 = *end;
			*start++ = color2;
			*end-- = color1;
		}
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_fillWithColor
 * Purpose:	Fills entire image with a color.
 *-------------------------------------------------------------------------*/
static void
MutableImage_fillWithColor (MutableImage *self, RGB color)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	size_t nPixels = self->width * self->height;
	if (!nPixels) {
		return;
	}
	if (!self->pixels) {
		return;
	}

	bool notCropping = $(self->croppingStack, isEmpty);
	if (notCropping) {
		RGB *pixels = self->pixels;
		for (size_t i = 0; i < nPixels; i++)
			*pixels++ = color;
	} else {
		Rect cropRect = $(self->croppingStack, tos);
		MutableImage_fillRect (self, cropRect, color);
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_clear
 * Purpose:	Sets all pixels to white.
 *-------------------------------------------------------------------------*/
static void
MutableImage_clear (MutableImage *self)
{
	verifyCorrectClass(self,MutableImage);
	MutableImage_fillWithColor (self, RGB_WHITE);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_grayscale
 * Purpose:	Converts entire image to grayscale.
 *-------------------------------------------------------------------------*/
static void
MutableImage_grayscale (MutableImage *self)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	if (!self->pixels) {
		return;
	}

	size_t limit = self->width * self->height;
	if (!limit) {
		return;
	}

	RGB *ptr = self->pixels;
	for (size_t i = 0; i < limit; i++) {
		RGB rgb = *ptr;
		RGB r = (rgb >> 16) & 0xff;
		RGB g = (rgb >> 8) & 0xff;
		RGB b = rgb & 0xff;
		RGB average = (r + g + b) / 3;
		rgb = average | (average << 8) | (average << 16);
		*ptr++ = rgb;
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_invert
 * Purpose:	Converts entire image to its color negative.
 *-------------------------------------------------------------------------*/
static void
MutableImage_invert (MutableImage *self)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	if (!self->pixels) {
		return;
	}

	size_t limit = self->width * self->height;
	if (!limit) {
		return;
	}

	RGB *ptr = self->pixels;
	for (size_t i = 0; i < limit; i++) {
		RGB rgb = *ptr;
		RGB r = (rgb >> 16) & 0xff;
		RGB g = (rgb >> 8) & 0xff;
		RGB b = rgb & 0xff;
		r = 0xff - r;
		g = 0xff - g;
		b = 0xff - b;
		rgb = b | (g << 8) | (r << 16);
		*ptr++ = rgb;
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawWideString
 * Purpose:	Draws characters into the image.
 *-------------------------------------------------------------------------*/
static int
MutableImage_drawWideString (MutableImage *self, const wchar_t *string, unsigned length, int x, int y, Font *font, RGB color)
{
	if (!self || !string || !length) {
		return 0;
	}
	if (!font) {
		Log_warning (__FUNCTION__, "No font specified.");
		return 0;
	}
	verifyCorrectClass(self,MutableImage);
	verifyCorrectClassOrSubclass(font,Font);
	if (x >= self->width || y >= self->height) {
		return 0;
	}
	//----------

	wchar_t firstChar = $(font, firstCharacter);
	wchar_t lastChar = $(font, lastCharacter);

	for (int index = 0; index < length; index++) {
		wchar_t ch = string[index];
	
		if (ch == ' ') {
			x += $(font, spaceWidth);
			continue;
		}
	
                if (ch < firstChar || ch > lastChar) {
                        continue;
		}
		
		unsigned bitsWide, bitsHigh, width;
		unsigned bytesPerRow;
		int descent, leftoffset;
		uint8_t *bitmap = (uint8_t*) $(font, bitmapForCharacter, ch, &width, &bytesPerRow, &bitsWide, &bitsHigh, &leftoffset, &descent);
		if (!bitmap) {
			char message[128];
			snprintf (message, sizeof(message), "Character %C of font %s (size %.1f) lacks bitmap!", (unsigned short)ch, font->name, font->pointSize);
			warning (__FUNCTION__, message);
			continue;
		}

		int charAscent = bitsHigh - descent;
		int yOffset = $(font,ascent) - charAscent;

		for (unsigned i=0; i < bitsHigh; i++) {
			uint64_t row = 0;
			switch (bytesPerRow) {
				case 1: row = *bitmap++; row <<= 56; break;
				case 2: row = *((uint16_t*) bitmap); bitmap += 2; row <<= 48; break; 
				case 4: row = *((uint32_t*) bitmap); bitmap += 4; row <<= 32; break;
				case 8: row = *((uint64_t*) bitmap); bitmap += 8; break;
			}

			int j=0;
			uint64_t mask = 1LU;
			mask <<= 63;
			while (row) {
				if (row & mask) {
					MutableImage_putPixel (self, x+j, y+i+yOffset, color);
				}
				j++;

				row <<= 1;
			}
		}

		x += width + leftoffset;
		x += kDefaultIntercharacterSpace;
	}

	return x;
}

static int
MutableImage_drawCString (MutableImage *self, const char *cstring, int x, int y, Font *font, RGB color)
{
	if (!self || !cstring || !font) {
		return 0;
	}
	unsigned len = strlen (cstring);
	wchar_t wide[len+1];
	for (int i=0; i < len; i++) {
		wide[i] = cstring[i];
	}
	wide[len] = 0;
	return MutableImage_drawWideString (self, (const wchar_t*) wide, len, x, y, font, color);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawString
 * Purpose:	Draws characters into the image.
 *-------------------------------------------------------------------------*/
static int
MutableImage_drawString (MutableImage *self, String *string, int x, int y, Font *font, RGB color)
{
	if (!self || !string || !font) {
		return 0;
	}
	verifyCorrectClass(self,MutableImage);
	verifyCorrectClassOrSubclass(font,Font);
	verifyCorrectClassOrSubclass(string,String);

	wchar_t *characters = $(string, characters);
	unsigned length = $(string, length);
	return MutableImage_drawWideString (self, characters, length, x, y, font, color);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawMiniString
 * Purpose:	Draws miniature 5x8 characters.
 *-------------------------------------------------------------------------*/
static int
MutableImage_drawMiniString (MutableImage *self, const char *string, int x, int y, RGB color)
{
	if (!self || !string) {
		return 0;
	}
	verifyCorrectClass(self,MutableImage);
	if (x >= self->width || y >= self->height || !*string) {
		return 0;
	}
	//----------

	char ch;
	const char *s;

#define MINI_HEIGHT (8)
	s = string;
	while ((ch = *s++)) {
		int ix = -1;
		if (ch == ' ') {
			x += 5;
			continue;
		}
		if (ch > 'z')
			continue;
		if (ch > ' ' && ch <= 'z')
			ix = MINI_HEIGHT * (ch - 33);
		
		if (ix >= 0) {
			int i;

			int width = 0;
			for (i=0; i<MINI_HEIGHT; i++) {
				int j=0;
				char ch2;
				const char *s2 = mini_chars[ix + i];
				int width2 = s2 ? ooc_strlen (s2) : 0;
				if (width < width2) {
					width = width2;
				}
				while ((ch2 = *s2++)) {
					if (ch2 == '#') {
						MutableImage_putPixel (self,x+j, y+i, color);
					}
					j++;
				}
			}

			x += width + 1/* kerning */;
		}
	}

	return x;
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_miniStringWidth
 * Purpose:	Gets width of miniature 5x8 characters.
 *-------------------------------------------------------------------------*/
static int
MutableImage_miniStringWidth (MutableImage *self, const char *string)
{
	if (!self || !string) {
		return 0;
	}
	verifyCorrectClass(self,MutableImage);
	//----------

	char ch;
	const char *s;
	int width = 0;

	s = string;
	while ((ch = *s++)) {
		int ix = -1;
		if (ch == ' ') {
			width += 5;
			continue;
		}
		if (ch > 'z')
			continue;
		if (ch > ' ' && ch <= 'z')
			ix = MINI_HEIGHT * (ch - 33);
		
		if (ix >= 0) {
			int max_w = 0;
			int j;
			for (j = 0; j < MINI_HEIGHT; j++) {
				const char *ptr = mini_chars [j+ix];
				int w = ptr ? ooc_strlen (ptr) : 0;
				if (max_w < w) max_w = w;
			}

			width += max_w + 1/*kerning*/;
		}
	}

	return width;
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_narrow_numbers
 * Purpose:	Draws miniature 4x7 characters.
 *-------------------------------------------------------------------------*/
static int
MutableImage_drawNarrowNumbers (MutableImage *self, const char *string, int x, int y, RGB color)
{
	if (!self || !string) {
		return 0;
	}
	verifyCorrectClass(self,MutableImage);
	if (x >= self->width || y >= self->height || !*string) {
		return 0;
	}
	//----------

	char ch;
	const char *s;

#define NARROW_HEIGHT (7)
	s = string;
	while ((ch = *s++)) {
		int ix = -1;
		if (ch == ' ') {
			x += 3;
			continue;
		}
		if (ch >= '0' && ch <= '9')
			ix = ch - '0';
		else
		if (ch == '.')
			ix = 10;
		
		ix *= NARROW_HEIGHT;
		
		if (ix >= 0) {
			int i;
			int width = ooc_strlen (narrow_nums [ix]);

			for (i=0; i<NARROW_HEIGHT; i++) {
				int j=0;
				char ch2;
				const char *s2 = narrow_nums [ix + i];
				while ((ch2 = *s2++)) {
					if (ch2 == '#') {
						MutableImage_putPixel (self, 
							x+j, y+i, color);
					}
					j++;
				}
			}

			x += width + 1;
		}
	}

	return x;
}

static void MutableImage_drawCirclePoints (MutableImage *self, int cx, int cy, int dx, int dy, unsigned quadrants, RGB rgb)
{
	if (quadrants & MutableImageCircleTopLeft) {
		MutableImage_putPixel (self, cx - dx, cy - dy, rgb);
		MutableImage_putPixel (self, cx - dy, cy - dx, rgb);
	}
	if (quadrants & MutableImageCircleTopRight) {
		MutableImage_putPixel (self, cx + dx, cy - dy, rgb);
		MutableImage_putPixel (self, cx + dy, cy - dx, rgb);
	}
	if (quadrants & MutableImageCircleBottomLeft) {
		MutableImage_putPixel (self, cx - dx, cy + dy, rgb);
		MutableImage_putPixel (self, cx - dy, cy + dx, rgb);
	}
	if (quadrants & MutableImageCircleBottomRight) {
		MutableImage_putPixel (self, cx + dx, cy + dy, rgb);
		MutableImage_putPixel (self, cx + dy, cy + dx, rgb);
	}
}

void MutableImage_drawCircle (MutableImage *self, int cx, int cy, unsigned radius, unsigned quadrants, RGB rgb)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	if (!quadrants || radius == 0) {
		return;
	}
	if (radius == 1) {
		MutableImage_putPixel (self, cx, cx, rgb);
		return;
	}

	int x = 0;
	int y = radius;
	int d = 3 - 2*radius;

	MutableImage_drawCirclePoints (self, cx, cy, x, y, quadrants, rgb);

	while (x <= y) {
		MutableImage_drawCirclePoints (self, cx, cy, x, y, quadrants, rgb);
		if (d < 0) {
			d += 4 * x + 6;
		} 
		else {
			d += 4 * (x - y) + 10;
			y--;
		}
		x++;
	} 
}

void MutableImage_fillCircle (MutableImage *self, int cx, int cy, unsigned radius, unsigned quadrants, RGB color)
{
	if (!self || !radius) {
		return;
	}

	int x = 0;
	int y = radius;
	int d = 3 - 2*radius;

	int values[radius + 1];

	while (x < y) {
		values [x] = y;
		
		if (d < 0) {
			d += 4 * x + 6;
		} 
		else {
			d += 4 * (x - y) + 10;
			y--;
		}
		x++;
	} 

	int nValues = x;
	int lastValue = values[0];

#ifdef GRATUITOUS_DEBUGGING
	for (int i=0; i < nValues; i++) {
		printf("CIRCLE CURVE VALUE %i = %i\n",i,values[i]);
	}
#endif
	int linesDrawnPerSide = radius - values[nValues-1];
	int x0 = cx - radius;
	int y0 = cy - radius;
	int width = 2 * radius + 1;
	int height = 2 * radius + 1;
	MutableImage_fillRectangle (self, x0 + linesDrawnPerSide, y0 + linesDrawnPerSide, width - 2*linesDrawnPerSide, height - 2*linesDrawnPerSide, color);

	// The delta is the distance from the axis.
	int delta = radius;

	// The i value is the location along that axis.
	int value = 0;
	int i = 0;
	while (i < nValues) {
		value = values[i];

		if (value != lastValue) {
			MutableImage_drawHorizontalLine (self, cx - i, cx + i, cy + delta, color);
			MutableImage_drawHorizontalLine (self, cx - i, cx + i, cy - delta, color);
			MutableImage_drawVerticalLine (self, cx + delta , cy - i, cy + i, color);
			MutableImage_drawVerticalLine (self, cx - delta , cy - i, cy + i, color);

			lastValue = value;

			delta--;
		}
		i++;
	}
	if (lastValue != value) {
		MutableImage_drawHorizontalLine (self, cx - i, cx + i, cy + delta, color);
		MutableImage_drawHorizontalLine (self, cx - i, cx + i, cy - delta, color);
		MutableImage_drawVerticalLine (self, cx + delta , cy - i, cy + i, color);
		MutableImage_drawVerticalLine (self, cx - delta , cy - i, cy + i, color);
	}
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_drawRoundedRectangle 
 * Purpose:	Draws a rounded rectangle.
 *-------------------------------------------------------------------------*/
void MutableImage_drawRoundedRectangle (MutableImage *self, int x0, int y0, int width, int height, int radius, RGB color)
{
	if (!self || !radius || !width || !height) {
		return;
	}

	if (width == height && radius > width/2) {
		radius = width/2;
	}
	else if (radius > width/2 || radius > height/2) {
		return;
	}

	int x1 = x0 + width - 1;
	int y1 = y0 + height - 1;

	if (width != height || radius < width/2) {
		MutableImage_drawLine (self, x0+radius, y0, x1-radius, y0, color);
		MutableImage_drawLine (self, x0+radius, y1, x1-radius, y1, color);
		MutableImage_drawLine (self, x0, y0+radius, x0, y1-radius, color);
		MutableImage_drawLine (self, x1, y0+radius, x1, y1-radius, color);
	}

	MutableImage_drawCircle (self, x0 + radius, y0 + radius, radius, MutableImageCircleTopLeft, color);
	MutableImage_drawCircle (self, x1 - radius, y0 + radius, radius, MutableImageCircleTopRight, color);
	MutableImage_drawCircle (self, x0 + radius, y1 - radius, radius, MutableImageCircleBottomLeft, color);
	MutableImage_drawCircle (self, x1 - radius, y1 - radius, radius, MutableImageCircleBottomRight, color);
}

/*---------------------------------------------------------------------------
 * Name:	MutableImage_fillRoundedRectangle 
 * Purpose:	Fills a rounded rectangle.
 *-------------------------------------------------------------------------*/
void MutableImage_fillRoundedRectangle (MutableImage *self, int x0, int y0, int width, int height, int radius, RGB color)
{
	if (!self || !radius || !width || !height) {
		return;
	}
	if (width == height && radius > width/2) {
		radius = width/2;
	}

	int x = 0;
	int y = radius;
	int d = 3 - 2*radius;

	int values[radius + 1];

	while (x < y) {
		values [x] = y;
		
		if (d < 0) {
			d += 4 * x + 6;
		} 
		else {
			d += 4 * (x - y) + 10;
			y--;
		}
		x++;
	} 

	int nValues = x;
	int lastValue = values[0];

	int x1 = x0 + width - 1;
	int y1 = y0 + height - 1;

	int linesDrawnPerSide = radius - values[nValues-1];
	MutableImage_fillRectangle (self, x0 + linesDrawnPerSide, y0 + linesDrawnPerSide, width - 2*linesDrawnPerSide, height - 2*linesDrawnPerSide, color);

	// The delta is the distance from the axis.
	int delta = radius;

	// The i value is the location along that axis.
	int value = 0;
	int i = 0;
	while (i < nValues) {
		value = values[i];

		if (value != lastValue) {
			MutableImage_drawHorizontalLine (self, x0 + radius - i, x1 - radius + i, y0 + radius - delta, color);
			MutableImage_drawHorizontalLine (self, x0 + radius - i, x1 - radius + i, y1 - radius + delta, color);
			MutableImage_drawVerticalLine (self, x0 + radius - delta , y0 + radius - i, y1 - radius + i, color);
			MutableImage_drawVerticalLine (self, x1 - radius + delta , y0 + radius - i, y1 - radius + i, color);

			lastValue = value;

			delta--;
		}
		i++;
	}
	if (lastValue != value) {
		MutableImage_drawHorizontalLine (self, x0 + radius - i, x1 - radius + i, y0 + radius - delta, color);
		MutableImage_drawHorizontalLine (self, x0 + radius - i, x1 - radius + i, y1 - radius + delta, color);
		MutableImage_drawVerticalLine (self, x0 + radius - delta , y0 + radius - i, y1 - radius + i, color);
		MutableImage_drawVerticalLine (self, x1 - radius + delta , y0 + radius - i, y1 - radius + i, color);
	}
}


void MutableImage_drawPixelsAt (MutableImage *self, Any* pixels, Point point, unsigned pixelCount)
{
	if (!self || !pixels || !pixelCount) {
		return;
	}
	verifyCorrectClass(self,MutableImage);
	int x = point.x;
	int y = point.y;
	if (x + pixelCount - 1 < 0) {
		return;
	}
	unsigned height = self->height;
	if (y < 0 || y >= height) {
		return;
	}
	unsigned width = self->width;
	if (x >= self->width) {
		return;
	}
	//----------

	int xmin = 0;
	int xmax = x + width - 1;
	if (!$(self->croppingStack, isEmpty)) {
		Rect cropRect = $(self->croppingStack, tos);
		int ymin = cropRect.origin.y;
		int ymax = ymin + cropRect.size.height - 1;
		if (y < ymin || y > ymax)
			return;
		xmin = cropRect.origin.x;
		xmax = xmin + cropRect.size.width - 1;
	}

	if (x < self->boundsWhereModified.x0)
		self->boundsWhereModified.x0 = x;
	if (y < self->boundsWhereModified.y0)
		self->boundsWhereModified.y0 = y;
	if (y > self->boundsWhereModified.y1)
		self->boundsWhereModified.y1 = y;

	// NOTE: Not as fast as memcpy but close enough for now.
	int index = y * width + x;
	int incomingIndex = 0; 
	switch (self->pixelFormat) {
	case PixelFormatRGB32: {
		RGB* incomingPixels = (RGB*) pixels;
		while (incomingIndex < pixelCount) { 
			if (x >= 0 && x >= xmin && x <= xmax && x < width)
				self->pixels[index] = incomingPixels [incomingIndex];
			index++;
			incomingIndex++;
			x++;
		}
		break;
	 }
	case PixelFormatRGB565: {
		RGB16* incomingPixels = (RGB16*) pixels;
		RGB16* destinationPixels = (RGB16*) self->pixels;
		while (incomingIndex < pixelCount) { 
			if (x >= 0 && x >= xmin && x <= xmax && x < width)
				destinationPixels[index] = incomingPixels [incomingIndex];
			index++;
			incomingIndex++;
			x++;
		}
		break;
	 }
	}

	// Back to last modified pixel.
	x--; 

	if (x > self->boundsWhereModified.x1) {
		self->boundsWhereModified.x1 = x;
	}

	self->isModified = true;
}

void MutableImage_putImageAt (MutableImage *self, Any *image_, Point point)
{
	if (!self || !image_) {
		return;
	}
	verifyCorrectClass(self,MutableImage);
	verifyCorrectClassOrSubclass(self,Image);

	Image *image = image_;
	unsigned iw = $(image, width);
	unsigned ih = $(image, height);
	if (!iw || !ih) {
		return;
	}

	if (point.x >= self->width || point.y >= self->height) {
		return;
	}

	int rightEdge = point.x + self->width - 1;
	int bottomEdge = point.y + self->height - 1;
	if (rightEdge < 0 || bottomEdge < 0) {
		return;
	}
	//==========

	int x0 = point.x;
	int y0 = point.y;

	if (iw + x0 > self->width) {
		iw = self->width - x0;
	}

	for (int y = 0; y < ih; y++) {
		RGB* rowAddress = (RGB*) $(image, pixelRowAddress, y);
		MutableImage_drawPixelsAt (self, rowAddress, Point_new(x0, y0+y), iw);
	}
}

void MutableImage_drawTestPattern (MutableImage *self)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	unsigned width = self->width;
	unsigned height = self->height;
	RGB *pixels = self->pixels;
	if (!pixels || !width || !height) {
		return;
	}

	for (int j=0; j<5; j++) {
		for (int i=0; i < width; i++) 
			pixels[width*j+i] = RGB_RED;
	}
	for (int j=5; j<10; j++) {
		for (int i=0; i < width; i++) 
			pixels[width*j+i] = RGB_GREEN;
	}
	for (int j=10; j<15; j++) {
		for (int i=0; i < width; i++) 
			pixels[width*j+i] = RGB_BLUE;
	}
	for (int j=15; j<height; j++) {
		uint32_t r = 255 - (j * 255) / height;
		for (int i=0; i < width; i++) {
			uint32_t g = 255 - (i * 255) / width;
			pixels[width*j+i] = (r << 16) | (g << 8) | 128;
		}
	}
	for (int i=0; i<width && i<height; i++) {
		pixels[i+width*i] = RGB_WHITE; // Diagonal from left
		pixels[i+width*i+1] = RGB_WHITE; // Diagonal from left
		pixels[width-1-i+width*i] = RGB_CYAN; // Diagonal from right
		pixels[width-2-i+width*i] = RGB_CYAN; // Diagonal from right
	}
}

static void reallocate_pixels (MutableImage* self, unsigned width, unsigned height)
{
	if (!self) { 
		return;
	}

	if (self->pixels) {
		ooc_free (self->pixels);
		self->pixels = NULL;
	}

	$(self->croppingStack, empty);

	if (width > 0 && height > 0) {
		self->width = width;
		self->height = height;

		size_t size = width * height * sizeof(RGB);
		self->pixels = (RGB*) ooc_alloc_memory(size);
		if (self->pixels != NULL) {
			MutableImage_clear (self);
		}
	}
	else {
		self->width = 0;
		self->height = 0;
	}
}
	
MutableImage* MutableImage_initWithSize (MutableImage* self, unsigned width, unsigned height)
{
	ENSURE_CLASS_READY(MutableImage);

	if (self) {
		MutableImage_init(self);

		reallocate_pixels (self, width, height);
	}
	return self;
}

void MutableImage_resize (MutableImage* self, unsigned width, unsigned height)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	if (self->pixels) {
		ooc_free (self->pixels);
		self->pixels = NULL;
		self->width = 0;
		self->height = 0;
	}

	reallocate_pixels (self, width, height);
}

MutableImage *MutableImage_withSize (unsigned width, unsigned height)
{
	MutableImage* image = allocate(MutableImage);
	return MutableImage_initWithSize (image, width, height);
}

void MutableImage_clearIsModified (MutableImage* self)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

	self->isModified = false;
}

bool MutableImage_isModified (MutableImage* self)
{
	if (!self) { 
		return false;
	}
	verifyCorrectClass(self,MutableImage);

	return self->isModified;
}

void MutableImage_startAnimation (MutableImage *self, Any *animation)
{
}

void MutableImage_continueAnimations (MutableImage *self)
{
}

bool MutableImage_pushCrop (MutableImage *self, Rect crop)
{
	if (!self) { 
		return false;
	}
	verifyCorrectClass(self,MutableImage);

	bool validCrop = true;

#ifdef NOT_YET
	if (self->usingDPI) {
		crop.origin = pointToPixels (crop.origin);
		crop.size = sizeToPixels (crop.size);
	}
#endif
	if (!$(self->croppingStack, isEmpty)) {
		Rect currentCropRect = $(self->croppingStack, tos);
		if (currentCropRect.size.width && currentCropRect.size.height) {
			int leftEdge = crop.origin.x;
			int topEdge = crop.origin.y;
			int rightEdge = leftEdge + crop.size.width - 1;
			int bottomEdge = topEdge + crop.size.height - 1;

			int currentLeftEdge = currentCropRect.origin.x;
			int currentTopEdge = currentCropRect.origin.y;
			int currentRightBoundary = currentLeftEdge + currentCropRect.size.width;
			int currentBottomBoundary = currentTopEdge + currentCropRect.size.height;

			// RULE: If new crop rect doesn't overlap current one at all, no drawing should be done.
			if ((leftEdge >= currentRightBoundary)
				||
				(topEdge >= currentBottomBoundary)
				||
				(rightEdge < currentLeftEdge)
				||
				(bottomEdge < currentTopEdge)
			   ) 
			{
				Log_debug(__FUNCTION__, "Zero crop!");
				crop = Rect_zero();
				validCrop = false;
			}
			else {
				if (leftEdge < currentLeftEdge) {
					int difference = currentLeftEdge - leftEdge;
					crop.origin.x = currentLeftEdge;
					crop.size.width -= difference;
				}
				if (rightEdge >= currentRightBoundary) {
					int difference = rightEdge - currentRightBoundary + 1;
					crop.size.width -= difference;
				}
				if (topEdge < currentTopEdge) {
					int difference = currentTopEdge - topEdge;
					crop.origin.y = currentTopEdge;
					crop.size.height -= difference;
				}
				if (bottomEdge >= currentBottomBoundary) {
					int difference = bottomEdge - currentBottomBoundary + 1;
					crop.size.height -= difference;
				}
			}
		}
	}

        $(self->croppingStack, push, crop);

	return validCrop;
}

void MutableImage_popCrop (MutableImage *self)
{
	if (!self) { 
		return;
	}
	verifyCorrectClass(self,MutableImage);

        if (!$(self->croppingStack, isEmpty)) {
        	$(self->croppingStack, pop);
	}
}

MutableImageClass* MutableImageClass_init (MutableImageClass *class)
{
	SET_SUPERCLASS(Image);

	SET_OVERRIDDEN_METHOD_POINTER(MutableImage, describe);
	SET_OVERRIDDEN_METHOD_POINTER(MutableImage, destroy);

	SET_INHERITED_METHOD_POINTER(MutableImage,Image,pixelAt);
	SET_INHERITED_METHOD_POINTER(MutableImage,Image,writeBMP);
	SET_INHERITED_METHOD_POINTER(MutableImage,Image,size);
	SET_INHERITED_METHOD_POINTER(MutableImage,Image,print);
	SET_INHERITED_METHOD_POINTER(MutableImage,Image,equals);

	SET_METHOD_POINTER(MutableImage, clear);
	SET_METHOD_POINTER(MutableImage, drawCircle);
	SET_METHOD_POINTER(MutableImage, drawDashedLine);
	SET_METHOD_POINTER(MutableImage, drawHorizontalLine);
	SET_METHOD_POINTER(MutableImage, drawLine);
	SET_METHOD_POINTER(MutableImage, drawMiniString);
	SET_METHOD_POINTER(MutableImage, drawNarrowNumbers);
	SET_METHOD_POINTER(MutableImage, drawString);
	SET_METHOD_POINTER(MutableImage, putPixel);
	SET_METHOD_POINTER(MutableImage, drawVerticalLine);
	SET_METHOD_POINTER(MutableImage, drawCString);
	SET_METHOD_POINTER(MutableImage, drawWideString);
	SET_METHOD_POINTER(MutableImage, drawRect);
	SET_METHOD_POINTER(MutableImage, drawRoundedRectangle);
	SET_METHOD_POINTER(MutableImage, drawRectangle);
	SET_METHOD_POINTER(MutableImage, grayscale);
	SET_METHOD_POINTER(MutableImage, fillCircle);
	SET_METHOD_POINTER(MutableImage, fillRect);
	SET_METHOD_POINTER(MutableImage, fillRectangle);
	SET_METHOD_POINTER(MutableImage, fillRoundedRectangle);
	SET_METHOD_POINTER(MutableImage, fillRectUsingVerticalGradient);
	SET_METHOD_POINTER(MutableImage, fillWithColor);
	SET_METHOD_POINTER(MutableImage, miniStringWidth);
	SET_METHOD_POINTER(MutableImage, invert);
	SET_METHOD_POINTER(MutableImage, rotate);
	SET_METHOD_POINTER(MutableImage, resize);
	SET_METHOD_POINTER(MutableImage, isModified);
	SET_METHOD_POINTER(MutableImage, clearIsModified);
	SET_METHOD_POINTER(MutableImage, putImageAt);
	SET_METHOD_POINTER(MutableImage, pushCrop);
	SET_METHOD_POINTER(MutableImage, popCrop);
	SET_METHOD_POINTER(MutableImage, startAnimation);
	SET_METHOD_POINTER(MutableImage, continueAnimations);
	SET_METHOD_POINTER(MutableImage, drawPixelsAt);
	SET_METHOD_POINTER(MutableImage, drawTestPattern);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

