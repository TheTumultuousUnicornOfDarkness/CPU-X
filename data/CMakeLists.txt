cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(data)


# Files needed by GTK GUI
if(WITH_GTK AND NOT PORTABLE_BINARY)
	# Set config
	file(MAKE_DIRECTORY ${DATA_DIR})
	set(EXEC ${CMAKE_INSTALL_FULL_BINDIR}/${CMAKE_PROJECT_NAME})
	set(TRYEXEC ${EXEC})
	configure_file(cpu-x.desktop.in ${DATA_DIR}/cpu-x.desktop)
	configure_file(org.pkexec.cpu-x.policy.in ${DATA_DIR}/org.pkexec.cpu-x.policy)
	configure_file(cpu-x_polkit.in ${DATA_DIR}/cpu-x_polkit)

	set(EXEC ${EXEC}_polkit)
	configure_file(cpu-x-root.desktop.in ${DATA_DIR}/cpu-x-root.desktop)

	# Install pictures + UIs
	file(GLOB
		pictures
		pictures/*.png
	)

	install(FILES
		cpux-gtk-3.8.ui
		cpux-gtk-3.14.ui
		${pictures}
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/
	)

	install(FILES
		pictures/CPU-X.png
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/pixmaps/
		RENAME cpu-x.png
	)

	# Install launchers
	install(FILES
		${DATA_DIR}/cpu-x.desktop
		${DATA_DIR}/cpu-x-root.desktop
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/
	)

	# Install policy rule
	install(FILES
		${DATA_DIR}/org.pkexec.cpu-x.policy
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/polkit-1/actions
	)

	# Install binary which can run CPU-X as root via Polkit
	install(FILES
		${DATA_DIR}/cpu-x_polkit
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
endif(WITH_GTK AND NOT PORTABLE_BINARY)
