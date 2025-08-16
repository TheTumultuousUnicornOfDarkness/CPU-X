/*============================================================================
  Data, an object-oriented C immutable byte array class.
  Copyright (C) 2021 by Zack T Smith.

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

#include "Data.h"
#include "Log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <openssl/sha.h>
#include <sys/mman.h>

DataClass *_DataClass = NULL;

void Data_print (Data* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Data);

	if (!file)
		file = stdout;

	size_t length = self->nBytes;
	for (unsigned i=0; i < length; i++) {
		fprintf (file, "%02x", (unsigned)self->bytes[i]);
	}
}

#if 0
bool Data_hash (Data* self)
{
	verifyCorrectClass(self,Data);

	// TODO
	SHA256_CTX context;
	SHA256_Init (&context);
	SHA256_Update (&context, input, self->nBytes);
	SHA256_Final (output, &context);
	self->print (self, output);

	return true;
}
#endif

bool Data_equals (Data* self, void *other_)
{
	if (!self || !other_)
		return false;
	verifyCorrectClass(self,Data);
	Data *other = (Data*)other_;
	verifyCorrectClassOrSubclass(other,Data);

	if (!self->nBytes && !other->nBytes)
		return true;
	if (self->nBytes != other->nBytes)
		return false;
	if (!self->bytes && !other->bytes)
		return true;
	if (!self->bytes || !other->bytes)
		return false;

	return !memcmp (self->bytes, other->bytes, self->nBytes);
}

void Data_describe (Data* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Data);
	
#ifdef IS_64BIT
	fprintf (file ?: stdout, "%s(%lu bytes)\n", $(self, className), self->nBytes);
#else
	fprintf (file ?: stdout, "%s(%u bytes)\n", $(self, className), (unsigned)self->nBytes);
#endif
}

void Data_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,Data);
	
	Data *self = (Data*) self_;
	if (self->bytes) {
		ooc_bzero (self->bytes, self->nBytes);
		ooc_free (self->bytes);
	}
}

Data* Data_init (Data* self)
{
	ENSURE_CLASS_READY(Data);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _DataClass;

		self->nBytes = 0;
		self->bytes = NULL;
	}
	return self;
}

Data* Data_initWithCString (Data* self, const char *str)
{
	ENSURE_CLASS_READY(Data);

	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Data);

	Object_init ((Object*)self);
	self->is_a = _DataClass;

	size_t len = ooc_strlen (str);
	self->nBytes = len;
	self->bytes = malloc (1 + self->nBytes); 
	if (str) {
		for (size_t i=0; i < len; i++)
			self->bytes[i] = str[i];
	}

	return self;
}

Data* Data_initFromFilePath (Data* self, const char *path)
{
	ENSURE_CLASS_READY(Data);

	if (!self) {
		return NULL;
	}
	if (!path)
		return self;

	Object_init ((Object*)self);
	self->is_a = _DataClass;

	int fd = open (path, O_RDONLY);
	if (!fd) {
		perror ("open");
	}
	else {
		struct stat st;
		size_t length;
		if (stat (path, &st)) {
			perror ("stat");
		}
		else {
			length = st.st_size;
			void *address = mmap (NULL, length, PROT_READ, 0, fd, 0);
			if (!address) {
				perror ("mmap");
			}
			else {
				self->bytes = address;
				self->nBytes = length;
				self->isMemoryMappedFile = true;
			}
		}
	}

	return self;
}

Data* Data_initWithData (Data* self, void *other_)
{
	ENSURE_CLASS_READY(Data);

	if (!self) {
		return NULL;
	}
	if (!other_)
		return self;

	verifyCorrectClass(self,Data);
	Data *other = (Data*)other_;
	verifyCorrectClassOrSubclass(other,Data);

	Object_init ((Object*)self);
	self->is_a = _DataClass;

	size_t len = other->nBytes;
	self->nBytes = len;
	self->bytes = malloc (len); 
	memcpy ((char*) self->bytes, (char*) other->bytes, len);

	return self;
}

// Note, this does not duplicate the bytes.
Data* Data_initWithBytes (Data *self, uint8_t *bytes, size_t length)
{
	ENSURE_CLASS_READY(Data);

	if (!self) {
		return NULL;
	}
	verifyCorrectClass(self,Data);

	Object_init ((Object*)self);
	self->is_a = _DataClass;

	if (bytes && length > 0) {
		self->nBytes = length;
		self->bytes = bytes;
	}

	return self;
}

size_t Data_length (Data *self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Data);

	return self->nBytes;
}

char *Data_asCString (Data *self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Data);

	size_t length = self->nBytes;
	if (!length || !self->bytes)
		return NULL;

	char *string = malloc (length + 1);
	if (!string) {
		Log_perror(__FUNCTION__, "malloc");
		return NULL;
	}
	memcpy (string, self->bytes, length);
	string[length] = 0;

	/* Caller frees the string */ 
	return string;
}

uint8_t *Data_bytes (Data *self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Data);

	return self->bytes;
}

uint8_t Data_byteAt (Data *self, size_t index)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Data);

	if (index < self->nBytes) {
		return self->bytes[index];
	}
	return 0;
}

unsigned long Data_sum (Data *self)
{
	if (!self) {
		return 0;
	}
	verifyCorrectClassOrSubclass(self,Data);

	unsigned long sum = 0;
	for (size_t index = 0; index < self->nBytes; index++) {
		sum += self->bytes[index];
	}
	return sum;
}

bool Data_writeToFilePath (Data *self, const char *path)
{
	if (!self || !path)
		return false;
	verifyCorrectClassOrSubclass(self,Data);

	FILE *file = fopen (path, "wb");
	if (!file)
		return false;

	if (self->nBytes && self->bytes) {
		fwrite (self->bytes, 1, self->nBytes, file);
	}

	fflush (file);
	fclose (file);

	return true;
}

Data* Data_withData (Any *other_)
{
	if (!other_)
		return NULL;
	verifyCorrectClassOrSubclass(other_,Data);

	Data *other = other_;
	Data* newData = new(Data);
	return Data_initWithData (newData, other);
}

Data* Data_withBytes (uint8_t *bytes, size_t length)
{
	// NULL pointer is allowed.
	Data* newData = new(Data);
	return Data_initWithBytes (newData, bytes, length);
}

DataClass* DataClass_init (DataClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Data,describe);
        SET_OVERRIDDEN_METHOD_POINTER(Data,destroy);

        SET_METHOD_POINTER(Data,length);
        SET_METHOD_POINTER(Data,print);
	SET_METHOD_POINTER(Data,byteAt);
	SET_METHOD_POINTER(Data,bytes);
	SET_METHOD_POINTER(Data,sum);
	SET_METHOD_POINTER(Data,asCString);
	SET_METHOD_POINTER(Data,writeToFilePath);

	VALIDATE_CLASS_STRUCT(class);
	return class;
}

