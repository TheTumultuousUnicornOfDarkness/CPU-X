/****************************************************************************
*    Copyright © 2014-2018 Xorg
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
* FILE label_names.h
*/

#ifndef _LABEL_NAMES_H_
#define _LABEL_NAMES_H_


typedef struct
{
	const unsigned index;
	const char *name;
} LabelNames;

typedef struct
{
	char **dim_names;
	char **dim_values;
	const LabelNames *names;
	int last;
} Arrays;

const LabelNames objects_names[] =
{
	{ TABCPU,              N_("CPU")                  },
	{ TABCACHES,           N_("Caches")               },
	{ TABMOTHERBOARD,      N_("Motherboard")          },
	{ TABMEMORY,           N_("Memory")               },
	{ TABSYSTEM,           N_("System")               },
	{ TABGRAPHICS,         N_("Graphics")             },
	{ TABBENCH,            N_("Bench")                },
	{ TABABOUT,            N_("About")                },

	/* CPU tab */
	{ FRAMPROCESSOR,       N_("Processor")            },
	{ FRAMCLOCKS,          N_("Clocks")               },
	{ FRAMCACHE,           N_("Cache")                },

	/* Caches tab */
	{ FRAML1CACHE,         N_("L1 Cache")             },
	{ FRAML2CACHE,         N_("L2 Cache")             },
	{ FRAML3CACHE,         N_("L3 Cache")             },
	{ FRAML4CACHE,         N_("L4 Cache")             },
	{ FRAMTEST,            N_("Test")                 },

	/* Motherboard tab */
	{ FRAMMOTHERBOARD,     N_("Motherboard")          },
	{ FRAMBIOS,            N_("BIOS")                 },
	{ FRAMCHIPSET,         N_("Chipset")              },

	/* Memory tab */
	{ FRAMBANK0,           N_("Bank 0")               },
	{ FRAMBANK1,           N_("Bank 1")               },
	{ FRAMBANK2,           N_("Bank 2")               },
	{ FRAMBANK3,           N_("Bank 3")               },
	{ FRAMBANK4,           N_("Bank 4")               },
	{ FRAMBANK5,           N_("Bank 5")               },
	{ FRAMBANK6,           N_("Bank 6")               },
	{ FRAMBANK7,           N_("Bank 7")               },

	/* System tab */
	{ FRAMOPERATINGSYSTEM, N_("Operating System")     },
	{ FRAMMEMORY,          N_("Memory")               },

	/* Graphics tab */
	{ FRAMGPU1,            N_("Card 1")               },
	{ FRAMGPU2,            N_("Card 2")               },
	{ FRAMGPU3,            N_("Card 3")               },
	{ FRAMGPU4,            N_("Card 4")               },

	/* Bench tab */
	{ FRAMPRIMESLOW,       N_("Prime numbers (slow)") },
	{ FRAMPRIMEFAST,       N_("Prime numbers (fast)") },
	{ FRAMPARAM,           N_("Parameters")           },

	/* About tab */
	{ FRAMABOUT,           N_("About")                },
	{ FRAMLICENSE,         N_("License")              },
};

const LabelNames tab_cpu_names[] =
{
	// Processor frame
	{ VENDOR,        N_("Vendor")        },
	{ CODENAME,      N_("Code Name")     },
	{ PACKAGE,       N_("Package")       },
	{ TECHNOLOGY,    N_("Technology")    },
	{ VOLTAGE,       N_("Voltage")       },
	{ SPECIFICATION, N_("Specification") },
	{ FAMILY,        N_("Family")        },
	{ EXTFAMILY,     N_("Ext. Family")   },
	{ MODEL,         N_("Model")         },
	{ EXTMODEL,      N_("Ext. Model")    },
	{ TEMPERATURE,   N_("Temp.")         },
	{ STEPPING,      N_("Stepping")      },
	{ INSTRUCTIONS,  N_("Instructions")  },

	// Clocks frame
	{ CORESPEED,     N_("Core Speed")    },
	{ MULTIPLIER,    N_("Multiplier")    },
	{ BUSSPEED,      N_("Bus Speed")     },
	{ USAGE,         N_("Usage")         },

	// Cache frame
	{ LEVEL1D,       N_("L1 Data")       },
	{ LEVEL1I,       N_("L1 Inst.")      },
	{ LEVEL2,        N_("Level 2")       },
	{ LEVEL3,        N_("Level 3")       },

	// Unnamed frame
	{ SOCKETS,       N_("Socket(s)")     },
	{ CORES,         N_("Core(s)")       },
	{ THREADS,       N_("Thread(s)")     },
};

const LabelNames tab_caches_names[] =
{
	// L1 Cache frame
	{ L1SIZE,  N_("Size")  },
	{ L1SPEED, N_("Speed") },

	// L2 Cache frame
	{ L2SIZE,  N_("Size")  },
	{ L2SPEED, N_("Speed") },

	// L3 Cache frame
	{ L3SIZE,  N_("Size")  },
	{ L3SPEED, N_("Speed") },

	// L4 Cache frame
	{ L4SIZE,  N_("Size")  },
	{ L4SPEED, N_("Speed") },
};

const LabelNames tab_motherboard_names[] =
{
	// Motherboard frame
	{ MANUFACTURER, N_("Manufacturer") },
	{ MBMODEL,      N_("Model")        },
	{ REVISION,     N_("Revision")     },

	// BIOS frame
	{ BRAND,        N_("Brand")        },
	{ BIOSVERSION,  N_("Version")      },
	{ DATE,         N_("Date")         },
	{ ROMSIZE,      N_("ROM Size")     },

	// Chipset frame
	{ CHIPVENDOR,   N_("Vendor")       },
	{ CHIPMODEL,    N_("Model")        },
};

const LabelNames tab_memory_names[] =
{
	{ BANK0, N_("Reference") },
	{ BANK1, N_("Reference") },
	{ BANK2, N_("Reference") },
	{ BANK3, N_("Reference") },
	{ BANK4, N_("Reference") },
	{ BANK5, N_("Reference") },
	{ BANK6, N_("Reference") },
	{ BANK7, N_("Reference") },
};

const LabelNames tab_system_names[] =
{
	// Operating System frame
	{ KERNEL,       N_("Kernel")       },
	{ DISTRIBUTION, N_("Distribution") },
	{ HOSTNAME,     N_("Hostname")     },
	{ UPTIME,       N_("Uptime")       },
	{ COMPILER,     N_("Compiler")     },

	// Memory frame
	{ USED,         N_("Used")         },
	{ BUFFERS,      N_("Buffers")      },
	{ CACHED,       N_("Cached")       },
	{ FREE,         N_("Free")         },
	{ SWAP,         N_("Swap")         },
};

const LabelNames tab_graphics_names[] =
{
	// Card 1 frame
	{ GPU1VENDOR,      N_("Vendor")       },
	{ GPU1MODEL,       N_("Model")        },
	{ GPU1TEMPERATURE, N_("Temperature")  },
	{ GPU1USAGE,       N_("Usage")        },
	{ GPU1CORECLOCK,   N_("GPU clock")    },
	{ GPU1MEMCLOCK,    N_("Memory clock") },

	// Card 2 frame
	{ GPU2VENDOR,      N_("Vendor")       },
	{ GPU2MODEL,       N_("Model")        },
	{ GPU2TEMPERATURE, N_("Temperature")  },
	{ GPU2USAGE,       N_("Usage")        },
	{ GPU2CORECLOCK,   N_("GPU clock")    },
	{ GPU2MEMCLOCK,    N_("Memory clock") },

	// Card 3 frame
	{ GPU3VENDOR,      N_("Vendor")       },
	{ GPU3MODEL,       N_("Model")        },
	{ GPU3TEMPERATURE, N_("Temperature")  },
	{ GPU3USAGE,       N_("Usage")        },
	{ GPU3CORECLOCK,   N_("GPU clock")    },
	{ GPU3MEMCLOCK,    N_("Memory clock") },

	// Card 4 frame
	{ GPU4VENDOR,      N_("Vendor")       },
	{ GPU4MODEL,       N_("Model")        },
	{ GPU4TEMPERATURE, N_("Temperature")  },
	{ GPU4USAGE,       N_("Usage")        },
	{ GPU4CORECLOCK,   N_("GPU clock")    },
	{ GPU4MEMCLOCK,    N_("Memory clock") },
};

const LabelNames tab_bench_names[] =
{
	// Prime numbers (slow) frame
	{ PRIMESLOWSCORE, N_("Score")    },
	{ PRIMESLOWRUN,   N_("Run")      },

	// Prime numbers (fast) frame
	{ PRIMEFASTSCORE, N_("Score")    },
	{ PRIMEFASTRUN,   N_("Run")      },

	// Parameters frame
	{ PARAMDURATION,  N_("Duration") },
	{ PARAMTHREADS,   N_("Threads")  },
};

const LabelNames tab_about_names[] =
{
	{ DESCRIPTION, PRGNAME N_(" is a Free software that gathers information\non CPU, motherboard and more.") },
	{ VERSIONSTR,  N_("Version ") PRGVER                                                                     },
	{ AUTHOR,      N_("Author: ") PRGAUTH                                                                    },
	{ SITE,        N_("Site: ") PRGURL                                                                       },
	{ COPYRIGHT,   PRGCPRGHT                                                                                 },
	{ LICENSE,     N_("This software is distributed under the terms of ") PRGLCNS                            },
	{ NOWARRANTY,  N_("This program comes with ABSOLUTELY NO WARRANTY")                                      },
};


#endif /* _LABEL_NAMES_H_ */
