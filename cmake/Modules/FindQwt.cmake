# - Try to find QWT
# Once done, this will define
#
#  QWT_FOUND - system has QWT
#  QWT_INCLUDE_DIRS - the QWT include directories
#  QWT_LIBRARIES - link these to use QWT

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(QWT_PKGCONF qwt-qt5)

# Include dir
find_path(QWT_INCLUDE_DIR
  NAMES qwt.h
  PATHS ${QWT_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES qwt
)

# Finally the library itself
find_library(QWT_LIBRARY
  NAMES qwt
  PATHS ${QWT_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(QWT_PROCESS_INCLUDES QWT_INCLUDE_DIR)
set(QWT_PROCESS_LIBS QWT_LIBRARY)
libfind_process(QWT)
