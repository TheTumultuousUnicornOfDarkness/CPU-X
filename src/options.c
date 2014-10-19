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
#include "includes.h"


void help(FILE *out, char *argv[]) {
	fprintf(out, "Usage: %s [OPTION]\n\n"
		"Available OPTION:\n"
		"\t--no-gui\tStart NCurses mode instead of GTK\n"
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
	static struct option longopts[] =
	{
		{"no-gui",	no_argument, 0, 'n'},
		{"help",	no_argument, 0, 'h'},
		{"version",	no_argument, 0, 'V'},
		{0,		0,	     0,  0}
	};

	if(argc > 1) {
		c = getopt_long(argc, argv, ":nhV", longopts, NULL);
		switch(c) {
			case 'n':
				return 'N';
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
	else {
		if(HAS_GTK)
			return 'G';
		else if(!HAS_GTK && HAS_NCURSES)
			return 'N';
	}
	
}
