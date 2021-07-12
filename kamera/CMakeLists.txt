project(kamera)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(KDE4 4.14.3 REQUIRED)
include(CMakePushCheckState)
include(CheckSymbolExists)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

find_package(Gphoto2)
set_package_properties(Gphoto2 PROPERTIES
    URL "http://heanet.dl.sourceforge.net/sourceforge/gphoto"
    PURPOSE "Required to build kamera"
    TYPE REQUIRED
)

cmake_reset_check_state()
set(CMAKE_REQUIRED_INCLUDES "${GPHOTO2_INCLUDE_DIR}")
set(CMAKE_REQUIRED_LIBRARIES "${GPHOTO2_LIBRARIES}")
check_symbol_exists(gp_port_info_get_name "gphoto2/gphoto2.h" HAVE_GPHOTO2_5)
cmake_reset_check_state()

if(HAVE_GPHOTO2_5)
    message(STATUS "Detected libgphoto2 2.5.0 or newer")
else()
    message(STATUS "Detected libgphoto2 2.4 or older")
endif()

include(ConfigureChecks.cmake)

configure_file(
    config-kamera.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kamera.h
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
add_subdirectory(kioslave) 

install(
    FILES solid_camera.desktop
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/solid/actions
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)