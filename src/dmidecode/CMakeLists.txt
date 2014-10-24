cmake_minimum_required(VERSION 2.8)

project(libdmi)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

file(GLOB_RECURSE
	dmidecode
        *.c
)

if(EMBED)
add_library(dmi-cpux
        STATIC
        ${dmidecode}
)
else()
add_library(dmi-cpux
        SHARED
        ${dmidecode}
)
endif()

add_definitions(-DCPUX)

target_link_libraries(dmi-cpux)

install(TARGETS dmi-cpux DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
