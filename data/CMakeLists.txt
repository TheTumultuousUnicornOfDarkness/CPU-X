cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(data)


# Installation of files needed by GTK GUI

if(WITH_GTK)
	file(GLOB_RECURSE
		pictures
		pictures/*.png
	)

	install(FILES
		cpux-gtk-3.8.ui
		${pictures}
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/
	)

	install(FILES
		cpu-x.desktop
		cpu-x-root.desktop
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/
	)

	install(FILES
		org.pkexec.cpu-x.policy
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/polkit-1/actions
	)

	install(FILES
		../cpu-x_polkit
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)

	install(FILES
		pictures/CPU-X.png
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/pixmaps/
		RENAME cpu-x.png
	)

endif(WITH_GTK)
