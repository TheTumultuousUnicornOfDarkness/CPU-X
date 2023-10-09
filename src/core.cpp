/****************************************************************************
*    Copyright © 2014-2023 The Tumultuous Unicorn Of Darkness
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
* FILE core.cpp
*/

#include <unistd.h>
#include <sys/utsname.h>
#include <cstring>
#include <cmath>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <regex>
#include "options.hpp"
#include "util.hpp"
#include "data.hpp"
#include "core.hpp"
#include "daemon.h"

#ifndef __linux__
# include <pthread_np.h>
# include <sys/sysctl.h>
# include <sys/cpuset.h>
#endif

#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
# include "databases.h"
#endif

#if HAS_DMIDECODE
# include "dmidecode/libdmidecode.h"
#endif

#if HAS_BANDWIDTH
# include "bandwidth/libbandwidth.h"
#endif

#if HAS_LIBPCI
extern "C" {
# include <pci/pci.h>
}
#endif

#if HAS_LIBPROC2
# include <libproc2/meminfo.h>
# include <libproc2/misc.h>
#endif

#if HAS_LIBPROCPS
# include <proc/sysinfo.h>
#endif

#if HAS_LIBSTATGRAB
# include <statgrab.h>
#endif

#if HAS_LIBGLFW
# include <GL/gl.h>
# include <GLFW/glfw3.h>
#endif

#if HAS_Vulkan
# include <vulkan/vulkan.h>
#endif

#if HAS_OpenCL
# define CL_TARGET_OPENCL_VERSION 120
# include <CL/cl.h>
# include "opencl_ext.h"
#endif

namespace fs = std::filesystem;
using GpuDrv = Data::Graphics::Card::GpuDrv;
extern bool __sigabrt_received;


/************************* Private functions *************************/

/* Avoid to re-run a function if an error was occurred in previous call */
static int err_func(int (*func)(Data &), Data &data)
{
	static std::unordered_map<int (*)(Data &), int> registered;

	MSG_DEBUG("err_func: func=%p ==> ret=%i", func, registered[func]);
	if(registered[func] == 0)
		registered[func] = func(data);

	return registered[func];
}

#if HAS_LIBCPUID
/* MSRs static values provided by libcpuid */
static int call_libcpuid_msr_debug(Data &data, uint16_t all_cpu_count)
{
	const DaemonCommand cmd = LIBCPUID_MSR_DEBUG;

	SEND_DATA(&data.socket_fd, &cmd, sizeof(DaemonCommand));
	SEND_DATA(&data.socket_fd, &all_cpu_count, sizeof(uint16_t));

	return 0;
}

#define RETURN_OR_EXIT(e) { if(Options::get_debug_database()) exit(e); else return e; }
/* Get CPU technology node */
static int cpu_technology(Data::Cpu::CpuType::Processor &processor, cpu_id_t *cpu_id)
{
	int i = -1;
	const Technology_DB *db;

	if(cpu_id->vendor < 0 || cpu_id->model < 0 || cpu_id->ext_model < 0 || cpu_id->ext_family < 0)
		RETURN_OR_EXIT(1);

	MSG_VERBOSE("%s", _("Finding CPU technology"));
	switch(cpu_id->vendor)
	{
		case VENDOR_AMD:   db = technology_amd;     break;
		case VENDOR_INTEL: db = technology_intel;   break;
		default:           db = technology_unknown; break;
	}

	MSG_DEBUG("cpu_technology: model %3i, ext. model %3i, ext. family %3i => values to find",
	          cpu_id->model, cpu_id->ext_model, cpu_id->ext_family);
	while(db[++i].cpu_model != -2)
	{
		if(((db[i].cpu_model      < 0) || (db[i].cpu_model      == cpu_id->model))     &&
		   ((db[i].cpu_ext_model  < 0) || (db[i].cpu_ext_model  == cpu_id->ext_model)) &&
		   ((db[i].cpu_ext_family < 0) || (db[i].cpu_ext_family == cpu_id->ext_family)))
		{
			processor.technology.value = db[i].process;
			MSG_DEBUG("cpu_technology: model %3i, ext. model %3i, ext. family %3i => entry #%03i matches",
			          db[i].cpu_model, db[i].cpu_ext_model, db[i].cpu_ext_family, i);
			RETURN_OR_EXIT(0);
		}
		else
			MSG_DEBUG("cpu_technology: model %3i, ext. model %3i, ext. family %3i => entry #%03i does not match",
			          db[i].cpu_model, db[i].cpu_ext_model, db[i].cpu_ext_family, i);
	}

	MSG_WARNING(_("Your CPU is not present in the database ==> %s, model: %i, ext. model: %i, ext. family: %i"),
	            processor.specification.value.c_str(), cpu_id->model, cpu_id->ext_model, cpu_id->ext_family);
	RETURN_OR_EXIT(2);
}
#undef RETURN_OR_EXIT

/* Static elements provided by libcpuid */
static int call_libcpuid_static(Data &data)
{
	int err = 0;
	uint16_t core_id_offset = 0;
	const char *cpu_purpose = NULL;
	struct cpu_id_t *cpu_id  = NULL;
	struct cpu_raw_data_array_t raw_data;
	struct system_id_t system_id;

	const std::unordered_map<cpu_vendor_t, std::string> cpuvendors =
	{
		{ VENDOR_INTEL,     "Intel"                  },
		{ VENDOR_AMD,       "AMD"                    },
		{ VENDOR_CYRIX,     "Cyrix"                  },
		{ VENDOR_NEXGEN,    "NexGen"                 },
		{ VENDOR_TRANSMETA, "Transmeta"              },
		{ VENDOR_UMC,       "UMC"                    },
		{ VENDOR_CENTAUR,   "Centaur"                },
		{ VENDOR_RISE,      "Rise"                   },
		{ VENDOR_SIS,       "SiS"                    },
		{ VENDOR_NSC,       "National Semiconductor" },
		{ VENDOR_UNKNOWN,   _("unknown")             }
	};

	const struct { const cpu_feature_t flag; const char *str; } cpu_flags[] =
	{
		/* SIMD x86 */
		{ CPU_FEATURE_MMX,      "MMX"    },
		{ CPU_FEATURE_MMXEXT,   "(+)"    },
		{ CPU_FEATURE_3DNOW,    "3DNOW!" },
		{ CPU_FEATURE_3DNOWEXT, "(+)"    },
		{ CPU_FEATURE_SSE,      "SSE(1"  },
		{ CPU_FEATURE_SSE2,     "2"      },
		{ CPU_FEATURE_PNI,      "3"      },
		{ CPU_FEATURE_SSSE3,    "3S"     },
		{ CPU_FEATURE_SSE4_1,   "4.1"    },
		{ CPU_FEATURE_SSE4_2,   "4.2"    },
		{ CPU_FEATURE_SSE4A,    "4A"     },
		{ CPU_FEATURE_SSE,      ")"      },
		{ CPU_FEATURE_XOP,      "XOP"    },
		{ CPU_FEATURE_AVX,      "AVX(1"  },
		{ CPU_FEATURE_AVX2,      "2"     },
		{ CPU_FEATURE_AVX512F,  "512"    },
		{ CPU_FEATURE_AVX,      ")"      },
		{ CPU_FEATURE_FMA3,     "FMA(3"  },
		{ CPU_FEATURE_FMA4,     "4"      },
		{ CPU_FEATURE_FMA3,     ")"      },
		/* Security and Cryptography */
		{ CPU_FEATURE_AES,      "AES"    },
		{ CPU_FEATURE_PCLMUL,   "CLMUL"  },
		{ CPU_FEATURE_RDRAND,   "RdRand" },
		{ CPU_FEATURE_SHA_NI,   "SHA"    },
		{ CPU_FEATURE_SGX,      "SGX"    },
		/* Virtualization */
		{ CPU_FEATURE_VMX,      "VT-x"   },
		{ CPU_FEATURE_SVM,      "AMD-V"  },
		/* Other */
		{ CPU_FEATURE_LM,       "x86-64" },
		{ NUM_CPU_FEATURES,     NULL     }
	};

	/* Call libcpuid */
	MSG_VERBOSE("%s", _("Calling libcpuid for retrieving static data"));
	if(Options::get_issue())
		cpuid_set_verbosiness_level(2);
	if(data.cpu.cpuid_raw_file == NULL)
		err = cpuid_get_all_raw_data(&raw_data);
	else
		err = cpuid_deserialize_all_raw_data(&raw_data, data.cpu.cpuid_raw_file);
	if(Options::get_issue())
	{
		cpuid_serialize_all_raw_data(&raw_data, "");
		if(DAEMON_UP) call_libcpuid_msr_debug(data, raw_data.num_raw);
	}

	if(err || cpu_identify_all(&raw_data, &system_id))
	{
		MSG_ERROR(_("failed to call libcpuid (%s)"), cpuid_error());
		return 1;
	}
	cpuid_free_raw_data_array(&raw_data);

	/* Basically fill CPU tab */
	for(uint8_t cpu_type = 0; cpu_type < system_id.num_cpu_types; cpu_type++)
	{
		cpu_id = &system_id.cpu_types[cpu_type];
		cpu_purpose = cpu_purpose_str(cpu_id->purpose);
		data.cpu.grow_cpu_types_vector(cpu_type, cpu_purpose);
		data.caches.grow_cpu_types_vector(cpu_type, cpu_purpose);

		/* Trivial assignments */
		data.cpu.vendor                                            = cpu_id->vendor;
		data.cpu.ext_family                                        = cpu_id->ext_family;
		data.cpu.cpu_types[cpu_type].purpose                       = cpu_id->purpose;
		data.cpu.cpu_types[cpu_type].processor.vendor.value        = cpuvendors.at(cpu_id->vendor);
		data.cpu.cpu_types[cpu_type].processor.codename.value      = cpu_id->cpu_codename;
		data.cpu.cpu_types[cpu_type].processor.specification.value = cpu_id->brand_str;
		data.cpu.cpu_types[cpu_type].processor.family.value        = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->family);
		data.cpu.cpu_types[cpu_type].processor.dispfamily.value    = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->ext_family);
		data.cpu.cpu_types[cpu_type].processor.model.value         = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->model);
		data.cpu.cpu_types[cpu_type].processor.dispmodel.value     = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->ext_model);
		data.cpu.cpu_types[cpu_type].processor.stepping.value      = std::to_string(cpu_id->stepping);
		data.cpu.cpu_types[cpu_type].footer.cores.value            = std::to_string(cpu_id->num_cores);
		data.cpu.cpu_types[cpu_type].footer.threads.value          = std::to_string(cpu_id->num_logical_cpus);
		data.cpu.cpu_types[cpu_type].footer.num_threads            = cpu_id->num_logical_cpus;
		string_trim(data.cpu.cpu_types[cpu_type].processor.specification.value);

		/* Add core offset */
		data.cpu.cpu_types[cpu_type].footer.core_id_offset = core_id_offset;
		core_id_offset = cpu_id->num_logical_cpus;

		/* Search in DB for CPU technology (depends on CPU vendor) */
		err += cpu_technology(data.cpu.cpu_types[cpu_type].processor, cpu_id);

		/* Cache level 1 (instruction) */
		if(cpu_id->l1_instruction_cache > 0)
			data.cpu.cpu_types[cpu_type].caches.level1i.value = Data::Cpu::CpuType::Caches::format_cache_level(cpu_id->l1_instruction_instances, cpu_id->l1_instruction_cache, UNIT_KB, cpu_id->l1_instruction_assoc);

		/* Cache level 1 (data) */
		if(cpu_id->l1_data_cache > 0)
		{
			data.cpu.cpu_types[cpu_type].caches.level1d.value = Data::Cpu::CpuType::Caches::format_cache_level(cpu_id->l1_data_instances, cpu_id->l1_data_cache, UNIT_KB, cpu_id->l1_data_assoc);
			data.caches.cpu_types[cpu_type].grow_caches_vector_with_cache_size(1, cpu_id->l1_data_cache, data.cpu.cpu_types[cpu_type].caches.level1d.value.c_str(), cpu_id->l1_data_cacheline, UNIT_B);
		}

		/* Cache level 2 */
		if(cpu_id->l2_cache > 0)
		{
			data.cpu.cpu_types[cpu_type].caches.level2.value = Data::Cpu::CpuType::Caches::format_cache_level(cpu_id->l2_instances, cpu_id->l2_cache, UNIT_KB, cpu_id->l2_assoc);
			data.caches.cpu_types[cpu_type].grow_caches_vector_with_cache_size(2, cpu_id->l2_cache, data.cpu.cpu_types[cpu_type].caches.level2.value.c_str(), cpu_id->l2_cacheline, UNIT_B);
		}

		/* Cache level 3 */
		if(cpu_id->l3_cache > 0)
		{
			data.cpu.cpu_types[cpu_type].caches.level3.value = Data::Cpu::CpuType::Caches::format_cache_level(cpu_id->l3_instances, cpu_id->l3_cache >> 10, UNIT_MB, cpu_id->l3_assoc);
			data.caches.cpu_types[cpu_type].grow_caches_vector_with_cache_size(3, cpu_id->l3_cache, data.cpu.cpu_types[cpu_type].caches.level3.value.c_str(), cpu_id->l3_cacheline, UNIT_B);
		}

		/* Cache level 4 */
		if(cpu_id->l4_cache > 0)
		{
			std::string l4size_str = Data::Cpu::CpuType::Caches::format_cache_level(cpu_id->l4_instances, cpu_id->l4_cache >> 10, UNIT_MB, cpu_id->l4_assoc);
			data.caches.cpu_types[cpu_type].grow_caches_vector_with_cache_size(4, cpu_id->l4_cache, l4size_str.c_str(), cpu_id->l4_cacheline, UNIT_B);
		}

		/* Add string "HT" in CPU Intructions label (if enabled) */
		if(cpu_id->num_cores < cpu_id->num_logical_cpus)
			data.cpu.cpu_types[cpu_type].processor.instructions.value = (cpu_id->vendor == VENDOR_INTEL) ? "HT" : "SMT";

		/* Fill CPU Intructions label */
		for(uint16_t i = 0; cpu_flags[i].flag != NUM_CPU_FEATURES; i++)
		{
			if(!cpu_id->flags[cpu_flags[i].flag])
				continue;

			if((data.cpu.cpu_types[cpu_type].processor.instructions.value.length() > 0) && (cpu_flags[i].str[0] != '(') && (cpu_flags[i].str[0] != ')'))
				data.cpu.cpu_types[cpu_type].processor.instructions.value += ", ";
			data.cpu.cpu_types[cpu_type].processor.instructions.value += cpu_flags[i].str;
		}
	}
	Options::set_selected_type(Options::get_selected_type(), system_id.num_cpu_types);
	Options::set_selected_core(Options::get_selected_core(), data.cpu.get_selected_cpu_type().footer.num_threads);
	cpuid_free_system_id(&system_id);

	return err;
}

/* Dynamic elements provided by libcpuid */
static int call_libcpuid_dynamic(Data &data)
{
	/* CPU frequency */
	MSG_VERBOSE("%s", _("Calling libcpuid for retrieving dynamic data"));
	data.cpu.clocks.set_cpu_freq(cpu_clock());

	return (data.cpu.clocks.cpu_freq <= 0);
}

/* MSRs static values provided by libcpuid */
static int call_libcpuid_msr_static(Data &data)
{
	const DaemonCommand cmd = LIBCPUID_MSR_STATIC;
	const uint16_t current_core_id = data.cpu.get_selected_core_id();
	MsrStaticData msg;

	MSG_VERBOSE("%s", _("Calling libcpuid for retrieving CPU MSR static values"));
	SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));
	SEND_DATA(&data.socket_fd,  &current_core_id, sizeof(current_core_id));
	RECEIVE_DATA(&data.socket_fd, &msg, sizeof(MsrStaticData));

	/* CPU Multipliers (minimum & maximum) */
	if((msg.min_mult != CPU_INVALID_VALUE) && (msg.max_mult != CPU_INVALID_VALUE))
	{
		data.cpu.clocks.cpu_min_mult = double(msg.min_mult) / 100.0;
		data.cpu.clocks.cpu_max_mult = double(msg.max_mult) / 100.0;
	}

	/* Base clock */
	if((msg.bclk != CPU_INVALID_VALUE) && (data.cpu.clocks.bus_freq == 0.0))
		data.cpu.clocks.set_bus_freq(double(msg.bclk) / 100.0);

	return 0;
}

/* MSRs dynamic values provided by libcpuid */
static int call_libcpuid_msr_dynamic(Data &data)
{
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const uint16_t current_core_id = data.cpu.get_selected_core_id();
	const DaemonCommand cmd = LIBCPUID_MSR_DYNAMIC;
	MsrDynamicData msg;

	MSG_VERBOSE("%s", _("Calling libcpuid for retrieving CPU MSR dynamic values"));
	SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));
	SEND_DATA(&data.socket_fd,  &current_core_id, sizeof(current_core_id));
	RECEIVE_DATA(&data.socket_fd, &msg, sizeof(MsrDynamicData));

	/* CPU Voltage */
	if(msg.voltage != CPU_INVALID_VALUE)
		cpu_type.processor.voltage.value = string_format("%.3f V", double(msg.voltage) / 100.0);

	/* CPU Temperature */
	if(msg.temp != CPU_INVALID_VALUE)
		cpu_type.processor.temperature.value = string_format_temperature_unit("%i", msg.temp);

	return 0;
}
#endif /* HAS_LIBCPUID */

#if HAS_DMIDECODE
/* Call Dmidecode through CPU-X but do nothing else */
int run_dmidecode(void)
{
	return dmidecode(Logger::get_verbosity() > LOG_VERBOSE, NULL);
}

/* Elements provided by dmidecode */
static int call_dmidecode(Data &data)
{
	const DaemonCommand cmd = DMIDECODE;
	DmidecodeData msg;
	DmidecodeMemoryData msg_memory;
	bool need_sep = false;

	MSG_VERBOSE("%s", _("Calling dmidecode"));
	SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));

	RECEIVE_DATA(&data.socket_fd, &msg.ret, sizeof(int));
	if(msg.ret)
		return 1;

	/* Tab CPU */
	RECEIVE_DATA(&data.socket_fd, &msg.processor, sizeof(DmidecodeCPUData));
	for(auto& cpu_type : data.cpu.cpu_types)
		cpu_type.processor.package.value = msg.processor.cpu_package;
	if(data.cpu.clocks.bus_freq == 0.0)
		data.cpu.clocks.set_bus_freq(double(msg.processor.bus_freq));

	/* Tab Motherboard */
	RECEIVE_DATA(&data.socket_fd, &msg.mb, sizeof(DmidecodeMBData));
	data.motherboard.board.manufacturer.value = msg.mb.manufacturer;
	data.motherboard.board.model.value        = msg.mb.model;
	data.motherboard.board.revision.value     = msg.mb.revision;
	RECEIVE_DATA(&data.socket_fd, &msg.bios, sizeof(DmidecodeBiosData));
	data.motherboard.bios.brand.value         = msg.bios.brand;
	data.motherboard.bios.version.value       = msg.bios.version;
	data.motherboard.bios.date.value          = msg.bios.date;
	data.motherboard.bios.romsize.value       = string_set_size_unit(msg.bios.romsize);

	/* Tab RAM */
	RECEIVE_DATA(&data.socket_fd, &msg.stick_count, sizeof(uint8_t));
	for(uint8_t i = 0; i < msg.stick_count; i++)
	{
		RECEIVE_DATA(&data.socket_fd, &msg_memory, sizeof(DmidecodeMemoryData));
		data.memory.grow_sticks_vector();
		data.memory.sticks[i].manufacturer.value   = msg_memory.manufacturer;
		data.memory.sticks[i].part_number.value    = msg_memory.part_number;
		data.memory.sticks[i].type.value           = msg_memory.type;
		data.memory.sticks[i].type_detail.value    = msg_memory.type_detail;
		data.memory.sticks[i].device_locator.value = msg_memory.device_locator;
		data.memory.sticks[i].bank_locator.value   = msg_memory.bank_locator;
		data.memory.sticks[i].size.value           = string_set_size_unit(msg_memory.size);
		data.memory.sticks[i].rank.value           = msg_memory.rank;
		need_sep = false;
		if(std::strlen(msg_memory.speed_configured) > 0)
		{
			data.memory.sticks[i].speed.value = string_format(_("%s (configured)"), msg_memory.speed_configured);
			need_sep = true;
		}
		if(std::strlen(msg_memory.speed_maximum) > 0)
		{
			if(need_sep)
				data.memory.sticks[i].speed.value += " / ";
			data.memory.sticks[i].speed.value += string_format(_("%s (max)"), msg_memory.speed_maximum);
		}
		need_sep = false;
		if(std::strlen(msg_memory.voltage_minimum) > 0)
		{
			data.memory.sticks[i].voltage.value = string_format(_("%s (min)"), msg_memory.voltage_minimum);
			need_sep = true;
		}
		if(std::strlen(msg_memory.voltage_configured) > 0)
		{
			if(need_sep)
				data.memory.sticks[i].voltage.value += " / ";
			data.memory.sticks[i].voltage.value += string_format(_("%s (configured)"), msg_memory.voltage_configured);
			need_sep = true;
		}
		if(std::strlen(msg_memory.voltage_maximum) > 0)
		{
			if(need_sep)
				data.memory.sticks[i].voltage.value += " / ";
			data.memory.sticks[i].voltage.value += string_format(_("%s (max)"), msg_memory.voltage_maximum);
		}
	}

	return 0;
}
#endif /* HAS_DMIDECODE */

/* Calculate total CPU usage */
#define BEFORE data.cpu.clocks.cpu_time_stat
static int cpu_usage(Data &data)
{
	double loadavg;
	enum StatType { USER, NICE, SYSTEM, INTR, IDLE, LASTSTAT };
	std::vector<long> after(LASTSTAT);

	MSG_VERBOSE("%s", _("Calculating CPU usage"));
	BEFORE.resize(LASTSTAT);

#ifdef __linux__
	FILE *fp;

	if((fp = fopen("/proc/stat","r")) == NULL)
		return 1;
	fscanf(fp,"%*s %li %li %li %li %*s %*s %*s %*s %*s %*s", &after[USER], &after[NICE], &after[SYSTEM], &after[IDLE]);
	fclose(fp);
#else /* __linux__ */
	size_t len = sizeof(long) * after.size();

	if(sysctlbyname("kern.cp_time", after.data(), &len, NULL, 0))
		return 1;
#endif /* __linux__ */

	loadavg = double((after[USER]  + after[NICE]  + after[SYSTEM]  + after[INTR]) -
	                 (BEFORE[USER] + BEFORE[NICE] + BEFORE[SYSTEM] + BEFORE[INTR])) /
	          double((after[USER]  + after[NICE]  + after[SYSTEM]  + after[INTR]  + after[IDLE]) -
	                 (BEFORE[USER] + BEFORE[NICE] + BEFORE[SYSTEM] + BEFORE[INTR] + BEFORE[IDLE]));
	data.cpu.clocks.usage.value = string_format("%6.2f %%", loadavg * 100);
	BEFORE.swap(after);

	return 0;
}
#undef BEFORE

#if HAS_BANDWIDTH
/* Call Bandwidth through CPU-X but do nothing else */
int run_bandwidth(void)
{
	const char *argv[] = { NULL, "--fastest", NULL };
	return bandwidth_main(2, argv);
}

/* Compute CPU cache speed */
static int call_bandwidth([[maybe_unused]] Data &data)
{
	int err = 0;
#if HAS_LIBCPUID
	static bool first = true;
	static struct BandwidthData bwd{};

	if(data.caches.get_selected_cpu_type().caches.size() == 0)
		return 1;

	for(std::size_t i = 0; i < data.caches.get_selected_cpu_type().caches.size(); i++)
		bwd.cache_size[i] = data.caches.get_selected_cpu_type().caches[i].size_i;

	MSG_VERBOSE("%s", _("Calling bandwidth"));
	if(first)
	{
		/* Init BandwidthData */
		bwd.is_amd_cpu    = (data.cpu.vendor == VENDOR_AMD),
		bwd.selected_test = Options::get_selected_test(),
		bwd.test_name     = NULL,
		pthread_mutex_init(&bwd.mutex, NULL);

		/* Call bandwidth */
		err   = bandwidth_cpux(&bwd);
		first = false;

		/* Copy test names */
		for(uint8_t i = 0; i < BANDWIDTH_LAST_TEST; i++)
		{
			data.caches.test.names.push_back(bwd.test_name[i]);
			free(bwd.test_name[i]);
		}
		free(bwd.test_name);
	}
	else
	{
		/* Run bandwidth in a separated thread if not running */
		if(pthread_mutex_trylock(&bwd.mutex) != EBUSY)
		{
			pthread_mutex_unlock(&bwd.mutex);
			std::thread bwt(bandwidth_cpux, &bwd);
			MSG_DEBUG("%s", "call_bandwidth: created new thread");
			bwt.detach();
		}
		else
			MSG_DEBUG("%s", "call_bandwidth: a previous thread is still running");
	}

	/* Speed labels */
	for(std::size_t i = 0; i < data.caches.get_selected_cpu_type().caches.size(); i++)
		data.caches.get_selected_cpu_type().caches[i].speed.value = string_format("%.2f %s/s", double(bwd.cache_speed[i]) / 10.0, UNIT_MB);
#endif /* HAS_LIBCPUID */

	return err;
}
#endif /* HAS_BANDWIDTH */

#if HAS_LIBPCI
/* Check is GPU is enabled */
static bool gpu_is_on([[maybe_unused]] std::string device_path)
{
	bool ret = true;
#ifdef __linux__
	ret = std::filesystem::is_directory(device_path);
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

	/* Check for discrete GPU */
	switch(it->second)
	{
		case GpuDrv::GPUDRV_NVIDIA:
		case GpuDrv::GPUDRV_NOUVEAU:
			if(command_exists("optirun") && !popen_to_str(cmd, "optirun --status") && (cmd.find("Bumblebee status: Ready") != std::string::npos))
				card.driver = (it->second == GpuDrv::GPUDRV_NVIDIA) ? GpuDrv::GPUDRV_NVIDIA_BUMBLEBEE : GpuDrv::GPUDRV_NOUVEAU_BUMBLEBEE;
			break;
		default:
			card.driver = it->second;
			break;
	}

	return 0;
}

static int set_gpu_user_mode_driver([[maybe_unused]] Data::Graphics::Card &card)
{
	int err = 0;

#if HAS_LIBGLFW
	const char *description;
	size_t umd_index = std::string::npos;
	std::string gl_ver, glsl_ver;
	GLFWwindow *win = NULL;

	if(glfwInit() == GLFW_FALSE)
	{
		err = glfwGetError(&description);
		goto clean;
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	if((err = glfwGetError(&description)) != GLFW_NO_ERROR)
		goto clean;

	win = glfwCreateWindow(640, 480, "", NULL, NULL);
	if((err = glfwGetError(&description)) != GLFW_NO_ERROR)
		goto clean;

	glfwMakeContextCurrent(win);
	if((err = glfwGetError(&description)) != GLFW_NO_ERROR)
		goto clean;

	gl_ver = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	glsl_ver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

	if(gl_ver.empty() || glsl_ver.empty())
	{
		err = glGetError() != GL_NO_ERROR ? static_cast<int>(glGetError()) : -1;
		description = "glGetString";
		goto clean;
	}
	card.opengl_version.value = glsl_ver;

	switch(card.driver)
	{
		case GpuDrv::GPUDRV_AMDGPU:
		case GpuDrv::GPUDRV_INTEL:
		case GpuDrv::GPUDRV_RADEON:
		case GpuDrv::GPUDRV_NOUVEAU:
		case GpuDrv::GPUDRV_NOUVEAU_BUMBLEBEE:
			umd_index = gl_ver.find("Mesa");
			break;
		case GpuDrv::GPUDRV_NVIDIA:
			umd_index = gl_ver.find("NVIDIA");
			break;
		default:
			break;
	}

	if(umd_index != std::string::npos)
		card.user_mode_driver.value = gl_ver.substr(umd_index);
	else
		MSG_WARNING(_("Your GPU user mode driver is unknown: %s"), gl_ver.c_str());

clean:
	if(err)
		MSG_ERROR(_("failed to call GLFW (%i): %s"), err, description);
	if(win != NULL)
		glfwDestroyWindow(win);
#endif /* HAS_LIBGLFW */

	return err;
}

#if HAS_Vulkan
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
		case VK_ERROR_FRAGMENTATION:
			return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_FRAGMENTED_POOL:
			return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
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
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
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
		case VK_ERROR_UNKNOWN:
			return "VK_ERROR_UNKNOWN";
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
#endif /* HAS_Vulkan */

static int set_gpu_vulkan_version([[maybe_unused]] Data::Graphics::Card &card, [[maybe_unused]] struct pci_dev *dev)
{
#if HAS_Vulkan
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
	createInfo.enabledExtensionCount   = (uint32_t) ext_create_info.size();
	createInfo.ppEnabledExtensionNames = ext_create_info.data();

	if((vk_err = vkCreateInstance(&createInfo, NULL, &instance)) != VK_SUCCESS)
	{
		MSG_ERROR(_("failed to call vkCreateInstance (%s)"), string_VkResult(vk_err));

		if(vk_err == VK_ERROR_EXTENSION_NOT_PRESENT)
			MSG_ERROR(_("%s is not supported"), VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		return 1;
	}

	/* Get number of devices */
	if((vk_err = vkEnumeratePhysicalDevices(instance, &device_count, NULL)) != VK_SUCCESS)
	{
		MSG_ERROR(_("failed to call vkEnumeratePhysicalDevices (%s)"), string_VkResult(vk_err));
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

# ifdef VK_EXT_PCI_BUS_INFO_EXTENSION_NAME
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
# endif /* VK_EXT_PCI_BUS_INFO_EXTENSION_NAME */

# ifdef VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
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
# endif /* VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME */
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
# ifdef VK_EXT_PCI_BUS_INFO_EXTENSION_NAME
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
# else
		use_device_id = true;
# endif /* VK_EXT_PCI_BUS_INFO_EXTENSION_NAME */
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

# ifdef VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
		VkDevice vk_dev_rt{};
		card.vulkan_rt.value = (vkCreateDevice(devices[i], &check_rt, NULL, &vk_dev_rt) == VK_SUCCESS) ? _("Enabled") : _("Disabled");
		MSG_DEBUG("Vulkan device %lu: Ray Tracing support is %s", i, card.vulkan_rt.value.c_str());
		vkDestroyDevice(vk_dev_rt, NULL);
# endif /* VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME */

		card.vulkan_version.value = string_format("%d.%d.%d",
# if(VK_API_VERSION_MAJOR && VK_API_VERSION_MINOR && VK_API_VERSION_PATCH)
			VK_API_VERSION_MAJOR(prop2.properties.apiVersion),
			VK_API_VERSION_MINOR(prop2.properties.apiVersion),
			VK_API_VERSION_PATCH(prop2.properties.apiVersion)
# else
			VK_VERSION_MAJOR(prop2.properties.apiVersion),
			VK_VERSION_MINOR(prop2.properties.apiVersion),
			VK_VERSION_PATCH(prop2.properties.apiVersion)
# endif /* (VK_API_VERSION_MAJOR && VK_API_VERSION_MINOR && VK_API_VERSION_PATCH) */
		);
		MSG_DEBUG("Vulkan device %lu: version is '%s'", i, card.vulkan_version.value.c_str());
		gpu_found = true;
	}
	vkDestroyInstance(instance, NULL);
#endif /* HAS_Vulkan */

	return 0;
}

#define CLINFO(dev_id, PARAM, prop) \
	clGetDeviceInfo(dev_id, PARAM, sizeof(prop), &prop, NULL)
static int set_gpu_compute_unit([[maybe_unused]] Data::Graphics::Card &card, [[maybe_unused]] struct pci_dev *dev)
{
	int ret_cl = 0;

#if HAS_OpenCL
	bool gpu_found = false;
	size_t size_ret;
	char platform_name[MAXSTR] = "", platform_version[MAXSTR] = "";
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

		ret_cl = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, &size_ret); // get platform name
		if((ret_cl != CL_SUCCESS) || (size_ret == 0))
		{
			MSG_ERROR(_("failed to get name for platform %u (%s)"), i, opencl_error(ret_cl));
			continue;
		}
		MSG_DEBUG("OpenCL platform %u: name is '%s'", i, platform_name);

		ret_cl = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(platform_version), platform_version, &size_ret); // get platform version
		if((ret_cl != CL_SUCCESS) || (size_ret == 0))
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
			char device_name[MAXSTR] = "", device_version[MAXSTR] = "";
			MSG_DEBUG("Looping into OpenCL platform %u, device %u", i, j);

			CLINFO(devices[j], CL_DEVICE_VENDOR_ID, ocl_vendor);
			if(dev->vendor_id != ocl_vendor)
				continue;
			MSG_DEBUG("OpenCL platform %u, device %u: found vendor 0x%X", i, j, ocl_vendor);

			ret_cl = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(device_name), device_name, &size_ret);
			if((ret_cl != CL_SUCCESS) || (size_ret == 0))
			{
				MSG_ERROR(_("failed to get name for device %u (%s)"), j, opencl_error(ret_cl));
				continue;
			}
			MSG_DEBUG("OpenCL platform %u, device %u: name is '%s'", i, j, device_name);

			ret_cl = clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, sizeof(device_version), device_version, &size_ret);
			if((ret_cl != CL_SUCCESS) || (size_ret == 0))
			{
				MSG_ERROR(_("failed to get version for device %u (%s)"), j, opencl_error(ret_cl));
				continue;
			}
			MSG_DEBUG("OpenCL platform %u, device %u: version is '%s'", i, j, device_version);

			/* Set OpenCL version */
			const std::string cl_version = device_version;
			const size_t cl_index = cl_version.find_last_of("OpenCL");
			card.opencl_version.value = (cl_index != std::string::npos) ? cl_version.substr(cl_index + 2) : cl_version;

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
#endif /* HAS_OpenCL */

	return ret_cl;
}

#define DEVICE_VENDOR_STR(d)  pci_lookup_name(pacc, buff, MAXSTR, PCI_LOOKUP_VENDOR, d->vendor_id, d->device_id)
#define DEVICE_PRODUCT_STR(d) pci_lookup_name(pacc, buff, MAXSTR, PCI_LOOKUP_DEVICE, d->vendor_id, d->device_id)
/* Find some PCI devices, like chipset and GPU */
static int find_devices(Data &data)
{
	/* Adapted from http://git.kernel.org/cgit/utils/pciutils/pciutils.git/tree/example.c */
	bool chipset_found = false;
	std::string gpu_vendor;
	char buff[MAXSTR] = "";
	struct pci_access *pacc;
	struct pci_dev *dev;

	MSG_VERBOSE("%s", _("Finding devices"));
	pacc = pci_alloc(); /* Get the pci_access structure */
#ifdef __FreeBSD__
	int ret = -1;
	const DaemonCommand cmd = ACCESS_DEV_PCI;
	if(DAEMON_UP && access(DEV_PCI, W_OK))
	{
		SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));
		RECEIVE_DATA(&data.socket_fd, &ret, sizeof(int));
	}
	if(ret && access(DEV_PCI, W_OK))
	{
		MSG_WARNING(_("Skip devices search (wrong permissions on %s device)"), DEV_PCI);
		return 1;
	}
#endif /* __FreeBSD__ */
	pci_init(pacc);	    /* Initialize the PCI library */
	pci_scan_bus(pacc); /* We want to get the list of devices */

	/* Iterate over all devices */
	for(dev = pacc->devices; dev != NULL; dev = dev->next)
	{
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);

		/* Looking for chipset */
		if(!chipset_found && (dev->device_class == PCI_CLASS_BRIDGE_ISA))
		{
			chipset_found                         = true;
			data.motherboard.chipset.vendor.value = DEVICE_VENDOR_STR(dev);
			data.motherboard.chipset.model.value  = DEVICE_PRODUCT_STR(dev);
		}

		/* Looking for GPU */
		if((dev->device_class >> 8) == PCI_BASE_CLASS_DISPLAY)
		{
			uint64_t bar_size = 0;
			const uint8_t card_index = data.graphics.cards.size();
			data.graphics.grow_cards_vector();

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

			data.graphics.cards[card_index].device_path     = get_gpu_device_path(dev);
			data.graphics.cards[card_index].vendor.value    = gpu_vendor;
			data.graphics.cards[card_index].model.value     = DEVICE_PRODUCT_STR(dev);
			data.graphics.cards[card_index].device_id.value = string_format("0x%04X:0x%02X", dev->device_id, pci_read_byte(dev, PCI_REVISION_ID));
			set_gpu_kernel_driver(data.graphics.cards[card_index]);
			set_gpu_user_mode_driver(data.graphics.cards[card_index]);
			set_gpu_vulkan_version(data.graphics.cards[card_index], dev);
			set_gpu_compute_unit(data.graphics.cards[card_index], dev);
			if(data.graphics.cards[card_index].vram_size > 0)
				data.graphics.cards[card_index].mem_used.value = string_format("??? / %lu %s", (data.graphics.cards[card_index].vram_size >> 20), UNIT_MIB);
			if((data.graphics.cards[card_index].vram_size > 0) && (bar_size > 0))
				data.graphics.cards[card_index].resizable_bar.value = ((data.graphics.cards[card_index].vram_size * 9 / 10) < bar_size) ? _("Enabled") : _("Disabled");
		}
	}

	pci_cleanup(pacc);
	if(!chipset_found)
		MSG_ERROR("%s", _("failed to find chipset vendor and model"));
	if(data.graphics.cards.size() == 0)
		MSG_ERROR("%s", _("failed to find graphic card vendor and model"));
	else
		Options::set_selected_gpu(Options::get_selected_gpu(), data.graphics.cards.size());

#if 0 // For testing purposes
	while(data.graphics.cards.size() < 8)
	{
		const uint8_t card_index = data.graphics.cards.size();
		data.graphics.grow_cards_vector();
		data.graphics.cards[card_index].vendor.value           = string_format("Vendor %u", card_index);
		data.graphics.cards[card_index].kernel_driver.value    = string_format("Driver %u", card_index);
		data.graphics.cards[card_index].user_mode_driver.value = string_format("UMB %u", card_index);
		data.graphics.cards[card_index].model.value            = string_format("Model %u", card_index);
	}
#endif /* 0 */

	return (chipset_found == false) + (data.graphics.cards.size() == 0);
}
#undef DEVICE_VENDOR_STR
#undef DEVICE_PRODUCT_STR

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
	std::string pcie_speed_raw, pcie_width_raw;
	// Linux 4.13+ (3 September 2017)
	const std::string link_speed_file = drm_path + "/device/" + type + "_link_speed";
	const std::string link_width_file = drm_path + "/device/" + type + "_link_width";

	if(access(link_speed_file.c_str(), F_OK) || access(link_width_file.c_str(), F_OK))
		return std::string();

	if(fopen_to_str(pcie_speed_raw, "%s", link_speed_file.c_str()) || fopen_to_str(pcie_width_raw, "%s", link_width_file.c_str()))
		return std::string();

	try
	{
		/* Speed */
		switch(std::stoi(pcie_speed_raw))
		{
			case  2: pcie_gen = 1; break; // 2.5 GT/s
			case  5: pcie_gen = 2; break;
			case  8: pcie_gen = 3; break;
			case 16: pcie_gen = 4; break;
			case 32: pcie_gen = 5; break;
			case 64: pcie_gen = 6; break;
			default: pcie_gen = 0; break;
		}

		/* Width */
		pcie_width = std::stoi(pcie_width_raw);
	}
	catch(const std::exception& e)
	{
		return std::string();
	}

	return string_format("Gen%1dx%d", pcie_gen, pcie_width);
}
#endif /* __linux__ */

#define FOPEN_TO_ITEM(item, ...)        item.ret = fopen_to_str(item.value, ##__VA_ARGS__)
#define POPEN_TO_ITEM(item, ...)        item.ret = popen_to_str(item.value, ##__VA_ARGS__)
#define POPEN_DRI_TO_ITEM(item, ...)    item.ret = can_access_sys_debug_dri(data) ? popen_to_str(item.value, ##__VA_ARGS__) : -1
#define SET_LABEL_VALUE(item, fmt, ...) item_count++; \
                                        if(!item.ret) card.item.value = string_format(fmt, ##__VA_ARGS__); \
                                        else error_count++
/* Retrieve GPU temperature and clocks */
static int gpu_monitoring([[maybe_unused]] Data &data)
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
				const std::string amdgpu_gpu_busy_file = (!card.drm_path.empty()) ? card.drm_path + "/device/gpu_busy_percent" : std::string();
				MSG_DEBUG("gpu_monitoring: amdgpu: amdgpu_gpu_busy_file=%s", amdgpu_gpu_busy_file.c_str());
				FOPEN_TO_ITEM(vbios_version, "%s/device/vbios_version",        card.drm_path.c_str());
				// temperature obtained above
				FOPEN_TO_ITEM(usage,         "%s", amdgpu_gpu_busy_file.c_str()); // Linux 4.19+ (22 October 2018)
				FOPEN_TO_ITEM(core_voltage,  "%s/in0_input",                   card.hwmon_path.c_str());
				FOPEN_TO_ITEM(power_avg,     "%s/power1_average",              card.hwmon_path.c_str());
				FOPEN_TO_ITEM(core_clock,    "%s/freq1_input",                 card.hwmon_path.c_str());
				FOPEN_TO_ITEM(mem_clock,     "%s/freq2_input",                 card.hwmon_path.c_str());
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
		SET_LABEL_VALUE(vbios_version, "%s",                  vbios_version.value.c_str());
		SET_LABEL_VALUE(temperature,   "%s",                  string_format_temperature_unit("%.2f", std::stoull(temperature.value) / temperature.divisor).c_str());
		SET_LABEL_VALUE(usage,         "%s%%",                usage.value.c_str());
		SET_LABEL_VALUE(core_voltage,  "%.2Lf V",             std::stoull(core_voltage.value) / core_voltage.divisor);
		SET_LABEL_VALUE(power_avg,     "%.2Lf W",             std::stoull(power_avg.value)    / power_avg.divisor);
		if (std::stoull(core_clock.value) > 0){
                 SET_LABEL_VALUE(core_clock,    "%.0Lf MHz",           std::stoull(core_clock.value)   / core_clock.divisor);
                 }
		SET_LABEL_VALUE(mem_clock,     "%.0Lf MHz",           std::stoull(mem_clock.value)    / mem_clock.divisor);
		SET_LABEL_VALUE(mem_used,      "%.0Lf %s / %.0Lf %s", std::stoull(mem_used.value)     / mem_used.divisor, UNIT_MIB,
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
#undef SET_LABEL_VALUE
#endif /* HAS_LIBPCI */

/* Satic elements for System tab, OS specific */
static int system_static(Data &data)
{
	int err = 0;
	struct utsname name;

	MSG_VERBOSE("%s", _("Identifying running system"));
	err = uname(&name);
	if(err)
		MSG_ERRNO("%s", _("failed to identify running system"));
	else
		data.system.os.hostname.value = Options::get_issue() ? "sensitive data" : name.nodename; /* Hostname label */

#ifdef __linux__
	/* Name label */
	std::string line;
	std::ifstream stream("/etc/os-release");
	std::regex regex("^PRETTY_NAME=\"(.*?)\"$");
	std::smatch match;

	while(std::getline(stream, line))
	{
		if(std::regex_search(line, match, regex))
		{
			data.system.os.name.value = match[1].str();
			break;
		}
	}

	/* Kernel label */
	if(!err)
		data.system.os.kernel.value = string_format("%s %s", name.sysname, name.release);

#else /* __linux__ */
	if(!err)
	{
		/* Name label */
		data.system.os.kernel.value = string_format("%s %s", name.sysname, name.release);

		/* Kernel label */
		data.system.os.kernel.value = string_format("%s", name.version);
	}

#endif /* __linux__ */

	return err;
}

#if HAS_LIBSYSTEM
/* Dynamic elements for System tab, provided by libprocps/libstatgrab */
static int system_dynamic(Data &data)
{
	int err = 0;
	time_t uptime_s = 0;
	struct tm *tm;
	static PrefixUnit pu_mem, pu_swap;

#if HAS_LIBPROC2
	double up_secs;
	struct meminfo_info *mem_info = NULL;

	MSG_VERBOSE("%s", _("Calling libprocps"));
	/* System uptime */
	if(procps_uptime(&up_secs, NULL) < 0)
		MSG_ERRNO("%s", _("unable to get system uptime"));
	else
		uptime_s = (time_t) up_secs;

	/* Memory variables */
	if(procps_meminfo_new(&mem_info) < 0)
		MSG_ERRNO("%s", _("unable to create meminfo structure"));
	else
	{
		if(!pu_mem.init || !pu_swap.init)
		{
			pu_mem.find_best_binary_prefix(MEMINFO_GET(mem_info, MEMINFO_MEM_TOTAL,  ul_int),  PrefixUnit::Multipliers::MULT_K);
			pu_swap.find_best_binary_prefix(MEMINFO_GET(mem_info, MEMINFO_SWAP_TOTAL, ul_int), PrefixUnit::Multipliers::MULT_K);
		}
		data.system.memory.mem_used    = MEMINFO_GET(mem_info, MEMINFO_MEM_USED,       ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_buffers = MEMINFO_GET(mem_info, MEMINFO_MEM_BUFFERS,    ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_cached  = MEMINFO_GET(mem_info, MEMINFO_MEM_CACHED_ALL, ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_free    = MEMINFO_GET(mem_info, MEMINFO_MEM_FREE,       ul_int) / (long double) pu_mem.divisor;
		data.system.memory.mem_total   = MEMINFO_GET(mem_info, MEMINFO_MEM_TOTAL,      ul_int) / (long double) pu_mem.divisor;
		data.system.memory.swap_used   = MEMINFO_GET(mem_info, MEMINFO_SWAP_USED,      ul_int) / (long double) pu_swap.divisor;
		data.system.memory.swap_total  = MEMINFO_GET(mem_info, MEMINFO_SWAP_TOTAL,     ul_int) / (long double) pu_swap.divisor;
		procps_meminfo_unref(&mem_info);
	}
#endif /* HAS_LIBPROC2 */

#if HAS_LIBPROCPS
	MSG_VERBOSE("%s", _("Calling libprocps"));
	/* System uptime */
	uptime_s = (time_t) uptime(NULL, NULL);

	/* Memory variables */
	meminfo();
	if(!pu_mem.init || !pu_swap.init)
	{
		pu_mem.find_best_binary_prefix(kb_main_total,  PrefixUnit::Multipliers::MULT_K);
		pu_swap.find_best_binary_prefix(kb_swap_total, PrefixUnit::Multipliers::MULT_K);
	}
	data.system.memory.mem_used    = kb_main_used    / (long double) pu_mem.divisor;
	data.system.memory.mem_buffers = kb_main_buffers / (long double) pu_mem.divisor;
	data.system.memory.mem_cached  = kb_main_cached  / (long double) pu_mem.divisor;
	data.system.memory.mem_free    = kb_main_free    / (long double) pu_mem.divisor;
	data.system.memory.mem_total   = kb_main_total   / (long double) pu_mem.divisor;
	data.system.memory.swap_used   = kb_swap_used    / (long double) pu_swap.divisor;
	data.system.memory.swap_total  = kb_swap_total   / (long double) pu_swap.divisor;
#endif /* HAS_LIBPROCPS */

#if HAS_LIBSTATGRAB
	static bool called = false;
	sg_mem_stats *mem; /* Memory labels */
	sg_swap_stats *swap;
	sg_host_info *info;

	MSG_VERBOSE("%s", _("Calling libstatgrab"));
	/* Libstatgrab initialization */
	if(!called)
	{
		err += sg_init(0);
		called = true;
	}
	mem  = sg_get_mem_stats(NULL);
	swap = sg_get_swap_stats(NULL);
	info = sg_get_host_info(NULL);

	/* System uptime */
	uptime_s = info->uptime;

	/* Memory variables */
	if(!pu_mem.init || !pu_swap.init)
	{
		pu_mem.find_best_binary_prefix(mem->total,   PrefixUnit::Multipliers::MULT_NONE);
		pu_swap.find_best_binary_prefix(swap->total, PrefixUnit::Multipliers::MULT_NONE);
	}
	data.system.memory.mem_used    = mem->used   / (long double) pu_mem.divisor;
	data.system.memory.mem_buffers = 0;
	data.system.memory.mem_cached  = mem->cache  / (long double) pu_mem.divisor;
	data.system.memory.mem_free    = mem->free   / (long double) pu_mem.divisor;
	data.system.memory.mem_total   = mem->total  / (long double) pu_mem.divisor;
	data.system.memory.swap_used   = swap->used  / (long double) pu_swap.divisor;
	data.system.memory.swap_total  = swap->total / (long double) pu_swap.divisor;
#endif /* HAS_LIBSTATGRAB */

	/* Memory labels */
	if(data.system.memory.mem_total > 0)
	{
		const int int_digits =  std::log10(data.system.memory.mem_total) + 4;
		data.system.memory.used.value    = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_used,    pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
		data.system.memory.buffers.value = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_buffers, pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
		data.system.memory.cached.value  = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_cached,  pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
		data.system.memory.free.value    = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.mem_free,    pu_mem.prefix, int_digits, data.system.memory.mem_total, pu_mem.prefix);
	}
	if(data.system.memory.swap_total > 0)
	{
		const int int_digits = std::log10(data.system.memory.swap_total) + 4;
		data.system.memory.swap.value = string_format("%*.2Lf %s / %*.2Lf %s", int_digits, data.system.memory.swap_used, pu_swap.prefix, int_digits, data.system.memory.swap_total, pu_swap.prefix);
	}

	/* Uptime label */
	if(uptime_s > 0)
	{
		tm = gmtime(&uptime_s);
		data.system.os.uptime.value = string_format(_("%i days, %i hours, %i minutes, %i seconds"), tm->tm_yday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}

	return err;
}
#endif /* HAS_LIBSYSTEM */

/* Compute all prime numbers in 'duration' seconds */
static void primes_bench(Data *data)
{
	uint_fast64_t i, number;
	Data::Bench::PrimeNumbers *bench = data->bench.fast_mode ? static_cast<Data::Bench::PrimeNumbers*>(&data->bench.prime_fast) : static_cast<Data::Bench::PrimeNumbers*>(&data->bench.prime_slow);

	while(data->bench.is_running)
	{
		/* data->bench.number is shared by all threads */
		number = (++bench->number);

		/* Slow mode: loop from i to number, prime if number == i
		   Fast mode: loop from i to sqrt(number), prime if number mod i != 0 */
		const uint_fast64_t sup = data->bench.fast_mode ? sqrt(number) : number;
		for(i = 2; (i < sup) && (number % i != 0); i++);

		if((data->bench.fast_mode && number % i) || (!data->bench.fast_mode && number == i))
			bench->primes++;
	}
}

/* Stop all threads running benchmark */
static void stop_benchmarks(Data *data)
{
	std::time_t start, end;

	/* Wait until the time is up or until user stops benchmark */
	std::time(&start);
	while((data->bench.parameters.elapsed_i < (data->bench.parameters.duration_i * 60)) && data->bench.is_running)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::time(&end);
		data->bench.parameters.elapsed_i = end - start;
	}

	if(data->bench.parameters.elapsed_i >= (data->bench.parameters.duration_i * 60))
		data->bench.is_completed = true;

	/* Exit all threads */
	data->bench.is_running = false;
	for(uint_fast16_t i = 0; i < data->bench.compute_threads.size(); i++)
	{
		data->bench.compute_threads[i].join();
		MSG_DEBUG("stop_benchmarks: stopped thread #%u", i);
	}
	data->bench.compute_threads.clear();
}

/* Report score of benchmarks */
static int benchmark_status(Data &data)
{
	if(!data.bench.did_run)
		return 0;

	Data::Bench::PrimeNumbers *bench = data.bench.fast_mode ? static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_fast) : static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_slow);

	MSG_VERBOSE("%s", _("Updating benchmark status"));
	if(data.bench.is_running)
	{
		bench->state.value = _("Active");
		bench->score.value = std::to_string(bench->primes) + " ";
		if(data.bench.parameters.duration_i * 60 - data.bench.parameters.elapsed_i > 60 * 59)
			bench->score.value += string_format(_("(%u hours left)"), (data.bench.parameters.duration_i - data.bench.parameters.elapsed_i / 60) / 60);
		else if(data.bench.parameters.duration_i * 60 - data.bench.parameters.elapsed_i >= 60)
			bench->score.value += string_format(_("(%u minutes left)"), data.bench.parameters.duration_i - data.bench.parameters.elapsed_i / 60);
		else
			bench->score.value += string_format(_("(%u seconds left)"), data.bench.parameters.duration_i * 60 - data.bench.parameters.elapsed_i);
	}
	else
	{
		bench->state.value = _("Inactive");
		bench->score.value = std::to_string(bench->primes) + " ";
		if(data.bench.parameters.elapsed_i >= 60 * 60)
			bench->score.value += string_format(_("in %u hours"),   data.bench.parameters.elapsed_i / 60 / 60);
		else if(data.bench.parameters.elapsed_i >= 60)
			bench->score.value += string_format(_("in %u minutes"), data.bench.parameters.elapsed_i / 60);
		else
			bench->score.value += string_format(_("in %u seconds"), data.bench.parameters.elapsed_i);
	}

	return 0;
}

/* Perform a multithreaded benchmark (compute prime numbers) */
void start_benchmarks(Data &data)
{
	int err = 0;
	Data::Bench::PrimeNumbers *bench = data.bench.fast_mode ? static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_fast) : static_cast<Data::Bench::PrimeNumbers*>(&data.bench.prime_slow);
#ifdef __FreeBSD__
	cpuset_t cpu_set;
#else
	cpu_set_t cpu_set;
#endif /* __FreeBSD__ */

	MSG_VERBOSE(_("Starting benchmark with %u threads"), data.bench.parameters.threads_i);
	data.bench.did_run              = true;
	data.bench.is_running           = true;
	data.bench.is_completed         = false;
	data.bench.parameters.elapsed_i = 0;
	bench->number                   = 2;
	bench->primes                   = 1;

	/* Start one thread per logical CPU */
	for(uint_fast16_t i = 0; i < data.bench.parameters.threads_i; i++)
	{
		CPU_ZERO(&cpu_set);
		CPU_SET(i, &cpu_set);
		std::thread bench_thread(primes_bench, &data);
		MSG_DEBUG("start_benchmarks: started thread #%u", i);
		err += pthread_setaffinity_np(bench_thread.native_handle(), sizeof(cpu_set), &cpu_set);
		data.bench.compute_threads.push_back(std::move(bench_thread));
	}

	std::thread timer_thread(stop_benchmarks, &data);
	timer_thread.detach();

	if(err)
		MSG_ERROR("%s", _("an error occurred while starting benchmark"));
}

/* Set initial values for benchmarks */
static void init_benchmarks(Data &data)
{
	data.bench.parameters.set_threads(data.bench.parameters.threads_i);
	data.bench.parameters.set_duration(data.bench.parameters.duration_i);
	data.bench.prime_slow.state.value    = _("Inactive");
	data.bench.prime_fast.state.value    = _("Inactive");
}


/************************* Fallback functions (static) *************************/

#if HAS_LIBCPUID
/* If dmidecode fails to find CPU package, check in database */
static int cputab_package_fallback(Data &data)
{
	int i = -1;
	const Package_DB *db;
	const auto& cpu_type = data.cpu.get_selected_cpu_type();

	if(cpu_type.processor.codename.value.empty() || cpu_type.processor.specification.value.empty())
		return 2;

	MSG_VERBOSE("%s", _("Finding CPU package in fallback mode"));
	switch(data.cpu.vendor)
	{
		case VENDOR_AMD:   db = package_amd;     break;
		case VENDOR_INTEL: db = package_intel;   break;
		default:           db = package_unknown; break;
	}

	MSG_DEBUG("cputab_package_fallback: codename %26s, specification %36s => values to find",
	          cpu_type.processor.codename.value.c_str(), cpu_type.processor.specification.value.c_str());
	while(db[++i].socket != NULL)
	{
		const bool codename_defined = (db[i].codename != NULL);
		const bool model_defined    = (db[i].model    != NULL);
		const bool codename_matchs  = codename_defined && (cpu_type.processor.codename.value.find(db[i].codename)   != std::string::npos);
		const bool model_matchs     = model_defined    && (cpu_type.processor.specification.value.find(db[i].model) != std::string::npos);

		if((codename_matchs && model_matchs) || (codename_matchs && !model_defined) || (!codename_defined && model_matchs))
		{
			for(auto& cpu_type : data.cpu.cpu_types)
				cpu_type.processor.package.value = db[i].socket;
			MSG_DEBUG("cputab_package_fallback: codename %26s, specification %36s => entry #%03i matches",
					db[i].codename, db[i].model, i);
			return 0;
		}
		else
			MSG_DEBUG("cputab_package_fallback: codename %26s, specification %36s => entry #%03i does not match",
					db[i].codename, db[i].model, i);
	}

	MSG_WARNING(_("Your CPU socket is not present in the database ==> %s, codename: %s"),
		    cpu_type.processor.specification.value.c_str(), cpu_type.processor.codename.value.c_str());
	for(auto& cpu_type : data.cpu.cpu_types)
		cpu_type.processor.package.value.clear();
	return 2;
}
#endif /* HAS_LIBCPUID */

/* Get minimum and maximum CPU multipliers */
static int cputab_multipliers_fallback(Data &data) noexcept
{
	int err = 0;

	if(data.cpu.clocks.bus_freq <= 0.0)
		return 1;

#ifdef __linux__
	std::string min_freq_str, max_freq_str;
	const uint16_t current_core_id = data.cpu.get_selected_core_id();

	MSG_VERBOSE("%s", _("Calculating CPU multipliers in fallback mode"));
	/* Minimum multiplier */
	if(!(err = fopen_to_str(min_freq_str, "%s%i/cpufreq/cpuinfo_min_freq", SYS_CPU, current_core_id)))
		data.cpu.clocks.cpu_min_mult = (std::stod(min_freq_str) / 1e3) / data.cpu.clocks.bus_freq;

	/* Maximum multiplier */
	if(!(err = fopen_to_str(max_freq_str, "%s%i/cpufreq/cpuinfo_max_freq", SYS_CPU, current_core_id)))
		data.cpu.clocks.cpu_max_mult = (std::stod(max_freq_str) / 1e3) / data.cpu.clocks.bus_freq;

#endif /* __linux__ */

	return err;
}

/* Retrieve missing Motherboard data if run as regular user */
static int motherboardtab_fallback([[maybe_unused]] Data &data)
{
	int err = 0;

#ifdef __linux__
	MSG_VERBOSE("%s", _("Retrieving motherboard information in fallback mode"));
	/* Frame Board */
	err += fopen_to_str(data.motherboard.board.manufacturer.value, "%s/%s", SYS_DMI, "board_vendor");
	err += fopen_to_str(data.motherboard.board.model.value,        "%s/%s", SYS_DMI, "board_name");
	err += fopen_to_str(data.motherboard.board.revision.value,     "%s/%s", SYS_DMI, "board_version");

	/* Frame Bios */
	err += fopen_to_str(data.motherboard.bios.brand.value,   "%s/%s", SYS_DMI, "bios_vendor");
	err += fopen_to_str(data.motherboard.bios.version.value, "%s/%s", SYS_DMI, "bios_version");
	err += fopen_to_str(data.motherboard.bios.date.value,    "%s/%s", SYS_DMI, "bios_date");
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve motherboard information (fallback mode)"));

	return err;
}

/* Retrieve static data if other functions failed */
static int fallback_mode_static(Data &data)
{
	int err = 0;

#if HAS_LIBCPUID
	const auto& cpu_type = data.cpu.get_selected_cpu_type();
	if((cpu_type.processor.package.value.empty()                                      ||
	   (cpu_type.processor.package.value.find("CPU")            != std::string::npos) ||
	   (cpu_type.processor.package.value.find("Microprocessor") != std::string::npos)))
		err += cputab_package_fallback(data);
#endif /* HAS_LIBCPUID */

	if((data.cpu.clocks.cpu_min_mult <= 0.0) || (data.cpu.clocks.cpu_max_mult <= 0.0))
		err += cputab_multipliers_fallback(data);

	if(data.motherboard.board.manufacturer.value.empty() ||
	   data.motherboard.board.model.value.empty()        ||
	   data.motherboard.board.revision.value.empty()     ||
	   data.motherboard.bios.brand.value.empty()         ||
	   data.motherboard.bios.version.value.empty()       ||
	   data.motherboard.bios.date.value.empty())
		err += motherboardtab_fallback(data);

	return err;
}


/************************* Fallback functions (dynamic) *************************/

/* Retrieve CPU temperature if run as regular user */
static int cputab_temp_fallback(Data &data)
{
	int err = 0;
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const uint16_t current_core_id = data.cpu.get_selected_core_id();

	MSG_VERBOSE("%s", _("Retrieving CPU temperature in fallback mode"));
#ifdef __linux__
	std::string temperature;
# if HAS_LIBCPUID
	static bool module_loaded = false;
	/* Load kernel modules */
	if(!module_loaded)
	{
		if(data.cpu.vendor == VENDOR_INTEL)
			module_loaded = !load_module("coretemp", &data.socket_fd);
		else if((data.cpu.vendor == VENDOR_AMD) && (data.cpu.ext_family <= 0x8))
			module_loaded = !load_module("k8temp", &data.socket_fd);
		else if((data.cpu.vendor== VENDOR_AMD) && (data.cpu.ext_family >= 0x10))
			module_loaded = !load_module("k10temp", &data.socket_fd);
	}
# endif /* HAS_LIBCPUID */

	/* Get sensor path */
	if(cpu_type.processor.path_cpu_temperature.empty())
		cpu_type.processor.path_cpu_temperature = get_sensor_path_cpu_temperature_driver(current_core_id);
	if(cpu_type.processor.path_cpu_temperature.empty())
		cpu_type.processor.path_cpu_temperature = get_sensor_path_cpu_temperature_generic();

	/* Get temperature */
	if(!cpu_type.processor.path_cpu_temperature.empty())
	{
		if(!(err = fopen_to_str(temperature, "%s", cpu_type.processor.path_cpu_temperature.c_str())))
		{
			const double tmp = std::stod(temperature);
			if(tmp > 0.0)
				cpu_type.processor.temperature.value = string_format_temperature_unit("%.1f", tmp / 1e3);
		}
	}

#else /* __linux__ */
	/* Tested on FreeBSD 12: https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/issues/121#issuecomment-575985765 */
	int temperature;
	size_t len = sizeof(temperature);
	char name[MAXSTR];

	snprintf(name, MAXSTR, "dev.cpu.%i.temperature", current_core_id);
	if(!(err = sysctlbyname(name, &temperature, &len, NULL, 0)))
		cpu_type.processor.temperature.value = string_format_temperature_unit("%.1f", double(temperature - 2731) / 10.0); // decikelvins
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve CPU temperature (fallback mode)"));

	return err;
}

/* Retrieve CPU voltage if run as regular user */
static int cputab_volt_fallback([[maybe_unused]] Data &data)
{
	int err = 0;

#ifdef __linux__
	std::string voltage;
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const uint16_t current_core_id = data.cpu.get_selected_core_id();

	MSG_VERBOSE("%s", _("Retrieving CPU voltage in fallback mode"));
	/* Get sensor path */
	if(cpu_type.processor.path_cpu_voltage.empty())
		cpu_type.processor.path_cpu_voltage = get_sensor_path_cpu_voltage(current_core_id);

	/* Get voltage */
	if(!cpu_type.processor.path_cpu_voltage.empty())
		if(!(err = fopen_to_str(voltage, "%s", cpu_type.processor.path_cpu_voltage.c_str())))
			cpu_type.processor.voltage.value = string_format("%.3f V", std::stod(voltage) / 1e3);
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve CPU voltage (fallback mode)"));

	return err;
}

/* Retrieve CPU frequency if Libcpuid is missing */
static int cputab_freq_fallback([[maybe_unused]] Data &data)
{
	int err = 0;

#ifdef __linux__
	std::string freq;
	const uint16_t current_core_id = data.cpu.get_selected_core_id();

	MSG_VERBOSE("%s", _("Retrieving CPU frequency in fallback mode"));
	if(!(err = fopen_to_str(freq, "%s%i/cpufreq/scaling_cur_freq", SYS_CPU, current_core_id)))
		data.cpu.clocks.set_cpu_freq(std::round(std::stod(freq) / 1e3));
#endif /* __linux__ */

	if(err)
		MSG_ERROR("%s", _("failed to retrieve CPU frequency (fallback mode)"));

	return err;
}

/* Retrieve dynamic data if other functions failed */
static int fallback_mode_dynamic(Data &data)
{
	int err = 0;
	auto& cpu_type = data.cpu.get_selected_cpu_type();

	/* CPU temperature */
	static bool use_fallback_temp = Options::get_fallback_cpu_temp();
	if((cpu_type.processor.temperature.value.empty()) || use_fallback_temp)
	{
		use_fallback_temp = true;
		err += err_func(cputab_temp_fallback, data);
	}

	/* CPU voltage */
	static bool use_fallback_volt = Options::get_fallback_cpu_volt();
	if((cpu_type.processor.voltage.value.empty()) || use_fallback_volt)
	{
		use_fallback_volt = true;
		err += err_func(cputab_volt_fallback, data);
	}

	/* CPU speed */
	static bool use_fallback_freq = Options::get_fallback_cpu_freq();
	if(data.cpu.clocks.core_speed.value.empty() || use_fallback_freq)
	{
		use_fallback_freq = true;
		err += err_func(cputab_freq_fallback, data);
	}

	return err;
}


/************************* Public functions *************************/

/* Fill labels by calling below functions */
int fill_labels(Data &data)
{
	int err = 0;

#if HAS_LIBCPUID
	err += call_libcpuid_static(data);
	if(DAEMON_UP) err += call_libcpuid_msr_static(data);
#endif

#if HAS_DMIDECODE
	if(DAEMON_UP) err += call_dmidecode(data);
#endif

#if HAS_LIBPCI
	err += find_devices(data);
#endif

	err += system_static       (data);
	err += fallback_mode_static(data);

	init_benchmarks(data);

	/* Call do_refresh() once to get dynamic values */
	for(TabNumber tab_number = TAB_CPU; tab_number < TAB_ABOUT; tab_number = TabNumber(tab_number + 1))
		err += do_refresh(data, tab_number);

	return err;
}

/* Refresh some labels */
int do_refresh(Data &data, TabNumber tab_number)
{
	int err = 0;

	switch(tab_number)
	{
		case TAB_CPU:
#if HAS_LIBCPUID
			err += err_func(call_libcpuid_dynamic, data);
			if(DAEMON_UP) err += err_func(call_libcpuid_msr_dynamic, data);
#endif /* HAS_LIBCPUID */
			err += err_func(cpu_usage, data);
			err += fallback_mode_dynamic(data);
			err += data.cpu.clocks.set_cpu_multiplier();
			break;
		case TAB_CACHES:
#if HAS_BANDWIDTH
			err += err_func(call_bandwidth, data);
#endif /* HAS_BANDWIDTH */
			break;
		case TAB_SYSTEM:
#if HAS_LIBSYSTEM
			err += err_func(system_dynamic, data);
#endif /* HAS_LIBSYSTEM */
			break;
		case TAB_GRAPHICS:
#if HAS_LIBPCI
			err += err_func(gpu_monitoring, data);
#endif /* HAS_LIBPCI */
			break;
		case TAB_BENCH:
			err += err_func(benchmark_status, data);
			break;
		default:
			break;
	}

	return err;
}
