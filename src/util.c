/****************************************************************************
*    Copyright © 2014-2019 Xorg
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
#include <dirent.h>
#include <regex.h>
#include <libintl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "cpu-x.h"
#include "ipc.h"


/************************* Public function *************************/

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
#define BUFFER_COUNT 5
char *format(char *str, ...)
{
	static unsigned count = 0;
	static char *buff[BUFFER_COUNT] = { NULL };
	va_list aptr;

	count++;
	const unsigned index = count % BUFFER_COUNT;
	if(count >= BUFFER_COUNT)
		free(buff[index]);

	va_start(aptr, str);
	vasprintf(&buff[index], str, aptr);
	va_end(aptr);

	return buff[index];
}

/* Similar to format(), but string can be colorized */
char *colorized_msg(const char *color, const char *str, ...)
{
	static char *buff;
	va_list aptr;

	free(buff);
	va_start(aptr, str);
	vasprintf(&buff, format("%s%s%s\n", opts->color ? color : DEFAULT, str, DEFAULT), aptr);
	va_end(aptr);

	return buff;
}

/* Check if a command exists */
bool command_exists(char *command)
{
	bool exists = false;
	char *dir;
	char *save_path = strdup(getenv("PATH"));
	char *path = save_path;

	while(((dir = strsep(&path, ":")) != NULL) && !exists)
		exists = !access(format("%s/%s", dir, command), X_OK);
	free(save_path);

	return exists;
}

/* Open a file and put its content in a variable ('str' accept printf-like format) */
int fopen_to_str(char **buffer, char *str, ...)
{
	char tmp[MAXSTR], error_str[MAXSTR] = "unknown";
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
		GOTO_ERROR("fopen");

	if(fgets(tmp, MAXSTR, file_descr) == NULL)
		GOTO_ERROR("fgets");

	tmp[strlen(tmp) - 1] = '\0';
	asprintf(buffer, tmp);
	free(file_str);

	return fclose(file_descr);

error:
	MSG_ERROR(_("an error occurred while opening file '%s' (%s)"), file_str, error_str);
	free(file_str);
	return (file_descr == NULL) ? 1 : 2 + fclose(file_descr);
}

/* Run a command and put output in a variable ('str' accept printf-like format) */
int popen_to_str(char **buffer, char *str, ...)
{
	bool command_ok;
	char tmp[MAXSTR], error_str[MAXSTR] = "unknown";
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
		GOTO_ERROR("popen");

	if(fgets(tmp, MAXSTR, pipe_descr) == NULL)
		GOTO_ERROR("fgets");

	tmp[strlen(tmp) - 1] = '\0';
	asprintf(buffer, tmp);
	free(cmd_str);

	return pclose(pipe_descr);

error:
	MSG_ERROR(_("an error occurred while running command '%s' (%s)"), cmd_str, error_str);
	free(cmd_str);
	return (pipe_descr == NULL) ? 1 : 2 + pclose(pipe_descr);
}

int privileged_popen_to_str(char **buffer, int *fd, char *str, ...)
{
	char *cmd_str = NULL;
	const DaemonCommand cmd = POPEN_TO_STR;
	va_list aptr;

	va_start(aptr, str);
	ssize_t len = vasprintf(&cmd_str, str, aptr);
	va_end(aptr);
	len++;

	/* Send command to daemon */
	SEND_DATA(fd, &cmd, sizeof(DaemonCommand));
	SEND_DATA(fd, &len, sizeof(ssize_t));
	SEND_DATA(fd, cmd_str, len);

	/* Receive string from daemon */
	RECEIVE_DATA(fd, &len, sizeof(ssize_t));
	*buffer = calloc(len, sizeof(char));
	RECEIVE_DATA(fd, *buffer, len);

	return 0;
}

/* Load a kernel module */
int load_module(char *module, int *fd)
{
	int ret = -1;
	char *check_cmd = NULL;
	const ssize_t len = strlen(module) + 1;
	const DaemonCommand cmd = LOAD_MODULE;

#if defined (__linux__)
	asprintf(&check_cmd, "grep -wq %s /proc/modules 2> /dev/null", module);
#elif defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	asprintf(&check_cmd, "kldstat | grep %s > /dev/null", module);
#else
# error "Unsupported operating system"
#endif

	ret = system(check_cmd);
	free(check_cmd);
	if((ret != 0) && (*fd >= 0))
	{
		/* Send module name to daemon */
		SEND_DATA(fd, &cmd, sizeof(DaemonCommand));
		SEND_DATA(fd, &len, sizeof(ssize_t));
		SEND_DATA(fd, module, len);

		/* Receive return value */
		RECEIVE_DATA(fd, &ret, sizeof(int));
	}

	return ret;
}

/* Search a sensor filename in a given directory corresponding to regex */
static int get_sensor_path(char *dir_path, regex_t *regex_filename, regex_t *regex_label, char **cached_path)
{
	int err     = 1;
	char *label = NULL;
	DIR *dp     = NULL;
	struct dirent *dir;

	/* Open given directory */
	if((dp = opendir(dir_path)) == NULL)
	{
		MSG_ERROR(_("failed to open %s directory"), dir_path);
		return 1;
	}

	while(((dir = readdir(dp)) != NULL) && err)
	{
		/* Ignore hidden files and files not mathing pattern */
		if((dir->d_name[0] == '.') || regexec(regex_filename, dir->d_name, 0, NULL, 0))
			continue;

		if(regex_label != NULL)
		{
			/* Open the label file */
			if(fopen_to_str(&label, "%s/%s", dir_path, dir->d_name))
				continue;

			/* Check if label matchs with pattern */
			if(regexec(regex_label, label, 0, NULL, 0))
				continue;
		}

		/* Try to open the corresponding file */
		strtok(dir->d_name, "_");
		casprintf(cached_path, false, "%s/%s_input", dir_path, dir->d_name);
		err = access(*cached_path, R_OK);
	}

	closedir(dp);
	free(label);

	return err;
}

/* Search a directory path corresponding to regex */
static int get_directory_path(char *dir_path, regex_t *regex_dirname, char **cached_path)
{
	int err = regexec(regex_dirname, dir_path, 0, NULL, 0);

	if(!err)
	{
		casprintf(cached_path, false, "%s", dir_path);
		err = access(*cached_path, R_OK);
	}

	return err;
}

/* Get a filename located in a directory corresponding to given request */
int request_sensor_path(char *base_dir, char **cached_path, enum RequestSensor which)
{
	int err      = 1;
	char *sensor = NULL;
	char *path   = NULL;
	DIR *dp      = NULL;
	struct dirent *dir;
	regex_t regex_filename_temp_in, regex_filename_temp_lab, regex_filename_in_in, regex_dirname_cardN;
	regex_t regex_label_coreN, regex_label_other;

	if((dp = opendir(base_dir)) == NULL)
	{
		MSG_ERROR(_("failed to open %s directory"), base_dir);
		return 1;
	}

	if(regcomp(&regex_filename_temp_in,  "temp1_input",                                     REG_NOSUB)             ||
	   regcomp(&regex_filename_temp_lab, "temp[[:digit:]]_label",                           REG_NOSUB)             ||
	   regcomp(&regex_filename_in_in,    "in0_input",                                       REG_NOSUB)             ||
	   regcomp(&regex_dirname_cardN,     "card[[:digit:]]",                                 REG_NOSUB)             ||
	   regcomp(&regex_label_coreN,       format("Core[[:space:]]*%u", opts->selected_core), REG_NOSUB | REG_ICASE) ||
	   regcomp(&regex_label_other,       "CPU",                                             REG_NOSUB | REG_ICASE))
	{
		MSG_ERROR(_("an error occurred while compiling regex"));
		return 2;
	}

	while(((dir = readdir(dp)) != NULL) && err)
	{
		/* Ignore hidden files */
		if(dir->d_name[0] == '.')
			continue;

		/* Find sensor name */
		if((which != RQT_GPU_DRM) && fopen_to_str(&sensor, "%s/%s/name", base_dir, dir->d_name))
			continue;

		/* Browse files in directory */
		casprintf(&path, false, "%s/%s", base_dir, dir->d_name);
		switch(which)
		{
			case RQT_CPU_TEMPERATURE:
				if(strstr(sensor, "coretemp") != NULL)
					/* 'sensors' output:
					Package id 0:  +37.0°C  (high = +80.0°C, crit = +98.0°C)
					Core 0:        +33.0°C  (high = +80.0°C, crit = +98.0°C)
					Core 1:        +34.0°C  (high = +80.0°C, crit = +98.0°C)
					Core 2:        +36.0°C  (high = +80.0°C, crit = +98.0°C)
					Core 3:        +37.0°C  (high = +80.0°C, crit = +98.0°C) */
					err = get_sensor_path(path, &regex_filename_temp_lab, &regex_label_coreN, cached_path);
				else if(strstr(sensor, "k8temp") != NULL)
					/* 'sensors' output:
					Core0 Temp:    +64.0°C
					Core0 Temp:    +63.0°C
					Core1 Temp:    +64.0°C
					Core1 Temp:    +64.0°C */
					err = get_sensor_path(path, &regex_filename_temp_lab, &regex_label_coreN, cached_path);
				else if(strstr(sensor, "k10temp") != NULL)
					/* 'sensors' output:
					temp1:         +29.5°C  (high = +70.0°C, crit = +90.0°C, hyst = +87.0°C) */
					err = get_sensor_path(path, &regex_filename_temp_in, NULL, cached_path);
				break;
			case RQT_CPU_TEMPERATURE_OTHERS:
				err = get_sensor_path(path, &regex_filename_temp_lab, &regex_label_other, cached_path);
				break;
			case RQT_CPU_VOLTAGE:
				/* 'sensors' output:
				Vcore:         +0.88 V  (min =  +0.80 V, max =  +1.38 V) */
				err = get_sensor_path(path, &regex_filename_in_in, NULL, cached_path);
				break;
			case RQT_GPU_TEMPERATURE:
				err = get_sensor_path(path, &regex_filename_temp_in, NULL, cached_path);
				break;
			case RQT_GPU_DRM:
				err = get_directory_path(path, &regex_dirname_cardN, cached_path);
				break;
		}
	}

	closedir(dp);
	free(sensor);
	free(path);
	regfree(&regex_filename_temp_in);
	regfree(&regex_filename_temp_lab);
	regfree(&regex_filename_in_in);
	regfree(&regex_dirname_cardN);
	regfree(&regex_label_coreN);
	regfree(&regex_label_other);

	return err;
}

bool start_daemon(bool use_pkexec)
{
	int wstatus = -1;
	pid_t pid;
	char *const cmd1[] = { DAEMON_PATH, NULL };
	char *const cmd2[] = { "pkexec", DAEMON_PATH, NULL };
	char *const *cmd   = use_pkexec ? cmd2 : cmd1;

	pid = fork();
	if(pid < 0)
		MSG_ERRNO("fork");
	else if(pid == 0)
		execvp(cmd[0], cmd);
	else
		waitpid(pid, &wstatus, 0);

	return (wstatus == 0);
}

bool daemon_is_alive()
{
	struct stat statbuf;

	int ret = stat(SOCKET_NAME, &statbuf);

	return !ret && (statbuf.st_uid == 0) && S_ISSOCK(statbuf.st_mode) && (statbuf.st_mode & ACCESSPERMS);
}
