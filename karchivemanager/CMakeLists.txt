project(karchivemanager)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(karchivemanager_sources
    strmode.c
    karchiveapp.cpp
    karchivemanager.cpp
    main.cpp
)

add_executable(karchivemanager ${karchivemanager_sources})
target_link_libraries(karchivemanager
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    z
    bz2
    archive
)

install(TARGETS karchivemanager DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(PROGRAMS karchivemanager.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
