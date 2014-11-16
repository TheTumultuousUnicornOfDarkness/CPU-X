#ifndef _LIBDMI_H_
#define _LIBDMI_H_

#include "../cpu-x.h"

#define PROC_PACKAGE 0
#define PROC_BUS 1

extern int verbose;
extern char dmicpu[2][MAXSTR];
extern char dmimb[LASTMB][MAXSTR];
extern char dmiram[LASTRAM][MAXSTR];

int maindmi(void);
int libdmi(char data[LASTRAM][MAXSTR], char c);


#endif
