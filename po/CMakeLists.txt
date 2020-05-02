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
			--language=C
			--keyword=_ --keyword=N_ --indent
			--add-comments=TRANSLATORS:
			--package-name=${CMAKE_PROJECT_NAME}
			--package-version=${PROJECT_VERSION}
			--default-domain=${CMAKE_PROJECT_NAME}
			--msgid-bugs-address=https://github.com/X0rg/CPU-X/issues
			--copyright-holder=Xorg
			--output-dir=${CMAKE_CURRENT_SOURCE_DIR}
			--output=${pot_file}
			--directory="../src"
			--from-code="utf-8"
			"*.c" "cpu-x.h"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
	)
	GETTEXT_CONVERT_TRANSLATIONS(${pot_file} ALL ${po_files})
else(GETTEXT_FOUND)
	set(GETTEXT_FOUND OFF PARENT_SCOPE)
endif(GETTEXT_FOUND)
