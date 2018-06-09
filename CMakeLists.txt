cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
project(cpu-x
	VERSION "3.2.2"
	LANGUAGES C
)


### DEFAULT CONFIG

# Global variables
if(PORTABLE_BINARY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else(PORTABLE_BINARY)
	include(GNUInstallDirs)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin/)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib/)
	set(CMAKE_DATA_OUTPUT_DIRECTORY    ${CMAKE_BINARY_DIR}/output/data/)
	set(CPU_X_DATA_DIRECTORY           ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
endif(PORTABLE_BINARY)
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "i.86")
	set(BITNESS "32")
else(${CMAKE_SYSTEM_PROCESSOR} MATCHES "i.86")
	set(BITNESS "64")
endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "i.86")
set(CPU_X_FLAGS           "-Wno-unused-parameter -Wno-unused-result -Wno-format-security")
set(CPU_X_DEBUG_FLAGS     "-Wall -Wextra -Wuninitialized -Wstrict-prototypes ${CPU_X_FLAGS}")
set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wno-deprecated-declarations ${CPU_X_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${CPU_X_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CPU_X_DEBUG_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")

# Options
option(WITH_GTK           "Build support for GUI in GTK3+"                                    ON)
option(WITH_NCURSES       "Build support for TUI in NCurses"                                  ON)
option(WITH_GETTEXT       "Build support for internationalization"                            ON)
option(WITH_LIBCURL       "Use Libcurl library"                                               ON)
option(WITH_LIBCPUID      "Use Libcpuid library"                                              ON)
option(WITH_LIBPCI        "Use Libpci library"                                                ON)
option(WITH_LIBPROCPS     "Use Libprocps library"                                             ON)
option(WITH_LIBSTATGRAB   "Use Libstatgrab library"                                           ON)
option(WITH_DMIDECODE     "Built-in Dmidecode"                                                ON)
option(WITH_BANDWIDTH     "Built-in Bandwidth"                                                ON)
option(FORCE_LIBSTATGRAB  "Force use of Libstatgrab instead of Libprocps (GNU/Linux system)"  OFF)
option(PORTABLE_BINARY    "Build a all-in-one binary (portable version)"                      OFF)
option(IGNORE_STATICLIBS  "Build portable binary without static libraries"                    OFF)

# Colours
string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(ColourBold  "${Esc}[1m")
set(Red         "${Esc}[31m")
set(Green       "${Esc}[32m")
set(Yellow      "${Esc}[33m")
set(Blue        "${Esc}[34m")
set(Magenta     "${Esc}[35m")
set(Cyan        "${Esc}[36m")
set(White       "${Esc}[37m")
set(BoldRed     "${Esc}[1;31m")
set(BoldGreen   "${Esc}[1;32m")
set(BoldYellow  "${Esc}[1;33m")
set(BoldBlue    "${Esc}[1;34m")
set(BoldMagenta "${Esc}[1;35m")
set(BoldCyan    "${Esc}[1;36m")
set(BoldWhite   "${Esc}[1;37m")


### MACROS

# Macro: print_config(libname foundstate withstate)
# Print the state of CPU-X features: in bold green if enabled, else bold red.
# @libname:    library name (e.g. GTK)
# @libver:     library version (e.g. XXXXX_VERSION)
# @foundstate: the result of pkg_check_modules(...) (e.g. XXXXX_FOUND)
# @withstate:  the CMake option which allows to disable this feature (e.g. WITH_XXXXX)
macro(print_config libname libver foundstate withstate)
	if(${foundstate} AND ${withstate})
		message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldGreen}enabled${ColourReset}")
		add_definitions(-DHAS_${libname}=1 -D${libname}_VERSION="${libver}")
	else(${foundstate} AND ${withstate})
		if(NOT ${withstate})
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldRed}disabled${ColourReset} (explicitly disabled)")
		elseif(NOT ${foundstate})
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldRed}disabled${ColourReset} (not found by pkg-config)")
		endif()
		add_definitions(-DHAS_${libname}=0 -D${libname}_VERSION=NULL)
	endif(${foundstate} AND ${withstate})
endmacro(print_config)

# Macro: add_embedded_library(lib)
# Include a library (satic or not) for building, needed for portable version.
# @LIBRARIES: variable name where to put libraries name
# @lib:	      library to add
macro(add_embedded_library LIBRARIES lib)
	if(PORTABLE_BINARY AND NOT IGNORE_STATICLIBS)
		set(${LIBRARIES} ${lib})
	endif(PORTABLE_BINARY AND NOT IGNORE_STATICLIBS)
endmacro(add_embedded_library)

# Macro: add_embedded_library_linux(linux_lib)
# Same as add_embedded_library macro, but only if target is Linux
# @LIBRARIES: variable name where to put libraries name
# @linux_lib: Linux library to add
macro(add_embedded_library_linux LIBRARIES linux_lib)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY AND NOT IGNORE_STATICLIBS)
		set(${LIBRARIES} ${linux_lib})
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY AND NOT IGNORE_STATICLIBS)
endmacro(add_embedded_library_linux)

# Macro: add_embedded_library_others(others_lib)
# Same as add_embedded_library macro, but only if target is NOT Linux
# @LIBRARIES:  variable name where to put libraries name
# @others_lib: NON-Linux library name
macro(add_embedded_library_others LIBRARIES others_lib)
	if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY AND NOT IGNORE_STATICLIBS)
		set(${LIBRARIES} ${others_lib})
	endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND PORTABLE_BINARY AND NOT IGNORE_STATICLIBS)
endmacro(add_embedded_library_others)


### HELPERS / SUBDIRECTORIES

find_package(PkgConfig REQUIRED)
find_package(Gettext)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.2")
	find_package(Intl)
endif(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.2")
find_package(Threads)
find_package(Backtrace)
include(CheckSymbolExists)
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
