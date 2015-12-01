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
	set(GETTEXT_FOUND ON PARENT_SCOPE)
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
			core.c core.h libsystem.c main.c gui_gtk.c tui_ncurses.c
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
			COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --indent --update --backup=existing ${absPoFile} ${absPotFile}
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
	add_custom_command(TARGET potfiles_1
		DEPENDS ${poFiles}
	)

	if(PORTABLE_BINARY)
		# Convert .gmo in a header file
		add_custom_target(convertgmo ALL)
		set(vars)
		set(moHeader "${CMAKE_SOURCE_DIR}/po/mo.h")
		add_custom_command(TARGET convertgmo
			COMMAND ${CMAKE_COMMAND} -E remove ${moHeader}
		)
		foreach(poFile ${poFiles})
			add_custom_command(TARGET convertgmo
				COMMAND xxd -i "${poFile}.gmo" >> ${moHeader}
				WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
				COMMENT "Convert file ${poFile}.gmo in header"
				DEPENDS ${poFiles}
			)
			set(vars ${vars} "${poFile}_gmo")
		endforeach(poFile)

		# Add variables 'lang', 'ptrlang' and 'ptrlen' in header
		set(lang	'char *lang[] = { ')
		set(ptrlang	'unsigned char *ptrlang[] = { ')
		set(ptrlen	'unsigned int *ptrlen[] = { ')
		foreach(poFile ${poFiles})
			set(lang	${lang}		'\"${poFile}\", ')
			set(ptrlang	${ptrlang}	'${poFile}_gmo, ')
			set(ptrlen	${ptrlen}	'&${poFile}_gmo_len, ')
		endforeach(poFile)
		set(lang	${lang}		'\"\" }\\\;')
		set(ptrlang	${ptrlang}	'NULL }\\\;')
		set(ptrlen	${ptrlen}	'NULL }\\\;')
		add_custom_command(TARGET convertgmo
			COMMAND ${CMAKE_COMMAND} -E echo ${lang} >> ${moHeader}
			COMMAND ${CMAKE_COMMAND} -E echo ${ptrlang} >> ${moHeader}
			COMMAND ${CMAKE_COMMAND} -E echo ${ptrlen} >> ${moHeader}
			DEPENDS ${lang} ${ptrlang} ${ptrlen})
		add_dependencies(convertgmo potfiles)
	endif(PORTABLE_BINARY)

else(GETTEXT_FOUND)
	set(GETTEXT_FOUND OFF PARENT_SCOPE)
endif(GETTEXT_FOUND)
