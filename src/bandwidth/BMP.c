
/*=============================================================================
  bmplib, a simple library to create, modify, and write BMP image files.
  Copyright (C) 2009-2014 by Zack T Smith.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  The author may be reached at veritas@comcast.net.
 *============================================================================*/

//--------------------------------------------------
// Change Log
// 0.8 ZS	Added larger font of my own design.
// 0.9 ZS	Removed attempt at anti-aliasing.
//--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BMP.h"
#include "font.h"
#include "minifont.h"

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


/*---------------------------------------------------------------------------
 * Name:	BMP_new
 * Purpose:	Creates new image.
 *-------------------------------------------------------------------------*/
BMP* 
BMP_new (int w, int h)
{
	unsigned long size;
	BMP* nu;
	if (w<1 || h<1)
		return NULL;
	//----------

	if (w & 3) 
		w += 4 - (w & 3);
	if (h & 3) 
		h += 4 - (h & 3);

	nu = (BMP*) malloc (sizeof (BMP));
	if (!nu)
		return NULL;
	memset (nu, 0, sizeof (BMP));
	nu->width = w;
	nu->height = h;
	size = w * h * sizeof (long);
	nu->pixels = (RGB*) malloc (size);
	if (!nu->pixels) {
		free (nu);
		return NULL;
	}
	memset (nu->pixels, 0, size);
	return nu;
}

/*---------------------------------------------------------------------------
 * Name:	BMP_destroy
 * Purpose:	Deallocates image.
 *-------------------------------------------------------------------------*/
void 
BMP_destroy (BMP* bmp)
{
	if (!bmp)
		return;
	//----------

	if (bmp->pixels)
		free (bmp->pixels);
	free (bmp);
}

/*---------------------------------------------------------------------------
 * Name:	BMP_point
 * Purpose:	Writes pixel into image.
 *-------------------------------------------------------------------------*/
void
BMP_point (BMP *bmp, int x, int y, RGB rgb)
{
	if (!bmp || x<0 || y<0)
		return;
	if (x >= bmp->width || y >= bmp->height)
		return;
	if (!bmp->pixels)
		return;
	//----------

	bmp->pixels[y*bmp->width + x] = rgb;
}

/*---------------------------------------------------------------------------
 * Name:	BMP_line_core
 * Purpose:	Draws a line in a BMP image.
 *-------------------------------------------------------------------------*/
void
BMP_line_core (BMP *bmp, int x0, int y0, int x1, int y1, RGB rgb,
			int dashed)
{
	if ((rgb >> 24) == 0xff)
		return;

	int dot_counter = 0;

	if (!dashed && x0 == x1 && y0 == y1) 
		BMP_point (bmp, x0, y0, rgb);
	else if (!dashed && x0 == x1)
		BMP_vline (bmp, x0, y0, y1, rgb);
	else if (!dashed && y0 == y1)
		BMP_hline (bmp, x0, x1, y0, rgb);
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
				BMP_point (bmp, x, y, rgb);

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
 * Name:	BMP_line
 * Purpose:	Draws a line in a BMP image.
 *-------------------------------------------------------------------------*/
void
BMP_line (BMP *bmp, int x0, int y0, int x1, int y1, RGB rgb)
{
	BMP_line_core (bmp, x0, y0, x1, y1, rgb, 0);
}

/*---------------------------------------------------------------------------
 * Name:	BMP_line_dashed
 * Purpose:	Draws a dashed line in a BMP image.
 *-------------------------------------------------------------------------*/
void
BMP_line_dashed (BMP *bmp, int x0, int y0, int x1, int y1, RGB rgb)
{
	BMP_line_core (bmp, x0, y0, x1, y1, rgb, 1);
}

/*---------------------------------------------------------------------------
 * Name:	BMP_rect
 * Purpose:	Fills a rectangle with a color.
 *-------------------------------------------------------------------------*/
void
BMP_rect (BMP *bmp, int x, int y, int w, int h, RGB rgb)
{
	BMP_hline (bmp, x, x+w-1, y, rgb);
	BMP_hline (bmp, x, x+w-1, y+h-1, rgb);
	BMP_vline (bmp, x, y, y+h-1, rgb);
	BMP_vline (bmp, x+w-1, y, y+h-1, rgb);
}

/*---------------------------------------------------------------------------
 * Name:	BMP_fillrect
 * Purpose:	Fills a rectangle with a color.
 *-------------------------------------------------------------------------*/
void
BMP_fillrect (BMP *bmp, int x, int y, int w, int h, RGB rgb)
{
	while (h > 0) {
		BMP_hline (bmp, x, x+w-1, y, rgb);
		h--;
		y++;
	}
}

/*---------------------------------------------------------------------------
 * Name:	BMP_clear
 * Purpose:	Sets all pixels to specified color.
 *-------------------------------------------------------------------------*/
void
BMP_clear (BMP *bmp, RGB rgb)
{
	BMP_fillrect (bmp, 0, 0, bmp->width, bmp->height, rgb);
}

/*---------------------------------------------------------------------------
 * Name:	BMP_hline
 * Purpose:	Draws horizontal line.
 *-------------------------------------------------------------------------*/
void
BMP_hline (BMP *bmp, int x0, int x1, int y, RGB rgb)
{
	if (x0 > x1) {
		int tmp=x1;
		x1=x0;
		x0=tmp;
	}
	
	while (x0 <= x1) {
		BMP_point (bmp, x0++, y, rgb);
	}
}

/*---------------------------------------------------------------------------
 * Name:	BMP_vline
 * Purpose:	Draws vertical line.
 *-------------------------------------------------------------------------*/
void
BMP_vline (BMP *bmp, int x, int y0, int y1, RGB rgb)
{
	if (y0 > y1) {
		int tmp=y1;
		y1=y0;
		y0=tmp;
	}
	
	while (y0 <= y1) {
		BMP_point (bmp, x, y0++, rgb);
	}
}

/*---------------------------------------------------------------------------
 * Name:	BMP_draw_string
 * Purpose:	Draws ature 5x8 characters into the image.
 *-------------------------------------------------------------------------*/
int
BMP_draw_string (BMP *bmp, const char *string, int x, int y, RGB color)
{
	char ch;
	const char *s;

	if (!bmp || !string)
		return 0;
	if (x >= bmp->width || y >= bmp->height || !*string)
		return 0;
	//----------

#if 0
	RGB r,g,b;
	RGB light;
	RGB dark;
	r = 0xff & (color >> 16);
	g = 0xff & (color >> 8);
	b = 0xff & color;
	r += 3*0xff;
	b += 3*0xff;
	g += 3*0xff;
	r /= 4;
	g /= 4;
	b /= 4;
	light = b | (g << 8) | (r << 16);

	r = 0xff & (color >> 16);
	g = 0xff & (color >> 8);
	b = 0xff & color;
	r += 0xff;
	b += 0xff;
	g += 0xff;
	r /= 2;
	g /= 2;
	b /= 2;
	dark = b | (g << 8) | (r << 16);
#endif

	const char **chars = get_font_chars ();

	s = string;
	while ((ch = *s++)) {
		int ix = -1;
		if (ch == ' ') {
			x += 10;
			continue;
		}
		if (ch > 'z')
			continue;
		if (ch > ' ' && ch <= 'z')
			ix = FONT_HEIGHT * (ch - 33);
		
		if (ix >= 0) {
			int i;
			int width = 0;

			for (i=0; i<FONT_HEIGHT ; i++) {
				int j=0;
				char ch2;
				const char *s2 = chars[ix + i];
				int width2 = s2 ? strlen (s2) : 0;
				if (width < width2)
					width = width2;
				while ((ch2 = *s2++)) {
					RGB color_to_use;
					char draw = 1;
					switch (ch2) {
					case '#':
						color_to_use = color;
						break;
					default:
						draw = 0;
					}
					if (draw)
						BMP_point (bmp,x+j, y+i, color_to_use);
					j++;
				}
			}

			x += width + 2/* kerning */;
		}
	}

	return x;
}

/*---------------------------------------------------------------------------
 * Name:	BMP_string_width
 * Purpose:	Gets width of 10x16 characters.
 *-------------------------------------------------------------------------*/
int
BMP_string_width (const char *string)
{
	char ch;
	const char *s;
	int width = 0;

	if (!string)
		return 0;
	//----------

	const char **_chars = get_font_chars ();

	s = string;
	while ((ch = *s++)) {
		int ix = -1;
		if (ch == ' ') {
			width += 10;
			continue;
		}
		if (ch > 'z')
			continue;
		if (ch > ' ' && ch <= 'z')
			ix = FONT_HEIGHT * (ch - 33);
		
		if (ix >= 0) {
			int j;
			int max_w = 0;
			for (j = 0; j < FONT_HEIGHT; j++) {
				const char *ptr = _chars [j+ix];
				int w = ptr ? strlen (ptr) : 0;
				if (max_w < w) max_w = w;
			}

			width += max_w + 2/* kerning */;
		}
	}

	return width;
}

/*---------------------------------------------------------------------------
 * Name:	BMP_draw_mini_string
 * Purpose:	Draws miniature 5x8 characters into the image.
 *-------------------------------------------------------------------------*/
int
BMP_draw_mini_string (BMP *bmp, const char *string, int x, int y, RGB color)
{
	char ch;
	const char *s;

	if (!bmp || !string)
		return 0;
	if (x >= bmp->width || y >= bmp->height || !*string)
		return 0;
	//----------

#if 0
	unsigned long r,g,b;
	unsigned long light, dark;
	r = 0xff & (color >> 16);
	g = 0xff & (color >> 8);
	b = 0xff & color;
	r += 3*0xff;
	b += 3*0xff;
	g += 3*0xff;
	r /= 4;
	g /= 4;
	b /= 4;
	light = b | (g << 8) | (r << 16);

	r = 0xff & (color >> 16);
	g = 0xff & (color >> 8);
	b = 0xff & color;
	r += 0xff;
	b += 0xff;
	g += 0xff;
	r /= 2;
	g /= 2;
	b /= 2;
	dark = b | (g << 8) | (r << 16);
#endif

	const char **mini_chars = get_minifont_chars ();

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
				int width2 = s2 ? strlen (s2) : 0;
				if (width < width2)
					width = width2;
				while ((ch2 = *s2++)) {
					RGB color_to_use;
					char draw = 1;
					switch (ch2) {
					case '#':
						color_to_use = color;
						break;
					default:
						draw = 0;
					}
					if (draw)
						BMP_point (bmp,x+j, y+i, color_to_use);
					j++;
				}
			}

			x += width + 1/* kerning */;
		}
	}

	return x;
}

/*---------------------------------------------------------------------------
 * Name:	BMP_mini_string_width
 * Purpose:	Gets width of miniature 5x8 characters.
 *-------------------------------------------------------------------------*/
int
BMP_mini_string_width (const char *string)
{
	char ch;
	const char *s;
	int width = 0;

	if (!string)
		return 0;
	//----------

	const char **mini_chars = get_minifont_chars ();

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
				int w = ptr ? strlen (ptr) : 0;
				if (max_w < w) max_w = w;
			}

			width += max_w + 1/*kerning*/;
		}
	}

	return width;
}

/*---------------------------------------------------------------------------
 * Name:	BMP_narrow_numbers
 * Purpose:	Draws miniature 4x7 characters into the image.
 *-------------------------------------------------------------------------*/
int
BMP_draw_narrow_numbers (BMP *bmp, const char *string, int x, int y, RGB color)
{
	char ch;
	const char *s;

	if (!bmp || !string)
		return 0;
	if (x >= bmp->width || y >= bmp->height || !*string)
		return 0;
	//----------

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
			int width = strlen (narrow_nums [ix]);

			for (i=0; i<NARROW_HEIGHT; i++) {
				int j=0;
				char ch2;
				const char *s2 = narrow_nums [ix + i];
				while ((ch2 = *s2++)) {
					if (ch2 == '#') {
						BMP_point (bmp, 
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

/*---------------------------------------------------------------------------
 * Name:	BMP_getpixel
 * Purpose:	Reads pixel out of image.
 *-------------------------------------------------------------------------*/
RGB
BMP_getpixel (BMP *bmp, int x, int y)
{
	if (!bmp || x<0 || y<0)
		return 0;
	if (x >= bmp->width || y >= bmp->height)
		return 0;
	if (!bmp->pixels)
		return 0;
	//----------

	return bmp->pixels[y*bmp->width + x];
}

/*---------------------------------------------------------------------------
 * Name:	BMP_write
 * Purpose:	Writes image to BMP file.
 *-------------------------------------------------------------------------*/
int 
BMP_write (const BMP* bmp, const char *path)
{
	FILE *f;
#define HDRLEN (54)
	unsigned char h[HDRLEN];
	unsigned long len;
	int i, j;

	if (!bmp || !path)
		return -1;
	//----------

	memset (h, 0, HDRLEN);

	//--------------------
	// Create the file.
	//
	f = fopen (path, "wb");
	if (!f)
		return 0;

	//--------------------
	// Prepare header
	//
	len = HDRLEN + 3 * bmp->width * bmp->height;
	h[0] = 'B';
	h[1] = 'M';
	h[2] = len & 0xff;
	h[3] = (len >> 8) & 0xff;
	h[4] = (len >> 16) & 0xff;
	h[5] = (len >> 24) & 0xff;
	h[10] = HDRLEN;
	h[14] = 40;
	h[18] = bmp->width & 0xff;
	h[19] = (bmp->width >> 8) & 0xff;
	h[20] = (bmp->width >> 16) & 0xff;
	h[22] = bmp->height & 0xff;
	h[23] = (bmp->height >> 8) & 0xff;
	h[24] = (bmp->height >> 16) & 0xff;
	h[26] = 1;
	h[28] = 24;
	h[34] = 16;
	h[36] = 0x13; // 2835 pixels/meter
	h[37] = 0x0b;
	h[42] = 0x13; // 2835 pixels/meter
	h[43] = 0x0b;

	//--------------------
	// Write header.
	//
	if (HDRLEN != fwrite (h, 1, HDRLEN, f)) {
		fclose (f);
		return 0;
	}

	//----------------------------------------
	// Write pixels.
	// Note that BMP has lower rows first.
	//
	for (j=bmp->height-1; j >= 0; j--) {
		for (i=0; i < bmp->width; i++) {
			unsigned char rgb[3];
			int ix = i + j * bmp->width;
			unsigned long pixel = bmp->pixels[ix];
			rgb[0] = pixel & 0xff;
			rgb[1] = (pixel >> 8) & 0xff;
			rgb[2] = (pixel >> 16) & 0xff;
			if (3 != fwrite (rgb, 1, 3, f)) {
				fclose (f);
				return 0;
			}
		}
	}

	fclose (f);
	return 1;
}


