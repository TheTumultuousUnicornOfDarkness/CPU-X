/*============================================================================
  Window, an object-oriented C OpenGL window class.
  Copyright (C) 2019, 2022, 2024 by Zack T Smith.

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

#ifndef _OOC_WINDOW_H
#define _OOC_WINDOW_H

#include "Object.h"
#include "Display.h"
#include "MutableArray.h"
#include "GraphicsTypes.h"
#include "Model3D.h"

#define INVALID_WINDOW_ID (-1)
#define INVALID_TEXTURE_ID (0)
#define MAX_LIGHTS (8)

typedef struct {
	const char *text;
	short x, y;
} OverlayedText;

#define DECLARE_WINDOW_INSTANCE_VARS(TYPE_POINTER) \
	Display *display; \
	long identifier; \
	RGBA backgroundColor; \
	unsigned short width, height; \
	short x, y; \
	char *title; \
	bool needsRedraw; \
	bool pointerInside; \
	short pointerX, pointerY; \
	Vector cameraVector; \
	Vector focusVector; \
	Vector upVector; \
	bool useLight[MAX_LIGHTS]; \
	Vector lightPositions[MAX_LIGHTS]; \
	RGBA lightAmbientColors[MAX_LIGHTS]; \
	RGBA lightDiffuseColors[MAX_LIGHTS]; \
	RGBA lightSpecularColors[MAX_LIGHTS]; \
	MutableArray *modelsArray; /* Model3D objects */ \
	MutableArray *viewsArray; /* View objects */ \
	Any *controller; \
	const char *topLeftMessage;

#define DECLARE_WINDOW_METHODS(TYPE_POINTER) \
	long (*identifier) (TYPE_POINTER); \
	void (*setIdentifier) (TYPE_POINTER, long); \
	void (*resize) (TYPE_POINTER, unsigned width, unsigned height); \
	void (*move) (TYPE_POINTER, int x, int y); \
	unsigned (*width) (TYPE_POINTER); \
	unsigned (*height) (TYPE_POINTER); \
	Rect (*redraw) (TYPE_POINTER); \
	/* 3D Model Stuff */ \
	void (*addModel3D) (TYPE_POINTER, Model3D*); \
	void (*moveCamera) (TYPE_POINTER, Vector); \
	void (*setLightEnabled) (TYPE_POINTER, unsigned which, bool); \
	void (*setLightColors) (TYPE_POINTER, unsigned which, RGBA diffuseAmbient, RGBA specular, RGBA emissive); \
	void (*setLightPosition) (TYPE_POINTER, unsigned which, Vector posn); \
	/* View Stuff */ \
	void (*addView) (TYPE_POINTER, Any*); \
	void (*setController) (TYPE_POINTER, Any*); \
	void (*setTopLeftMessage) (TYPE_POINTER, const char *);

struct window;

typedef struct windowclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct window*)
        DECLARE_WINDOW_METHODS(struct window*)
} WindowClass;

extern WindowClass *_WindowClass;
extern WindowClass* WindowClass_init (WindowClass*);

typedef struct window {
        WindowClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct window*)
	DECLARE_WINDOW_INSTANCE_VARS(struct window*)
} Window;

extern Window *Window_initWith (Window*, Display *, const char*, unsigned width, unsigned height, int x, int y);
extern Window *Window_newWith (Display *, const char*, unsigned width, unsigned height, int x, int y);
extern Window *Window_init (Window *self);
extern void Window_destroy (Any *);
extern void Window_describe (Window*, FILE*);
extern void Window_print (Window*, FILE*);

#endif
