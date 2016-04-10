cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 OLD)
project(data)


# Files needed by GTK GUI
if(WITH_GTK AND NOT PORTABLE_BINARY)
	# Set config
	file(MAKE_DIRECTORY ${CMAKE_DATA_OUTPUT_DIRECTORY})
	set(EXEC    ${CMAKE_INSTALL_FULL_BINDIR}/${CMAKE_PROJECT_NAME})
	set(TRYEXEC ${EXEC})
	configure_file(cpu-x.desktop.in           ${CMAKE_DATA_OUTPUT_DIRECTORY}/cpu-x.desktop)
	configure_file(org.pkexec.cpu-x.policy.in ${CMAKE_DATA_OUTPUT_DIRECTORY}/org.pkexec.cpu-x.policy)
	configure_file(cpu-x_polkit.in            ${CMAKE_DATA_OUTPUT_DIRECTORY}/cpu-x_polkit)

	set(EXEC ${EXEC}_polkit)
	configure_file(cpu-x-root.desktop.in ${CMAKE_DATA_OUTPUT_DIRECTORY}/cpu-x-root.desktop)

	# Install UIs
	install(FILES
		cpu-x-gtk-3.8.ui
		cpu-x-gtk-3.16.ui
		cpu-x-gtk-3.16.css
		cpu-x-gtk-3.20.css
		DESTINATION ${CPU_X_DATA_DIRECTORY}/
	)

	# Install launchers
	install(FILES
		${CMAKE_DATA_OUTPUT_DIRECTORY}/cpu-x.desktop
		${CMAKE_DATA_OUTPUT_DIRECTORY}/cpu-x-root.desktop
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/
	)

	# Install policy rule
	install(FILES
		${CMAKE_DATA_OUTPUT_DIRECTORY}/org.pkexec.cpu-x.policy
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/polkit-1/actions/
	)

	# Install binary which can run CPU-X as root via Polkit
	install(FILES
		${CMAKE_DATA_OUTPUT_DIRECTORY}/cpu-x_polkit
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)

	add_subdirectory(icons)
	add_subdirectory(logos)
endif(WITH_GTK AND NOT PORTABLE_BINARY)
