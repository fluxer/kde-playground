project(boombox)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

find_package(Sqlite REQUIRED)
find_package(Taglib REQUIRED)

include_directories(${SQLITE_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} ${TAGLIB_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(WARNING "enabling debug output!")
  add_definitions(-DDEBUG)
else()
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()


add_subdirectory( src )
add_subdirectory( icons )
