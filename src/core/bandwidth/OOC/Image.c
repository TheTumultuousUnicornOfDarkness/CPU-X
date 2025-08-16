/*============================================================================
  Image, an object-oriented C image class.
  Copyright (C) 2009-2019 by Zack T Smith.

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
#include <stdbool.h>
#include <wchar.h>

#include "Image.h"
#include "Image_shrink.h"
#include "Utility.h"
#include "Log.h"

// Not needed for bandwidth program.
#undef HAVE_JPEGLIB
#undef HAVE_TIFF

#ifdef HAVE_JPEGLIB
#include <setjmp.h>
#include <jpeglib.h>
#endif

#ifdef HAVE_TIFF
#include <tiff.h>
#include <tiffio.h>
#endif

ImageClass *_ImageClass = NULL;

void Image_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,Image);

	Image *self = self_;
	
	if (self->pixels) {
		ooc_free (self->pixels);
		self->pixels = NULL;
	}
}

void Image_print (Image* self, FILE *file)
{
	if (!self)
		return;
	verifyCorrectClass(self,Image);
	
	// No op
}

static void 
Image_describe (Image* self, FILE *file)
{
	if (!self)
		return;
	verifyCorrectClass(self,Image);
	
	fprintf (file ?: stdout, "%s: %dx%d\n", $(self, className), self->width, self->height);
}

Image* Image_init (Image* self)
{
	ENSURE_CLASS_READY(Image);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _ImageClass;

#ifdef DEBUG
		self->width = 0;
		self->height = 0;
		self->pixelFormat = PixelFormatRGB32;
		self->pixels = NULL;
#endif
	}
	return self;
}

Image* Image_initWithPath (Image* self, char *imageFilePath)
{
	ENSURE_CLASS_READY(Image);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _ImageClass;

		if (imageFilePath) {
			self->path = strdup (imageFilePath);
		}
	}
	return self;
}

/*---------------------------------------------------------------------------
 * Name:	Image_pixelAt
 * Purpose:	Reads pixel out of self.
 *-------------------------------------------------------------------------*/
RGB Image_pixelAt(Image *self, int x, int y)
{
	if (!self || x<0 || y<0)
		return 0;
	verifyCorrectClassOrSubclass(self,Image);
	if (x >= self->width || y >= self->height)
		return 0;
	if (!self->pixels)
		return 0;
	//----------

	return self->pixels[y*self->width + x];
}

/*---------------------------------------------------------------------------
 * Name:	Image_size
 * Purpose:	Returns the image's size.
 *-------------------------------------------------------------------------*/
Size Image_size (Image *self)
{
	Size size;
	size.width = 0;
	size.height = 0;
	if (!self)
		return size;
	verifyCorrectClass(self,Image);

	size.width = self->width;
	size.height = self->height;

	return size;
}

/*---------------------------------------------------------------------------
 * Name:	Image_writeBMP24
 * Purpose:	Writes to BMP 24bpp file.
 *-------------------------------------------------------------------------*/
static bool
Image_writeBMP24 (Image* self, const char *path)
{
	if (!self || !path)
		return false;
	verifyCorrectClassOrSubclass(self,Image);
	if (!self->width) { error(__FUNCTION__, "Zero width image."); }
	if (!self->height) { error(__FUNCTION__, "Zero height image."); }
	//----------

	//--------------------
	// Create the file.
	//
	FILE *f = fopen (path, "wb");
	if (!f)
		return false;

	//---------------------
	// Prepare 24bpp header
	//
#define BMP_HDR_LENGTH (54)
	unsigned char h[BMP_HDR_LENGTH];
	unsigned long len;
	ooc_bzero (h, BMP_HDR_LENGTH);
	unsigned nBytesPerLine = self->width * 3;
	if (nBytesPerLine & 3) {
		nBytesPerLine &= ~3;
		nBytesPerLine += 4;
	}
	len = BMP_HDR_LENGTH + nBytesPerLine * self->height;
	h[0] = 'B';
	h[1] = 'M';
	h[2] = len & 0xff;
	h[3] = (len >> 8) & 0xff;
	h[4] = (len >> 16) & 0xff;
	h[5] = (len >> 24) & 0xff;
	h[10] = BMP_HDR_LENGTH;
	h[14] = 40;
	h[18] = self->width & 0xff;
	h[19] = (self->width >> 8) & 0xff;
	h[20] = (self->width >> 16) & 0xff;
	h[22] = self->height & 0xff;
	h[23] = (self->height >> 8) & 0xff;
	h[24] = (self->height >> 16) & 0xff;
	h[26] = 1;
	h[28] = 24;
	h[34] = 16;
	h[38] = 0x13; // 2835 pixels/meter
	h[39] = 0x0b;
	h[42] = 0x13; // 2835 pixels/meter
	h[43] = 0x0b;

	//--------------------
	// Write header.
	//
	if (BMP_HDR_LENGTH != fwrite (h, 1, BMP_HDR_LENGTH, f)) {
		fclose (f);
		return false;
	}

	//----------------------------------------
	// Write pixels.
	// Note that Image has lower rows first.
	//
	unsigned endOfLinePaddingLength = 4 - (3 & nBytesPerLine);
	if (endOfLinePaddingLength == 4)
		endOfLinePaddingLength = 0;

	unsigned char rgb[4] = { 0, 0, 0, 0 };

	for (int j=self->height-1; j >= 0; j--) {
		for (unsigned i=0; i < self->width; i++) {
			int ix = i + j * self->width;
			RGB pixel = self->pixels[ix];
			rgb[0] = pixel & 0xff;
			rgb[1] = (pixel >> 8) & 0xff;
			rgb[2] = (pixel >> 16) & 0xff;
			if (3 != fwrite (rgb, 1, 3, f)) {
				fclose (f);
				return false;
			}
		}

		if (endOfLinePaddingLength) {
			unsigned char padding[4] = { 0, 0, 0, 0 };
			if (endOfLinePaddingLength != fwrite (padding, 1, endOfLinePaddingLength, f)) {
				fclose (f);
				return false;
			}
		}
	}

	fclose (f);
	return true;
}

/*---------------------------------------------------------------------------
 * Name:	Image_readTIFF
 * Purpose:	Reads an image from a TIFF file.
 *-------------------------------------------------------------------------*/
static bool
Image_readTIFF (Image* self, const char *path)
{
#ifdef HAVE_TIFF
	TIFF *tiff = TIFFOpen (path, "r");
	if (!tiff) {
		return false;
	}

	unsigned long width, height;
	unsigned short depth;
	unsigned short samples;

	bool ok = true;
	do {
		if (!TIFFGetField (tiff, TIFFTAG_IMAGEWIDTH, &width)) {
			ok = false;
			break;
		}
		if (!TIFFGetField (tiff, TIFFTAG_IMAGELENGTH, &height))  {
			ok = false;
			break;
		}
		if (!TIFFGetField (tiff, TIFFTAG_BITSPERSAMPLE, &depth)) {
			ok = false;
			break;
		}
		if (TIFFGetField (tiff, TIFFTAG_SAMPLESPERPIXEL, &samples)) {
			depth *= samples;
			if (depth != 32) {
				ok = false;
			}
		} else {
			ok = false;
		}
	}
	while (false);

	if (!ok) {
		TIFFClose (tiff);
		Log_error_printf (__FUNCTION__, "TIFF %s: missing dimensions or depth\n", path);
		return false;
	}

	Log_debug_printf (__FUNCTION__, "TIFF information: %s, width %lu, height %lu, depth %u\n", path, width, height, depth);

	self->width = width;
	self->height = height;
	self->pixelFormat = PixelFormatRGB32;

	if (self->pixels) {
		free (self->pixels);
	}

	size_t nBytes = width * height * sizeof(RGB);
	self->pixels = (RGB*) malloc (nBytes);
	if (!self->pixels) {
		TIFFClose (tiff);
		perror ("malloc");
		return false;
	}

	if (!TIFFReadRGBAImage (tiff, width, height, self->pixels, 1)) {
		TIFFClose (tiff);
		Log_error (__FUNCTION__, "Failed to read TIFF image file.");
		return false;
	}

	TIFFClose (tiff);
	return true;
#else
	return false;
#endif
}

/*---------------------------------------------------------------------------
 * Name:	Image_writeTIFF
 * Purpose:	Writes to a TIFF file.
 *-------------------------------------------------------------------------*/
static bool
Image_writeTIFF (Image* self, const char *path)
{
#ifdef HAVE_TIFF
	if (!self || !path) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,Image);
	if (!self->width) { 
		error(__FUNCTION__, "Zero width image."); 
	}
	if (!self->height) { 
		error(__FUNCTION__, "Zero height image."); 
	}
	//----------

	//--------------------------------
	// Determine the zoomed image size.
	//
	uint32_t width = self->width;
	uint32_t height = self->height;
	
	printf ("Image %lu x %lu\n", (unsigned long)width, (unsigned long)height);
	
	//--------------------
	// Create the path.
	//
	TIFF* tif = TIFFOpen (path, "wb");
	if (!tif) {
		printf ("CANNOT CREATE TIFF FILE TO: %s\n", path);
		return false;
	}
	TIFFIsByteSwapped(tif);
	
	//TIFFSetField(tif, TIFFTAG_HOSTCOMPUTER, "");
	//TIFFSetField(tif, TIFFTAG_MAKE, "");
	//TIFFSetField(tif, TIFFTAG_MODEL, "");
	//TIFFSetField(tif, TIFFTAG_SOFTWARE, "");
	
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
	
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	const char *timeString = asctime(t);
	TIFFSetField(tif, TIFFTAG_DATETIME, timeString);
	
	printf ("WRITING TIFF (%ux%u): %s\n", width, height, path);
	
	TIFFSetDirectory (tif, 0);
	
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);

	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

	size_t lineLength = width * sizeof(RGB);
	uint8_t buffer [lineLength];
	for (int y = 0; y < height; y++) {
		uint8_t *line = (uint8_t*) &self->pixels [width * y];
		memcpy (buffer, line, lineLength);

		if (TIFFIsBigEndian (tif)) {
			// Swap red and blue.
			int ix = 0;
			for (int x = 0; x < width; x++) {
				uint8_t tmp = buffer [ix];
				buffer[ix] = buffer[ix+2];
				buffer[ix+2] = tmp;
				buffer[ix+3] = 0xff;
				ix += 4;
			}
		}
		TIFFWriteScanline(tif, (void*) buffer, y, 0);
	}
	
	TIFFClose (tif);

	return true;
#else
	return false;
#endif
}

/*---------------------------------------------------------------------------
 * Name:	Image_width
 * Purpose:	Returns the width of the image.
 *-------------------------------------------------------------------------*/
unsigned Image_width (Image* self)
{
	if (!self)
		return 0;
	verifyCorrectClassOrSubclass(self,Image);
	return self->width;
}

/*---------------------------------------------------------------------------
 * Name:	Image_height
 * Purpose:	Returns the height of the image.
 *-------------------------------------------------------------------------*/
unsigned Image_height (Image* self)
{
	if (!self)
		return 0;
	verifyCorrectClassOrSubclass(self,Image);
	return self->height;
}

/*---------------------------------------------------------------------------
 * Name:	Image_writeBMP	
 * Purpose:	Writes to BMP 8bpp or 24bpp file.
 * Returns:	True on success.
 *-------------------------------------------------------------------------*/
bool Image_writeBMP (Image* self, const char *path)
{
	if (!self)
		return 0;
	verifyCorrectClassOrSubclass(self,Image);
	return Image_writeBMP24 (self, path);
}

static uint8_t *Image_pixels (Image *self)
{
	if (!self)
		return NULL;
	verifyCorrectClassOrSubclass(self, Image);

	return (uint8_t*) self->pixels;
}

static uint8_t *Image_pixelRowAddress (Image *self, int y)
{
	if (!self)
		return NULL;
	verifyCorrectClassOrSubclass(self, Image);

	if (!self->pixels || y < 0) {
		return NULL;
	}

	unsigned width = self->width;
	unsigned height = self->height;
	if (y >= height) {
		return NULL;
	}

	switch (self->pixelFormat) {
	case PixelFormatRGB32:
		return (uint8_t*) &self->pixels [y * width];
	case PixelFormatRGB565: {
		uint16_t *pixels = (uint16_t*) self->pixels;
		return (uint8_t*) &pixels [y * width];
	 }
	case PixelFormatGrayscale4:
	case PixelFormatMonochrome:
	default:
		break;
	}
	return NULL;
}

bool Image_equals (Image *self, void *other_)
{ 
	if (!self || !other_) 
		return false;
	verifyCorrectClass(self,Image);
	Image *other = (Image*)other_;
	verifyCorrectClassOrSubclass(self,Image);
	if (self->width != other->width)
		return false;
	if (self->height != other->height)
		return false;

	unsigned width = self->width;
	unsigned height = self->height;
	for (unsigned x=0; x < width; x++) {
		for (unsigned y=0; y < height; y++) {
			RGB first = Image_pixelAt (self, x, y);
			RGB second = Image_pixelAt (other, x, y);
			if (first != second)
				return false;
		}
	}

	return true;
}

ImageClass* ImageClass_init (ImageClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Image, describe);
	SET_OVERRIDDEN_METHOD_POINTER(Image, destroy);
	SET_OVERRIDDEN_METHOD_POINTER(Image, print);
	SET_OVERRIDDEN_METHOD_POINTER(Image, equals);

	SET_METHOD_POINTER(Image, pixelAt);
	SET_METHOD_POINTER(Image, writeBMP);
	SET_METHOD_POINTER(Image, readTIFF);
	SET_METHOD_POINTER(Image, writeTIFF);
	SET_METHOD_POINTER(Image, size);
	SET_METHOD_POINTER(Image, width);
	SET_METHOD_POINTER(Image, height);
	SET_METHOD_POINTER(Image, pixels);
	SET_METHOD_POINTER(Image, pixelRowAddress);
	SET_METHOD_POINTER(Image, shrink);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

/*------------------------------------------------------------------------*/

#ifdef HAVE_JPEGLIB
struct jpeg_error_manager {
	struct jpeg_error_mgr pub;
	jmp_buf setjmpBuffer;
};

typedef struct jpeg_error_manager *jpeg_error_mgr;

static void jpeg_error_fallback (j_common_ptr cinfo)
{
	jpeg_error_mgr myerror = (jpeg_error_mgr) cinfo->err;
	(*cinfo->err->output_message) (cinfo); 
	longjmp (myerror->setjmpBuffer, 1);
}

static inline bool save_jpeg_scanline (JSAMPLE* data, int nBytes, 
					RGB *pixels,
					unsigned y, 
					unsigned nComponents,
					unsigned w, unsigned h)
{
	uint8_t *source = (uint8_t *) data;
	unsigned byteOffset = y * sizeof(RGB) * w;
	uint8_t *destination = byteOffset + (uint8_t *) pixels;

	if (y >= h) {
		Log_warning (__FUNCTION__, "Excessive JPEG data, ignoring it.");
		return false;
	}

	switch (nComponents) {
	case 1: 
		for (int i = 0; i < nBytes; i++) {
			uint8_t byte = *source++;
			*destination++ = byte;
			*destination++ = byte;
			*destination++ = byte;
			*destination++ = 0;
		}
		break;

	case 3:
		for (int i = 0; i < nBytes; i += 3) {
			uint8_t r = *source++;
			uint8_t g = *source++;
			uint8_t b = *source++;
			*destination++ = b;
			*destination++ = g;
			*destination++ = r;
			*destination++ = 0;
		}
		break;

	case 4:
		for (int i = 0; i < nBytes; i += 4) {
			uint8_t r = *source++;
			uint8_t g = *source++;
			uint8_t b = *source++;
			uint8_t a = *source++;
			*destination++ = b;
			*destination++ = g;
			*destination++ = r;
			*destination++ = a;
		}
		break;
	}

	return true;
}

Image *Image_fromJPEGFile (const char *path)
{
	if (!path) {
		return NULL;
	}

	FILE *file = fopen (path, "rb");
	if (!file) {
		Log_perror  (__FUNCTION__, "fopen");
		return NULL;
	}

	struct jpeg_decompress_struct decompressionInfo;
	struct jpeg_error_manager jerr;
	JSAMPARRAY buffer;

	decompressionInfo.err = jpeg_std_error (&jerr.pub);
	jerr.pub.error_exit = jpeg_error_fallback;
	if (setjmp(jerr.setjmpBuffer)) {
		jpeg_destroy_decompress (&decompressionInfo);
		fclose (file);
		return NULL;
	}

	jpeg_create_decompress (&decompressionInfo);
	jpeg_stdio_src (&decompressionInfo, file);
	jpeg_read_header (&decompressionInfo, TRUE);

	decompressionInfo.dct_method = JDCT_FLOAT;
	decompressionInfo.dither_mode = JDITHER_NONE;
	decompressionInfo.out_color_space = JCS_RGB;
	decompressionInfo.output_gamma = 1.0;

	decompressionInfo.buffered_image = false;
	decompressionInfo.do_block_smoothing = true;
	decompressionInfo.do_fancy_upsampling = true;
	decompressionInfo.quantize_colors = false;
	decompressionInfo.raw_data_out = false;
	decompressionInfo.scale_denom = false;
	decompressionInfo.scale_num = true;
	decompressionInfo.two_pass_quantize = false;

	jpeg_start_decompress (&decompressionInfo);

	unsigned width = decompressionInfo.output_width;
	unsigned height = decompressionInfo.output_height;
	unsigned nComponents = decompressionInfo.output_components;
	if (width < 1 || height < 1) {
		Log_warning (__FUNCTION__, "Screen has invalid dimensions.");
		fclose (file);
		return false;
	}

	unsigned rowStride = width * nComponents;

	unsigned nBytes = width * height * sizeof(RGB);
	RGB *pixels = (RGB*) malloc (nBytes);
	if (!pixels) {
		Log_perror(__FUNCTION__, "malloc");
		fclose (file);
		return NULL;
	}

	ooc_bzero (pixels, nBytes);
	
	buffer = (*decompressionInfo.mem->alloc_sarray) ((j_common_ptr) &decompressionInfo, JPOOL_IMAGE, rowStride, 1);

	int y = 0;
	while (decompressionInfo.output_scanline < height) {
		jpeg_read_scanlines (&decompressionInfo, buffer, 1);

		if (!save_jpeg_scanline (buffer[0], rowStride, 
					pixels, y, nComponents, 
					width, height))
			break;
		y++;
	}

	jpeg_finish_decompress (&decompressionInfo);
	jpeg_destroy_decompress (&decompressionInfo);

	fclose (file);

	char temp[64];
	snprintf (temp, sizeof(temp)-1, "Read JPEG (%u x %u, depth %d).", (unsigned)width, (unsigned)height, nComponents);
	Log_debug (__FUNCTION__, temp);

	Image *image = new(Image);
	image->width = width;
	image->height = height;
	image->pixels = pixels;

	return image;
}
#endif

Image *Image_fromFile (const char *path)
{
	if (!path) {
		return NULL;
	}

#ifdef HAVE_JPEGLIB
	if (has_suffix(path, ".jpg") 
	 || has_suffix(path, ".jpeg")
	 || has_suffix(path, ".JPG"))
	{
		return Image_fromJPEGFile (path);
	}
#endif
#ifdef HAVE_TIFF
	if (has_suffix(path, ".tif") 
	 || has_suffix(path, ".tiff")
	 || has_suffix(path, ".TIF"))
	{
		Image *image = new(Image);
		if ($(image, readTIFF, path)) {
			return image;
		} else {
			release(image);
			return false;
		}
	}
#endif

	return NULL;
}

