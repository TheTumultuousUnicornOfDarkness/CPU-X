/*============================================================================
  SimpleGraphing, an object-oriented C class for graphing.
  Copyright (C) 2005-2019, 2021 by Zack T Smith.

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

#ifndef _OOC_SIMPLEGRAPHING_H
#define _OOC_SIMPLEGRAPHING_H

#include <stdint.h>
#include <stdbool.h>

#include "Object.h"
#include "MutableArray.h"
#include "String.h"
#include "GraphicsTypes.h"
#include "Font.h"
#include "MutableImage.h"

#define FONT_HEIGHT (17)
#define MINIFONT_HEIGHT (8)

typedef uint32_t Coordinate;

enum {
	MODE_X_AXIS_LINEAR = 0,
	MODE_X_AXIS_LOG2 = 1,
};

#define SIMPLE_GRAPHING_RELEASE "0.4" // OOC

#define XVALUE_MIN (15)
#define XVALUE_MAX (28)

enum {
	DATUM_X=0,
	DATUM_Y=1,
	DATUM_COLOR=2,
};

#define DECLARE_SIMPLEGRAPHING_METHODS(TYPE_POINTER) \
	void (*clear) (TYPE_POINTER); \
	void (*setTitle) (TYPE_POINTER, Any*); \
	void (*setSubtitle) (TYPE_POINTER, Any*); \
	void (*drawAxes) (TYPE_POINTER); \
	void (*drawLabelsLog2) (TYPE_POINTER); \
	void (*addLine) (TYPE_POINTER, const char *str, RGB color); \
	void (*addPoint) (TYPE_POINTER, long x, long y); \
	void (*plotLog2) (TYPE_POINTER, long x, long y); \
	void (*plotLinear) (TYPE_POINTER, long x, long y, long max_amt); \
	void (*make) (TYPE_POINTER); \
	MutableImage* (*image) (TYPE_POINTER); \
	void (*setXAxisMode) (TYPE_POINTER, int x_axis_mode); 

struct simplegraphing;

typedef struct simplegraphingclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct simplegraphing*)
	DECLARE_SIMPLEGRAPHING_METHODS(struct simplegraphing*)
} SimpleGraphingClass;

extern SimpleGraphingClass *_SimpleGraphingClass;
extern SimpleGraphingClass* SimpleGraphingClass_init (SimpleGraphingClass*);

#define MAX_GRAPH_DATA 50000
#define DASHED 0x1000000 // dashed line flag

#define DECLARE_SIMPLEGRAPHING_INSTANCE_VARS(TYPE_POINTER) \
	MutableImage *image; \
	Font *font; \
	Font *titleFont; \
	Font *subtitleFont; \
	MutableArray *lineNamesArray; \
	MutableArray *lineColorsArray; \
	String *title; \
	String *subtitle; \
	unsigned char x_axis_mode; \
	Coordinate width; \
	Coordinate height; \
	Coordinate left_margin; \
	Coordinate legendMargin; \
	Coordinate margin; \
	Coordinate last_x; \
	Coordinate last_y; \
	Coordinate x_span; \
	Coordinate y_span; \
	RGB fg; \
	MutableArray *data; \
	int data_index; \
	long max_y; \
	long min_y; \
	long min_x; \
	long max_x; 

typedef struct simplegraphing {
	SimpleGraphingClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct simplegraphing*)
	DECLARE_SIMPLEGRAPHING_INSTANCE_VARS(struct simplegraphing*)
} SimpleGraphing;

SimpleGraphing* SimpleGraphing_init (SimpleGraphing*);
SimpleGraphing* SimpleGraphing_initWithSize (SimpleGraphing*, int width, int height);
SimpleGraphing* SimpleGraphing_withSize (int width, int height);
SimpleGraphingClass* SimpleGraphingClass_init (SimpleGraphingClass*);
void SimpleGraphing_destroy (Any *);

#endif
