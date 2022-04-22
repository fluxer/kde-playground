project(kpowercontrol)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

########### next target ###############

set(kpowercontrol_SRCS
    main.cpp
    kpowercontrol.cpp
)

add_executable(kpowercontrol ${kpowercontrol_SRCS})

target_link_libraries(kpowercontrol
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPOWERMANAGER_LIBS}
    ${KDE4_SOLID_LIBS}
)

install(TARGETS kpowercontrol ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

install(
    PROGRAMS kpowercontrol.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

install(
    FILES kpowercontrol.notifyrc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kpowercontrol
)