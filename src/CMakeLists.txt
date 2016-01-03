cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 OLD)
project(binary)


### FIND NEEDED LIBRAIRIES

# GTK3+
if(WITH_GTK)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	if(GTK3_FOUND)
		include_directories(${GTK3_INCLUDE_DIRS})
		link_directories(${GTK3_LIBRARY_DIRS})
		add_definitions(${GTK3_CFLAGS_OTHER})
		if(PORTABLE_BINARY)
			execute_process(COMMAND glib-compile-resources --target=${CMAKE_SOURCE_DIR}/src/gtk-resources.c --generate-source cpu-x.gresource.xml
					COMMAND	glib-compile-resources --target=${CMAKE_SOURCE_DIR}/src/gtk-resources.h --generate-header cpu-x.gresource.xml
					WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/data
			)
			set(GRESOURCES "gtk-resources.c")
		endif(PORTABLE_BINARY)
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

# NCurses
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
	endif(NCURSES_FOUND)
endif(WITH_NCURSES)

# Libcpuid
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
	endif(LIBCPUID_FOUND)
endif(WITH_LIBCPUID)

# Dmidecode
if(WITH_DMIDECODE)
	add_subdirectory(dmidecode)
	include_directories(dmidecode)
	set(DMIDECODE_LIBRARY "dmidecode")
endif(WITH_DMIDECODE)

# Bandwidth
if(WITH_BANDWIDTH)
	add_subdirectory(bandwidth)
	if(BANDWIDTH_FOUND)
		include_directories(bandwidth)
		set(BANDWIDTH_LIBRARY "bandwidth")
	endif(BANDWIDTH_FOUND)
endif(WITH_BANDWIDTH)

# Pciutils
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
	endif(LIBPCI_FOUND)
endif(WITH_LIBPCI)

# Libprocps / Libstatgrab
if(WITH_LIBSYSTEM)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB) # Linux: Libprocps
		pkg_check_modules(LIBPROCPS libprocps)
		if(LIBPROCPS_FOUND)
			include_directories(${LIBPROCPS_INCLUDE_DIRS})
			link_directories(${LIBPROCPS_LIBRARY_DIRS})
			add_definitions(${LIBPROCPS_CFLAGS_OTHER})
			add_embedded_library(-l:libprocps.a)
		endif(LIBPROCPS_FOUND)
	elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB) # Others: Libstatgrab
		pkg_check_modules(LIBSTATGRAB libstatgrab)
		if(LIBSTATGRAB_FOUND)
			include_directories(${LIBSTATGRAB_INCLUDE_DIRS})
			link_directories(${LIBSTATGRAB_LIBRARY_DIRS})
			add_definitions(${LIBSTATGRAB_CFLAGS_OTHER})
			add_embedded_library(-l:libstatgrab.a)
			add_embedded_library_others(-ldevstat)
		endif(LIBSTATGRAB_FOUND)
	endif()
endif(WITH_LIBSYSTEM)

# Various needed librairies
set(ADDITIONNAL_LIBRARIES "-lm")
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(ADDITIONNAL_LIBRARIES "${ADDITIONNAL_LIBRARIES} -lexecinfo -lintl")
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")


### FINAL CONFIG

message("${BoldCyan}** ${CMAKE_PROJECT_NAME} ${PROJECT_VERSION} configuration **${ColourReset}")

# UI librairies
print_config("GTK" "${GTK3_VERSION}" GTK3_FOUND WITH_GTK)
print_config("NCURSES" "${NCURSES_VERSION}" NCURSES_FOUND WITH_NCURSES)

# Others librairies
print_config("GETTEXT" 0 GETTEXT_FOUND WITH_GETTEXT)
print_config("LIBCPUID" "${LIBCPUID_VERSION}" LIBCPUID_FOUND WITH_LIBCPUID)
print_config("LIBPCI" "${LIBPCI_VERSION}" LIBPCI_FOUND WITH_LIBPCI)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)
	print_config("LIBPROCPS" "${LIBPROCPS_VERSION}" LIBPROCPS_FOUND WITH_LIBSYSTEM)
	add_definitions(-DHAS_LIBSTATGRAB=0 -DLIBSTATGRAB_VERSION=NULL)
elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB)
	print_config("LIBSTATGRAB" "${LIBSTATGRAB_VERSION}" LIBSTATGRAB_FOUND WITH_LIBSYSTEM)
	add_definitions(-DHAS_LIBPROCPS=0 -DLIBPROCPS_VERSION=NULL)
endif()

# Subprojects
print_config("DMIDECODE" "${DMIDECODE_VERSION}" 1 WITH_DMIDECODE)
print_config("BANDWIDTH" "${BANDWIDTH_VERSION}" BANDWIDTH_FOUND WITH_BANDWIDTH)


### MAIN BINARY

add_executable(cpu-x
	main.c
	cpu-x.h
	core.c
	core.h
)

if(PORTABLE_BINARY)
	message("${BoldBlue}${CMAKE_PROJECT_NAME} will be compiled as portable binary.${ColourReset}")
	message("${BoldBlue}These librairies will be added to binary:${ColourReset} ${EMBEDDED_LIBRARIES}")
	add_definitions(-DPORTABLE_BINARY=1)
	if(WITH_GETTEXT)
		set(CMAKE_INSTALL_FULL_LOCALEDIR "/tmp/.cpu-x")
		add_dependencies(cpu-x convertgmo)
	endif(WITH_GETTEXT)
else(PORTABLE_BINARY)
	if(GETTEXT_FOUND)
		add_dependencies(cpu-x potfiles)
	endif(GETTEXT_FOUND)
	add_definitions(-DPORTABLE_BINARY=0)
endif(PORTABLE_BINARY)

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


### INSTALLATION

install(TARGETS cpu-x DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
