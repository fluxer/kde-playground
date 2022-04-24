project(knetpkg)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(knetpkg_SRCS
    knetpkg.cpp
    knetpkg.ui
)

add_executable(knetpkg ${knetpkg_SRCS})

target_link_libraries(knetpkg
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
)

install(TARGETS knetpkg ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(
    PROGRAMS knetpkg.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
