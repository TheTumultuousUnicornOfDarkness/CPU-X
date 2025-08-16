/*============================================================================
  View, an object-oriented C view class.
  Copyright (C) 2019, 2023, 2024 by Zack T Smith.

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

#include "Window.h"
#include "View.h"

#include <stdlib.h>

#ifdef HAVE_OPENGL
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif

static unsigned createTexture ()
{
#ifdef HAVE_OPENGL
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	return textureID;
#else
	return INVALID_TEXTURE_ID;
#endif
}

ViewClass *_ViewClass = NULL;

void View_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_) {
		return;
	}
	verifyCorrectClassOrSubclass(self_,View);

	View *self = self_;

	if (self->textureID != INVALID_TEXTURE_ID) {
#ifdef HAVE_OPENGL
		GLuint oldTextureID = self->textureID;
		self->textureID = INVALID_TEXTURE_ID;
		glDeleteTextures (1, (const GLuint*) &oldTextureID);
		CHECK_GL_ERROR("glDeleteTextures");
#endif
	}

	if (self->image) {
		MutableImage_destroy (self->image);
		self->image = NULL;
	}

	Object_destroy ((Object*) self);
}

static long View_message (View* self, long message, Any *sender, long first, long second)
{
	if (!self) {
		return -1;
	}
	verifyCorrectClassOrSubclass(self,View);

	switch (message) {
		case kClickedMessage: {
			Log_debug_printf (__FUNCTION__, "Pointer click in View");
			break;
	 	}
		default:
			break;
	}

	return 0;
}

static void View_print (View* self, FILE *outputFile)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);

	if (!outputFile) {
		outputFile = stdout;
	}
}

static void View_describe (View* self, FILE *outputFile)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%s", $(self, className));
}

static void View_drawBorder (View *self)
{
	if (!self
	  || (self->borderColor >> 24) == 0xff
	  || self->borderWidth <= 0
	  || self->borderType == ViewBorderTypeNone) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	MutableImage *drawable = self->image;
	if (!drawable) {
		return;
	}

	int width = self->rect.size.width;
	int height = self->rect.size.height;
	int x1 = width - 1;
	int y1 = height - 1;

	ViewBorderType borderType = self->borderType;
	if (borderType == ViewBorderTypeNone && self->borderWidth > 0 && self->borderColor != RGB_CLEAR) {
		borderType = ViewBorderTypeSolid;
	}

	switch (borderType) {
	case ViewBorderTypeNone:
		break;

	case ViewBorderTypeRounded:
		if (self->cornerRadius > 0) {
			$(drawable, drawRoundedRectangle, 0, 0, width, height, self->cornerRadius, self->borderColor);
			break;
		}
		// If cornerRadius is 0, just draw a rectangle below.
		/* FALLTHRU */

	default:
	case ViewBorderTypeSolid: {
		for (int i=0; i < self->borderWidth; i++) {
			int y = i;
			$(drawable, drawLine, i, y, x1-i, y, self->borderColor);
			y = y1-i;
			$(drawable, drawLine, i, y, x1-i, y, self->borderColor);

			int x = i;
			$(drawable, drawLine, x, i, x, y1-i, self->borderColor);
			x = x1-i;
			$(drawable, drawLine, x, 1+i, x, y1-i-1, self->borderColor);
		}
	 } break;

	case ViewBorderType3DOut:
	case ViewBorderType3DIn: {
		// RULE: 3D borders are based on the upper gradient color.

		RGB color = self->borderColor;
		RGB dark = RGBChangeIntensity (color, 0.15);
		RGB light = RGBChangeIntensity (color, 0.4);
		RGB light2 = RGBChangeIntensity (color, 0.3);
		RGB medium = RGBChangeIntensity (color, 0.28);

		bool outward = (self->borderType == ViewBorderType3DOut);

		for (int i=0; i < self->borderWidth; i++) {
			// Left
			int x = i;
			$(drawable, drawLine, x, i, x, y1-i, medium);

			// Top
			int y = i;
			$(drawable, drawLine, i, y, x1-i, y, outward ? light : dark);

			// Right
			x = x1-i;
			$(drawable, drawLine, x, 1+i, x, y1-i-1, outward ? light2 : dark);

			// Bottom
			y = y1-i;
			$(drawable, drawLine, i, y, x1-i, y, outward ? dark : light);
		}
	 } break;
	}
}

RGBA View_backgroundColor (View *self)
{
	if (!self) {
		return RGB_CLEAR;
	}
	verifyCorrectClassOrSubclass(self,View);
	return self->backgroundColor;
}

void View_setBackgroundColor (View *self, RGBA color)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	self->backgroundColor = color;
	self->needsRedraw = true;
}

RGBA View_foregroundColor (View *self)
{
	if (!self) {
		return RGB_CLEAR;
	}
	verifyCorrectClassOrSubclass(self,View);
	return self->foregroundColor;
}

void View_setForegroundColor (View *self, RGBA color)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	self->foregroundColor = color;
	self->needsRedraw = true;
}

bool View_hidden (View *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,View);
	return self->hidden;
}

void View_setHidden (View *self, bool value)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	self->hidden = value;
	self->needsRedraw = true;
}

void View_defineBorder (View *self, ViewBorderType type, RGB color, int width, int radius)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	if (type < ViewBorderTypeNone || type > ViewBorderType_last) {
		return;
	}
	self->borderType = type;
	self->borderColor = color;
	self->borderWidth = width;
	self->cornerRadius = radius;
	self->needsRedraw = true;
}

void View_clear (View *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	if (self->image) {
		$(self->image, fillWithColor, self->backgroundColor);
	}
}

static void View_setRect (View *self, Rect rect)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);

	if (self->image) {
		release(self->image);
		self->image = NULL;
	}

	unsigned width = rect.size.width;
	unsigned height = rect.size.height;
	if (width > 0 && height > 0) {
		self->rect = rect;
		self->image = retain(MutableImage_withSize (width, height));
		View_clear (self);
	}
	self->needsRedraw = true;
}

static void View_redraw (View *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	self->needsRedraw = false;
}

static void View_setNeedsRedraw (View *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,View);
	self->needsRedraw = true;
}

ViewClass* ViewClass_init (ViewClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(View,describe);
	SET_OVERRIDDEN_METHOD_POINTER(View,print);
	SET_OVERRIDDEN_METHOD_POINTER(View,message);

	SET_METHOD_POINTER(View,clear);
	SET_METHOD_POINTER(View,defineBorder);
	SET_METHOD_POINTER(View,drawBorder);
	SET_METHOD_POINTER(View,backgroundColor);
	SET_METHOD_POINTER(View,setBackgroundColor);
	SET_METHOD_POINTER(View,foregroundColor);
	SET_METHOD_POINTER(View,setForegroundColor);
	SET_METHOD_POINTER(View,hidden);
	SET_METHOD_POINTER(View,setHidden);
	SET_METHOD_POINTER(View,setNeedsRedraw);
	SET_METHOD_POINTER(View,redraw);
	SET_METHOD_POINTER(View,setRect);

        VALIDATE_CLASS_STRUCT(class);
	return class;
}

View *View_with (Rect rect, RGB backgroundColor)
{
	View *self = new(View);
	self->backgroundColor = backgroundColor;
	View_setRect (self, rect);
	return self;
}

View* View_init (View *self)
{
	ENSURE_CLASS_READY(View);

	if (self) {
		Object_init ((Object*) self);
		self->is_a = _ViewClass;

		self->image = NULL;
		self->rect = Rect_zero();
		self->textureID = createTexture();
		self->backgroundColor = RGB_WHITE;
		self->foregroundColor = RGB_BLACK;
		self->needsRedraw = true;
	}

	return self;
}
