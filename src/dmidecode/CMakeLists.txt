project(dmidecode
	VERSION "3.2.20190129"
	LANGUAGES C
)

# Config (dmidecode)
set(DMIDECODE_DEBUG_FLAGS "-Wall -Wextra -Wuninitialized -Wno-shift-count-overflow")
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${DMIDECODE_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DMIDECODE_DEBUG_FLAGS}")
message(STATUS "Using built-in ${PROJECT_NAME}, version ${PROJECT_VERSION}")


# Build (dmidecode)
add_definitions(-DVERSION="${PROJECT_VERSION}")
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
