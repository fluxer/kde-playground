project(kwrite)

cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(${KDE4_INCLUDES})
add_definitions(${KDE4_DEFINITIONS} ${QT_DEFINITIONS})

set(kwrite_sources
    kwritemain.cpp
    kwriteapp.cpp
)

add_executable(kwrite ${kwrite_sources})
target_link_libraries(kwrite KDE4::ktexteditor KDE4::kio KDE4::kparts)

install(TARGETS kwrite  DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES kwriteui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kwrite)
install(PROGRAMS kwrite.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
