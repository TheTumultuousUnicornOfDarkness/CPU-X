/****************************************************************************
*    Copyright © 2014-2024 The Tumultuous Unicorn Of Darkness
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
* FILE databases.h
*/

#ifndef _DATABASES_H_
#define _DATABASES_H_


/****************************** CPU Technology ******************************/

typedef struct
{
	const int32_t cpu_model;
	const int32_t cpu_ext_model;
	const int32_t cpu_ext_family;
	const char    *process;
} Technology_DB_x86;

typedef struct
{
	const int32_t part_num;
	const char    *codename;
	const char    *process;
} Technology_DB_ARM;

const Technology_DB_x86 technology_x86_intel[] =
{
	//Model        E. Model     E. Family   Process
	{  0,           0,          -1,        "0.18 µm" }, // P4 Willamette
	{  1,           1,           6,        "0.35 µm" }, // Pentium Pro
	{  1,           1,          15,        "0.18 µm" }, // P4 Willamette
	{  2,           2,          -1,        "0.13 µm" }, // P4 Northwood / Gallatin
	{  3,           3,           5,        "0.35 µm" }, // PII Overdrive
	{  3,           3,           6,        "0.35 µm" }, // PII Klamath
	{  3,           3,          15,          "90 nm" }, // P4 Prescott
	{  4,           4,          -1,          "90 nm" }, // P4 Prescott/Irwindale / PD Smithfield
	{  5,           5,           6,        "0.25 µm" }, // PII Deschutes / Tonga / Xeon Drake / Celeron Covington
	{  5,          37,          -1,          "32 nm" }, // Westmere
	{  5,          53,          -1,          "32 nm" }, // Atom Cloverview
	{  5,          69,          -1,          "22 nm" }, // Haswell
	{  5,          85,          -1,          "14 nm" }, // Skylake-X / Skylake-W / Skylake-DE / Skylake-SP / Cascade Lake-X / Cascade Lake-W / Cascade Lake-SP
	{  5,          165,          6,          "14 nm" }, // Comet Lake
	{  6,           6,           6,        "0.25 µm" }, // PII Dixon / Celeron Mendocino
	{  6,           6,          15,          "65 nm" }, // P4 Cedar Mill / PD Presler
	{  6,          22,          -1,          "65 nm" }, // C2 Conroe-L
	{  6,          54,          -1,          "32 nm" }, // Atom Cedarview
	{  6,          70,          -1,          "22 nm" }, // Haswell (Crystalwell, L4 cache)
	{  6,         102,          -1,          "10 nm" }, // Cannon Lake
	{  6,         150,          -1,          "10 nm" }, // Tremont (Elkhart Lake)
	{  7,           7,          -1,        "0.25 µm" }, // PIII Katmai
	{  7,          23,          -1,          "45 nm" }, // C2 Wolfdale / Yorkfield / Penryn
	{  7,          55,          -1,          "22 nm" }, // Atom Bay Trail
	{  7,          71,          -1,          "14 nm" }, // Broadwell
	{  7,         151,          -1,        "Intel 7" }, // Alder Lake-S / Alder Lake-HX
	{  7,         167,          -1,          "14 nm" }, // Rocket Lake
	{  7,         183,          -1,        "Intel 7" }, // Raptor Lake-S / Raptor Lake-HX
	{  8,           0,           0,        "0.18 µm" }, // PIII Coppermine-T
	{  8,           8,          -1,        "0.18 µm" }, // PIII Coppermine
	{  9,           9,          -1,        "0.13 µm" }, // Pentium M Banias
	{ 10,          26,          -1,          "45 nm" }, // Nehalem
	{ 10,          30,          -1,          "45 nm" }, // Nehalem
	{ 10,          42,          -1,          "32 nm" }, // Sandy Bridge
	{ 10,          58,          -1,          "22 nm" }, // Ivy Bridge
	{ 10,         106,          -1,          "10 nm" }, // Ice Lake-D / Ice Lake-W / Ice Lake-SP
	{ 10,         122,          -1,          "14 nm" }, // Gemini Lake
	{ 10,         138,          -1,          "10 nm" }, // Tremont (Lakefield)
	{ 10,         154,          -1,        "Intel 7" }, // Alder Lake-P / Alder Lake-H
	{ 10,         170,          -1,        "Intel 4" }, // Meteor Lake-H
	{ 10,         186,          -1,        "Intel 7" }, // Raptor Lake-P / Raptor Lake-U / Raptor Lake-H
	{ 11,          11,          -1,        "0.13 µm" }, // PIII Tualatine
	{ 12,          28,          -1,          "45 nm" }, // Atom Diamondville / Pineview / Silverthorne
	{ 12,          44,          -1,          "32 nm" }, // Westmere
	{ 12,          60,          -1,          "22 nm" }, // Haswell
	{ 12,          76,          -1,          "14 nm" }, // Atom Cherry Trail
	{ 12,          92,          -1,          "14 nm" }, // Apollo Lake
	{ 12,         108,          -1,          "10 nm" }, // Ice Lake
	{ 12,         140,          -1,           "10SF" }, // Tiger Lake
	{ 12,         156,          -1,          "10 nm" }, // Tremont (Jasper Lake)
	{ 13,          13,          -1,          "90 nm" }, // Pentium M Dothan
	{ 13,          45,          -1,          "32 nm" }, // Sandy Bridge-E
	{ 13,          61,          -1,          "14 nm" }, // Broadwell-U
	{ 14,          14,          -1,          "65 nm" }, // Yonah (Core Solo)
	{ 14,          30,          -1,          "45 nm" }, // Nehalem (Lynnfield)
	{ 14,          62,          -1,          "22 nm" }, // Ivy Bridge-E
	{ 14,          78,          -1,          "14 nm" }, // Skylake
	{ 14,          94,          -1,          "14 nm" }, // Skylake
	{ 14,         126,          -1,          "10 nm" }, // Ice Lake
	{ 14,         142,          -1,          "14 nm" }, // Kaby Lake / Coffee Lake / Comet Lake-U
	{ 14,         158,          -1,          "14 nm" }, // Kaby Lake / Coffee Lake
	{ 14,         165,          -1,          "14 nm" }, // Comet Lake
	{ 14,         190,          -1,        "Intel 7" }, // Alder Lake-N
	{ 15,          15,          -1,          "65 nm" }, // C2 Conroe / Allendale / Kentsfield / Merom
	{ 15,          63,          -1,          "22 nm" }, // Haswell-E
	{ 15,          79,          -1,          "14 nm" }, // Broadwell-E
	{ 15,         143,          -1,        "Intel 7" }, // Sapphire Rapids-WS / Sapphire Rapids-SP
	{ 15,         191,          -1,        "Intel 7" }, // Raptor Lake-S ("Golden Cove" cores)
	{ 15,         207,          -1,        "Intel 7" }, // Emerald Rapids-SP
	{ -2,          -2,          -2,             NULL }
	//Model        E. Model     E. Family   Process
};

const Technology_DB_x86 technology_x86_amd[] =
{
	//Model        E. Model     E. Family   Process
	{  0,          16,          21,          "32 nm" }, // Bulldozer (Piledriver: Trinity)
	{  0,          48,          21,          "28 nm" }, // Bulldozer (Steamroller: Kaveri)
	{  0,         112,          21,          "28 nm" }, // Bulldozer (Excavator: Stoney Ridge)
	{  0,          -1,          22,          "28 nm" }, // Jaguar (Kabini/Mullins)
	{  1,          -1,          18,          "32 nm" }, // K10 (Llano)
	{  1,          -1,          20,          "40 nm" }, // Bobcat (Brazos Zacate)
	{  1,           1,          21,          "32 nm" }, // Bulldozer
	{  1,          96,          21,          "28 nm" }, // Bulldozer (Excavator: Carrizo)
	{  2,          -1,          16,          "65 nm" }, // K10 (Kuma/Agena)
	{  2,          -1,          20,          "40 nm" }, // Bobcat (Brazos Desna/Ontario)
	{  2,          -1,          21,          "32 nm" }, // Bulldozer (Piledriver: Vishera)
	{  3,          -1,          15,          "90 nm" }, // K8 (Toledo)
	{  3,          -1,          21,          "32 nm" }, // Bulldozer (Piledriver: Richland)
	{  4,          -1,          15,          "90 nm" }, // K8 (Lancaster)
	{  4,          -1,          16,          "45 nm" }, // K10 (Deneb/Heka)
	{  5,          -1,          16,          "45 nm" }, // K10 (Propus)
	{  5,          -1,          21,          "28 nm" }, // Bulldozer (Excavator: Bristol Ridge)
	{  6,          -1,          16,          "45 nm" }, // K10 (Champlain/Sargas/Regor)
	{  8,          -1,           6,        "0.13 µm" }, // K7 (Thoroughbred/Applebred)
	{  8,          -1,          15,          "65 nm" }, // K8 (Tyler)
	{  8,          -1,          21,          "28 nm" }, // Bulldozer (Steamroller: Godavari)
	{  9,          -1,          16,          "45 nm" }, // Magny-Cours (Opteron)
	{ 10,          -1,           6,        "0.13 µm" }, // K7 (Barton)
	{ 10,          -1,          16,          "45 nm" }, // K10 (Thuban)
	{ 11,          -1,          15,          "65 nm" }, // K8 (Brisbane)
	{ 12,         124,          15,          "65 nm" }, // K8 (Sherman)
	{ 12,          -1,          15,          "90 nm" }, // K8 (Venice/Sonora)
	{ 15,          79,          15,          "90 nm" }, // K8 (Manila)
	{ 15,         127,          15,          "65 nm" }, // K8 (Sparta)
	{ -1,           1,          23,          "14 nm" }, // Zen (Summit Ridge/Whitehaven/Naples)
	{ -1,          17,          23,          "14 nm" }, // Zen (Raven Ridge)
	{ -1,           8,          23,          "12 nm" }, // Zen+ (Pinnacle Ridge/Colfax)
	{ -1,          32,          23,          "14 nm" }, // Zen (Dali)
	{ -1,          24,          23,          "12 nm" }, // Zen+ (Picasso)
	{ -1,          49,          23,           "7 nm" }, // Zen 2 (Rome/Castle Peak)
	{ -1,          71,          23,           "7 nm" }, // Zen 2 (4700S Desktop Kit)
	{ -1,          96,          23,           "7 nm" }, // Zen 2 (Renoir)
	{ -1,         104,          23,           "7 nm" }, // Zen 2 (Lucienne)
	{ -1,         113,          23,           "7 nm" }, // Zen 2 (Matisse)
	{ -1,         132,          23,           "7 nm" }, // Zen 2 (4800S Desktop Kit)
	{ -1,         144,          23,           "7 nm" }, // Zen 2 (Custom APU 0405 for Steam Deck)
	{ -1,         145,          23,           "7 nm" }, // Zen 2 (Custom APU 0932 for Steam Deck)
	{ -1,         160,          23,           "6 nm" }, // Zen 2 (Mendocino)
	{ -1,          33,          25,           "7 nm" }, // Zen 3 (Vermeer)
	{ -1,          80,          25,           "7 nm" }, // Zen 3 (Cezanne)
	{ -1,           1,          25,           "7 nm" }, // Zen 3 (Milan)
	{ -1,          68,          25,           "7 nm" }, // Zen 3 (Rembrandt)
	{ -1,           8,          25,           "7 nm" }, // Zen 3 (Chagall)
	{ -1,          24,          25,           "5 nm" }, // Zen 4 (Storm Peak)
	{ -1,          97,          25,           "5 nm" }, // Zen 4 (Raphael/Dragon Range)
	{ -1,          17,          25,           "5 nm" }, // Zen 4 (Genoa)
	{ -1,         116,          25,           "4 nm" }, // Zen 4 (Phoenix)
	{ -1,         117,          25,           "4 nm" }, // Zen 4 (Phoenix/Hawk Point)
	{ -1,         120,          25,           "4 nm" }, // Zen 4 (Z1)
	{ -1,          36,          26,           "4 nm" }, // Zen 5 (Strix Point)
	{ -1,          68,          26,           "4 nm" }, // Zen 4 (Granite Ridge)
	{ -2,          -2,          -2,             NULL }
	//Model        E. Model     E. Family   Process
};

const Technology_DB_ARM technology_arm_apple[] =
{
	//       Codename      Process
	{    -1, "Swift",     "32 nm"    },
	{    -1, "Cyclone",   "28 nm"    },
	{    -1, "Typhoon",   "20 nm"    },
	{    -1, "Twister",   "16-14 nm" },
	{    -1, "Zephyr",    "16-10 nm" },
	{    -1, "Hurricane", "16-10 nm" },
	{    -1, "Monsoon",   "10 nm"    },
	{    -1, "Mistral",   "10 nm"    },
	{    -1, "Vortex",    "7 nm"     },
	{    -1, "Tempest",   "7 nm"     },
	{    -1, "Lightning", "7 nm"     },
	{    -1, "Thunder",   "7 nm"     },
	{    -1, "Icestorm",  "5 nm"     },
	{    -1, "Firestorm", "5 nm"     },
	{    -1, "Blizzard",  "5 nm"     },
	{    -1, "Avalanche", "5 nm"     },
	{    -1, "Sawtooth",  "5 nm"     },
	{    -1, "Everest",   "5 nm"     },
	//       Codename      Process
};

const Technology_DB_ARM technology_arm_arm[] =
{
	//PartNum     Codename      Process
	{ 0xc05,      NULL,         "40-28 nm" }, // Cortex-A5
	{ 0xc07,      NULL,         "40-28 nm" }, // Cortex-A7
	{ 0xc08,      NULL,         "65-45 nm" }, // Cortex-A8
	{ 0xc09,      NULL,         "65-28 nm" }, // Cortex-A9
	{ 0xc0d,      NULL,         "28 nm"    }, // Cortex-A12
	{ 0xc0f,      NULL,         "32-20 nm" }, // Cortex-A15
	{ 0xc0e,      NULL,         "28 nm"    }, // Cortex-A17
	{ 0xd01,      NULL,         "28 nm"    }, // Cortex-A32
	{ 0xd03,      NULL,         "28-10 nm" }, // Cortex-A53
	{ 0xd04,      NULL,         "28-10 nm" }, // Cortex-A35
	{ 0xd05,      NULL,         "28-5 nm"  }, // Cortex-A55
	{ 0xd07,      NULL,         "28-14 nm" }, // Cortex-A57
	{ 0xd08,      NULL,         "28-16 nm" }, // Cortex-A72
	{ 0xd09,      NULL,         "28-10 nm" }, // Cortex-A73
	{ 0xd0a,      NULL,         "28-10 nm" }, // Cortex-A75
	{ 0xd0b,      NULL,         "10-7 nm"  }, // Cortex-A76
	{ 0xd0c,      NULL,         "7 nm"     }, // Neoverse-N1
	{ 0xd0d,      NULL,         "7 nm"     }, // Cortex-A77
	{ 0xd40,      NULL,         "7 nm"     }, // Neoverse-V1
	{ 0xd41,      NULL,         "5 nm"     }, // Cortex-A78
	{ 0xd44,      NULL,         "10-5 nm"  }, // Cortex-X1
	{ 0xd46,      NULL,         "7-5 nm"   }, // Cortex-A510
	{ 0xd47,      NULL,         "7-5 nm"   }, // Cortex-A710
	{ 0xd49,      NULL,         "5 nm"     }, // Neoverse-N2
	{ 0xd4b,      NULL,         "5 nm"     }, // Cortex-A78C
	{ 0xd4d,      NULL,         "7-5 nm"   }, // Cortex-A715
	{ 0xd80,      NULL,         "3 nm"     }, // Cortex-A520
	{ 0xd81,      NULL,         "3 nm"     }, // Cortex-A720
	{ -2,         NULL,         NULL       }
	//PartNum     Codename      Process
};

const Technology_DB_ARM technology_arm_qualcomm[] =
{
	//PartNum     Codename      Process
	{ 0x00f,      NULL,         "65-45 nm" }, // Scorpion
	{ 0x02d,      NULL,         "65-45 nm" }, // Scorpion
	{ 0x04d,      NULL,         "28 nm"    }, // Krait
	{ 0x06f,      NULL,         "28 nm"    }, // Krait
	{ 0x201,      NULL,         "14 nm"    }, // Kryo
	{ 0x205,      NULL,         "14 nm"    }, // Kryo
	{ 0x211,      NULL,         "14 nm"    }, // Kryo
	{ 0x801,      NULL,         "14-6 nm"  }, // Kryo-V2
	{ 0x802,      NULL,         "10 nm"    }, // Kryo-3XX-Gold
	{ 0x803,      NULL,         "10 nm"    }, // Kryo-3XX-Silver
	{ 0x804,      NULL,         "11-7 nm"  }, // Kryo-4XX-Gold
	{ 0x805,      NULL,         "8-7 nm"   }, // Kryo-4XX-Silver
	{ 0xc00,      NULL,         "10 nm"    }, // Falkor
	//PartNum     Codename      Process
};

const Technology_DB_ARM technology_arm_samsung[] =
{
	//PartNum     Codename      Process
	{ 0x001,      NULL,         "14 nm" }, // Exynos M1
	{ 0x002,      NULL,         "10 nm" }, // Exynos M3
	{ 0x003,      NULL,         "8 nm"  }, // Exynos M4
	{ 0x004,      NULL,         "7 nm"  }, // Exynos M5
	//PartNum     Codename      Process
};

const Technology_DB_ARM technology_arm_nvidia[] =
{
	//PartNum     Codename      Process
	{ 0x000,      NULL,         "28 nm" }, // Denver
	{ 0x003,      NULL,         "16 nm" }, // Denver 2
	{ 0x004,      NULL,         "12 nm" }, // Carmel
	//PartNum     Codename      Process
};

/****************************** CPU Package ******************************/

typedef struct {
	const char *codename;
	const char *model;
	const char *socket;
} Package_DB;

const Package_DB package_intel[] =
{
	//Codename                          Model                               Socket
	{ "Atom (Diamondville)",            NULL,                               "BGA 437"        },
	{ "Pentium D (SmithField)",         NULL,                               "LGA 775"        },
	{ "Pentium D (Presler)",            NULL,                               "LGA 775"        },
	{ "Bloomfield",                     NULL,                               "LGA 1366"       },
	{ NULL,                             "Intel(R) Core(TM)2 Duo CPU E7400", "LGA 775"        },
	{ NULL,                             "Intel(R) Core(TM)2 Quad CPU Q6600","LGA 775"        },
	{ NULL,                             "Intel(R) Core(TM) i5-2500K CPU",   "LGA 1155"       },
	{ NULL,                             "Intel(R) Celeron(R) CPU G550T",    "LGA 1155"       },
	{ NULL,                             "Intel(R) Core(TM) i5-2520M CPU",   "rPGA 988B"      },
	{ NULL,                             "Intel(R) Xeon(R) CPU E31275",      "LGA 1155"       },
	{ NULL,                             "Intel(R) Core(TM) i7-2860QM CPU",  "rPGA 988B"      },
	{ NULL,                             "Intel(R) Core(TM) i7-3740QM CPU",  "rPGA 988B"      },
	{ NULL,                             "Intel(R) Core(TM) i5-4300U CPU",   "BGA 1168"       },
	{ NULL,                             "Intel(R) Core(TM) i7-4600U CPU",   "BGA 1168"       },
	{ NULL,                             "Intel(R) Core(TM) i7-4790 CPU",    "LGA 1150"       },
	{ NULL,                             "Intel(R) Core(TM) i7-5775C CPU",   "LGA 1150"       },
	{ NULL,                             "Intel(R) Core(TM) i7-6560U CPU",   "BGA 1356"       },
	{ NULL,                             "Intel(R) Core(TM) i5-6400 CPU",    "LGA 1151"       },
	{ NULL,                             "Intel(R) Core(TM) i7-7300U CPU",   "BGA 1356"       },
	{ NULL,                             "Intel(R) Core(TM) i7-7820HQ CPU",  "BGA 1440"       },
	{ NULL,                             "Intel(R) Core(TM) i7-8250U CPU",   "BGA 1356"       },
	{ NULL,                             "Intel(R) Core(TM) i7-8350U CPU",   "BGA 1356"       },
	{ NULL,                             "Intel(R) Core(TM) i7-8550U CPU",   "BGA 1356"       },
	{ NULL,                             "Intel(R) Core(TM) i7-8650U CPU",   "BGA 1356"       },
	{ NULL,                             "Intel(R) Core(TM) i5-9300H CPU",   "BGA 1440"       },
	{ NULL,                             "Intel(R) Core(TM) i7-9700 CPU",    "LGA 1151"       },
	{ NULL,                             "Intel(R) Core(TM) i9-9900K CPU",   "LGA 1151"       },
	{ NULL,                             "Intel(R) Core(TM) i5-10500 CPU",   "LGA 1200"       },
	{ NULL,                             "Intel(R) Core(TM) i5-1135G7",      "BGA 1449"       },
	{ NULL,                             "Intel(R) Core(TM) i7-1165G7",      "BGA 1449"       },
	{ NULL,                             "Intel(R) Core(TM) i7-11700K",      "LGA 1200"       },
	{ NULL,                             "Intel(R) Core(TM) i9-11900K",      "LGA 1200"       },
	{ NULL,                             "Intel(R) Core(TM) i3-1220P",       "BGA 1744"       },
	{ NULL,                             "Intel(R) Core(TM) i5-12400",       "LGA 1700"       },
	{ NULL,                             "Intel(R) Core(TM) i7-12700H",      "BGA 1744"       },
	{ NULL,                             "Intel(R) Core(TM) i7-12700KF",     "LGA 1700"       },
	{ NULL,                             "Intel(R) Core(TM) i9-12900K",      "LGA 1700"       },
	{ NULL,                             "Intel(R) Core(TM) i9-12900HK",     "BGA 1744"       },
	{ NULL,                             "Intel(R) Core(TM) i5-13500T",      "LGA 1700"       },
	{ NULL,                             "Intel(R) Core(TM) i5-13600K",      "LGA 1700"       },
	{ NULL,                             "Intel(R) Core(TM) i7-13700K",      "LGA 1700"       },
	{ NULL,                             "Intel(R) Core(TM) i9-13900K",      "LGA 1700"       },
	{ NULL,                             "Intel(R) Celeron(R) CPU N2930",    "BGA 1170"       },
	{ NULL,                             "Intel(R) Celeron(R) CPU N3350",    "BGA 1296"       },
	{ NULL,	                            "Intel(R) Celeron(R) N4000",        "BGA 1090"       },
	{ NULL,                             "Intel(R) Core(TM) i5-3470 CPU",    "LGA 1155"       },
	{ NULL,                             "Intel(R) Core(TM) i7-4770 CPU",    "LGA 1155"       },
	{ NULL,                             "Intel(R) Core(TM) i3-4350 CPU",    "LGA 1150"       },
	{ NULL,                             "Intel(R) Xeon(R) Gold 5220 CPU",   "LGA 3647"       },
	{ NULL,                             "Intel(R) Xeon(R) CPU E5-2689",     "LGA 2011"       },
	{ NULL,                             "Intel(R) Xeon(R) CPU E5-2690 ",    "LGA 2011"       },
	{ NULL,                             "Intel(R) Xeon(R) CPU E5-2690 v2",  "LGA 2011"       },
	{ NULL,                             "Intel(R) Xeon(R) CPU E5-2690 v3",  "LGA 2011-3"     },
	{ NULL,                             "Intel(R) Xeon(R) CPU E5-2690 v4",  "LGA 2011-3"     },
	{ NULL,                             NULL,                               NULL             }
	//Codename                          Model                               Socket
};

const Package_DB package_amd[] =
{
	//Codename                          Model            Socket
	/* K8 */
	{ "Athlon 64 FX X2 (Toledo)",       NULL,            "939 (PGA-939)"  },
	{ "Turion X2",                      "TL",            "S1g1 (PGA-638)" },
	{ "Turion X2",                      "TK",            "S1g1 (PGA-638)" },
	/* K10 */
	  /* Desktop */
	{ "Kuma",                          NULL,             "AM2+ (PGA-940)" }, // 2008-2009, 65 nm
	{ "Agena",                         NULL,             "AM2+ (PGA-940)" }, // 2007-2008, 65 nm
	{ "Toliman",                       NULL,             "AM2+ (PGA-940)" }, // 2008, 65 nm
	{ "Thuban",                        NULL,             "AM3 (PGA-938)"  }, // 2010, 45 nm
	{ "Zosma",                         NULL,             "AM3 (PGA-938)"  }, // 2010-2011, 45 nm
	{ "Deneb",          "AMD Phenom(tm) II X4 920",      "AM2+ (PGA-940)" }, // 2009, 45 nm
	{ "Deneb",          "AMD Phenom(tm) II X4 940",      "AM2+ (PGA-940)" }, // 2009, 45 nm
	{ "Deneb",                         NULL,             "AM3 (PGA-938)"  }, // 2009-2011, 45 nm
	{ "Propus",                        NULL,             "AM3 (PGA-938)"  }, // 2009-2011, 45 nm
	{ "Heka",           "AMD Phenom(tm) II X3 715",      "AM2+ (PGA-940)" }, // 2009, 45 nm
	{ "Heka",                          NULL,             "AM3 (PGA-938)"  }, // 2009-2010, 45 nm
	{ "Callisto",                      NULL,             "AM3 (PGA-938)"  }, // 2009-2011, 45 nm
	{ "Rana",                          NULL,             "AM3 (PGA-938)"  }, // 2009-2011, 45 nm
	{ "Sargas",                        NULL,             "AM3 (PGA-938)"  }, // 2009-2011, 45 nm
	{ "Regor",                         NULL,             "AM3 (PGA-938)"  }, // 2009-2013, 45 nm
	  /* Mobile */
	{ "Caspian",                       NULL,             "S1g3 (PGA-638)" }, // 2009-2010, 45 nm
	{ "Champlain",                     NULL,             "S1g4 (PGA-638)" }, // 2010-2011, 45 nm
	{ "Geneva",                        NULL,             "S1g4 (PGA-638)" }, // 2010-2011, 45 nm
	/* Bulldozer */
	{ "Zambezi",                        NULL,            "AM3+ (PGA-942)" },
	{ "Vishera",                        NULL,            "AM3+ (PGA-942)" },
	{ "Kabini X4",                      "Athlon",        "AM1 (PGA-721)"  },
	{ "Kabini X4",                      "Sempron",       "AM1 (PGA-721)"  },
	{ "Trinity X4",                     NULL,            "FM2 (PGA-904)"  },
	/* Zen */
	{ "Naples",                         NULL,            "SP3 (LGA-4094)"   },
	{ "Whitehaven",                     NULL,            "SP3r2 (LGA-4094)" },
	{ "Summit Ridge",                   NULL,            "AM4 (PGA-1331)"   },
	{ "Raven Ridge",                    NULL,            "FP5 (BGA-1140)"   },
	{ "Dali",                           NULL,            "FP5 (BGA-1140)"   },
	/* Zen+ */
	{ "Colfax",                         NULL,            "SP3r2 (LGA-4094)" },
	{ "Pinnacle Ridge",                 NULL,            "AM4 (PGA-1331)"   },
	{ "Picasso",                        NULL,            "FP5 (BGA-1140)"   },
	/* Zen 2 */
	{ "Rome",                           NULL,            "SP3 (LGA-4094)"   },
	{ "Castle Peak",                    NULL,            "SP3r2 (LGA-4094)" },
	{ "Matisse",                        NULL,            "AM4 (PGA-1331)"   },
	{ "Renoir",                         NULL,            "FP6 (BGA-1140)"   },
	{ "Lucienne",                       NULL,            "FP6 (BGA-1140)"   },
	{ "Mendocino",                      NULL,            "FT6 (BGA)"        },
	/* Zen 3 */
	{ "Milan",                          NULL,            "SP3 (LGA-4094)"   },
	{ "Chagall",                        NULL,            "sWRX8 (LGA-4094)" },
	{ "Vermeer",                        NULL,            "AM4 (PGA-1331)"   },
	{ "Cezanne",                        NULL,            "FP6 (BGA-1140)"   },
	/* Zen 3+ */
	{ "Rembrandt",                      NULL,            "FP7 (BGA-1140)"   },
	/* Zen 4 */
	{ "Genoa",                          NULL,            "SP5 (LGA-6096)"   },
	{ "Storm Peak",                     NULL,            "sTR5 (LGA-4844)"  },
	{ "Raphael",                        NULL,            "AM5 (LGA-1718)"   },
	{ "Dragon Range",                   NULL,            "FL1 (BGA)"        },
	{ "Phoenix",                        NULL,            "FP7/FP8"          },
	{ "Hawk Point",                     NULL,            "FP7/FP8"          },
	{ "Granite Ridge",                  NULL,            "AM5 (LGA-1718)"   },
	{ "Strix Point",                    NULL,            "FP8"              },
	{ NULL,                             NULL,            NULL               }
	//Codename                          Model            Socket
};


#endif /* _DATABASES_H_ */
