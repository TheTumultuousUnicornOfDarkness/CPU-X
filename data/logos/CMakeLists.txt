cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 OLD)
project(logos)


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
