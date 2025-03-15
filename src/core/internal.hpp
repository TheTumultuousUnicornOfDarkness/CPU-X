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
* FILE core/internal.hpp
*/

#ifndef _CORE_INTERNAL_HPP_
#define _CORE_INTERNAL_HPP_

#include "data.hpp"

using GpuDrv = Data::Graphics::Card::GpuDrv;


/***************************** Bandwidth *****************************/

/* Compute CPU cache speed */
int call_bandwidth(Data &data);


/***************************** Dmidecode *****************************/

/* Elements provided by dmidecode */
int call_dmidecode(Data &data);


/***************************** Benchmarks *****************************/

/* Report score of benchmarks */
int benchmark_status(Data &data);

/* Set initial values for benchmarks */
void init_benchmarks(Data &data);


/***************************** Libcpuid *****************************/

/* Static elements provided by libcpuid */
int call_libcpuid_static(Data &data);

/* MSRs static values provided by libcpuid */
int call_libcpuid_msr_static(Data &data);

/* Dynamic elements provided by libcpuid */
int call_libcpuid_dynamic(Data &data);

/* MSRs dynamic values provided by libcpuid */
int call_libcpuid_msr_dynamic(Data &data);

/* If dmidecode fails to find CPU package, check in database */
int cputab_package_fallback(Data &data);


/***************************** Libopencl *****************************/

/* Set the OpenCL version and Compute Unit (CU) / Workgroup Processor (WGP) / Execution Unit (EU) / Streaming Multiprocessor (SM) for a GPU */
int set_gpu_opencl_version(std::string card_vendor, struct pci_dev *pci_dev, int pfd_out);


/***************************** Libopengl *****************************/

/* Set the OpenGL version for GPU */
int set_gpu_opengl_version(std::string card_vendor, int pfd_out);


/***************************** Libpci *****************************/

/* Find some PCI devices, like chipset and GPU */
int find_devices(Data &data);

/* Retrieve GPU temperature and clocks */
int gpu_monitoring(Data &data);


/***************************** Libsystem *****************************/

/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
int system_dynamic(Data &data);


/***************************** Libvulkan *****************************/

/* Set the Vulkan version for GPU */
int set_gpu_vulkan_version(std::string card_vendor, struct pci_dev *dev, int pfd_out);


#endif /* _CORE_INTERNAL_HPP_ */
