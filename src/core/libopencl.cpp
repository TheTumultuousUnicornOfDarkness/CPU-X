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


struct PipeDataOpenCL
{
	std::string opencl_version;
	std::string comp_unit;
};

#define CLINFO(dev_id, PARAM, prop) \
	clGetDeviceInfo(dev_id, PARAM, sizeof(prop), &prop, NULL)
static bool opencl_get_vendor_amd(struct PipeDataOpenCL *pdata,
	cl_uint platform_num,
	cl_uint device_num, cl_device_id device_id,
	char *device_name, char *device_version,
	struct pci_dev *pci_dev)
{
	cl_int ret_cl = 0;
	uint32_t comp_unit = 0;
	cl_uint amd_gfx_major = 0;
	cl_device_topology_amd topo_amd;
	MSG_DEBUG("OpenCL platform %u, device %u: vendor is AMD", platform_num, device_num);

	ret_cl = CLINFO(device_id, CL_DEVICE_TOPOLOGY_AMD, topo_amd);
	if(ret_cl != CL_SUCCESS)
	{
		MSG_WARNING(_("OpenCL driver for '%s %s' does not support CL_DEVICE_TOPOLOGY_AMD (%s)"), device_name, device_version, opencl_error(ret_cl));
		return false;
	}

	if((pci_dev->bus  !=  topo_amd.pcie.bus)     ||
	   (pci_dev->dev  !=  topo_amd.pcie.device)  ||
	   (pci_dev->func !=  topo_amd.pcie.function))
	{
		MSG_DEBUG("PCI device %02x:%02x.%d is not matching OpenCL device %02x:%02x.%d", pci_dev->bus, pci_dev->dev, pci_dev->func, topo_amd.pcie.bus, topo_amd.pcie.device, topo_amd.pcie.function);
		return false;
	}

	ret_cl = CLINFO(device_id, CL_DEVICE_GFXIP_MAJOR_AMD, amd_gfx_major);
	if(ret_cl != CL_SUCCESS)
	{
		MSG_WARNING(_("OpenCL driver for '%s %s' does not support CL_DEVICE_GFXIP_MAJOR_AMD (%s)"), device_name, device_version, opencl_error(ret_cl));
		amd_gfx_major = 0;
	}
	MSG_DEBUG("OpenCL platform %u, device %u: CL_DEVICE_GFXIP_MAJOR_AMD is %u", platform_num, device_num, amd_gfx_major);

	ret_cl = CLINFO(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, comp_unit);
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("OpenCL driver for '%s %s' does not support CL_DEVICE_MAX_COMPUTE_UNITS (%s)"), device_name, device_version, opencl_error(ret_cl));
		return false;
	}

	/* Set unit type:
		- Compute Unit (CU): GCN
		- Workgroup Processor (WGP): RDNA, i.e. GFX10+
	*/
	const std::string comp_unit_type = (amd_gfx_major < 10) ? "CU" : "WGP";
	pdata->comp_unit = std::to_string(comp_unit) + " " + comp_unit_type;
	MSG_DEBUG("OpenCL platform %u, device %u: found %s", platform_num, device_num, pdata->comp_unit.c_str());

	return true;
}

static bool opencl_get_vendor_intel(struct PipeDataOpenCL *pdata,
	cl_uint platform_num,
	cl_uint device_num, cl_device_id device_id,
	char *device_name, char *device_version)
{
	cl_int ret_cl = 0;
	uint32_t comp_unit = 0;

	MSG_DEBUG("OpenCL platform %u, device %u: vendor is Intel", platform_num, device_num);
	ret_cl = CLINFO(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, comp_unit);
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("OpenCL driver for '%s %s' does not support CL_DEVICE_MAX_COMPUTE_UNITS (%s)"), device_name, device_version, opencl_error(ret_cl));
		return false;
	}

	pdata->comp_unit = std::to_string(comp_unit) + " EU"; // Execution Unit
	MSG_DEBUG("OpenCL platform %u, device %u: found %s", platform_num, device_num, pdata->comp_unit.c_str());

	return true;
}

static bool opencl_get_vendor_nvidia(struct PipeDataOpenCL *pdata,
	cl_uint platform_num,
	cl_uint device_num, cl_device_id device_id,
	char *device_name, char *device_version,
	struct pci_dev *pci_dev)
{
	cl_int ret_cl = 0;
	cl_uint ocl_domain_nv, ocl_bus_nv, ocl_dev_nv;
	uint8_t ret_domain_nv = 0, ret_bus_nv = 0, ret_dev_nv = 0;
	uint32_t comp_unit = 0;
	MSG_DEBUG("OpenCL platform %u, device %u: vendor is NVIDIA", platform_num, device_num);

	ret_domain_nv = CLINFO(device_id, CL_DEVICE_PCI_DOMAIN_ID_NV, ocl_domain_nv);
	ret_bus_nv    = CLINFO(device_id, CL_DEVICE_PCI_BUS_ID_NV,    ocl_bus_nv);
	ret_dev_nv    = CLINFO(device_id, CL_DEVICE_PCI_SLOT_ID_NV,   ocl_dev_nv); // Slot == Device

	if((ret_domain_nv != CL_SUCCESS) || (ret_bus_nv != CL_SUCCESS) || (ret_dev_nv != CL_SUCCESS))
	{
		MSG_WARNING(_("OpenCL driver for '%s %s' does not support CL_DEVICE_PCI_DOMAIN_ID_NV (%s), CL_DEVICE_PCI_BUS_ID_NV (%s) or CL_DEVICE_PCI_SLOT_ID_NV (%s)"),
					device_name, device_version, opencl_error(ret_domain_nv), opencl_error(ret_bus_nv), opencl_error(ret_dev_nv));
		return false;
	}

	if((pci_dev->domain != static_cast<int>(ocl_domain_nv)) ||
	   (pci_dev->bus    !=       ocl_bus_nv)                ||
	   (pci_dev->dev    !=       ocl_dev_nv))
	{
		MSG_DEBUG("PCI device %04x:%02x:%02x is not matching OpenCL device %04x:%02x:%02x", pci_dev->domain, pci_dev->bus, pci_dev->dev, ocl_domain_nv, ocl_bus_nv, ocl_dev_nv);
		return false;
	}

	ret_cl = CLINFO(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, comp_unit);
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("OpenCL driver for '%s %s' does not support CL_DEVICE_MAX_COMPUTE_UNITS (%s)"), device_name, device_version, opencl_error(ret_cl));
		return false;
	}

	pdata->comp_unit = std::to_string(comp_unit) + " SM"; // Streaming Multiprocessor
	MSG_DEBUG("OpenCL platform %u, device %u: found %s", platform_num, device_num, pdata->comp_unit.c_str());

	return true;
}

/* Get compute units depending on vendor */
static bool opencl_get_vendor(struct PipeDataOpenCL *pdata,
	cl_uint platform_num,
	cl_uint device_num, cl_device_id device_id,
	cl_uint ocl_vendor, char *device_name, char *device_version,
	struct pci_dev *pci_dev)
{
	switch(ocl_vendor)
	{
		case DEV_VENDOR_ID_AMD:    return opencl_get_vendor_amd   (pdata, platform_num, device_num, device_id, device_name, device_version, pci_dev);
		case DEV_VENDOR_ID_INTEL:  return opencl_get_vendor_intel (pdata, platform_num, device_num, device_id, device_name, device_version);
		case DEV_VENDOR_ID_NVIDIA: return opencl_get_vendor_nvidia(pdata, platform_num, device_num, device_id, device_name, device_version, pci_dev);
		default: MSG_WARNING(_("OpenCL is not supported with your GPU vendor (0x%X)"), ocl_vendor);
	}

	return false;
}

#define OPENCL_INFO_BUFFER_SIZE 1024
static bool opencl_get_device(struct PipeDataOpenCL *pdata,
	cl_uint platform_num,
	cl_uint device_num, cl_device_id device_id,
	struct pci_dev *pci_dev)
{
	cl_int ret_cl = 0;
	cl_uint ocl_vendor;
	std::string comp_unit_type;
	char device_name[OPENCL_INFO_BUFFER_SIZE] = "", device_version[OPENCL_INFO_BUFFER_SIZE] = "";
	MSG_DEBUG("Looping into OpenCL platform %u, device %u", platform_num, device_num);

	CLINFO(device_id, CL_DEVICE_VENDOR_ID, ocl_vendor);
	if(pci_dev->vendor_id != ocl_vendor)
		return false;
	MSG_DEBUG("OpenCL platform %u, device %u: found vendor 0x%X", platform_num, device_num, ocl_vendor);

	ret_cl = clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("failed to get name for device %u (%s)"), device_num, opencl_error(ret_cl));
		return false;
	}
	MSG_DEBUG("OpenCL platform %u, device %u: name is '%s'", platform_num, device_num, device_name);

	ret_cl = clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(device_version), device_version, NULL);
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("failed to get version for device %u (%s)"), device_num, opencl_error(ret_cl));
		return false;
	}
	MSG_DEBUG("OpenCL platform %u, device %u: version is '%s'", platform_num, device_num, device_version);

	/* Set OpenCL version */
	pdata->opencl_version = device_version;
	const size_t cl_index = pdata->opencl_version.find("OpenCL");
	if(cl_index != std::string::npos)
		pdata->opencl_version.erase(cl_index, 6 + 1); // 6 = "OpenCL" string
	const size_t mesa_index = pdata->opencl_version.find("Mesa");
	if(mesa_index != std::string::npos)
		pdata->opencl_version.erase(mesa_index, std::string::npos);

	return opencl_get_vendor(pdata, platform_num, device_num, device_id, ocl_vendor, device_name, device_version, pci_dev);
}
#undef CLINFO

static bool opencl_get_platform(struct PipeDataOpenCL *pdata,
	cl_uint platform_num, cl_platform_id platform_id,
	struct pci_dev *pci_dev)
{
	bool gpu_found = false;
	cl_int ret_cl = 0;
	cl_uint num_devices = 0;
	char platform_name[OPENCL_INFO_BUFFER_SIZE] = "", platform_version[OPENCL_INFO_BUFFER_SIZE] = "";
	MSG_DEBUG("Looping into OpenCL platform %u", platform_num);

	ret_cl = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL); // get platform name
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("failed to get name for platform %u (%s)"), platform_num, opencl_error(ret_cl));
		return false;
	}
	MSG_DEBUG("OpenCL platform %u: name is '%s'", platform_num, platform_name);

	ret_cl = clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, sizeof(platform_version), platform_version, NULL); // get platform version
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("failed to get version for platform %u (%s)"), platform_num, opencl_error(ret_cl));
		return false;
	}
	MSG_DEBUG("OpenCL platform %u: version is '%s'", platform_num, platform_version);

	ret_cl = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices); // get number of device
	if((ret_cl != CL_SUCCESS) || (num_devices == 0))
	{
		MSG_ERROR(_("failed to find number of OpenCL devices for platform '%s %s' (%s)"), platform_name, platform_version, (num_devices == 0) ? _("0 device") : opencl_error(ret_cl));
		return false;
	}
	MSG_DEBUG("OpenCL platform %u: found %u devices", platform_num, num_devices);

	std::vector<cl_device_id> devices(num_devices);
	ret_cl = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, num_devices, devices.data(), NULL); // get all devices
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("failed to get all of OpenCL devices for platform '%s %s' (%s)"), platform_name, platform_version, opencl_error(ret_cl));
		return false;
	}

	for(cl_uint device_num = 0; (device_num < num_devices) && !gpu_found; device_num++)
		gpu_found = opencl_get_device(pdata, platform_num, device_num, devices[device_num], pci_dev);

	return gpu_found;
}

/* Set the OpenCL version and Compute Unit (CU) / Workgroup Processor (WGP) / Execution Unit (EU) / Streaming Multiprocessor (SM) for a GPU */
int set_gpu_opencl_version(std::string card_vendor, struct pci_dev *pci_dev, int pfd_out)
{
	bool gpu_found = false;
	struct PipeDataOpenCL pdata = { std::string(), std::string() };
	cl_int ret_cl = 0;
	cl_uint num_platforms = 0;

	MSG_VERBOSE("%s", _("Finding OpenCL API version"));
	ret_cl = clGetPlatformIDs(0, NULL, &num_platforms); // get number of platform
	if((ret_cl != CL_SUCCESS) || (num_platforms == 0))
	{
		MSG_WARNING(_("There is no platform with OpenCL support (%s)"), opencl_error(ret_cl));
		return ret_cl;
	}
	MSG_DEBUG("Number of OpenCL platforms: %u", num_platforms);

	std::vector<cl_platform_id> platforms(num_platforms);
	ret_cl = clGetPlatformIDs(num_platforms, platforms.data(), NULL); // get all platforms
	if(ret_cl != CL_SUCCESS)
	{
		MSG_ERROR(_("failed to get all OpenCL platforms (%s)"), opencl_error(ret_cl));
		return ret_cl;
	}

	for(cl_uint platform_num = 0; (platform_num < num_platforms) && !gpu_found; platform_num++) // find GPU devices
		gpu_found = opencl_get_platform(&pdata, platform_num, platforms[platform_num], pci_dev);

	if(!gpu_found)
		MSG_WARNING(_("Unable to find OpenCL driver for vendor %s"), card_vendor.c_str());

	write_string_to_pipe(pdata.opencl_version, pfd_out);
	write_string_to_pipe(pdata.comp_unit,      pfd_out);

	return (int) ret_cl;
}
#undef OPENCL_INFO_BUFFER_SIZE


#endif /* HAS_LIBPCI */
