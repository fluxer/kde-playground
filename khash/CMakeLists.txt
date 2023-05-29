project(khash)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDE4 4.23.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(khash_SRCS
    khash.cpp
)

add_executable(khash ${khash_SRCS})

target_link_libraries(khash
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${QT_QTNETWORK_LIBRARY}
)

install(TARGETS khash ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(
    PROGRAMS khash.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

install(
    FILES khash_checksum.desktop DESTINATION
    ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus
)
