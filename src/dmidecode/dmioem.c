/*
 * Decoding of OEM-specific entries
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2007-2020 Jean Delvare <jdelvare@suse.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "util.h"
#include "dmidecode.h"
#include "dmioem.h"
#include "dmiopt.h"
#include "dmioutput.h"

/*
 * Globals for vendor-specific decodes
 */

enum DMI_VENDORS
{
	VENDOR_UNKNOWN,
	VENDOR_ACER,
	VENDOR_HP,
	VENDOR_HPE,
	VENDOR_IBM,
	VENDOR_LENOVO,
};

static enum DMI_VENDORS dmi_vendor = VENDOR_UNKNOWN;
static const char *dmi_product = NULL;

/*
 * Remember the system vendor for later use. We only actually store the
 * value if we know how to decode at least one specific entry type for
 * that vendor.
 */
void dmi_set_vendor(const char *v, const char *p)
{
	const struct { const char *str; enum DMI_VENDORS id; } vendor[] = {
		{ "Acer",			VENDOR_ACER },
		{ "HP",				VENDOR_HP },
		{ "Hewlett-Packard",		VENDOR_HP },
		{ "HPE",			VENDOR_HPE },
		{ "Hewlett Packard Enterprise",	VENDOR_HPE },
		{ "IBM",			VENDOR_IBM },
		{ "LENOVO",			VENDOR_LENOVO },
	};
	unsigned int i;
	size_t len;

	/*
	 * Often DMI strings have trailing spaces. Ignore these
	 * when checking for known vendor names.
	 */
	len = v ? strlen(v) : 0;
	while (len && v[len - 1] == ' ')
		len--;

	for (i = 0; i < ARRAY_SIZE(vendor); i++)
	{
		if (strlen(vendor[i].str) == len &&
		    strncmp(v, vendor[i].str, len) == 0)
		{
			dmi_vendor = vendor[i].id;
			break;
		}
	}

	dmi_product = p;
}

/*
 * Acer-specific data structures are decoded here.
 */

static int dmi_decode_acer(const struct dmi_header *h)
{
	u8 *data = h->data;
	u16 cap;

	switch (h->type)
	{
		case 170:
			/*
			 * Vendor Specific: Acer Hotkey Function
			 *
			 * Source: acer-wmi kernel driver
			 *
			 * Probably applies to some laptop models of other
			 * brands, including Fujitsu-Siemens, Medion, Lenovo,
			 * and eMachines.
			 */
			pr_handle_name("Acer Hotkey Function");
			if (h->length < 0x0F) break;
			cap = WORD(data + 0x04);
			pr_attr("Function bitmap for Communication Button", "0x%04hx", cap);
			pr_subattr("WiFi", "%s", cap & 0x0001 ? "Yes" : "No");
			pr_subattr("3G", "%s", cap & 0x0040 ? "Yes" : "No");
			pr_subattr("WiMAX", "%s", cap & 0x0080 ? "Yes" : "No");
			pr_subattr("Bluetooth", "%s", cap & 0x0800 ? "Yes" : "No");
			pr_attr("Function bitmap for Application Button", "0x%04hx", WORD(data + 0x06));
			pr_attr("Function bitmap for Media Button", "0x%04hx", WORD(data + 0x08));
			pr_attr("Function bitmap for Display Button", "0x%04hx", WORD(data + 0x0A));
			pr_attr("Function bitmap for Others Button", "0x%04hx", WORD(data + 0x0C));
			pr_attr("Communication Function Key Number", "%d", data[0x0E]);
			break;

		default:
			return 0;
	}
	return 1;
}

/*
 * HPE-specific data structures are decoded here.
 *
 * Code contributed by John Cagle and Tyler Bell.
 */

static void dmi_print_hp_net_iface_rec(u8 id, u8 bus, u8 dev, const u8 *mac)
{
	/* Some systems do not provide an id. nic_ctr provides an artificial
	 * id, and assumes the records will be provided "in order".  Also,
	 * using 0xFF marker is not future proof. 256 NICs is a lot, but
	 * 640K ought to be enough for anybody(said no one, ever).
	 * */
	static u8 nic_ctr;
	char attr[8];

	if (id == 0xFF)
		id = ++nic_ctr;

	sprintf(attr, "NIC %hhu", id);
	if (dev == 0x00 && bus == 0x00)
		pr_attr(attr, "Disabled");
	else if (dev == 0xFF && bus == 0xFF)
		pr_attr(attr, "Not Installed");
	else
	{
		pr_attr(attr, "PCI device %02x:%02x.%x, "
			"MAC address %02X:%02X:%02X:%02X:%02X:%02X",
			bus, dev >> 3, dev & 7,
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
}

typedef enum { G6 = 6, G7, G8, G9, G10, G10P } dmi_hpegen_t;

static int dmi_hpegen(const char *s)
{
	struct { const char *name; dmi_hpegen_t gen; } table[] = {
		{ "Gen10 Plus",	G10P },
		{ "Gen10",	G10 },
		{ "Gen9",	G9 },
		{ "Gen8",	G8 },
		{ "G7",		G7 },
		{ "G6",		G6 },
	};
	unsigned int i;

	if (!strstr(s, "ProLiant") && !strstr(s, "Apollo") &&
	    !strstr(s, "Synergy")  && !strstr(s, "Edgeline"))
		return -1;

	for (i = 0; i < ARRAY_SIZE(table); i++) {
		if (strstr(s, table[i].name))
			return(table[i].gen);
	}

	return (dmi_vendor == VENDOR_HPE) ? G10P : G6;
}

static void dmi_hp_240_attr(u64 defined, u64 set)
{
	static const char *attributes[] = {
		"Updatable",
		"Reset Required",
		"Authentication Required",
		"In Use",
		"UEFI Image",
	};
	unsigned int i;

	pr_list_start("Attributes Defined/Set", NULL);
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
	{
		if (!(defined.l & (1UL << i)))
			continue;
		pr_list_item("%s: %s", attributes[i], set.l & (1UL << i) ? "Yes" : "No");
	}
	pr_list_end();
}

static void dmi_hp_203_assoc_hndl(const char *fname, u16 num)
{
	if (opt.flags & FLAG_QUIET)
		return;

	if (num == 0xFFFE)
		pr_attr(fname, "N/A");
	else
		pr_attr(fname, "0x%04X", num);
}

static void dmi_hp_203_pciinfo(const char *fname, u16 num)
{
	if (num == 0xFFFF)
		pr_attr(fname, "Device Not Present");
	else
		pr_attr(fname, "0x%04x", num);
}

static void dmi_hp_203_bayenc(const char *fname, u8 num)
{
	switch (num)
	{
		case 0x00:
			pr_attr(fname, "Unknown");
			break;
		case 0xff:
			pr_attr(fname, "Do Not Display");
			break;
		default:
			pr_attr(fname, "%d", num);
	}
}

static void dmi_hp_203_devtyp(const char *fname, unsigned int code)
{
	const char *str = "Reserved";
	static const char *type[] = {
		"Unknown", /* 0x00 */
		"Reserved",
		"Reserved",
		"Flexible LOM",
		"Embedded LOM",
		"NIC in a Slot",
		"Storage Controller",
		"Smart Array Storage Controller",
		"USB Hard Disk",
		"Other PCI Device",
		"RAM Disk",
		"Firmware Volume",
		"UEFI Shell",
		"Generic UEFI USB Boot Entry",
		"Dynamic Smart Array Controller",
		"File",
		"NVME Hard Drive",
		"NVDIMM" /* 0x11 */
	};

	if (code < ARRAY_SIZE(type))
		str = type[code];

	pr_attr(fname, "%s", str);
}

static void dmi_hp_203_devloc(const char *fname, unsigned int code)
{
	const char *str = "Reserved";
	static const char *location[] = {
		"Unknown", /* 0x00 */
		"Embedded",
		"iLO Virtual Media",
		"Front USB Port",
		"Rear USB Port",
		"Internal USB",
		"Internal SD Card",
		"Internal Virtual USB (Embedded NAND)",
		"Embedded SATA Port",
		"Embedded Smart Array",
		"PCI Slot",
		"RAM Memory",
		"USB",
		"Dynamic Smart Array Controller",
		"URL",
		"NVMe Drive Bay" /* 0x0F */
	};

	if (code < ARRAY_SIZE(location))
		str = location[code];

	pr_attr(fname, "%s", str);
}

static int dmi_hp_224_status(u8 code)
{
	static const char * const present[] = {
		"Not Present", /* 0x00 */
		"Present/Enabled",
		"Present/Disabled",
		"Reserved" /* 0x03 */
	};

	pr_attr("Status", "%s", present[code & 0x03]);
	if ((code & 0x03) == 0x00)
		return 0;
	pr_attr("Option ROM Measuring", "%s", (code & (1 << 2)) ? "Yes" : "No");
	pr_attr("Hidden", "%s", (code & (1 << 3)) ? "Yes" : "No");
	return 1;
}

static void dmi_hp_224_ex_status(u8 status, u8 code)
{
	const char *str = "Reserved";
	static const char * const disable_reason[] = {
		"Not Specified", /* 0x00 */
		"User Disabled",
		"Error Condition",
		"Reserved"	/* 0x03 */
	};
	static const char * const error_condition[] = {
		"Not Specified", /* 0x00 */
		"Self-Test",     /* 0x01 */
	};
	if ((status & 0x03) == 0x02)
		pr_attr("Disable Reason", "%s", disable_reason[code & 0x03]);
	if ((code & 0x03) == 0x02) {
		u8 error = (code >> 2) & 0x0f;
		if (error < ARRAY_SIZE(error_condition))
			str = error_condition[error];
		pr_attr("Error Condition", "%s", str);
	}
}

static void dmi_hp_224_module_type(u8 code)
{
	const char *str = "Reserved";
	static const char * const type[] = {
		"Not Specified", /* 0x00 */
		"TPM 1.2",
		"TPM 2.0",
		"Intel PTT fTPM" /* 0x03 */
	};
	if ((code & 0x0f) < ARRAY_SIZE(type))
		str = type[code & 0x0f];
	pr_attr("Type", "%s", str);
	pr_attr("Standard Algorithm Supported", "%s", (code & (1 << 4)) ? "Yes" : "No");
	pr_attr("Chinese Algorithm Supported", "%s", (code & (1 << 5)) ? "Yes" : "No");
}

static void dmi_hp_224_module_attr(u8 code)
{
	static const char * const phys_attr[] = {
		"Not Specified", /* 0x00 */
		"Pluggable and Optional",
		"Pluggable but Standard",
		"Soldered Down on System Board"  /* 0x03 */
	};
	static const char * const fips_attr[] = {
		"Not Specified", /* 0x00 */
		"Not FIPS Certified",
		"FIPS Certified",
		"Reserved"  /* 0x03 */
	};
	pr_attr("Trusted Module Attributes", "%s", phys_attr[code & 0x3]);
	pr_attr("FIPS Certification", "%s", fips_attr[((code >> 2) & 0x03)]);
}

static void dmi_hp_224_chipid(u16 code)
{
	const char *str = "Reserved";
	static const char * const chipid[] = {
		"None", /* 0x00 */
		"STMicroGen10 TPM",
		"Intel firmware TPM (PTT)",
		"Nationz TPM",
		"STMicroGen10 Plus TPM",
		"STMicroGen11 TPM", /* 0x05 */
	};
	if ((code & 0xff) < ARRAY_SIZE(chipid))
		str = chipid[code & 0xff];
	pr_attr("Chip Identifier", "%s", str);
}

static void dmi_hp_230_method_bus_seg_addr(u8 code, u8 bus_seg, u8 addr)
{
	const char *str = "Reserved";
	static const char * const method[] = {
		"Not Available", /* 0x00 */
		"IPMI I2C",
		"iLO",
		"Chassis Manager", /* 0x03 */
	};
	if (code < ARRAY_SIZE(method))
		str = method[code];
	pr_attr("Access Method", "%s", str);
	if (code == 0 || code >= ARRAY_SIZE(method))
		return;
	if (bus_seg != 0xFF)
	{
		if (code == 2)
			pr_attr("I2C Segment Number", "%d", bus_seg);
		else
			pr_attr("I2C Bus Number", "%d", bus_seg);
	}
	if (addr != 0xFF)
		pr_attr("I2C Address", "0x%02x", addr >> 1);
}

static void dmi_hp_238_loc(const char *fname, unsigned int code)
{
	const char *str = "Reserved";
	static const char *location[] = {
		"Internal", /* 0x00 */
		"Front of Server",
		"Rear of Server",
		"Embedded internal SD Card",
		"iLO USB",
		"USB Hub for NAND Controller",
		"Reserved",
		"Debug Port", /* 0x07 */
	};

	if (code < ARRAY_SIZE(location))
		str = location[code];

	pr_attr(fname, "%s", str);
}

static void dmi_hp_238_flags(const char *fname, unsigned int code)
{
	const char *str = "Reserved";
	static const char *flags[] = {
		"Not Shared", /* 0x00 */
		"Shared with physical switch",
		"Shared with automatic control", /* 0x02 */
	};

	if (code < ARRAY_SIZE(flags))
		str = flags[code];

	pr_attr(fname, "%s", str);
}

static void dmi_hp_238_speed(const char *fname, unsigned int code)
{
	const char *str = "Reserved";
	static const char *speed[] = {
		"Reserved", /* 0x00 */
		"USB 1.1 Full Speed",
		"USB 2.0 High Speed",
		"USB 3.0 Super Speed" /* 0x03 */
	};

	if (code < ARRAY_SIZE(speed))
		str = speed[code];

	pr_attr(fname, "%s", str);
}

static int dmi_decode_hp(const struct dmi_header *h)
{
	u8 *data = h->data;
	int nic, ptr;
	u32 feat;
	const char *company = (dmi_vendor == VENDOR_HP) ? "HP" : "HPE";
	int gen;

	gen = dmi_hpegen(dmi_product);
	if (gen < 0)
		return 0;

	switch (h->type)
	{
		case 194:
			/*
			 * Vendor Specific: Super IO Enable/Disable Features
			 *
			 * Offset |  Name      | Width | Description
			 * -------------------------------------
			 *  0x00  | Type       | BYTE  | 0xC2, Super IO Enable/Disable Indicator
			 *  0x01  | Length     | BYTE  | Length of structure
			 *  0x02  | Handle     | WORD  | Unique handle
			 *  0x04  | Dev Status | BYTE  | Device Status
			 */
			pr_handle_name("%s ProLiant Super IO Enable/Disable Indicator", company);
			if (h->length < 0x05) break;
			feat = data[0x04];
			pr_attr("Serial Port A", "%s", feat & (1 << 0) ? "Enabled" : "Disabled");
			pr_attr("Serial Port B", "%s", feat & (1 << 1) ? "Enabled" : "Disabled");
			pr_attr("Parallel Port", "%s", feat & (1 << 2) ? "Enabled" : "Disabled");
			pr_attr("Floppy Disk Port", "%s", feat & (1 << 3) ? "Enabled" : "Disabled");
			pr_attr("Virtual Serial Port", "%s", feat & (1 << 4) ? "Enabled" : "Disabled");
			break;

		case 199:
			/*
			 * Vendor Specific: CPU Microcode Patch
			 *
			 * Offset |  Name      | Width | Description
			 * -------------------------------------
			 *  0x00  | Type       | BYTE  | 0xC7, CPU Microcode Patch
			 *  0x01  | Length     | BYTE  | Length of structure
			 *  0x02  | Handle     | WORD  | Unique handle
			 *  0x04  | Patch Info | Varies| { <DWORD: ID, DWORD Date, DWORD CPUID> ...}
			 */
			if (gen < G9) return 0;
			pr_handle_name("%s ProLiant CPU Microcode Patch Support Info", company);

			for (ptr = 0x4; ptr + 12 <= h->length; ptr += 12) {
				u32 cpuid = DWORD(data + ptr + 2 * 4);
				u32 date;

				/* AMD omits BaseFamily. Reconstruction valid on family >= 15. */
				if (cpuid_type == cpuid_x86_amd)
					cpuid = ((cpuid & 0xfff00) << 8) | 0x0f00 | (cpuid & 0xff);

				dmi_print_cpuid(pr_attr, "CPU ID", cpuid_type, (u8 *) &cpuid);

				date = DWORD(data + ptr + 4);
				pr_subattr("Date", "%04x-%02x-%02x",
					date & 0xffff, (date >> 24) & 0xff, (date >> 16) & 0xff);
				pr_subattr("Patch", "0x%X", DWORD(data + ptr));
			}
			break;

		case 203:
			/*
			 * Vendor Specific: HP Device Correlation Record
			 *
			 * Offset |  Name        | Width | Description
			 * -------------------------------------
			 *  0x00  | Type         | BYTE  | 0xCB, Correlation Record
			 *  0x01  | Length       | BYTE  | Length of structure
			 *  0x02  | Handle       | WORD  | Unique handle
			 *  0x04  | Assoc Device | WORD  | Handle of Associated Type 9 or Type 41 Record
			 *  0x06  | Assoc SMBus  | WORD  | Handle of Associated Type 228 SMBus Segment Record
			 *  0x08  | PCI Vendor ID| WORD  | PCI Vendor ID of device 0xFFFF -> not present
			 *  0x0A  | PCI Device ID| WORD  | PCI Device ID of device 0xFFFF -> not present
			 *  0x0C  | PCI SubVendor| WORD  | PCI Sub Vendor ID of device 0xFFFF -> not present
			 *  0x0E  | PCI SubDevice| WORD  | PCI Sub Device ID of device 0xFFFF -> not present
			 *  0x10  | Class Code   | BYTE  | PCI Class Code of Endpoint. 0xFF if device not present.
			 *  0x11  | Class SubCode| BYTE  | PCI Sub Class Code of Endpoint. 0xFF if device not present.
			 *  0x12  | Parent Handle| WORD  |
			 *  0x14  | Flags        | WORD  |
			 *  0x16  | Device Type  | BYTE  | UEFI only
			 *  0x17  | Device Loc   | BYTE  | Device Location
			 *  0x18  | Dev Instance | BYTE  | Device Instance
			 *  0x19  | Sub Instance | BYTE  | NIC Port # or NVMe Drive Bay
			 *  0x1A  | Bay          | BYTE  |
			 *  0x1B  | Enclosure    | BYTE  |
			 *  0x1C  | UEFI Dev Path| STRING| String number for UEFI Device Path
			 *  0x1D  | Struct Name  | STRING| String number for UEFI Device Structured Name
			 *  0x1E  | Device Name  | STRING| String number for UEFI Device Name
			 *  0x1F  | UEFI Location| STRING| String number for UEFI Location
			 *  0x20  | Assoc Handle | WORD  | Type 9 Handle.  Defined if Flags[0] == 1.
			 *  0x22  | Part Number  | STRING| PCI Device Part Number
			 *  0x23  | Serial Number| STRING| PCI Device Serial Number
			 *  0x24  | Seg Number   | WORD  | Segment Group number. 0 -> Single group topology
			 *  0x26  | Bus Number   | BYTE  | PCI Device Bus Number
			 *  0x27  | Func Number  | BTYE  | PCI Device and Function Number
			 */
			if (gen < G9) return 0;
			pr_handle_name("%s Device Correlation Record", company);
			if (h->length < 0x1F) break;
			dmi_hp_203_assoc_hndl("Associated Device Record", WORD(data + 0x04));
			dmi_hp_203_assoc_hndl("Associated SMBus Record",  WORD(data + 0x06));
			if (WORD(data + 0x08) == 0xffff && WORD(data + 0x0A) == 0xffff &&
			    WORD(data + 0x0C) == 0xffff && WORD(data + 0x0E) == 0xffff &&
			    data[0x10] == 0xFF && data[0x11] == 0xFF)
			{
				pr_attr("PCI Device Info", "Device Not Present");
			}
			else
			{
				dmi_hp_203_pciinfo("PCI Vendor ID", WORD(data + 0x08));
				dmi_hp_203_pciinfo("PCI Device ID", WORD(data + 0x0A));
				dmi_hp_203_pciinfo("PCI Sub Vendor ID", WORD(data + 0x0C));
				dmi_hp_203_pciinfo("PCI Sub Device ID", WORD(data + 0x0E));
				dmi_hp_203_pciinfo("PCI Class Code", (char)data[0x10]);
				dmi_hp_203_pciinfo("PCI Sub Class Code", (char)data[0x11]);
			}
			dmi_hp_203_assoc_hndl("Parent Handle", WORD(data + 0x12));
			pr_attr("Flags", "0x%04X", WORD(data + 0x14));
			dmi_hp_203_devtyp("Device Type", data[0x16]);
			dmi_hp_203_devloc("Device Location", data[0x17]);
			pr_attr("Device Instance", "%d", data[0x18]);
			pr_attr("Device Sub-Instance", "%d", data[0x19]);
			dmi_hp_203_bayenc("Bay", data[0x1A]);
			dmi_hp_203_bayenc("Enclosure", data[0x1B]);
			pr_attr("Device Path", "%s", dmi_string(h, data[0x1C]));
			pr_attr("Structured Name", "%s", dmi_string(h, data[0x1D]));
			pr_attr("Device Name", "%s", dmi_string(h, data[0x1E]));
			if (h->length < 0x22) break;
			pr_attr("UEFI Location", "%s", dmi_string(h, data[0x1F]));
			if (!(opt.flags & FLAG_QUIET))
			{
				if (WORD(data + 0x14) & 1)
					pr_attr("Associated Real/Phys Handle", "0x%04X",
						WORD(data + 0x20));
				else
					pr_attr("Associated Real/Phys Handle", "N/A");
			}
			if (h->length < 0x24) break;
			pr_attr("PCI Part Number", "%s", dmi_string(h, data[0x22]));
			pr_attr("Serial Number", "%s", dmi_string(h, data[0x23]));
			if (h->length < 0x28) break;
			pr_attr("Segment Group Number", "0x%04x", WORD(data + 0x24));
			pr_attr("PCI Device", "%02x:%02x.%x",
				data[0x26], data[0x27] >> 3, data[0x27] & 7);
			break;

		case 204:
			/*
			 * Vendor Specific: HPE ProLiant System/Rack Locator
			 */
			pr_handle_name("%s ProLiant System/Rack Locator", company);
			if (h->length < 0x0B) break;
			pr_attr("Rack Name", "%s", dmi_string(h, data[0x04]));
			pr_attr("Enclosure Name", "%s", dmi_string(h, data[0x05]));
			pr_attr("Enclosure Model", "%s", dmi_string(h, data[0x06]));
			pr_attr("Enclosure Serial", "%s", dmi_string(h, data[0x0A]));
			pr_attr("Enclosure Bays", "%d", data[0x08]);
			pr_attr("Server Bay", "%s", dmi_string(h, data[0x07]));
			pr_attr("Bays Filled", "%d", data[0x09]);
			break;

		case 209:
		case 221:
			/*
			 * Vendor Specific: HPE ProLiant NIC MAC Information
			 *
			 * This prints the BIOS NIC number,
			 * PCI bus/device/function, and MAC address
			 *
			 * Type 209:
			 * Offset |  Name  | Width | Description
			 * -------------------------------------
			 *  0x00  |  Type  | BYTE  | 0xD1, MAC Info
			 *  0x01  | Length | BYTE  | Length of structure
			 *  0x02  | Handle | WORD  | Unique handle
			 *  0x04  | Dev No | BYTE  | PCI Device/Function No
			 *  0x05  | Bus No | BYTE  | PCI Bus
			 *  0x06  |   MAC  | 6B    | MAC addr
			 *  0x0C  | NIC #2 | 8B    | Repeat 0x04-0x0B
			 *
			 * Type 221: is deprecated in the latest docs
			 */
			pr_handle_name("%s %s", company, h->type == 221 ?
				       "BIOS iSCSI NIC PCI and MAC Information" :
				       "BIOS PXE NIC PCI and MAC Information");
			nic = 1;
			ptr = 4;
			while (h->length >= ptr + 8)
			{
				dmi_print_hp_net_iface_rec(nic,
							   data[ptr + 0x01],
							   data[ptr],
							   &data[ptr + 0x02]);
				nic++;
				ptr += 8;
			}
			break;

		case 212:
			/*
			 * Vendor Specific: HPE 64-bit CRU Information
			 *
			 * Source: hpwdt kernel driver
			 */
			pr_handle_name("%s 64-bit CRU Information", company);
			if (h->length < 0x18) break;
			if (is_printable(data + 0x04, 4))
				pr_attr("Signature", "0x%08x (%c%c%c%c)",
					DWORD(data + 0x04),
					data[0x04], data[0x05],
					data[0x06], data[0x07]);
			else
				pr_attr("Signature", "0x%08x", DWORD(data + 0x04));
			if (DWORD(data + 0x04) == 0x55524324)
			{
				u64 paddr = QWORD(data + 0x08);
				paddr.l += DWORD(data + 0x14);
				if (paddr.l < DWORD(data + 0x14))
					paddr.h++;
				pr_attr("Physical Address", "0x%08x%08x",
					paddr.h, paddr.l);
				pr_attr("Length", "0x%08x", DWORD(data + 0x10));
			}
			break;

		case 219:
			/*
			 * Vendor Specific: HPE ProLiant Information
			 *
			 * Source: hpwdt kernel driver
			 */
			pr_handle_name("%s ProLiant Information", company);
			if (h->length < 0x08) break;
			pr_attr("Power Features", "0x%08x", DWORD(data + 0x04));
			if (h->length < 0x0C) break;
			pr_attr("Omega Features", "0x%08x", DWORD(data + 0x08));
			if (h->length < 0x14) break;
			feat = DWORD(data + 0x10);
			pr_attr("Misc. Features", "0x%08x", feat);
			pr_subattr("iCRU", "%s", feat & 0x0001 ? "Yes" : "No");
			pr_subattr("UEFI", "%s", feat & 0x1400 ? "Yes" : "No");
			break;

		case 224:
			/*
			 * Vendor Specific: Trusted Module (TPM or TCM) Status
			 *
			 * Offset |  Name  | Width | Description
			 * -------------------------------------
			 *  0x00  | Type   | BYTE  | 0xE0, Trusted Module (TPM or TCM) Status
			 *  0x01  | Length | BYTE  | Length of structure
			 *  0x02  | Handle | WORD  | Unique handle
			 *  0x04  | Status | BYTE  | Status Flag Byte
			 *  0x05  | Ex Stat| BYTE  | TPM Extended Status
			 *  0x06  | Type   | BYTE  | Trusted Module Type
			 *  0x07  | Attrib | BYTE  | Trusted Module Attributes
			 *  0x08  | Handle | WORD  | Handle to map to Type 216
			 *  0x0A  | Chip ID| WORD  | Chip Identifier Values
			 */
			pr_handle_name("%s Trusted Module (TPM or TCM) Status", company);
			if (h->length < 0x05) break;
			if (!dmi_hp_224_status(data[0x04]))
				break;
			if (h->length < 0x0a) break;
			dmi_hp_224_ex_status(data[0x04], data[0x05]);
			dmi_hp_224_module_type(data[0x06]);
			dmi_hp_224_module_attr(data[0x07]);
			if (!(opt.flags & FLAG_QUIET))
				pr_attr("Associated Handle", "0x%04X", WORD(data + 0x8));
			if (h->length < 0x0c) break;
			dmi_hp_224_chipid(WORD(data + 0x0a));
			break;

		case 230:
			/*
			 * Vendor Specific: Power Supply Information OEM SMBIOS Record
			 *
			 * This record is used to communicate additional Power Supply Information
			 * beyond the Industry Standard System Power Supply (Type 39) Record.
			 *
			 * Offset| Name        | Width | Description
			 * -----------------------------------------
			 *  0x00 | Type        | BYTE  | 0xE6, Power Supply Information Indicator
			 *  0x01 | Length      | BYTE  | Length of structure
			 *  0x02 | Handle      | WORD  | Unique handle
			 *  0x04 | Assoc Handle| WORD  | Associated Handle (Type 39)
			 *  0x06 | Manufacturer| STRING| Actual third party manufacturer
			 *  0x07 | Revision    | STRING| Power Supply Revision Level
			 *  0x08 | FRU Access  | BYTE  | Power Supply FRU Access Method
			 *  0x09 | I2C Bus Num | BYTE  | I2C Bus #. Value based upon context
			 *  0x0A | I2C Address | BYTE  | I2C Address
			 */
			pr_handle_name("%s Power Supply Information", company);
			if (h->length < 0x0B) break;
			if (!(opt.flags & FLAG_QUIET))
				pr_attr("Associated Handle", "0x%04X", WORD(data + 0x4));
			pr_attr("Manufacturer", "%s", dmi_string(h, data[0x06]));
			pr_attr("Revision", "%s", dmi_string(h, data[0x07]));
			dmi_hp_230_method_bus_seg_addr(data[0x08], data[0x09], data[0x0A]);
			break;

		case 233:
			/*
			 * Vendor Specific: HPE ProLiant NIC MAC Information
			 *
			 * This prints the BIOS NIC number,
			 * PCI bus/device/function, and MAC address
			 *
			 * Offset |  Name  | Width | Description
			 * -------------------------------------
			 *  0x00  |  Type  | BYTE  | 0xE9, NIC structure
			 *  0x01  | Length | BYTE  | Length of structure
			 *  0x02  | Handle | WORD  | Unique handle
			 *  0x04  | Grp No | WORD  | 0 for single segment
			 *  0x06  | Bus No | BYTE  | PCI Bus
			 *  0x07  | Dev No | BYTE  | PCI Device/Function No
			 *  0x08  |   MAC  | 32B   | MAC addr padded w/ 0s
			 *  0x28  | Port No| BYTE  | Each NIC maps to a Port
			 */
			pr_handle_name("%s BIOS PXE NIC PCI and MAC Information",
				       company);
			if (h->length < 0x0E) break;
			/* If the record isn't long enough, we don't have an ID
			 * use 0xFF to use the internal counter.
			 * */
			nic = h->length > 0x28 ? data[0x28] : 0xFF;
			dmi_print_hp_net_iface_rec(nic, data[0x06], data[0x07],
						   &data[0x08]);
			break;

		case 236:
			/*
			 * Vendor Specific: HPE ProLiant HDD Backplane
			 *
			 * Offset |  Name      | Width | Description
			 * ---------------------------------------
			 *  0x00  | Type       | BYTE  | 0xEC, HDD Backplane
			 *  0x01  | Length     | BYTE  | Length of structure
			 *  0x02  | Handle     | WORD  | Unique handle
			 *  0x04  | I2C Address| BYTE  | Backplane FRU I2C Address
			 *  0x05  | Box Number | WORD  | Backplane Box Number
			 *  0x07  | NVRAM ID   | WORD  | Backplane NVRAM ID
			 *  0x09  | WWID       | QWORD | SAS Expander WWID
			 *  0x11  | Total Bays | BYTE  | Total SAS Bays
			 *  0x12  | A0 Bays    | BYTE  | (deprecated) Number of SAS drive bays behind port 0xA0
			 *  0x13  | A2 Bays    | BYTE  | (deprecated) Number of SAS drive bays behind port 0xA2
			 *  0x14  | Name       | STRING| (deprecated) Backplane Name
			 */
			pr_handle_name("%s HDD Backplane FRU Information", company);
			if (h->length < 0x08) break;
			pr_attr("FRU I2C Address", "0x%X raw(0x%X)", data[0x4] >> 1, data[0x4]);
			pr_attr("Box Number", "%d", WORD(data + 0x5));
			pr_attr("NVRAM ID", "0x%X", WORD(data + 0x7));
			if (h->length < 0x11) break;
			pr_attr("SAS Expander WWID", "0x%X", QWORD(data + 0x9));
			if (h->length < 0x12) break;
			pr_attr("Total SAS Bays", "%d", data[0x11]);
			if (h->length < 0x15) break;
			if (gen < G10P) {
				pr_attr("A0 Bay Count", "%d", data[0x12]);
				pr_attr("A2 Bay Count", "%d", data[0x13]);
				pr_attr("Backplane Name", "%s", dmi_string(h, data[0x14]));
			}
			break;

		case 237:
			/*
			 * Vendor Specific: HPE DIMM Vendor Part Number Information
			 *
			 * Offset |  Name      | Width | Description
			 * ---------------------------------------
			 *  0x00  | Type       | BYTE  | 0xED, DIMM Vendor Part Number information record
			 *  0x01  | Length     | BYTE  | Length of structure
			 *  0x02  | Handle     | WORD  | Unique handle
			 *  0x04  | Hand Assoc | WORD  | Handle to map to Type 17
			 *  0x06  | Manufacture|STRING | DIMM Manufacturer
			 *  0x07  | Part Number|STRING | DIMM Manufacturer's Part Number
			 *  0x08  | Serial Num |STRING | DIMM Vendor Serial Number
			 *  0x09  | Spare Part |STRING | DIMM Spare Part Number
			 */
			if (gen < G9) return 0;
			pr_handle_name("%s DIMM Vendor Information", company);
			if (h->length < 0x08) break;
			if (!(opt.flags & FLAG_QUIET))
				pr_attr("Associated Handle", "0x%04X", WORD(data + 0x4));
			pr_attr("DIMM Manufacturer", "%s", dmi_string(h, data[0x06]));
			pr_attr("DIMM Manufacturer Part Number", "%s", dmi_string(h, data[0x07]));
			if (h->length < 0x09) break;
			pr_attr("DIMM Vendor Serial Number", "%s", dmi_string(h, data[0x08]));
			if (h->length < 0x0A) break;
			pr_attr("DIMM Spare Part Number", "%s", dmi_string(h, data[0x09]));
			break;

		case 238:
			/*
			 * Vendor Specific: HPE USB Port Connector Correlation Record
			 *
			 * Offset |  Name      | Width | Description
			 * ---------------------------------------
			 *  0x00  | Type       | BYTE  | 0xEE, HP Device Correlation Record
			 *  0x01  | Length     | BYTE  | Length of structure
			 *  0x02  | Handle     | WORD  | Unique handle
			 *  0x04  | Hand Assoc | WORD  | Handle to map to Type 8
			 *  0x06  | Parent Bus | BYTE  | PCI Bus number of USB controller of this port
			 *  0x07  | Par Dev/Fun| BYTE  | PCI Dev/Fun of USB Controller of this port
			 *  0x08  | Location   | BYTE  | Enumerated value of location of USB port
			 *  0x09  | Flags      | WORD  | USB Shared Management Port
			 *  0x0B  | Port Inst  | BYTE  | Instance number for this type of USB port
			 *  0x0C  | Parent Hub | BYTE  | Instance number of internal Hub
			 *  0x0D  | Port Speed | BYTE  | Enumerated value of speed configured by BIOS
			 *  0x0E  | Device Path| STRING| UEFI Device Path of USB endpoint
			 */
			if (gen < G9) return 0;
			pr_handle_name("%s Proliant USB Port Connector Correlation Record", company);
			if (h->length < 0x0F) break;
			if (!(opt.flags & FLAG_QUIET))
				pr_attr("Associated Handle", "0x%04X", WORD(data + 0x4));
			pr_attr("PCI Device", "%02x:%02x.%x", data[0x6],
				data[0x7] >> 3, data[0x7] & 0x7);
			dmi_hp_238_loc("Location", data[0x8]);
			dmi_hp_238_flags("Management Port", WORD(data + 0x9));
			pr_attr("Port Instance", "%d", data[0xB]);
			if (data[0xC] != 0xFE)
				pr_attr("Parent Hub Port Instance", "%d", data[0xC]);
			else
				pr_attr("Parent Hub Port Instance", "N/A");
			dmi_hp_238_speed("Port Speed Capability", data[0xD]);
			pr_attr("Device Path", "%s", dmi_string(h, data[0xE]));
			break;

		case 240:
			/*
			 * Vendor Specific: HPE Proliant Inventory Record
			 *
			 * Reports firmware version information for devices that report their
			 * firmware using their UEFI drivers. Additionally provides association
			 * with other SMBIOS records, such as Type 203 (which in turn is
			 * associated with Types 9, 41, and 228).
			 *
			 * Offset |  Name      | Width | Description
			 * ---------------------------------------
			 *  0x00  | Type       | BYTE  | 0xF0, HP Firmware Inventory Record
			 *  0x01  | Length     | BYTE  | Length of structure
			 *  0x02  | Handle     | WORD  | Unique handle
			 *  0x04  | Hndl Assoc | WORD  | Handle to map to Type 203
			 *  0x06  | Pkg Vers   | DWORD | FW Vers Release of All FW in Device
			 *  0x0A  | Ver String | STRING| FW Version String
			 *  0x0B  | Image Size | QWORD | FW image size (bytes)
			 *  0x13  | Attributes | QWORD | Bitfield: Is attribute defined?
			 *  0x1B  | Attr Set   | QWORD | BitField: If defined, is attribute set?
			 *  0x23  | Version    | DWORD | Lowest supported version.
			 */
			pr_handle_name("%s Proliant Inventory Record", company);
			if (h->length < 0x27) break;
			if (!(opt.flags & FLAG_QUIET))
				pr_attr("Associated Handle", "0x%04X", WORD(data + 0x4));
			pr_attr("Package Version", "0x%08X", DWORD(data + 0x6));
			pr_attr("Version String", "%s", dmi_string(h, data[0x0A]));

			if (DWORD(data + 0x0B))
				dmi_print_memory_size("Image Size", QWORD(data + 0xB), 0);
			else
				pr_attr("Image Size", "Not Available");

			dmi_hp_240_attr(QWORD(data + 0x13), QWORD(data + 0x1B));

			if (DWORD(data + 0x23))
				pr_attr("Lowest Supported Version", "0x%08X", DWORD(data + 0x23));
			else
				pr_attr("Lowest Supported Version", "Not Available");
			break;

		default:
			return 0;
	}
	return 1;
}

static int dmi_decode_ibm_lenovo(const struct dmi_header *h)
{
	u8 *data = h->data;

	switch (h->type)
	{
		case 131:
			/*
			 * Vendor Specific: ThinkVantage Technologies feature bits
			 *
			 * Source: Compal hel81 Service Manual Software Specification,
			 *         documented under "System Management BIOS(SM BIOS)
			 *         version 2.4 or greater"
			 *
			 * Offset |  Name         | Width   | Description
			 * ----------------------------------------------
			 *  0x00  | Type          | BYTE    | 0x83
			 *  0x01  | Length        | BYTE    | 0x16
			 *  0x02  | Handle        | WORD    | Varies
			 *  0x04  | Version       | BYTE    | 0x01
			 *  0x05  | TVT Structure | BYTEx16 | Each of the 128 bits represents a TVT feature:
			 *        |               |         |  - bit 127 means diagnostics (PC Doctor) is available
			 *        |               |         |    (http://www.pc-doctor.com/company/pr-articles/45-lenovo-introduces-thinkvantage-toolbox)
			 *        |               |         |  - the rest (126-0) are reserved/unknown
			 *
			 * It must also be followed by a string containing
			 * "TVT-Enablement". There exist other type 131 records
			 * with different length and a different string, for
			 * other purposes.
			 */

			if (h->length != 0x16
			 || strcmp(dmi_string(h, 1), "TVT-Enablement") != 0)
				return 0;

			pr_handle_name("ThinkVantage Technologies");
			pr_attr("Version", "%u", data[0x04]);
			pr_attr("Diagnostics", "%s",
				data[0x14] & 0x80 ? "Available" : "No");
			break;

		case 135:
			/*
			 * Vendor Specific: Device Presence Detection bits
			 *
			 * Source: Compal hel81 Service Manual Software Specification,
			 *         documented as "SMBIOS Type 135: Bulk for Lenovo
			 *         Mobile PC Unique OEM Data" under appendix D.
			 *
			 * Offset |  Name                | Width | Description
			 * ---------------------------------------------------
			 *  0x00  | Type                 | BYTE  | 0x87
			 *  0x01  | Length               | BYTE  | 0x0A
			 *  0x02  | Handle               | WORD  | Varies
			 *  0x04  | Signature            | WORD  | 0x5054 (ASCII for "TP")
			 *  0x06  | OEM struct offset    | BYTE  | 0x07
			 *  0x07  | OEM struct number    | BYTE  | 0x03, for this structure
			 *  0x08  | OEM struct revision  | BYTE  | 0x01, for this format
			 *  0x09  | Device presence bits | BYTE  | Each of the 8 bits indicates device presence:
			 *        |                      |       |  - bit 0 indicates the presence of a fingerprint reader
			 *        |                      |       |  - the rest (7-1) are reserved/unknown
			 *
			 * Other OEM struct number+rev combinations have been
			 * seen in the wild but we don't know how to decode
			 * them.
			 */

			if (h->length < 0x0A || data[0x04] != 'T' || data[0x05] != 'P')
				return 0;

			/* Bail out if not the expected format */
			if (data[0x06] != 0x07 || data[0x07] != 0x03 || data[0x08] != 0x01)
				return 0;

			pr_handle_name("ThinkPad Device Presence Detection");
			pr_attr("Fingerprint Reader", "%s",
				data[0x09] & 0x01 ? "Present" : "No");
			break;

		case 140:
			/*
			 * Vendor Specific: ThinkPad Embedded Controller Program
			 *
			 * Source: some guesswork, and publicly available information;
			 *         Lenovo's BIOS update READMEs often contain the ECP IDs
			 *         which match the first string in this type.
			 *
			 * Offset |  Name                | Width  | Description
			 * ----------------------------------------------------
			 *  0x00  | Type                 | BYTE   | 0x8C
			 *  0x01  | Length               | BYTE   |
			 *  0x02  | Handle               | WORD   | Varies
			 *  0x04  | Signature            | BYTEx6 | ASCII for "LENOVO"
			 *  0x0A  | OEM struct offset    | BYTE   | 0x0B
			 *  0x0B  | OEM struct number    | BYTE   | 0x07, for this structure
			 *  0x0C  | OEM struct revision  | BYTE   | 0x01, for this format
			 *  0x0D  | ECP version ID       | STRING |
			 *  0x0E  | ECP release date     | STRING |
			 */

			if (h->length < 0x0F || memcmp(data + 4, "LENOVO", 6) != 0)
				return 0;

			/* Bail out if not the expected format */
			if (data[0x0A] != 0x0B || data[0x0B] != 0x07 || data[0x0C] != 0x01)
				return 0;

			pr_handle_name("ThinkPad Embedded Controller Program");
			pr_attr("Version ID", "%s", dmi_string(h, 1));
			pr_attr("Release Date", "%s", dmi_string(h, 2));
			break;

		default:
			return 0;
	}
	return 1;
}

/*
 * Dispatch vendor-specific entries decoding
 * Return 1 if decoding was successful, 0 otherwise
 */
int dmi_decode_oem(const struct dmi_header *h)
{
	switch (dmi_vendor)
	{
		case VENDOR_HP:
		case VENDOR_HPE:
			return dmi_decode_hp(h);
		case VENDOR_ACER:
			return dmi_decode_acer(h);
		case VENDOR_IBM:
		case VENDOR_LENOVO:
			return dmi_decode_ibm_lenovo(h);
		default:
			return 0;
	}
}
