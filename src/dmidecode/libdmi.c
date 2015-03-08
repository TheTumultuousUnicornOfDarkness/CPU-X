#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libdmi.h"
#include "types.h"
#include "dmiopt.h"


/* Options are global */
struct opt opt;
char **dmidata;


static u8 *dmiparse(u8 *p, int l)
{
	/* Allocate memory on first call only */
	if (p == NULL)
	{
		p = (u8 *)calloc(256, sizeof(u8));
		if (p == NULL)
		{
			perror("calloc");
			return NULL;
		}
	}

	p[l] = 1;
	return p;
}

int dmialloc(char ***str, int l)
{
	int i;

	*str = (char **) malloc(l * sizeof(char*));
	if(*str == NULL)
	{
		perror("malloc");
		return 1;
	}

	for(i = 0; i < l; i++)
	{
		(*str)[i] = (char *) calloc(MAXSTR, sizeof(char));
		if((*str)[i] == NULL)
		{
			perror("malloc");
			return 2;
		}
		memset((*str)[i], '\0', MAXSTR);
	}

	return 0;
}

void dmifree(char ***str, int l) {
	int i;

	for(i = 0; i < l; i++)
		free((*str)[i]);

	free(*str);
}

int libdmi(char data[][MAXSTR], char c)
{
	int i, err = 0;

	/* Dmidecode options */
	opt.flags = 0;
	opt.type = NULL;
	if(verbose == 0 || verbose == 2)
		opt.flags |= FLAG_QUIET;

	switch(c)
	{
		case 'c':
			if(!dmialloc(&dmidata, LASTPROC))
			{
				opt.type = dmiparse(opt.type, 4);
				err = maindmi();
				for(i = PROC_PACKAGE; i < LASTPROC; i++)
					strncpy(data[i], dmidata[i], MAXSTR);
				dmifree(&dmidata, LASTPROC);
			}
			break;
		case 'm':
			if(!dmialloc(&dmidata, LASTMB))
			{
				opt.type = dmiparse(opt.type, 0);
				opt.type = dmiparse(opt.type, 2);
				err = maindmi();
				for(i = MANUFACTURER; i < LASTMB; i++)
					strncpy(data[i], dmidata[i], MAXSTR);
				dmifree(&dmidata, LASTMB);
			}
			break;
		case 'r':
			if(!dmialloc(&dmidata, LASTRAM))
			{
				opt.type = dmiparse(opt.type, 17);
				err = maindmi();
				for(i = BANK0_0; i < LASTRAM; i++)
					strncpy(data[i], dmidata[i], MAXSTR);
				dmifree(&dmidata, LASTRAM);
			}
			break;
		case 'D':
			err = maindmi();
			break;
		default:
			break;
	}

	return err;
}
