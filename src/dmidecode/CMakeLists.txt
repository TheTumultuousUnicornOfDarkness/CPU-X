cmake_minimum_required(VERSION 2.8)
if(APPLE)
  cmake_policy(SET CMP0025 NEW)
endif(APPLE)

project(libdmi)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(dmi-cpux
        STATIC
	types.h
	util.c
	util.h
	dmiopt.h
	dmioem.c
	dmioem.h
	dmidecode.c
	dmidecode.h
	libdmi.c
	libdmi.h
)

add_definitions(-DCPUX)

target_link_libraries(dmi-cpux)
