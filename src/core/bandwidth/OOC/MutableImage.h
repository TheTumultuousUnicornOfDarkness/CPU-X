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

#ifndef _OOC_MUTABLE_IMAGE_H
#define _OOC_MUTABLE_IMAGE_H

#include <stdint.h>

#include "Object.h"
#include "Image.h"
#include "String.h"
#include "GraphicsTypes.h"
#include "Font.h"
#include "RectStack.h"
#include "MutableArray.h"

#define MINIFONT_HEIGHT (8)

#define MutableImageCircleTopLeft 1
#define MutableImageCircleTopRight 2
#define MutableImageCircleBottomLeft 4
#define MutableImageCircleBottomRight 8
#define MutableImageCircleFull 15

typedef enum {
	MutableImageRotationAngle90 = 0,
	MutableImageRotationAngle180 = 1,
	MutableImageRotationAngle270 = 2,
} MutableImageRotationAngle;

#define DECLARE_MUTABLE_IMAGE_METHODS(TYPE_POINTER) \
	int (*drawMiniString) (TYPE_POINTER, const char *, int x, int y, RGB); \
	int (*drawNarrowNumbers) (TYPE_POINTER, const char *string, int x, int y, RGB color); \
	int (*miniStringWidth) (TYPE_POINTER, const char *); \
	int (*drawString) (TYPE_POINTER, String*, int x, int y, Font*, RGB); \
	int (*drawCString) (TYPE_POINTER, const char*, int x, int y, Font*, RGB); \
	int (*drawWideString) (TYPE_POINTER, const wchar_t*, unsigned length, int x, int y, Font*, RGB); \
	void (*clear) (TYPE_POINTER); \
	void (*drawDashedLine) (TYPE_POINTER, int x0, int y0, int x1, int y1, RGB); \
	void (*drawHorizontalLine) (TYPE_POINTER, int x0, int x1, int y, RGB); \
	void (*drawLine) (TYPE_POINTER, int x0, int y0, int x1, int y1, RGB); \
	void (*putPixel) (TYPE_POINTER, int, int, RGB); \
	void (*drawCircle) (TYPE_POINTER, int cx, int cy, unsigned radius, unsigned quadrants, RGB); \
	void (*fillCircle) (TYPE_POINTER, int cx, int cy, unsigned radius, unsigned quadrants, RGB); \
	void (*drawVerticalLine) (TYPE_POINTER, int x, int y0, int y1, RGB); \
	void (*drawRectangle) (TYPE_POINTER, int x, int y, int w, int h, RGB); \
	void (*fillRectangle) (TYPE_POINTER, int x, int y, int w, int h, RGB); \
	void (*drawRoundedRectangle) (TYPE_POINTER, int x, int y, int w, int h, int radius, RGB); \
	void (*fillRoundedRectangle) (TYPE_POINTER, int x, int y, int w, int h, int radius, RGB); \
	void (*drawRect) (TYPE_POINTER, Rect, RGB); \
	void (*fillRect) (TYPE_POINTER, Rect, RGB); \
	void (*fillRectUsingVerticalGradient) (TYPE_POINTER, Rect, RGB top, RGB bottom); \
	void (*fillWithColor) (TYPE_POINTER, RGB); \
	void (*grayscale) (TYPE_POINTER); \
	void (*invert) (TYPE_POINTER); \
	void (*rotate) (TYPE_POINTER, MutableImageRotationAngle); \
	void (*resize) (TYPE_POINTER, unsigned width, unsigned height); \
        void (*startAnimation) (TYPE_POINTER, Any *animator); \
        void (*continueAnimations) (TYPE_POINTER); \
	bool (*pushCrop) (TYPE_POINTER, Rect); \
	void (*popCrop) (TYPE_POINTER); \
	bool (*isModified) (TYPE_POINTER); \
	void (*clearIsModified) (TYPE_POINTER); \
	void (*drawPixelsAt) (TYPE_POINTER, Any*, Point, unsigned width); \
	void (*putImageAt) (TYPE_POINTER, Any*, Point); \
	void (*drawTestPattern) (TYPE_POINTER);

struct mutableimage;

typedef struct mutableimageclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct mutableimage*)
	DECLARE_IMAGE_METHODS(struct mutableimage*)
	DECLARE_MUTABLE_IMAGE_METHODS(struct mutableimage*)
} MutableImageClass;

extern MutableImageClass *_MutableImageClass;
extern MutableImageClass* MutableImageClass_init (MutableImageClass*);

#define DECLARE_MUTABLE_IMAGE_INSTANCE_VARS(TYPE_POINTER) \
	bool isModified; \
	RectStack *croppingStack; \
        MutableArray *animations; \
	struct { int x0, y0, x1, y1; } boundsWhereModified; \
        bool cropping; 

typedef struct mutableimage {
	MutableImageClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct mutableimage*)
	DECLARE_IMAGE_INSTANCE_VARS(struct mutableimage*)
	DECLARE_MUTABLE_IMAGE_INSTANCE_VARS(struct mutableimage*)
} MutableImage;

extern MutableImage* MutableImage_init (MutableImage* self);
extern MutableImage* MutableImage_initWithSize (MutableImage* self, unsigned width, unsigned height);
extern MutableImage *MutableImage_withSize (unsigned width, unsigned height);
extern void MutableImage_destroy (Any *);

#endif
