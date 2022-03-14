project(ksnapshot)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(ksnapshot_SRCS
    ksnapshot.cpp
    snapshottimer.cpp
    ksnapshotwidget.ui
)

add_executable(ksnapshot ${ksnapshot_SRCS})

target_link_libraries(ksnapshot
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
)

install(TARGETS ksnapshot ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(
    PROGRAMS ksnapshot.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})
