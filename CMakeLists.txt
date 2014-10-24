cmake_minimum_required(VERSION 2.8)

project(CPU-X)

set(EXECUTABLE_OUTPUT_PATH bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib/)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

option(WITH_GTK "Add support for GUI in GTK3+" ON)
option(WITH_NCURSES "Add support for GUI in NCurses" ON)
option(WITH_LIBCPUID "Allow use of library Libcpuid" ON)
option(WITH_LIBDMI "Allow use of library Libdmi" ON)


find_package(PkgConfig REQUIRED)

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
if(WITH_GTK)
  pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
  include_directories(${GTK3_INCLUDE_DIRS})
  link_directories(${GTK3_LIBRARY_DIRS})
  add_definitions(${GTK3_CFLAGS_OTHER} -DGTK)
  add_library(gui_gtk
		src/gui_gtk.c
		src/cpu-x_gtk.h
  )
  set(GUI_GTK_LIBRARY "gui_gtk" ${GTK3_LIBRARIES})
endif()

# NCurses libs
if(WITH_NCURSES)
  pkg_check_modules(NCURSES REQUIRED ncurses)
  include_directories(${NCURSES_INCLUDE_DIRS})
  link_directories(${NCURSES_LIBRARY_DIRS})
  add_definitions(-DNCURSES)
  add_library(gui_ncurses
		src/gui_ncurses.c
		src/cpu-x_ncurses.h
  )
  set(GUI_NCURSES_LIBRARY "gui_ncurses" ${NCURSES_LIBRARIES} -lpthread)
endif()

# Libcpuid lib
if(WITH_LIBCPUID)
  pkg_check_modules(LIBCPUID libcpuid)
  if(LIBCPUID_FOUND)
    include_directories(${LIBCPUID_INCLUDE_DIRS})
    link_directories(${LIBCPUID_LIBRARY_DIRS})
    add_definitions(-DLIBCPUID)
  endif()
endif(WITH_LIBCPUID)

# Dmidecode lib
if(WITH_LIBDMI)
  add_subdirectory(src/dmidecode)
  include_directories(src/dmidecode)
  add_definitions(-DLIBDMI)
  set(LIBDMI_LIBRARY "dmi-cpux")
endif()

if(EMBED)
  message("${BoldBlue}CPU-X will be compiled as portable binary.${ColourReset}")
  if(WITH_LIBCPUID)
    set(LIBCPUID_LIBRARIES -l:libcpuid.a)
  endif()
  if(WITH_GTK)
    execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/embed.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )
  endif()
  add_definitions(-DEMBED)
endif()

# Main executable
add_executable(cpu-x
	src/cpu-x.c
	src/cpu-x.h
	src/options.c
)

target_link_libraries(cpu-x ${GUI_GTK_LIBRARY} ${GUI_NCURSES_LIBRARY} ${LIBCPUID_LIBRARIES} ${LIBDMI_LIBRARY})

# Installation
install(TARGETS cpu-x DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
if(WITH_GTK)
  install(FILES
	data/AMD.png
	data/CPU-X.png
	data/cpu-x.ui
	data/Intel.png
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cpu-x/
)
endif()
