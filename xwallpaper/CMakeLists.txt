project(plasma-wallpaper-xwallpaper)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(xwallpaper_SRCS
    xwallpaper.cpp
)

kde4_add_plugin(plasma_wallpaper_xwallpaper ${xwallpaper_SRCS})
target_link_libraries(plasma_wallpaper_xwallpaper KDE4::plasma KDE4::kdeui)

install(
    TARGETS plasma_wallpaper_xwallpaper
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-wallpaper-xwallpaper.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
