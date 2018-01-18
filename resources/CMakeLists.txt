
# Desktop entry
configure_file(strata.cmake.desktop strata.desktop @ONLY)

if (UNIX)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/strata.desktop
        DESTINATION .
        )
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/images/application-icon.svg
        DESTINATION .
        RENAME strata.svg
        )
endif()
