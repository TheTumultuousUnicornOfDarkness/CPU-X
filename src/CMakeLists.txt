cmake_minimum_required(VERSION 2.8)
if(${CMAKE_VERSION} VERSION_GREATER "2.9")
	cmake_policy(SET CMP0048 OLD)
endif(${CMAKE_VERSION} VERSION_GREATER "2.9")
project(binary)


### FIND NEEDED LIBRAIRIES

# GTK3+
if(WITH_GTK)
	pkg_check_modules(GTK3 gtk+-3.0>=3.12.0)
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
		set(GTK3_LIBRARIES "gui_gtk" ${GTK3_LIBRARIES})
	else(GTK3_FOUND)
		message("${BoldYellow}GTK3+ GUI will not be build:${ColourReset} ${Yellow}GTK 3.8 or higher is needed.${ColourReset}")
		message("${Yellow}Set ${BoldYellow}PKG_CONFIG_PATH${ColourReset}${Yellow} environment variable if development files are installed.${ColourReset}")
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
		add_embedded_library(NCURSES_LIBRARIES "-l:libncursesw.a")
		add_embedded_library_others(NCURSES_LIBRARIES "${NCURSES_LIBRARIES} -l:libtinfo.a")
		set(NCURSES_LIBRARIES "tui_ncurses" ${NCURSES_LIBRARIES})
	endif(NCURSES_FOUND)
endif(WITH_NCURSES)

# Libcpuid
if(WITH_LIBCPUID)
	pkg_check_modules(LIBCPUID libcpuid>=0.3.0)
	if(LIBCPUID_FOUND)
		include_directories(${LIBCPUID_INCLUDE_DIRS})
		link_directories(${LIBCPUID_LIBRARY_DIRS})
		add_definitions(${LIBCPUID_CFLAGS_OTHER})
		add_embedded_library(LIBCPUID_LIBRARIES "-l:libcpuid.a")
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
		add_embedded_library(LIBPCI_LIBRARIES "-l:libpci.a")
		add_embedded_library_linux(LIBPCI_LIBRARIES "${LIBPCI_LIBRARIES} -lresolv -ludev")
		add_embedded_library_others(LIBPCI_LIBRARIES "${LIBPCI_LIBRARIES} -lz")
	endif(LIBPCI_FOUND)
endif(WITH_LIBPCI)

# Libprocps / Libstatgrab
if(WITH_LIBSYSTEM)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB) # Linux: Libprocps
		pkg_check_modules(LIBPROCPS libprocps)
		if(LIBPROCPS_FOUND)
			include_directories(${LIBPROCPS_INCLUDE_DIRS})
			link_directories(${LIBPROCPS_LIBRARY_DIRS})
			add_definitions(${LIBPROCPS_CFLAGS_OTHER} -DHAS_LIBSYSTEM=1)
			add_embedded_library(LIBPROCPS_LIBRARIES "-l:libprocps.a")
		endif(LIBPROCPS_FOUND)
	elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB) # Others: Libstatgrab
		pkg_check_modules(LIBSTATGRAB libstatgrab)
		if(LIBSTATGRAB_FOUND)
			include_directories(${LIBSTATGRAB_INCLUDE_DIRS})
			link_directories(${LIBSTATGRAB_LIBRARY_DIRS})
			add_definitions(${LIBSTATGRAB_CFLAGS_OTHER} -DHAS_LIBSYSTEM=1)
			add_embedded_library(LIBSTATGRAB_LIBRARIES "-l:libstatgrab.a")
			add_embedded_library_others(LIBSTATGRAB_LIBRARIES "${LIBSTATGRAB_LIBRARIES} -ldevstat")
		endif(LIBSTATGRAB_FOUND)
	endif()
endif(WITH_LIBSYSTEM)
if(NOT WITH_LIBSYSTEM OR (NOT LIBPROCPS_FOUND AND NOT LIBSTATGRAB_FOUND))
	add_definitions(-DHAS_LIBSYSTEM=0)
endif(NOT WITH_LIBSYSTEM OR (NOT LIBPROCPS_FOUND AND NOT LIBSTATGRAB_FOUND))

# Libcurl
if(WITH_LIBCURL)
	pkg_check_modules(LIBCURL libcurl)
	if(LIBCURL_FOUND)
		include_directories(${LIBCURL_INCLUDE_DIRS})
		link_directories(${LIBCURL_LIBRARY_DIRS})
		add_definitions(${LIBCURL_CFLAGS_OTHER})
		add_embedded_library(LIBCURL_LIBRARIES "-l:libcurl.a")
		add_embedded_library_linux(LIBCURL_LIBRARIES "${LIBCURL_LIBRARIES} -lidn -lssl -lssh2 -lcrypto -lgssapi_krb5 -lrtmp -lz")
	endif(LIBCURL_FOUND)
endif(WITH_LIBCURL)


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
print_config("LIBCURL" "${LIBCURL_VERSION}" LIBCURL_FOUND WITH_LIBCURL)

# Subprojects
print_config("DMIDECODE" "${DMIDECODE_VERSION}" 1 WITH_DMIDECODE)
print_config("BANDWIDTH" "${BANDWIDTH_VERSION}" BANDWIDTH_FOUND WITH_BANDWIDTH)

### MAIN BINARY

add_executable(cpu-x
	main.c
	util.c
	cpu-x.h
	core.c
	core.h
)

if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
	set(BITNESS "32")
else("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
	set(BITNESS "64")
endif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DOS="linux${BITNESS}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES FreeBSD OR ${CMAKE_SYSTEM_NAME} MATCHES DragonFly)
	add_definitions(-DOS="bsd${BITNESS}")
else()
	message("${BoldYellow}Unknown operating system: ${CMAKE_SYSTEM_NAME}${ColourReset}")
	add_definitions(-DOS="unknown${BITNESS}")
endif()

if(PORTABLE_BINARY)
	message("${BoldBlue}${CMAKE_PROJECT_NAME} will be compiled as portable binary.${ColourReset}")
	add_definitions(-DPORTABLE_BINARY=1)
	if(WITH_GETTEXT)
		set(CMAKE_INSTALL_FULL_LOCALEDIR "/tmp/.cpu-x")
		add_dependencies(cpu-x convertgmo)
	endif(WITH_GETTEXT)
else(PORTABLE_BINARY)
	add_definitions(-DPORTABLE_BINARY=0)
	if(GETTEXT_FOUND)
		add_dependencies(cpu-x potfiles)
	endif(GETTEXT_FOUND)
endif(PORTABLE_BINARY)

add_definitions(-DPRGVER="${PROJECT_VERSION}" -DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}" -DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}" -DCC="${CMAKE_C_COMPILER_ID}")

target_link_libraries(cpu-x
	${GTK3_LIBRARIES}
	${NCURSES_LIBRARIES}
	${LIBCPUID_LIBRARIES}
	${DMIDECODE_LIBRARY}
	${BANDWIDTH_LIBRARY}
	${LIBPCI_LIBRARIES}
	${LIBPROCPS_LIBRARIES}
	${LIBSTATGRAB_LIBRARIES}
	${LIBCURL_LIBRARIES}

	${Intl_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${Backtrace_LIBRARIES}
	m
)


### INSTALLATION

install(TARGETS cpu-x DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
