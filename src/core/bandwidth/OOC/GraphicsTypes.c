
#include <stdbool.h>
#include <math.h>

#include "Log.h"
#include "GraphicsTypes.h"

#define MIN2(AA,BB) ( (AA)<(BB) ? (AA)  :(BB) )
#define MAX2(AA,BB) ( (AA)>(BB) ? (AA)  :(BB) )
#define MAX3(AA,BB,CC) ( MAX2(AA,BB) > (CC) ? MAX2(AA,BB)  :(CC))
#define MIN3(AA,BB,CC) ( MIN2(AA,BB) < (CC) ? MIN2(AA,BB) : (CC))

#define kTempStringLength 64
static char sizeString [kTempStringLength];
static char pointString [kTempStringLength];
static char rectString [kTempStringLength];

Size Size_new(int width, int height) 
{
	Size size;
	if (width < 0)
		Log_warning (__FUNCTION__, "Negative width");
	if (height < 0)
		Log_warning (__FUNCTION__, "Negative height");
	size.width = width;
	size.height = height;
	return size;
}

Size Size_zero (void)
{
	Size size;
	size.width = 0;
	size.height = 0;
	return size;
}

Point Point_new(int x, int y)
{
	Point point;
	point.x = x;
	point.y = y;
	return point;
}

Point Point_zero (void)
{
	return Point_new(0,0);
}

double Point_distance (Point p1, Point p2)
{
	int dx = p1.x - p2.x;
	int dy = p1.y - p2.y;
	int dx2 = dx*dx;
	int dy2 = dy*dy;
	return sqrt ((double)(dx2 + dy2));
}

Point Point_add (Point p1,Point p2)
{
	return Point_new(p1.x + p2.x, p1.y + p2.y);
}

Point Point_subtract (Point p1,Point p2)
{
	return Point_new(p1.x - p2.x, p1.y - p2.y);
}

Point Point_divide (Point p, float scale)
{
	if (!scale)
		return Point_new(0, 0);
	else
		return Point_new(p.x / scale, p.y / scale);
}

Point Point_scale (Point p, float scale)
{
	return Point_new(p.x * scale, p.y * scale);
}

Rect Rect_newWithSize (Size size)
{
	Rect rect;
	rect.size = size;
	rect.origin = Point_zero ();
	return rect;
}

Rect Rect_new(int x, int y, int width, int height)
{
	Rect rect;
	rect.size = Size_new(width, height);
	rect.origin = Point_new(x, y);
	return rect;
}

Rect Rect_zero (void)
{
	return Rect_new(0,0,0,0);
}

bool Rect_containsPoint (Rect r, Point p)
{
	if (p.x < r.origin.x)
		return false;
	if (p.y < r.origin.y)
		return false;
	int rightEdge = r.origin.x + r.size.width - 1;
	if (p.x > rightEdge)
		return false;
	int bottomEdge = r.origin.y + r.size.height - 1;
	if (p.y > bottomEdge)
		return false;

	return true;
}

void Point_print (Point p, FILE *outputFile)
{
	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "(%d,%d)", p.x, p.y);
}

const char *Size_string (Size s)
{
	snprintf (sizeString, kTempStringLength-1, "%dx%d", s.width, s.height);
	return sizeString;
}

const char *Point_string (Point p)
{
	snprintf (pointString, kTempStringLength-1, "(%d,%d)", p.x, p.y);
	return pointString;
}

const char *Rect_string (Rect r)
{
	snprintf (rectString, kTempStringLength-1, "(%d,%d)%dx%d", 
		r.origin.x, r.origin.y, r.size.width, r.size.height);
	return rectString;
}

void Size_print (Size s, FILE *outputFile)
{
	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%dx%d", s.width, s.height);
}

void Rect_print (Rect r, FILE *outputFile)
{
	if (!outputFile)
		outputFile = stdout;

	Point_print (r.origin, outputFile);
	Size_print (r.size, outputFile);
}

bool Point_equals (Point p1, Point p2)
{
	return p1.x == p2.x && p1.y == p2.y;
}

static int saved_dpi = 0;
int convertPointToPixelCoordinate (int coordinate)
{
	if (!saved_dpi)
		saved_dpi = 100; // settings()->dpi;

	switch (saved_dpi) {
	case 100: return coordinate;
	case 125: return (5*coordinate)/4;
	case 150: return (3*coordinate)/2;
	case 200: return coordinate*2;
	case 250: return (5*coordinate)/2;
	case 300: return coordinate*3;
	case 400: return coordinate*4;
	case 500: return coordinate*5;
	default:
		Log_error_printf (__FUNCTION__, "Unsupported DPI %d", saved_dpi);
	}
	return 0;
}

int convertPixelToPointCoordinate (int coordinate)
{
	if (!saved_dpi)
		saved_dpi = 100; // settings()->dpi;

	//coordinate *= kDefaultDPI/(float)saved_dpi;
	//return coordinate;

	switch (saved_dpi) {
	case 100: return coordinate;
	case 125: return (4*coordinate)/5;
	case 150: return (2*coordinate)/3;
	case 200: return coordinate/2;
	case 250: return (2*coordinate)/5;
	case 300: return coordinate/3;
	case 400: return coordinate/4;
	case 500: return coordinate/5;
	default:
		Log_error_printf (__FUNCTION__, "Unsupported DPI %d", saved_dpi);
	}
	return 0;
}

Size sizeToPixels (Size size)
{
	size.width = convertPointToPixelCoordinate(size.width);
	size.height = convertPointToPixelCoordinate(size.height);
	return size;
}

Point pointToPixels (Point location)
{
	location.x = convertPointToPixelCoordinate(location.x);
	location.y = convertPointToPixelCoordinate(location.y);
	return location;
}

Size sizeToPoints (Size size)
{
	size.width = convertPixelToPointCoordinate(size.width);
	size.height = convertPixelToPointCoordinate(size.height);
	return size;
}

Point pixelsToPoints (Point location)
{
	location.x = convertPixelToPointCoordinate(location.x);
	location.y = convertPixelToPointCoordinate(location.y);
	return location;
}

bool Size_equals (Size s1, Size s2)
{
	return s1.width == s2.width && s1.height == s2.height;
}

bool Rect_equals (Rect r1, Rect r2)
{
	return Point_equals (r1.origin, r2.origin) 
		&& Size_equals (r1.size, r2.size);
}

Margins Margins_new (int left, int right, int top, int bottom)
{
	Margins m;
	m.left = left;
	m.right = right;
	m.top = top;
	m.bottom = bottom;
	return m;
}

Margins Margins_zero ()
{
	return Margins_new(0,0,0,0);
}

RGB RGBAverage (RGB A, RGB B) 
{
	RGB r = ((A) >> 16) & 255;	
	RGB g = ((A) >> 8) & 255;	
	RGB b = (A) & 255;	
	RGB r2 = ((B) >> 16) & 255;	
	RGB g2 = ((B) >> 8) & 255;	
	RGB b2 = (B) & 255;	
	r = (r + r2) / 2;
	b = (b + b2) / 2;
	g = (g + g2) / 2;
	return (r << 16) | (g << 8) | b;
}

static void 
rgbToHsv (float r, float g, float b, float *h, float *s, float *v)
{
	// r,g,b values are from 0 to 1
	// h = [0,360], s = [0,1], v = [0,1]
	// if s == 0, then h = -1 (undefined)
	float min, max, delta;
	
	min = MIN3(r, g, b);
	max = MAX3(r, g, b);
	*v = max;
	delta = max - min;
	if (max != 0) {
		*s = delta / max;
	} else {
		*s = 0;
		*h = -1;
		return;
	}
	if (r == max)
		*h = (g - b) / delta;
	else if( g == max )
		*h = 2 + (b - r) / delta;
	else
		*h = 4 + (r - g) / delta;
	*h *= 60;
	if (*h < 0)
		*h += 360;
}

static void 
hsvToRgb (float *r, float *g, float *b, float h, float s, float v)
{
	int i;
	float f, p, q, t;
	
	if (s == 0) {
		*r = *g = *b = v;
		return;
	}
	
	h /= 60.f;
	i = (int) h;
	f = h - i;
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	
	switch (i) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:		
		case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}

RGB RGBChangeIntensity (RGB color, float offset)
{
	long r,g,b;
	r = (color>>16) & 255;
	g = (color >> 8) & 255;
	b = color & 255;

	float red = r / 255.f;
	float green = g / 255.f;
	float blue = b / 255.f;
	float h, s, v;
	rgbToHsv (red, green, blue, &h, &s, &v);

	v += offset;
	if (v > 1)
		v = 1;
	hsvToRgb (&red, &green, &blue, h, s, v);

	r = red * 255.;
	g = green * 255.;
	b = blue * 255.;
	if (r > 255)
		r = 255;
	if (g > 255)
		g = 255;
	if (b > 255)
		b = 255;
	if (r < 0)
		r = 0;
	if (g < 0)
		g = 0;
	if (b < 0)
		b = 0;
	return (r << 16) | (g << 8) | b;
}

bool Rect_overlaps (Rect r, Rect s)
{
	int rx0 = r.origin.x;
	int rx1 = rx0 + r.size.width - 1;
	int ry0 = r.origin.y;
	int ry1 = ry0 + r.size.height - 1;

	int sx0 = s.origin.x;
	int sx1 = sx0 + s.size.width - 1;
	int sy0 = s.origin.y;
	int sy1 = sy0 + s.size.height - 1;
	
	if (rx1 < sx0) return false; // r is left of s.
	if (rx0 > sx1) return false; // r is right of s.
	if (ry1 < sy0) return false; // r is above s.
	if (ry0 > sy1) return false; // r is below s.

	return true;
}
