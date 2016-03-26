# - Try to find FFTW
# Once done, this will define
#
#  FFTW_FOUND - system has FFTW
#  FFTW_INCLUDE_DIRS - the FFTW include directories
#  FFTW_LIBRARIES - link these to use FFTW

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(FFTW_PKGCONF fftw3)

# Include dir
find_path(FFTW_INCLUDE_DIR
  NAMES fftw3.h
  PATHS ${FFTW_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(FFTW_LIBRARY
  NAMES fftw3
  PATHS ${FFTW_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(FFTW_PROCESS_INCLUDES FFTW_INCLUDE_DIR)
set(FFTW_PROCESS_LIBS FFTW_LIBRARY)
libfind_process(FFTW)
