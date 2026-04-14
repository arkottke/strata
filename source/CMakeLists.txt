## #
#Copyright(c) 2009, Paul Gideon Dann
#
#Permission to use, copy, modify, and/or distribute this software for any
#purpose with or without fee is hereby granted, provided that the above
#copyright notice and this permission notice appear in all copies.
#
#THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
## #

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/defines.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/defines.h
    )

file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE CODE_FILES *.cpp)

qt6_wrap_ui(UI_HEADERS ${UI_FILES})
qt6_add_resources(RESOURCE_FILES ../resources/resources.qrc)

#Windows application icon
if (WIN32)
    set(WINDOWS_RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/resources.obj)
    if (MSVC)
        add_custom_command(
            OUTPUT ${WINDOWS_RES_FILE}
            COMMAND rc.exe /fo ${WINDOWS_RES_FILE} resources.rc
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/win)
    else ()
        add_custom_command(
            OUTPUT ${WINDOWS_RES_FILE}
            COMMAND windres.exe resources.rc ${WINDOWS_RES_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/win)
    endif ()
endif ()

qt_add_executable(${CMAKE_PROJECT_NAME}
    ${OS_BUNDLE}
    ${UI_HEADERS}
    ${CODE_FILES}
    ${RESOURCE_FILES}
    ${WINDOWS_RES_FILE}
    )
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
    Qt6::OpenGLWidgets
    Qt6::PrintSupport
    Qt6::Widgets
    Qt6::Xml
    ${LIBS}
    )
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if (UNIX AND NOT APPLE)
    install(TARGETS ${CMAKE_PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT executable
        )
    install(FILES
        ${CMAKE_BINARY_DIR}/resources/strata.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
        COMPONENT executable
        )
    install(FILES
        ${CMAKE_SOURCE_DIR}/resources/images/application-icon.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        COMPONENT executable
        RENAME strata.svg
        )
    # Manual and example
    install(FILES
        ${CMAKE_SOURCE_DIR}/manual/manual.pdf
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/doc
        COMPONENT manual
        RENAME manual.pdf
    )

    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/example
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}
        COMPONENT example
    )
elseif (WIN32)
    install(TARGETS ${CMAKE_PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT executable
        )
    # Manual and example
    install(FILES
        ${CMAKE_SOURCE_DIR}/manual/manual.pdf
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/doc
        COMPONENT manual
        RENAME manual.pdf
        )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/example
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}
        COMPONENT example
        )
elseif (APPLE)
    install(TARGETS ${CMAKE_PROJECT_NAME}
        BUNDLE DESTINATION .
        COMPONENT executable
        )
endif ()

# Deploy Qt runtime dependencies at install time.
# OUTPUT_SCRIPT and NO_TRANSLATIONS require Qt 6.5+; Qt 6.3-6.4 uses
# FILENAME_VARIABLE. On older Qt (e.g., Ubuntu 22.04's Qt 6.2) the command
# does not exist and platform-specific tools handle deployment instead.
if(COMMAND qt_generate_deploy_app_script)
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.5")
        qt_generate_deploy_app_script(
            TARGET ${CMAKE_PROJECT_NAME}
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
            NO_TRANSLATIONS
        )
    else()
        qt_generate_deploy_app_script(
            TARGET ${CMAKE_PROJECT_NAME}
            FILENAME_VARIABLE deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
    endif()
    install(SCRIPT ${deploy_script})
endif()
