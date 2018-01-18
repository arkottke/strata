###
# Copyright (c) 2009, Paul Gideon Dann
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
###

# Ensure that WiX is in the path
find_path(WIX_PATH candle.exe)
if (NOT WIX_PATH)
    message(WARNING "Unable to find WiX in the PATH. The WiX installer will be disabled.")
endif()

# Ensure that 7z is in the path
find_path(7Z_PATH 7z.exe)
if (NOT 7Z_PATH)
    message(WARNING "Unable to find 7z in the PATH. The zip archive will be disabled.")
endif()

# Configure the installer scripts
if (WIX_PATH)
    if ($ENV{PLATFORM} MATCHES "[xX]64") 
        set(PROGRAM_FILES_FOLDER "ProgramFiles64Folder" CACHE STRING "Use C:/Program Files on Windows 64-bit")
    else()
        set(PROGRAM_FILES_FOLDER "ProgramFilesFolder" CACHE STRING "Use C:/Program Files (x86) on Windows 64-bit")
    endif()
    configure_file(installer.cmake.wxs installer.wxs @ONLY)
else()
    configure_file(installer.cmake.nsi ${CMAKE_BINARY_DIR}/installer.nsi @ONLY)
endif()

# Paths to Qt DLLs
get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)

# Compiler Runtime DLLs
if (MSVC)
    # Visual Studio
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP true)
    include(InstallRequiredSystemLibraries)
    install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION .)
else()
    # MinGW
    get_filename_component(MINGW_DLL_DIR ${CMAKE_CXX_COMPILER} PATH)
    install(FILES
        ${MINGW_DLL_DIR}/libgcc_s_dw2-1.dll
        ${MINGW_DLL_DIR}/libstdc++-6.dll
        ${QT_DLL_DIR}/libwinpthread-1.dll
        DESTINATION .
        )
endif()

# Qt Libraries
install(FILES
    ${QT_DLL_DIR}/Qt5Core.dll
    ${QT_DLL_DIR}/Qt5Gui.dll
    ${QT_DLL_DIR}/Qt5OpenGL.dll
    ${QT_DLL_DIR}/Qt5PrintSupport.dll
    ${QT_DLL_DIR}/Qt5Svg.dll
    ${QT_DLL_DIR}/Qt5Widgets.dll
    ${QT_DLL_DIR}/Qt5Xml.dll
    ${CMAKE_SOURCE_DIR}/win/qt.conf
    DESTINATION .
    )
# Qwt Library
get_filename_component(QWT_DLL_DIR ${QWT_LIBRARY} PATH)
install(FILES
    ${QWT_DLL_DIR}/qwt.dll
    DESTINATION .
    )
# GSL Library
get_filename_component(GSL_DLL_DIR ${GSL_LIBRARY} PATH)
install(FILES
    ${GSL_DLL_DIR}/../bin/gsl.dll
    DESTINATION .
    )  
# Qt Platform Plugin
install(FILES
    ${QT_DLL_DIR}/../plugins/platforms/qwindows.dll
    DESTINATION plugins/platforms
    )
# Custom target to build the WiX installer. Should be run after the install
# target.
if (WIX_PATH)
    add_custom_target(installer
        COMMAND ${WIX_PATH}/candle.exe 
        -arch $ENV{platform}
        ${CMAKE_CURRENT_BINARY_DIR}/installer.wxs
        COMMAND ${WIX_PATH}/light.exe -ext WixUIExtension -ext WixUtilExtension
        -b ${CMAKE_INSTALL_PREFIX} -b ${CMAKE_SOURCE_DIR}/win
        -out "${PROJECT_LONGNAME}-$ENV{platform}-v${PROJECT_VERSION}-${PROJECT_GITHASH}.msi"
        installer.wixobj
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        VERBATIM
        )
endif()

if (7Z_PATH)
    message(${CMAKE_BINARY_DIR})
    message(${CMAKE_SOURCE_DIR})
    message(${CMAKE_CURRENT_BINARY_DIR})
    message(${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_target(archive
        COMMAND 7z a ${CMAKE_BINARY_DIR}/SigmaSpectra-$ENV{platform}-v${PROJECT_VERSION}-${PROJECT_GITHASH}.zip *
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dist
        VERBATIM
        )
endif()
