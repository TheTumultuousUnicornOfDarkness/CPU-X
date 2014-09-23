#ifndef _LIBDMI_H_
#define _LIBDMI_H_

#define L 9
#define C 30

#define BIOS_VENDOR		0
#define BIOS_VERSION		1
#define BIOS_RELEASE_DATE	2
#define BIOS_ROM_SIZE		3

#define BASEBOARD_MANUFACTURER	4
#define BASEBOARD_PRODUCT_NAME	5
#define BASEBOARD_VERSION	6

#define PROCESSOR_SOCKET	7
#define PROCESSOR_CLOCK		8


int libdmi(char dmiexport[L][C]);

#endif
