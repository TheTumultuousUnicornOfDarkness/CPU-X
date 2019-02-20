### FIND NEEDED LIBRAIRIES

# GTK3+
if(WITH_GTK)
	set(MINIMUM_GTK_VERSION 3.12.0)
	pkg_check_modules(GTK3 gtk+-3.0>=${MINIMUM_GTK_VERSION})
	if(GTK3_FOUND)
		include_directories(${GTK3_INCLUDE_DIRS})
		link_directories(${GTK3_LIBRARY_DIRS})
		add_definitions(${GTK3_CFLAGS_OTHER})
		add_library(gui_gtk
			gui_gtk.c
			gui_gtk.h
			gui_gtk_id.h
			${GRESOURCES}
		)
		set(GTK3_LIBRARIES "gui_gtk" ${GTK3_LIBRARIES})
	else(GTK3_FOUND)
		message("${BoldYellow}GTK3+ GUI will not be build:${ColourReset} ${Yellow}GTK ${MINIMUM_GTK_VERSION} or higher is needed.${ColourReset}")
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
		set(NCURSES_LIBRARIES "tui_ncurses" ${NCURSES_LIBRARIES})
	endif(NCURSES_FOUND)
endif(WITH_NCURSES)

# Gettext
if(Intl_FOUND)
	include_directories(${CMAKE_BINARY_DIR}/po)
endif(Intl_FOUND)

# Libcurl
if(WITH_LIBCURL)
	pkg_check_modules(LIBCURL libcurl)
	if(LIBCURL_FOUND)
		include_directories(${LIBCURL_INCLUDE_DIRS})
		link_directories(${LIBCURL_LIBRARY_DIRS})
		add_definitions(${LIBCURL_CFLAGS_OTHER})
	endif(LIBCURL_FOUND)
endif(WITH_LIBCURL)

# Libjson-c
if(WITH_LIBJSONC)
	pkg_check_modules(LIBJSONC json-c)
	if(LIBJSONC_FOUND)
		include_directories(${LIBJSONC_INCLUDE_DIRS})
		link_directories(${LIBJSONC_LIBRARY_DIRS})
		add_definitions(${LIBJSONC_CFLAGS_OTHER})
	endif(LIBJSONC_FOUND)
endif(WITH_LIBJSONC)

# Libcpuid
if(WITH_LIBCPUID)
	pkg_check_modules(LIBCPUID libcpuid>=0.4.0)
	if(LIBCPUID_FOUND)
		include_directories(${LIBCPUID_INCLUDE_DIRS})
		link_directories(${LIBCPUID_LIBRARY_DIRS})
		add_definitions(${LIBCPUID_CFLAGS_OTHER})
		set(CMAKE_REQUIRED_LIBRARIES ${LIBCPUID_LIBRARIES})
		check_symbol_exists(msr_serialize_raw_data "libcpuid/libcpuid.h" HAVE_MSR_SERIALIZE_RAW_DATA)
		if(HAVE_MSR_SERIALIZE_RAW_DATA)
			add_definitions(-DHAVE_MSR_SERIALIZE_RAW_DATA)
		endif(HAVE_MSR_SERIALIZE_RAW_DATA)
		check_include_file("stdint.h" HAVE_STDINT_H)
		if(NOT HAVE_STDINT_H)
			check_include_file("sys/stdint.h" HAVE_STDINT_H)
		endif(NOT HAVE_STDINT_H)
		if(HAVE_STDINT_H)
			add_definitions(-DHAVE_STDINT_H)
		endif(HAVE_STDINT_H)
	endif(LIBCPUID_FOUND)
endif(WITH_LIBCPUID)

# Libpci
if(WITH_LIBPCI)
	pkg_check_modules(LIBPCI libpci)
	if(LIBPCI_FOUND)
		include_directories(${LIBPCI_INCLUDE_DIRS})
		link_directories(${LIBPCI_LIBRARY_DIRS})
		add_definitions(${LIBPCI_CFLAGS_OTHER})
	endif(LIBPCI_FOUND)
endif(WITH_LIBPCI)

# Libprocps
set(LIBSYSTEM 0)
if(WITH_LIBPROCPS AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)
	pkg_check_modules(LIBPROCPS libprocps)
	if(LIBPROCPS_FOUND)
		include_directories(${LIBPROCPS_INCLUDE_DIRS})
		link_directories(${LIBPROCPS_LIBRARY_DIRS})
		add_definitions(${LIBPROCPS_CFLAGS_OTHER})
		set(LIBSYSTEM 1)
	endif(LIBPROCPS_FOUND)
endif(WITH_LIBPROCPS AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)

# Libstatgrab
if(WITH_LIBSTATGRAB AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB))
	pkg_check_modules(LIBSTATGRAB libstatgrab)
	if(LIBSTATGRAB_FOUND)
		include_directories(${LIBSTATGRAB_INCLUDE_DIRS})
		link_directories(${LIBSTATGRAB_LIBRARY_DIRS})
		add_definitions(${LIBSTATGRAB_CFLAGS_OTHER})
		set(LIBSYSTEM 2)
	endif(LIBSTATGRAB_FOUND)
endif(WITH_LIBSTATGRAB AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR FORCE_LIBSTATGRAB))

# Dmidecode
if(WITH_DMIDECODE)
	add_subdirectory(dmidecode)
	include_directories(dmidecode)
	set(DMIDECODE_LIBRARIES "dmidecode")
endif(WITH_DMIDECODE)

# Bandwidth
if(WITH_BANDWIDTH)
	add_subdirectory(bandwidth)
	if(BANDWIDTH_FOUND)
		include_directories(bandwidth)
		set(BANDWIDTH_LIBRARIES "bandwidth")
	endif(BANDWIDTH_FOUND)
endif(WITH_BANDWIDTH)


### FINAL CONFIG

message("${BoldCyan}** ${CMAKE_PROJECT_NAME} ${PROJECT_VERSION} configuration **${ColourReset}")

# UI libraries
print_config("GTK"                 "${GTK3_VERSION}"        GTK3_FOUND        WITH_GTK)
print_config("NCURSES"             "${NCURSES_VERSION}"     NCURSES_FOUND     WITH_NCURSES)

# Other libraries
print_config("GETTEXT"             0                        GETTEXT_FOUND     WITH_GETTEXT)
print_config("LIBCURL"             "${LIBCURL_VERSION}"     LIBCURL_FOUND     WITH_LIBCURL)
print_config("LIBJSONC"            "${LIBJSONC_VERSION}"    LIBJSONC_FOUND    WITH_LIBJSONC)
print_config("LIBCPUID"            "${LIBCPUID_VERSION}"    LIBCPUID_FOUND    WITH_LIBCPUID)
print_config("LIBPCI"              "${LIBPCI_VERSION}"      LIBPCI_FOUND      WITH_LIBPCI)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT FORCE_LIBSTATGRAB)
	print_config("LIBPROCPS"   "${LIBPROCPS_VERSION}"   LIBPROCPS_FOUND   WITH_LIBPROCPS)
else()
	print_config("LIBSTATGRAB" "${LIBSTATGRAB_VERSION}" LIBSTATGRAB_FOUND WITH_LIBSTATGRAB)
endif()

# Subprojects
print_config("DMIDECODE"           "${DMIDECODE_VERSION}"   1                 WITH_DMIDECODE)
print_config("BANDWIDTH"           "${BANDWIDTH_VERSION}"   BANDWIDTH_FOUND   WITH_BANDWIDTH)


### MAIN BINARY

# Executable
add_executable(${APP_EXEC}
	main.c
	util.c
	cpu-x.h
	core.c
	core.h
	ipc.h
)

add_executable(${DAEMON_EXEC}
	daemon.c
	daemon.h
	ipc.h
	cpu-x.h
)

# Gettext
if(GETTEXT_FOUND)
	add_dependencies(${APP_EXEC} potfiles)
endif(GETTEXT_FOUND)

# OS macro definition
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DOS="linux${BITNESS}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES FreeBSD OR ${CMAKE_SYSTEM_NAME} MATCHES DragonFly)
	add_definitions(-DOS="bsd${BITNESS}")
else()
	message("${BoldYellow}Unknown operating system: ${CMAKE_SYSTEM_NAME}${ColourReset}")
	add_definitions(-DOS="unknown${BITNESS}")
endif()

# Libsystem extra-definitions
if(${LIBSYSTEM} EQUAL 0 OR ${LIBSYSTEM} EQUAL 1)
	add_definitions(-DHAS_LIBSTATGRAB=0 -DLIBSTATGRAB_VERSION=NULL)
endif()
if(${LIBSYSTEM} EQUAL 0 OR ${LIBSYSTEM} EQUAL 2)
	add_definitions(-DHAS_LIBPROCPS=0   -DLIBPROCPS_VERSION=NULL)
endif()

# Various definitions
add_definitions(-DPRGVER="${PROJECT_VERSION}" -DCC="${CMAKE_C_COMPILER_ID}" -DHAS_LIBSYSTEM="${LIBSYSTEM}")
add_definitions(-DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}" -DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}" -DTERMINFODIR="/lib/terminfo")
add_definitions(-DDAEMON_PATH="${DAEMON_PATH}")

# Link libraries
target_link_libraries(${APP_EXEC}
	m
	${Intl_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${Backtrace_LIBRARIES}

	${GTK3_LIBRARIES}
	${NCURSES_LIBRARIES}
	${LIBCURL_LIBRARIES}
	${LIBJSONC_LIBRARIES}
	${LIBCPUID_LIBRARIES}
	${LIBPCI_LIBRARIES}
	${LIBPROCPS_LIBRARIES}
	${LIBSTATGRAB_LIBRARIES}
	${DMIDECODE_LIBRARIES}
	${BANDWIDTH_LIBRARIES}
	${OTHER_LIBRARIES}
)

target_link_libraries(${DAEMON_EXEC}
	${CMAKE_THREAD_LIBS_INIT}

	${LIBCPUID_LIBRARIES}
	${DMIDECODE_LIBRARIES}
)


### INSTALLATION
install(TARGETS ${APP_EXEC}    DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
install(TARGETS ${DAEMON_EXEC} DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/)
