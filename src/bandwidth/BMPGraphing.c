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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "BMP.h"
#include "BMPGraphing.h"

//----------------------------------------------------------------------------
// Name:	BMPGraphing_draw_labels_log2
// Purpose:	Draw the labels and ticks.
//----------------------------------------------------------------------------
void
BMPGraphing_draw_labels_log2 (BMPGraph* graph)
{
	if (!graph || !graph->image)
		return;

	//----------------------------------------
	// Horizontal
	//
	// Establish min & max x values.
	//
	int i = 0;
	Value min_x = 0x4000000000000000;
	Value max_x = 0;
	for (i = 0; i < graph->data_index; i += 2) {
		Value type = graph->data[i];
		Value value = graph->data[i+1];
		if (type == DATUM_X) {
			if (value < min_x)
				min_x = value;
			if (value > max_x)
				max_x = value;
		}
	}
	graph->min_x = (long long) log2 (min_x);
	graph->max_x = (long long) ceil (log2 (max_x));

	for (i = graph->min_x; i <= graph->max_x; i++) {
		char str [200];
		int x = graph->left_margin + 
			((i-graph->min_x) * graph->x_span) / 
			(graph->max_x - graph->min_x);
		int y = graph->height - graph->margin + 10;

		unsigned long y2 = 1 << i;
		if (y2 < 1536) 
			snprintf (str, 199, "%ld B", y2);
		else if (y2 < (1<<20)) {
			snprintf (str, 199, "%ld kB", y2 >> 10);
		}
		else {
			Value j = y2 >> 20;
			switch ((y2 >> 18) & 3) {
			case 0: snprintf (str, 199, "%lld MB", j); break;
			case 1: snprintf (str, 199, "%lld.25 MB", j); break;
			case 2: snprintf (str, 199, "%lld.5 MB", j); break;
			case 3: snprintf (str, 199, "%lld.75 MB", j); break;
			}
		}

		BMP_vline (graph->image, x, y, y - 10, RGB_BLACK);
		BMP_draw_mini_string (graph->image, str, x - 10, y + 8, RGB_BLACK);
	}

	//----------------------------------------
	// Vertical
	//
	// Establish min & max y values.
	//
	Value min_y = 0x4000000000000000;
	Value max_y = 0;
	for (i = 0; i < graph->data_index; i += 2) {
		Value type = graph->data[i];
		Value value = graph->data[i+1];
		if (type == DATUM_Y) {
			if (value < min_y)
				min_y = value;
			if (value > max_y)
				max_y = value;
		}
	}
	graph->min_y = min_y;
	graph->max_y = max_y;

	int font_height = 10;
	int available_height = graph->y_span;
	int max_labels = available_height / font_height;
	int preferred_n_labels = graph->max_y/10000;
	int actual_n_labels;
	float multiplier = 1;
	if (preferred_n_labels < max_labels) {
		actual_n_labels = preferred_n_labels;
	} else {
		actual_n_labels = max_labels;
		multiplier = preferred_n_labels / (float) actual_n_labels;
	}

	for (i = 0; i <= actual_n_labels; i++) {
		char str [200];
		int x = graph->left_margin - 10;
		int y = graph->height - graph->margin - (i * graph->y_span) / (float)actual_n_labels;

		BMP_hline (graph->image, x, x+10, y, RGB_BLACK);

		int value = (int) (i * multiplier);
		snprintf (str, 199, "%d GB/s", value);
		BMP_draw_mini_string (graph->image, str, x - 40, y - MINIFONT_HEIGHT/2, RGB_BLACK);
	}
}

BMPGraph *
BMPGraphing_new (int w, int h, int x_axis_mode)
{
	if (x_axis_mode != MODE_X_AXIS_LINEAR && x_axis_mode != MODE_X_AXIS_LOG2)
		return NULL;

	BMPGraph *graph = (BMPGraph*) malloc (sizeof(BMPGraph));
	if (!graph)
		return NULL;

	bzero (graph, sizeof(BMPGraph));

	graph->x_axis_mode = x_axis_mode;

	if (w <= 0 || h <= 0) {
		w = 1920;
		h = 1080;
	}

	graph->width = w;
	graph->height = h;
	graph->image = BMP_new (w, h);
	graph->margin = 40;
	graph->left_margin = 80;

	BMP_clear (graph->image, RGB_WHITE);

	BMP_hline (graph->image, graph->left_margin, graph->width - graph->margin, graph->height - graph->margin, RGB_BLACK);
	BMP_vline (graph->image, graph->left_margin, graph->margin, graph->height - graph->margin, RGB_BLACK);

	graph->x_span = graph->width - (graph->margin + graph->left_margin);
	graph->y_span = graph->height - 2 * graph->margin;

	graph->legend_y = graph->margin;

	return graph;
}

void BMPGraphing_set_title (BMPGraph* graph, const char *title)
{
	if (!graph || !title)
		return;

	if (graph->title)
		free (graph->title);
	graph->title = strdup (title);

	BMP_draw_string (graph->image, graph->title, graph->left_margin, graph->margin/2, RGB_BLACK);
}

void
BMPGraphing_new_line (BMPGraph *graph, char *str, RGB color)
{
	if (!graph || !graph->image)
		return;

	BMP_draw_string (graph->image, str, graph->width - graph->margin - 320, graph->legend_y, 0xffffff & color);
	
	graph->legend_y += 17;

	graph->fg = 0;
	graph->last_x = graph->last_y = -1;

	if (graph->data_index >= MAX_GRAPH_DATA-2) 
		return; // error ("Too many graph data.");

	graph->data [graph->data_index++] = DATUM_COLOR;
	graph->data [graph->data_index++] = color;
}

//----------------------------------------------------------------------------
// Name:	BMPGraphing_add_point
// Purpose:	Adds a point to this list to be drawn.
//----------------------------------------------------------------------------
void
BMPGraphing_add_point (BMPGraph *graph, Value x, Value y)
{
	if (!graph || !graph->image)
		return;

	if (graph->data_index >= MAX_GRAPH_DATA-4) 
		return; // error ("Too many graph data.");

	graph->data [graph->data_index++] = DATUM_X;
	graph->data [graph->data_index++] = x;
	graph->data [graph->data_index++] = DATUM_Y;
	graph->data [graph->data_index++] = y;
}

//----------------------------------------------------------------------------
// Name:	BMPGraphing_plot_log2
// Purpose:	Plots a point on the current graph.
//----------------------------------------------------------------------------

void
BMPGraphing_plot_log2 (BMPGraph *graph, Value x, Value y)
{
	if (!graph || !graph->image)
		return;

	int i = 0;

	//----------------------------------------
	// Plot the point. The x axis is 
	// logarithmic, base 2.
	//
	double tmp = log2 (x);
	tmp -= (double) graph->min_x;
	tmp *= (double) graph->x_span;
	tmp /= (double) (graph->max_x - graph->min_x);

	int x2 = graph->left_margin + (int) tmp;
	int y2 = graph->height - graph->margin - (y * graph->y_span) / graph->max_y;

	if (graph->last_x != -1 && graph->last_y != -1) {
		if (graph->fg & DASHED) 
			BMP_line_dashed (graph->image, graph->last_x, graph->last_y, x2, y2, graph->fg & 0xffffff);
		else
			BMP_line (graph->image, graph->last_x, graph->last_y, x2, y2, graph->fg);
	}

	graph->last_x = x2;
	graph->last_y = y2;
}

//----------------------------------------------------------------------------
// Name:	BMPGraphing_plot_linear
// Purpose:	Plots a point on the current graph.
//----------------------------------------------------------------------------

void
BMPGraphing_plot_linear (BMPGraph *graph, Value x, Value y, Value max_y)
{
	if (!graph || !graph->image)
		return;

	//----------------------------------------
	// Plot the point. The x axis is 
	// logarithmic, base 2. The units of the
	// y value is kB.
	//
	double tmp = 10. + log2 (x);
	tmp -= (double) XVALUE_MIN;
	tmp *= (double) graph->x_span;
	tmp /= (double) (XVALUE_MAX - XVALUE_MIN);
	int x2 = graph->left_margin + (int) tmp;
	int y2 = graph->height - graph->margin - (y * graph->y_span) / max_y;

//printf ("\tx=%d, y=%d\n",x,y); fflush(stdout);

	if (graph->last_x != -1 && graph->last_y != -1) {
		if (graph->fg & DASHED) 
			BMP_line_dashed (graph->image, graph->last_x, graph->last_y, x2, y2, graph->fg & 0xffffff);
		else
			BMP_line (graph->image, graph->last_x, graph->last_y, x2, y2, graph->fg);
	}

	graph->last_x = x2;
	graph->last_y = y2;
}

//----------------------------------------------------------------------------
// Name:	BMPGraphing_make_log2
// Purpose:	Plots all lines.
//----------------------------------------------------------------------------

static void
BMPGraphing_make_log2 (BMPGraph *graph)
{
	if (!graph || !graph->image)
		return;

	BMPGraphing_draw_labels_log2 (graph);

	//----------------------------------------
	// OK, now draw the lines.
	//
	int i;
	int x = -1, y = -1;
	for (i = 0; i < graph->data_index; i += 2) 
	{
		Value type = graph->data[i];
		Value value = graph->data[i+1];

		switch (type) {
		case DATUM_Y:	y = value; break;
		case DATUM_X:	x = value; break;
		case DATUM_COLOR:	
			graph->fg = (unsigned long) value; 
			graph->last_x = -1;
			graph->last_y = -1;
			break;
		}

		if (x != -1 && y != -1) {
			BMPGraphing_plot_log2 (graph, x, y);
			x = y = -1;
		}
	}
}

//----------------------------------------------------------------------------
// Name:	BMPGraphing_make_linear
// Purpose:	Plots all lines for the network test graph.
//----------------------------------------------------------------------------

static void
BMPGraphing_make_linear (BMPGraph *graph)
{
	if (!graph || !graph->image)
		return;

	int i;

	// No data
	if (!graph->data_index)
		return;

	//----------------------------------------
	// Get the maximum bandwidth in order to
	// properly scale the graph vertically.
	//
	int max_y = 0;
	for (i = 0; i < graph->data_index; i += 2) {
		if (graph->data[i] == DATUM_Y) {
			int y = graph->data [i+1];
			if (y > max_y)
				max_y = y;
		}
	}

	int range = max_y > 10000 ? 2 : (max_y > 1000 ? 1 : 0);
	int y_spacing = 1;
	switch (range) {
	case 2:
		// Round up to the next 100.00 MB/sec. (=10000).
		y_spacing = 10000;
		break;
	case 1:
		// Round up to the next 10.00 MB/sec. 
		y_spacing = 1000;
		break;
	case 0:
		// Round up to the next 1.00 MB/sec. 
		y_spacing = 100;
		break;
	} 
	max_y /= y_spacing;
	max_y *= y_spacing;
	max_y += y_spacing;

	//----------------------------------------
	// Draw the axes, ticks & labels.
	//
	// X axis:
	if (XVALUE_MIN < 10)
		return; // error ("Minimum y is too small.");

	for (i = XVALUE_MIN; i <= XVALUE_MAX; i++) {
		char str[200];
		unsigned long y2 = 1 << (i-10); // XX XVALUE_MIN>=10
		if (y2 < 1024)
			snprintf (str, 199, "%u kB", (unsigned int) y2);
		else
			snprintf (str, 199, "%lu MB", (unsigned long) (y2 >> 10));

		int x = graph->left_margin + ((i - XVALUE_MIN) * graph->x_span) / (XVALUE_MAX - XVALUE_MIN);
		int y = graph->height - graph->margin + 10;
		
		BMP_vline (graph->image, x, y, y-10, RGB_BLACK);
		BMP_draw_mini_string (graph->image, str, x - 10, y+8, RGB_BLACK);
	}

	//----------
	// Y axis:
	// Decide what the tick spacing will be.
	for (i = 0; i <= max_y; i += y_spacing) {
		char str[200];
		unsigned long whole = i / 100;
		unsigned long frac = i % 100;
		snprintf (str, 199, "%lu.%02lu MB/s", whole, frac);

		int x = graph->left_margin - 10;
		int y = graph->height - graph->margin - (i * graph->y_span) / max_y;

		BMP_hline (graph->image, x, x+10, y, RGB_BLACK);
		BMP_draw_mini_string (graph->image, str, x - 60, y - MINIFONT_HEIGHT/2, RGB_BLACK);
	}

	//----------------------------------------
	// Draw the data lines.
	//
	int x = -1, y = -1;
	graph->last_x = -1;
	graph->last_y = -1;
	for (i = 0; i < graph->data_index; i += 2) 
	{
		int type = graph->data[i];
		long value = graph->data[i+1];

		switch (type) {
		case DATUM_Y:	y = value; break;
		case DATUM_X:	x = value; break;
		case DATUM_COLOR:	
			graph->fg = (unsigned long) value; 
			graph->last_x = -1;
			graph->last_y = -1;
			break;
		}

		if (x != -1 && y != -1) {
			BMPGraphing_plot_linear (graph, x, y, max_y);
			x = y = -1;
		}
	}
}

void
BMPGraphing_make (BMPGraph *graph)
{
	if (!graph)
		return; // XX silent error

	switch (graph->x_axis_mode) {
	case MODE_X_AXIS_LOG2:
		BMPGraphing_make_log2 (graph);
		break;
	case MODE_X_AXIS_LINEAR:
		BMPGraphing_make_linear (graph);
		break;
	default:
		fprintf (stderr, "Invalid graph mode %d.\n", graph->x_axis_mode);
		break;
	}
}

void
BMPGraphing_destroy (BMPGraph *graph)
{
	if (!graph)
		return;

	if (graph->title) {
		free (graph->title);
		graph->title = NULL;
	}
	if (graph->image) {
		BMP_destroy (graph->image);
		graph->image = NULL;
	}

	free (graph);
}
