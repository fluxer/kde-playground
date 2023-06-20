project(khash)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(khash_SRCS
    khash.cpp
)

add_executable(khash ${khash_SRCS})

target_link_libraries(khash
    KDE4::kdeui
    KDE4::kfile
    ${QT_QTNETWORK_LIBRARY}
)

install(TARGETS khash ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(
    FILES khash_checksum.desktop DESTINATION
    ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus
)
