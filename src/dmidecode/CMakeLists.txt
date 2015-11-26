cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 NEW)
project(dmidecode
	VERSION 3.0
	LANGUAGES C
)

# Print libdmi version
message("-- Checking for submodule '${PROJECT_NAME}'")
message("--   Found ${PROJECT_NAME}, version ${PROJECT_VERSION}")

# Compile statically dmi
add_library(dmi
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
set(DMIDECODE_VERSION ${PROJECT_VERSION} PARENT_SCOPE)

target_link_libraries(dmi)
