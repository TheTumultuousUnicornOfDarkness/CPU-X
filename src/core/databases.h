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
* FILE core/databases.h
*/

#ifndef _CORE_DATABASES_H_
#define _CORE_DATABASES_H_


/****************************** CPU Package ******************************/

typedef struct {
	const char *codename;
	const char *model;
	const char *socket;
} Package_DB;

const Package_DB package_intel[] = {
	// Codename                          Model                               Socket
	{ "Atom (Diamondville)",    NULL,                                "BGA 437"    },
	{ "Pentium D (SmithField)", NULL,                                "LGA 775"    },
	{ "Pentium D (Presler)",    NULL,                                "LGA 775"    },
	{ "Bloomfield",             NULL,                                "LGA 1366"   },
	{ NULL,                     "Intel(R) Core(TM)2 Duo CPU E7400",  "LGA 775"    },
	{ NULL,                     "Intel(R) Core(TM)2 Quad CPU Q6600", "LGA 775"    },
	{ NULL,                     "Intel(R) Core(TM) i5-2500K CPU",    "LGA 1155"   },
	{ NULL,                     "Intel(R) Celeron(R) CPU G550T",     "LGA 1155"   },
	{ NULL,                     "Intel(R) Celeron(R) CPU G1610",     "LGA 1155"   },
	{ NULL,                     "Intel(R) Pentium(R) CPU G620",      "LGA 1155"   },
	{ NULL,                     "Intel(R) Pentium(R) CPU G2030",     "LGA 1155"   },
	{ NULL,                     "Intel(R) Core(TM) i5-2520M CPU",    "rPGA 988B"  },
	{ NULL,                     "Intel(R) Core(TM) i5-3360M CPU",    "rPGA 988B"  },
	{ NULL,                     "Intel(R) Xeon(R) CPU E31275",       "LGA 1155"   },
	{ NULL,                     "Intel(R) Core(TM) i7-2860QM CPU",   "rPGA 988B"  },
	{ NULL,                     "Intel(R) Core(TM) i7-3740QM CPU",   "rPGA 988B"  },
	{ NULL,                     "Intel(R) Core(TM) i5-4300U CPU",    "BGA 1168"   },
	{ NULL,                     "Intel(R) Core(TM) i7-4600U CPU",    "BGA 1168"   },
	{ NULL,                     "Intel(R) Core(TM) i7-4790 CPU",     "LGA 1150"   },
	{ NULL,                     "Intel(R) Core(TM) i7-5775C CPU",    "LGA 1150"   },
	{ NULL,                     "Intel(R) Core(TM) i7-6560U CPU",    "BGA 1356"   },
	{ NULL,                     "Intel(R) Core(TM) i5-6400 CPU",     "LGA 1151"   },
	{ NULL,                     "Intel(R) Core(TM) i7-7300U CPU",    "BGA 1356"   },
	{ NULL,                     "Intel(R) Core(TM) i7-7820HQ CPU",   "BGA 1440"   },
	{ NULL,                     "Intel(R) Core(TM) i7-8250U CPU",    "BGA 1356"   },
	{ NULL,                     "Intel(R) Core(TM) i7-8350U CPU",    "BGA 1356"   },
	{ NULL,                     "Intel(R) Core(TM) i7-8550U CPU",    "BGA 1356"   },
	{ NULL,                     "Intel(R) Core(TM) i7-8650U CPU",    "BGA 1356"   },
	{ NULL,                     "Intel(R) Core(TM) i5-9300H CPU",    "BGA 1440"   },
	{ NULL,                     "Intel(R) Core(TM) i7-9700 CPU",     "LGA 1151"   },
	{ NULL,                     "Intel(R) Core(TM) i9-9900K CPU",    "LGA 1151"   },
	{ NULL,                     "Intel(R) Core(TM) i5-10500 CPU",    "LGA 1200"   },
	{ NULL,                     "Intel(R) Core(TM) i5-1135G7",       "BGA 1449"   },
	{ NULL,                     "Intel(R) Core(TM) i7-1165G7",       "BGA 1449"   },
	{ NULL,                     "Intel(R) Core(TM) i7-11700K",       "LGA 1200"   },
	{ NULL,                     "Intel(R) Core(TM) i9-11900K",       "LGA 1200"   },
	{ NULL,                     "Intel(R) Core(TM) i3-1220P",        "BGA 1744"   },
	{ NULL,                     "Intel(R) Core(TM) i5-12400",        "LGA 1700"   },
	{ NULL,                     "Intel(R) Core(TM) i7-12700H",       "BGA 1744"   },
	{ NULL,                     "Intel(R) Core(TM) i7-12700KF",      "LGA 1700"   },
	{ NULL,                     "Intel(R) Core(TM) i9-12900K",       "LGA 1700"   },
	{ NULL,                     "Intel(R) Core(TM) i9-12900HK",      "BGA 1744"   },
	{ NULL,                     "Intel(R) Core(TM) i5-13500T",       "LGA 1700"   },
	{ NULL,                     "Intel(R) Core(TM) i5-13600T",       "LGA 1700"   },
	{ NULL,                     "Intel(R) Core(TM) i5-13600K",       "LGA 1700"   },
	{ NULL,                     "Intel(R) Core(TM) i7-13700K",       "LGA 1700"   },
	{ NULL,                     "Intel(R) Core(TM) i9-13900K",       "LGA 1700"   },
	{ NULL,                     "Intel(R) Celeron(R) CPU N2930",     "BGA 1170"   },
	{ NULL,                     "Intel(R) Celeron(R) CPU N3350",     "BGA 1296"   },
	{ NULL,                     "Intel(R) Celeron(R) N4000",         "BGA 1090"   },
	{ NULL,                     "Intel(R) Core(TM) i3-3240 CPU",     "LGA 1155"   },
	{ NULL,                     "Intel(R) Core(TM) i5-3470 CPU",     "LGA 1155"   },
	{ NULL,                     "Intel(R) Core(TM) i5-3570 CPU",     "LGA 1155"   },
	{ NULL,                     "Intel(R) Core(TM) i7-4770 CPU",     "LGA 1155"   },
	{ NULL,                     "Intel(R) Core(TM) i3-4350 CPU",     "LGA 1150"   },
	{ NULL,                     "Intel(R) Xeon(R) Gold 5220 CPU",    "LGA 3647"   },
	{ NULL,                     "Intel(R) Xeon(R) CPU E5-2689",      "LGA 2011"   },
	{ NULL,                     "Intel(R) Xeon(R) CPU E5-2690 ",     "LGA 2011"   },
	{ NULL,                     "Intel(R) Xeon(R) CPU E5-2690 v2",   "LGA 2011"   },
	{ NULL,                     "Intel(R) Xeon(R) CPU E5-2690 v3",   "LGA 2011-3" },
	{ NULL,                     "Intel(R) Xeon(R) CPU E5-2690 v4",   "LGA 2011-3" },
	/* ArrowLake */
	{ NULL,                     "Intel(R) Core(TM) Ultra 9 285k",   "LGA 1851" },
	{ NULL,                     "Intel(R) Core(TM) Ultra 7 265k",   "LGA 1851" },
	{ NULL,                     "Intel(R) Core(TM) Ultra 5 245k",   "LGA 1851" },
	{ NULL,                     NULL,                                NULL         }
	// Codename                          Model                               Socket
};

const Package_DB package_amd[] = {
	// Codename                          Model            Socket
	/* K8 */
	{ "Athlon 64 FX X2 (Toledo)", NULL,                       "939 (PGA-939)"    },
	{ "Turion X2",                "TL",                       "S1g1 (PGA-638)"   },
	{ "Turion X2",                "TK",                       "S1g1 (PGA-638)"   },
	/* K10 */
	/* Desktop */
	{ "Kuma",                     NULL,                       "AM2+ (PGA-940)"   }, // 2008-2009, 65 nm
	{ "Agena",                    NULL,                       "AM2+ (PGA-940)"   }, // 2007-2008, 65 nm
	{ "Toliman",                  NULL,                       "AM2+ (PGA-940)"   }, // 2008, 65 nm
	{ "Thuban",                   NULL,                       "AM3 (PGA-938)"    }, // 2010, 45 nm
	{ "Zosma",                    NULL,                       "AM3 (PGA-938)"    }, // 2010-2011, 45 nm
	{ "Deneb",                    "AMD Phenom(tm) II X4 920", "AM2+ (PGA-940)"   }, // 2009, 45 nm
	{ "Deneb",                    "AMD Phenom(tm) II X4 940", "AM2+ (PGA-940)"   }, // 2009, 45 nm
	{ "Deneb",                    NULL,                       "AM3 (PGA-938)"    }, // 2009-2011, 45 nm
	{ "Propus",                   NULL,                       "AM3 (PGA-938)"    }, // 2009-2011, 45 nm
	{ "Heka",                     "AMD Phenom(tm) II X3 715", "AM2+ (PGA-940)"   }, // 2009, 45 nm
	{ "Heka",                     NULL,                       "AM3 (PGA-938)"    }, // 2009-2010, 45 nm
	{ "Callisto",                 NULL,                       "AM3 (PGA-938)"    }, // 2009-2011, 45 nm
	{ "Rana",                     NULL,                       "AM3 (PGA-938)"    }, // 2009-2011, 45 nm
	{ "Sargas",                   NULL,                       "AM3 (PGA-938)"    }, // 2009-2011, 45 nm
	{ "Regor",                    NULL,                       "AM3 (PGA-938)"    }, // 2009-2013, 45 nm
	/* Mobile */
	{ "Caspian",                  NULL,                       "S1g3 (PGA-638)"   }, // 2009-2010, 45 nm
	{ "Champlain",                NULL,                       "S1g4 (PGA-638)"   }, // 2010-2011, 45 nm
	{ "Geneva",                   NULL,                       "S1g4 (PGA-638)"   }, // 2010-2011, 45 nm
	/* Bulldozer */
	{ "Zambezi",                  NULL,                       "AM3+ (PGA-942)"   },
	{ "Vishera",                  NULL,                       "AM3+ (PGA-942)"   },
	{ "Kabini X4",                "Athlon",                   "AM1 (PGA-721)"    },
	{ "Kabini X4",                "Sempron",                  "AM1 (PGA-721)"    },
	{ "Trinity X4",               NULL,                       "FM2 (PGA-904)"    },
	/* Zen */
	{ "Naples",                   NULL,                       "SP3 (LGA-4094)"   },
	{ "Whitehaven",               NULL,                       "SP3r2 (LGA-4094)" },
	{ "Summit Ridge",             NULL,                       "AM4 (PGA-1331)"   },
	{ "Raven Ridge",              NULL,                       "FP5 (BGA-1140)"   },
	{ "Dali",                     NULL,                       "FP5 (BGA-1140)"   },
	/* Zen+ */
	{ "Colfax",                   NULL,                       "SP3r2 (LGA-4094)" },
	{ "Pinnacle Ridge",           NULL,                       "AM4 (PGA-1331)"   },
	{ "Picasso",                  NULL,                       "FP5 (BGA-1140)"   },
	/* Zen 2 */
	{ "Rome",                     NULL,                       "SP3 (LGA-4094)"   },
	{ "Castle Peak",              NULL,                       "SP3r2 (LGA-4094)" },
	{ "Matisse",                  NULL,                       "AM4 (PGA-1331)"   },
	{ "Renoir",                   NULL,                       "FP6 (BGA-1140)"   },
	{ "Lucienne",                 NULL,                       "FP6 (BGA-1140)"   },
	{ "Mendocino",                NULL,                       "FT6 (BGA)"        },
	/* Zen 3 */
	{ "Milan",                    NULL,                       "SP3 (LGA-4094)"   },
	{ "Chagall",                  NULL,                       "sWRX8 (LGA-4094)" },
	{ "Vermeer",                  NULL,                       "AM4 (PGA-1331)"   },
	{ "Cezanne",                  NULL,                       "FP6 (BGA-1140)"   },
	/* Zen 3+ */
	{ "Rembrandt",                NULL,                       "FP7 (BGA-1140)"   },
	/* Zen 4 */
	{ "Genoa",                    NULL,                       "SP5 (LGA-6096)"   },
	{ "Storm Peak",               NULL,                       "sTR5 (LGA-4844)"  },
	{ "Raphael",                  NULL,                       "AM5 (LGA-1718)"   },
	{ "Dragon Range",             NULL,                       "FL1 (BGA)"        },
	{ "Phoenix",                  NULL,                       "FP7/FP8"          },
	{ "Hawk Point",               NULL,                       "FP7/FP8"          },
	/* Zen 5 */
	{ "Turin",                    NULL,                       "SP5 (LGA-6096)"   },
	{ "Granite Ridge",            NULL,                       "AM5 (LGA-1718)"   },
	{ "Strix Point",              NULL,                       "FP8"              },
	{ "Krackan Point",            NULL,                       "FP8"              },
	{ "Strix Halo",               NULL,                       "FP11"             },

	{ NULL,                       NULL,                       NULL               }
	// Codename                          Model            Socket
};


#endif /* _CORE_DATABASES_H_ */
