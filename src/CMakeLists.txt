cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(binary)


### Libs

# GTK3+ libs
if(WITH_GTK)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	if(GTK3_FOUND)
		include_directories(${GTK3_INCLUDE_DIRS})
		link_directories(${GTK3_LIBRARY_DIRS})
		add_definitions(${GTK3_CFLAGS_OTHER} -DGTK)
		if(EMBED)
			set(GRESOURCES "gtk-resources.c")
		endif(EMBED)
		add_library(gui_gtk
			gui_gtk.c
			gui_gtk.h
			gui_gtk_id.h
			${GRESOURCES}
		)
		set(GUI_GTK_LIBRARY "gui_gtk" ${GTK3_LIBRARIES})
	else(GTK3_FOUND)
		message("${BoldYellow} GTK3 headers not found!")
		message("Set PKG_CONFIG_PATH environment variable,")
		message("or use CMake variable ${ColourReset}${ColourBold}-DWITH_GTK=0${ColourReset}${BoldYellow}.${ColourReset}")
	endif(GTK3_FOUND)
endif(WITH_GTK)

# NCurses libs
if(WITH_NCURSES)
	pkg_check_modules(NCURSES ncursesw)
	if(NCURSES_FOUND)
		include_directories(${NCURSES_INCLUDE_DIRS})
		link_directories(${NCURSES_LIBRARY_DIRS})
		add_definitions(-DNCURSES)
		add_library(tui_ncurses
			tui_ncurses.c
			tui_ncurses.h
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
	add_subdirectory(dmidecode)
	include_directories(dmidecode)
	add_definitions(-DLIBDMI)
	set(LIBDMI_LIBRARY "dmi")
endif(WITH_LIBDMI)

# Pciutils lib
if(WITH_LIBPCI)
	pkg_check_modules(LIBPCI libpci)
	if(LIBPCI_FOUND)
		include_directories(${LIBPCI_INCLUDE_DIRS})
		link_directories(${LIBPCI_LIBRARY_DIRS})
		add_definitions(${LIBPCI_CFLAGS_OTHER} -DLIBPCI)
		# Can't link with libpci.a, always use shared library
		#if(EMBED)
		#	set(LIBPCI_LIBRARIES -l:libpci.a)
		#endif(EMBED)
	endif(LIBPCI_FOUND)
endif(WITH_LIBPCI)

# Libprocps / Libstatgrab lib
if(WITH_LIBSYSTEM)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux") # Linux: Libprocps
		pkg_check_modules(LIBPROCPS libprocps)
		if(LIBPROCPS_FOUND)
			include_directories(${LIBPROCPS_INCLUDE_DIRS})
			link_directories(${LIBPROCPS_LIBRARY_DIRS})
			add_definitions(${LIBPROCPS_CFLAGS_OTHER} -DLIBPROCPS)
			if(EMBED)
				set(LIBPROCPS_LIBRARIES -l:libprocps.a)
			endif(EMBED)
		endif(LIBPROCPS_FOUND)

	else(${CMAKE_SYSTEM_NAME} MATCHES "Linux") # Others: Libstatgrab
		pkg_check_modules(LIBSTATGRAB libstatgrab)
		if(LIBSTATGRAB_FOUND)
			include_directories(${LIBSTATGRAB_INCLUDE_DIRS})
			link_directories(${LIBSTATGRAB_LIBRARY_DIRS})
			add_definitions(${LIBSTATGRAB_CFLAGS_OTHER} -DLIBSTATGRAB)
			if(EMBED)
				set(LIBSTATGRAB_LIBRARIES -l:libstatgrab.a)
			endif(EMBED)
		endif(LIBSTATGRAB_FOUND)
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endif(WITH_LIBSYSTEM)


### Main executable

if(EMBED)
	add_definitions(-DEMBED)
endif(EMBED)

add_definitions(-DPRGVER="${PROJECT_VERSION}" -DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}" -DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

add_executable(cpu-x
	cpu-x.c
	cpu-x.h
	libsystem.c
	options.c
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
	${LIBPCI_LIBRARIES}
	${LIBPROCPS_LIBRARIES}
	${LIBSTATGRAB_LIBRARIES}
	${ADDITIONNAL_LIBRARIES}
)


# Installation
install(TARGETS cpu-x DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
