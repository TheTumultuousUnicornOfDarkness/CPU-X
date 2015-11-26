cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 NEW)
project(libbandwidth
	VERSION 1.1
	LANGUAGES C ASM
)

# Compile statically bandwidth
enable_language(ASM_NASM OPTIONAL)
if(NOT ${CMAKE_ASM_NASM_COMPILER} STREQUAL "CMAKE_ASM_NASM_COMPILER-NOTFOUND")
	message("-- ${PROJECT_NAME}, version ${PROJECT_VERSION}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}		-Wno-strict-prototypes -Wno-sign-compare -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}	-Wno-strict-prototypes -Wno-sign-compare -Wno-unused-variable")
	set(LIBBDWT_FOUND ON PARENT_SCOPE)
	file(COPY routines32.asm routines64.asm DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

	if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
		add_custom_command(OUTPUT routines32.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf routines32.asm -o routines32.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
		add_library(bandwidth
			STATIC
			main.c
			defs.h
			routines32.o
		)
	else("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
		add_custom_command(OUTPUT routines64.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf64 routines64.asm -o routines64.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
		add_library(bandwidth
			STATIC
			main.c
			defs.h
			routines64.o
		)
	endif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")

	add_definitions(-DCPUX)

	target_link_libraries(bandwidth)
endif()
