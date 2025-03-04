project(data
	LANGUAGES NONE
)


add_subdirectory(completions)

# Files needed by GTK GUI
if(WITH_GTK)
	include(GSettings)

	# Set config
	file(MAKE_DIRECTORY ${CMAKE_DATA_OUTPUT_DIRECTORY})
	configure_file(io.github.thetumultuousunicornofdarkness.cpu-x.desktop.in       ${CMAKE_DATA_OUTPUT_DIRECTORY}/io.github.thetumultuousunicornofdarkness.cpu-x.desktop)
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
