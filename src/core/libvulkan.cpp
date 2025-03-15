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
* FILE core/libvulkan.cpp
*/

#include <vulkan/vulkan.h>
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "internal.hpp"

#if HAS_LIBPCI

extern "C" {
	#include <pci/pci.h>
}


#define CASE_STR(value) case value: return #value;
static inline const char* vk_get_error_string(VkResult vk_err)
{
	/* VkResult: https://registry.khronos.org/vulkan/specs/latest/man/html/VkResult.html */
	switch(vk_err)
	{
		CASE_STR(VK_SUCCESS)
		CASE_STR(VK_NOT_READY)
		CASE_STR(VK_TIMEOUT)
		CASE_STR(VK_EVENT_SET)
		CASE_STR(VK_EVENT_RESET)
		CASE_STR(VK_INCOMPLETE)
		CASE_STR(VK_ERROR_OUT_OF_HOST_MEMORY)
		CASE_STR(VK_ERROR_OUT_OF_DEVICE_MEMORY)
		CASE_STR(VK_ERROR_INITIALIZATION_FAILED)
		CASE_STR(VK_ERROR_DEVICE_LOST)
		CASE_STR(VK_ERROR_MEMORY_MAP_FAILED)
		CASE_STR(VK_ERROR_LAYER_NOT_PRESENT)
		CASE_STR(VK_ERROR_EXTENSION_NOT_PRESENT)
		CASE_STR(VK_ERROR_FEATURE_NOT_PRESENT)
		CASE_STR(VK_ERROR_INCOMPATIBLE_DRIVER)
		CASE_STR(VK_ERROR_TOO_MANY_OBJECTS)
		CASE_STR(VK_ERROR_FORMAT_NOT_SUPPORTED)
		CASE_STR(VK_ERROR_FRAGMENTED_POOL)
		CASE_STR(VK_ERROR_UNKNOWN)
#ifdef VK_VERSION_1_1
		CASE_STR(VK_ERROR_OUT_OF_POOL_MEMORY)
		CASE_STR(VK_ERROR_INVALID_EXTERNAL_HANDLE)
#endif
#ifdef VK_VERSION_1_2
		CASE_STR(VK_ERROR_FRAGMENTATION)
		CASE_STR(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
#endif
#ifdef VK_VERSION_1_3
		CASE_STR(VK_PIPELINE_COMPILE_REQUIRED)
#endif
#ifdef VK_VERSION_1_4
		CASE_STR(VK_ERROR_NOT_PERMITTED)
#endif
#ifdef VK_KHR_surface
		CASE_STR(VK_ERROR_SURFACE_LOST_KHR)
		CASE_STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
#endif
#ifdef VK_KHR_swapchain
		CASE_STR(VK_SUBOPTIMAL_KHR)
		CASE_STR(VK_ERROR_OUT_OF_DATE_KHR)
#endif
#ifdef VK_KHR_display_swapchain
		CASE_STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
#endif
#ifdef VK_EXT_debug_report
		CASE_STR(VK_ERROR_VALIDATION_FAILED_EXT)
#endif
#ifdef VK_NV_glsl_shader
		CASE_STR(VK_ERROR_INVALID_SHADER_NV)
#endif
#ifdef VK_KHR_video_queue
		CASE_STR(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
		CASE_STR(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
		CASE_STR(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
		CASE_STR(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
		CASE_STR(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
		CASE_STR(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
#endif
#ifdef VK_EXT_image_drm_format_modifier
		CASE_STR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
#endif
#ifdef VK_EXT_full_screen_exclusive
		CASE_STR(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
#endif
#ifdef VK_KHR_deferred_host_operations
		CASE_STR(VK_THREAD_IDLE_KHR)
		CASE_STR(VK_THREAD_DONE_KHR)
		CASE_STR(VK_OPERATION_DEFERRED_KHR)
		CASE_STR(VK_OPERATION_NOT_DEFERRED_KHR)
#endif
#ifdef VK_KHR_video_encode_queue
		CASE_STR(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR)
#endif
#ifdef VK_EXT_image_compression_control
		CASE_STR(VK_ERROR_COMPRESSION_EXHAUSTED_EXT)
#endif
#ifdef VK_EXT_shader_object
		/* VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT is a deprecated alias */
# ifdef VK_INCOMPATIBLE_SHADER_BINARY_EXT
		CASE_STR(VK_INCOMPATIBLE_SHADER_BINARY_EXT)
# else
		CASE_STR(VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT)
# endif
#endif
#ifdef VK_KHR_pipeline_binary
		CASE_STR(VK_PIPELINE_BINARY_MISSING_KHR)
		CASE_STR(VK_ERROR_NOT_ENOUGH_SPACE_KHR)
#endif
		default: return "Unhandled VkResult";
	}
}
#undef CASE_STR

/* Set the Vulkan version for GPU */
int set_gpu_vulkan_version(std::string card_vendor, struct pci_dev *dev, int pfd_out)
{
	uint32_t device_count = 0;
	uint64_t vram_size = 0;
	bool gpu_found = false;
	bool use_device_id = false;
	VkResult vk_err;
	VkInstance instance{};
	std::vector<VkPhysicalDevice> devices;

	MSG_VERBOSE("%s", _("Finding Vulkan API version"));
	std::vector<const char*> ext_create_info;
	ext_create_info.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	VkInstanceCreateInfo createInfo{};
	createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext                   = NULL;
	createInfo.flags                   = 0;
	createInfo.pApplicationInfo        = NULL;
	createInfo.enabledLayerCount       = 0;
	createInfo.ppEnabledLayerNames     = NULL;
	createInfo.enabledExtensionCount   = (uint32_t) ext_create_info.size();
	createInfo.ppEnabledExtensionNames = ext_create_info.data();

	vk_err = vkCreateInstance(&createInfo, NULL, &instance);
	if(vk_err != VK_SUCCESS)
	{
		MSG_ERROR(_("failed to call vkCreateInstance (%s)"), vk_get_error_string(vk_err));
		if(vk_err == VK_ERROR_EXTENSION_NOT_PRESENT)
			MSG_ERROR(_("%s is not supported"), VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		return 1;
	}

	/* Get number of devices */
	vk_err = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	if(vk_err != VK_SUCCESS)
	{
		MSG_ERROR(_("failed to call vkEnumeratePhysicalDevices (%s)"), vk_get_error_string(vk_err));
		return 2;
	}

	MSG_DEBUG("Vulkan devices count: %u", device_count);
	if(device_count == 0)
	{
		MSG_WARNING("%s", _("No available Vulkan devices"));
		return 3;
	}

	/* Get all device handles */
	devices.resize(device_count);
	if((vk_err = vkEnumeratePhysicalDevices(instance, &device_count, devices.data())) != VK_SUCCESS)
	{
		MSG_WARNING(_("No available physical devices (%s)"), vk_get_error_string(vk_err));
		return 4;
	}

	const float queue_priorities[] = { 1.0f };
	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.pNext            = NULL;
	queue_create_info.flags            = 0;
	queue_create_info.queueFamilyIndex = 0;
	queue_create_info.queueCount       = 1;
	queue_create_info.pQueuePriorities = queue_priorities;

#ifdef VK_EXT_PCI_BUS_INFO_EXTENSION_NAME
	std::vector<const char*> ext_pci_bus_info;
	ext_pci_bus_info.emplace_back(VK_EXT_PCI_BUS_INFO_EXTENSION_NAME);
	VkDeviceCreateInfo check_pci_bus_info{};
	check_pci_bus_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	check_pci_bus_info.pNext                   = NULL;
	check_pci_bus_info.flags                   = 0;
	check_pci_bus_info.queueCreateInfoCount    = 1;
	check_pci_bus_info.pQueueCreateInfos       = &queue_create_info;
	check_pci_bus_info.enabledLayerCount       = 0;
	check_pci_bus_info.ppEnabledLayerNames     = NULL;
	check_pci_bus_info.enabledExtensionCount   = (uint32_t) ext_pci_bus_info.size();
	check_pci_bus_info.ppEnabledExtensionNames = ext_pci_bus_info.data();
	check_pci_bus_info.pEnabledFeatures        = NULL;
#endif /* VK_EXT_PCI_BUS_INFO_EXTENSION_NAME */

#ifdef VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
	std::vector<const char*> ext_rt;
	ext_rt.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	VkDeviceCreateInfo check_rt{};
	check_rt.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	check_rt.pNext                   = NULL;
	check_rt.flags                   = 0;
	check_rt.queueCreateInfoCount    = 1;
	check_rt.pQueueCreateInfos       = &queue_create_info;
	check_rt.enabledLayerCount       = 0;
	check_rt.ppEnabledLayerNames     = NULL;
	check_rt.enabledExtensionCount   = (uint32_t) ext_rt.size();
	check_rt.ppEnabledExtensionNames = ext_rt.data();
	check_rt.pEnabledFeatures        = NULL;
#endif /* VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME */
	VkPhysicalDeviceMemoryProperties2 heap_info{};
	heap_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	heap_info.pNext = NULL;

	VkPhysicalDevicePCIBusInfoPropertiesEXT bus_info{};
	bus_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
	bus_info.pNext = NULL;

	VkPhysicalDeviceProperties2 prop2{};
	prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	prop2.pNext = &bus_info;

	for(uint32_t i = 0; (i < device_count) && !gpu_found; i++)
	{
		MSG_DEBUG("Looping into Vulkan device %lu", i);
#ifdef VK_EXT_PCI_BUS_INFO_EXTENSION_NAME
		VkDevice vk_dev_bus_info{};
		if((vk_err = vkCreateDevice(devices[i], &check_pci_bus_info, NULL, &vk_dev_bus_info)) != VK_SUCCESS)
		{
			MSG_WARNING(_("Failed to create Vulkan for device %u (%s)"), i, vk_get_error_string(vk_err));

			if(vk_err == VK_ERROR_EXTENSION_NOT_PRESENT)
			{
				MSG_WARNING(_("%s is not supported for device %u, use only deviceID for matching"), VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, i);
				use_device_id = true;
			}
		}
		vkDestroyDevice(vk_dev_bus_info, NULL);
#else
		use_device_id = true;
#endif /* VK_EXT_PCI_BUS_INFO_EXTENSION_NAME */
		vkGetPhysicalDeviceProperties2(devices[i], &prop2);
		if(use_device_id && ((uint32_t) dev->device_id != prop2.properties.deviceID))
		{
			MSG_DEBUG("Vulkan device %lu: use only deviceID but device %u does not match device %lu", i, dev->device_id, prop2.properties.deviceID);
			continue;
		}
		else if(!use_device_id && (uint32_t(dev->domain)   != bus_info.pciDomain    ||
		                           dev->bus                != bus_info.pciBus       ||
		                           dev->dev                != bus_info.pciDevice    ||
		                           dev->func               != bus_info.pciFunction))
		{
			MSG_DEBUG("Vulkan device %lu: device does not match with VkPhysicalDevicePCIBusInfoPropertiesEXT", i);
			continue;
		}
		else
			MSG_DEBUG("Vulkan device %lu: device matches with pci_dev", i);

		vkGetPhysicalDeviceMemoryProperties2(devices[i], &heap_info);
		for(uint32_t heap = 0; heap < heap_info.memoryProperties.memoryHeapCount; heap++)
			if(VK_MEMORY_HEAP_DEVICE_LOCAL_BIT == heap_info.memoryProperties.memoryHeaps[heap].flags)
				vram_size = heap_info.memoryProperties.memoryHeaps[heap].size;
		write(pfd_out, &vram_size, sizeof(vram_size));

#ifdef VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
		VkDevice vk_dev_rt{};
		const std::string vulkan_rt = (vkCreateDevice(devices[i], &check_rt, NULL, &vk_dev_rt) == VK_SUCCESS) ? _("Enabled") : _("Disabled");
		MSG_DEBUG("Vulkan device %lu: Ray Tracing support is %s", i, vulkan_rt.c_str());
		write_string_to_pipe(vulkan_rt, pfd_out);
		vkDestroyDevice(vk_dev_rt, NULL);
#endif /* VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME */

		const std::string vulkan_version = string_format("%d.%d.%d",
#if(VK_API_VERSION_MAJOR && VK_API_VERSION_MINOR && VK_API_VERSION_PATCH)
			VK_API_VERSION_MAJOR(prop2.properties.apiVersion),
			VK_API_VERSION_MINOR(prop2.properties.apiVersion),
			VK_API_VERSION_PATCH(prop2.properties.apiVersion)
#else
			VK_VERSION_MAJOR(prop2.properties.apiVersion),
			VK_VERSION_MINOR(prop2.properties.apiVersion),
			VK_VERSION_PATCH(prop2.properties.apiVersion)
#endif /* (VK_API_VERSION_MAJOR && VK_API_VERSION_MINOR && VK_API_VERSION_PATCH) */
		);
		MSG_DEBUG("Vulkan device %lu: version is '%s'", i, vulkan_version.c_str());
		write_string_to_pipe(vulkan_version, pfd_out);
		gpu_found = true;
	}
	vkDestroyInstance(instance, NULL);

	if(!gpu_found)
	{
		uint64_t vram_size = 0;
		MSG_WARNING(_("Unable to find Vulkan driver for vendor %s"), card_vendor.c_str());
		write(pfd_out, &vram_size, sizeof(vram_size)); // VRAM size
#ifdef VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
		write_string_to_pipe(std::string(), pfd_out); // Vulkan RT
#endif /* VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME */
		write_string_to_pipe(std::string(), pfd_out); // Vulkan version
	}

	return 0;
}


#endif /* HAS_LIBPCI */
