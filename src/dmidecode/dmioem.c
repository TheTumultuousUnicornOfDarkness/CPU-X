/*
 * Decoding of OEM-specific entries
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2007-2008 Jean Delvare <jdelvare@suse.de>
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

/*
 * Globals for vendor-specific decodes
 */

enum DMI_VENDORS
{
	VENDOR_UNKNOWN,
	VENDOR_HP,
	VENDOR_ACER,
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

	if (strncmp(s, "HP", len) == 0 || strncmp(s, "Hewlett-Packard", len) == 0)
		dmi_vendor = VENDOR_HP;
	else if (strncmp(s, "Acer", len) == 0)
		dmi_vendor = VENDOR_ACER;
}

/*
 * HP-specific data structures are decoded here.
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

	if (id == 0xFF)
		id = ++nic_ctr;

	if (dev == 0x00 && bus == 0x00)
		printf("\tNIC %d: Disabled\n", id);
	else if (dev == 0xFF && bus == 0xFF)
		printf("\tNIC %d: Not Installed\n", id);
	else
	{
		printf("\tNIC %d: PCI device %02x:%02x.%x, "
			"MAC address %02X:%02X:%02X:%02X:%02X:%02X\n",
			id, bus, dev >> 3, dev & 7,
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
}

static int dmi_decode_hp(const struct dmi_header *h)
{
	u8 *data = h->data;
	int nic, ptr;
	u32 feat;

	switch (h->type)
	{
		case 204:
			/*
			 * Vendor Specific: HP ProLiant System/Rack Locator
			 */
			printf("HP ProLiant System/Rack Locator\n");
			if (h->length < 0x0B) break;
			printf("\tRack Name: %s\n", dmi_string(h, data[0x04]));
			printf("\tEnclosure Name: %s\n", dmi_string(h, data[0x05]));
			printf("\tEnclosure Model: %s\n", dmi_string(h, data[0x06]));
			printf("\tEnclosure Serial: %s\n", dmi_string(h, data[0x0A]));
			printf("\tEnclosure Bays: %d\n", data[0x08]);
			printf("\tServer Bay: %s\n", dmi_string(h, data[0x07]));
			printf("\tBays Filled: %d\n", data[0x09]);
			break;

		case 209:
		case 221:
			/*
			 * Vendor Specific: HP ProLiant NIC MAC Information
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
			printf(h->type == 221 ?
				"HP BIOS iSCSI NIC PCI and MAC Information\n" :
				"HP BIOS PXE NIC PCI and MAC Information\n");
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
			 * Vendor Specific: HP ProLiant NIC MAC Information
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
			printf("HP BIOS PXE NIC PCI and MAC Information\n");
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
			 * Vendor Specific: HP 64-bit CRU Information
			 *
			 * Source: hpwdt kernel driver
			 */
			printf("HP 64-bit CRU Information\n");
			if (h->length < 0x18) break;
			printf("\tSignature: 0x%08x", DWORD(data + 0x04));
			if (is_printable(data + 0x04, 4))
				printf(" (%c%c%c%c)", data[0x04], data[0x05],
					data[0x06], data[0x07]);
			printf("\n");
			if (DWORD(data + 0x04) == 0x55524324)
			{
				u64 paddr = QWORD(data + 0x08);
				paddr.l += DWORD(data + 0x14);
				if (paddr.l < DWORD(data + 0x14))
					paddr.h++;
				printf("\tPhysical Address: 0x%08x%08x\n",
					paddr.h, paddr.l);
				printf("\tLength: 0x%08x\n", DWORD(data + 0x10));
			}
			break;

		case 219:
			/*
			 * Vendor Specific: HP ProLiant Information
			 *
			 * Source: hpwdt kernel driver
			 */
			printf("HP ProLiant Information\n");
			if (h->length < 0x08) break;
			printf("\tPower Features: 0x%08x\n", DWORD(data + 0x04));
			if (h->length < 0x0C) break;
			printf("\tOmega Features: 0x%08x\n", DWORD(data + 0x08));
			if (h->length < 0x14) break;
			feat = DWORD(data + 0x10);
			printf("\tMisc. Features: 0x%08x\n", feat);
			printf("\t\tiCRU: %s\n", feat & 0x0001 ? "Yes" : "No");
			printf("\t\tUEFI: %s\n", feat & 0x0408 ? "Yes" : "No");
			break;

		default:
			return 0;
	}
	return 1;
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
			printf("Acer Hotkey Function\n");
			if (h->length < 0x0F) break;
			cap = WORD(data + 0x04);
			printf("\tFunction bitmap for Communication Button: 0x%04hx\n", cap);
			printf("\t\tWiFi: %s\n", cap & 0x0001 ? "Yes" : "No");
			printf("\t\t3G: %s\n", cap & 0x0040 ? "Yes" : "No");
			printf("\t\tWiMAX: %s\n", cap & 0x0080 ? "Yes" : "No");
			printf("\t\tBluetooth: %s\n", cap & 0x0800 ? "Yes" : "No");
			printf("\tFunction bitmap for Application Button: 0x%04hx\n", WORD(data + 0x06));
			printf("\tFunction bitmap for Media Button: 0x%04hx\n", WORD(data + 0x08));
			printf("\tFunction bitmap for Display Button: 0x%04hx\n", WORD(data + 0x0A));
			printf("\tFunction bitmap for Others Button: 0x%04hx\n", WORD(data + 0x0C));
			printf("\tCommunication Function Key Number: %d\n", data[0x0E]);
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
			return dmi_decode_hp(h);
		case VENDOR_ACER:
			return dmi_decode_acer(h);
		default:
			return 0;
	}
}
