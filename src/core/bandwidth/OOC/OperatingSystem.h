/*============================================================================
  OperatingSystem, an object-oriented C operating system information class.
  Copyright (C) 2009-2023 by Zack T Smith.

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

#ifndef _OOC_OPERATINGSYSTEM_H
#define _OOC_OPERATINGSYSTEM_H

#include "String.h"
#include "MutableArray.h"

#define DECLARE_OPERATINGSYSTEM_METHODS(TYPE_POINTER) \
	long (*load) (TYPE_POINTER); \
	long (*uptime) (TYPE_POINTER); \
	unsigned (*memoryPageSize) (TYPE_POINTER); \
	String* (*osType) (TYPE_POINTER); \
	String* (*kernelName) (TYPE_POINTER); \
	String* (*kernelRelease) (TYPE_POINTER); \
	String* (*username) (TYPE_POINTER); \
	unsigned (*userid) (TYPE_POINTER);

struct operatingsysteminfo;

typedef struct operatingsysteminfoclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct operatingsysteminfo*)
	DECLARE_OPERATINGSYSTEM_METHODS(struct operatingsysteminfo*)
} OperatingSystemClass;

extern OperatingSystemClass *_OperatingSystemClass;
extern OperatingSystemClass* OperatingSystemClass_init (OperatingSystemClass*);

#define DECLARE_OPERATINGSYSTEM_INSTANCE_VARS(TYPE_POINTER) 

typedef struct operatingsysteminfo {
	OperatingSystemClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct operatingsysteminfo*)
	DECLARE_OPERATINGSYSTEM_INSTANCE_VARS(struct operatingsysteminfo*)
} OperatingSystem;

extern OperatingSystem* OperatingSystem_init (OperatingSystem* object);
extern void OperatingSystem_destroy (Any* object);

#endif

