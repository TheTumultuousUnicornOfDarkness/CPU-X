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
* FILE core/libcpuid.cpp
*/

#include <cstring>
#include <unordered_map>
#include <libcpuid/libcpuid.h>
#include "options.hpp"
#include "data.hpp"
#include "databases.h"
#include "daemon/daemon.h"


/* MSRs static values provided by libcpuid */
static int call_libcpuid_msr_debug(Data &data, uint16_t all_cpu_count)
{
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const DaemonCommand cmd = LIBCPUID_MSR_DEBUG;

	if(cpu_type.processor.architecture != ARCHITECTURE_X86)
		return 1;

	SEND_DATA(&data.socket_fd, &cmd, sizeof(DaemonCommand));
	SEND_DATA(&data.socket_fd, &all_cpu_count, sizeof(uint16_t));

	return 0;
}

/* Static elements provided by libcpuid */
int call_libcpuid_static(Data &data)
{
	int err = 0;
	uint16_t core_id_offset = 0;
	const char *cpu_purpose = NULL;
	struct cpu_id_t *cpu_id  = NULL;
	struct cpu_raw_data_array_t raw_data;
	struct system_id_t system_id;
	const struct cpu_flags { const cpu_feature_t flag; const char *str; } *cpu_flags;

	const std::unordered_map<cpu_vendor_t, std::string> cpuvendors =
	{
		{ VENDOR_INTEL,     "Intel"                               },
		{ VENDOR_AMD,       "AMD"                                 },
		{ VENDOR_CYRIX,     "Cyrix"                               },
		{ VENDOR_NEXGEN,    "NexGen"                              },
		{ VENDOR_TRANSMETA, "Transmeta"                           },
		{ VENDOR_UMC,       "UMC"                                 },
		{ VENDOR_CENTAUR,   "Centaur"                             },
		{ VENDOR_RISE,      "Rise"                                },
		{ VENDOR_SIS,       "SiS"                                 },
		{ VENDOR_NSC,       "National Semiconductor"              },
		{ VENDOR_HYGON,	    "Hygon"                               },
		{ VENDOR_ARM,       "ARM Holdings"                        },
		{ VENDOR_BROADCOM,  "Broadcom"                            },
		{ VENDOR_CAVIUM,    "Cavium"                              },
		{ VENDOR_DEC,       "Digital Equipment Corporation"       },
		{ VENDOR_FUJITSU,   "Fujitsu"                             },
		{ VENDOR_HISILICON, "HiSilicon"                           },
		{ VENDOR_INFINEON,  "Infineon"                            },
		{ VENDOR_FREESCALE, "Motorola or Freescale Semiconductor" },
		{ VENDOR_NVIDIA,    "NVIDIA"                              },
		{ VENDOR_APM,       "Applied Micro Circuits"              },
		{ VENDOR_QUALCOMM,  "Qualcomm"                            },
		{ VENDOR_SAMSUNG,   "Samsung"                             },
		{ VENDOR_MARVELL,   "Marvell"                             },
		{ VENDOR_APPLE,     "Apple"                               },
		{ VENDOR_FARADAY,   "Faraday"                             },
		{ VENDOR_MICROSOFT, "Microsoft"                           },
		{ VENDOR_PHYTIUM,   "Phytium"                             },
		{ VENDOR_AMPERE,    "Ampere Computing"                    },
		{ VENDOR_UNKNOWN,   _("unknown")                          }
	};

	const struct cpu_flags cpu_flags_x86[] =
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
	const struct cpu_flags cpu_flags_arm[] =
	{
		{ CPU_FEATURE_THUMB,        "Thumb"    },
		{ CPU_FEATURE_JAZELLE,      "Jazelle"  },
		{ CPU_FEATURE_THUMB2,       "Thumb-2"  },
		{ CPU_FEATURE_THUMBEE,      "ThumbEE"  },
		{ CPU_FEATURE_FP,           "FP"       },
		{ CPU_FEATURE_FP16,         "FP16"     },
		{ CPU_FEATURE_ADVSIMD,      "Neon"     },
		{ CPU_FEATURE_AES,          "AES"      },
		{ CPU_FEATURE_PMULL,        "PMULL"    },
		{ CPU_FEATURE_RDM,          "RDM"      },
		{ CPU_FEATURE_DOTPROD,      "DOT"      },
		{ CPU_FEATURE_RNG,          "RNG"      },
		{ CPU_FEATURE_SHA1,         "SHA(1"    },
		{ CPU_FEATURE_SHA3,         "3"        },
		{ CPU_FEATURE_SHA256,       "256"      },
		{ CPU_FEATURE_SHA512,       "512"      },
		{ CPU_FEATURE_SHA1,         ")"        },
		{ CPU_FEATURE_SM3,          "SM(3"     },
		{ CPU_FEATURE_SM4,          "4"        },
		{ CPU_FEATURE_SM3,          ")"        },
		{ CPU_FEATURE_SVE,          "SVE(1"    },
		{ CPU_FEATURE_SVE2,         "2"        },
		{ CPU_FEATURE_SVE2P1,       "2.1"      },
		{ CPU_FEATURE_SVE_AES,      "AES"      },
		{ CPU_FEATURE_SVE_PMULL128, "PMULL128" },
		{ CPU_FEATURE_SVE_SHA3,     "SHA3"     },
		{ CPU_FEATURE_SVE_SM4,      "SM4"      },
		{ CPU_FEATURE_SVE_B16B16,   "B16"      },
		{ CPU_FEATURE_SVE,          ")"        },
		{ CPU_FEATURE_SME,          "SME(1"    },
		{ CPU_FEATURE_SME2,         "2"        },
		{ CPU_FEATURE_SME2P1,       "2.1"      },
		{ CPU_FEATURE_SME_F64F64,   "F64"      },
		{ CPU_FEATURE_SME_I16I64,   "I16"      },
		{ CPU_FEATURE_SME_F16F16,   "F16"      },
		{ CPU_FEATURE_SME,          ")"        },
		/* Virtualization */
		{ CPU_FEATURE_VHE,          "VHE"      },
		/* Other */
		{ NUM_CPU_FEATURES,         NULL       }
	};

	/* Call libcpuid */
	MSG_VERBOSE("%s", _("Calling libcpuid for retrieving static data"));
	if(Options::get_issue())
		cpuid_set_verbosiness_level(2);
#if defined(__arm__) || defined(__aarch64__)
	if(!cpuid_present())
		load_module("cpuid", &data.socket_fd); // https://github.com/anrieff/libcpuid/tree/master/drivers/arm
#endif /* defined(__arm__) || defined(__aarch64__) */
	if(data.cpu.cpuid_raw_file == NULL)
		err = cpuid_get_all_raw_data(&raw_data);
	else
		err = cpuid_deserialize_all_raw_data(&raw_data, data.cpu.cpuid_raw_file);

	if(err || cpu_identify_all(&raw_data, &system_id))
	{
		MSG_ERROR(_("failed to call libcpuid (%s)"), cpuid_error());
		return 1;
	}

	/* Basically fill CPU tab */
	for(uint8_t cpu_type = 0; cpu_type < system_id.num_cpu_types; cpu_type++)
	{
		cpu_id = &system_id.cpu_types[cpu_type];
		cpu_purpose = cpu_purpose_str(cpu_id->purpose);
		data.cpu.grow_cpu_types_vector(cpu_type, cpu_purpose);
		data.caches.grow_cpu_types_vector(cpu_type, cpu_purpose);

		/* Find kernel module for CPU temperature (used by cputab_temp_fallback()) only for the first type */
		if(cpu_type == 0)
		{
			if(cpu_id->architecture == ARCHITECTURE_X86)
			{
				cpu_flags = cpu_flags_x86;
				if(cpu_id->vendor == VENDOR_INTEL)
					data.cpu.sensors_module_name = "coretemp";
				else if((cpu_id->vendor == VENDOR_AMD) && (cpu_id->x86.ext_family <= 0x8))
					data.cpu.sensors_module_name = "k8temp";
				else if((cpu_id->vendor == VENDOR_AMD) && (cpu_id->x86.ext_family >= 0x10))
					data.cpu.sensors_module_name = "k10temp";
			}
			else if(cpu_id->architecture == ARCHITECTURE_ARM)
			{
				cpu_flags = cpu_flags_arm;
			}
		}

		/* Trivial assignments */
		data.cpu.vendor                                            = cpu_id->vendor;
		data.cpu.cpu_types[cpu_type].purpose                       = cpu_id->purpose;
		data.cpu.cpu_types[cpu_type].processor.architecture        = cpu_id->architecture;
		data.cpu.cpu_types[cpu_type].processor.vendor.value        = (cpuvendors.find(cpu_id->vendor) != cpuvendors.end()) ? cpuvendors.at(cpu_id->vendor) : cpuvendors.at(VENDOR_UNKNOWN);
		data.cpu.cpu_types[cpu_type].processor.codename.value      = cpu_id->cpu_codename;
		data.cpu.cpu_types[cpu_type].processor.technology.value    = cpu_id->technology_node;
		data.cpu.cpu_types[cpu_type].processor.specification.value = cpu_id->brand_str;
		if(cpu_id->architecture == ARCHITECTURE_X86)
		{
			data.cpu.cpu_types[cpu_type].processor.family.value        = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->x86.family);
			data.cpu.cpu_types[cpu_type].processor.dispfamily.value    = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->x86.ext_family);
			data.cpu.cpu_types[cpu_type].processor.model.value         = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->x86.model);
			data.cpu.cpu_types[cpu_type].processor.dispmodel.value     = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->x86.ext_model);
			data.cpu.cpu_types[cpu_type].processor.stepping.value      = std::to_string(cpu_id->x86.stepping);
		}
		else if(cpu_id->architecture == ARCHITECTURE_ARM)
		{
			data.cpu.cpu_types[cpu_type].processor.implementer.value   = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->arm.implementer);
			data.cpu.cpu_types[cpu_type].processor.variant.value       = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->arm.variant);
			data.cpu.cpu_types[cpu_type].processor.partnum.value       = Data::Cpu::CpuType::Processor::format_cpuid_value(cpu_id->arm.part_num);
			data.cpu.cpu_types[cpu_type].processor.revision.value      = std::to_string(cpu_id->arm.revision);
		}
		data.cpu.cpu_types[cpu_type].footer.cores.value            = std::to_string(cpu_id->num_cores);
		data.cpu.cpu_types[cpu_type].footer.threads.value          = std::to_string(cpu_id->num_logical_cpus);
		data.cpu.cpu_types[cpu_type].footer.num_threads            = cpu_id->num_logical_cpus;
		string_trim(data.cpu.cpu_types[cpu_type].processor.specification.value);

		/* Add core offset */
		data.cpu.cpu_types[cpu_type].footer.core_id_offset = core_id_offset;
		core_id_offset = cpu_id->num_logical_cpus;

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
	if(Options::get_issue())
	{
		cpuid_serialize_all_raw_data(&raw_data, "");
		if(DAEMON_UP) call_libcpuid_msr_debug(data, raw_data.num_raw);
	}
	cpuid_free_raw_data_array(&raw_data);
	cpuid_free_system_id(&system_id);

	return err;
}

/* Dynamic elements provided by libcpuid */
int call_libcpuid_dynamic(Data &data)
{
	/* CPU frequency */
	MSG_VERBOSE("%s", _("Calling libcpuid for retrieving dynamic data"));
	data.cpu.clocks.set_cpu_freq(cpu_clock());

	return (data.cpu.clocks.cpu_freq <= 0);
}

/* MSRs static values provided by libcpuid */
int call_libcpuid_msr_static(Data &data)
{
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const DaemonCommand cmd = LIBCPUID_MSR_STATIC;
	const uint16_t current_core_id = data.cpu.get_selected_core_id();
	MsrStaticData msg;

	if(cpu_type.processor.architecture != ARCHITECTURE_X86)
		return 1;

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
int call_libcpuid_msr_dynamic(Data &data)
{
	auto& cpu_type = data.cpu.get_selected_cpu_type();
	const uint16_t current_core_id = data.cpu.get_selected_core_id();
	const DaemonCommand cmd = LIBCPUID_MSR_DYNAMIC;
	MsrDynamicData msg;

	if(cpu_type.processor.architecture != ARCHITECTURE_X86)
		return 1;

	MSG_VERBOSE("%s", _("Calling libcpuid for retrieving CPU MSR dynamic values"));
	SEND_DATA(&data.socket_fd,  &cmd, sizeof(DaemonCommand));
	SEND_DATA(&data.socket_fd,  &current_core_id, sizeof(current_core_id));
	RECEIVE_DATA(&data.socket_fd, &msg, sizeof(MsrDynamicData));

	/* CPU Voltage */
	if(msg.voltage != CPU_INVALID_VALUE)
		cpu_type.processor.voltage.value = string_format("%.3f V", double(msg.voltage) / 100.0);

	/* CPU Temperature */
	if(msg.temp != CPU_INVALID_VALUE)
		cpu_type.processor.temperature.value = string_with_temperature_unit(double(msg.temp));

	return 0;
}


/************************* Fallback functions (static) *************************/

/* If dmidecode fails to find CPU package, check in database */
int cputab_package_fallback(Data &data)
{
	int i = -1;
	const Package_DB *db;
	const auto& cpu_type = data.cpu.get_selected_cpu_type();

	if(cpu_type.processor.codename.value.empty() || cpu_type.processor.specification.value.empty())
		return 1;

	MSG_VERBOSE("%s", _("Finding CPU package in fallback mode"));
	switch(data.cpu.vendor)
	{
		case VENDOR_AMD:   db = package_amd;     break;
		case VENDOR_INTEL: db = package_intel;   break;
		default:           return 2;
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
	return 3;
}
