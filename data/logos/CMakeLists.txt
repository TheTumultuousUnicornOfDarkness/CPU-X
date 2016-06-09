cmake_minimum_required(VERSION 2.8)
if(${CMAKE_VERSION} VERSION_GREATER "2.9")
	cmake_policy(SET CMP0048 OLD)
endif(${CMAKE_VERSION} VERSION_GREATER "2.9")
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
