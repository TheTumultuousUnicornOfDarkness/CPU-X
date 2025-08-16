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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "colors.h"
#include "SimpleGraphing.h"
#include "FontPK.h"
#include "FontFreetype.h"
#include "Int.h"

#define kDefaultMargin 40
#define kDefaultLeftMargin 80
#define kDefaultFontHeight (10)
#define kMaxTitleLength (200)
#define kMaxTickTextLength (40)
#define kDefaultWidth 1920
#define kDefaultHeight 1080

SimpleGraphingClass *_SimpleGraphingClass = NULL;

//----------------------------------------------------------------------------
// Name:	SimpleGraphing_drawAxes 
// Purpose:	Draw the horizontal and vertical axes.
//----------------------------------------------------------------------------
void
SimpleGraphing_drawAxes (SimpleGraphing* self)
{
	$(self->image, drawHorizontalLine, self->left_margin, self->width - self->margin, self->height - self->margin, RGB_BLACK);
	$(self->image, drawVerticalLine, self->left_margin, self->height - self->margin - self->y_span, self->height - self->margin, RGB_BLACK);
}

//----------------------------------------------------------------------------
// Name:	SimpleGraphing_drawLabelsLog2
// Purpose:	Draw the labels and ticks.
//----------------------------------------------------------------------------
void
SimpleGraphing_drawLabelsLog2 (SimpleGraphing* self)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image)
		return;

	$(self, drawAxes);

	//----------------------------------------
	// Horizontal
	//
	// Establish min & max x values.
	//
	int i = 0;
	long min_x = LONG_MAX;
	long max_x = 0;

	int count = $(self->data, count);
	for (i = 0; i < count; i += 2) {
		Int *typeObj = $(self->data, at, i); 
		Int *valueObj = $(self->data, at, i+1); 
		long type = $(typeObj,asLongLong);
		long long value = $(valueObj,asLongLong);

		if (type == DATUM_X) {
			if (value < min_x)
				min_x = value;
			if (value > max_x)
				max_x = value;
		}
	}

	self->min_x = (long long) log2 (min_x);
	self->max_x = (long long) ceil (log2 (max_x));

	for (i = self->min_x; i <= self->max_x; i++) {
		char str [kMaxTitleLength];
		int x = self->left_margin + ((i-self->min_x) * self->x_span) / (self->max_x - self->min_x);
		int y = self->height - self->margin + 10;

		unsigned long y2 = 1 << i;
		if (y2 < 1536) 
			snprintf (str, sizeof(str)-1, "%ld B", y2);
		else if (y2 < (1<<20)) {
			snprintf (str, sizeof(str)-1, "%ld kB", y2 >> 10);
		}
		else {
			unsigned long j = y2 >> 20;
			switch ((y2 >> 18) & 3) {
			case 0: snprintf (str, sizeof(str)-1, "%lld MB", (unsigned long long) j); break;
			case 1: snprintf (str, sizeof(str)-1, "%lld.25 MB", (unsigned long long) j); break;
			case 2: snprintf (str, sizeof(str)-1, "%lld.5 MB", (unsigned long long) j); break;
			case 3: snprintf (str, sizeof(str)-1, "%lld.75 MB", (unsigned long long) j); break;
			}
		}

		$(self->image, drawVerticalLine, x, y, y - 10, RGB_BLACK);
		$(self->image, drawMiniString, str, x - 10, y + 8, RGB_BLACK);
	}

	//----------------------------------------
	// Vertical
	//
	// Establish min & max y values.
	//
	long min_y = LONG_MAX;
	long max_y = 0;
	for (i = 0; i < count; i += 2) {
		Int *typeObj = $(self->data, at, i); 
		Int *valueObj = $(self->data, at, i+1); 
		long type = $(typeObj,asLongLong);
		long long value = $(valueObj,asLongLong);
		
		if (type == DATUM_Y) {
			if (value < min_y)
				min_y = value;
			if (value > max_y)
				max_y = value;
		}
	}
	self->min_y = min_y;
	self->max_y = max_y;

	int font_height = kDefaultFontHeight;
	int available_height = self->y_span;
	int max_labels = available_height / font_height;
	int preferred_n_labels = self->max_y/10000;
	int actual_n_labels;
	float multiplier = 1;
	if (preferred_n_labels < max_labels) {
		actual_n_labels = preferred_n_labels;
	} else {
		actual_n_labels = max_labels;
		multiplier = preferred_n_labels / (float) actual_n_labels;
	}

	for (i = 0; i <= actual_n_labels; i++) {
		int x = self->left_margin - 10;
		int y = self->height - self->margin - (i * self->y_span) / (float)actual_n_labels;

		$(self->image, drawHorizontalLine, x, x+10, y, RGB_BLACK);

		int value = (int) (i * multiplier);

		char str [kMaxTickTextLength];
		snprintf (str, kMaxTickTextLength-1, "%d GB/s", value);

		$(self->image, drawMiniString, str, x - 40, y - MINIFONT_HEIGHT/2, RGB_BLACK);
	}
}

static void
SimpleGraphing_setXAxisMode (SimpleGraphing* self, int x_axis_mode)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image)
		return;
	if (self) {
		if (x_axis_mode != MODE_X_AXIS_LINEAR && x_axis_mode != MODE_X_AXIS_LOG2)
			return;

		self->x_axis_mode = x_axis_mode;
	}
}

static void 
SimpleGraphing_clear (SimpleGraphing* self)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image)
		return;
	if (self) {
		$(self->image, clear);
		$(self->data, removeAll);
	}
}

static MutableImage *SimpleGraphing_image (SimpleGraphing* self)
{       
	if (!self) {
		return NULL;
	}
	verifyCorrectClass(self,SimpleGraphing);
        if (self) 
		return self->image;
	
	return NULL;
}

SimpleGraphing * SimpleGraphing_init (SimpleGraphing* self)
{
	ENSURE_CLASS_READY(SimpleGraphing);

	if (!self) {
		return NULL;
	}
	
	Object_init ((Object*)self);
	self->is_a = _SimpleGraphingClass;

	self->x_axis_mode = MODE_X_AXIS_LINEAR; // Default value

	self->margin = kDefaultMargin;
	self->legendMargin = kDefaultMargin/3;
	self->left_margin = kDefaultLeftMargin;

	self->data = retain(new(MutableArray));
	self->lineNamesArray = retain(new(MutableArray));
	self->lineColorsArray = retain(new(MutableArray));

	self->font = retain(FontPK_with ("cmr", 18));

	if (FontFreetype_isAvailable()) {
		self->subtitleFont = (Font*) FontFreetype_fromFile ("/usr/share/fonts/truetype/dejavu/DejaVuSerifCondensed-Italic.ttf", 14);
	}

	if (!self->subtitleFont) {
		self->subtitleFont = (Font*) FontPK_with ("cmr", 14);
	}
	if (self->subtitleFont) {
		retain(self->subtitleFont);
	}

	return self;
}

SimpleGraphing * SimpleGraphing_initWithSize (SimpleGraphing* self, int width, int height)
{
	ENSURE_CLASS_READY(SimpleGraphing);

	if (!self) {
		return NULL;
	}

	SimpleGraphing_init (self);

	if (width <= 0 || height <= 0) {
		width = kDefaultWidth;
		height = kDefaultHeight;
	}

	self->width = width;
	self->height = height;

	self->image = retain(MutableImage_withSize(width, height));

	$(self->image, clear);

	self->x_span = self->width - (self->margin + self->left_margin);
	self->y_span = self->height - 2 * self->margin;

	return self;
}

SimpleGraphing* SimpleGraphing_withSize (int width, int height)
{
	SimpleGraphing* graphing = allocate(SimpleGraphing);
	return SimpleGraphing_initWithSize (graphing, width, height);
}

static void 
SimpleGraphing_setTitle (SimpleGraphing* self, Any *title_)
{
	if (!self) {
		return;
	}
	if (title_ == NULL) {
		error_null_parameter (__FUNCTION__);
	}
	verifyCorrectClass(self,SimpleGraphing);
	verifyCorrectClassOrSubclass(title_,String);
	String *title = title_;

	if (self->image == NULL) {
		return;
	}

	// RULE: We'll decide what font to use below
	// based on image size and text length,
	// to get the maximum size that will fit.

	if (title != self->title) {
		if (self->title) {
			release(self->title);
		}
		self->title = retain(title);
	}

	int y = self->margin/4;

	static int fontSizesToTry[4] = { 35, 26, 18, 14 };

	int x = 0;
	int lastTried = 0;
	for (int fs = 0; fs < 4; fs++) {
		int sizeToTry = fontSizesToTry[fs];
		lastTried = sizeToTry;
		self->titleFont = retain(FontPK_with ("cmr", sizeToTry));
		int w = $(self->titleFont, stringWidth, title);
		x = (self->width - w) / 2;
		if (x >= 0) {
			break;
		} else {
			release(self->titleFont);
			self->titleFont = NULL;
		}
	}
	if (!self->titleFont) {
		self->titleFont = retain(FontPK_with ("cmr", lastTried));
	}

	$(self->image, drawString, self->title, x, y, self->titleFont, RGB_BLACK);

	self->y_span -= self->titleFont->height;
}

static void 
SimpleGraphing_setSubtitle (SimpleGraphing* self, Any *subtitle_)
{
	if (!self) {
		return;
	}
	if (!subtitle_) {
		error_null_parameter (__FUNCTION__);
	}
	verifyCorrectClass(self,SimpleGraphing);
	verifyCorrectClassOrSubclass(subtitle_,String);
	String *subtitle = subtitle_;
	if (!self->image) {
		return;
	}
	if (!self->subtitleFont) {
		return;
	}

	if (subtitle != self->subtitle) {
		if (self->subtitle) {
			release(self->subtitle);
		}
		self->subtitle = retain(subtitle);
	}

	int x = (self->width - $(self->subtitleFont, stringWidth, subtitle)) / 2;
	int y = self->margin/4 + self->titleFont->height + 3;
	$(self->image, drawString, self->subtitle, x, y, self->subtitleFont, RGB_BLACK);

	self->y_span -= self->subtitleFont->height;
}

static void
SimpleGraphing_addLine (SimpleGraphing *self, const char *str, RGB color)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!str)
		error_null_parameter (__FUNCTION__);

	// XX refactor, needs a dictionary

	String *string = String_withCString (str);
	$(self->lineNamesArray, append, string);

	Int *colorObject = Int_withUnsignedLong (color);
	$(self->lineColorsArray, append, colorObject);

	$(self->data, append, Int_withLongLong (DATUM_COLOR));
	$(self->data, append, Int_withLongLong (color));
}

static void
SimpleGraphing_drawLegend (SimpleGraphing *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image)
		return;

	// RULE: Find out what the maximum width is for all the strings in the legend
	//	so that they can be put in a proper box.

	int n = $(self->lineNamesArray, count);
	int maxWidth = 0;
	for (int i=0; i < n; i++) {
		String *string = $(self->lineNamesArray, at, i);
		int width = $(self->font, stringWidth, string);
		if (width > maxWidth)
			maxWidth = width;
	}

	int legend_x0 = self->width - maxWidth - self->margin - 2*self->legendMargin;
	int legend_y0 = self->margin;
	int legend_width = maxWidth + 2*self->legendMargin;

	int fontHeight = $(self->font, height);
	int legend_height = fontHeight * n + 2*self->legendMargin;

	if (self->title && self->titleFont) {
		legend_y0 += self->titleFont->height;
	}
	if (self->subtitle && self->subtitleFont) {
		legend_y0 += self->subtitleFont->height;
	}

	$(self->image, drawRoundedRectangle, legend_x0, legend_y0, legend_width, legend_height, 10, RGB_GRAY8);

	int x = legend_x0 + self->legendMargin;
	int y = legend_y0 + self->legendMargin;

	for (int i=0; i < n; i++) {
		Int *color = $(self->lineColorsArray, at, i);
		RGB rgb = (RGB) $(color, asLongLong);

		String *string = $(self->lineNamesArray, at, i);
		$(self->image, drawString, string, x, y, self->font, rgb);
		y += fontHeight;
	}
	
	self->fg = 0;
	self->last_x = self->last_y = -1;
}

//----------------------------------------------------------------------------
// Name:	SimpleGraphing_addPoint
// Purpose:	Adds a point to self list to be drawn.
//----------------------------------------------------------------------------

static void
SimpleGraphing_addPoint (SimpleGraphing *self, long x, long y)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image) {
		return;
	}

	Int *datumX = Int_withLongLong (DATUM_X);
	$(self->data, append, datumX);

	Int *xObject = Int_withLongLong (x);
	$(self->data, append, xObject);

	Int *datumY = Int_withLongLong (DATUM_Y);
	$(self->data, append, datumY);

	Int *yObject = Int_withLongLong (y);
	$(self->data, append, yObject);
}

//----------------------------------------------------------------------------
// Name:	SimpleGraphing_plotLog2
// Purpose:	Plots a point on the current graph.
//----------------------------------------------------------------------------

static void
SimpleGraphing_plotLog2 (SimpleGraphing *self, long x, long y)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image) {
		return;
	}

	//----------------------------------------
	// Plot the point. The x axis is 
	// logarithmic, base 2.
	//
	double tmp = log2 (x);
	tmp -= (double) self->min_x;
	tmp *= (double) self->x_span;
	tmp /= (double) (self->max_x - self->min_x);

	int x2 = self->left_margin + (int) tmp;
	int y2 = self->height - self->margin - (y * self->y_span) / self->max_y;

	if (self->last_x != -1 && self->last_y != -1) {
		if (self->fg & DASHED) 
			$(self->image, drawDashedLine, self->last_x, self->last_y, x2, y2, self->fg & 0xffffff);
		else
			$(self->image, drawLine, self->last_x, self->last_y, x2, y2, self->fg);
	}

	self->last_x = x2;
	self->last_y = y2;
}

//----------------------------------------------------------------------------
// Name:	SimpleGraphing_plotLinear
// Purpose:	Plots a point on the current graph.
//----------------------------------------------------------------------------

static void
SimpleGraphing_plotLinear (SimpleGraphing *self, long x, long y, long max_y)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image) {
		return;
	}

	//----------------------------------------
	// Plot the point. The x axis is 
	// logarithmic, base 2. The units of the
	// y value is kB.
	//
	double tmp = 10. + log2 (x);
	tmp -= (double) XVALUE_MIN;
	tmp *= (double) self->x_span;
	tmp /= (double) (XVALUE_MAX - XVALUE_MIN);
	int x2 = self->left_margin + (int) tmp;
	int y2 = self->height - self->margin - (y * self->y_span) / max_y;

//printf ("\tx=%d, y=%d\n",x,y); fflush(stdout);

	if (self->last_x != -1 && self->last_y != -1) {
		if (self->fg & DASHED) 
			$(self->image, drawDashedLine, self->last_x, self->last_y, x2, y2, self->fg & 0xffffff);
		else
			$(self->image, drawLine, self->last_x, self->last_y, x2, y2, self->fg);
	}

	self->last_x = x2;
	self->last_y = y2;
}

//----------------------------------------------------------------------------
// Name:	SimpleGraphing_makeLog2
// Purpose:	Plots all lines.
//----------------------------------------------------------------------------

static void
SimpleGraphing_makeLog2 (SimpleGraphing *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image) {
		return;
	}

	int count = $(self->data, count);
	if (!count) {
		return;
	}

	SimpleGraphing_drawLabelsLog2 (self);

	//----------------------------------------
	// Draw the lines.
	//
	int i;
	int x = -1, y = -1;
	for (i = 0; i < count; i += 2) 
	{
		Int *typeObj = $(self->data, at, i);
		Int *valueObj = $(self->data, at, i+1);
		long type = $(typeObj,asLongLong);
		long long value = $(valueObj,asLongLong);

		switch (type) {
		case DATUM_Y:	y = value; break;
		case DATUM_X:	x = value; break;
		case DATUM_COLOR:	
			self->fg = (unsigned long) value; 
			self->last_x = -1;
			self->last_y = -1;
			break;
		default:
			exit(1);
		}

		if (x != -1 && y != -1) {
			$(self, plotLog2, x, y);
			x = y = -1;
		}
	}
}

//----------------------------------------------------------------------------
// Name:	SimpleGraphing_makeLinear
// Purpose:	Plots linear graph.
//----------------------------------------------------------------------------

static void
SimpleGraphing_makeLinear (SimpleGraphing *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image) {
		return;
	}

	int i;

	// No data
	int count = $(self->data, count);
	if (!count) {
		return;
	}

	//----------------------------------------
	// Get the maximum bandwidth in order to
	// properly scale the graph vertically.
	//
	int max_y = 0;
	for (i = 0; i < count; i += 2) {
		Int *typeObj = $(self->data, at, i);
		Int *valueObj = $(self->data, at, i+1);
		long type = $(typeObj,asLongLong);
		long long value = $(valueObj,asLongLong);

		if (type == DATUM_Y) {
			if (value > max_y)
				max_y = value;
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
	$(self, drawAxes);

	//----------
	// X axis:
	//
	if (XVALUE_MIN < 10)
		return; // error ("Minimum y is too small.");

	for (i = XVALUE_MIN; i <= XVALUE_MAX; i++) {
		char str[kMaxTickTextLength];
		unsigned long y2 = 1 << (i-10); // XX XVALUE_MIN>=10
		if (y2 < 1024)
			snprintf (str, sizeof(str)-1, "%u kB", (unsigned int) y2);
		else
			snprintf (str, sizeof(str)-1, "%lu MB", (unsigned long) (y2 >> 10));

		int x = self->left_margin + ((i - XVALUE_MIN) * self->x_span) / (XVALUE_MAX - XVALUE_MIN);
		int y = self->height - self->margin + 10;
		
		$(self->image, drawVerticalLine, x, y, y-10, RGB_BLACK);
		$(self->image, drawMiniString, str, x - 10, y+8, RGB_BLACK);
	}

	//----------
	// Y axis:
	//
	// Decide what the tick spacing will be.
	for (i = 0; i <= max_y; i += y_spacing) {
		char str[kMaxTickTextLength];
		unsigned long whole = i / 100;
		unsigned long frac = i % 100;
		snprintf (str, sizeof(str)-1, "%lu.%02lu MB/s", whole, frac);

		int x = self->left_margin - 10;
		int y = self->height - self->margin - (i * self->y_span) / max_y;

		$(self->image, drawHorizontalLine, x, x+10, y, RGB_BLACK);
		$(self->image, drawMiniString, str, x - 60, y - MINIFONT_HEIGHT/2, RGB_BLACK);
	}

	//----------------------------------------
	// Draw the data lines.
	//
	int x = -1, y = -1;
	self->last_x = -1;
	self->last_y = -1;
	for (i = 0; i < self->data_index; i += 2) 
	{
		Int *typeObj = $(self->data, at, i);
		Int *valueObj = $(self->data, at, i+1);
		long type = $(typeObj,asLongLong);
		long long value = $(valueObj,asLongLong);

		switch (type) {
		case DATUM_Y:	
			y = value; 
			break;
		case DATUM_X:	
			x = value; 
			break;
		case DATUM_COLOR:	
			self->fg = (unsigned long) value; 
			self->last_x = -1;
			self->last_y = -1;
			break;
		}

		if (x != -1 && y != -1) {
			$(self, plotLinear, x, y, max_y);
			x = y = -1;
		}
	}
}

static void
SimpleGraphing_make (SimpleGraphing *self)
{
	if (!self) {
		return; 
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image) {
		return;
	}

	switch (self->x_axis_mode) {
	case MODE_X_AXIS_LOG2:
		SimpleGraphing_makeLog2 (self);
		break;
	case MODE_X_AXIS_LINEAR:
		SimpleGraphing_makeLinear (self);
		break;
	default:
		fprintf (stderr, "Invalid graph mode %d.\n", self->x_axis_mode);
		break;
	}

	SimpleGraphing_drawLegend (self);
}

void SimpleGraphing_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClass(self_,SimpleGraphing);

	SimpleGraphing *self = self_;

	if (self->image) {
		release(self->image);
		self->image = NULL;
	}

	release(self->data);
	release(self->lineNamesArray);
	release(self->lineColorsArray);
	release(self->title);
	release(self->subtitle);
	release(self->titleFont);
	release(self->font);
	release(self->subtitleFont);

	self->data = NULL;
	self->lineNamesArray = NULL;
	self->lineColorsArray = NULL;
	self->title = NULL;
	self->subtitle = NULL;
	self->titleFont = NULL;
	self->font = NULL;
	self->subtitleFont = NULL;
}

static void SimpleGraphing_describe (SimpleGraphing* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,SimpleGraphing);
	if (!self->image)
		return;
	if (!file)
		file = stdout;
	fprintf (file, "%s(%dx%d)", $(self, className), self->width, self->height);
}

SimpleGraphingClass* SimpleGraphingClass_init (SimpleGraphingClass *class)
{
        SET_SUPERCLASS(Object);

        SET_OVERRIDDEN_METHOD_POINTER(SimpleGraphing,describe);
        SET_OVERRIDDEN_METHOD_POINTER(SimpleGraphing,destroy);

	SET_INHERITED_METHOD_POINTER(SimpleGraphing,Object,print);
	SET_INHERITED_METHOD_POINTER(SimpleGraphing,Object,equals);

	SET_METHOD_POINTER(SimpleGraphing,clear);
	SET_METHOD_POINTER(SimpleGraphing,addPoint);
	SET_METHOD_POINTER(SimpleGraphing,drawLabelsLog2);
	SET_METHOD_POINTER(SimpleGraphing,image);
	SET_METHOD_POINTER(SimpleGraphing,make);
	SET_METHOD_POINTER(SimpleGraphing,addLine);
	SET_METHOD_POINTER(SimpleGraphing,plotLinear);
	SET_METHOD_POINTER(SimpleGraphing,plotLog2);
	SET_METHOD_POINTER(SimpleGraphing,setTitle);
	SET_METHOD_POINTER(SimpleGraphing,setSubtitle);
	SET_METHOD_POINTER(SimpleGraphing,setXAxisMode);
	SET_METHOD_POINTER(SimpleGraphing,drawAxes);

	VALIDATE_CLASS_STRUCT(class);
        return class;
}

