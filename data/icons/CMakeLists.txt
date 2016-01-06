cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 OLD)
project(icons)


# Icons
file(GLOB
	icons
	CPU-X_*x*.png
)

foreach(icon ${icons})
	get_filename_component(filename ${icon} NAME_WE)
	string(REGEX REPLACE "CPU-X_" "" size "${filename}")
	install(FILES
		${icon}
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/${size}/apps
		RENAME cpu-x.png
	)
endforeach()

install(FILES
	CPU-X_original.png
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/
	RENAME CPU-X.png
)
