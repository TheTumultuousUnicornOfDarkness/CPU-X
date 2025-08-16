/*============================================================================
  CPU, an object-oriented C CPU information class.
  Copyright (C) 2009-2024 by Zack T Smith.

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

#ifndef _OOC_CPU_H
#define _OOC_CPU_H

#include "String.h"
#include "MutableArray.h"

#define DECLARE_CPU_METHODS(TYPE_POINTER) \
	String* (*family) (TYPE_POINTER); \
	String* (*make) (TYPE_POINTER); \
	String* (*model) (TYPE_POINTER); \
	MutableArray* (*features) (TYPE_POINTER); \
	unsigned (*registerSize) (TYPE_POINTER); \
	String* (*instructionSet) (TYPE_POINTER); \
	unsigned (*nCores) (TYPE_POINTER); \
	bool (*hasVectorUnit) (TYPE_POINTER); \
	unsigned (*levelNCacheSize) (TYPE_POINTER, unsigned, unsigned, bool); \
	float (*maximumSpeed) (TYPE_POINTER, unsigned); /* GHz */ \
	float (*minimumSpeed) (TYPE_POINTER, unsigned); /* GHz */ \
	float (*currentSpeed) (TYPE_POINTER, unsigned); /* GHz */ \
	bool (*has128bitVectors) (TYPE_POINTER); \
	bool (*has256bitVectors) (TYPE_POINTER); \
	bool (*has512bitVectors) (TYPE_POINTER); \
	float (*temperature) (TYPE_POINTER, unsigned); 

struct cpu;

typedef struct cpuclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct cpu*)
	DECLARE_CPU_METHODS(struct cpu*)
} CPUClass;

extern CPUClass *_CPUClass;
extern CPUClass* CPUClass_init (CPUClass*);

#define DECLARE_CPU_INSTANCE_VARS(TYPE_POINTER) \
	MutableArray *features; \
	unsigned nCores; \
	unsigned *level1d_sizes;\
	unsigned *level1i_sizes;\
	unsigned *level2_sizes;\
	unsigned *level3_sizes;\
	bool has128bitVectors;\
	bool has128bitCacheBypass;\
	bool has256bitVectors;\
	bool has512bitVectors;\
	bool is_intel;

typedef struct cpu {
	CPUClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct cpu*)
	DECLARE_CPU_INSTANCE_VARS(struct cpu*)
} CPU;

extern CPU* CPU_init (CPU* object);
extern void CPU_destroy (Any* object);

#endif

