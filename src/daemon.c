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
* FILE daemon.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libcpuid/libcpuid.h>
#include "cpu-x.h"
#include "daemon.h"
#include "ipc.h"

static ThreadsInfo *ti = &(ThreadsInfo)
{
	.count     = 0,
	.allocated = 1,
	.thread    = NULL,
};

char *colorized_msg(const char *color, const char *str, ...)
{
	//STUB
	return NULL;
}

#if HAS_LIBCPUID
/* Try to open a CPU MSR */
static int libcpuid_init_msr(int fd, struct msr_driver_t **msr)
{
	unsigned selected_core = 0;

	read(fd, &selected_core, sizeof(uint8_t)); // Core 0 on failure
	if((*msr = cpu_msr_driver_open_core(selected_core)) == NULL)
	{
		MSG_ERROR(_("failed to open CPU#%u MSR (%s)"), selected_core, cpuid_error());
		return 2;
	}

	return 0;
}

static void __call_libcpuid_msr_static(int fd)
{
	struct msr_driver_t *msr = NULL;
	MsrStaticData msg = { CPU_INVALID_VALUE, CPU_INVALID_VALUE, CPU_INVALID_VALUE };

	if(!libcpuid_init_msr(fd, &msr))
	{
		msg.min_mult = cpu_msrinfo(msr, INFO_MIN_MULTIPLIER);
		msg.max_mult = cpu_msrinfo(msr, INFO_MAX_MULTIPLIER);
		msg.bclk     = cpu_msrinfo(msr, INFO_BCLK);

#if 0 //TODO
#ifdef HAVE_MSR_SERIALIZE_RAW_DATA
		if(opts->issue)
			msr_serialize_raw_data(msr, "");
#endif /* HAVE_MSR_SERIALIZE_RAW_DATA */
#endif

		cpu_msr_driver_close(msr);
	}

	write(fd, &msg, sizeof(MsrStaticData));
}

static void __call_libcpuid_msr_dynamic(int fd)
{
	struct msr_driver_t *msr = NULL;
	MsrDynamicData msg = { CPU_INVALID_VALUE, CPU_INVALID_VALUE };

	if(!libcpuid_init_msr(fd, &msr))
	{
		msg.voltage = cpu_msrinfo(msr, INFO_VOLTAGE);
		msg.temp    = cpu_msrinfo(msr, INFO_TEMPERATURE);

		cpu_msr_driver_close(msr);
	}

	write(fd, &msg, sizeof(MsrDynamicData));
}
#endif /* HAS_LIBCPUID */

#if HAS_DMIDECODE
static int __call_dmidecode(int fd)
{
	//TODO
	return 0;
}
#endif /* HAS_DMIDECODE */

static void __popen_to_str(int fd)
{
	size_t len;
	char *cmd_str = NULL, tmp[MAXSTR] = "";
	FILE *pipe_descr = NULL;

	/* Get command from client */
	read(fd, &len, sizeof(size_t));
	cmd_str = malloc(len);
	read(fd, cmd_str, len);

	if((pipe_descr = popen(cmd_str, "r")) == NULL)
		perror("popen");
	free(cmd_str);

	if(fgets(tmp, MAXSTR, pipe_descr) == NULL)
		perror("fgets");
	fclose(pipe_descr);

	/* Send string to client */
	len = strlen(tmp);
	tmp[len - 1] = '\0';
	len++;
	write(fd, &len, sizeof(size_t));
	write(fd, tmp, len);
}

/* Load a kernel module */
static bool __load_module(int fd)
{
	//TODO
	(void) fd;
#if 0
	char *module = "";
#if defined (__linux__)
	if(!system(format("grep -wq %s /proc/modules 2> /dev/null", module)))
		return true;
	else if(getuid())
		return false;
	else
		return !system(format("modprobe %s 2> /dev/null", module));
#elif defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	if(!system(format("kldstat | grep %s > /dev/null", module)))
		return true;
	else if(getuid())
		return false;
	else
		return !system(format("kldload -n %s 2> /dev/null", module));
#else
	return false;
#endif
#endif
	return false;
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
			case LIBCPUID_MSR_STATIC:  if(HAS_LIBCPUID)  __call_libcpuid_msr_static(td->fd);  break;
			case LIBCPUID_MSR_DYNAMIC: if(HAS_LIBCPUID)  __call_libcpuid_msr_dynamic(td->fd); break;
			case DMIDECODE:            if(HAS_DMIDECODE) __call_dmidecode(td->fd);            break;
			case POPEN_TO_STR:                           __popen_to_str(td->fd);              break;
			case LOAD_MODULE:                            __load_module(td->fd);               break;
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
		MSG_ERRNO("pthread_mutex_lock");
		close(fd);
	}
	for(i = 0; i < ti->allocated; i++)
		if(ti->thread[i].fd < 0)
			goto found;

	/* Extend array if no available index */
	if((tmp = realloc(ti->thread, (ti->allocated * 2) * sizeof(Thread))) == NULL)
	{
		MSG_ERRNO("realloc");
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

int main(void)
{
	bool quit_loop = false;
	int listen_socket, data_socket, ret, err = EXIT_SUCCESS;
	uint8_t i;
	char error_str[MAXSTR] = "unknown";
	struct sockaddr_un name;
	struct pollfd fds[NFDS];

	/* Pre-initialization */
	umask(0);
	unlink(SOCKET_NAME);

	/* Initialize mutex */
	if(pthread_mutex_init(&ti->mutex, NULL) < 0)
	{
		MSG_ERRNO(_("failed to initialize mutex"));
		return EXIT_FAILURE;
	}

	/* Initialize threads array */
	if((ti->thread = malloc(ti->allocated * sizeof(Thread))) == NULL)
	{
		MSG_ERRNO(_("failed to allocate memory"));
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

	/* This is the main loop for handling connections */
	while(!quit_loop)
	{
		/* Wait for incoming connection */
		ret = poll(fds, NFDS, POLL_TIMEOUT);
		if(ret < 0)
			MSG_ERRNO("poll");
		else if(ret == 0) // Timeout
			quit_loop = ti->count == 0;
		else if(fds[0].revents & POLLIN)
		{
			if((data_socket = accept(listen_socket, NULL, NULL)) < 0)
				MSG_ERRNO("accept");
			else
				create_thread(data_socket);
		}
	}
	goto clean;

error:
	MSG_ERRNO(_("an error occurred while starting daemon: %s"), error_str);
	err = EXIT_FAILURE;

clean:
	unlink(SOCKET_NAME);
	close(listen_socket);
	free(ti->thread);
	pthread_mutex_destroy(&ti->mutex);

	return err;
}
