project (kde-playground)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

set(QT_MIN_VERSION "4.8.2")
find_package(KDE4 4.14.3 REQUIRED)
include(KDE4Defaults)
include(GenerateExportHeader)

set(LIBRARY_TYPE SHARED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

macro_optional_add_subdirectory (kdbg)
macro_optional_add_subdirectory (pykde4)
macro_optional_add_subdirectory (kamera)
macro_optional_add_subdirectory (kfilereplace)
# NOTE: you will need kdoctools from kdelibs ported for the help KIO slave
macro_optional_add_subdirectory (khelpcenter)
macro_optional_add_subdirectory (kaffeine)
macro_optional_add_subdirectory (kamoso)
macro_optional_add_subdirectory (kcachegrind)
macro_optional_add_subdirectory (kdepimlibs)
macro_optional_add_subdirectory (kdepim-runtime)
macro_optional_add_subdirectory (kdepim)
