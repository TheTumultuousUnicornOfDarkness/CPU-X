/*============================================================================
  View, an object-oriented C view class.
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

#ifndef _OOC_VIEW_H
#define _OOC_VIEW_H

#include "Object.h"
#include "Display.h"
#include "GraphicsTypes.h"
#include "MutableImage.h"

typedef enum {
        ViewBorderTypeNone = 0,
        ViewBorderTypeSolid = 1,
        ViewBorderType3DIn = 2,
        ViewBorderType3DOut = 3,
        ViewBorderTypeRounded = 4,
} ViewBorderType;
#define ViewBorderType_last ViewBorderTypeRounded 

#define DECLARE_VIEW_INSTANCE_VARS(FOO) \
	unsigned textureID; \
	Rect rect; \
	bool hidden; \
	bool needsRedraw; \
	RGBA backgroundColor; \
	RGBA foregroundColor; \
	MutableImage *image; \
        int borderWidth; \
        int cornerRadius; \
        RGB borderColor; \
        ViewBorderType borderType; \

#define DECLARE_VIEW_METHODS(TYPE_POINTER) \
	void (*clear) (TYPE_POINTER); \
	void (*defineBorder) (TYPE_POINTER, ViewBorderType, RGB, int width, int radius); \
	void (*drawBorder) (TYPE_POINTER); \
	RGBA (*backgroundColor) (TYPE_POINTER); \
	void (*setBackgroundColor) (TYPE_POINTER, RGBA color); \
	RGBA (*foregroundColor) (TYPE_POINTER); \
	void (*setForegroundColor) (TYPE_POINTER, RGBA color); \
	bool (*hidden) (TYPE_POINTER); \
	void (*setHidden) (TYPE_POINTER, bool); \
	void (*setNeedsRedraw) (TYPE_POINTER); \
	void (*redraw) (TYPE_POINTER); \
	void (*setRect) (TYPE_POINTER, Rect); 

struct view;

typedef struct viewclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct view*)
        DECLARE_VIEW_METHODS(struct view*)
} ViewClass;

extern ViewClass *_ViewClass;
extern ViewClass* ViewClass_init (ViewClass*);

typedef struct view {
        ViewClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct view*)
	DECLARE_VIEW_INSTANCE_VARS(struct view*)
} View;

extern void View_destroy (Any *);
extern View *View_init (View *self); 
extern View *View_with (Rect, RGBA);

#endif
