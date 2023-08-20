project(kupdatenotifier)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

########### next target ###############

set(kupdatenotifier_SRCS
    main.cpp
    kupdatenotifier.cpp
)

add_executable(kupdatenotifier ${kupdatenotifier_SRCS})

target_link_libraries(kupdatenotifier KDE4::kdeui)

install(
    TARGETS kupdatenotifier
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

########### next target ###############

install(
    PROGRAMS kupdatenotifier.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
