project(kprintjobs)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

########### next target ###############

set(kded_kprintjobs_SRCS 
    kded_kprintjobs.cpp
    kprintjobsimpl.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kprintjobs.xml
)

qt4_generate_dbus_interface(kded_kprintjobs.h org.kde.kprintjobs.xml)

kde4_add_plugin(kded_kprintjobs ${kded_kprintjobs_SRCS})
target_link_libraries(kded_kprintjobs PRIVATE
    KDE4::kdecore
    ${QT_QTDBUS_LIBRARY}
    cups
)

install(
    TARGETS kded_kprintjobs
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kprintjobs.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kprintjobs.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)


