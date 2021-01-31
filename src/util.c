/****************************************************************************
*    Copyright © 2014-2021 Xorg
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

/* glibc's stat.h has it but musl's does not. */
#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO)
#endif

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
	tmp = realloc(*str, j + 1);
	ALLOC_CHECK(tmp);
	*str = tmp;

	return j;
}

/* Return a formatted string */
#define BUFFER_COUNT 10
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
#undef BUFFER_COUNT

/* Divide a number (in bytes) with the appropriate prefix */
void find_best_prefix(uint64_t value, enum EnMultipliers multiplier, bool use_si_prefixes, PrefixUnit *pu)
{
	int i, d;
	struct Table { enum EnMultipliers multiplier; char *prefix; uint64_t divisor; };

	const struct Table si_prefixes[] =
	{
		{ MULT_NONE,  UNIT_B,  1    },
		{ MULT_K,     UNIT_KB, 1e3  },
		{ MULT_M,     UNIT_MB, 1e6  },
		{ MULT_G,     UNIT_GB, 1e9  },
		{ MULT_T,     UNIT_TB, 1e12 },
		{ multiplier, "?B",    1    }
	};
	const struct Table binary_prefixes[] =
	{
		{ MULT_NONE,  UNIT_B,   1ULL       },
		{ MULT_K,     UNIT_KIB, 1ULL << 10 },
		{ MULT_M,     UNIT_MIB, 1ULL << 20 },
		{ MULT_G,     UNIT_GIB, 1ULL << 30 },
		{ MULT_T,     UNIT_TIB, 1ULL << 40 },
		{ multiplier, "?iB",    1ULL       }
	};
	const struct Table *prefixes = use_si_prefixes ? si_prefixes : binary_prefixes;
	const ssize_t table_len = (sizeof(si_prefixes) / sizeof(struct Table)) - 1;

	/* Find current multiplier */
	for(i = 0; prefixes[i].multiplier != multiplier; i++);
	pu->init    = true;
	pu->prefix  = prefixes[i].prefix;
	pu->divisor = prefixes[0].divisor;
	if(i >= table_len - 1)
	{
		/* Due to the loop over an enum, this case is not possible */
		MSG_ERROR("multiplier=%i, value=%llu", multiplier, value);
		return;
	}

	/* Find new prefix and new divisor */
	for(i = i + 1, d = 1; (i < table_len) && (d < table_len) && ((value / prefixes[d].divisor) > 0); i++, d++)
	{
		pu->prefix  = prefixes[i].prefix;
		pu->divisor = prefixes[d].divisor;
	}
}

#define TOKEN_LEN 4
/* Duplicate a string and set unit */
char *strdup_and_set_unit(char *str)
{
	if(str == NULL)
		return NULL;

	const ssize_t len = MAXSTR;
	bool full = false;
	ssize_t i = 0, j = 0, free = len, written;
	char *ptr = malloc(len);
	ALLOC_CHECK(ptr);

	while((i < len) && !full)
	{
		if((str[i] == '@') && (i + TOKEN_LEN - 1 < len) && (str[i + TOKEN_LEN - 1] == '@'))
		{
			/* Set unit in destination string */
			if(!strncmp(&str[i], "@0B@", TOKEN_LEN))
				written = snprintf(&ptr[j], free, "%s", UNIT_B);
			else if(!strncmp(&str[i], "@KB@", TOKEN_LEN))
				written = snprintf(&ptr[j], free, "%s", UNIT_KB);
			else if(!strncmp(&str[i], "@MB@", TOKEN_LEN))
				written = snprintf(&ptr[j], free, "%s", UNIT_MB);
			else if(!strncmp(&str[i], "@GB@", TOKEN_LEN))
				written = snprintf(&ptr[j], free, "%s", UNIT_GB);
			else if(!strncmp(&str[i], "@TB@", TOKEN_LEN))
				written = snprintf(&ptr[j], free, "%s", UNIT_TB);
			else
				MSG_ERROR(_("cannot find unit in '%s' string at position %i"), str, i);
			i += TOKEN_LEN;
		}
		else
		{
			/* Copy one character */
			ptr[j]  = str[i];
			written = 1;
			i      += written;
		}
		if((written >= free) && (str[i - 1] != '\0'))
		{
			MSG_WARNING(_("String '%s' is too long, truncating…"), str);
			full = true;
		}
		else
		{
			j    += written;
			free -= written;
		}
	}
	ptr[j - 1] = '\0';

	return ptr;
}
#undef TOKEN_LEN

/* Check is string is empty (e.g. contains only non printable characters) */
bool string_is_empty(char *str)
{
	int i;

	if(str == NULL)
		return true;

	for(i = 0; (!isalnum(str[i])) && (str[i] != '\0'); i++);

	return (str[i] == '\0');
}

/* Similar to format(), but string can be colorized */
char *colorized_msg(const char *color, const char *str, ...)
{
	char fmt[MSG_BUFF_LEN];
	static char buff[MSG_BUFF_LEN];
	va_list aptr;

	va_start(aptr, str);
	if(opts->color)
		snprintf(fmt, MSG_BUFF_LEN, "%s%s%s\n", color, str, DEFAULT);
	else
		snprintf(fmt, MSG_BUFF_LEN, "%s\n", str);
	vsnprintf(buff, MSG_BUFF_LEN, fmt, aptr);
	va_end(aptr);

	return buff;
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
	asprintf(buffer, "%s", tmp);
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
	char tmp[MAXSTR], error_str[MAXSTR] = "unknown";
	char *cmd_str = NULL;
	FILE *pipe_descr = NULL;
	va_list aptr;

	va_start(aptr, str);
	vasprintf(&cmd_str, str, aptr);
	va_end(aptr);

	if((pipe_descr = popen(cmd_str, "r")) == NULL)
		GOTO_ERROR("popen");

	if(fgets(tmp, MAXSTR, pipe_descr) == NULL)
		GOTO_ERROR("fgets");

	tmp[strlen(tmp) - 1] = '\0';
	asprintf(buffer, "%s", tmp);
	free(cmd_str);

	return pclose(pipe_descr);

error:
	MSG_ERROR(_("an error occurred while running command '%s' (%s)"), cmd_str, error_str);
	free(cmd_str);
	return (pipe_descr == NULL) ? 1 : 2 + pclose(pipe_descr);
}

/* Check if a command exists */
bool command_exists(char *cmd)
{
	char buff[MAXSTR];

	snprintf(buff, MAXSTR, "command -v %s > /dev/null", cmd);

	return !system(buff);
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
	regex_t regex_filename_temp_in, regex_filename_temp_lab, regex_filename_in_in, regex_dirname_cardN, regex_dirname_hwmonN;
	regex_t regex_label_coreN, regex_label_tdie, regex_label_other;

	if((dp = opendir(base_dir)) == NULL)
	{
		MSG_ERROR(_("failed to open %s directory"), base_dir);
		return 1;
	}

	if(regcomp(&regex_filename_temp_in,  "temp1_input",                                     REG_NOSUB)             ||
	   regcomp(&regex_filename_temp_lab, "temp[[:digit:]]_label",                           REG_NOSUB)             ||
	   regcomp(&regex_filename_in_in,    "in0_input",                                       REG_NOSUB)             ||
	   regcomp(&regex_dirname_cardN,     "card[[:digit:]]",                                 REG_NOSUB)             ||
	   regcomp(&regex_dirname_hwmonN,    "hwmon[[:digit:]]",                                 REG_NOSUB)             ||
	   regcomp(&regex_label_coreN,       format("Core[[:space:]]*%u", opts->selected_core), REG_NOSUB | REG_ICASE) ||
	   regcomp(&regex_label_tdie,        "Tdie",                                            REG_NOSUB | REG_ICASE) ||
	   regcomp(&regex_label_other,       "CPU",                                             REG_NOSUB | REG_ICASE))
	{
		MSG_ERROR("%s", _("an error occurred while compiling regex"));
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

		/* Ignore batteries */
		if((sensor != NULL) && (strcasestr(sensor, "bat") != NULL))
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
				{
					/* 'sensors' output with Ryzen CPUs since Linux 5.6:
					Tdie:         +41.4°C */
					if((err = get_sensor_path(path, &regex_filename_temp_lab, &regex_label_tdie, cached_path)))
						/* 'sensors' output for other cases:
						temp1:         +29.5°C  (high = +70.0°C, crit = +90.0°C, hyst = +87.0°C) */
						err = get_sensor_path(path, &regex_filename_temp_in, NULL, cached_path);
				}
				else if(strstr(sensor, "zenpower") != NULL)
					/* 'sensors' output:
					Tdie:         +67.9°C  (high = +95.0°C) */
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
			case RQT_GPU_HWMON:
				err = get_directory_path(path, &regex_dirname_hwmonN, cached_path);
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
	regfree(&regex_dirname_hwmonN);
	regfree(&regex_label_coreN);
	regfree(&regex_label_tdie);
	regfree(&regex_label_other);

	MSG_DEBUG("request_sensor_path(base_dir=%s, cached_path=%s, which=%i) ==> %i", base_dir, *cached_path, which, err);
	return err;
}

const char *start_daemon(bool graphical)
{
	int wstatus = -1;
	pid_t pid;
	char *msg          = NULL;
	char *const appdir = getenv("APPDIR");
	char *const daemon = (appdir == NULL) ? DAEMON_PATH : format("/tmp/%s", DAEMON_EXEC);
	char *const cmd1[] = { daemon, NULL };
	char *const cmd2[] = { "pkexec", daemon, NULL };
	char *const cmd3[] = { "pkexec", "--disable-internal-agent", daemon, NULL };
	char *const *cmd   = cmd2;

	if(appdir != NULL)
	{
		/* Hack to allow pkexec to run daemon (when running from AppImage) */
		char *const cmdcopy = format("cp %s/%s %s", appdir, DAEMON_PATH, daemon);
		system(cmdcopy);
	}

	if(graphical)
		cmd = cmd3;
	else if(IS_ROOT)
		cmd = cmd1;

	pid = fork();
	if(pid < 0)
		MSG_ERRNO("%s", "fork");
	else if(pid == 0)
		execvp(cmd[0], cmd);
	else
		waitpid(pid, &wstatus, 0);

	switch(WEXITSTATUS(wstatus))
	{
		case 0:
			msg = NULL; // Normal status code
			break;
		case 126:
			msg = N_("pkexec: authorization could not be obtained (dialog dismissed)");
			break;
		case 127:
			msg = N_("pkexec: authorization could not be obtained (not authorized)");
			break;
		case 255:
			msg = N_("pkexec: command not found");
			break;
		default:
			msg = N_("pkexec: unexpected error code");
			break;
	}

	if(msg != NULL)
		MSG_WARNING("%s", _(msg));

	return msg;
}

bool daemon_is_alive()
{
	struct stat statbuf;

	int ret = stat(SOCKET_NAME, &statbuf);

	return !ret && (statbuf.st_uid == 0) && S_ISSOCK(statbuf.st_mode) && (statbuf.st_mode & ACCESSPERMS);
}
