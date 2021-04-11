/*******************************************************************************
 * Copyright (c) 2008-2020 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

/*
* PROJECT CPU-X
* FILE opencl_ext.h
*/

#ifndef _OPENCL_EXT_H_
#define _OPENCL_EXT_H_

/* https://stackoverflow.com/a/60890691 */
#define CaseReturnString(x) case x: return #x;
const char *opencl_error(cl_int err)
{
	switch (err)
	{
		CaseReturnString(CL_SUCCESS                        )
		CaseReturnString(CL_DEVICE_NOT_FOUND               )
		CaseReturnString(CL_DEVICE_NOT_AVAILABLE           )
		CaseReturnString(CL_COMPILER_NOT_AVAILABLE         )
		CaseReturnString(CL_MEM_OBJECT_ALLOCATION_FAILURE  )
		CaseReturnString(CL_OUT_OF_RESOURCES               )
		CaseReturnString(CL_OUT_OF_HOST_MEMORY             )
		CaseReturnString(CL_PROFILING_INFO_NOT_AVAILABLE   )
		CaseReturnString(CL_MEM_COPY_OVERLAP               )
		CaseReturnString(CL_IMAGE_FORMAT_MISMATCH          )
		CaseReturnString(CL_IMAGE_FORMAT_NOT_SUPPORTED     )
		CaseReturnString(CL_BUILD_PROGRAM_FAILURE          )
		CaseReturnString(CL_MAP_FAILURE                    )
		CaseReturnString(CL_MISALIGNED_SUB_BUFFER_OFFSET   )
		CaseReturnString(CL_COMPILE_PROGRAM_FAILURE        )
		CaseReturnString(CL_LINKER_NOT_AVAILABLE           )
		CaseReturnString(CL_LINK_PROGRAM_FAILURE           )
		CaseReturnString(CL_DEVICE_PARTITION_FAILED        )
		CaseReturnString(CL_KERNEL_ARG_INFO_NOT_AVAILABLE  )
		CaseReturnString(CL_INVALID_VALUE                  )
		CaseReturnString(CL_INVALID_DEVICE_TYPE            )
		CaseReturnString(CL_INVALID_PLATFORM               )
		CaseReturnString(CL_INVALID_DEVICE                 )
		CaseReturnString(CL_INVALID_CONTEXT                )
		CaseReturnString(CL_INVALID_QUEUE_PROPERTIES       )
		CaseReturnString(CL_INVALID_COMMAND_QUEUE          )
		CaseReturnString(CL_INVALID_HOST_PTR               )
		CaseReturnString(CL_INVALID_MEM_OBJECT             )
		CaseReturnString(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
		CaseReturnString(CL_INVALID_IMAGE_SIZE             )
		CaseReturnString(CL_INVALID_SAMPLER                )
		CaseReturnString(CL_INVALID_BINARY                 )
		CaseReturnString(CL_INVALID_BUILD_OPTIONS          )
		CaseReturnString(CL_INVALID_PROGRAM                )
		CaseReturnString(CL_INVALID_PROGRAM_EXECUTABLE     )
		CaseReturnString(CL_INVALID_KERNEL_NAME            )
		CaseReturnString(CL_INVALID_KERNEL_DEFINITION      )
		CaseReturnString(CL_INVALID_KERNEL                 )
		CaseReturnString(CL_INVALID_ARG_INDEX              )
		CaseReturnString(CL_INVALID_ARG_VALUE              )
		CaseReturnString(CL_INVALID_ARG_SIZE               )
		CaseReturnString(CL_INVALID_KERNEL_ARGS            )
		CaseReturnString(CL_INVALID_WORK_DIMENSION         )
		CaseReturnString(CL_INVALID_WORK_GROUP_SIZE        )
		CaseReturnString(CL_INVALID_WORK_ITEM_SIZE         )
		CaseReturnString(CL_INVALID_GLOBAL_OFFSET          )
		CaseReturnString(CL_INVALID_EVENT_WAIT_LIST        )
		CaseReturnString(CL_INVALID_EVENT                  )
		CaseReturnString(CL_INVALID_OPERATION              )
		CaseReturnString(CL_INVALID_GL_OBJECT              )
		CaseReturnString(CL_INVALID_BUFFER_SIZE            )
		CaseReturnString(CL_INVALID_MIP_LEVEL              )
		CaseReturnString(CL_INVALID_GLOBAL_WORK_SIZE       )
		CaseReturnString(CL_INVALID_PROPERTY               )
		CaseReturnString(CL_INVALID_IMAGE_DESCRIPTOR       )
		CaseReturnString(CL_INVALID_COMPILER_OPTIONS       )
		CaseReturnString(CL_INVALID_LINKER_OPTIONS         )
		CaseReturnString(CL_INVALID_DEVICE_PARTITION_COUNT )
		default: return "Unknown OpenCL error code";
	}
}


/*********************************
* cl_amd_device_attribute_query *
*********************************/

#define CL_DEVICE_TOPOLOGY_AMD                          0x4037
#define CL_DEVICE_BOARD_NAME_AMD                        0x4038
#define CL_DEVICE_GFXIP_MAJOR_AMD                       0x404A
#define CL_DEVICE_GFXIP_MINOR_AMD                       0x404B
#define CL_DEVICE_PCIE_ID_AMD                           0x4034

typedef union
{
	struct { cl_uint type; cl_uint data[5]; } raw;
	struct { cl_uint type; cl_uchar unused[17]; cl_uchar bus; cl_uchar device; cl_uchar function; } pcie;
} cl_device_topology_amd;

#define CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD            1


/******************************************
* cl_nv_device_attribute_query extension *
******************************************/

#define CL_DEVICE_PCI_BUS_ID_NV                     0x4008
#define CL_DEVICE_PCI_SLOT_ID_NV                    0x4009
#define CL_DEVICE_PCI_DOMAIN_ID_NV                  0x400A

#endif /* _OPENCL_EXT_H_ */
