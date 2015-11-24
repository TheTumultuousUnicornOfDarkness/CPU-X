cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
	set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/X11/lib/pkgconfig/")
endif(APPLE)

cmake_policy(SET CMP0048 NEW)
project(cpu-x
	VERSION 2.2.0
	LANGUAGES C
)

# CMake default config
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/accomplished/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/accomplished/lib/)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}		-Wall -Wextra -Wuninitialized -Wstrict-prototypes -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}	-Wall -Wextra -Wuninitialized -Wstrict-prototypes -Wno-unused-parameter")
set(WITH_GETTEXT ON)
set(LIBBDWT_FOUND OFF)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)


# Options
option(WITH_GTK		"Add support for GUI in GTK3+"			ON)
option(WITH_NCURSES	"Add support for TUI in NCurses"		ON)
option(WITH_LIBCPUID	"Allow use of library Libcpuid"			ON)
option(WITH_LIBDMI	"Allow use of library Libdmi"			ON)
option(WITH_LIBBDWT	"Allow use of library Libwandwidth"		ON)
option(WITH_LIBPCI	"Allow use of library Libpci"			ON)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	option(WITH_LIBSYSTEM	"Allow use of library Libprocps"	ON)
else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	option(WITH_LIBSYSTEM	"Allow use of library Libstatgrab"	ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


# Colours
if(NOT WIN32)
	string(ASCII 27 Esc)
	set(ColourReset "${Esc}[m")
	set(ColourBold	"${Esc}[1m")
	set(Red		"${Esc}[31m")
	set(Green	"${Esc}[32m")
	set(Yellow	"${Esc}[33m")
	set(Blue	"${Esc}[34m")
	set(Magenta	"${Esc}[35m")
	set(Cyan	"${Esc}[36m")
	set(White	"${Esc}[37m")
	set(BoldRed	"${Esc}[1;31m")
	set(BoldGreen	"${Esc}[1;32m")
	set(BoldYellow	"${Esc}[1;33m")
	set(BoldBlue	"${Esc}[1;34m")
	set(BoldMagenta "${Esc}[1;35m")
	set(BoldCyan	"${Esc}[1;36m")
	set(BoldWhite   "${Esc}[1;37m")
endif(NOT WIN32)


# Subdirectories
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)


# Embed compilation
if(EMBED)
	message("${BoldBlue}${CMAKE_PROJECT_NAME} will be compiled as portable binary.${ColourReset}")
	if(WITH_GTK)
		message("${Green}Converting files to embed${ColourReset}")
		execute_process(COMMAND glib-compile-resources --target=${CMAKE_CURRENT_LIST_DIR}/src/gtk-resources.c --generate-source cpu-x.gresource.xml
				COMMAND	glib-compile-resources --target=${CMAKE_CURRENT_LIST_DIR}/src/gtk-resources.h --generate-header cpu-x.gresource.xml
			WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data
		)
	endif(WITH_GTK)
	add_definitions(-DEMBED)
endif(EMBED)


# Uninstall target
set(CPUX_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY
)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
