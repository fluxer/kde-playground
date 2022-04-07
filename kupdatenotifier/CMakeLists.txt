project(kupdatenotifier)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

########### next target ###############

set(kupdatenotifier_SRCS
    main.cpp
    kupdatenotifier.cpp
)

add_executable(kupdatenotifier ${kupdatenotifier_SRCS})

target_link_libraries(kupdatenotifier ${KDE4_KDEUI_LIBS})

install(TARGETS kupdatenotifier ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

install(
    PROGRAMS kupdatenotifier.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
