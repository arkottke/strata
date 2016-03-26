# - Try to find GSL
# Once done, this will define
#
#  GSL_FOUND - system has GSL
#  GSL_INCLUDE_DIR - the GSL include directories
#  GSL_LIBRARY - link these to use GSL

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(GSL_PKGCONF gsl)

# Include dir
find_path(GSL_INCLUDE_DIR
  NAMES gsl_fft.h
  PATHS ${GSL_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES gsl
)

# Finally the library itself
find_library(GSL_LIBRARY
  NAMES gsl
  PATHS ${GSL_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(GSL_PROCESS_INCLUDES GSL_INCLUDE_DIR)
set(GSL_PROCESS_LIBS GSL_LIBRARY)
libfind_process(GSL)
