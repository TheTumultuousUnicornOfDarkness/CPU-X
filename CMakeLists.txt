cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 NEW)
project(cpu-x
	VERSION 2.0.2
	LANGUAGES C
)

# CMake default config
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/accomplished/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/accomplished/lib/)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wuninitialized -Wstrict-prototypes")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}   -Wall -Wextra -Wuninitialized -Wstrict-prototypes")

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)


# Options
option(WITH_GTK		"Add support for GUI in GTK3+"		ON)
option(WITH_NCURSES	"Add support for TUI in NCurses"	ON)
option(WITH_LIBCPUID	"Allow use of library Libcpuid"		ON)
option(WITH_LIBDMI	"Allow use of library Libdmi"		ON)
option(WITH_LIBPROCPS	"Allow use of library Libprocps"	ON)


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
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)


# Embed compilation
if(EMBED)
	message("${BoldBlue}${CMAKE_PROJECT_NAME} will be compiled as portable binary.${ColourReset}")
	if(WITH_GTK)
		message("${Green}Converting files to embed${ColourReset}")
		execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/embed.sh
			WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		)
	endif(WITH_GTK)
	add_definitions(-DEMBED)
endif(EMBED)


### Final config
message("${BoldCyan}** ${CMAKE_PROJECT_NAME} ${PROJECT_VERSION} configuration **${ColourReset}")

# GTK3+ libs
if(WITH_GTK)
	message("${BoldCyan}**   ${BoldCyan}GTK${ColourReset} is ${BoldGreen}ON${ColourReset}")
else(WITH_GTK)
	message("${BoldCyan}**	 ${BoldCyan}GTK${ColourReset} is ${BoldRed}OFF${ColourReset}")
endif(WITH_GTK)

# NCurses libs
if(WITH_NCURSES)
	message("${BoldCyan}**	 NCURSES${ColourReset} is ${BoldGreen}ON${ColourReset}")
else(WITH_NCURSES)
	message("${BoldCyan}**	 NCURSES${ColourReset} is ${BoldRed}OFF${ColourReset}")
endif(WITH_NCURSES)

# Libcpuid lib
if(WITH_LIBCPUID)
	message("${BoldCyan}**	 ${BoldCyan}LIBCPUID${ColourReset} is ${BoldGreen}ON${ColourReset}")
else(WITH_LIBCPUID)
	message("${BoldCyan}**	 ${BoldCyan}LIBCPUID${ColourReset} is ${BoldRed}OFF${ColourReset}")
endif(WITH_LIBCPUID)

# Dmidecode lib
if(WITH_LIBDMI)
	message("${BoldCyan}**	 ${BoldCyan}LIBDMI${ColourReset} is ${BoldGreen}ON${ColourReset}")
else(WITH_LIBDMI)
	message("${BoldCyan}**	 ${BoldCyan}LIBDMI${ColourReset} is ${BoldRed}OFF${ColourReset}")
endif(WITH_LIBDMI)

# Libprocps lib
if(WITH_LIBPROCPS)
	message("${BoldCyan}**	 ${BoldCyan}LIBPROCPS${ColourReset} is ${BoldGreen}ON${ColourReset}")
else(WITH_LIBPROCPS)
	message("${BoldCyan}**	 ${BoldCyan}LIBPROCPS${ColourReset} is ${BoldRed}OFF${ColourReset}")
endif(WITH_LIBPROCPS)
