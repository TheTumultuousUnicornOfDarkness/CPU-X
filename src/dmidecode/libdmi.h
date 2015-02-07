#ifndef _LIBDMI_H_
#define _LIBDMI_H_

#include "../cpu-x.h"

#define DMIVERSION "2.12"
#define PROC_PACKAGE 0
#define PROC_BUS 1
#define LASTPROC 2

extern int verbose;
extern char **dmidata;

int maindmi(void);
int libdmi(char data[][MAXSTR], char c);


#endif
