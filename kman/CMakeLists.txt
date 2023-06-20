project(kman)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDELibs4 4.19.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(kman_sources
    main.cpp
    kmanmainwindow.cpp
)

add_executable(kman ${kman_sources})
target_link_libraries(kman
    KDE4::kdeui
    KDE4::kfile
    KDE4::karchive
)

install(
    TARGETS kman
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)
install(
    FILES kmanui.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kman
)
install(
    PROGRAMS kman.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
