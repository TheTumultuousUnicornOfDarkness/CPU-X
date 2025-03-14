project(data
	LANGUAGES NONE
)


add_subdirectory(completions)

# Files needed by GTK GUI
if(WITH_GTK)
	include(GSettings)

	# Set config
	file(MAKE_DIRECTORY ${CMAKE_DATA_OUTPUT_DIRECTORY})
	if(GETTEXT_FOUND)
		set(TMP_DESKTOP_FILE_FILENAME    "${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x.desktop.in")
		set(OUTPUT_DESKTOP_FILE_FILENAME "${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x.desktop")
		add_custom_command(
			OUTPUT "${OUTPUT_DESKTOP_FILE_FILENAME}"
			DEPENDS "${TMP_DESKTOP_FILE_FILENAME}"
			COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
				--desktop
				--template="${TMP_DESKTOP_FILE_FILENAME}"
				-d "${CMAKE_SOURCE_DIR}/po"
				--output-file="${OUTPUT_DESKTOP_FILE_FILENAME}"
		)
		add_custom_target (
			gendesktop ALL DEPENDS
			"${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x.desktop"
		)
	else()
		set(TMP_DESKTOP_FILE_FILENAME "${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x.desktop")
	endif(GETTEXT_FOUND)
	configure_file(io.github.thetumultuousunicornofdarkness.cpu-x.desktop.in       ${TMP_DESKTOP_FILE_FILENAME})
	configure_file(io.github.thetumultuousunicornofdarkness.cpu-x-daemon.policy.in ${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x-daemon.policy)

	# Install UIs
	install(FILES
		cpu-x-gtk-3.12.ui
		cpu-x-gtk-3.12.css
		cpu-x-gtk-3.12-dark.css
		cpu-x-gtk-3.20.css
		cpu-x-gtk-3.20-dark.css
		DESTINATION ${CPU_X_DATA_DIRECTORY}/
	)

	# AppData metainfo file
	install(FILES
		io.github.thetumultuousunicornofdarkness.cpu-x.appdata.xml
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo/
	)

	# Install launcher
	install(FILES
		${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x.desktop
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/
	)

	# Install policy rule
	pkg_get_variable(POLKIT_ACTION_DIR polkit-gobject-1 actiondir)
	if(POLKIT_ACTION_DIR STREQUAL "")
		set(POLKIT_ACTION_DIR "${CMAKE_INSTALL_FULL_DATADIR}/polkit-1/actions/")
	endif(POLKIT_ACTION_DIR STREQUAL "")
	install(FILES
		${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x-daemon.policy
		DESTINATION ${POLKIT_ACTION_DIR}
	)

	# Install schema file
	add_schema("io.github.thetumultuousunicornofdarkness.cpu-x.gschema.xml")

	add_subdirectory(icons)
	add_subdirectory(logos)
endif(WITH_GTK)
