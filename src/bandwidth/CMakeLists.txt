cmake_minimum_required(VERSION 2.8)
set(LOCALVERSION "1.1")
if(${CMAKE_VERSION} VERSION_GREATER "2.9")
	cmake_policy(SET CMP0048 NEW)
	project(bandwidth
		VERSION ${LOCALVERSION}
		LANGUAGES C
	)
else(${CMAKE_VERSION} VERSION_GREATER "2.9")
	project(bandwidth)
	set(PROJECT_VERSION ${LOCALVERSION})
endif(${CMAKE_VERSION} VERSION_GREATER "2.9")

# Config (bandwidth)
set(BANDWIDTH_DEBUG_FLAGS "-Wall -Wextra -Wuninitialized -Wno-strict-prototypes -Wno-unused-parameter -Wno-sign-compare")
set(CMAKE_C_FLAGS_DEBUG	  "${CMAKE_C_FLAGS_DEBUG}   ${BANDWIDTH_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${BANDWIDTH_DEBUG_FLAGS}")
message("-- Checking for submodule '${PROJECT_NAME}'")

find_program(NASM_FOUND nasm)
if(NASM_FOUND)
	enable_language(ASM_NASM)
endif(NASM_FOUND)

# Build (bandwidth)
if(CMAKE_ASM_NASM_INFOMATION_LOADED)
	set(BANDWIDTH_FOUND ON PARENT_SCOPE)
	message("--   Found ${PROJECT_NAME}, version ${PROJECT_VERSION}")

	if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86") # 32-bit build
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/routines32.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf routines32.asm -o ${CMAKE_CURRENT_BINARY_DIR}/routines32.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
		set(ROUTINES "routines32.o")
	else("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86") # 64-bit build
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/routines64.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf64 routines64.asm -o ${CMAKE_CURRENT_BINARY_DIR}/routines64.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
		set(ROUTINES "routines64.o")
	endif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")

	add_library(bandwidth
		STATIC
		bandwidth.c
		BMP.c
		BMPGraphing.c
		font.c
		minifont.c
		libbandwidth.h
		${ROUTINES}
	)

	target_link_libraries(bandwidth)
	set(BANDWIDTH_VERSION ${PROJECT_VERSION} PARENT_SCOPE)

else(CMAKE_ASM_NASM_INFOMATION_LOADED)
	 set(BANDWIDTH_FOUND OFF PARENT_SCOPE)
	 message("--   Package '${PROJECT_NAME}' not found, NASM compiler not found")
endif(CMAKE_ASM_NASM_INFOMATION_LOADED)
