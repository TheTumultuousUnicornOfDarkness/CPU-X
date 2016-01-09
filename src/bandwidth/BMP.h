
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

#ifndef _BMP_H
#define _BMP_H

#include <stdint.h>

#define BMPLIB_RELEASE "0.9"
#define BMPLIB_RELEASE_MAJOR 0
#define BMPLIB_RELEASE_MINOR 9

typedef uint32_t RGB;
typedef uint32_t RGBA;

typedef struct {
	int width, height;
	RGB *pixels;
} BMP;

#define FONT_HEIGHT (17)
#define MINIFONT_HEIGHT (8)

extern BMP* BMP_new (int, int);
extern void BMP_destroy (BMP*);
extern void BMP_clear (BMP*, RGB);
extern int BMP_write (const BMP*, const char *path);
extern void BMP_point (BMP*, int, int, RGB);
extern void BMP_line (BMP *, int x0, int y0, int x1, int y1, RGB);
extern void BMP_line_dashed (BMP *, int x0, int y0, int x1, int y1, RGB);
extern void BMP_hline (BMP *, int x0, int x1, int y, RGB);
extern void BMP_vline (BMP *, int x, int y0, int y1, RGB);
extern void BMP_rect (BMP *, int x, int y, int w, int h, RGB);
extern void BMP_fillrect (BMP *, int x, int y, int w, int h, RGB);
extern RGB BMP_getpixel (BMP*, int, int);

extern int BMP_draw_string (BMP *, const char *, int x, int y, RGB);
extern int BMP_string_width (const char *);

extern int BMP_draw_mini_string (BMP *, const char *, int x, int y, RGB);
extern int BMP_mini_string_width (const char *);

#define RGB_BLACK (0)
#define RGB_BLUE (0xff)
#define RGB_BRASS (0xc3a368)
#define RGB_BROWN (0x8b4513)
#define RGB_CADETBLUE (0x5f9ea0)
#define RGB_CHARTREUSE (0x7fff00)
#define RGB_CORAL (0xff7f50)
#define RGB_CYAN (0xffff)
#define RGB_DARKGREEN (0x6400)
#define RGB_DARKKHAKI (0xbdb76b)
#define RGB_DARKOLIVEGREEN (0x556b2f)
#define RGB_DARKORANGE (0xff8c00)
#define RGB_DODGERBLUE (0x1e90ff)
#define RGB_GOLDENROD (0xdaa520)
#define RGB_GRAY (0xc0c0c0)
#define RGB_GREEN (0xff00)
#define RGB_KHAKI (0xf0e68c)
#define RGB_LEMONYELLOW (0xfde910)
#define RGB_MAGENTA (0xff00ff)
#define RGB_MAROON (0x800000)
#define RGB_NAVYBLUE (0x80)
#define RGB_ORANGE (0xffa500)
#define RGB_PINK (0xf77fbe)
#define RGB_PURPLE (0xa020f0)
#define RGB_RED (0xff0000)
#define RGB_ROYALBLUE (0x4169e1)
#define RGB_SALMON (0xfa8072)
#define RGB_TURQUOISE (0x40e0d0)
#define RGB_VIOLET (0xee82ee)
#define RGB_WHITE (0xffffff)
#define RGB_YELLOW (0xffff00)

#define RGB_GRAY6 (0x606060)
#define RGB_GRAY8 (0x808080)
#define RGB_GRAY10 (0xa0a0a0)
#define RGB_GRAY12 (0xc0c0c0)
#define RGB_GRAY14 (0xe0e0e0)

#endif

