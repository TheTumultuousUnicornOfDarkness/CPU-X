cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)
if(POLICY CMP0075)
	cmake_policy(SET CMP0075 NEW)
endif()
project(cpu-x
	VERSION "4.5.3"
	LANGUAGES C CXX
)


### DEFAULT CONFIG

# Global variables
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
if(NOT EXISTS ${CMAKE_INSTALL_FULL_LIBEXECDIR})
	set(CMAKE_INSTALL_LIBEXECDIR      lib/${CMAKE_PROJECT_NAME})
	set(CMAKE_INSTALL_FULL_LIBEXECDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR})
endif(NOT EXISTS ${CMAKE_INSTALL_FULL_LIBEXECDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib/)
set(CMAKE_DATA_OUTPUT_DIRECTORY    ${CMAKE_BINARY_DIR}/output/data/)
set(CPU_X_DATA_DIRECTORY           ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
set(APP_EXEC                       ${CMAKE_PROJECT_NAME})
set(DAEMON_EXEC                    ${CMAKE_PROJECT_NAME}-daemon)
set(DAEMON_PATH                    ${CMAKE_INSTALL_FULL_LIBEXECDIR}/${DAEMON_EXEC})
set(DAEMON_SOCKET                  ${CMAKE_INSTALL_FULL_RUNSTATEDIR}/cpu-x.sock)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(BITNESS "32")
else(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(BITNESS "64")
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-unused-result")
set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -Wpedantic -Wno-deprecated-declarations -Wno-unused-result")
set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -Wuninitialized -Wformat -Wformat-security -Wstrict-prototypes")
set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wuninitialized -Wformat -Wformat-security")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")

include(CheckCCompilerFlag)
check_c_compiler_flag("-no-pie" HAS_NO_PIE)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND HAS_NO_PIE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND HAS_NO_PIE)

# Options
option(WITH_GTK           "Build support for GUI in GTK3+"                                    ON)
option(WITH_NCURSES       "Build support for TUI in NCurses"                                  ON)
option(WITH_GETTEXT       "Build support for internationalization"                            ON)
option(WITH_LIBCPUID      "Use Libcpuid library"                                              ON)
option(WITH_LIBPCI        "Use Libpci library"                                                ON)
option(WITH_LIBGLFW       "Use Libglfw library"                                               ON)
option(WITH_VULKAN        "Use Vulkan library"                                                ON)
option(WITH_OPENCL        "Use OpenCL library"                                                OFF) # Disabled due to issues (#238, #258, #264)
option(WITH_LIBPROCPS     "Use Libprocps library"                                             ON)
option(WITH_LIBSTATGRAB   "Use Libstatgrab library"                                           ON)
option(WITH_DMIDECODE     "Built-in Dmidecode"                                                ON)
option(WITH_BANDWIDTH     "Built-in Bandwidth"                                                ON)
option(FORCE_LIBSTATGRAB  "Force use of Libstatgrab instead of Libprocps (GNU/Linux system)"  OFF)

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


### HELPERS / SUBDIRECTORIES

set(GIT_REVISION "")
if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
	find_package(Git)
	if(GIT_FOUND)
		execute_process(COMMAND sh -c "${GIT_EXECUTABLE} describe --long --tags --exclude continuous --abbrev=40 --dirty | cut --delimiter=- --fields=2-"
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
			OUTPUT_VARIABLE GIT_DESCRIBE_REVISION
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		if(NOT ${GIT_DESCRIBE_REVISION} STREQUAL "")
			set(GIT_REVISION "+git-r${GIT_DESCRIBE_REVISION}")
		endif(NOT ${GIT_DESCRIBE_REVISION} STREQUAL "")
	endif(GIT_FOUND)
endif(EXISTS ${CMAKE_SOURCE_DIR}/.git)

enable_testing()
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Backtrace REQUIRED)
find_package(Filesystem REQUIRED)
if(WITH_GETTEXT)
	find_package(Gettext)
	if(${CMAKE_VERSION} VERSION_GREATER "3.1")
		find_package(Intl)
	endif(${CMAKE_VERSION} VERSION_GREATER "3.1")
endif(WITH_GETTEXT)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckIncludeFile)
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
