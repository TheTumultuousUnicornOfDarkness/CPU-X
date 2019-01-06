project(logos
	LANGUAGES NONE
)


# Logos
install(FILES
	AMD.png
	Centaur.png
	Cyrix.png
	Intel.png
	NexGen.png
	NSC.png
	Rise.png
	SiS.png
	Transmeta.png
	UMC.png
	Unknown.png
	DESTINATION ${CPU_X_DATA_DIRECTORY}/
)
