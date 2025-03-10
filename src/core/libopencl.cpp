/****************************************************************************
*    Copyright © 2014-2025 The Tumultuous Unicorn Of Darkness
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
****************************************************************************/

/*
* PROJECT CPU-X
* FILE core/libopencl.cpp
*/

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include "opencl_ext.h"
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "internal.hpp"

#if HAS_LIBPCI

extern "C" {
	#include <pci/pci.h>
}


#define OPENCL_INFO_BUFFER_SIZE 1024
#define CLINFO(dev_id, PARAM, prop) \
	clGetDeviceInfo(dev_id, PARAM, sizeof(prop), &prop, NULL)
/* Set number of Compute Unit (CU) / Workgroup Processor (WGP) / Execution Unit (EU) / Streaming Multiprocessor (SM) for a GPU */
int set_gpu_compute_unit([[maybe_unused]] Data::Graphics::Card &card, [[maybe_unused]] struct pci_dev *dev)
{
	int ret_cl = 0;

	bool gpu_found = false;
	char platform_name[OPENCL_INFO_BUFFER_SIZE] = "", platform_version[OPENCL_INFO_BUFFER_SIZE] = "";
	cl_uint num_pf = 0;

	MSG_VERBOSE("%s", _("Finding OpenCL API version"));
	ret_cl = clGetPlatformIDs(0, NULL, &num_pf); // get number of platform
	if(__sigabrt_received || (ret_cl != CL_SUCCESS) || (num_pf == 0))
	{
		MSG_WARNING(_("There is no platform with OpenCL support (%s)"), __sigabrt_received ? "SIGABRT" : opencl_error(ret_cl));
		__sigabrt_received = false;
		return ret_cl;
	}
	MSG_DEBUG("Number of OpenCL platforms: %u", num_pf);

	std::vector<cl_platform_id> platforms(num_pf);
	ret_cl = clGetPlatformIDs(num_pf, platforms.data(), NULL); // get all platforms
	if(__sigabrt_received || (ret_cl != CL_SUCCESS))
	{
		MSG_ERROR(_("failed to get all OpenCL platforms (%s)"), __sigabrt_received ? "SIGABRT" : opencl_error(ret_cl));
		__sigabrt_received = false;
		return ret_cl;
	}

	for(cl_uint i = 0; (i < num_pf) && !gpu_found; i++) // find GPU devices
	{
		cl_uint num_ocl_dev = 0;
		MSG_DEBUG("Looping into OpenCL platform %u", i);

		ret_cl = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL); // get platform name
		if(ret_cl != CL_SUCCESS)
		{
			MSG_ERROR(_("failed to get name for platform %u (%s)"), i, opencl_error(ret_cl));
			continue;
		}
		MSG_DEBUG("OpenCL platform %u: name is '%s'", i, platform_name);

		ret_cl = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(platform_version), platform_version, NULL); // get platform version
		if(ret_cl != CL_SUCCESS)
		{
			MSG_ERROR(_("failed to get version for platform %u (%s)"), i, opencl_error(ret_cl));
			continue;
		}
		MSG_DEBUG("OpenCL platform %u: version is '%s'", i, platform_version);

		ret_cl = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_ocl_dev); // get number of device
		if((ret_cl != CL_SUCCESS) || (num_ocl_dev == 0))
		{
			MSG_ERROR(_("failed to find number of OpenCL devices for platform '%s %s' (%s)"), platform_name, platform_version, (num_ocl_dev == 0) ? _("0 device") : opencl_error(ret_cl));
			continue;
		}
		MSG_DEBUG("OpenCL platform %u: found %u devices", i, num_ocl_dev);

		std::vector<cl_device_id> devices(num_ocl_dev);
		ret_cl = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_ocl_dev, devices.data(), NULL); // get all devices
		if(ret_cl != CL_SUCCESS)
		{
			MSG_ERROR(_("failed to get all of OpenCL devices for platform '%s %s' (%s)"), platform_name, platform_version, opencl_error(ret_cl));
			continue;
		}

		for(cl_uint j = 0; (j < num_ocl_dev) && !gpu_found; j++)
		{
			cl_uint ocl_vendor;
			uint32_t comp_unit = 0;
			std::string comp_unit_type;
			char device_name[OPENCL_INFO_BUFFER_SIZE] = "", device_version[OPENCL_INFO_BUFFER_SIZE] = "";
			MSG_DEBUG("Looping into OpenCL platform %u, device %u", i, j);

			CLINFO(devices[j], CL_DEVICE_VENDOR_ID, ocl_vendor);
			if(dev->vendor_id != ocl_vendor)
				continue;
			MSG_DEBUG("OpenCL platform %u, device %u: found vendor 0x%X", i, j, ocl_vendor);

			ret_cl = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
			if(ret_cl != CL_SUCCESS)
			{
				MSG_ERROR(_("failed to get name for device %u (%s)"), j, opencl_error(ret_cl));
				continue;
			}
			MSG_DEBUG("OpenCL platform %u, device %u: name is '%s'", i, j, device_name);

			ret_cl = clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, sizeof(device_version), device_version, NULL);
			if(ret_cl != CL_SUCCESS)
			{
				MSG_ERROR(_("failed to get version for device %u (%s)"), j, opencl_error(ret_cl));
				continue;
			}
			MSG_DEBUG("OpenCL platform %u, device %u: version is '%s'", i, j, device_version);

			/* Set OpenCL version */
			card.opencl_version.value = device_version;
			const size_t cl_index = card.opencl_version.value.find("OpenCL");
			if(cl_index != std::string::npos)
				card.opencl_version.value.erase(cl_index, 6 + 1); // 6 = "OpenCL" string
			const size_t mesa_index = card.opencl_version.value.find("Mesa");
			if(mesa_index != std::string::npos)
				card.opencl_version.value.erase(mesa_index, std::string::npos);

			/* Get compute units depending on vendor */
			switch (ocl_vendor)
			{
				case DEV_VENDOR_ID_AMD:
				{
					cl_uint amd_gfx_major = 0;
					cl_device_topology_amd topo_amd;
					MSG_DEBUG("OpenCL platform %u, device %u: vendor is AMD", i, j);

					ret_cl = CLINFO(devices[j], CL_DEVICE_TOPOLOGY_AMD, topo_amd);
					if(ret_cl != CL_SUCCESS)
					{
						MSG_WARNING(_("OpenCL driver for '%s %s' does not support CL_DEVICE_TOPOLOGY_AMD (%s)"), device_name, device_version, opencl_error(ret_cl));
						continue;
					}

					if((dev->bus  ==  topo_amd.pcie.bus)     &&
					   (dev->dev  ==  topo_amd.pcie.device)  &&
					   (dev->func ==  topo_amd.pcie.function))
					{
						ret_cl = CLINFO(devices[j], CL_DEVICE_GFXIP_MAJOR_AMD, amd_gfx_major);
						if(ret_cl != CL_SUCCESS)
						{
							MSG_WARNING(_("OpenCL driver for '%s %s' does not support CL_DEVICE_GFXIP_MAJOR_AMD (%s)"), device_name, device_version, opencl_error(ret_cl));
							amd_gfx_major = 0;
						}
						MSG_DEBUG("OpenCL platform %u, device %u: CL_DEVICE_GFXIP_MAJOR_AMD is %u", i, j, amd_gfx_major);

						ret_cl = CLINFO(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, comp_unit);
						if(ret_cl != CL_SUCCESS)
						{
							MSG_ERROR(_("OpenCL driver for '%s %s' does not support CL_DEVICE_MAX_COMPUTE_UNITS (%s)"), device_name, device_version, opencl_error(ret_cl));
							continue;
						}
						/* Set unit type:
						   - Compute Unit (CU): GCN
						   - Workgroup Processor (WGP): RDNA, i.e. GFX10+
						*/
						comp_unit_type = (amd_gfx_major < 10) ? "CU" : "WGP";
						MSG_DEBUG("OpenCL platform %u, device %u: found %lu %s", i, j, comp_unit, comp_unit_type.c_str());
						card.comp_unit.value = std::to_string(comp_unit) + " " + comp_unit_type;
						gpu_found = true;
					}
					break;
				}
				case DEV_VENDOR_ID_INTEL:
				{
					MSG_DEBUG("OpenCL platform %u, device %u: vendor is Intel", i, j);
					ret_cl = CLINFO(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, comp_unit);
					if(ret_cl != CL_SUCCESS)
					{
						MSG_ERROR(_("OpenCL driver for '%s %s' does not support CL_DEVICE_MAX_COMPUTE_UNITS (%s)"), device_name, device_version, opencl_error(ret_cl));
						continue;
					}
					comp_unit_type = "EU"; // Execution Unit
					MSG_DEBUG("OpenCL platform %u, device %u: found %lu %s", i, j, comp_unit, comp_unit_type.c_str());
					card.comp_unit.value = std::to_string(comp_unit) + " " + comp_unit_type;
					gpu_found = true;
					break;
				}
				case DEV_VENDOR_ID_NVIDIA:
				{
					cl_uint ocl_domain_nv, ocl_bus_nv, ocl_dev_nv;
					uint8_t ret_domain_nv = 0, ret_bus_nv = 0, ret_dev_nv = 0;
					MSG_DEBUG("OpenCL platform %u, device %u: vendor is NVIDIA", i, j);

					ret_domain_nv = CLINFO(devices[j], CL_DEVICE_PCI_DOMAIN_ID_NV, ocl_domain_nv);
					ret_bus_nv    = CLINFO(devices[j], CL_DEVICE_PCI_BUS_ID_NV,    ocl_bus_nv);
					ret_dev_nv    = CLINFO(devices[j], CL_DEVICE_PCI_SLOT_ID_NV,   ocl_dev_nv); // Slot == Device

					if((ret_domain_nv != CL_SUCCESS) || (ret_bus_nv != CL_SUCCESS) || (ret_dev_nv != CL_SUCCESS))
					{
						MSG_WARNING(_("OpenCL driver for '%s %s' does not support CL_DEVICE_PCI_DOMAIN_ID_NV (%s), CL_DEVICE_PCI_BUS_ID_NV (%s) or CL_DEVICE_PCI_SLOT_ID_NV (%s)"),
						            device_name, device_version, opencl_error(ret_domain_nv), opencl_error(ret_bus_nv), opencl_error(ret_dev_nv));
						continue;
					}

					if((dev->domain == static_cast<int>(ocl_domain_nv)) &&
					   (dev->bus    ==       ocl_bus_nv)                &&
					   (dev->dev    ==       ocl_dev_nv))
					{
						ret_cl = CLINFO(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, comp_unit);
						if(ret_cl != CL_SUCCESS)
						{
							MSG_ERROR(_("OpenCL driver for '%s %s' does not support CL_DEVICE_MAX_COMPUTE_UNITS (%s)"), device_name, device_version, opencl_error(ret_cl));
							continue;
						}
						comp_unit_type = "SM"; // Streaming Multiprocessor
						MSG_DEBUG("OpenCL platform %u, device %u: found %lu %s", i, j, comp_unit, comp_unit_type.c_str());
						card.comp_unit.value = std::to_string(comp_unit) + " " + comp_unit_type;
						gpu_found = true;
					}
					break;
				}
				default:
					MSG_WARNING(_("OpenCL is not supported with your GPU vendor (0x%X)"), ocl_vendor);
					break;
			} /* end switch (vendor_id) */
		} /* end num_ocl_dev */
	} /* end num_pf */

	return ret_cl;
}
#undef OPENCL_INFO_BUFFER_SIZE
#undef CLINFO


#endif /* HAS_LIBPCI */
