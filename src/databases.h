/****************************************************************************
*    Copyright © 2014-2021 Xorg
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
	const int process;
} Technology_DB;

const Technology_DB technology_unknown[] = { { -1, -1, -1, -1 } };

const Technology_DB technology_intel[] =
{
	//Model        E. Model     E. Family   Process
	{  0,           0,          -1,         180 }, // P4 Willamette
	{  1,           1,           6,         350 }, // Pentium Pro
	{  1,           1,          15,         180 }, // P4 Willamette
	{  2,           2,          -1,         130 }, // P4 Northwood / Gallatin
	{  3,           3,           5,         350 }, // PII Overdrive
	{  3,           3,           6,         350 }, // PII Klamath
	{  3,           3,          15,          90 }, // P4 Prescott
	{  4,           4,          -1,          90 }, // P4 Prescott/Irwindale / PD Smithfield
	{  5,           5,           6,         250 }, // PII Deschutes / Tonga / Xeon Drake / Celeron Covington
	{  5,          37,          -1,          32 }, // Westmere
	{  5,          53,          -1,          32 }, // Atom Cloverview
	{  5,          69,          -1,          22 }, // Haswell
	{  5,          85,          -1,          14 }, // Skylake (Xeon Scalable)
	{  6,           6,           6,         250 }, // PII Dixon / Celeron Mendocino
	{  6,           6,          15,          65 }, // P4 Cedar Mill / PD Presler
	{  6,          22,          -1,          65 }, // C2 Conroe-L
	{  6,          54,          -1,          32 }, // Atom Cedarview
	{  6,          70,          -1,          22 }, // Haswell (Crystalwell, L4 cache)
	{  6,         102,          -1,          10 }, // Cannon Lake
	{  7,           7,          -1,         250 }, // PIII Katmai
	{  7,          23,          -1,          45 }, // C2 Wolfdale / Yorkfield / Penryn
	{  7,          55,          -1,          22 }, // Atom Bay Trail
	{  7,          71,          -1,          14 }, // Broadwell
	{  7,         167,          -1,          14 }, // Rocket Lake
	{  8,           0,           0,         180 }, // PIII Coppermine-T
	{  8,           8,          -1,         180 }, // PIII Coppermine
	{  9,           9,          -1,         130 }, // Pentium M Banias
	{ 10,          26,          -1,          45 }, // Nehalem
	{ 10,          30,          -1,          45 }, // Nehalem
	{ 10,          42,          -1,          32 }, // Sandy Bridge
	{ 10,          58,          -1,          22 }, // Ivy Bridge
	{ 10,         122,          -1,          14 }, // Gemini Lake
	{ 11,          11,          -1,         130 }, // PIII Tualatine
	{ 12,          28,          -1,          45 }, // Atom Diamondville / Pineview / Silverthorne
	{ 12,          44,          -1,          32 }, // Westmere
	{ 12,          60,          -1,          22 }, // Haswell
	{ 12,          76,          -1,          14 }, // Atom Cherry Trail
	{ 13,          13,          -1,          90 }, // Pentium M Dothan
	{ 13,          45,          -1,          32 }, // Sandy Bridge-E
	{ 13,          61,          -1,          14 }, // Broadwell-U
	{ 14,          14,          -1,          65 }, // Yonah (Core Solo)
	{ 14,          30,          -1,          45 }, // Nehalem (Lynnfield)
	{ 14,          62,          -1,          22 }, // Ivy Bridge-E
	{ 14,          78,          -1,          14 }, // Skylake
	{ 14,          94,          -1,          14 }, // Skylake
	{ 14,         126,          -1,          10 }, // Ice Lake
	{ 14,         142,          -1,          14 }, // Kaby Lake / Coffee Lake / Comet Lake-U
	{ 14,         158,          -1,          14 }, // Kaby Lake / Coffee Lake
	{ 14,         165,          -1,          14 }, // Comet Lake
	{ 15,          15,          -1,          65 }, // C2 Conroe / Allendale / Kentsfield / Merom
	{ 15,          63,          -1,          22 }, // Haswell-E
	{ 15,          79,          -1,          14 }, // Broadwell-E
	{ -2,          -2,          -2,          -2 }
	//Model        E. Model     E. Family   Process
};

const Technology_DB technology_amd[] =
{
	//Model        E. Model     E. Family   Process
	{  0,          16,          21,          32 }, // Bulldozer (Piledriver: Trinity)
	{  0,          48,          21,          28 }, // Bulldozer (Steamroller: Kaveri)
	{  0,         112,          21,          28 }, // Bulldozer (Excavator: Stoney Ridge)
	{  0,          -1,          22,          28 }, // Jaguar (Kabini/Mullins)
	{  1,          -1,          18,          32 }, // K10 (Llano)
	{  1,          -1,          20,          40 }, // Bobcat (Brazos Zacate)
	{  1,           1,          21,          32 }, // Bulldozer
	{  1,          96,          21,          28 }, // Bulldozer (Excavator: Carrizo)
	{  2,          -1,          16,          65 }, // K10 (Kuma/Agena)
	{  2,          -1,          20,          40 }, // Bobcat (Brazos Desna/Ontario)
	{  2,          -1,          21,          32 }, // Bulldozer (Piledriver: Vishera)
	{  3,          -1,          15,          90 }, // K8 (Toledo)
	{  3,          -1,          21,          32 }, // Bulldozer (Piledriver: Richland)
	{  4,          -1,          15,          90 }, // K8 (Lancaster)
	{  4,          -1,          16,          45 }, // K10 (Deneb/Heka)
	{  5,          -1,          16,          45 }, // K10 (Propus)
	{  5,          -1,          21,          28 }, // Bulldozer (Excavator: Bristol Ridge)
	{  6,          -1,          16,          45 }, // K10 (Champlain/Sargas/Regor)
	{  8,          -1,           6,         130 }, // K7 (Thoroughbred/Applebred)
	{  8,          -1,          15,          65 }, // K8 (Tyler)
	{  8,          -1,          21,          28 }, // Bulldozer (Steamroller: Godavari)
	{  9,          -1,          16,          45 }, // Magny-Cours (Opteron)
	{ 10,          -1,           6,         130 }, // K7 (Barton)
	{ 10,          -1,          16,          45 }, // K10 (Thuban)
	{ 11,          -1,          15,          65 }, // K8 (Brisbane)
	{ 12,          -1,          15,          90 }, // K8 (Venice/Sonora)
	{ 15,          79,          15,          90 }, // K8 (Manila)
	{ 15,         127,          15,          65 }, // K8 (Sparta)
	{ -1,           1,          23,          14 }, // Zen (Summit Ridge/Whitehaven/Naples)
	{ -1,          17,          23,          14 }, // Zen (Raven Ridge)
	{ -1,           8,          23,          12 }, // Zen+ (Pinnacle Ridge/Colfax)
	{ -1,          24,          23,          12 }, // Zen+ (Picasso)
	{ -1,          49,          23,           7 }, // Zen 2 (Rome/Castle Peak)
	{ -1,          96,          23,           7 }, // Zen 2 (Renoir)
	{ -1,         113,          23,           7 }, // Zen 2 (Matisse)
	{ -1,          33,          25,           7 }, // Zen 3 (Vermeer)
	{ -1,          80,          25,           7 }, // Zen 3 (Cezanne)
	{ -1,           1,          25,           7 }, // Zen 3 (Milan)

	{ -2,          -2,          -2,          -2 }
	//Model        E. Model     E. Family   Process
};


/****************************** CPU Package ******************************/

typedef struct {
	const char *codename;
	const char *model;
	const char *socket;
} Package_DB;

const Package_DB package_unknown[] = { { NULL, NULL, NULL } };

const Package_DB package_intel[] =
{
	//Codename                          Model                               Socket
	{ "Atom (Diamondville)",            NULL,                               "BGA 437"        },
	{ "Pentium D (SmithField)",         NULL,                               "LGA 775"        },
	{ "Pentium D (Presler)",            NULL,                               "LGA 775"        },
	{ "Bloomfield",                     NULL,                               "LGA 1366"       },
	{ NULL,                             "Intel(R) Core(TM) i5-2520M CPU",   "rPGA 988B"      },
	{ NULL,                             "Intel(R) Xeon(R) CPU E31275",      "LGA 1155"       },
	{ NULL,                             "Intel(R) Core(TM) i7-2860QM CPU",  "rPGA 988B"      },
	{ NULL,                             "Intel(R) Core(TM) i7-3740QM CPU",  "rPGA 988B"      },
	{ NULL,                             "Intel(R) Core(TM) i5-4300U CPU",   "BGA 1168"       },
	{ NULL,                             "Intel(R) Core(TM) i7-5775C CPU",   "LGA 1150"       },
	{ NULL,                             "Intel(R) Core(TM) i7-6560U CPU",   "FCBGA 1356"     },
	{ NULL,                             "Intel(R) Core(TM) i7-7300U CPU",   "FCBGA 1356"     },
	{ NULL,                             "Intel(R) Core(TM) i7-8250U CPU",   "FCBGA 1356"     },
	{ NULL,                             "Intel(R) Core(TM) i7-8350U CPU",   "FCBGA 1356"     },
	{ NULL,                             "Intel(R) Core(TM) i7-8550U CPU",   "FCBGA 1356"     },
	{ NULL,                             "Intel(R) Core(TM) i7-8650U CPU",   "FCBGA 1356"     },
	{ NULL,                             "Intel(R) Core(TM) i9-9900K CPU",   "LGA 1151"       },
	{ NULL,                             NULL,                               NULL             }
	//Codename                          Model                               Socket
};

const Package_DB package_amd[] =
{
	//Codename                          Model            Socket
	{ "Athlon 64 FX X2 (Toledo)",       NULL,            "939 (PGA-ZIF)"  },
	{ "Kabini X4",                      "Athlon",        "AM1 (PGA-ZIF)"  },
	{ "Kabini X4",                      "Sempron",       "AM1 (PGA-ZIF)"  },
	{ "Trinity X4",                     NULL,            "FM2 (PGA-ZIF)"  },
	{ "Turion X2",                      "TL",            "S1g1 (PGA-ZIF)" },
	{ "Turion X2",                      "TK",            "S1g1 (PGA-ZIF)" },
	{ "Deneb",    "AMD Phenom(tm) II X4 945",            "AM3 (PGA-938"   },
	{ "Zambezi",                        NULL,            "AM3+ (PGA-942)" },
	{ "Vishera",                        NULL,            "AM3+ (PGA-942)" },
	/* Zen */
	{ "Summit Ridge",                   NULL,            "AM4 (PGA-1331)"   },
	{ "Whitehaven",                     NULL,            "SP3r2 (LGA-4094)" },
	{ "Naples",                         NULL,            "SP3 (LGA-4094)"   },
	/* Zen+ */
	{ "Picasso",                        NULL,            "FP5 (BGA)"        },
	{ "Pinnacle Ridge",                 NULL,            "AM4 (PGA-1331)"   },
	{ "Colfax",                         NULL,            "SP3r2 (LGA-4094)" },
	/* Zen 2 */
	{ "Matisse",                        NULL,            "AM4 (PGA-1331)"   },
	{ "Castle Peak",                    NULL,            "SP3r2 (LGA-4094)" },
	{ "Rome",                           NULL,            "SP3 (LGA-4094)"   },
	{ NULL,                             NULL,            NULL               }
	//Codename                          Model            Socket
};


#endif /* _DATABASES_H_ */
