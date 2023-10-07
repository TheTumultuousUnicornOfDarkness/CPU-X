### FIND NEEDED LIBRAIRIES

# GTK3+
if(WITH_GTK)
	set(MINIMUM_GTK_VERSION 3.12.0)
	pkg_check_modules(GTK3 gtkmm-3.0>=${MINIMUM_GTK_VERSION})
	if(GTK3_FOUND)
		include_directories(${GTK3_INCLUDE_DIRS})
		link_directories(${GTK3_LIBRARY_DIRS})
		add_definitions(${GTK3_CFLAGS_OTHER})
		add_library(gui_gtk STATIC
			gui_gtk.cpp
			gui_gtk.hpp
			${GRESOURCES}
		)
		set(GTK3_LIBRARIES "gui_gtk" ${GTK3_LIBRARIES})
	else(GTK3_FOUND)
		message("${BoldYellow}GTK3+ GUI will not be build:${ColourReset} ${Yellow}GTK ${MINIMUM_GTK_VERSION} or higher is needed.${ColourReset}")
		message("${Yellow}Set ${BoldYellow}PKG_CONFIG_PATH${ColourReset}${Yellow} environment variable if development files are installed.${ColourReset}")
	endif(GTK3_FOUND)
endif(WITH_GTK)

# NCurses
if(WITH_NCURSES)
	# Assume ncurses from the base on FreeBSD
	if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
		set(NCURSES_FOUND 1)
		set(NCURSES_LIBRARIES "-l:libncursesw.so")
	else()
		pkg_check_modules(NCURSES ncursesw)
	endif()
	if(NCURSES_FOUND)
		include_directories(${NCURSES_INCLUDE_DIRS})
		link_directories(${NCURSES_LIBRARY_DIRS})
		add_library(tui_ncurses STATIC
			tui_ncurses.cpp
			tui_ncurses.hpp
		)
		set(NCURSES_LIBRARIES "tui_ncurses" ${NCURSES_LIBRARIES})
	endif(NCURSES_FOUND)
endif(WITH_NCURSES)

# Gettext
if(Intl_FOUND)
	include_directories(${Intl_INCLUDE_DIRS})
endif(Intl_FOUND)

# Libcpuid
if(WITH_LIBCPUID)
	pkg_check_modules(LIBCPUID libcpuid>=0.6.0)
	if(LIBCPUID_FOUND)
		include_directories(${LIBCPUID_INCLUDE_DIRS})
		link_directories(${LIBCPUID_LIBRARY_DIRS})
		add_definitions(${LIBCPUID_CFLAGS_OTHER})
		set(CMAKE_REQUIRED_LIBRARIES ${LIBCPUID_LIBRARIES})
	endif(LIBCPUID_FOUND)
endif(WITH_LIBCPUID)

# Libpci
if(WITH_LIBPCI)
	pkg_check_modules(LIBPCI libpci)
	if(LIBPCI_FOUND)
		include_directories(${LIBPCI_INCLUDE_DIRS})
		link_directories(${LIBPCI_LIBRARY_DIRS})
		add_definitions(${LIBPCI_CFLAGS_OTHER})
	endif(LIBPCI_FOUND)
endif(WITH_LIBPCI)

# Libglfw
if(WITH_LIBGLFW)
	pkg_check_modules(LIBGL gl)
	pkg_check_modules(LIBGLFW glfw3>=3.3)
	if(LIBGL_FOUND AND LIBGLFW_FOUND)
		include_directories(${LIBGL_INCLUDE_DIRS} ${LIBGLFW_INCLUDE_DIRS})
		link_directories(${LIBGL_LIBRARY_DIRS} ${LIBGLFW_LIBRARY_DIRS})
		add_definitions(${LIBGL_CFLAGS_OTHER} ${LIBGLFW_CFLAGS_OTHER})
	endif(LIBGL_FOUND AND LIBGLFW_FOUND)

endif(WITH_LIBGLFW)

# Vulkan
if(WITH_VULKAN)
	pkg_check_modules(VULKAN vulkan)
	if(VULKAN_FOUND)
		include_directories(${VULKAN_INCLUDE_DIRS})
		link_directories(${VULKAN_LIBRARY_DIRS})
		add_definitions(${VULKAN_CFLAGS_OTHER})
	endif(VULKAN_FOUND)
endif(WITH_VULKAN)

# OpenCL
if(WITH_OPENCL)
	pkg_check_modules(OPENCL OpenCL)
	if(OPENCL_FOUND)
		include_directories(${LIBOPENCL_INCLUDE_DIRS} ${OpenCL_INCLUDE_DIRS})
		link_directories(${LIBOPENCL_LIBRARY_DIRS} ${OpenCL_LIBRARY_DIRS})
		add_definitions(${LIBOPENCL_CFLAGS_OTHER} ${OpenCL_CFLAGS_OTHER})
	endif(OPENCL_FOUND)
endif(WITH_OPENCL)

# Libprocps
set(LIBSYSTEM 0)
set(LIBPROC2 -1)
if(WITH_LIBPROCPS AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)
	pkg_check_modules(LIBPROC2 libproc2)
	if(LIBPROC2_FOUND)
		include_directories(${LIBPROC2_INCLUDE_DIRS})
		link_directories(${LIBPROC2_LIBRARY_DIRS})
		add_definitions(${LIBPROC2_CFLAGS_OTHER})
		set(LIBSYSTEM 1)
		set(LIBPROC2 1)
	else()
		pkg_check_modules(LIBPROCPS libprocps)
		set(LIBPROC2 0)
	endif()
	if(LIBPROCPS_FOUND)
		include_directories(${LIBPROCPS_INCLUDE_DIRS})
		link_directories(${LIBPROCPS_LIBRARY_DIRS})
		add_definitions(${LIBPROCPS_CFLAGS_OTHER})
		set(LIBSYSTEM 1)
	endif(LIBPROCPS_FOUND)
endif(WITH_LIBPROCPS AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)

# Libstatgrab
if(WITH_LIBSTATGRAB AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB))
	pkg_check_modules(LIBSTATGRAB libstatgrab)
	if(LIBSTATGRAB_FOUND)
		include_directories(${LIBSTATGRAB_INCLUDE_DIRS})
		link_directories(${LIBSTATGRAB_LIBRARY_DIRS})
		add_definitions(${LIBSTATGRAB_CFLAGS_OTHER})
		set(LIBSYSTEM 2)
	endif(LIBSTATGRAB_FOUND)
endif(WITH_LIBSTATGRAB AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB))

# Dmidecode
if(WITH_DMIDECODE)
	add_subdirectory(dmidecode)
	include_directories(dmidecode)
	set(DMIDECODE_LIBRARIES "dmidecode")
endif(WITH_DMIDECODE)

# Bandwidth
if(WITH_BANDWIDTH)
	add_subdirectory(bandwidth)
	if(BANDWIDTH_FOUND)
		include_directories(bandwidth)
		set(BANDWIDTH_LIBRARIES "bandwidth")
	endif(BANDWIDTH_FOUND)
endif(WITH_BANDWIDTH)


### FINAL CONFIG

message("${BoldCyan}** ${CMAKE_PROJECT_NAME} ${PROJECT_VERSION} configuration **${ColourReset}")

# UI libraries
print_config("GTK"                 "${GTK3_VERSION}"        GTK3_FOUND        WITH_GTK)
print_config("NCURSES"             "${NCURSES_VERSION}"     NCURSES_FOUND     WITH_NCURSES)

# Other libraries
print_config("GETTEXT"             0                        GETTEXT_FOUND     WITH_GETTEXT)
print_config("LIBCPUID"            "${LIBCPUID_VERSION}"    LIBCPUID_FOUND    WITH_LIBCPUID)
print_config("LIBPCI"              "${LIBPCI_VERSION}"      LIBPCI_FOUND      WITH_LIBPCI)
print_config("LIBGLFW"             "${LIBGLFW_VERSION}"     LIBGLFW_FOUND     WITH_LIBGLFW)
print_config("Vulkan"              "${VULKAN_VERSION}"      VULKAN_FOUND      WITH_VULKAN)
print_config("OpenCL"              "${OPENCL_VERSION}"      OPENCL_FOUND      WITH_OPENCL)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)
	if(${LIBPROC2} EQUAL 1)
		print_config("LIBPROC2"    "${LIBPROC2_VERSION}"    LIBPROC2_FOUND    WITH_LIBPROCPS)
	else()
		print_config("LIBPROCPS"   "${LIBPROCPS_VERSION}"   LIBPROCPS_FOUND   WITH_LIBPROCPS)
	endif()
else()
	print_config("LIBSTATGRAB" "${LIBSTATGRAB_VERSION}" LIBSTATGRAB_FOUND WITH_LIBSTATGRAB)
endif()

# Subprojects
print_config("DMIDECODE"           "${DMIDECODE_VERSION}"   1                 WITH_DMIDECODE)
print_config("BANDWIDTH"           "${BANDWIDTH_VERSION}"   BANDWIDTH_FOUND   WITH_BANDWIDTH)


### MAIN BINARY

# Executable
add_executable(${APP_EXEC}
	core.cpp
	core.hpp
	daemon_client.cpp
	daemon_client.hpp
	daemon.h
	data.cpp
	data.hpp
	logger.cpp
	logger.hpp
	main.cpp
	options.cpp
	options.hpp
	util.cpp
	util.hpp
)

add_executable(${DAEMON_EXEC}
	daemon_server.cpp
	daemon_server.hpp
	daemon.h
	logger.cpp
	logger.hpp
	options.cpp
	options.hpp
	util.cpp
	util.hpp
)

# Libsystem extra-definitions
if(${LIBSYSTEM} EQUAL 0 OR ${LIBSYSTEM} EQUAL 1) # When Libprocps is used
	add_definitions(-DHAS_LIBSTATGRAB=0 -DLIBSTATGRAB_VERSION=NULL)
	if(${LIBPROC2} EQUAL 1)
		add_definitions(-DHAS_LIBPROCPS=0 -DLIBPROCPS_VERSION=NULL)
	else(${LIBPROC2} EQUAL 0)
		add_definitions(-DHAS_LIBPROC2=0  -DLIBPROC2_VERSION=NULL)
	endif()
endif()
if(${LIBSYSTEM} EQUAL 0 OR ${LIBSYSTEM} EQUAL 2) # When Libstatgrab is used
	add_definitions(-DHAS_LIBPROC2=0  -DLIBPROC2_VERSION=NULL)
	add_definitions(-DHAS_LIBPROCPS=0 -DLIBPROCPS_VERSION=NULL)
endif()

# Various definitions
add_definitions(-DPRGVER="${PROJECT_VERSION}"
	-DGITREV="${GIT_REVISION}"
	-DSYSTEM_NAME="${CMAKE_SYSTEM_NAME}"
	-DSYSTEM_PROCESSOR="${CMAKE_SYSTEM_PROCESSOR}"
	-DC_COMPILER_ID="${CMAKE_C_COMPILER_ID}"
	-DHAS_LIBSYSTEM=${LIBSYSTEM}
	-DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}"
	-DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}"
	-DTERMINFODIR="/lib/terminfo"
	-DDAEMON_EXEC="${DAEMON_EXEC}"
	-DDAEMON_TMP_EXEC="/tmp/${DAEMON_EXEC}"
	-DDAEMON_PATH="${DAEMON_PATH}"
	-DSOCKET_NAME="${DAEMON_SOCKET}"
)

# Link libraries
target_link_libraries(${APP_EXEC}
	${Intl_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${Backtrace_LIBRARIES}
	std::filesystem

	${GTK3_LIBRARIES}
	${NCURSES_LIBRARIES}
	${LIBCPUID_LIBRARIES}
	${LIBPCI_LIBRARIES}
	${LIBGL_LIBRARIES}
	${LIBGLFW_LIBRARIES}
	${VULKAN_LIBRARIES}
	${OPENCL_LIBRARIES}
	${LIBPROC2_LIBRARIES}
	${LIBPROCPS_LIBRARIES}
	${LIBSTATGRAB_LIBRARIES}
	${DMIDECODE_LIBRARIES}
	${BANDWIDTH_LIBRARIES}
	${OTHER_LIBRARIES}
)

target_link_libraries(${DAEMON_EXEC}
	${DAEMON_FLAGS}
	${Intl_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	std::filesystem

	${LIBCPUID_LIBRARIES}
	${DMIDECODE_LIBRARIES}
)


### INSTALLATION
install(TARGETS ${APP_EXEC}    DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
install(TARGETS ${DAEMON_EXEC} DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/)
