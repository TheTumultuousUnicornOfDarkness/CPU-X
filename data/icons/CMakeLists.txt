project(icons
	LANGUAGES NONE
)


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
		RENAME com.github.thetumultuousunicornofdarkness.cpu-x.png
	)
endforeach()

install(FILES
	CPU-X_original.png
	DESTINATION ${CPU_X_DATA_DIRECTORY}/
	RENAME CPU-X.png
)
