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
		if(EMBED AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			set(NCURSES_LIBRARIES -l:libncursesw.a)
		elseif(EMBED AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			set(NCURSES_LIBRARIES -l:libncursesw.a -l:libtinfo.a)
		endif(EMBED AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
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
		if(${LIBCPUID_VERSION} VERSION_GREATER "0.2.1")
			add_definitions(${LIBCPUID_CFLAGS_OTHER} -DHAVE_LIBCPUID_0_2_2)
		endif(${LIBCPUID_VERSION} VERSION_GREATER "0.2.1")
	endif(LIBCPUID_FOUND)
endif(WITH_LIBCPUID)

# Dmidecode lib
if(WITH_LIBDMI)
	add_subdirectory(dmidecode)
	include_directories(dmidecode)
	add_definitions(-DLIBDMI)
	set(LIBDMI_LIBRARY "dmi")
endif(WITH_LIBDMI)

# Bandwidth lib
if(WITH_LIBBDWT)
	set(LIBBDWT_FOUND OFF)
	add_subdirectory(bandwidth)
	if(LIBBDWT_FOUND)
		include_directories(bandwidth)
		add_definitions(-DLIBBDWT)
		set(LIBBDWT_LIBRARY "bandwidth")
		set(LIBBDWT_FOUND ON PARENT_SCOPE)
	endif(LIBBDWT_FOUND)
endif(WITH_LIBBDWT)

# Pciutils lib
if(WITH_LIBPCI)
	pkg_check_modules(LIBPCI libpci)
	if(LIBPCI_FOUND)
		include_directories(${LIBPCI_INCLUDE_DIRS})
		link_directories(${LIBPCI_LIBRARY_DIRS})
		add_definitions(${LIBPCI_CFLAGS_OTHER} -DLIBPCI)
		if(EMBED AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			set(LIBPCI_LIBRARIES -l:libpci.a -lresolv -ludev)
		elseif(EMBED AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			set(LIBPCI_LIBRARIES -l:libpci.a -lz)
		endif(EMBED AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
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
			set(LIBSYSTEM_FOUND ON PARENT_SCOPE)
		endif(LIBPROCPS_FOUND)

	else(${CMAKE_SYSTEM_NAME} MATCHES "Linux") # Others: Libstatgrab
		pkg_check_modules(LIBSTATGRAB libstatgrab)
		if(LIBSTATGRAB_FOUND)
			include_directories(${LIBSTATGRAB_INCLUDE_DIRS})
			link_directories(${LIBSTATGRAB_LIBRARY_DIRS})
			add_definitions(${LIBSTATGRAB_CFLAGS_OTHER} -DLIBSTATGRAB)
			if(EMBED)
				set(LIBSTATGRAB_LIBRARIES -l:libstatgrab.a -ldevstat)
			endif(EMBED)
			set(LIBSYSTEM_FOUND ON PARENT_SCOPE)
		endif(LIBSTATGRAB_FOUND)
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endif(WITH_LIBSYSTEM)


### Final config
macro(print_config libname foundstate withstate)
	if(${foundstate} AND ${withstate})
		if(${foundstate} EQUAL 2)
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldGreen}enable${ColourReset} (built-in)")
		else(${foundstate} EQUAL 2)
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldGreen}enable${ColourReset}")
		endif(${foundstate} EQUAL 2)
		add_definitions(-DHAS_${libname}=1)
	else(${foundstate} AND ${withstate})
		if(NOT ${withstate})
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldRed}disable${ColourReset} (explicitly disable)")
		elseif(NOT ${foundstate})
			message("${BoldCyan}**   ${libname}${ColourReset}\tsupport is ${BoldRed}disable${ColourReset} (not found by pkg-config)")
		endif()
		add_definitions(-DHAS_${libname}=0)
	endif(${foundstate} AND ${withstate})
endmacro(print_config)

message("${BoldCyan}** ${CMAKE_PROJECT_NAME} ${PROJECT_VERSION} configuration **${ColourReset}")
# UI librairies
print_config("GTK" GTK3_FOUND WITH_GTK)
print_config("NCURSES" NCURSES_FOUND WITH_NCURSES)

# Others librairies
print_config("LIBCPUID" LIBCPUID_FOUND WITH_LIBCPUID)
print_config("LIBPCI" LIBPCI_FOUND WITH_LIBPCI)
print_config("LIBSYSTEM" LIBSYSTEM_FOUND WITH_LIBSYSTEM)

# Subprojects
print_config("DMIDECODE" 2 WITH_LIBDMI)
print_config("BANDWIDTH" 2 WITH_LIBBDWT)


### Main executable

add_executable(cpu-x
	main.c
	core.c
	core.h
	libsystem.c
)

if(EMBED)
	add_definitions(-DEMBED)
	if(WITH_GETTEXT)
		set(CMAKE_INSTALL_FULL_LOCALEDIR "/tmp/.cpu-x")
		add_dependencies(cpu-x convertgmo)
		add_definitions(-DGETTEXT)
	endif(WITH_GETTEXT)
elseif(NOT EMBED AND WITH_GETTEXT)
	add_dependencies(cpu-x potfiles)
endif(EMBED)

add_definitions(-DPRGVER="${PROJECT_VERSION}" -DGETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}" -DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")
set(ADDITIONNAL_LIBRARIES "-lm")

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(ADDITIONNAL_LIBRARIES "${ADDITIONNAL_LIBRARIES} -lintl")
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

target_link_libraries(cpu-x
	${GUI_GTK_LIBRARY}
	${TUI_NCURSES_LIBRARY}
	${LIBCPUID_LIBRARIES}
	${LIBDMI_LIBRARY}
	${LIBBDWT_LIBRARY}
	${LIBPCI_LIBRARIES}
	${LIBPROCPS_LIBRARIES}
	${LIBSTATGRAB_LIBRARIES}
	${ADDITIONNAL_LIBRARIES}
)


# Installation
install(TARGETS cpu-x DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
