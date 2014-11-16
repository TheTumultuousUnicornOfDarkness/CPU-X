#include <stdio.h>
#include <string.h>
#include "libdmi.h"


char dmicpu[2][MAXSTR];
char dmimb[LASTMB][MAXSTR];
char dmiram[LASTRAM][MAXSTR];

int libdmi(char data[LASTRAM][MAXSTR], char c)
{
	int i, err;
	err = maindmi();

	switch(c)
	{
		case 'c':
			
			for(i = 0; i < 2; i++)
				strcpy(data[i], dmicpu[i]);
			break;
		case 'm':
			for(i = MANUFACTURER; i < LASTMB; i++)
				strcpy(data[i], dmimb[i]);
			break;
		case 'r':
			for(i = BANK0_0; i < LASTRAM; i++)
				strcpy(data[i], dmiram[i]);
			break;
		default:
			break;
	}

	return err;
}
