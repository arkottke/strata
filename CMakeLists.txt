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

cmake_minimum_required(VERSION 3.16)

# Default configuration values. These must be before the project command or
# they won't work in Windows.
# If no build type is specified, default to "Release"
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "None Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()
# Install to "dist" directory in Windows for testing and as a staging directory
# for the installer.
if (WIN32 AND NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX dist CACHE STRING "Install path prefix.")
endif()

project(strata)
set(PROJECT_LONGNAME "Strata")

execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE PROJECT_GITHASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(PROJECT_VERSION "0.9.0")

# Global CMake options
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Enable advanced features like:
#  - frequency-dependent moduli
option(ADVANCED_FEATURES "Compile with advanced features" ON)


# Required Qt5.16+ for Qt6 compatibility
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)


if(APPLE)
    # Detect Homebrew Qt6 prefix on both Intel and Apple Silicon Macs
    execute_process(
        COMMAND brew --prefix qt@6
        OUTPUT_VARIABLE _QT6_BREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _QT6_BREW_RESULT
    )
    if(_QT6_BREW_RESULT EQUAL 0 AND EXISTS "${_QT6_BREW_PREFIX}")
        list(APPEND CMAKE_PREFIX_PATH "${_QT6_BREW_PREFIX}")
    endif()
endif()

# Enable the C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure libraries
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Xml PrintSupport OpenGLWidgets Svg)
find_package(GSL REQUIRED)
find_package(qwt CONFIG REQUIRED)

if(COMMAND qt_standard_project_setup)
    qt_standard_project_setup()
endif()

include(GNUInstallDirs)

set(LIBS ${LIBS} GSL::gsl qwt::qwt)

if (ADVANCED_FEATURES)
    add_definitions(-DADVANCED_FEATURES)
endif()

if(MSVC)
    # Add define for math constants
    add_definitions(-D_USE_MATH_DEFINES)
    # Optimize for speed and link dynamically
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MD")
else()
    # Add optimization for GCC
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

add_subdirectory(source)
add_subdirectory(resources)

# Example regression tests using Python comparison script
find_package(Python3 COMPONENTS Interpreter)
if (Python3_FOUND)
    enable_testing()
    add_test(
        NAME example_regression
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_SOURCE_DIR}/scripts/compare_examples.py
            $<TARGET_FILE:strata>
            ${CMAKE_SOURCE_DIR}/example
    )
    set_tests_properties(example_regression PROPERTIES TIMEOUT 600)
endif()

# CPack Configuration
set(CPACK_PACKAGE_NAME "Strata")
set(CPACK_PACKAGE_VENDOR "Albert Kottke")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Equivalent linear site response analysis")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/win/appicon.ico")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
