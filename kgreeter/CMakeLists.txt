cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(kgreeter CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTOUIC TRUE)

include(GNUInstallDirs)

find_package(Katie REQUIRED)
find_package(KDE4 REQUIRED)

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

include_directories(
    ${KATIE_INCLUDES}
    ${KDE4_INCLUDES}
    ${LIGHTDM_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIR}
)

add_definitions(
    ${LIGHTDM_DEFINITIONS}
)

add_executable(kgreeter kgreeter.cpp)
target_link_libraries(kgreeter
    Katie::Core
    Katie::Gui
    KDE4::kdeui
    ${LIGHTDM_LIBRARIES}
    ${GLIB2_LIBRARIES}
    gobject-2.0
)

set_target_properties(kgreeter PROPERTIES
    OUTPUT_NAME lightdm-kgreeter-greeter
)

install(
    TARGETS kgreeter
    DESTINATION ${CMAKE_INSTALL_FULL_SBINDIR}
)

install(
    FILES lightdm-kgreeter-greeter.conf
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/lightdm
)

install(
    FILES 50-lightdm-kgreeter-greeter.conf
    DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/lightdm/lightdm.conf.d
)

install(
    FILES lightdm-kgreeter-greeter.desktop
    DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/xgreeters
)
