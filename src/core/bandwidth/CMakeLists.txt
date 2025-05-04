project(bandwidth
	VERSION "1.5.1"
	LANGUAGES C
)

# Config (bandwidth)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-strict-prototypes -Wno-sign-compare")

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(NASM_FLAGS_DEBUG "-gdwarf")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

# Build (bandwidth)
check_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER)
	enable_language(ASM_NASM)
	set(BANDWIDTH_FOUND ON PARENT_SCOPE)
	message(STATUS "Using built-in ${PROJECT_NAME}, version ${PROJECT_VERSION}")

	if(BITNESS EQUAL 32)
		set(ROUTINES "routines-x86-32bit")
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} ${NASM_FLAGS_DEBUG} -f elf ${ROUTINES}.asm -o ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	elseif(BITNESS EQUAL 64)
		set(ROUTINES "routines-x86-64bit")
		add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			COMMAND ${CMAKE_ASM_NASM_COMPILER} ${NASM_FLAGS_DEBUG} -f elf64 ${ROUTINES}.asm -o ${CMAKE_CURRENT_BINARY_DIR}/${ROUTINES}.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	endif()

	if(LIBCPUID_FOUND AND WITH_LIBCPUID)
		add_definitions(-DHAS_LIBCPUID=1)
	else(LIBCPUID_FOUND AND WITH_LIBCPUID)
		add_definitions(-DHAS_LIBCPUID=0)
	endif(LIBCPUID_FOUND AND WITH_LIBCPUID)

	add_library(bandwidth
		STATIC
		BMP.c
		BMPGraphing.c
		font.c
		main.c
		minifont.c
		libbandwidth.h
		${ROUTINES}.o
	)

	target_link_libraries(bandwidth)
	set(BANDWIDTH_VERSION ${PROJECT_VERSION} PARENT_SCOPE)

else(CMAKE_ASM_NASM_COMPILER)
	set(BANDWIDTH_FOUND OFF PARENT_SCOPE)
endif(CMAKE_ASM_NASM_COMPILER)
