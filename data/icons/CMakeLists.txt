cmake_minimum_required(VERSION 2.8)
if(${CMAKE_VERSION} VERSION_GREATER "2.9")
	cmake_policy(SET CMP0048 OLD)
endif(${CMAKE_VERSION} VERSION_GREATER "2.9")
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
		DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/${size}/apps/
		RENAME cpu-x.png
	)
endforeach()

install(FILES
	CPU-X_original.png
	DESTINATION ${CPU_X_DATA_DIRECTORY}/
	RENAME CPU-X.png
)
