cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(binary)


### Check libraries

# GTK3+ libs
if(WITH_GTK)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	if(GTK3_FOUND)
		include_directories(${GTK3_INCLUDE_DIRS})
		link_directories(${GTK3_LIBRARY_DIRS})
		add_definitions(${GTK3_CFLAGS_OTHER})
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
		add_definitions(-DGTK_VERSION="${GTK3_VERSION}")
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
		add_library(tui_ncurses
			tui_ncurses.c
			tui_ncurses.h
		)
		add_embedded_library(-l:libncursesw.a)
		add_embedded_library_others(-l:libtinfo.a)
		set(TUI_NCURSES_LIBRARY "tui_ncurses" ${NCURSES_LIBRARIES} -lpthread)
		add_definitions(-DNCURSES_VERSION="${NCURSES_VERSION}")
	endif(NCURSES_FOUND)
endif(WITH_NCURSES)

# Libcpuid lib
if(WITH_LIBCPUID)
	pkg_check_modules(LIBCPUID libcpuid)
	if(LIBCPUID_FOUND)
		include_directories(${LIBCPUID_INCLUDE_DIRS})
		link_directories(${LIBCPUID_LIBRARY_DIRS})
		add_definitions(${LIBCPUID_CFLAGS_OTHER})
		add_embedded_library(-l:libcpuid.a)
		if(${LIBCPUID_VERSION} VERSION_GREATER "0.2.1")
			add_definitions(${LIBCPUID_CFLAGS_OTHER} -DHAVE_LIBCPUID_0_2_2)
		endif(${LIBCPUID_VERSION} VERSION_GREATER "0.2.1")
		add_definitions(-DLIBCPUID_VERSION="${LIBCPUID_VERSION}")
	endif(LIBCPUID_FOUND)
endif(WITH_LIBCPUID)

# Dmidecode lib
if(WITH_DMIDECODE)
	add_subdirectory(dmidecode)
	include_directories(dmidecode)
	set(DMIDECODE_LIBRARY "dmi")
	add_definitions(-DDMIDECODE_VERSION="${DMIDECODE_VERSION}")
endif(WITH_DMIDECODE)

# Bandwidth lib
if(WITH_BANDWIDTH)
	set(BANDWIDTH_FOUND OFF)
	add_subdirectory(bandwidth)
	if(BANDWIDTH_FOUND)
		include_directories(bandwidth)
		set(BANDWIDTH_LIBRARY "bandwidth")
		add_definitions(-DBANDWIDTH_VERSION="${BANDWIDTH_VERSION}")
	endif(BANDWIDTH_FOUND)
endif(WITH_BANDWIDTH)

# Pciutils lib
if(WITH_LIBPCI)
	pkg_check_modules(LIBPCI libpci)
	if(LIBPCI_FOUND)
		include_directories(${LIBPCI_INCLUDE_DIRS})
		link_directories(${LIBPCI_LIBRARY_DIRS})
		add_definitions(${LIBPCI_CFLAGS_OTHER})
		add_embedded_library(-l:libpci.a)
		add_embedded_library_linux(-lresolv)
		add_embedded_library_linux(-ludev)
		add_embedded_library_others(-lz)
		add_definitions(-DLIBPCI_VERSION="${LIBPCI_VERSION}")
	endif(LIBPCI_FOUND)
endif(WITH_LIBPCI)

# Libprocps / Libstatgrab lib
if(WITH_LIBSYSTEM)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB) # Linux: Libprocps
		pkg_check_modules(LIBPROCPS libprocps)
		if(LIBPROCPS_FOUND)
			include_directories(${LIBPROCPS_INCLUDE_DIRS})
			link_directories(${LIBPROCPS_LIBRARY_DIRS})
			add_definitions(${LIBPROCPS_CFLAGS_OTHER})
			add_embedded_library(-l:libprocps.a)
			add_definitions(-DHAS_LIBPROCPS -DLIBPROCPS_VERSION="${LIBPROCPS_VERSION}")
		endif(LIBPROCPS_FOUND)
	elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB) # Others: Libstatgrab
		pkg_check_modules(LIBSTATGRAB libstatgrab)
		if(LIBSTATGRAB_FOUND)
			include_directories(${LIBSTATGRAB_INCLUDE_DIRS})
			link_directories(${LIBSTATGRAB_LIBRARY_DIRS})
			add_definitions(${LIBSTATGRAB_CFLAGS_OTHER})
			add_embedded_library(-l:libstatgrab.a)
			add_embedded_library_others(-ldevstat)
			add_definitions(-DHAS_LIBSTATGRAB -DLIBSTATGRAB_VERSION="${LIBSTATGRAB_VERSION}")
		endif(LIBSTATGRAB_FOUND)
	endif()
endif(WITH_LIBSYSTEM)

# Needed librairies
set(ADDITIONNAL_LIBRARIES "-lm")
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(ADDITIONNAL_LIBRARIES "${ADDITIONNAL_LIBRARIES} -lintl")
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")


### Final config

message("${BoldCyan}** ${CMAKE_PROJECT_NAME} ${PROJECT_VERSION} configuration **${ColourReset}")

# UI librairies
print_config("GTK" GTK3_FOUND WITH_GTK)
print_config("NCURSES" NCURSES_FOUND WITH_NCURSES)

# Others librairies
print_config("LIBCPUID" LIBCPUID_FOUND WITH_LIBCPUID)
print_config("LIBPCI" LIBPCI_FOUND WITH_LIBPCI)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)
	print_config("LIBPROCPS" LIBPROCPS_FOUND WITH_LIBSYSTEM)
elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB)
	print_config("LIBSTATGRAB" LIBSTATGRAB_FOUND WITH_LIBSYSTEM)
endif()

# Subprojects
print_config("DMIDECODE" 1 WITH_DMIDECODE)
print_config("BANDWIDTH" BANDWIDTH_FOUND WITH_BANDWIDTH)


### Main executable

add_executable(cpu-x
	main.c
	core.c
	core.h
	libsystem.c
)

if(PORTABLE_BINARY OR EMBED)
	message("${BoldBlue}${CMAKE_PROJECT_NAME} will be compiled as portable binary.${ColourReset}")
	message("${BoldBlue}These librairies will be added to binary:${ColourReset} ${EMBEDDED_LIBRARIES}")
	add_definitions(-DEMBED)
	if(WITH_GETTEXT)
		set(CMAKE_INSTALL_FULL_LOCALEDIR "/tmp/.cpu-x")
		add_dependencies(cpu-x convertgmo)
		add_definitions(-DGETTEXT)
	endif(WITH_GETTEXT)
elseif(NOT (PORTABLE_BINARY OR EMBED) AND WITH_GETTEXT)
	add_dependencies(cpu-x potfiles)
endif(PORTABLE_BINARY OR EMBED)

add_definitions(-DPRGVER="${PROJECT_VERSION}" -DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}" -DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

target_link_libraries(cpu-x
	${GUI_GTK_LIBRARY}
	${TUI_NCURSES_LIBRARY}
	${LIBCPUID_LIBRARIES}
	${DMIDECODE_LIBRARY}
	${BANDWIDTH_LIBRARY}
	${LIBPCI_LIBRARIES}
	${LIBPROCPS_LIBRARIES}
	${LIBSTATGRAB_LIBRARIES}
	${ADDITIONNAL_LIBRARIES}
	${EMBEDDED_LIBRARIES}
)


# Installation
install(TARGETS cpu-x DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
