/*
 * DMI Decode
 *
 *   Copyright (C) 2000-2002 Alan Cox <alan@redhat.com>
 *   Copyright (C) 2002-2010 Jean Delvare <khali@linux-fr.org>
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
 *
 *   For the avoidance of doubt the "preferred form" of this code is one which
 *   is in an open unpatent encumbered format. Where cryptographic key signing
 *   forms part of the process of creating an executable the information
 *   including keys needed to generate an equivalently functional executable
 *   are deemed to be part of the source code.
 *
 * Unless specified otherwise, all references are aimed at the "System
 * Management BIOS Reference Specification, Version 2.8.0" document,
 * available from http://www.dmtf.org/standards/smbios.
 *
 * Note to contributors:
 * Please reference every value you add or modify, especially if the
 * information does not come from the above mentioned specification.
 *
 * Additional references:
 *  - Intel AP-485 revision 36
 *    "Intel Processor Identification and the CPUID Instruction"
 *    http://www.intel.com/support/processors/sb/cs-009861.htm
 *  - DMTF Common Information Model
 *    CIM Schema version 2.19.1
 *    http://www.dmtf.org/standards/cim/
 *  - IPMI 2.0 revision 1.0
 *    "Intelligent Platform Management Interface Specification"
 *    http://developer.intel.com/design/servers/ipmi/spec.htm
 *  - AMD publication #25481 revision 2.28
 *    "CPUID Specification"
 *    http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
 *  - BIOS Integrity Services Application Programming Interface version 1.0
 *    http://www.intel.com/design/archives/wfm/downloads/bisspec.htm
 *  - DMTF DSP0239 version 1.1.0
 *    "Management Component Transport Protocol (MCTP) IDs and Codes"
 *    http://www.dmtf.org/standards/pmci
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include "libdmi.h"
#include "../cpu-x.h"

#include "config.h"
#include "types.h"
#include "util.h"
#include "dmidecode.h"
#include "dmiopt.h"

#define out_of_spec "<OUT OF SPEC>"
static const char *bad_index = "<BAD INDEX>";

#define SUPPORTED_SMBIOS_VER 0x0207

/*
 * Type-independant Stuff
 */

const char *dmi_string(const struct dmi_header *dm, u8 s)
{
	char *bp = (char *)dm->data;
	size_t i, len;

	if (s == 0)
		return "Not Specified";

	bp += dm->length;
	while (s > 1 && *bp)
	{
		bp += strlen(bp);
		bp++;
		s--;
	}

	if (!*bp)
		return bad_index;

	if (!(opt.flags & FLAG_DUMP))
	{
		/* ASCII filtering */
		len = strlen(bp);
		for (i = 0; i < len; i++)
			if (bp[i] < 32 || bp[i] == 127)
				bp[i] = '.';
	}

	return bp;
}

static void dmi_dump(const struct dmi_header *h, const char *prefix)
{
	int row, i;
	const char *s;

	printf("%sHeader and Data:\n", prefix);
	for (row = 0; row < ((h->length - 1) >> 4) + 1; row++)
	{
		printf("%s\t", prefix);
		for (i = 0; i < 16 && i < h->length - (row << 4); i++)
			printf("%s%02X", i ? " " : "",
			       (h->data)[(row << 4) + i]);
		printf("\n");
	}

	if ((h->data)[h->length] || (h->data)[h->length + 1])
	{
		printf("%sStrings:\n", prefix);
		i = 1;
		while ((s = dmi_string(h, i++)) != bad_index)
		{
			if (opt.flags & FLAG_DUMP)
			{
				int j, l = strlen(s) + 1;
				for (row = 0; row < ((l - 1) >> 4) + 1; row++)
				{
					printf("%s\t", prefix);
					for (j = 0; j < 16 && j < l - (row << 4); j++)
						printf("%s%02X", j ? " " : "",
						       (unsigned char)s[(row << 4) + j]);
					printf("\n");
				}
				/* String isn't filtered yet so do it now */
				printf("%s\t\"", prefix);
				while (*s)
				{
					if (*s < 32 || *s == 127)
						fputc('.', stdout);
					else
						fputc(*s, stdout);
					s++;
				}
				printf("\"\n");
			}
			else
				printf("%s\t%s\n", prefix, s);
		}
	}
}


/*
 * 7.1 BIOS Information (Type 0)
 */

static char *dmi_bios_runtime_size(u32 code)
{
	static char ret[20];
	if (code & 0x000003FF)
		sprintf(ret, " %u bytes", code);
	else
		sprintf(ret, " %u kB", code >> 10);

	return ret;
}


/*
 * 7.5 Processor Information (Type 4)
 */

static void dmi_processor_frequency(const u8 *p)
{
	u16 code = WORD(p);

	if (code)
		printf("%u MHz", code);
	else
		printf("Unknown");
}


/*
 * 7.18 Memory Device (Type 17)
 */

static const char *dmi_memory_device_size(u16 code)
{
	static char ret[20];
	if (code == 0)
		sprintf(ret, "No Module Installed");
	else if (code == 0xFFFF)
		sprintf(ret, "Unknown");
	else
	{
		if (code & 0x8000)
			sprintf(ret, " %u kB", code & 0x7FFF);
		else
			sprintf(ret, " %u MB", code);
	}
	return ret;
}

static const char *dmi_memory_device_form_factor(u8 code)
{
	/* 7.18.1 */
	static const char *form_factor[] = {
		"Other", /* 0x01 */
		"Unknown",
		"SIMM",
		"SIP",
		"Chip",
		"DIP",
		"ZIP",
		"Proprietary Card",
		"DIMM",
		"TSOP",
		"Row Of Chips",
		"RIMM",
		"SODIMM",
		"SRIMM",
		"FB-DIMM" /* 0x0F */
	};

	if (code >= 0x01 && code <= 0x0F)
		return form_factor[code - 0x01];
	return out_of_spec;
}

static const char *dmi_memory_device_type(u8 code)
{
	/* 7.18.2 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"DRAM",
		"EDRAM",
		"VRAM",
		"SRAM",
		"RAM",
		"ROM",
		"Flash",
		"EEPROM",
		"FEPROM",
		"EPROM",
		"CDRAM",
		"3DRAM",
		"SDRAM",
		"SGRAM",
		"RDRAM",
		"DDR",
		"DDR2",
		"DDR2 FB-DIMM",
		"Reserved",
		"Reserved",
		"Reserved",
		"DDR3",
		"FBD2", /* 0x19 */
	};

	if (code >= 0x01 && code <= 0x19)
		return type[code - 0x01];
	return out_of_spec;
}


/*
 * Main
 */

static void dmi_decode(const struct dmi_header *h, u16 ver)
{
	const u8 *data = h->data;
	char bankcur[7];
	static int bank = BANK0_0;

	/*
	 * Note: DMI types 37, 39 and 40 are untested
	 */
	switch (h->type)
	{
		case 0: /* 7.1 BIOS Information */
			strcpy(dmimb[3], dmi_string(h, data[0x04]));
			strcpy(dmimb[4], dmi_string(h, data[0x05]));
			strcpy(dmimb[5], dmi_string(h, data[0x08]));
			sprintf(dmimb[6], "%s / %u kB", dmi_bios_runtime_size((0x10000 - WORD(data + 0x06)) << 4), (data[0x09] + 1) << 6);
			break;
		
		case 2: /* 7.3 Base Board Information */
			strcpy(dmimb[0], dmi_string(h, data[0x04]));
			strcpy(dmimb[1], dmi_string(h, data[0x05]));
			strcpy(dmimb[2], dmi_string(h, data[0x06]));
			break;
		
		case 4: /* 7.5 Processor Information */
			strcpy(dmicpu[0], dmi_string(h, data[0x04]));
			sprintf(dmicpu[1], "%u MHz", WORD(data + 0x12));
			break;
		case 17: /* 7.18 Memory Device */
			strcpy(bankcur, dmi_string(h, data[0x11]));
			if(bank <= BANK7_0)
			{
				if(bank == (bankcur[5] - '0') * 2)
				{
					if(!strcmp(dmi_string(h, data[0x17]), "[Empty]"))
						strcpy(dmiram[bank], "Empty");
					else
					{
						snprintf(dmiram[bank], 60, "%s %s", dmi_string(h, data[0x17]), dmi_string(h, data[0x1A]));
						snprintf(dmiram[bank + 1], 60, "%s (%s %s) @ %uMHz", dmi_memory_device_size(WORD(data + 0x0C)), dmi_memory_device_form_factor(data[0x0E]), dmi_memory_device_type(data[0x12]), (WORD(data + 0x15)));
					}
				}
				bank +=2;
			}
			break;
		default:
			break;
	}
}

static void to_dmi_header(struct dmi_header *h, u8 *data)
{
	h->type = data[0];
	h->length = data[1];
	h->handle = WORD(data + 2);
	h->data = data;
}

static void dmi_table_string(const struct dmi_header *h, const u8 *data, u16 ver)
{
	int key;
	u8 offset = opt.string->offset;

	if (offset >= h->length)
		return;

	key = (opt.string->type << 8) | offset;
	switch (key)
	{
		case 0x416:
			dmi_processor_frequency(data + offset);
			printf("\n");
			break;
		default:
			printf("%s\n", dmi_string(h, data[offset]));
	}
}

static void dmi_table_dump(u32 base, u16 len, const char *devmem)
{
	u8 *buf;

	if ((buf = mem_chunk(base, len, devmem)) == NULL)
	{
		fprintf(stderr, "Failed to read table, sorry.\n");
		return;
	}

	if (!(opt.flags & FLAG_QUIET))
		printf("# Writing %d bytes to %s.\n", len, opt.dumpfile);
	write_dump(32, len, buf, opt.dumpfile, 0);
	free(buf);
}

static void dmi_table(u32 base, u16 len, u16 num, u16 ver, const char *devmem)
{
	u8 *buf;
	u8 *data;
	int i = 0;

	if (ver > SUPPORTED_SMBIOS_VER)
	{
		printf("# SMBIOS implementations newer than version %u.%u are not\n"
		       "# fully supported by this version of dmidecode.\n",
		       SUPPORTED_SMBIOS_VER >> 8, SUPPORTED_SMBIOS_VER & 0xFF);
	}

	if (opt.flags & FLAG_DUMP_BIN)
	{
		dmi_table_dump(base, len, devmem);
		return;
	}

	if (!(opt.flags & FLAG_QUIET))
	{
		if (opt.type == NULL)
		{
			printf("%u structures occupying %u bytes.\n",
				num, len);
			if (!(opt.flags & FLAG_FROM_DUMP))
				printf("Table at 0x%08X.\n", base);
		}
		printf("\n");
	}

	if ((buf = mem_chunk(base, len, devmem)) == NULL)
	{
		fprintf(stderr, "Table is unreachable, sorry."
#ifndef USE_MMAP
			" Try compiling dmidecode with -DUSE_MMAP."
#endif
			"\n");
		return;
	}

	data = buf;
#ifdef CPUX
	while (i <= num && data+4 <= buf + len) /* 4 is last handle ; 4 is the length of an SMBIOS structure header */
#else
	while (i < num && data+4 <= buf + len) /* 4 is the length of an SMBIOS structure header */
#endif
	{
		u8 *next;
		struct dmi_header h;
		int display;

		to_dmi_header(&h, data);
		display = ((opt.type == NULL || opt.type[h.type])
			&& !((opt.flags & FLAG_QUIET) && (h.type > 39 && h.type <= 127))
			&& !opt.string);

		/*
		 * If a short entry is found (less than 4 bytes), not only it
		 * is invalid, but we cannot reliably locate the next entry.
		 * Better stop at this point, and let the user know his/her
		 * table is broken.
		 */
		if (h.length < 4)
		{
			printf("Invalid entry length (%u). DMI table is "
			       "broken! Stop.\n\n", (unsigned int)h.length);
			opt.flags |= FLAG_QUIET;
			break;
		}

		/* In quiet mode, stop decoding at end of table marker */
		if ((opt.flags & FLAG_QUIET) && h.type == 127)
			break;

		if (display
		 && (!(opt.flags & FLAG_QUIET) || (opt.flags & FLAG_DUMP)))
			printf("Handle 0x%04X, DMI type %d, %d bytes\n",
				h.handle, h.type, h.length);

		/* assign vendor for vendor-specific decodes later */
		/*if (h.type == 0 && h.length >= 5)
			dmi_set_vendor(dmi_string(&h, data[0x04]));*/

		/* look for the next handle */
		next = data + h.length;
		while (next - buf + 1 < len && (next[0] != 0 || next[1] != 0))
			next++;
		next += 2;
		if (display)
		{
			if (next - buf <= len)
			{
				if (opt.flags & FLAG_DUMP)
				{
					dmi_dump(&h, "\t");
					printf("\n");
				}
				else
					dmi_decode(&h, ver);
			}
			else if (!(opt.flags & FLAG_QUIET))
				printf("\t<TRUNCATED>\n\n");
		}
		else if (opt.string != NULL
		      && opt.string->type == h.type)
			dmi_table_string(&h, data, ver);

		data = next;
		i++;
	}

#ifndef CPUX
	if (!(opt.flags & FLAG_QUIET))
	{
		if (i != num)
			printf("Wrong DMI structures count: %d announced, "
				"only %d decoded.\n", num, i);
		if (data - buf != len)
			printf("Wrong DMI structures length: %d bytes "
				"announced, structures occupy %d bytes.\n",
				len, (unsigned int)(data - buf));
	}
#endif

	free(buf);
}


/*
 * Build a crafted entry point with table address hard-coded to 32,
 * as this is where we will put it in the output file. We adjust the
 * DMI checksum appropriately. The SMBIOS checksum needs no adjustment.
 */
static void overwrite_dmi_address(u8 *buf)
{
	buf[0x05] += buf[0x08] + buf[0x09] + buf[0x0A] + buf[0x0B] - 32;
	buf[0x08] = 32;
	buf[0x09] = 0;
	buf[0x0A] = 0;
	buf[0x0B] = 0;
}

static int smbios_decode(u8 *buf, const char *devmem)
{
	u16 ver;

	if (!checksum(buf, buf[0x05])
	 || memcmp(buf + 0x10, "_DMI_", 5) != 0
	 || !checksum(buf + 0x10, 0x0F))
		return 0;

	ver = (buf[0x06] << 8) + buf[0x07];
	/* Some BIOS report weird SMBIOS version, fix that up */
	switch (ver)
	{
		case 0x021F:
		case 0x0221:
			if (!(opt.flags & FLAG_QUIET))
				printf("SMBIOS version fixup (2.%d -> 2.%d).\n",
				       ver & 0xFF, 3);
			ver = 0x0203;
			break;
		case 0x0233:
			if (!(opt.flags & FLAG_QUIET))
				printf("SMBIOS version fixup (2.%d -> 2.%d).\n",
				       51, 6);
			ver = 0x0206;
			break;
	}
	if (!(opt.flags & FLAG_QUIET))
		printf("SMBIOS %u.%u present.\n",
			ver >> 8, ver & 0xFF);

	dmi_table(DWORD(buf + 0x18), WORD(buf + 0x16), WORD(buf + 0x1C),
		ver, devmem);

	if (opt.flags & FLAG_DUMP_BIN)
	{
		u8 crafted[32];

		memcpy(crafted, buf, 32);
		overwrite_dmi_address(crafted + 0x10);

		if (!(opt.flags & FLAG_QUIET))
			printf("# Writing %d bytes to %s.\n", crafted[0x05],
				opt.dumpfile);
		write_dump(0, crafted[0x05], crafted, opt.dumpfile, 1);
	}

	return 1;
}

static int legacy_decode(u8 *buf, const char *devmem)
{
	if (!checksum(buf, 0x0F))
		return 0;

	if (!(opt.flags & FLAG_QUIET))
		printf("Legacy DMI %u.%u present.\n",
			buf[0x0E] >> 4, buf[0x0E] & 0x0F);

	dmi_table(DWORD(buf + 0x08), WORD(buf + 0x06), WORD(buf + 0x0C),
		((buf[0x0E] & 0xF0) << 4) + (buf[0x0E] & 0x0F), devmem);

	if (opt.flags & FLAG_DUMP_BIN)
	{
		u8 crafted[16];

		memcpy(crafted, buf, 16);
		overwrite_dmi_address(crafted);

		printf("# Writing %d bytes to %s.\n", 0x0F, opt.dumpfile);
		write_dump(0, 0x0F, crafted, opt.dumpfile, 1);
	}

	return 1;
}

/*
 * Probe for EFI interface
 */
#define EFI_NOT_FOUND   (-1)
#define EFI_NO_SMBIOS   (-2)
static int address_from_efi(size_t *address)
{
	FILE *efi_systab;
	const char *filename;
	char linebuf[64];
	int ret;

	*address = 0; /* Prevent compiler warning */

	/*
	 * Linux up to 2.6.6: /proc/efi/systab
	 * Linux 2.6.7 and up: /sys/firmware/efi/systab
	 */
	if ((efi_systab = fopen(filename = "/sys/firmware/efi/systab", "r")) == NULL
	 && (efi_systab = fopen(filename = "/proc/efi/systab", "r")) == NULL)
	{
		/* No EFI interface, fallback to memory scan */
		return EFI_NOT_FOUND;
	}
	ret = EFI_NO_SMBIOS;
	while ((fgets(linebuf, sizeof(linebuf) - 1, efi_systab)) != NULL)
	{
		char *addrp = strchr(linebuf, '=');
		*(addrp++) = '\0';
		if (strcmp(linebuf, "SMBIOS") == 0)
		{
			*address = strtoul(addrp, NULL, 0);
			if (!(opt.flags & FLAG_QUIET))
				printf("# SMBIOS entry point at 0x%08lx\n",
				       (unsigned long)*address);
			ret = 0;
			break;
		}
	}
	if (fclose(efi_systab) != 0)
		perror(filename);

	if (ret == EFI_NO_SMBIOS)
		fprintf(stderr, "%s: SMBIOS entry point missing\n", filename);
	return ret;
}

#ifdef CPUX
int maindmi(void)
#else
int main(int argc, char * const argv[])
#endif
{
	int ret = 0;                /* Returned value */
	int found = 0;
	size_t fp;
	int efi;
	u8 *buf;

#ifdef CPUX
	if (sizeof(u8) != 1 || sizeof(u16) != 2 || sizeof(u32) != 4 || '\0' != 0)
	{
		fprintf(stderr, "libdmi: compiler incompatibility\n");
		exit(255);
	}
#else
	if (sizeof(u8) != 1 || sizeof(u16) != 2 || sizeof(u32) != 4 || '\0' != 0)
	{
		fprintf(stderr, "%s: compiler incompatibility\n", argv[0]);
		exit(255);
	}
#endif

	/* Set default option values */
	opt.devmem = DEFAULT_MEM_DEV;
	opt.flags = 0;

#ifdef CPUX
	if(!verbose)
		opt.flags |= FLAG_QUIET;
#else
	if (parse_command_line(argc, argv)<0)
	{
		ret = 2;
		goto exit_free;
	}

	if (opt.flags & FLAG_HELP)
	{
		print_help();
		goto exit_free;
	}

	if (opt.flags & FLAG_VERSION)
	{
		printf("%s\n", VERSION);
		goto exit_free;
	}

	if (!(opt.flags & FLAG_QUIET))
		printf("# dmidecode %s\n", VERSION);

	/* Read from dump if so instructed */
	if (opt.flags & FLAG_FROM_DUMP)
	{
		if (!(opt.flags & FLAG_QUIET))
			printf("Reading SMBIOS/DMI data from file %s.\n",
			       opt.dumpfile);
		if ((buf = mem_chunk(0, 0x20, opt.dumpfile)) == NULL)
		{
			ret = 1;
			goto exit_free;
		}

		if (memcmp(buf, "_SM_", 4) == 0)
		{
			if (smbios_decode(buf, opt.dumpfile))
				found++;
		}
		else if (memcmp(buf, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf, opt.dumpfile))
				found++;
		}
		goto done;
	}
#endif
	/* First try EFI (ia64, Intel-based Mac) */
	efi = address_from_efi(&fp);
	switch (efi)
	{
		case EFI_NOT_FOUND:
			goto memory_scan;
		case EFI_NO_SMBIOS:
			ret = 1;
			goto exit_free;
	}

	if ((buf = mem_chunk(fp, 0x20, opt.devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	if (smbios_decode(buf, opt.devmem))
		found++;
	goto done;

memory_scan:
	/* Fallback to memory scan (x86, x86_64) */
	if ((buf = mem_chunk(0xF0000, 0x10000, opt.devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	for (fp = 0; fp <= 0xFFF0; fp += 16)
	{
		if (memcmp(buf + fp, "_SM_", 4) == 0 && fp <= 0xFFE0)
		{
			if (smbios_decode(buf+fp, opt.devmem))
			{
				found++;
				fp += 16;
			}
		}
		else if (memcmp(buf + fp, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf + fp, opt.devmem))
				found++;
		}
	}

done:
	if (!found && !(opt.flags & FLAG_QUIET))
		printf("# No SMBIOS nor DMI entry point found, sorry.\n");

	free(buf);
exit_free:
	free(opt.type);

	return ret;
}
