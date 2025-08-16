/*============================================================================
  Display, an object-oriented C OpenGL display class.
  Copyright (C) 2019, 2024 by Zack T Smith.

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

// This code is derived from my FrugalWidgets project.

#ifndef _OOC_DISPLAY_H
#define _OOC_DISPLAY_H

#include "Object.h"
#include "GraphicsTypes.h"
#include "Log.h"

#define kDisplayShiftFlag	(1)
#define kDisplayAltFlag 	(2)
#define kDisplayControlFlag 	(4)

#define DECLARE_DISPLAY_METHODS(TYPE_POINTER) \
	unsigned (*width) (TYPE_POINTER); \
	unsigned (*height) (TYPE_POINTER); \
	const char *(*name) (TYPE_POINTER); \
	unsigned (*flags) (TYPE_POINTER); \
	void (*setFlags) (TYPE_POINTER, unsigned); \
	void (*addWindow) (TYPE_POINTER, Any*); \
	bool (*canAddWindows) (TYPE_POINTER); \
	Any* (*lookupWindowByID) (TYPE_POINTER, long); \
	unsigned (*brightness) (TYPE_POINTER); \
	void (*mainLoop) (TYPE_POINTER); 

struct display;

typedef struct displayclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct display*)
	DECLARE_DISPLAY_METHODS(struct display*)
} DisplayClass;

extern DisplayClass *_DisplayClass;
extern DisplayClass* DisplayClass_init (DisplayClass*);

#define MAX_DISPLAYS (4)
#define MAX_WINDOWS_PER_DISPLAY (32)

#define DECLARE_DISPLAY_INSTANCE_VARS(TYPE_POINTER) \
	unsigned short width, height; \
	unsigned char depth; \
	unsigned char flags; \
	unsigned char nWindows; \
	Any *windows[MAX_WINDOWS_PER_DISPLAY]; 

typedef struct display {
	DisplayClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct display*)
	DECLARE_DISPLAY_INSTANCE_VARS(struct display*)
} Display;

extern Display* Display_init (Display*);
extern void Display_destroy (Any *);

#define CHECK_GL_ERROR(NAME) {\
        GLint err = glGetError(); \
        if (err) { \
                char temp [32]; \
                snprintf (temp, sizeof(temp)-1, "GL error %d", err); \
                Log_error (NAME, temp); \
        }}

#endif

