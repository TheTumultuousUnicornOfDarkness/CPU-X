cmake_minimum_required(VERSION 2.8)

project(libdmi)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE
	dmidecode
        *.c
)

add_library(dmi-cpux
        STATIC
        ${dmidecode}
)

add_definitions(-DCPUX)

target_link_libraries(dmi-cpux)
