cmake_minimum_required(VERSION 2.8)
if(APPLE)
  cmake_policy(SET CMP0025 NEW)
endif(APPLE)

project(CPU-X)

set(EXECUTABLE_OUTPUT_PATH bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib/)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wuninitialized -Wstrict-prototypes")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -Wuninitialized -Wstrict-prototypes")

option(WITH_GTK "Add support for GUI in GTK3+" ON)
option(WITH_NCURSES "Add support for TUI in NCurses" ON)
option(WITH_LIBCPUID "Allow use of library Libcpuid" ON)
option(WITH_LIBDMI "Allow use of library Libdmi" ON)
option(WITH_LIBPROCPS "Allow use of library Libprocps" ON)


include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
add_subdirectory(po)

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
endif(NOT WIN32)

# GTK3+ libs
if(WITH_GTK)
  pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
  include_directories(${GTK3_INCLUDE_DIRS})
  link_directories(${GTK3_LIBRARY_DIRS})
  add_definitions(${GTK3_CFLAGS_OTHER} -DGTK)
  add_library(gui_gtk
		src/gui_gtk.c
		src/gui_gtk.h
  )
  set(GUI_GTK_LIBRARY "gui_gtk" ${GTK3_LIBRARIES})
endif(WITH_GTK)

# NCurses libs
if(WITH_NCURSES)
  pkg_check_modules(NCURSES ncurses)
  if(NCURSES_FOUND)
    include_directories(${NCURSES_INCLUDE_DIRS})
    link_directories(${NCURSES_LIBRARY_DIRS})
    add_definitions(-DNCURSES)
    add_library(tui_ncurses
		src/tui_ncurses.c
		src/tui_ncurses.h
    )
  if(EMBED)
    set(NCURSES_LIBRARIES -l:libncursesw.a)
  endif(EMBED)
  set(TUI_NCURSES_LIBRARY "tui_ncurses" ${NCURSES_LIBRARIES} -lpthread)
  endif(NCURSES_FOUND)
endif(WITH_NCURSES)

# Libcpuid lib
if(WITH_LIBCPUID)
  pkg_check_modules(LIBCPUID libcpuid)
  if(LIBCPUID_FOUND)
    include_directories(${LIBCPUID_INCLUDE_DIRS})
    link_directories(${LIBCPUID_LIBRARY_DIRS})
    add_definitions(${LIBCPUID_CFLAGS_OTHER} -DLIBCPUID)
    if(EMBED)
      set(LIBCPUID_LIBRARIES -l:libcpuid.a)
    endif(EMBED)
  endif(LIBCPUID_FOUND)
endif(WITH_LIBCPUID)

# Dmidecode lib
if(WITH_LIBDMI)
  add_subdirectory(src/dmidecode)
  include_directories(src/dmidecode)
  add_definitions(-DLIBDMI)
  set(LIBDMI_LIBRARY "dmi-cpux")
endif()

# Libprocps lib
if(WITH_LIBPROCPS)
  pkg_check_modules(LIBPROCPS libprocps)
  if(LIBPROCPS_FOUND)
    include_directories(${LIBPROCPS_INCLUDE_DIRS})
    link_directories(${LIBPROCPS_LIBRARY_DIRS})
    add_definitions(${LIBPROCPS_CFLAGS_OTHER} -DLIBPROCPS)
    if(EMBED)
      set(LIBPROCPS_LIBRARIES -l:libprocps.a)
    endif(EMBED)
  endif(LIBPROCPS_FOUND)
endif(WITH_LIBPROCPS)

# Embed compilation
if(EMBED)
  message("${BoldBlue}CPU-X will be compiled as portable binary.${ColourReset}")
  if(WITH_GTK)
    message("${Green}Converting files to embed${ColourReset}")
    execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/embed.sh
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
  endif(WITH_GTK)
  add_definitions(-DEMBED)
endif(EMBED)

# Main executable
add_definitions(-DGETTEXT_PACKAGE="cpux" -DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

add_executable(cpu-x
	src/cpu-x.c
	src/cpu-x.h
	src/options.c
)

set(ADDITIONNAL_LIBRARIES "-lm")
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(ADDITIONNAL_LIBRARIES "${ADDITIONNAL_LIBRARIES} -lintl")
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

target_link_libraries(cpu-x
	${GUI_GTK_LIBRARY}
	${TUI_NCURSES_LIBRARY}
	${LIBCPUID_LIBRARIES}
	${LIBDMI_LIBRARY}
	${LIBPROCPS_LIBRARIES}
	${ADDITIONNAL_LIBRARIES}
)

# Installation
install(TARGETS cpu-x DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
if(WITH_GTK)
  file(GLOB_RECURSE
	pictures
        *.png
  )
  install(FILES
	data/cpux-gtk-3.8.ui
	${pictures}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cpu-x/
  )
  install(FILES
	data/cpu-x.desktop
	data/cpu-x-root.desktop
	DESTINATION /usr/share/applications/
  )
  install(FILES
	data/org.pkexec.cpu-x.policy
	DESTINATION /usr/share/polkit-1/actions
  )
  install(FILES
	cpu-x_polkit
	DESTINATION /usr/bin
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
  install(FILES
	data/CPU-X.png
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps/
	RENAME cpu-x.png
  )
endif()
