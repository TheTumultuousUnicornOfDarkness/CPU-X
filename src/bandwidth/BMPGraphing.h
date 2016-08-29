/*============================================================================
  BMPGraphing, a library for graphing.
  Copyright (C) 2005-2014 by Zack T Smith.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  The author may be reached at veritas@comcast.net.
 *===========================================================================*/

#ifndef _BMPGRAPHING_H
#define _BMPGRAPHING_H

#include <stdbool.h>
#include <stdint.h>

#define BMPGRAPHING_RELEASE "0.3"

#define XVALUE_MIN (15)
#define XVALUE_MAX (28)

enum {
	DATUM_X=0,
	DATUM_Y=1,
	DATUM_COLOR=2,
};

typedef uint32_t Coordinate;
typedef uint64_t Value;

enum {
	MODE_X_AXIS_LINEAR = 0,
	MODE_X_AXIS_LOG2 = 1,
};

//---------------
// Graphing data.
//
typedef struct {
	BMP *image;
	char *title;

	unsigned char x_axis_mode;

	Coordinate width;
	Coordinate height;
	Coordinate left_margin;
	Coordinate margin;
	Coordinate last_x;
	Coordinate last_y;
	Coordinate x_span;
	Coordinate y_span;
	Coordinate legend_y;

	RGB fg;
#define MAX_GRAPH_DATA 50000
	Value data [MAX_GRAPH_DATA];
	int data_index;
#define DASHED 0x1000000 // dashed line flag

	Value max_y;
	Value min_y;
	Value min_x;
	Value max_x;
} BMPGraph;

extern void BMPGraphing_set_title (BMPGraph*, const char *);
extern void BMPGraphing_draw_labels_log2 (BMPGraph*);
extern BMPGraph *BMPGraphing_new (int w, int h, int x_axis_mode);
extern void BMPGraphing_new_line (BMPGraph *, const char *str, RGB color);
extern void BMPGraphing_add_point (BMPGraph *, Value x, Value y);
extern void BMPGraphing_plot_log2 (BMPGraph *, Value x, Value y);
extern void BMPGraphing_plot_linear (BMPGraph *, Value x, Value y, Value max_amt);
extern void BMPGraphing_make (BMPGraph*);
extern BMP *BMPGraphing_get_graph (BMPGraph*);
extern void BMPGraphing_destroy (BMPGraph*);

#endif
