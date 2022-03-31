cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(kgreeter CXX)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTOUIC TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include(GNUInstallDirs)

find_package(Katie REQUIRED)
find_package(KDE4 REQUIRED)

include_directories(
    ${KATIE_INCLUDES}
    ${KDE4_INCLUDES}
    /usr/include/lightdm-gobject-1
    /usr/include/gio-unix-2.0
    /usr/include/libmount
    /usr/include/blkid
    /usr/include/glib-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
    /usr/include/libxml2
)
link_directories(/usr/pkg/lib)

add_definitions(
)

add_executable(kgreeter kgreeter.cpp)
target_link_libraries(kgreeter
    Katie::Core
    Katie::Gui
    KDE4::kdeui
    glib-2.0
    gobject-2.0
    lightdm-gobject-1
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
