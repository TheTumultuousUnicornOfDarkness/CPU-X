cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 NEW)
project(libbandwidth
	VERSION 1.1
	LANGUAGES C ASM
)

# Print libbandwidth version
message("-- ${PROJECT_NAME}, version ${PROJECT_VERSION}")


# Compile statically bandwidth
enable_language(ASM_NASM)
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
	add_library(bandwidth
		STATIC
		main.c
		defs.h
		routines32.asm
	)
else("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
	add_library(bandwidth
		STATIC
		main.c
		defs.h
		routines64.asm
	)
endif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")

add_definitions(-DCPUX)

target_link_libraries(bandwidth)
