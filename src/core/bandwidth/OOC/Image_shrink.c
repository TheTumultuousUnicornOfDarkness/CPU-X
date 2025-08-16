/*============================================================================
  Image_shrink, based on image shrinker in fbview (my image viewer).
  Copyright (C) 2004, 2023 by Zack T Smith.

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

/* Changes
 *
 * Sept 26, 2004: Updated for grayscale images.
 * Dec 18, 2004: Updated for TIFFs.
 * Jan 11, 2005: Fixed memory allocation bugs.
 * July 28, 2023: Ported to Object Oriented C.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "Image.h"

typedef struct
{
        int dx, dy;
        int e;
        int j;
        int sum;
}
BresenhamInfo;

static void
bresenham_init (BresenhamInfo *bi, int target_size, int original_size)
{
	if (!bi) 
		return;
	if (!target_size || !original_size)
		return;
	if (target_size > original_size)
		return; // expansion requested

	bi->dx = original_size;
	bi->dy = target_size;
	bi->e = (bi->dy<<1) - bi->dx;
	bi->j = 0;
	bi->sum = 0; // diag
}

static int
bresenham_get (BresenhamInfo *bi)
{
	if (!bi) 
		return 0;

	char done = false;

	int initial_j = bi->j;

	while (bi->j <= bi->dx && !done)
	{
		if (bi->e >= 0)
		{
			done = true;
			bi->e -= (bi->dx<<1);
		}
		bi->e += (bi->dy<<1);
		++bi->j;
	}

	int count = bi->j - initial_j;
	bi->sum += count;
	if (bi->sum > bi->dx)
	{
		// Shouln't reach here.
		return 0;
	}
	return count;
}

Image* Image_shrink (Image *self, unsigned target_w, unsigned target_h)
{
	if (!self) {
		return NULL;
	}

	verifyCorrectClassOrSubclass(self,Image);

	static bool grayscale = false;

	RGB* image_buffer = self->pixels;
	int image_width = self->width; 
	int image_height = self->height;
	int image_ncomponents = 4;

	if (!image_buffer || !image_width || !image_height) {
		return NULL;
	}

	if (target_w > image_width || target_h > image_height) {
		// Only shrinking is supported.
		return NULL;
	}

	RGB *shrunken_image_buffer;

	shrunken_image_buffer = malloc (target_w * target_h * sizeof(RGB));
	if (!shrunken_image_buffer) {
		perror ("malloc");
		return NULL;
	}

	BresenhamInfo b;

	unsigned char bres_x_ary [image_width];
	unsigned char bres_y_ary [image_height];

	bresenham_init (&b, target_w, image_width);
	int n = 0, j, inc = 1;
	for (int i=0; i<image_width && inc > 0; i+=inc) {
		inc = bres_x_ary[n++] = bresenham_get(&b);
	}

	bresenham_init (&b, target_h, image_height);
	n = 0;
	inc = 1;
	for (int i=0; i<image_height && inc > 0; i+=inc) {
		inc = bres_y_ary[n++] = bresenham_get(&b);
	}

	/* shrink */
	int yinc = 0;
	int i, x, y;
	for (y=0, j=0; j<image_height; j+=yinc, y++) 
	{
		yinc = bres_y_ary [y];

		int xinc = 0;

		for(x=0, i=0; i<image_width; i+=xinc, x++) 
		{
			xinc = bres_x_ary [x];

			RGB r = 0, g = 0, b = 0;

			/* collect a pixel average */
			int k,l;
			for (k=0; k < xinc; k++) {
				for (l=0; l<yinc; l++) {
					if (image_ncomponents == 1) {
						uint8_t *p = ((uint8_t*)image_buffer) + (i+k + (j+l)*image_width);
						uint8_t pixel = *p;
						r += pixel;			
						g += pixel;			
						b += pixel;			
					} else {
						RGB *p = image_buffer + (i+k + (j+l)*image_width);
						RGB red = (*p >> 16) & 0xff;
						RGB green = (*p >> 8) & 0xff;
						RGB blue = *p & 0xff;
						/* ignore alpha */
						r += red;
						g += green;
						b += blue;
					}
				}
			}

			RGB area = xinc * yinc;
			r /= area;
			g /= area;
			b /= area;

			unsigned long ix = x + y * target_w;

			if (grayscale) {
				short average = (r + g + b) / 3;
				r = g = b = average;
			}

			shrunken_image_buffer [ix] = (r << 16) | (g << 8) | b;
		} 
	}

	Image *image = new(Image);
	image->width = target_w;
	image->height = target_h;
	image->pixels = shrunken_image_buffer;
	return image;
}

