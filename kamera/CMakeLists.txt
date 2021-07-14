project(kamera)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(KDE4 4.14.3 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

find_package(Gphoto2 2.5)
set_package_properties(Gphoto2 PROPERTIES
    URL "http://heanet.dl.sourceforge.net/sourceforge/gphoto"
    PURPOSE "Required to build kamera"
    TYPE REQUIRED
)

add_definitions(
    ${QT_DEFINITIONS}
    ${KDE4_DEFINITIONS}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${GPHOTO2_INCLUDE_DIR}
)

add_subdirectory(kcontrol) 


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)