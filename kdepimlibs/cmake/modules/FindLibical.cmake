# Find Libical
#
#  LIBICAL_FOUND - system has Libical with the minimum version needed
#  LIBICAL_INCLUDE_DIRS - the Libical include directories
#  LIBICAL_LIBRARIES - The libraries needed to use Libical
#  LIBICAL_VERSION = The value of ICAL_VERSION defined in ical.h
#  LIBICAL_MAJOR_VERSION = The library major version number
#  LIBICAL_MINOR_VERSION = The library minor version number

# Copyright (c) 2008,2010 Allen Winter <winter@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT LIBICAL_MIN_VERSION)
  set(LIBICAL_MIN_VERSION "0.33")
endif()

if (WIN32)
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _program_FILES_DIR)
endif()

set(LIBICAL_FIND_REQUIRED ${Libical_FIND_REQUIRED})
if(LIBICAL_INCLUDE_DIRS AND LIBICAL_LIBRARIES)

  # Already in cache, be silent
  set(LIBICAL_FIND_QUIETLY TRUE)

endif()

#set the root from the LIBICAL_BASE environment
file(TO_CMAKE_PATH "$ENV{LIBICAL_BASE}" libical_root )
#override the root from LIBICAL_BASE defined to cmake
if(DEFINED LIBICAL_BASE)
  file(TO_CMAKE_PATH "${LIBICAL_BASE}" libical_root )
endif()

find_path(LIBICAL_INCLUDE_DIRS NAMES libical/ical.h
  HINTS ${libical_root}/include ${_program_FILES_DIR}/libical/include ${KDE4_INCLUDE_DIR}
)

find_library(LIBICAL_LIBRARY NAMES ical libical
  HINTS ${libical_root}/lib ${_program_FILES_DIR}/libical/lib ${KDE4_LIB_DIR}
)
find_library(LIBICALSS_LIBRARY NAMES icalss libicalss
  HINTS ${libical_root}/lib ${_program_FILES_DIR}/libical/lib ${KDE4_LIB_DIR}
)
set(LIBICAL_LIBRARIES ${LIBICAL_LIBRARY} ${LIBICALSS_LIBRARY})

if(LIBICAL_INCLUDE_DIRS AND LIBICAL_LIBRARIES)
  set(FIND_LIBICAL_VERSION_SOURCE
    "#include <libical/ical.h>\n int main()\n {\n printf(\"%s\",ICAL_VERSION);return 1;\n }\n")
  set(FIND_LIBICAL_VERSION_SOURCE_FILE ${CMAKE_BINARY_DIR}/CMakeTmp/FindLIBICAL.cxx)
  file(WRITE "${FIND_LIBICAL_VERSION_SOURCE_FILE}" "${FIND_LIBICAL_VERSION_SOURCE}")

  set(FIND_LIBICAL_VERSION_ADD_INCLUDES
    "-DINCLUDE_DIRECTORIES:STRING=${LIBICAL_INCLUDE_DIRS}")

  if(NOT CMAKE_CROSSCOMPILING)
  try_run(RUN_RESULT COMPILE_RESULT
    ${CMAKE_BINARY_DIR}
    ${FIND_LIBICAL_VERSION_SOURCE_FILE}
    CMAKE_FLAGS "${FIND_LIBICAL_VERSION_ADD_INCLUDES}"
    RUN_OUTPUT_VARIABLE LIBICAL_VERSION)
  endif()

  if(COMPILE_RESULT AND RUN_RESULT EQUAL 1 AND NOT CMAKE_CROSSCOMPILING)
    message(STATUS "Found Libical version ${LIBICAL_VERSION}")
    if(${LIBICAL_VERSION} VERSION_LESS ${LIBICAL_MIN_VERSION})
      message(STATUS "Libcal version ${LIBICAL_VERSION} is too old. At least version ${LIBICAL_MIN_VERSION} is needed.")
      set(LIBICAL_INCLUDE_DIRS "")
      set(LIBICAL_LIBRARIES "")
    endif()
    if(NOT LIBICAL_VERSION VERSION_LESS 0.46)
      set(USE_ICAL_0_46 TRUE)
    endif()
    if(NOT LIBICAL_VERSION VERSION_LESS 1.00)
      set(USE_ICAL_1_0 TRUE)
    endif()

  else(COMPILE_RESULT AND RUN_RESULT EQUAL 1 AND NOT CMAKE_CROSSCOMPILING)
    if(NOT CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Unable to compile or run the libical version detection program.")
    endif()
  endif()

  #compute the major and minor version numbers
  if(NOT CMAKE_CROSSCOMPILING)
    string(REGEX REPLACE "\\..*$" "" LIBICAL_MAJOR_VERSION ${LIBICAL_VERSION})
    string(REGEX REPLACE "^.*\\." "" LIBICAL_MINOR_VERSION ${LIBICAL_VERSION})
  endif()

endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBICAL DEFAULT_MSG LIBICAL_LIBRARIES LIBICAL_INCLUDE_DIRS)

mark_as_advanced(LIBICAL_INCLUDE_DIRS LIBICAL_LIBRARIES)

