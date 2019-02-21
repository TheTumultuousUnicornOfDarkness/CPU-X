project(tests
	LANGUAGES NONE
)

add_test(NAME test_regex COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/awk/test_regex.sh")
