/*
 * Decoding of OEM-specific entries
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2007-2008 Jean Delvare <khali@linux-fr.org>
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

enum DMI_VENDORS { VENDOR_UNKNOWN, VENDOR_HP };

static enum DMI_VENDORS dmi_vendor = VENDOR_UNKNOWN;

/*
 * Remember the system vendor for later use. We only actually store the
 * value if we know how to decode at least one specific entry type for
 * that vendor.
 */
void dmi_set_vendor(const char *s)
{
	if (strcmp(s, "HP") == 0 || strcmp(s, "Hewlett-Packard") == 0)
		dmi_vendor = VENDOR_HP;
}

/*
 * HP-specific data structures are decoded here.
 *
 * Code contributed by John Cagle.
 */

static int dmi_decode_hp(const struct dmi_header *h)
{
	u8 *data = h->data;
	int nic, ptr;

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
			 */
			printf(h->type == 221 ?
				"HP BIOS iSCSI NIC PCI and MAC Information\n" :
				"HP BIOS PXE NIC PCI and MAC Information\n");
			nic = 1;
			ptr = 4;
			while (h->length >= ptr + 8)
			{
				if (data[ptr] == 0x00 && data[ptr + 1] == 0x00)
					printf("\tNIC %d: Disabled\n", nic);
				else if (data[ptr] == 0xFF && data[ptr + 1] == 0xFF)
					printf("\tNIC %d: Not Installed\n", nic);
				else
				{
					printf("\tNIC %d: PCI device %02x:%02x.%x, "
						"MAC address %02X:%02X:%02X:%02X:%02X:%02X\n",
						nic, data[ptr + 1],
						data[ptr] >> 3, data[ptr] & 7,
						data[ptr + 2], data[ptr + 3],
						data[ptr + 4], data[ptr + 5],
						data[ptr + 6], data[ptr + 7]);
				}
				nic++;
				ptr += 8;
			}
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
		default:
			return 0;
	}
}
