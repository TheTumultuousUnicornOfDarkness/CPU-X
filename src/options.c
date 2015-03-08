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
#include <locale.h>
#include <libintl.h>
#include "cpu-x.h"

int refreshtime = 1;
int verbose = 0;

const char *optstring[] =
{	"ncurses",
	"dump",
	"refresh",
#if HAS_LIBDMI
	"dmidecode",
	"verbose",
#endif /* HAS_LIBDMI */
	"help",
	"version"
};


void help(FILE *out, char *argv[]) {
	fprintf(out, _("Usage: %s [OPTION]\n\n"
		"Available OPTION:\n"
		"  -n, --%-10s Start text-based user interface (TUI)\n"
		"  -d, --%-10s Dump all data on standard output and exit\n"
		"  -r, --%-10s Set custom time between two refreshes (in seconds)\n"
#if HAS_LIBDMI
		"  -D, --%-10s Run embedded command dmidecode and exit\n"
		"  -v, --%-10s Verbose output (in Dmidecode)\n"
#endif /* HAS_LIBDMI */
		"  -h, --%-10s Print help and exit\n"
		"  -V, --%-10s Print version and exit\n"), argv[0],
		optstring[0], optstring[1], optstring[2], optstring[3], optstring[4]
#if HAS_LIBDMI
		, optstring[5], optstring[6]
#endif /* HAS_LIBDMI */
		);
}

void version(void) {
	printf(_("%s %s\n"
	"%s\n\n"
	"This is free software: you are free to change and redistribute it.\n"
	"This program comes with ABSOLUTELY NO WARRANTY\n"
	"See the GPLv3 license: <http://www.gnu.org/licenses/gpl.txt>\n"), PRGNAME, PRGVER, PRGCPYR);
}

char menu(int argc, char *argv[]) {
	int c;
	char r = 'G';
	const struct option longopts[] =
	{
		{optstring[0],	no_argument, 0, 'n'}, /* Arg ncurses */
		{optstring[1],	no_argument, 0, 'd'}, /* Arg dump */
		{optstring[2],	required_argument, 0, 'r'}, /* Arg refresh */
#if HAS_LIBDMI
		{optstring[3],	no_argument, 0, 'D'}, /* Arg Dmidecode */
		{optstring[4],	no_argument, 0, 'v'}, /* Arg verbose */
#endif /* HAS_LIBDMI */
		{optstring[5],	no_argument, 0, 'h'}, /* Arg help */
		{optstring[6],	no_argument, 0, 'V'}, /* Arg version */
		{0,		0,	     0,  0}
	};

	setlocale(LC_ALL, "");
	bindtextdomain("cpux", get_path("locale"));
	textdomain("cpux");

	while((c = getopt_long(argc, argv, ":ndDr:vhV", longopts, NULL)) != -1) {
		switch(c) {
			case 'n':
			case 'd':
				r = c;
				break;
			case 'r':
				if(atoi(optarg) > 1)
					refreshtime = atoi(optarg);
				break;
#if HAS_LIBDMI
			case 'D':
				r = c;
				verbose += 2;
				break;
			case 'v':
				verbose++;
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

	if(!HAS_GTK && HAS_NCURSES && r == 'G')
		r = 'n';

	return r;
}
