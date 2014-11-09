#ifndef _LIBDMI_H_
#define _LIBDMI_H_

#define L 10
#define C 30

#define BASEBOARD_MANUFACTURER	0
#define BASEBOARD_PRODUCT_NAME	1
#define BASEBOARD_VERSION	2

#define BIOS_VENDOR		3
#define BIOS_VERSION		4
#define BIOS_RELEASE_DATE	5
#define BIOS_ROM_SIZE		6

#define PROCESSOR_SOCKET	7
#define PROCESSOR_CLOCK		8

extern int verbose;

int libdmi(char dmiexport[L][C]);

#endif
