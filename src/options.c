/****************************************************************************
*    Copyright © 2014 Xorg
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
****************************************************************************/

/*
* options.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "cpu-x.h"

int refreshtime = 1;
int verbose = 0;


void help(FILE *out, char *argv[]) {
	fprintf(out, "Usage: %s [OPTION]\n\n"
		"Available OPTION:\n"
		"\t--no-gui\tStart NCurses mode instead of GTK\n"
		"\t--dump\t\tDump all data on stdout and exit\n"
		"\t--refresh\tTime between two refreshs in seconds\n"
#if HAS_LIBDMI
		"\t--verbose\tVerbose output (in Dmidecode)\n"
#endif /* HAS_LIBDMI */
		"\t--help\t\tPrint help and exit\n"
		"\t--version\tPrint version and exit\n", argv[0]);
}

void version() {
	printf("%s %s\n"
	"Copyright © 2014 Xorg\n\n"
	"This is free software: you are free to change and redistribute it.\n"
	"This program comes with ABSOLUTELY NO WARRANTY\n"
	"See the GPLv3 license: <http://www.gnu.org/licenses/gpl.txt>\n", PRGNAME, PRGVER);
}

char menu(int argc, char *argv[]) {
	int c;
	char r = 'G';
	static struct option longopts[] =
	{
		{"no-gui",	no_argument, 0, 'n'},
		{"dump",	no_argument, 0, 'd'},
		{"refresh",	required_argument, 0, 'r'},
#if HAS_LIBDMI
		{"verbose",	no_argument, 0, 'v'},
#endif /* HAS_LIBDMI */
		{"help",	no_argument, 0, 'h'},
		{"version",	no_argument, 0, 'V'},
		{0,		0,	     0,  0}
	};

	while((c = getopt_long(argc, argv, ":ndr:vhV", longopts, NULL)) != -1) {
		switch(c) {
			case 'n':
				r = 'N';
				break;
			case 'd':
				r = 'D';
				break;
			case 'r':
				if(atoi(optarg) > 1)
					refreshtime = atoi(optarg);
				break;
#if HAS_LIBDMI
			case 'v':
				verbose = 1;
				break;
#endif /* HAS_LIBDMI */
			case 'h':
				help(stdout, argv);
				exit(EXIT_SUCCESS);
			case 'V':
				version();
				exit(EXIT_SUCCESS);
			case '?':
			default:
				help(stderr, argv);
				exit(EXIT_FAILURE);
		}
	}

	if(HAS_GTK && r == 'G')
		r = 'G';
	else if(!HAS_GTK && HAS_NCURSES)
		r = 'N';

	return r;
}
