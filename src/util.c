/****************************************************************************
*    Copyright © 2014-2016 Xorg
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
* PROJECT CPU-X
* FILE util.c
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include "cpu-x.h"


/************************* Private functions *************************/

/* Run system() command, depending of running operating system */
static int system_os_specific(char *cmd_linux, char *cmd_bsd)
{
#if defined (__linux__)
	return system(cmd_linux);
#elif defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	return system(cmd_bsd);
#else
	return -255;
#endif
}


/************************* Public function *************************/

/* Add a newline for given string (used by MSG_XXX macros) */
char *msg_newline(char *color, char *str)
{
	static char *buff = NULL;

	free(buff);
	if(opts->color)
		asprintf(&buff, "%s%s%s\n", color, str, DEFAULT);
	else
		asprintf(&buff, "%s\n",     str);

	return buff;
}

/* Add a newline and more informations for given string (used by MSG_ERROR macro) */
char *msg_error(char *color, char *file, int line, char *str)
{
	static char *buff = NULL;

	free(buff);
	if(errno)
		asprintf(&buff, "%s%s:%s:%i: %s (%s)%s\n", opts->color ? color : DEFAULT, PRGNAME, file, line, str, strerror(errno), DEFAULT);
	else
		asprintf(&buff, "%s%s:%s:%i: %s%s\n", opts->color ? color : DEFAULT, PRGNAME, file, line, str, DEFAULT);

	errno = 0;
	return buff;
}

/* An asprintf-like function, but which can clean some parts of 'str' if 'clean_str' is true
 * - It calls vasprintf if 'fmt' is a valid string
 * - If 'clean_str' is true, it removes "unvalid args" from 'str' until next "valid arg"
     E.g.: casprintf(&str, false, "%i nm", 0): str = "0 nm"
           casprintf(&str, true,  "%i nm", 0): str = ""
	   casprintf(&str, true,  "%i nm", 32): str = "32 nm"
	   casprintf(&str, true,  "%i KB %i-way", -1, 12): str = "12-way" */
int casprintf(char **str, bool clean_str, const char *fmt, ...)
{
	bool remove;
	int i, j, ret;
	char *tmp = NULL;
	va_list aptr;

	if(*str != NULL)
	{
		free(*str);
		*str = NULL;
	}

	if(fmt == NULL)
		return 0;

	va_start(aptr, fmt);
	ret = vasprintf(str, fmt, aptr);
	va_end(aptr);

	if(!clean_str)
		return ret;

	remove = (((*str)[0] == '0') && (atof(*str) == 0.0)) || (atoi(*str) < 0);
	j      = remove ? -1 : 0;

	for(i = 1; (*str)[i] != '\0'; i++)
	{
		if(((((*str)[i] == '0') && (atof(*str + i) == 0.0)) || (atoi(*str + i) < 0)) && (isspace((*str)[i - 1])))
			remove = true;
		if(!remove)
			(*str)[++j] = (*str)[i];
		if((isspace((*str)[i])) && !(isdigit((*str)[i - 1])))
			remove = false;
	}

	(*str)[++j] = '\0';
	if((tmp = realloc(*str, j + 1)) != NULL)
		*str = tmp;

	return j;
}

/* Return a formatted string */
char *format(char *str, ...)
{
	static char *buff = NULL;
	va_list aptr;

	free(buff);
	va_start(aptr, str);
	vasprintf(&buff, str, aptr);
	va_end(aptr);

	return buff;
}

/* Check if a command exists */
bool command_exists(char *command)
{
	return !system(format("which %s >/dev/null 2>&1", command));
}

/* Open a file and put its content in a variable ('str' accept printf-like format) */
int fopen_to_str(char **buffer, char *str, ...)
{
	char tmp[MAXSTR];
	char *file_str = NULL;
	FILE *file_descr = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&file_str, str, aptr);
	va_end(aptr);

	if(access(file_str, R_OK))
	{
		free(file_str);
		return -1;
	}

	if((file_descr = fopen(file_str, "r")) == NULL)
		goto error;

	if(fgets(tmp, MAXSTR, file_descr) == NULL)
		goto error;

	tmp[strlen(tmp) - 1] = '\0';
	asprintf(buffer, tmp);
	free(file_str);

	return fclose(file_descr);

error:
	MSG_ERROR(_("an error occurred while opening file '%s'"), file_str);
	free(file_str);
	return (file_descr == NULL) ? 1 : 2 + fclose(file_descr);
}

/* Run a command and put output in a variable ('str' accept printf-like format) */
int popen_to_str(char **buffer, char *str, ...)
{
	bool command_ok;
	char tmp[MAXSTR];
	char *cmd_str = NULL, *test_command  = NULL;
	FILE *pipe_descr = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&cmd_str, str, aptr);
	va_end(aptr);

	test_command = strdup(cmd_str);
	command_ok = command_exists(strtok(test_command, " "));
	free(test_command);
	if(!command_ok)
	{
		free(cmd_str);
		return -1;
	}

	if((pipe_descr = popen(cmd_str, "r")) == NULL)
		goto error;

	if(fgets(tmp, MAXSTR, pipe_descr) == NULL)
		goto error;

	tmp[strlen(tmp) - 1] = '\0';
	asprintf(buffer, tmp);
	free(cmd_str);

	return pclose(pipe_descr);

error:
	MSG_ERROR(_("an error occurred while running command '%s'"), cmd_str);
	free(cmd_str);
	return (pipe_descr == NULL) ? 1 : 2 + pclose(pipe_descr);
}

/* Load a kernel module */
bool load_module(char *module)
{
	if(!system_os_specific(format("lsmod | grep %s > /dev/null", module),
	                       format("kldstat | grep %s > /dev/null", module)))
		return true;
	else if(getuid())
		return false;
	else
		return !system_os_specific(format("modprobe %s 2> /dev/null", module),
		                           format("kldload -n %s 2> /dev/null", module));
}
