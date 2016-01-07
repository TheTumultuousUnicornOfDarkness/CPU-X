cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
project(dmidecode
	VERSION 3.0
	LANGUAGES C
)

# Config (dmidecode)
message("-- Checking for submodule '${PROJECT_NAME}'")
message("--   Found ${PROJECT_NAME}, version ${PROJECT_VERSION}")

# Build (dmidecode)
add_library(dmidecode
        STATIC
	types.h
	util.c
	util.h
	dmiopt.h
	dmioem.c
	dmioem.h
	dmidecode.c
	dmidecode.h
	libdmi.h
)

target_link_libraries(dmidecode)
set(DMIDECODE_VERSION ${PROJECT_VERSION} PARENT_SCOPE)
