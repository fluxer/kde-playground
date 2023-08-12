project(plasma-wallpaper-anipaper)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(anipaper_SRCS
    anipaper.cpp
)

kde4_add_plugin(plasma_wallpaper_anipaper ${anipaper_SRCS})
target_link_libraries(plasma_wallpaper_anipaper KDE4::plasma KDE4::kdeui)

install(
    TARGETS plasma_wallpaper_anipaper
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-wallpaper-anipaper.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
