cmake_minimum_required(VERSION 2.8)
set(LOCALVERSION "3.0.1")
if(${CMAKE_VERSION} VERSION_GREATER "2.9")
	cmake_policy(SET CMP0048 NEW)
	project(cpu-x
		VERSION ${LOCALVERSION}
		LANGUAGES C
	)
else(${CMAKE_VERSION} VERSION_GREATER "2.9")
	project(cpu-x)
	set(PROJECT_VERSION ${LOCALVERSION})
endif(${CMAKE_VERSION} VERSION_GREATER "2.9")

### DEFAULT CONFIG

# Global variables
include(GNUInstallDirs)
if(PORTABLE_BINARY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else(PORTABLE_BINARY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin/)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib/)
	set(CMAKE_DATA_OUTPUT_DIRECTORY    ${CMAKE_BINARY_DIR}/output/data/)
	set(CPU_X_DATA_DIRECTORY           ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
endif(PORTABLE_BINARY)
set(CPU_X_FLAGS           "-Wno-format-security -Wno-unused-result -Wno-unused-parameter")
set(CPU_X_DEBUG_FLAGS     "-Wall -Wextra -Wuninitialized -Wstrict-prototypes ${CPU_X_FLAGS}")
set(CMAKE_C_FLAGS         "-Wno-deprecated-declarations ${CPU_X_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${CPU_X_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CPU_X_DEBUG_FLAGS}")

# Options
option(WITH_GTK			"Add support for GUI in GTK3+"				ON)
option(WITH_NCURSES		"Add support for TUI in NCurses"			ON)
option(WITH_GETTEXT		"Add support for internationalization"			ON)
option(WITH_LIBCPUID		"Allow use of library Libcpuid"				ON)
option(WITH_DMIDECODE		"Allow use of Dmidecode (external modified software)"	ON)
option(WITH_BANDWIDTH		"Allow use of Bandwidth (external modified software)"	ON)
option(WITH_LIBPCI		"Allow use of library Libpci"				ON)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	option(WITH_LIBSYSTEM	"Allow use of library Libprocps"			ON)
	option(FORCE_LIBSTATGRAB "Force use of library Libstatgrab"			OFF)
else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	option(WITH_LIBSYSTEM	"Allow use of library Libstatgrab"			ON)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
option(PORTABLE_BINARY		"Do a all-in-one binary, useful for a portable version"	OFF)

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

### MACROS

# Macro: print_config(libname foundstate withstate)
# Print the state of CPU-X features: in bold green if enabled, else bold red.
# @libname:	the name of the library (e.g. GTK)
# @libver:	the library version
# @foundstate:	the result of pkg_check_modules(...) (e.g. XXXXX_FOUND)
# @withstate:	the CMake option that allow to disable some features (e.g. WITH_XXXXX)
macro(print_config libname libver foundstate withstate)
	if(${foundstate} AND ${withstate})
		message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldGreen}enable${ColourReset}")
		add_definitions(-DHAS_${libname}=1 -D${libname}_VERSION="${libver}")
	else(${foundstate} AND ${withstate})
		if(NOT ${withstate})
			set(cause "(explicitly disable)")
		elseif(NOT ${foundstate})
			set(cause "(not found by pkg-config)")
		endif()
		message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldRed}disable${ColourReset} ${cause}")
		add_definitions(-DHAS_${libname}=0 -D${libname}_VERSION=NULL)
	endif(${foundstate} AND ${withstate})
endmacro(print_config)

# Macro: add_embedded_library(lib)
# Include a library (satic or not) for building, needed for portable version.
# @LIBRARIES:	variable name where to put librairies name
# @lib:		the name of the library to add
macro(add_embedded_library LIBRARIES lib)
	if(PORTABLE_BINARY)
		set(${LIBRARIES} ${lib})
	endif(PORTABLE_BINARY)
endmacro(add_embedded_library)

# Macro: add_embedded_library_linux(linux_lib)
# Same as add_embedded_library macro, but only if target is Linux
# @LIBRARIES:	variable name where to put librairies name
# @linux_lib:	the name of the library to add (Linux OS)
macro(add_embedded_library_linux LIBRARIES linux_lib)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY)
		set(${LIBRARIES} ${linux_lib})
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY)
endmacro(add_embedded_library_linux)

# Macro: add_embedded_library_others(others_lib)
# Same as add_embedded_library macro, but only if target is NOT Linux
# @LIBRARIES:	variable name where to put librairies name
# @others_lib:	the name of the library to add (NON-Linux OS)
macro(add_embedded_library_others LIBRARIES others_lib)
	if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY)
		set(${LIBRARIES} ${others_lib})
	endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY)
endmacro(add_embedded_library_others)


### HELPERS / SUB-DIRECTORIES
find_package(PkgConfig REQUIRED)
find_package(Gettext)
find_package(Intl)
find_package(Threads REQUIRED)
find_package(Backtrace REQUIRED)
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)


### UNINSTALL TARGET
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY
)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
