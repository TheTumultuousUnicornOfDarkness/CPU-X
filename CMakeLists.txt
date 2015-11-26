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
	option(FORCE_LIBSTATGRAB "Force use of library Libstatgrab"	OFF)
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

# Macros
# This macro print the state of CPU-X features: in bold green if enabled, else bold red.
#   @libname: the name of the library (e.g. GTK)
#   @foundstate: the result of pkg_check_modules(...) (e.g. XXXXX_FOUND)
#   @withstate: the CMake option that allow to disable some features (e.g. WITH_XXXXX)
macro(print_config libname foundstate withstate)
	if(${foundstate} AND ${withstate})
		if(${foundstate} EQUAL 2)
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldGreen}enable${ColourReset} (imported source code)")
		else(${foundstate} EQUAL 2)
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldGreen}enable${ColourReset}")
		endif(${foundstate} EQUAL 2)
		add_definitions(-DHAS_${libname}=1)
	else(${foundstate} AND ${withstate})
		if(NOT ${withstate})
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldRed}disable${ColourReset} (explicitly disable)")
		elseif(NOT ${foundstate})
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldRed}disable${ColourReset} (not found by pkg-config)")
		endif()
		add_definitions(-DHAS_${libname}=0)
	endif(${foundstate} AND ${withstate})
endmacro(print_config)

# This macro add a (static) library to build, useful for portable version.
#   @lib: the name of the library to add
macro(add_embedded_library lib)
	if(PORTABLE_BINARY OR EMBED)
		set(EMBEDDED_LIBRARIES ${EMBEDDED_LIBRARIES} ${lib})
	endif(PORTABLE_BINARY OR EMBED)
endmacro(add_embedded_library)

# This macro calls add_embedded_library macro if target is Linux.
#   @linux_lib: the name of the library to add (Linux OS)
macro(add_embedded_library_linux linux_lib)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		add_embedded_library(${linux_lib})
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endmacro(add_embedded_library_linux)

# This macro calls add_embedded_library macro if target is NOT Linux.
#   @others_lib: the name of the library to add (NON-Linux OS)
macro(add_embedded_library_others others_lib)
	if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		add_embedded_library(${others_lib})
	endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endmacro(add_embedded_library_others)


# Subdirectories
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)


# Embed compilation
if(EMBED)
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
