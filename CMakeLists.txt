project (kde-playground)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

#search packages used by KDE
set(QT_MIN_VERSION "4.8.2")
find_package(KDE4 4.14.3 REQUIRED)
include (KDE4Defaults)
include(GenerateExportHeader)

# shared libs
set (LIBRARY_TYPE SHARED)

# KDE/QT definitions we want
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DKTEXTEDITOR_NO_DEPRECATED)

# global include directories
include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

macro_optional_add_subdirectory (kdbg)
macro_optional_add_subdirectory (pykde4)
macro_optional_add_subdirectory (libkdcraw)
macro_optional_add_subdirectory (libkexiv2)
macro_optional_add_subdirectory (kamera)
macro_optional_add_subdirectory (kfilereplace)
macro_optional_add_subdirectory (khelpcenter)