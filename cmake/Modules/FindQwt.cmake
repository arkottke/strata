# - Try to find Qwt
# Once done, this will define
#
#  Qwt_FOUND - system has Qwt
#  Qwt_INCLUDE_DIR - the Qwt include directories
#  Qwt_LIBRARY - link these to use Qwt

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Qwt_PKGCONF qwt)

# Include dir
find_path(Qwt_INCLUDE_DIR
  NAMES qwt.h
  PATHS ${Qwt_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES qwt
)

# Finally the library itself
find_library(Qwt_LIBRARY
  NAMES qwt
  PATHS ${Qwt_PKGCONF_LIBRARY_DIRS}
)

# Find the version
file( STRINGS "${Qwt_INCLUDE_DIR}/qwt_global.h" qwt_global_h_contents REGEX "define QWT_VERSION_STR" )
string( REGEX REPLACE ".*([0-9].[0-9].[0-9]).*" "\\1" Qwt_VERSION ${qwt_global_h_contents} )

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Qwt_PROCESS_INCLUDES Qwt_INCLUDE_DIR)
set(Qwt_PROCESS_LIBS Qwt_LIBRARY)
libfind_process(Qwt)
