/*============================================================================
  Image, an object-oriented C image manipulation class.
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

#ifndef _OOC_IMAGE_H
#define _OOC_IMAGE_H

#include <stdint.h>

#include "Object.h"
#include "String.h"
#include "GraphicsTypes.h"
#include "Font.h"

#define JPEG_AVAILABLE

#define MINIFONT_HEIGHT (8)

#define DECLARE_IMAGE_METHODS(TYPE_POINTER) \
	RGB (*pixelAt) (TYPE_POINTER, int, int); \
	bool (*writeBMP) (TYPE_POINTER, const char *path); \
	bool (*readTIFF) (TYPE_POINTER, const char *path); \
	bool (*writeTIFF) (TYPE_POINTER, const char *path); \
	Size (*size) (TYPE_POINTER); \
	uint8_t *(*pixels) (TYPE_POINTER); \
	uint8_t *(*pixelRowAddress) (TYPE_POINTER, int y); \
	unsigned (*width) (TYPE_POINTER); \
	unsigned (*height) (TYPE_POINTER); \
	TYPE_POINTER (*shrink) (TYPE_POINTER, unsigned, unsigned);

struct image;

typedef struct imageclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct image*)
	DECLARE_IMAGE_METHODS(struct image*)
} ImageClass;

extern ImageClass *_ImageClass;

#define DECLARE_IMAGE_INSTANCE_VARS(TYPE_POINTER) \
	const char *path; \
	uint16_t width, height; \
	uint16_t pixelFormat; \
	RGB *pixels;

typedef struct image {
	ImageClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct image*)
	DECLARE_IMAGE_INSTANCE_VARS(struct image*)
} Image;

Image *Image_init (Image* self);
void Image_destroy (Any* self);
Image *Image_initWithSize (Image* self, int width, int height);
Image *Image_withSize (int width, int height);
ImageClass *ImageClass_init (ImageClass*);

Image *Image_fromFile (const char *path);

// Inherited by MutableImage
void Image_print (Image *self, FILE*);
RGB Image_pixelAt (Image *self, int x, int y);
bool Image_writeBMP (Image* self, const char *path);
Size Image_size (Image* self);
bool Image_equals (Image *self, void *other);

#include "Image_shrink.h"

#endif

