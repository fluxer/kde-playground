project(kemu)

cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

find_package(KDE4)
include(KDE4Defaults)

include_directories(${KDE4_INCLUDES})

set(kemu_sources
    main.cpp
    kemumainwindow.cpp
)

add_executable(kemu ${kemu_sources})
target_link_libraries(kemu KDE4::kdeui KDE4::kfile)

install(TARGETS kemu DESTINATION ${BIN_INSTALL_DIR})
install(FILES kemuui.rc DESTINATION ${DATA_INSTALL_DIR}/kemu)
install(PROGRAMS kemu.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
