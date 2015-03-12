cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(po)

set(GMO_DIR ${CMAKE_BINARY_DIR}/accomplished/locale)

execute_process(COMMAND which msgfmt
	 OUTPUT_VARIABLE MSGFMT_EXECUTABLE
)
if(MSGFMT_EXECUTABLE STREQUAL "")
	 execute_process(COMMAND find /usr/local -name msgfmt
		OUTPUT_VARIABLE MSGFMT_EXECUTABLE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif(MSGFMT_EXECUTABLE STREQUAL "")


# French
execute_process(COMMAND mkdir -p ${GMO_DIR}/fr)

execute_process(COMMAND ${MSGFMT_EXECUTABLE} fr.po -o ${GMO_DIR}/fr/cpu-x.mo
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

install(FILES ${GMO_DIR}/fr/cpu-x.mo
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/locale/fr/LC_MESSAGES/
)
