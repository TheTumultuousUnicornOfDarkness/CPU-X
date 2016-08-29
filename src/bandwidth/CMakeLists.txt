cmake_minimum_required(VERSION 2.8)
set(LOCALVERSION "1.3.1")
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
set(BANDWIDTH_DEBUG_FLAGS "-Wall -Wextra -Wuninitialized -Wno-strict-prototypes -Wno-unused-parameter -Wno-sign-compare -Wno-format-security")
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
		set(ROUTINES "routines-x86-32bit")
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf ${ROUTINES}.asm -o ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	else("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86") # 64-bit build
		set(ROUTINES "routines-x86-64bit")
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf64 ${ROUTINES}.asm -o ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	endif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")

	if(${LIBCPUID_FOUND} AND ${WITH_LIBCPUID})
		add_definitions(-DHAS_LIBCPUID=1)
	else(${LIBCPUID_FOUND} AND ${WITH_LIBCPUID})
		add_definitions(-DHAS_LIBCPUID=0)
	endif(${LIBCPUID_FOUND} AND ${WITH_LIBCPUID})

	add_library(bandwidth
		STATIC
		bandwidth.c
		BMP.c
		BMPGraphing.c
		font.c
		minifont.c
		libbandwidth.h
		${ROUTINES}.o
	)

	target_link_libraries(bandwidth)
	set(BANDWIDTH_VERSION ${PROJECT_VERSION} PARENT_SCOPE)

else(CMAKE_ASM_NASM_INFOMATION_LOADED)
	 set(BANDWIDTH_FOUND OFF PARENT_SCOPE)
	 message("--   Package '${PROJECT_NAME}' not found, NASM compiler not found")
endif(CMAKE_ASM_NASM_INFOMATION_LOADED)
