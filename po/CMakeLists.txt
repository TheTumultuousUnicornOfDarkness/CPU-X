cmake_minimum_required(VERSION 3.0)
if(APPLE)
	cmake_policy(SET CMP0025 NEW)
endif(APPLE)

cmake_policy(SET CMP0048 OLD)
project(po)

set(GMO_DIR ${CMAKE_BINARY_DIR}/accomplished/locale/)
set(MSGFMT_EXECUTABLE msgfmt)


# French
execute_process(COMMAND mkdir -p ${GMO_DIR}/fr)

execute_process(COMMAND ${MSGFMT_EXECUTABLE} fr.po -o ${GMO_DIR}/fr/cpu-x.mo
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

install(FILES ${GMO_DIR}/fr/cpu-x.mo
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/locale/fr/LC_MESSAGES/
)
