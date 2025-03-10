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


static inline const char* string_VkResult(VkResult input_value)
{
	switch(input_value)
	{
		/*case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
			return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";*/
		case VK_ERROR_DEVICE_LOST:
			return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
#if (VK_HEADER_VERSION >150)
		case VK_ERROR_FRAGMENTATION:
			return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_ERROR_UNKNOWN:
			return "VK_ERROR_UNKNOWN";
#endif
		case VK_ERROR_FRAGMENTED_POOL:
			return "VK_ERROR_FRAGMENTED_POOL";
#ifdef VK_ENABLE_BETA_EXTENSIONS
		case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
			return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
#endif /* VK_ENABLE_BETA_EXTENSIONS */
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_INVALID_SHADER_NV:
			return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		/*case VK_ERROR_NOT_PERMITTED_KHR:
			return "VK_ERROR_NOT_PERMITTED_KHR";*/
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "VK_ERROR_VALIDATION_FAILED_EXT";
#ifdef VK_ENABLE_BETA_EXTENSIONS
		case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
#endif /* VK_ENABLE_BETA_EXTENSIONS */
#ifdef VK_ENABLE_BETA_EXTENSIONS
		case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
#endif /* VK_ENABLE_BETA_EXTENSIONS */
#ifdef VK_ENABLE_BETA_EXTENSIONS
		case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
#endif /* VK_ENABLE_BETA_EXTENSIONS */
#ifdef VK_ENABLE_BETA_EXTENSIONS
		case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
#endif /* VK_ENABLE_BETA_EXTENSIONS */
#ifdef VK_ENABLE_BETA_EXTENSIONS
		case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
#endif /* VK_ENABLE_BETA_EXTENSIONS */
		case VK_EVENT_RESET:
			return "VK_EVENT_RESET";
		case VK_EVENT_SET:
			return "VK_EVENT_SET";
		case VK_INCOMPLETE:
			return "VK_INCOMPLETE";
		case VK_NOT_READY:
			return "VK_NOT_READY";
		/*case VK_OPERATION_DEFERRED_KHR:
			return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_PIPELINE_COMPILE_REQUIRED:
			return "VK_PIPELINE_COMPILE_REQUIRED";*/
		case VK_SUBOPTIMAL_KHR:
			return "VK_SUBOPTIMAL_KHR";
		case VK_SUCCESS:
			return "VK_SUCCESS";
		/*case VK_THREAD_DONE_KHR:
			return "VK_THREAD_DONE_KHR";
		case VK_THREAD_IDLE_KHR:
			return "VK_THREAD_IDLE_KHR";*/
		case VK_TIMEOUT:
			return "VK_TIMEOUT";
		default:
			return "Unhandled VkResult";
	}
}

/* Set the Vulkan version for GPU */
int set_gpu_vulkan_version([[maybe_unused]] Data::Graphics::Card &card, [[maybe_unused]] struct pci_dev *dev)
{
	uint32_t device_count = 0;
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
	if(__sigabrt_received || (vk_err != VK_SUCCESS))
	{
		MSG_ERROR(_("failed to call vkCreateInstance (%s)"), __sigabrt_received ? "SIGABRT" : string_VkResult(vk_err));
		__sigabrt_received = false;

		if(vk_err == VK_ERROR_EXTENSION_NOT_PRESENT)
			MSG_ERROR(_("%s is not supported"), VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		return 1;
	}

	/* Get number of devices */
	vk_err = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	if(__sigabrt_received || (vk_err != VK_SUCCESS))
	{
		MSG_ERROR(_("failed to call vkEnumeratePhysicalDevices (%s)"), __sigabrt_received ? "SIGABRT" : string_VkResult(vk_err));
		__sigabrt_received = false;
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
		MSG_WARNING(_("No available physical devices (%s)"), string_VkResult(vk_err));
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
			MSG_WARNING(_("Failed to create Vulkan for device %u (%s)"), i, string_VkResult(vk_err));

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
				card.vram_size = heap_info.memoryProperties.memoryHeaps[heap].size;

#ifdef VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
		VkDevice vk_dev_rt{};
		card.vulkan_rt.value = (vkCreateDevice(devices[i], &check_rt, NULL, &vk_dev_rt) == VK_SUCCESS) ? _("Enabled") : _("Disabled");
		MSG_DEBUG("Vulkan device %lu: Ray Tracing support is %s", i, card.vulkan_rt.value.c_str());
		vkDestroyDevice(vk_dev_rt, NULL);
#endif /* VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME */

		card.vulkan_version.value = string_format("%d.%d.%d",
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
		MSG_DEBUG("Vulkan device %lu: version is '%s'", i, card.vulkan_version.value.c_str());
		gpu_found = true;
	}
	vkDestroyInstance(instance, NULL);

	return 0;
}


#endif /* HAS_LIBPCI */
