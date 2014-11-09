#ifndef _LIBDMI_H_
#define _LIBDMI_H_

#define L 10
#define C 30

#define PROCESSOR_SOCKET	7
#define PROCESSOR_CLOCK		8

extern int verbose;

int libdmi(char dmiexport[L][C]);

#endif
