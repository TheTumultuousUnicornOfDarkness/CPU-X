project(po
	LANGUAGES NONE
)


### INTERNATIONALIZATION TARGETS

# PO files
set(pot_file ${CMAKE_PROJECT_NAME}.pot)
file(GLOB
	po_files
	*.po
)

if(GETTEXT_FOUND)
	set(GETTEXT_FOUND ON PARENT_SCOPE)
	include(gettext_create_translations)
	add_custom_target(genpot
		COMMAND xgettext
			--language=C++
			--keyword=_ --keyword=N_ --indent
			--add-comments=TRANSLATORS:
			--package-name=${CMAKE_PROJECT_NAME}
			--package-version=${PROJECT_VERSION}
			--default-domain=${CMAKE_PROJECT_NAME}
			--msgid-bugs-address=https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/issues
			--copyright-holder=TheTumultuousUnicornOfDarkness
			--output-dir=${CMAKE_CURRENT_BINARY_DIR}
			--output=source_c.pot
			--directory="../src"
			--from-code="utf-8"
			"*.cpp" "data.hpp" "util.hpp"
		COMMAND xgettext
			--language=Glade
			--keyword=translatable --indent
			--add-comments
			--package-name=${CMAKE_PROJECT_NAME}
			--package-version=${PROJECT_VERSION}
			--default-domain=${CMAKE_PROJECT_NAME}
			--msgid-bugs-address=https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/issues
			--copyright-holder=TheTumultuousUnicornOfDarkness
			--output-dir=${CMAKE_CURRENT_BINARY_DIR}
			--output=source_ui.pot
			--directory="../data"
			--from-code="utf-8"
			"cpu-x-gtk-3.12.ui"
		COMMAND	msgcat --indent --directory=${CMAKE_CURRENT_BINARY_DIR} source_c.pot source_ui.pot --output-file="${CMAKE_CURRENT_SOURCE_DIR}/${pot_file}"
		COMMAND sed --in-place "s/SOME DESCRIPTIVE TITLE./CPU-X translations template/" "${CMAKE_CURRENT_SOURCE_DIR}/${pot_file}"
		COMMAND sed --in-place "s/\\(C\\) YEAR/© 2014-2023/"  "${CMAKE_CURRENT_SOURCE_DIR}/${pot_file}"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
	)
	GETTEXT_CONVERT_TRANSLATIONS(${pot_file} ALL ${po_files})
else(GETTEXT_FOUND)
	set(GETTEXT_FOUND OFF PARENT_SCOPE)
endif(GETTEXT_FOUND)
