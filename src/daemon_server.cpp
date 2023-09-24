/****************************************************************************
*    Copyright © 2014-2023 The Tumultuous Unicorn Of Darkness
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
* FILE daemon_server.cpp
*/

#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <cstring>
#include <csignal>
#include <filesystem>
#include "util.hpp"
#include "daemon.h"
#include "daemon_server.hpp"

#ifdef __linux__
# include <sys/mount.h>
#endif

#if HAS_LIBCPUID
# include <libcpuid/libcpuid.h>
#endif

#if HAS_DMIDECODE
# include "dmidecode/libdmidecode.h"
#endif

namespace fs = std::filesystem;
static bool quit_loop  = false;
static ThreadsInfo ti;


#if HAS_LIBCPUID
/* Try to open a CPU MSR */
static int libcpuid_init_msr(int *fd, struct msr_driver_t **msr)
{
	uint16_t current_core_id = 0;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
	read(*fd, &current_core_id, sizeof(uint16_t)); // Core 0 on failure
	if((*msr = cpu_msr_driver_open_core(current_core_id)) == NULL)
	{
		MSG_ERROR("cpu_msr_driver_open_core(%u) (%s)", current_core_id, cpuid_error());
		return 2;
	}

	return 0;
}

static int __call_libcpuid_msr_debug(int *fd)
{
	uint16_t current_core_id, all_cpu_count = 0;
	struct msr_driver_t *msr;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
	MSG_STDOUT("libcpuid version %s", cpuid_lib_version());
	read(*fd, &all_cpu_count, sizeof(uint16_t)); // CPU count 0 on failure
	for(current_core_id = 0; current_core_id < all_cpu_count; current_core_id++)
	{
		MSG_STDOUT("______________ MSR CPU #%i ______________", current_core_id);
		if((msr = cpu_msr_driver_open_core(current_core_id)) == NULL)
			MSG_ERROR("cpu_msr_driver_open_core(%u) (%s)", current_core_id, cpuid_error());
		else
		{
			msr_serialize_raw_data(msr, NULL);
			cpu_msr_driver_close(msr);
		}
	}

	return 0;
}

static int __call_libcpuid_msr_static(int *fd)
{
	struct msr_driver_t *msr = NULL;
	MsrStaticData msg{};
	msg.min_mult = CPU_INVALID_VALUE;
	msg.max_mult = CPU_INVALID_VALUE;
	msg.bclk     = CPU_INVALID_VALUE;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
	if(!libcpuid_init_msr(fd, &msr))
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
	MsrDynamicData msg{};
	msg.voltage = CPU_INVALID_VALUE;
	msg.temp    = CPU_INVALID_VALUE;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
	if(!libcpuid_init_msr(fd, &msr))
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
	DmidecodeData msg{};
	msg.ret                = -1;
	msg.stick_count        = 0;
	msg.processor.bus_freq = 0.0;
	msg.memory             = NULL;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
	msg.ret = dmidecode(1, &msg);
	SEND_DATA(fd, &msg.ret,         sizeof(int));
	SEND_DATA(fd, &msg.processor,   sizeof(DmidecodeCPUData));
	SEND_DATA(fd, &msg.mb,          sizeof(DmidecodeMBData));
	SEND_DATA(fd, &msg.bios,        sizeof(DmidecodeBiosData));
	SEND_DATA(fd, &msg.stick_count, sizeof(uint8_t));
	for(uint8_t i = 0; i < msg.stick_count; i++)
		SEND_DATA(fd, &msg.memory[i], sizeof(DmidecodeMemoryData));

	return msg.ret;
}
#endif /* HAS_DMIDECODE */

#if HAS_LIBPCI
static int __find_devices(int *fd)
{
	int ret = -1;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
#ifdef __FreeBSD__
	std::error_code fs_code;
	fs::permissions(DEV_PCI, fs::perms::owner_read | fs::perms::owner_write | fs::perms::group_read | fs::perms::group_write | fs::perms::others_read | fs::perms::others_write, fs::perm_options::add, fs_code); // 0666
	if(fs_code)
		ret = 1;
#endif /* __FreeBSD__ */
	SEND_DATA(fd, &ret, sizeof(int));

	return ret;
}
#endif /* HAS_LIBPCI */

static int __can_access_sys_debug_dri(int *fd)
{
	int ret = -1;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
#ifdef __linux__
	std::error_code fs_code;
	if(!fs::is_directory(SYS_DEBUG_DRI))
	{
		fs::create_directory(SYS_DEBUG);
		mount("none", SYS_DEBUG, "debugfs", MS_NOSUID|MS_NODEV|MS_NOEXEC, "");
	}

	fs::permissions(SYS_DEBUG, fs::perms::owner_all | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add, fs_code); // 0711
	if(fs_code)
		ret = 1;

#endif /* __linux__ */
	SEND_DATA(fd, &ret, sizeof(int));

	return ret;
}

/* Load a kernel module */
static int __load_module(int *fd)
{
	int ret = -1;
	ssize_t len;
	char *module = NULL;

	MSG_DEBUG("%s: fd=%i", __func__, *fd);
	RECEIVE_DATA(fd, &len, sizeof(ssize_t));
	module = new char[len];
	RECEIVE_DATA(fd, module, len);

#if defined (__linux__)
	ret = run_command("modprobe %s 2> /dev/null", module);
#elif defined (__DragonFly__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	ret = run_command("kldload -n %s 2> /dev/null", module);
#else
# error "Unsupported operating system"
#endif

	delete[] module;
	SEND_DATA(fd, &ret, sizeof(int));

	return 0;
}

static void cleanup_thread(int fd)
{
	ti.mutex.lock();
	close(fd);
	ti.running_threads--;
	MSG_DEBUG("%s: fd=%i, %u clients are still alive", __func__, fd, ti.running_threads);
	ti.mutex.unlock();
}

static void request_handler(int fd)
{
	DaemonCommand cmd;

	MSG_DEBUG("%s: fd=%i", __func__, fd);
	while(42)
	{
		if(read(fd, &cmd, sizeof(DaemonCommand)) != sizeof(DaemonCommand))
			break;

		switch(cmd)
		{
#if HAS_LIBCPUID
			case LIBCPUID_MSR_DEBUG:   __call_libcpuid_msr_debug(&fd);   break;
			case LIBCPUID_MSR_STATIC:  __call_libcpuid_msr_static(&fd);  break;
			case LIBCPUID_MSR_DYNAMIC: __call_libcpuid_msr_dynamic(&fd); break;
#endif /* HAS_LIBCPUID */
#if HAS_DMIDECODE
			case DMIDECODE:            __call_dmidecode(&fd);            break;
#endif /* HAS_DMIDECODE */
#if HAS_LIBPCI
			case ACCESS_DEV_PCI:       __find_devices(&fd);              break;
#endif /* HAS_LIBPCI */
			case ACCESS_SYS_DEBUG:     __can_access_sys_debug_dri(&fd);  break;
			case LOAD_MODULE:          __load_module(&fd);               break;
			default: MSG_WARNING(_("request_handler: case %i not handled"), cmd);
		}
	}

	cleanup_thread(fd);
}

static void sighandler([[maybe_unused]] int __signum)
{
	quit_loop = true;
}

int main([[maybe_unused]] int argc, char *argv[])
{
	bool background = true;
	int listen_socket, data_socket, ret, err = EXIT_SUCCESS;
	pid_t pid;
	std::string error_str = "unknown";
	struct sockaddr_un name;
	struct pollfd fds[NFDS];

	if(!IS_ROOT)
	{
		MSG_ERROR("%s cannot be executed as regular user.", argv[0]);
		return 1;
	}

	/* Check if daemon must run in foreground */
	if(std::getenv("CPUX_DAEMON_BG"))
		background = ((std::atoi(std::getenv("CPUX_DAEMON_BG"))) > 0);
	if(std::getenv("CPUX_DAEMON_DEBUG"))
		if((std::atoi(std::getenv("CPUX_DAEMON_DEBUG"))) > 0)
			Logger::set_verbosity(LOG_DEBUG);

	/* Pre-initialization */
	umask(0);
	fs::remove(SOCKET_NAME);
	std::signal(SIGINT,  sighandler);
	std::signal(SIGTERM, sighandler);

	/* Logs */
	if(background)
	{
		std::freopen(LOG_FILE, "w", stdout);
		std::setvbuf(stdout, NULL, _IONBF, 0);
		dup2(STDOUT_FILENO, STDERR_FILENO);
	}
	PRGINFO(stdout);

	/* Create local socket. */
	if((listen_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
		GOTO_ERROR("socket");

	/* Bind socket to socket name. */
	memset(&name, 0, sizeof(struct sockaddr_un));
	name.sun_family = AF_UNIX;
	std::strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
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
	if(background)
	{
		pid = fork();
		if(pid > 0)
			return 0;
		else if(pid < 0)
			GOTO_ERROR("fork");
	}

	/* This is the main loop for handling connections */
	while(!quit_loop)
	{
		/* Wait for incoming connection */
		ret = poll(fds, NFDS, background ? POLL_TIMEOUT : -1);
		if(ret < 0)
			MSG_ERRNO("%s", "poll");
		else if(ret == 0) // Timeout
			quit_loop = ti.running_threads == 0;
		else if(fds[0].revents & POLLIN)
		{
			if((data_socket = accept(listen_socket, NULL, NULL)) < 0)
				MSG_ERRNO("%s", "accept");
			else
			{
				MSG_DEBUG("%s: fd=%i, %u clients", __func__, data_socket, ti.running_threads + 1);
				ti.mutex.lock();
				std::thread thread(request_handler, data_socket);
				thread.detach();
				ti.running_threads++;
				ti.threads.push_back(std::move(thread));
				ti.mutex.unlock();
			}
		}
	}
	goto clean;

error:
	MSG_ERRNO("%s", error_str.c_str());
	err = EXIT_FAILURE;

clean:
	std::fclose(stdout);
	fs::remove(SOCKET_NAME);
	close(listen_socket);

	if(std::getenv("APPDIR") != NULL)
		fs::remove(DAEMON_TMP_EXEC);

	return err;
}
