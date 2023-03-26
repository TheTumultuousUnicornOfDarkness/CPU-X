project(dmidecode
	VERSION "3.5.20230314"
	LANGUAGES C
)

# Config (dmidecode)
set(CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-shift-count-overflow")
message(STATUS "Using built-in ${PROJECT_NAME}, version ${PROJECT_VERSION}")

# Build (dmidecode)
add_definitions(-DVERSION="${PROJECT_VERSION}")
add_library(dmidecode
	STATIC
	types.h
	util.c
	util.h
	dmiopt.h
	dmiopt.c
	dmioem.c
	dmioem.h
	dmidecode.c
	dmidecode.h
	dmioutput.c
	dmioutput.h
)

target_link_libraries(dmidecode)
set(DMIDECODE_VERSION ${PROJECT_VERSION} PARENT_SCOPE)
