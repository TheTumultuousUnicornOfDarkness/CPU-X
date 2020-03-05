project(po
	LANGUAGES NONE
)


### INTERNATIONALIZATION TARGETS

# PO files
file(GLOB
	po_files
	*.po
)

if(GETTEXT_FOUND)
	set(GETTEXT_FOUND ON PARENT_SCOPE)
	foreach(po_file ${po_files})
		get_filename_component(lang ${po_file} NAME_WE)
		GETTEXT_PROCESS_PO_FILES(${lang} ALL INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/share/locale PO_FILES ${po_file})
	endforeach()
else(GETTEXT_FOUND)
	set(GETTEXT_FOUND OFF PARENT_SCOPE)
endif(GETTEXT_FOUND)
