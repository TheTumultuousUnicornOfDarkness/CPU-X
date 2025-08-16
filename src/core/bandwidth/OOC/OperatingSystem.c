/*============================================================================
  OperatingSystem, an object-oriented C operating system information class.
  Copyright (C) 2019, 2023 by Zack T Smith.

  Object-Oriented C is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  Object-Oriented C is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOperatingSystemE.  See the
  GNU Lesser General Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public License
  along with this software.  If not, see <http://www.gnu.org/licenses/>.

  The author may be reached at 1@zsmith.co.
 *===========================================================================*/

#include "OperatingSystem.h"
#include "Utility.h"

#include <sys/types.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#endif

OperatingSystemClass *_OperatingSystemClass = NULL;

String* OperatingSystem_osType (OperatingSystem *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

	char *type = NULL;
#ifdef __CYGWIN__
	type = "Windows/Cygwin";
#elif defined(__ANDROID__)
	type = "Android";
#elif defined(__linux__)
	type = "Linux";
#elif defined(__APPLE__)
	type = "macOS";
#elif defined(__WIN32__) || defined(__WIN64__)
	type = "Windows";
#endif

	return type ? String_withCString (type) : NULL;
}

String* OperatingSystem_kernelName (OperatingSystem *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

	char *type = NULL;
#ifdef __CYGWIN__
	type = "WindowsNT";
#elif defined(__ANDROID__)
	type = "Linux";
#elif defined(__linux__)
	type = "Linux";
#elif defined(__APPLE__)
	type = "Darwin";
#elif defined(__WIN32__) || defined(__WIN64__)
	type = "WindowsNT";
#endif

	return type ? String_withCString (type) : NULL;
}

String* OperatingSystem_kernelRelease (OperatingSystem *self) 
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

#ifdef __linux__
	struct utsname uts;
	uname (&uts);
	const char *cstring = uts.release;

	// Truncate to leave out extraneous build info.
	static char truncated_str [512];
	strncpy (truncated_str, cstring, sizeof(truncated_str)-1);
	char *s = strchr (truncated_str, '(');
	if (s) *s = 0;
	s = strchr (truncated_str, '-');
	if (s) *s = 0;

	return strlen(cstring) ? String_withCString (truncated_str) : NULL;
#endif

#if defined(__APPLE__)
        char *result = execute_and_return_first_line ("system_profiler SPSoftwareDataType | grep 'System Version:' | sed 's/^.*System Version: //' | sed 's/(.*//' | sed 's/macOS //'");
	if (result) {
		return String_withCString (result);
	}
#endif

#if defined(__WIN32__) || defined(__WIN64__)
	// If have windows.h, call GetVersionEx() or VerifyVersionInfo()
	// If have uname(), just use Linux code.
#endif

	return NULL;
}

void OperatingSystem_describe (OperatingSystem* self, FILE *file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);
	
	fprintf (file ?: stdout, "%s\n", $(self, className));
}

void OperatingSystem_print (OperatingSystem *self, FILE* file)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

	if (!file)
		file = stdout;
}

bool OperatingSystem_equals (OperatingSystem *self, void *other_) 
{ 
	if (!self || !other_) 
		return false;
	verifyCorrectClassOrSubclass(self,OperatingSystem);
	OperatingSystem *other = (OperatingSystem*) other_;
	verifyCorrectClassOrSubclass(other,OperatingSystem);

	return false;
}

void OperatingSystem_destroy (Any* self_)
{
        DEBUG_DESTROY;

	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,OperatingSystem);

	OperatingSystem *self = self_;

	Object_destroy (self);
}

OperatingSystem* OperatingSystem_init (OperatingSystem* self)
{
	ENSURE_CLASS_READY(OperatingSystem);

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _OperatingSystemClass;
	}
	return self;
}

static long OperatingSystem_load (OperatingSystem *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

#if defined(__linux__) 
	struct sysinfo info;
	if (!sysinfo (&info)) {
		return info.loads[0];
	} else {
		perror("sysinfo");
	}
#endif

	return 0;
}

static long OperatingSystem_uptime (OperatingSystem *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

#if defined(__linux__) 
	struct sysinfo info;
	if (!sysinfo (&info)) {
		return info.uptime;
	} else {
		perror("sysinfo");
	}
#endif

	return 0;
}

static unsigned OperatingSystem_memoryPageSize (OperatingSystem *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

	return getpagesize();
}

static unsigned OperatingSystem_userid (OperatingSystem *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClassOrSubclass(self,OperatingSystem);

	return (unsigned) getuid();
}

String* OperatingSystem_username (OperatingSystem *self)
{
	char *username = getlogin() ?: "";
	String *result = String_withCString(username);
	return result;
}

OperatingSystemClass* OperatingSystemClass_init (OperatingSystemClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(OperatingSystem,describe);
        SET_OVERRIDDEN_METHOD_POINTER(OperatingSystem,destroy);
        SET_OVERRIDDEN_METHOD_POINTER(OperatingSystem,equals);

	SET_METHOD_POINTER(OperatingSystem,load);
	SET_METHOD_POINTER(OperatingSystem,memoryPageSize);
	SET_METHOD_POINTER(OperatingSystem,osType);
	SET_METHOD_POINTER(OperatingSystem,kernelName);
	SET_METHOD_POINTER(OperatingSystem,kernelRelease);
	SET_METHOD_POINTER(OperatingSystem,uptime);
	SET_METHOD_POINTER(OperatingSystem,username);
	SET_METHOD_POINTER(OperatingSystem,userid);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

