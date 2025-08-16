/*============================================================================
  Hardware, an object-oriented C hardware information class.
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

#ifndef _OOC_HARDWARE_H
#define _OOC_HARDWARE_H

#include "String.h"
#include "MutableArray.h"

#define DECLARE_HARDWARE_METHODS(TYPE_POINTER) \
	unsigned (*totalRAM) (TYPE_POINTER); \
	String* (*systemMake) (TYPE_POINTER); \
	String* (*systemModel) (TYPE_POINTER); \
        float (*temperature) (TYPE_POINTER, unsigned zone);

struct hardwareinfo;

typedef struct hardwareinfoclass {
	DECLARE_OBJECT_CLASS_VARS
	DECLARE_OBJECT_METHODS(struct hardwareinfo*)
	DECLARE_HARDWARE_METHODS(struct hardwareinfo*)
} HardwareClass;

extern HardwareClass *_HardwareClass;
extern HardwareClass* HardwareClass_init (HardwareClass*);

#define DECLARE_HARDWARE_INSTANCE_VARS(TYPE_POINTER) 

typedef struct hardwareinfo {
	HardwareClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct hardwareinfo*)
	DECLARE_HARDWARE_INSTANCE_VARS(struct hardwareinfo*)
} Hardware;

extern Hardware* Hardware_init (Hardware* object);
extern void Hardware_destroy (Any* object);

#ifdef __APPLE__
// For reading the CPU/GPU/ambient temperatures.
struct smc_version {
	char major;
	char minor;
	char build;
	char reserved[1];
	uint16_t release;
};
struct smc_plimits {
	uint16_t version;
	uint16_t length;
	uint32_t cpu_plimit;
	uint32_t gpu_plimit;
	uint32_t mem_plimit;
};
struct smc_keyinfo {
	uint32_t data_size;
	uint32_t data_type;
	uint8_t data_attributes;
};
typedef char smc_bytes[32];
struct smc_keydata {
	uint32_t key;
	struct smc_version version;
	struct smc_plimits plimits;
	struct smc_keyinfo keyinfo;
	uint8_t result;
	uint8_t status;
	uint8_t data8;
	uint32_t data32;
	smc_bytes bytes;
};
#define KERNEL_INDEX_SMC 2
#define SMC_CPU_TEMPERATURE "TC0P"
#define SMC_GPU_TEMPERATURE "TG0P"
#define SMC_AMBIENT_TEMPERATURE "TA0V"
#define SMC_DATA_TYPE_SP78 "sp78"
enum {
	kSMCCommandReadKeyInfo = 9,
	kSMCCommandReadBytes = 5,
};
#endif
#endif

