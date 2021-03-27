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
* FILE daemon.c
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cpu-x.h"
#include "daemon.h"
#include "ipc.h"

#ifdef __linux__
# include <sys/mount.h>
#endif

#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
#endif


static bool quit_loop  = false;
static ThreadsInfo *ti = &(ThreadsInfo)
{
	.count     = 0,
	.allocated = 1,
	.thread    = NULL,
};

char *colorized_msg(const char *__color, const char *str, ...)
{
	UNUSED(__color);
	char fmt[MSG_BUFF_LEN];
	static char buff[MSG_BUFF_LEN];
	va_list aptr;

	va_start(aptr, str);
	snprintf(fmt, MSG_BUFF_LEN, "%s\n", str);
	vsnprintf(buff, MSG_BUFF_LEN, fmt, aptr);
	va_end(aptr);

	return buff;
}

#if HAS_LIBCPUID
static void libcpuid_msr_serialize(void)
{
	struct msr_driver_t *msr = cpu_msr_driver_open_core(0);

	if(msr != NULL)
	{
		MSG_STDOUT("libcpuid version %s", cpuid_lib_version());
		msr_serialize_raw_data(msr, NULL);
		cpu_msr_driver_close(msr);
	}
}

/* Try to open a CPU MSR */
static int libcpuid_init_msr(int fd, struct msr_driver_t **msr)
{
	unsigned selected_core = 0;

	read(fd, &selected_core, sizeof(uint8_t)); // Core 0 on failure
	if((*msr = cpu_msr_driver_open_core(selected_core)) == NULL)
	{
		MSG_ERROR("cpu_msr_driver_open_core(%u) (%s)", selected_core, cpuid_error());
		return 2;
	}

	return 0;
}

static int __call_libcpuid_msr_static(int *fd)
{
	struct msr_driver_t *msr = NULL;
	MsrStaticData msg = { CPU_INVALID_VALUE, CPU_INVALID_VALUE, CPU_INVALID_VALUE };

	if(!libcpuid_init_msr(*fd, &msr))
	{
		msg.min_mult = cpu_msrinfo(msr, INFO_MIN_MULTIPLIER);
		msg.max_mult = cpu_msrinfo(msr, INFO_MAX_MULTIPLIER);
		msg.bclk     = cpu_msrinfo(msr, INFO_BCLK);
		cpu_msr_driver_close(msr);
	}

	SEND_DATA(fd, &msg, sizeof(MsrStaticData));

	return 0;
}

static int __call_libcpuid_msr_dynamic(int *fd)
{
	struct msr_driver_t *msr = NULL;
	MsrDynamicData msg = { CPU_INVALID_VALUE, CPU_INVALID_VALUE };

	if(!libcpuid_init_msr(*fd, &msr))
	{
		msg.voltage = cpu_msrinfo(msr, INFO_VOLTAGE);
		msg.temp    = cpu_msrinfo(msr, INFO_TEMPERATURE);

		cpu_msr_driver_close(msr);
	}

	SEND_DATA(fd, &msg, sizeof(MsrDynamicData));

	return 0;
}
#endif /* HAS_LIBCPUID */

#if HAS_DMIDECODE
static int __call_dmidecode(int *fd)
{
	DmidecodeData msg = { .ret = -1, .dimm_count = 0, .bus_freq = 0.0 };

	msg.ret = dmidecode(1, &msg);
	SEND_DATA(fd, &msg, sizeof(DmidecodeData));

	return msg.ret;
}
#endif /* HAS_DMIDECODE */

#if HAS_LIBPCI
static int __find_devices(int *fd)
{
	int ret = -1;

#ifdef __FreeBSD__
	ret = chmod(DEV_PCI, DEFFILEMODE); // 0666
#endif /* __FreeBSD__ */
	SEND_DATA(fd, &ret, sizeof(int));

	return ret;
}
#endif /* HAS_LIBPCI */

static int __can_access_sys_debug_dri(int *fd)
{
	int ret = -1;

#ifdef __linux__
	if(access(SYS_DEBUG_DRI, R_OK))
	{
		mkdir(SYS_DEBUG, S_IRWXU|S_IXGRP|S_IXOTH);
		mount("none", SYS_DEBUG, "debugfs", MS_NOSUID|MS_NODEV|MS_NOEXEC, "");
	}
	ret = chmod(SYS_DEBUG, S_IRWXU|S_IXGRP|S_IXOTH); // 0711
#endif /* __linux__ */
	SEND_DATA(fd, &ret, sizeof(int));

	return ret;
}

/* Load a kernel module */
static int __load_module(int *fd)
{
	int ret = -1;
	ssize_t len;
	char *module = NULL, *load_cmd = NULL;

	RECEIVE_DATA(fd, &len, sizeof(ssize_t));
	if((module = malloc(len)) == NULL)
	{
		MSG_ERRNO("%s", "malloc");
		close(*fd);
		*fd = -1;
		return 1;
	}
	RECEIVE_DATA(fd, module, len);

#if defined (__linux__)
	asprintf(&load_cmd, "modprobe %s 2> /dev/null", module);
#elif defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	asprintf(&load_cmd, "kldload -n %s 2> /dev/null", module);
#else
# error "Unsupported operating system"
#endif

	ret = system(load_cmd);
	free(module);
	free(load_cmd);
	SEND_DATA(fd, &ret, sizeof(int));

	return 0;
}

static void cleanup_thread(void *p_data)
{
	Thread *td = (Thread*) p_data;

	pthread_mutex_lock(&ti->mutex);
	ti->count--;
	close(td->fd);
	td->fd = -1;
	pthread_mutex_unlock(&ti->mutex);
}

static void *request_handler(void *p_data)
{
	DaemonCommand cmd;
	Thread *td = (Thread*) p_data;

	pthread_cleanup_push(cleanup_thread, p_data);

	while(42)
	{
		if(read(td->fd, &cmd, sizeof(DaemonCommand)) != sizeof(DaemonCommand))
			break;

		switch(cmd)
		{
#if HAS_LIBCPUID
			case LIBCPUID_MSR_STATIC:
			__call_libcpuid_msr_static(&td->fd);  break;
			case LIBCPUID_MSR_DYNAMIC: __call_libcpuid_msr_dynamic(&td->fd); break;
#endif /* HAS_LIBCPUID */
#if HAS_DMIDECODE
			case DMIDECODE:            __call_dmidecode(&td->fd);            break;
#endif /* HAS_DMIDECODE */
#if HAS_LIBPCI
			case ACCESS_DEV_PCI:       __find_devices(&td->fd);              break;
#endif /* HAS_LIBPCI */
			case ACCESS_SYS_DEBUG:     __can_access_sys_debug_dri(&td->fd);  break;
			case LOAD_MODULE:          __load_module(&td->fd);               break;
			default: MSG_WARNING(_("request_handler: case %i not handled"), cmd);
		}
	}

	pthread_cleanup_pop(1);
	return NULL;
}

static void create_thread(int fd)
{
	uint8_t i, j;
	Thread *tmp;

	/* Search available index */
	if(pthread_mutex_lock(&ti->mutex) < 0)
	{
		MSG_ERRNO("%s", "pthread_mutex_lock");
		close(fd);
	}
	for(i = 0; i < ti->allocated; i++)
		if(ti->thread[i].fd < 0)
			goto found;

	/* Extend array if no available index */
	if((tmp = realloc(ti->thread, (ti->allocated * 2) * sizeof(Thread))) == NULL)
	{
		MSG_ERRNO("%s", "realloc");
		close(fd);
		pthread_mutex_unlock(&ti->mutex);
		return;
	}
	ti->allocated *= 2;
	ti->thread     = tmp;
	for(j = i; j < ti->allocated; j++)
		ti->thread[j].fd = -1;

found:
	/* Add file descriptor in array */
	ti->count++;
	ti->thread[i].fd = fd;
	pthread_mutex_unlock(&ti->mutex);

	/* Start a new thread */
	pthread_create(&ti->thread[i].id, NULL, request_handler, &ti->thread[i]);
	pthread_detach(ti->thread[i].id);
}

static void sighandler(int __signum)
{
	UNUSED(__signum);
	quit_loop = true;
}

int main(void)
{
	int listen_socket, data_socket, ret, err = EXIT_SUCCESS;
	uint8_t i;
	pid_t pid;
	char error_str[MAXSTR] = "unknown";
	struct sockaddr_un name;
	struct pollfd fds[NFDS];

	/* Pre-initialization */
	umask(0);
	unlink(SOCKET_NAME);
	signal(SIGINT,  sighandler);
	signal(SIGTERM, sighandler);

	/* Logs */
	unlink(LOG_FILE);
	freopen(LOG_FILE, "a", stdout);
	setvbuf(stdout, NULL, _IONBF, 0);
	dup2(STDOUT_FILENO, STDERR_FILENO);
	PRGINFO(stdout);
#if HAS_LIBCPUID
	if(getenv("CPUX_DAEMON_DEBUG"))
		libcpuid_msr_serialize();
#endif /* HAS_LIBCPUID */

	/* Initialize mutex */
	if(pthread_mutex_init(&ti->mutex, NULL) < 0)
	{
		MSG_ERRNO("%s", "pthread_mutex_init");
		return EXIT_FAILURE;
	}

	/* Initialize threads array */
	if((ti->thread = malloc(ti->allocated * sizeof(Thread))) == NULL)
	{
		MSG_ERRNO("%s", "malloc");
		pthread_mutex_destroy(&ti->mutex);
		return EXIT_FAILURE;
	}
	for(i = 0; i < ti->allocated; i++)
		ti->thread[i].fd = -1;

	/* Create local socket. */
	if((listen_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
		GOTO_ERROR("socket");

	/* Bind socket to socket name. */
	memset(&name, 0, sizeof(struct sockaddr_un));
	name.sun_family = AF_UNIX;
	strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
	if(bind(listen_socket, (const struct sockaddr*) &name, sizeof(struct sockaddr_un)) < 0)
		GOTO_ERROR("bind");

	/* Prepare for accepting connections. */
	if(listen(listen_socket, SOMAXCONN) < 0)
		GOTO_ERROR("listen");

	/* Initialize pollfd */
	memset(fds, 0 , sizeof(fds));
	fds[0].fd     = listen_socket;
	fds[0].events = POLLIN;

	/* Fork daemon in background */
	pid = fork();
	if(pid > 0)
		return 0;
	else if(pid < 0)
		GOTO_ERROR("fork");

	/* This is the main loop for handling connections */
	while(!quit_loop)
	{
		/* Wait for incoming connection */
		ret = poll(fds, NFDS, POLL_TIMEOUT);
		if(ret < 0)
			MSG_ERRNO("%s", "poll");
		else if(ret == 0) // Timeout
			quit_loop = ti->count == 0;
		else if(fds[0].revents & POLLIN)
		{
			if((data_socket = accept(listen_socket, NULL, NULL)) < 0)
				MSG_ERRNO("%s", "accept");
			else
				create_thread(data_socket);
		}
	}
	goto clean;

error:
	MSG_ERRNO("%s", error_str);
	err = EXIT_FAILURE;

clean:
	for(i = 0; i < ti->count; i++)
		pthread_join(ti->thread[i].id, NULL);
	fclose(stdout);
	unlink(SOCKET_NAME);
	close(listen_socket);
	free(ti->thread);
	pthread_mutex_destroy(&ti->mutex);

	if(getenv("APPDIR") != NULL)
		remove("/tmp/"DAEMON_EXEC);

	return err;
}
