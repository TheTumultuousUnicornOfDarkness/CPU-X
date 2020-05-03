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
#include "dmidecode.h"
#include "dmioem.h"
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

/*
 * Remember the system vendor for later use. We only actually store the
 * value if we know how to decode at least one specific entry type for
 * that vendor.
 */
void dmi_set_vendor(const char *s)
{
	int len;

	/*
	 * Often DMI strings have trailing spaces. Ignore these
	 * when checking for known vendor names.
	 */
	len = strlen(s);
	while (len && s[len - 1] == ' ')
		len--;

	if (strncmp(s, "Acer", len) == 0)
		dmi_vendor = VENDOR_ACER;
	else if (strncmp(s, "HP", len) == 0 || strncmp(s, "Hewlett-Packard", len) == 0)
		dmi_vendor = VENDOR_HP;
	else if (strncmp(s, "HPE", len) == 0 || strncmp(s, "Hewlett Packard Enterprise", len) == 0)
		dmi_vendor = VENDOR_HPE;
	else if (strncmp(s, "IBM", len) == 0)
		dmi_vendor = VENDOR_IBM;
	else if (strncmp(s, "LENOVO", len) == 0)
		dmi_vendor = VENDOR_LENOVO;
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

	sprintf(attr, "NIC %hu", id);
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

static int dmi_decode_hp(const struct dmi_header *h)
{
	u8 *data = h->data;
	int nic, ptr;
	u32 feat;
	const char *company = (dmi_vendor == VENDOR_HP) ? "HP" : "HPE";

	switch (h->type)
	{
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
