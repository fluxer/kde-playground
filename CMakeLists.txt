project(kde-playground)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(KDE4 4.18.0 REQUIRED)
include(KDE4Defaults)

set(LIBRARY_TYPE SHARED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

macro_optional_add_subdirectory (kdbg)
macro_optional_add_subdirectory (pykde4)
macro_optional_add_subdirectory (kamera)
macro_optional_add_subdirectory (kfilereplace)
macro_optional_add_subdirectory (khelpcenter)
macro_optional_add_subdirectory (kaffeine)
# macro_optional_add_subdirectory (kamoso)
macro_optional_add_subdirectory (kcachegrind)
# macro_optional_add_subdirectory (kdepimlibs)
# macro_optional_add_subdirectory (kdepim-runtime)
# macro_optional_add_subdirectory (kdepim)
macro_optional_add_subdirectory (kman)
macro_optional_add_subdirectory (zeroconf-ioslave)
macro_optional_add_subdirectory (kdestopspy)
macro_optional_add_subdirectory (kvolume)
macro_optional_add_subdirectory (ksnapshot)
macro_optional_add_subdirectory (kupdatenotifier)