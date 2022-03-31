project(kgreeter)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTOUIC TRUE)

include(GNUInstallDirs)

find_package(LightDM)
set_package_properties(LightDM PROPERTIES
    PURPOSE "Required for the greeter"
    DESCRIPTION "Cross-desktop display manager"
    URL "https://github.com/canonical/lightdm"
    TYPE REQUIRED
)

find_package(GLIB2)
set_package_properties(GLIB2 PROPERTIES
    PURPOSE "Required for the greeter"
    DESCRIPTION "Low-level core library that forms the basis for projects such as GTK and GNOME"
    URL "https://gitlab.gnome.org/GNOME/glib"
    TYPE REQUIRED
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-kgreeter.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kgreeter.h
    @ONLY
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(kcm)

include_directories(
    ${LIGHTDM_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIR}
)

add_definitions(
    ${LIGHTDM_DEFINITIONS}
)

add_executable(kgreeter kgreeter.cpp)
target_link_libraries(kgreeter
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${LIGHTDM_LIBRARIES}
    ${GLIB2_LIBRARIES}
    gobject-2.0
)

set_target_properties(kgreeter PROPERTIES
    OUTPUT_NAME lightdm-kgreeter-greeter
)

install(
    TARGETS kgreeter
    DESTINATION ${KDE4_SBIN_INSTALL_DIR}
)

install(
    FILES lightdm-kgreeter-greeter.conf
    DESTINATION ${KDE4_SYSCONF_INSTALL_DIR}/lightdm
)

install(
    FILES 50-lightdm-kgreeter-greeter.conf
    DESTINATION ${KDE4_SHARE_INSTALL_PREFIX}/lightdm/lightdm.conf.d
)

install(
    FILES lightdm-kgreeter-greeter.desktop
    DESTINATION ${KDE4_SHARE_INSTALL_PREFIX}/xgreeters
)
