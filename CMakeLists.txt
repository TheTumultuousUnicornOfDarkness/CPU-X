cmake_minimum_required(VERSION 3.12)
# https://cmake.org/cmake/help/latest/policy/CMP0177.html
if(POLICY CMP0177)
	cmake_policy(SET CMP0177 NEW)
endif()
project(cpu-x
	VERSION "5.4.0"
	LANGUAGES C CXX
)

# Check for x86 CPU
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$|(x86)|(X86)|(amd64)|(AMD64)")
	set(CPU_ISA_X86 TRUE)
else()
	set(CPU_ISA_X86 FALSE)
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$|(x86)|(X86)|(amd64)|(AMD64)")

# Check for ARM CPU
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^armv.*|aarch64")
	set(CPU_ISA_ARM TRUE)
else()
	set(CPU_ISA_ARM FALSE)
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "^armv.*|aarch64")

# Check if platform is supported
if(NOT CMAKE_SYSTEM_NAME MATCHES "Linux|FreeBSD")
	message(WARNING "'${CMAKE_SYSTEM_NAME}' system is not officially supported by CPU-X, some features will not be available.")
endif(NOT CMAKE_SYSTEM_NAME MATCHES "Linux|FreeBSD")
if(NOT CPU_ISA_X86 AND NOT CPU_ISA_ARM)
	message(WARNING "'${CMAKE_SYSTEM_PROCESSOR}' processor is not officially supported by CPU-X, some features will not be available.")
endif(NOT CPU_ISA_X86 AND NOT CPU_ISA_ARM)


### DEFAULT CONFIG

# Global variables
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
# Avoid to put files under /usr/libexec or /usr/local/libexec on Linux when the path does not exist
# On FreeBSD, /usr/local/libexec is missing under Poudriere (https://gitlab.com/FreeBSD/freebsd-ports/-/commit/eb6c9c8c4aa5981078c6b9b69527ecdaee5f22c9)
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT EXISTS ${CMAKE_INSTALL_FULL_LIBEXECDIR})
	set(CMAKE_INSTALL_LIBEXECDIR      lib/${CMAKE_PROJECT_NAME})
	set(CMAKE_INSTALL_FULL_LIBEXECDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR})
endif(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT EXISTS ${CMAKE_INSTALL_FULL_LIBEXECDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib/)
set(CMAKE_DATA_OUTPUT_DIRECTORY    ${CMAKE_BINARY_DIR}/output/data/)
set(CPU_X_DATA_DIRECTORY           ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
set(APP_EXEC                       ${CMAKE_PROJECT_NAME})
set(DAEMON_EXEC                    ${CMAKE_PROJECT_NAME}-daemon)
set(DAEMON_PATH                    ${CMAKE_INSTALL_FULL_LIBEXECDIR}/${DAEMON_EXEC})
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(DAEMON_SOCKET              "/run/cpu-x.sock")
else()
	set(DAEMON_SOCKET              "/var/run/cpu-x.sock")
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} -Wno-unused-result")
set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -Wpedantic -Wno-unused-result -Wno-psabi")
set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -Wuninitialized -Wformat -Wformat-security -Wstrict-prototypes")
set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wuninitialized -Wformat -Wformat-security")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")

include(CheckCCompilerFlag)
check_c_compiler_flag("-no-pie" HAS_NO_PIE)
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND HAS_NO_PIE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif(CMAKE_SYSTEM_NAME MATCHES "Linux" AND HAS_NO_PIE)

# Options
option(WITH_GTK           "Build support for GUI in GTK3+"                                    ON)
option(WITH_NCURSES       "Build support for TUI in NCurses"                                  ON)
option(WITH_GETTEXT       "Build support for internationalization"                            ON)
option(WITH_LIBCPUID      "Use Libcpuid library"                                              ON)
option(WITH_LIBPCI        "Use Libpci library"                                                ON)
option(WITH_LIBEGL        "Use Libegl library"                                                ON)
option(WITH_VULKAN        "Use Vulkan library"                                                ON)
option(WITH_OPENCL        "Use OpenCL library"                                                ON)
option(WITH_LIBPROCPS     "Use Libprocps library"                                             ON)
option(WITH_LIBSTATGRAB   "Use Libstatgrab library"                                           ON)
option(WITH_DMIDECODE     "Built-in Dmidecode"                                                ON)
if(CPU_ISA_X86)
	option(WITH_BANDWIDTH "Built-in Bandwidth"                                                ON) # Bandwidth is only supported on x86 CPUs (requires cache size)
else()
	set(WITH_BANDWIDTH OFF)
endif(CPU_ISA_X86)
option(FORCE_LIBSTATGRAB  "Force use of Libstatgrab instead of Libprocps (GNU/Linux system)"  OFF)
option(APPIMAGE           "Enable workarounds for AppImage"                                   OFF)
option(FLATPAK            "Enable workarounds for Flatpak"                                    OFF)

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


### FUNCTIONS

# Credits to jtanx: https://gist.github.com/jtanx/96ded5e050d5ee5b19804195ee5cf5f9
# Function: pad_string(output str padchar length)
# Add padding to a string
# @output:  variable name for output string
# @str:     input string
# @padchar: character used for padding
# @length:  padding length
function(pad_string output str padchar length)
	string(LENGTH "${str}" _strlen)
	math(EXPR _strlen "${length} - ${_strlen}")
	if(_strlen GREATER 0)
		if(${CMAKE_VERSION} VERSION_LESS "3.14")
			unset(_pad)
			foreach(_i RANGE 1 ${_strlen}) # inclusive
				string(APPEND _pad ${padchar})
			endforeach()
		else()
			string(REPEAT ${padchar} ${_strlen} _pad)
		endif()
		string(APPEND str ${_pad})
	endif()
	set(${output} "${str}" PARENT_SCOPE)
endfunction()


### MACROS

# Macro: print_config(libname libver withstate)
# Print the state of CPU-X features: in bold green if enabled, else bold red.
# @libname:    library name (e.g. GTK)
# @libver:     library version (e.g. XXXXX_VERSION)
# @withstate:  the CMake option which allows to disable this feature (e.g. WITH_XXXXX)
macro(print_config libname libver withstate)
	pad_string(lib_string ${libname} " " 15)
	if(${withstate})
		message("${BoldCyan}**   ${lib_string}${ColourReset}\tsupport is ${BoldGreen}enabled${ColourReset} (version is ${libver})")
		add_definitions(-DHAS_${libname}=1 -D${libname}_VERSION="${libver}")
	else(${withstate})
		message("${BoldCyan}**   ${lib_string}${ColourReset}\tsupport is ${BoldRed}disabled${ColourReset}")
		add_definitions(-DHAS_${libname}=0 -D${libname}_VERSION=NULL)
	endif(${withstate})
endmacro(print_config)


### HELPERS / SUBDIRECTORIES

set(GIT_REVISION "")
if(DEFINED ENV{GITHUB_SHA})
	set(GIT_REVISION "+git-$ENV{GITHUB_SHA}")
elseif(EXISTS ${CMAKE_SOURCE_DIR}/.git)
	find_package(Git)
	if(GIT_FOUND)
		execute_process(COMMAND sh -c "${GIT_EXECUTABLE} describe --long --tags --exclude continuous --abbrev=40 --dirty | cut --delimiter=- --fields=2-"
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
			OUTPUT_VARIABLE GIT_DESCRIBE_REVISION
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(GIT_DESCRIBE_REVISION STREQUAL "")
			message(WARNING "Unknown Git revision.")
		else(GIT_DESCRIBE_REVISION STREQUAL "")
			set(GIT_REVISION "+git-r${GIT_DESCRIBE_REVISION}")
		endif(GIT_DESCRIBE_REVISION STREQUAL "")
	else(GIT_FOUND)
		message(WARNING "Git package not found, GIT_REVISION cannot be set.")
	endif(GIT_FOUND)
endif()

enable_testing()
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Backtrace REQUIRED)
find_package(Filesystem REQUIRED)
if(WITH_GETTEXT)
	find_package(Gettext REQUIRED)
	if(${CMAKE_VERSION} VERSION_GREATER "3.1")
		find_package(Intl REQUIRED)
	endif(${CMAKE_VERSION} VERSION_GREATER "3.1")
endif(WITH_GETTEXT)
#include(CheckSymbolExists)
#include(CheckStructHasMember)
#include(CheckCSourceCompiles)
include(CheckCSourceRuns)
#include(CheckIncludeFile)
include(CheckLanguage)
add_subdirectory(data)
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(tests)


### UNINSTALL TARGET

configure_file("${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY
)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
