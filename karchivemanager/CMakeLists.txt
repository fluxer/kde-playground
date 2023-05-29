project(karchivemanager)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

find_package(LibArchive 3.0.3)
set_package_properties(LibArchive PROPERTIES
    DESCRIPTION "A library for dealing with a wide variety of archive file formats"
    URL "https://www.libarchive.org/"
    PURPOSE "Required for among others tar, tar.gz, tar.bz2 formats in KArchiveManager"
    TYPE REQUIRED
)

include_directories(${LibArchive_INCLUDE_DIRS})

set(karchivemanager_sources
    karchiveapp.cpp
    karchivemanager.cpp
    main.cpp
    # TODO: only if system does not have strmode()
    strmode.c
)

add_executable(karchivemanager ${karchivemanager_sources})
target_link_libraries(karchivemanager
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${LibArchive_LIBRARIES}
)

install(TARGETS karchivemanager DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(PROGRAMS karchivemanager.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
