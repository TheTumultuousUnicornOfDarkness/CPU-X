project(tests
	LANGUAGES NONE
)

add_test(NAME test_awk_regex  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/awk/test_regex.sh")
add_test(NAME test_grep_regex COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/grep/test_regex.sh")
