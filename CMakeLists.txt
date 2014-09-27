cmake_minimum_required(VERSION 3.0)

project(CPU-X)

set(EXECUTABLE_OUTPUT_PATH bin/)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# Colours
if(NOT WIN32)
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
endif()

# GTK3+ libs
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

# Libcpuid lib
pkg_check_modules(LIBCPUID REQUIRED libcpuid)
include_directories(${LIBCPUID_INCLUDE_DIRS})
link_directories(${LIBCPUID_LIBRARY_DIRS})
add_definitions(-DLIBCPUID)

# Dmidecode lib
add_subdirectory(src/dmidecode)
include_directories(src/dmidecode)
add_definitions(-DLIBDMI)

if(${EMBED})
message("${BoldBlue}CPU-X will be compiled as portable binary.${ColourReset}")
set(LIBCPUID_LIBRARIES -l:libcpuid.a)
execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/embed.sh
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
add_definitions(-DEMBED)
endif(${EMBED})

# Main executable
add_executable(
	cpu-x
	src/cpu-x.c
	src/cpu-x.h
	src/gui.c
)

target_link_libraries(cpu-x ${GTK3_LIBRARIES} ${LIBCPUID_LIBRARIES} dmi-cpux)

# Installation
install(TARGETS cpu-x DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(FILES
	data/AMD.png
	data/CPU-X.png
	data/cpu-x.ui
	data/Intel.png
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cpu-x/
)
