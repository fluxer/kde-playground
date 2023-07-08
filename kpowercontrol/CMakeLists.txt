project(kpowercontrol)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDELibs4 4.21.0 REQUIRED)

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
    KDE4::kdeui
    KDE4::kpowermanager
    KDE4::solid
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