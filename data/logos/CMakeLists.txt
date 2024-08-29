project(logos
	LANGUAGES NONE
)


# Logos
file(GLOB
	logos
	*.png
)

install(FILES
	${logos}
	DESTINATION ${CPU_X_DATA_DIRECTORY}/
)
