project(completions
	LANGUAGES NONE
)

install(FILES
	bash
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/bash-completion/completions/
	RENAME ${CMAKE_PROJECT_NAME}
)

install(FILES
	fish
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/fish/vendor_completions.d/
	RENAME ${CMAKE_PROJECT_NAME}.fish
)

install(FILES
	zsh
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/zsh/site-functions/
	RENAME _${CMAKE_PROJECT_NAME}
)
