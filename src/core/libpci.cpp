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
* FILE core/libpci.cpp
*/

#include <cstdarg>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "internal.hpp"
#include "daemon/daemon.h"

#ifndef __linux__
# include <sys/sysctl.h>
#endif

extern "C" {
	#include <pci/pci.h>
}

namespace fs = std::filesystem;


/* Check is GPU is enabled */
static bool gpu_is_on([[maybe_unused]] std::string device_path)
{
	bool ret = true;
#ifdef __linux__
	ret = fs::is_directory(device_path);
#endif /* __linux__ */
	MSG_DEBUG("gpu_is_on: ret=%s", ret ? "true" : "false");
	return ret;
}

/* Find driver name for a device */
static std::string get_gpu_device_path(struct pci_dev *dev)
{
	int err = -1;
	std::string device_path;
#ifdef __linux__
	/* Adapted from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/ls-kernel.c */
	char *base = NULL;

	if(dev->access == NULL)
	{
		MSG_ERROR(_("pci_access is not properly initialized: it is a common issue when %s was built with a lower libpci version.\n"
		          "Check that libpci %s library is present on your system. Otherwise, please rebuild %s."), PRGNAME, LIBPCI_VERSION, PRGNAME);
		return std::string();
	}

	if(dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
	{
		MSG_ERROR("dev->access->method=%u", dev->access->method);
		return std::string();
	}

	if((base = pci_get_param(dev->access, const_cast<char*>("sysfs.path"))) == NULL)
	{
		MSG_ERROR("%s", "pci_get_param (sysfs.path)");
		return std::string();
	}

	device_path = string_format("%s/devices/%04x:%02x:%02x.%d", base, dev->domain, dev->bus, dev->dev, dev->func);
	err         = !fs::is_directory(device_path);
#else /* __linux__ */
	err = popen_to_str(device_path, "sysctl hw.dri | grep busid | grep %04x:%02x:%02x.%d | cut -d. -f1-3", dev->domain, dev->bus, dev->dev, dev->func);
#endif /* __linux__ */

	MSG_DEBUG("find_gpu_device_path: ret=%s", device_path.c_str());
	return (err == 0) ? device_path : std::string();
}

static int set_gpu_kernel_driver(Data::Graphics::Card &card)
{
	std::string cmd;
	std::unordered_map<std::string, Data::Graphics::Card::GpuDrv> gpu_drivers =
	{
		{ "fglrx",    GpuDrv::GPUDRV_FGLRX   },
		{ "radeon",   GpuDrv::GPUDRV_RADEON  },
		{ "amdgpu",   GpuDrv::GPUDRV_AMDGPU  },
		{ "i915",     GpuDrv::GPUDRV_INTEL   },
		{ "nvidia",   GpuDrv::GPUDRV_NVIDIA  },
		{ "nouveau",  GpuDrv::GPUDRV_NOUVEAU },
		{ "vfio-pci", GpuDrv::GPUDRV_VFIO    },
	};

	/* Check GPU state */
	if(!gpu_is_on(card.device_path))
	{
		MSG_WARNING(_("No kernel driver in use for graphic card at path %s"), card.device_path.c_str());
		return 1;
	}

#ifdef __linux__
	fs::path driver_path;
	std::error_code fs_code;

	driver_path = fs::read_symlink(fs::path(card.device_path) / "driver", fs_code);
	if(fs_code)
	{
		MSG_ERROR("set_gpu_kernel_driver(%s): read_symlink: value=%i message=%s", card.device_path.c_str(), fs_code.value(), fs_code.message().c_str());
		return 1;
	}
	card.kernel_driver.value = driver_path.filename();
#else /* __linux__ */
	size_t len                    = MAXSTR;
	char driver_name[MAXSTR]      = "";
	const std::string sysctl_name = card.device_path + ".name";

	if(sysctlbyname(sysctl_name.c_str(), driver_name, &len, NULL, 0) != 0)
	{
		MSG_ERRNO("set_gpu_kernel_driver(%s): sysctlbyname: sysctl_name=%s", card.device_path.c_str(), sysctl_name.c_str());
		return 1;
	}
	card.kernel_driver.value = driver_name;
	card.kernel_driver.value.erase(0, card.kernel_driver.value.find(" ") + 1);
#endif /* __linux__ */

	/* Find GPU driver in gpu_drivers */
	auto it = std::find_if(gpu_drivers.begin(), gpu_drivers.end(), [card](const std::pair<std::string, GpuDrv> gpu_driver)
	{
		return (gpu_driver.first.find(card.kernel_driver.value) != std::string::npos);
	});
	if(it == gpu_drivers.end())
	{
		MSG_WARNING(_("Your GPU kernel driver is unknown: %s"), card.kernel_driver.value.c_str());
		return 1;
	}
	card.driver = it->second;
	MSG_DEBUG("set_gpu_kernel_driver: driver '%s' is index %i", card.kernel_driver.value.c_str(), card.driver);

	/* Check for discrete GPU */
	switch(card.driver)
	{
		case GpuDrv::GPUDRV_NVIDIA:
			if(command_exists("optirun") && !popen_to_str(cmd, "optirun --status") && (cmd.find("Bumblebee status: Ready") != std::string::npos))
				card.driver = GpuDrv::GPUDRV_NVIDIA_BUMBLEBEE;
			break;
		case GpuDrv::GPUDRV_NOUVEAU:
			if(command_exists("optirun") && !popen_to_str(cmd, "optirun --status") && (cmd.find("Bumblebee status: Ready") != std::string::npos))
				card.driver = GpuDrv::GPUDRV_NOUVEAU_BUMBLEBEE;
			break;
		default:
			break;
	}

	return 0;
}

#ifdef PCI_NONRET
PCI_NONRET
#endif /* PCI_NONRET */
static void pcilib_msg_error(char *str, ...)
{
	char *buff = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&buff, str, aptr);
	va_end(aptr);
	MSG_ERROR("pcilib: %s", buff);
	free(buff);

	exit(EXIT_FAILURE);
}

static void pcilib_msg_warning(char *str, ...)
{
	char *buff = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&buff, str, aptr);
	va_end(aptr);
	MSG_WARNING("pcilib: %s", buff);
	free(buff);
}

static void pcilib_msg_debug(char *str, ...)
{
	char *buff = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&buff, str, aptr);
	va_end(aptr);
	MSG_DEBUG("pcilib: %s", buff);
	free(buff);
}

#define DEVICE_VENDOR_STR(d)  pci_lookup_name(pacc, buff, MAXSTR, PCI_LOOKUP_VENDOR, d->vendor_id, d->device_id)
#define DEVICE_PRODUCT_STR(d) pci_lookup_name(pacc, buff, MAXSTR, PCI_LOOKUP_DEVICE, d->vendor_id, d->device_id)
static bool set_chipset_information(struct pci_access *pacc, struct pci_dev *dev, Data::Motherboard::Chipset &chipset)
{
	char buff[MAXSTR] = "";

	chipset.vendor.value = DEVICE_VENDOR_STR(dev);
	chipset.model.value  = DEVICE_PRODUCT_STR(dev);

	return true;
}

#if HAS_LIBEGL
static int set_gpu_opengl_version_dedicated_process(Data::Graphics::Card &card)
{
	int pfds[2];
	pid_t pid;

	if(pipe(pfds) != 0)
	{
		MSG_ERRNO("%s", _("failed to create pipe"));
		return 1;
	}

	if((pid = fork()) < 0)
	{
		MSG_ERRNO("%s", _("failed to create process"));
		return 2;
	}

	if(pid != 0)
	{
		/* Parent process */
		int status;
		MSG_DEBUG("Child process %i created with success for OpenGL", pid);
		close(pfds[STDOUT_FILENO]);
		const int ret = waitpid(pid, &status, 0);
		if((ret > 0) && WIFEXITED(status))
		{
			MSG_DEBUG("PID %i terminated normally for OpenGL", pid);
			card.user_mode_driver.value = read_string_from_pipe(pfds[STDIN_FILENO]);
			card.opengl_version.value   = read_string_from_pipe(pfds[STDIN_FILENO]);
		}
		else
			MSG_DEBUG("PID %i terminated abnormally for OpenGL", pid);
		close(pfds[STDIN_FILENO]);
	}
	else
	{
		/* Child process */
		close(pfds[STDIN_FILENO]);
		const int err = set_gpu_opengl_version(card.vendor.value, pfds[STDOUT_FILENO]);
		close(pfds[STDOUT_FILENO]);
		exit(err);
	}

	return 0;
}
#endif /* HAS_LIBEGL */

static int set_gpu_information(struct pci_access *pacc, struct pci_dev *dev, Data::Graphics &graphics)
{
	int err = 0;
	const uint8_t card_index = graphics.cards.size();
	uint64_t bar_size = 0;
	char buff[MAXSTR] = "";
	std::string gpu_vendor;

	switch(dev->vendor_id)
	{
		case DEV_VENDOR_ID_AMD:
			gpu_vendor = "AMD";
			bar_size   = dev->size[0];
			break;
		case DEV_VENDOR_ID_INTEL:
			gpu_vendor = "Intel";
			bar_size   = dev->size[2];
			break;
		case DEV_VENDOR_ID_NVIDIA:
			gpu_vendor = "NVIDIA";
			bar_size   = dev->size[1];
			break;
		default:
			gpu_vendor = DEVICE_VENDOR_STR(dev);
			MSG_WARNING(_("Your GPU vendor is unknown: %s (0x%X)"), gpu_vendor.c_str(), dev->vendor_id);
	}

	graphics.grow_cards_vector();
	graphics.cards[card_index].device_path     = get_gpu_device_path(dev);
	graphics.cards[card_index].vendor.value    = gpu_vendor;
	graphics.cards[card_index].model.value     = DEVICE_PRODUCT_STR(dev);
	graphics.cards[card_index].device_id.value = string_format("0x%04X:0x%04X", dev->vendor_id, dev->device_id);
	err = set_gpu_kernel_driver(graphics.cards[card_index]);

#if HAS_LIBEGL
	err += set_gpu_opengl_version_dedicated_process(graphics.cards[card_index]);
#endif /* HAS_LIBEGL */
#if HAS_VULKAN
	set_gpu_vulkan_version(graphics.cards[card_index], dev);
#endif /* HAS_VULKAN */
#if HAS_OPENCL
	set_gpu_compute_unit(graphics.cards[card_index], dev);
#endif /* HAS_OPENCL */

	if(graphics.cards[card_index].vram_size > 0)
		graphics.cards[card_index].mem_used.value = string_format("??? / %lu %s", (graphics.cards[card_index].vram_size >> 20), UNIT_MIB);
	if((graphics.cards[card_index].vram_size > 0) && (bar_size > 0))
		graphics.cards[card_index].resizable_bar.value = ((graphics.cards[card_index].vram_size * 9 / 10) < bar_size) ? _("Enabled") : _("Disabled");

#if 0 // For testing purposes
	while(graphics.cards.size() < 8)
	{
		const uint8_t card_index = graphics.cards.size();
		graphics.grow_cards_vector();
		graphics.cards[card_index].vendor.value           = string_format("Vendor %u", card_index);
		graphics.cards[card_index].kernel_driver.value    = string_format("Driver %u", card_index);
		graphics.cards[card_index].user_mode_driver.value = string_format("UMB %u", card_index);
		graphics.cards[card_index].model.value            = string_format("Model %u", card_index);
	}
#endif /* 0 */

	return err;
}
#undef DEVICE_VENDOR_STR
#undef DEVICE_PRODUCT_STR

/* Find some PCI devices, like chipset and GPU */
int find_devices(Data &data)
{
	/* Adapted from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/example.c */
	bool chipset_found = false;
	struct pci_access *pacc;

	MSG_VERBOSE("%s", _("Finding devices"));
#if defined (__linux__) || defined (__gnu_linux__)
	if(access(PCI_PATH_SYS_BUS_PCI, R_OK) || access(PCI_PATH_PROC_BUS_PCI, R_OK))
	{
		MSG_WARNING("%s", _("Skip devices search (PCI device does not exist)"));
		return 1;
	}
#elif defined (__FreeBSD__) || defined (__DragonFly__)
	int ret = -1;
	const DaemonCommand cmd = ACCESS_DEV_PCI;
	if(DAEMON_UP && access(PCI_PATH_FBSD_DEVICE, W_OK))
	{
		SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));
		RECEIVE_DATA(&data.socket_fd, &ret, sizeof(int));
	}
	if(ret && access(PCI_PATH_FBSD_DEVICE, W_OK))
	{
		MSG_WARNING(_("Skip devices search (wrong permissions on %s device)"), PCI_PATH_FBSD_DEVICE);
		return 1;
	}
#else
	MSG_WARNING("This operating system is not officially supported by %s.", PRGNAME);
#endif

	/* Get the pci_access structure */
	MSG_DEBUG("%s", "find_devices: pci_alloc");
	pacc = pci_alloc();
	pacc->error     = pcilib_msg_error;
	pacc->warning   = pcilib_msg_warning;
	pacc->debug     = pcilib_msg_debug;
	pacc->debugging = Logger::get_verbosity() == LOG_DEBUG;

	/* Initialize the PCI library */
	MSG_DEBUG("%s", "find_devices: pci_init");
	pci_init(pacc);

	/* We want to get the list of devices */
	MSG_DEBUG("%s", "find_devices: pci_scan_bus");
	pci_scan_bus(pacc);

	/* Iterate over all devices */
	for(auto dev = pacc->devices; dev != NULL; dev = dev->next)
	{
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
		/* Looking for chipset or GPU */
		if(!chipset_found && (dev->device_class == PCI_CLASS_BRIDGE_ISA))
			chipset_found = set_chipset_information(pacc, dev, data.motherboard.chipset);
		else if((dev->device_class >> 8) == PCI_BASE_CLASS_DISPLAY)
			set_gpu_information(pacc, dev, data.graphics);
	}

	MSG_DEBUG("%s", "find_devices: pci_cleanup");
	pci_cleanup(pacc);
	if(!chipset_found)
		MSG_ERROR("%s", _("failed to find chipset vendor and model"));

	if(data.graphics.cards.size() == 0)
		MSG_ERROR("%s", _("failed to find graphic card vendor and model"));
	else
		Options::set_selected_gpu(Options::get_selected_gpu(), data.graphics.cards.size());

	return (chipset_found == false) + (data.graphics.cards.size() == 0);
}

#ifdef __linux__
/* Check access on /sys/kernel/debug/dri */
static bool can_access_sys_debug_dri(Data &data)
{
	static int ret = 1;
	const DaemonCommand cmd = ACCESS_SYS_DEBUG;

	if(ret == 1)
	{
		if(!access(SYS_DEBUG_DRI, X_OK))
			ret = 0;
		else if(DAEMON_UP)
		{
			SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));
			RECEIVE_DATA(&data.socket_fd, &ret, sizeof(int));
		}
		else
			ret = -2;
	}

	MSG_DEBUG("can_access_sys_debug_dri() ==> %i", ret);
	return !ret;
}

/* Get PCIe interface speed and width */
static std::string get_gpu_interface_info(std::string drm_path, std::string type)
{
	int pcie_width   = 0;
	uint8_t pcie_gen = 0;
	float pcie_speed = 0.0;
	const std::string pp_dpm_pcie_file = drm_path + "/device/" + "pp_dpm_pcie";
	const std::string link_speed_file  = drm_path + "/device/" + type + "_link_speed";
	const std::string link_width_file  = drm_path + "/device/" + type + "_link_width";

	/* Try to parse the pp_dpm_pcie file (AMDGPU) if present */
	if(file_exists(pp_dpm_pcie_file))
	{
		char current, line[40];
		std::string prev_loc = std::setlocale(LC_NUMERIC, nullptr); // for %f in sscanf
		MSG_DEBUG("get_gpu_interface_info: opening '%s'", pp_dpm_pcie_file.c_str());
		std::FILE* fp = std::fopen(pp_dpm_pcie_file.c_str(), "r");
		if(fp)
		{
			std::setlocale(LC_NUMERIC, "C");
			while(std::fgets(line, sizeof(line), fp) != nullptr) // parse all lines in pp_dpm_pcie file
			{
				if(type == "current") // look for line containing '*' at the end
				{
					if((std::sscanf(line, "%*d: %fGT/s, x%d %*dMhz %c", &pcie_speed, &pcie_width, &current) >= 3) && (current == '*'))
						break; // break the loop when current link profile is found
					else
						pcie_width = 0; // reset values when sscanf() does not match pattern
				}
				else if(type == "max") // last line contains the max profile
					std::sscanf(line, "%*d: %fGT/s, x%d %*dMhz", &pcie_speed, &pcie_width);
				else // in case get_gpu_interface_info() is badly called
					MSG_ERROR("get_gpu_interface_info: unknown type '%s'", type.c_str());
			}
			std::fclose(fp);
			std::setlocale(LC_NUMERIC, prev_loc.c_str());
		}
		else
			MSG_ERRNO("get_gpu_interface_info: failed to open '%s' file", pp_dpm_pcie_file.c_str());
	}

	/* Try to parse the {current,max}_link_{speed,width} files otherwise, present since Linux 4.13+ (3 September 2017) */
	if((pcie_width == 0) && file_exists(link_speed_file) && file_exists(link_width_file))
	{
		std::string pcie_speed_raw, pcie_width_raw;
		if(fopen_to_str(pcie_speed_raw, "%s", link_speed_file.c_str()) || fopen_to_str(pcie_width_raw, "%s", link_width_file.c_str()))
			return std::string();
		try
		{
			pcie_speed = std::stof(pcie_speed_raw);
			pcie_width = std::stoi(pcie_width_raw);
		}
		catch(const std::exception& e)
		{
			return std::string();
		}
	}

	switch(int(pcie_speed * 100)) // we multiply by 100 because switch statements do not support floats
	{
		case  250: pcie_gen = 1; break; // 2.5 GT/s is PCIe 1.0
		case  500: pcie_gen = 2; break; // 5.0 GT/s is PCIe 2.0
		case  800: pcie_gen = 3; break; // 8.0 GT/s is PCIe 3.0
		case 1600: pcie_gen = 4; break; // 16.0 GT/s is PCIe 4.0
		case 3200: pcie_gen = 5; break; // 32.0 GT/s is PCIe 5.0
		case 6400: pcie_gen = 6; break; // 64.0 GT/s is PCIe 6.0
		default:   pcie_gen = 0; break;
	}

	return ((pcie_gen == 0) || (pcie_width == 0)) ? std::string() : string_format("Gen%1dx%d", pcie_gen, pcie_width);
}
#endif /* __linux__ */

#define FOPEN_TO_ITEM(item, ...)        item.ret = fopen_to_str(item.value, ##__VA_ARGS__)
#define POPEN_TO_ITEM(item, ...)        item.ret = popen_to_str(item.value, ##__VA_ARGS__)
#define POPEN_DRI_TO_ITEM(item, ...)    item.ret = can_access_sys_debug_dri(data) ? popen_to_str(item.value, ##__VA_ARGS__) : -1
#define SET_LABEL_VALUE_NO_CONV(item, fmt, ...) item_count++; \
                                        if(!item.ret) card.item.value = string_format(fmt, ##__VA_ARGS__); \
                                        else error_count++
#define SET_LABEL_VALUE_CONV(item, fmt, ...) item_count++; \
                                        if(!item.ret && is_unsigned_integer(item.value)) card.item.value = string_format(fmt, ##__VA_ARGS__); \
                                        else error_count++
#define SET_LABEL_VALUE_CONV2(item1, item2, fmt, ...) item_count++; \
                                        if(!item1.ret && !item2.ret && is_unsigned_integer(item1.value) && is_unsigned_integer(item2.value)) card.item1.value = string_format(fmt, ##__VA_ARGS__); \
                                        else error_count++
/* Retrieve GPU temperature and clocks */
int gpu_monitoring([[maybe_unused]] Data &data)
{
	int ret = 0;
	struct Item
	{
		int ret             = - 1;
		long double divisor = 1.0;
		std::string value;
	};

#ifdef __linux__
	static bool init_done = false;
	uint8_t failed_count = 0, fglrx_count = 0, nvidia_count = 0;

	MSG_VERBOSE("%s", _("Retrieving GPU clocks"));
	for(auto& card : data.graphics.cards)
	{
		int item_count = 0, error_count = 0;
		Item vbios_version, interface, temperature, usage, core_voltage, power_avg, core_clock, mem_clock, mem_used, mem_total;

		/* Set kernel driver name in case of changed state for GPU */
		const bool gpu_ok = gpu_is_on(card.device_path);
		if(gpu_ok && (card.driver == GpuDrv::GPUDRV_UNKNOWN))
			set_gpu_kernel_driver(card);
		else if(!gpu_ok)
		{
			card.driver              = GpuDrv::GPUDRV_UNKNOWN;
			card.kernel_driver.value = _("None");
			continue;
		}

		/* Get DRM path and card number */
		if(!init_done)
		{
			card.drm_path = get_device_path_drm(card.device_path);
			if(card.drm_path.empty())
				MSG_WARNING(_("DRM path for %s is unknown"), card.name.c_str());
			else
			{
				const std::size_t pos = card.drm_path.find_last_of("card");
				if(pos != std::string::npos)
					card.drm_card_number = std::stoi(card.drm_path.substr(pos + 1));
				else
					MSG_WARNING(_("Card number for %s is unknown"), card.name.c_str());
			}
		}

		/* Get HWmon path */
		switch(card.driver)
		{
			case GpuDrv::GPUDRV_AMDGPU:
			case GpuDrv::GPUDRV_RADEON:
			case GpuDrv::GPUDRV_NOUVEAU:
			case GpuDrv::GPUDRV_NOUVEAU_BUMBLEBEE:
				temperature.divisor = 1e3;
				if(!init_done)
					card.hwmon_path = get_device_path_hwmon(card.device_path);
				if(!card.hwmon_path.empty())
					FOPEN_TO_ITEM(temperature, "%s/temp1_input", card.hwmon_path.c_str());
				break;
			default:
				break;
		}

		/* PCIe interface information */
		std::string pcie_current_interface = get_gpu_interface_info(card.drm_path, "current");
		std::string pcie_max_interface     = get_gpu_interface_info(card.drm_path, "max");
		if(!pcie_current_interface.empty() && !pcie_max_interface.empty())
			card.interface.value = string_format(_("PCIe %s (current) / %s (max)"), pcie_current_interface.c_str(), pcie_max_interface.c_str());

		/* GPU kernel driver dependent variables */
		switch(card.driver)
		{
			case GpuDrv::GPUDRV_AMDGPU:
			{
				const std::string amdgpu_gpu_busy_file  = (!card.drm_path.empty()) ? card.drm_path + "/device/gpu_busy_percent" : std::string();
				const std::string amdgpu_mem_clock      = card.hwmon_path + "/freq2_input";
				const std::string amdgpu_power1_average = card.hwmon_path + "/power1_average";
				const std::string amdgpu_power1_input   = card.hwmon_path + "/power1_input";
				MSG_DEBUG("gpu_monitoring: amdgpu: amdgpu_gpu_busy_file=%s", amdgpu_gpu_busy_file.c_str());
				FOPEN_TO_ITEM(vbios_version, "%s/device/vbios_version",        card.drm_path.c_str());
				// temperature obtained above
				FOPEN_TO_ITEM(usage,         "%s", amdgpu_gpu_busy_file.c_str()); // Linux 4.19+ (22 October 2018)
				FOPEN_TO_ITEM(core_voltage,  "%s/in0_input",                   card.hwmon_path.c_str());
				if(file_exists(amdgpu_power1_input))
					FOPEN_TO_ITEM(power_avg, "%s", amdgpu_power1_input.c_str());
				else if(file_exists(amdgpu_power1_average))	// still in Linux 6.6 pre-release
					FOPEN_TO_ITEM(power_avg, "%s", amdgpu_power1_average.c_str());
				FOPEN_TO_ITEM(core_clock,    "%s/freq1_input",                 card.hwmon_path.c_str());
				if(file_exists(amdgpu_mem_clock)) // there is no memory frequency for iGPU
					FOPEN_TO_ITEM(mem_clock, "%s", amdgpu_mem_clock.c_str());
				FOPEN_TO_ITEM(mem_used,      "%s/device/mem_info_vram_used",   card.drm_path.c_str());
				FOPEN_TO_ITEM(mem_total,     "%s/device/mem_info_vram_total",  card.drm_path.c_str());
				core_voltage.divisor = 1e3;
				power_avg.divisor    = 1e6;
				core_clock.divisor   = 1e6;
				mem_clock.divisor    = 1e6;
				mem_used.divisor     = mem_total.divisor = 1 << 20;
				break;
			}
			case GpuDrv::GPUDRV_FGLRX:
			{
				// vbios_version not available
				POPEN_TO_ITEM(temperature,  "aticonfig --adapter=%1u --odgt | awk '/Sensor/ { print $5 }'",                       fglrx_count);
				POPEN_TO_ITEM(usage,        "aticonfig --adapter=%1u --odgc | awk '/GPU load/ { sub(\"%\",\"\",$4); print $4 }'", fglrx_count);
				// core_voltage not available
				// power_avg not available
				POPEN_TO_ITEM(core_clock,   "aticonfig --adapter=%1u --odgc | awk '/Current Clocks/ { print $4 }'",               fglrx_count);
				POPEN_TO_ITEM(mem_clock,    "aticonfig --adapter=%1u --odgc | awk '/Current Clocks/ { print $5 }'",               fglrx_count);
				// mem_used not available
				// mem_total not available
				fglrx_count++;
				break;
			}
			case GpuDrv::GPUDRV_INTEL:
			{
				// vbios_version not available
				// temperature not available
				// usage not available
				// core_voltage not available
				// power_avg not available
				FOPEN_TO_ITEM(core_clock, "%s/gt_cur_freq_mhz", card.drm_path.c_str());
				// mem_clock not available
				// mem_used not available
				// mem_total not available
				break;
			}
			case GpuDrv::GPUDRV_RADEON:
			{
				// vbios_version not available
				// temperature obtained above
				// usage not available
				POPEN_DRI_TO_ITEM(core_voltage, "awk -F '(vddc: | vddci:)' 'NR==2 { print $2 }' %s/%u/radeon_pm_info", SYS_DEBUG_DRI, card.drm_card_number);
				// power_avg not available
				POPEN_DRI_TO_ITEM(core_clock,   "awk -F '(sclk: | mclk:)'  'NR==2 { print $2 }' %s/%u/radeon_pm_info", SYS_DEBUG_DRI, card.drm_card_number);
				POPEN_DRI_TO_ITEM(mem_clock,    "awk -F '(mclk: | vddc:)'  'NR==2 { print $2 }' %s/%u/radeon_pm_info", SYS_DEBUG_DRI, card.drm_card_number);
				// mem_used not available
				// mem_total not available
				core_voltage.divisor = 1e3;
				core_clock.divisor   = 100.0;
				mem_clock.divisor    = 100.0;
				break;
			}
			case GpuDrv::GPUDRV_NVIDIA:
			case GpuDrv::GPUDRV_NVIDIA_BUMBLEBEE:
			{
				/* Doc: https://nvidia.custhelp.com/app/answers/detail/a_id/3751/~/useful-nvidia-smi-queries
				        https://briot-jerome.developpez.com/fichiers/blog/nvidia-smi/list.txt */
				const std::string nvidia_cmd_base = (card.driver == GpuDrv::GPUDRV_NVIDIA_BUMBLEBEE) ? "optirun -b none nvidia-smi -c :8" : "nvidia-smi";
				const std::string nvidia_cmd_args = nvidia_cmd_base + " --format=csv,noheader,nounits --id=" + std::to_string(nvidia_count);
				MSG_DEBUG("gpu_monitoring: nvidia: nvidia_cmd_args=%s", nvidia_cmd_args.c_str());
				POPEN_TO_ITEM(vbios_version, "%s --query-gpu=vbios_version",   nvidia_cmd_args.c_str());
				POPEN_TO_ITEM(temperature,   "%s --query-gpu=temperature.gpu", nvidia_cmd_args.c_str());
				POPEN_TO_ITEM(usage,         "%s --query-gpu=utilization.gpu", nvidia_cmd_args.c_str());
				// core_voltage not available
				POPEN_TO_ITEM(power_avg,     "%s --query-gpu=power.draw",      nvidia_cmd_args.c_str());
				POPEN_TO_ITEM(core_clock,    "%s --query-gpu=clocks.gr",       nvidia_cmd_args.c_str());
				POPEN_TO_ITEM(mem_clock,     "%s --query-gpu=clocks.mem",      nvidia_cmd_args.c_str());
				POPEN_TO_ITEM(mem_used,      "%s --query-gpu=memory.used",     nvidia_cmd_args.c_str());
				POPEN_TO_ITEM(mem_total,     "%s --query-gpu=memory.total",    nvidia_cmd_args.c_str());
				nvidia_count++;
				break;
			}
			case GpuDrv::GPUDRV_NOUVEAU:
			case GpuDrv::GPUDRV_NOUVEAU_BUMBLEBEE:
			{
				std::string pstate;
				if(popen_to_str(pstate, "grep '*' %1$s/%2$u/pstate || sed -n 1p %1$s/%2$u/pstate ", SYS_DEBUG_DRI, card.drm_card_number))
					break;
				MSG_DEBUG("gpu_monitoring: nouveau: pstate=%s", pstate.c_str());
				// vbios_version not available
				// temperature obtained above
				// usage not available
				// core_voltage not available
				// power_avg not available
				POPEN_DRI_TO_ITEM(core_clock, "echo %s | grep -oP '(?<=core )[^ ]*' | cut -d- -f2", pstate.c_str());
				POPEN_DRI_TO_ITEM(mem_clock,  "echo %s | grep -oP '(?<=memory )[^ ]*'",             pstate.c_str());
				// mem_used not available
				// mem_total not available
				break;
			}
			default:
				if(!init_done)
					MSG_WARNING(_("Driver for %s doesn't report frequencies"), card.name.c_str());
				continue;
		}

		/* Set labels value */
		SET_LABEL_VALUE_NO_CONV(vbios_version,     "%s",                  vbios_version.value.c_str());
		SET_LABEL_VALUE_CONV(temperature,          "%s",                  string_with_temperature_unit(std::stoull(temperature.value) / temperature.divisor).c_str());
		SET_LABEL_VALUE_NO_CONV(usage,             "%s%%",                usage.value.c_str());
		SET_LABEL_VALUE_CONV(core_voltage,         "%.2Lf V",             std::stoull(core_voltage.value) / core_voltage.divisor);
		SET_LABEL_VALUE_CONV(power_avg,            "%.2Lf W",             std::stoull(power_avg.value)    / power_avg.divisor);
		if(!core_clock.ret && is_unsigned_integer(core_clock.value) && (std::stoull(core_clock.value) > 0)) // sometimes core_clock.value is 0 (#301)
		{
			SET_LABEL_VALUE_CONV(core_clock, "%.0Lf MHz", std::stoull(core_clock.value) / core_clock.divisor);
		}
		SET_LABEL_VALUE_CONV(mem_clock,            "%.0Lf MHz",           std::stoull(mem_clock.value)    / mem_clock.divisor);
		SET_LABEL_VALUE_CONV2(mem_used, mem_total, "%.0Lf %s / %.0Lf %s", std::stoull(mem_used.value)     / mem_used.divisor, UNIT_MIB,
		                                                                  std::stoull(mem_total.value)    / mem_total.divisor, UNIT_MIB);

		if(!init_done && (item_count == error_count))
		{
			failed_count++;
			MSG_ERROR(_("failed to retrieve all monitoring data for %s"), card.name.c_str());
		}
	}
	init_done = true;

	ret = (failed_count == data.graphics.cards.size());
#endif /* __linux__ */
	return ret;
}
#undef FOPEN_TO_ITEM
#undef POPEN_TO_ITEM
#undef POPEN_DRI_TO_ITEM
#undef SET_LABEL_VALUE_NO_CONV
#undef SET_LABEL_VALUE_CONV
#undef SET_LABEL_VALUE_CONV2
