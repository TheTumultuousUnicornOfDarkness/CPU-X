cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(po)


set(absPotFile "${CMAKE_SOURCE_DIR}/po/cpu-x.pot")
FILE(GLOB absPoFiles ${CMAKE_SOURCE_DIR}/po/*.po)

find_package(Gettext)
if(GETTEXT_FOUND)
	# Creation of file cpu-x.pot
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
			cpu-x.c cpu-x.h libsystem.c options.c gui_gtk.c tui_ncurses.c
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
	)

	# Start a new translation
	add_custom_target(newpo
		COMMAND ${CMAKE_COMMAND} -E copy ${absPotFile} NEW_TRANSLATION.po
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/po
	)

	# Update existing translations
	add_custom_target(updatepo)
	set(results)
	foreach(absPoFile ${absPoFiles})
		add_custom_command(TARGET updatepo
			COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --indent --update --backup=existing fr.po ${absPotFile}
			COMMENT "Update po file ${absPoFile}"
		)
		set(results ${results} ${absPoFile})
	endforeach(absPoFile)
	add_custom_command(TARGET updatepo
		DEPENDS ${results}
	)

	# Convert .po to .mo and install
	file(COPY ${absPoFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	foreach(absPoFile ${absPoFiles})
		get_filename_component(poFile ${absPoFile} NAME)
		string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" poFile ${poFile})
		set(poFiles ${poFiles} ${poFile})
	endforeach(absPoFile)
	GETTEXT_PROCESS_POT_FILE(${absPotFile}
		ALL
		INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/share/locale"
		LANGUAGES ${poFiles}
	)

else(GETTEXT_FOUND)
	message("${BoldRed}GETTEXT${ColourReset}${Red} not found. Internationalization support is ${BoldRed}OFF${ColourReset}")
endif(GETTEXT_FOUND)
