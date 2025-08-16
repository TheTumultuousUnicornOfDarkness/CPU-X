#ifndef _GRAPHICS_TYPES_H
#define _GRAPHICS_TYPES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "colors.h"

typedef uint32_t RGB;
typedef uint32_t RGBA;
typedef uint16_t RGB16;

#define INVALID_WINDOW_ID (-1)

typedef enum {
	PixelFormatRGB32 = 0, // OpenGL, X11
	PixelFormatGrayscale4 = 1, // ePaper 16 grays
	PixelFormatMonochrome = 2, // black & white
        PixelFormatRGB565 = 3, // Android
} PixelFormat;

typedef enum {
	StringWrappingBehaviorTruncate = 0,
	StringWrappingBehaviorWrap = 1,
} StringWrappingBehavior;

typedef enum {
	HorizontalAlignmentLeft = 0,
	HorizontalAlignmentRight = 1,
	HorizontalAlignmentCenter = 2,
	HorizontalAlignmentJustified = 3, // <- text only.
} HorizontalAlignment;

typedef enum {
	VerticalAlignmentTop = 0,
	VerticalAlignmentMiddle = 1,
	VerticalAlignmentBottom = 2,
} VerticalAlignment;

typedef union {
	struct {
		short width;
		short height;
	};
	unsigned long zeroable;
} Size;

typedef union {
	struct {
		short x;
		short y;
	};
	unsigned long zeroable;
} Point;

typedef union {
	struct {
		Size size;
		Point origin;
	};
	unsigned long zeroable;
} Rect;

typedef struct {
	struct {
		short left, right, top, bottom;
	};
	unsigned long zeroable;
} Margins;

extern Margins Margins_new (int left, int right, int top, int bottom);
extern Margins Margins_zero ();

extern Point Point_new (int x, int y);
extern Point Point_zero ();
extern Point Point_add (Point,Point);
extern Point Point_subtract (Point,Point);
extern Point Point_scale (Point,float);
extern Point Point_divide (Point,float);
extern bool Point_equals (Point p1, Point p2);
extern void Point_print (Point p, FILE *output);
extern const char *Point_string (Point p);
extern double Point_distance (Point, Point);

extern bool Size_equals (Size s1, Size s2);
extern void Size_print (Size s, FILE *output);
extern Size Size_new (int width, int height);
extern Size Size_zero ();
extern const char *Size_string (Size s);

extern bool Rect_equals (Rect r1, Rect r2);
extern bool Rect_containsPoint (Rect r, Point p);
extern void Rect_print (Rect r, FILE *output);
extern Rect Rect_new (int x, int y, int width, int height);
extern Rect Rect_newWithSize (Size);
extern Rect Rect_zero ();
extern const char *Rect_string (Rect r);
extern bool Rect_overlaps (Rect r1, Rect r2);

extern RGB RGBChangeIntensity (RGB color, float offset);

static inline uint16_t rgbTo565 (uint32_t rgb)
{
	uint32_t red = (rgb >> 5) & 0xf800;     // Upper 5 bits
	uint32_t green = (rgb >> 3) & 0x7e0;   // Middle 5 bits
	uint32_t blue = 31 & (rgb >> 3);        // Low 5 bits
	return (uint16_t) (red | green | blue);
}

static inline RGB rgb565ToRGB (uint16_t rgb)
{
	uint32_t red = (rgb >> 11) << 19;
	uint32_t green = (rgb >> 5) << 10;
	uint32_t blue = (31 & rgb) << 3;
	return red | green | blue;
}

static inline RGB rgbToBGR (uint32_t rgb)
{
	uint32_t red = (rgb >> 16) & 0xff;
	uint32_t green = rgb & 0xff00;
	uint32_t blue = (rgb & 0xff) << 16;
	return red | green | blue;
}

extern Size sizeToPixels (Size size);
extern Point pointToPixels (Point location);
extern Size sizeToPoints (Size size);
extern Point pixelsToPoints (Point location);
extern int convertPointToPixelCoordinate (int coordinate);
extern int convertPixelToPointCoordinate (int coordinate);

typedef enum {
	kWindowPointerEntryMessage = 1,
	kWindowVisibilityMessage,
	kKeyDownMessage,
	kKeyUpMessage,
	kClickedMessage,
	kPointerMovedMessage,
	kPointerDownMessage,
	kPointerUpMessage,
	kQuittingNowMessage,

} EventListenerMessage;

#endif
