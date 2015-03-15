cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(po)

set(GMO_DIR ${CMAKE_BINARY_DIR}/accomplished/locale)

execute_process(COMMAND which msgfmt
	 OUTPUT_VARIABLE MSGFMT_EXECUTABLE
	 OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(MSGFMT_EXECUTABLE STREQUAL "")
	 execute_process(COMMAND find /usr/local -name msgfmt
		OUTPUT_VARIABLE MSGFMT_EXECUTABLE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif(MSGFMT_EXECUTABLE STREQUAL "")


# French
file(MAKE_DIRECTORY ${GMO_DIR}/fr)

execute_process(COMMAND ${MSGFMT_EXECUTABLE} fr.po -o ${GMO_DIR}/fr/cpu-x.mo
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

install(FILES ${GMO_DIR}/fr/cpu-x.mo
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/locale/fr/LC_MESSAGES/
)

add_custom_target(genpot
	COMMAND xgettext
		--language=C
		--package-name=${CMAKE_PROJECT_NAME}
		--package-version=${PROJECT_VERSION}
		--default-domain=${CMAKE_PROJECT_NAME}
		--msgid-bugs-address=https://github.com/X0rg/CPU-X/issues
		--copyright-holder=Xorg
		--output-dir=${CMAKE_SOURCE_DIR}/po
		--output=${CMAKE_PROJECT_NAME}.pot
		--keyword=_ --indent
		cpu-x.c cpu-x.h options.c gui_gtk.c tui_ncurses.c
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
)

add_custom_target(updatepo
	COMMAND msgmerge --indent --update --backup=existing fr.po cpu-x.pot
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
