# - Try to find Qwt
# Once done, this will define
#
#  Qwt_FOUND - system has Qwt
#  Qwt_INCLUDE_DIRS - the Qwt include directories
#  Qwt_LIBRARIES - link these to use Qwt

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

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Qwt_PROCESS_INCLUDES Qwt_INCLUDE_DIR)
set(Qwt_PROCESS_LIBS Qwt_LIBRARY)
libfind_process(Qwt)
