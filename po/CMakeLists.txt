cmake_minimum_required(VERSION 2.8)
if(APPLE)
  cmake_policy(SET CMP0025 NEW)
endif(APPLE)

project(po)

set(_GMO_DIR ${CMAKE_BINARY_DIR})
set(MSGFMT_EXECUTABLE msgfmt)

execute_process(COMMAND ${MSGFMT_EXECUTABLE} -o ${_GMO_DIR}/cpu-x.mo fr.po
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

install(FILES ${_GMO_DIR}/cpu-x.mo
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/locale/fr/LC_MESSAGES/
)
