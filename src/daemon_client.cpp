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
* FILE daemon_client.cpp
*/

#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.hpp"
#include "daemon.h"
#include "daemon_client.hpp"

namespace fs = std::filesystem;


static char *get_daemon_socket()
{
	static char *socket = NULL;

	if(socket == NULL)
	{
#ifdef FLATPAK
		asprintf(&socket, "%s/app/%s/%s", std::getenv("XDG_RUNTIME_DIR"), std::getenv("FLATPAK_ID"), SOCKET_NAME);
#else
		asprintf(&socket, "%s", SOCKET_NAME);
#endif /* FLATPAK */
	}
	MSG_DEBUG("get_daemon_socket: socket_path=%s", socket);

	return socket;
}

/* Start daemon in background */
const char *start_daemon(bool graphical)
{
	int wstatus = -1;
	pid_t pid;
	char const *msg = NULL;
	char const *daemon_args = get_daemon_socket();

	MSG_VERBOSE("%s", _("Starting daemon in background…"));
#ifdef FLATPAK
	std::string line, app_path;
	std::ifstream stream("/.flatpak-info");
	std::regex regex("^app-path=(.*?)$");
	std::smatch match;

	while(std::getline(stream, line))
	{
		if(std::regex_search(line, match, regex))
		{
			app_path = match[1].str() + "/lib/" + PRGNAME_LOW + "/" + DAEMON_EXEC;
			break;
		}
	}

	char const *daemon_exec = app_path.c_str();
#else
	char const *appdir      = std::getenv("APPDIR");
	char const *daemon_exec = (appdir == NULL) ? DAEMON_PATH : DAEMON_TMP_EXEC;

	/* Hack to allow pkexec to run daemon (when running from AppImage) */
	if(appdir != NULL)
	{
		const std::string appdir_daemon_path = std::string(appdir) + std::string(DAEMON_PATH);
		MSG_DEBUG("start_daemon: copy '%s' to '%s'", appdir_daemon_path.c_str(), daemon_exec);
		fs::copy(appdir_daemon_path, daemon_exec, fs::copy_options::overwrite_existing);
	}
#endif /* FLATPAK */

	pid = fork();
	if(pid < 0)
		MSG_ERRNO("%s", "fork");
	else if(pid == 0)
	{
		if(graphical)
		{
			const char* const args[] =
			{
#ifdef FLATPAK
				"flatpak-spawn",
				"--host",
#endif /* FLATPAK */
				"pkexec",
				"--disable-internal-agent",
				daemon_exec,
				daemon_args,
				nullptr
			};
			execvp_cpp(args[0], args);
		}
		else if(IS_ROOT)
		{
			const char* const args[] =
			{
#ifdef FLATPAK
				"flatpak-spawn",
				"--host",
#endif /* FLATPAK */
				daemon_exec,
				daemon_args,
				nullptr
			};
			execvp_cpp(args[0], args);
		}
		else
		{
			const char* const args[] =
			{
#ifdef FLATPAK
				"flatpak-spawn",
				"--host",
#endif /* FLATPAK */
				"pkexec",
				daemon_exec,
				daemon_args,
				nullptr
			};
			execvp_cpp(args[0], args);
		}
	}
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

/* Check if daemon is running */
bool daemon_is_alive()
{
	const fs::file_status status = fs::status(get_daemon_socket());
	const fs::perms perms = status.permissions();
	const bool is_socket = fs::is_socket(status);
	const bool perms_all = (perms == fs::perms::all);
	const bool is_alive  = is_socket && perms_all;

	MSG_DEBUG("daemon_is_alive: is_socket=%i, perms_all=%i ==> ret=%i", is_socket, perms_all, is_alive);

	return is_alive;
}

/* Establish connection to daemon */
int connect_to_daemon(int &socket_fd)
{
	std::string error_str;
	struct sockaddr_un addr;

	MSG_VERBOSE("%s", _("Connecting to daemon…"));
	/* Create local socket */
	if((socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
		GOTO_ERROR("socket");

	/* Connect socket to socket address */
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, get_daemon_socket(), sizeof(addr.sun_path) - 1);
	if(connect(socket_fd, (const struct sockaddr*) &addr, sizeof(struct sockaddr_un)) < 0)
		GOTO_ERROR("connect");

	return 0;

error:
	MSG_ERRNO("failed to connect to daemon: %s", error_str.c_str());
	if(socket_fd >= 0) close(socket_fd);
	return 1;
}
